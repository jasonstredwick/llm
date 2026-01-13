#pragma once


#include <curl/curl.h>
#if defined(_WIN32)
#   undef min
#   undef max
#   undef ERROR
#endif

#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

#include "../interface/policy.hpp"
#include "http.hpp"


namespace jai::llm::curl {


class HeaderList {
private:
    using Handle_t = std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)>;

    Handle_t list{nullptr, &curl_slist_free_all};

public:
    explicit HeaderList(const http::Headers& headers) {
        for (const auto& header : headers.GetHeaders()) {
            auto* next = curl_slist_append(list.get(), header.c_str());
            if (!next) { throw std::runtime_error("Failed to create HeaderList."); }
            list.reset(next);
        }
    }

    curl_slist* Get() const noexcept { return list.get(); }
};


class Attempt {
private:
    using Handle_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    enum class State {
        COMPLETED,
        FAILED,
        IN_PROGRESS,
        NOT_INITIALIZED
    };

private:
    const AttemptPolicy& policy;
    const std::string& request_body;

    Handle_t handle;
    State state{State::NOT_INITIALIZED};
    std::string error_message{};
    bool unhooked{true};

    std::vector<std::vector<std::byte>> read_buffer{};
    std::vector<std::vector<std::byte>> write_buffer{};
    std::vector<std::vector<std::byte>> header_buffer{};

public:
    explicit Attempt(CURLM* multi,
                     const AttemptPolicy& policy_,
                     const HeaderList& headers,
                     const std::string& body)
    : policy{policy_},
      request_body{body},
      handle{curl_easy_init(), &curl_easy_cleanup}
    {
        if (!handle) { Fail("Failed to construct Attempt.", true); }

        auto SetOpt = [h=handle.get()](auto opt, auto val) {
            const CURLcode code = curl_easy_setopt(h, opt, val);
            if (code != CURLE_OK) {
                Fail(std::string("Failed to create Attempt; failed to apply policy: ") + curl_easy_strerror(code),
                     true);
            }
        };

        policy.connect_timeout        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_CONNECTTIMEOUT_MS, v.count()); });
        policy.total_timeout          .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_TIMEOUT_MS,        v.count()); });
        policy.low_speed_time         .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_LOW_SPEED_TIME,    v.count()); });
        policy.low_speed_bytes_per_sec.and_then([&SetOpt](auto v) { SetOpt(CURLOPT_LOW_SPEED_LIMIT,   static_cast<long>(v)); });
        policy.http_version           .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_HTTP_VERSION,
                                                                            v == HTTPVersion::HTTP1_1 ?
                                                                                CURL_HTTP_VERSION_1_1 :
                                                                                v == HTTPVersion::HTTP2 ?
                                                                                    CURL_HTTP_VERSION_2 :
                                                                                    CURL_HTTP_VERSION_3); });
        policy.follow_redirects       .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FOLLOWLOCATION,    v ? 1L : 0L); });
        policy.max_redirects          .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_MAXREDIRS,         static_cast<long>(v)); });
        policy.ca_bundle_path         .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_CAINFO,            v.c_str()); });
        policy.client_cert_path       .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLCERT,           v.c_str()); });
        policy.client_key_path        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLKEY,            v.c_str()); });
        policy.tls_min_version        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLVERSION,
                                                                            v == TLSVersion::TLSv1_2 ?
                                                                                CURL_SSLVERSION_TLSv1_2 :
                                                                                CURL_SSLVERSION_TLSv1_3); });
        policy.verify_peer            .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSL_VERIFYPEER,    v ? 1L : 0L); });
        policy.verify_host            .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSL_VERIFYHOST,    v ? 2L : 0L); });
        policy.forbid_connection_reuse.and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FORBID_REUSE,      v ? 1L : 0L); });
        policy.fail_on_http_error     .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FAILONERROR,       v ? 1L : 0L); });

        SetOpt(CURLOPT_HTTPHEADER, header_list.Get());
        SetOpt(CURLOPT_NOSIGNAL, 1L);
        SetOpt(CURLOPT_WRITEFUNCTION, WriteCallback);
        SetOpt(CURLOPT_WRITEDATA, this);
        SetOpt(CURLOPT_HEADERFUNCTION, HeaderCallback);
        SetOpt(CURLOPT_HEADERDATA, this);
        SetOpt(CURLOPT_READFUNCTION, ReadCallback);
        SetOpt(CURLOPT_READDATA, this);
        SetOpt(CURLOPT_XFERINFOFUNCTION, XferInfoCallback);
        SetOpt(CURLOPT_XFERINFODATA, this);
        SetOpt(CURLOPT_NOPROGRESS, 0L);
        policy->enable_debugging.and_then([&](auto v) {
            if (v) {
                SetOpt(CURLOPT_DEBUGFUNCTION, DebugCallback);
                SetOpt(CURLOPT_DEBUGDATA, this);
                SetOpt(CURLOPT_VERBOSE, 1L);
            }
        });
        SetOpt(CURLOPT_PRIVATE, this);

        CURLMcode code = curl_multi_add_handle(multi, handle);
        if (code != CURLM_OK) {
            Fail(std::string("Failed to create Attempt; parent registration failed: ") + curl_multi_strerror(code), true);
        }
        unhooked = false;

        state = State::IN_PROGRESS;
    }

    Attempt() = delete;
    Attempt(const Attempt&) = delete;
    Attempt(Attempt&&) = delete;
    Attempt& operator=(const Attempt&) = delete;
    Attempt& operator=(Attempt&&) = delete;
    ~Attempt() = default;

    void Finalize(CURLM* multi, CURLcode result) noexcept {
        CURLMcode unhook_code = curl_multi_remove_handle(multi, handle);
        if (unhook_code == CURLM_OK) { unhooked = true; }

        if (result != CURLE_OK) { Fail(std::string("Attempt failed: ") + curl_easy_strerror(result)); }
        else {
            // handle final data reads from CURL* before deleting handle.
            // curl_easy_getinfo();
        }

        handle.reset();
        if (state != State::FAILED) { state = State::COMPLETED; }
    }

    State GetState() const { return state; }
    bool IsCompleted() const { return state == State::COMPLETED; }
    bool IsDone() const { return state == State::COMPLETED || state == State::FAILED; }
    bool IsFailed() const { return state == State::FAILED; }
    bool IsUnhooked() const { return unhooked; }
    const std::string& GetErrorMessage() const { return error_message; }

private:
    void Fail(const std::string message, bool should_raise = false) {
        state = State::FAILED;
        error_message = message;
        if (should_raise) { throw std::runtime_error(error_message); }
    }

    // Callback Handlers
    int OnDebug(CURL* debug_handle, curl_infotype type, std::byte *data, size_t size) {
        // See https://curl.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
        return static_cast<int>(size);
    }

    size_t OnHeader(std::byte* ptr, size_t size, size_t nmemb) {
        header_buffer.push_back(std::vector<std::byte>(ptr, ptr + size * nmemb));
        return size * nmemb;
    }

    size_t OnRead(std::byte* ptr, size_t size, size_t nmemb) {
        if (read_index >= read_buffer.size()) { return 0; }
        std::vector<std::byte>& buffer = read_buffer[read_index];
        size_t bytes_to_read = std::min(buffer.size() - read_index, size * nmemb);
        std::copy(buffer.begin() + read_index, buffer.begin() + read_index + bytes_to_read, ptr);
        read_index += bytes_to_read;
        return bytes_to_read;
        //return size * nmemb;
    }

    size_t OnWrite(std::byte* ptr, size_t size, size_t nmemb) {
        write_buffer.push_back(std::vector<std::byte>(ptr, ptr + size * nmemb));
        return size * nmemb;
    }

    size_t OnXferInfo(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        return 0;
    }

private:
    // Raw callbacks
    static size_t DebugCallback(CURL* debug_handle, curl_infotype type, char* data, size_t size, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnDebug(debug_handle, type, static_cast<std::byte*>(data), size);
    }

    static size_t HeaderCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnHeader(static_cast<std::byte*>(ptr), size, nmemb);
    }

    static size_t ReadCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnRead(static_cast<std::byte*>(ptr), size, nmemb);
    }

    static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnWrite(static_cast<std::byte*>(ptr), size, nmemb);
    }

    static int XferInfoCallback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnXferInfo(dltotal, dlnow, ultotal, ulnow);
    }
};


class Interface {
private:
    using Handle_t = std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)>;

private:
    inline static Global global{};

    const ConnectionPolicy& policy;
    Handle_t handle;

public:
    explicit Interface(const ConnectionPolicy& policy_)
    : policy{policy_},
      handle{curl_multi_init(), &curl_multi_cleanup}
    {
        if (!handle) { throw std::runtime_error("Failed to create curl::Interface; curl_multi_init failed."); }

        auto SetOpt = [h=handle.get()](auto opt, auto val) {
            const CURLMcode code = curl_multi_setopt(h, opt, val);
            if (code != CURLM_OK) {
                throw std::runtime_error(std::string("Failed to create curl::Interface; failed to apply policy: ") +
                                         curl_multi_strerror(code));
            }
        };

        policy->max_total_connections.and_then([&SetOpt](auto v) {
            SetOpt(CURLMOPT_MAX_TOTAL_CONNECTIONS, static_cast<long>(v)); });
        policy->max_http2_streams_per_connection.and_then([&SetOpt](auto v) {
            SetOpt(CURLMOPT_MAX_CONCURRENT_STREAMS, static_cast<long>(v)); });
    }

    Interface() = delete;
    Interface(const Interface&) = delete;
    Interface(Interface&&) = delete;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(Interface&&) = delete;
    ~Interface() = default;

    Attempt MakeAttempt(const AttemptPolicy& policy,
                        const std::vector<std::string>& headers,
                        const std::string& body) const
    {
        return Attempt(handle.get(), policy, headers, body);
    }

    Attempt MakeAttemptSync(const AttemptPolicy& policy,
                            const std::vector<std::string>& headers,
                            const std::string& body) const
    {
        Attempt attempt{handle.get(), policy, headers, body};
        while (!attempt.IsDone()) { ExecOnce(); }
        return attempt;
    }

    std::vector<Attempt*> ExecOnce() {
        int still_running{0};
        CURLMcode code = curl_multi_perform(handle.get(), &still_running);
        if (code != CURLM_OK) {
            throw std::runtime_error("curl::Interface::ExecOnce curl_multi_perform failed with code " +
                                     std::to_string(code));
        }

        std::vector<Attempt*> attempts{};

        CURLMsg* msg{nullptr};
        int msgs_left{0};
        while ((msg = curl_multi_info_read(handle.get(), &msgs_left))) {
            if (msg->msg != CURLMSG_DONE) { continue; }

            void* raw_ptr{nullptr};
            CURLcode code = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &raw_ptr);
            if (code != CURL_OK) {
                throw std::runtime_error("curl::Interface::ExecOnce failed to retrieve attempt: " +
                                         std::to_string(code));
            }

            Attempt* attempt = static_cast<Attempt*>(raw_ptr);
            attempts.push_back(attempt);
            attempt->Finalize(msg->data.result, handle.get());
        }

        return attempts;
    }

private:
    struct Global {
        Global() { curl_global_init(CURL_GLOBAL_DEFAULT); }
        ~Global() { curl_global_cleanup(); }
    };
};


} // namespace jai::llm::curl
