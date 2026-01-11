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


namespace jai::llm::curl {


enum class HTTPVersion {
    HTTP1_1,
    HTTP2//,
    //HTTP3
};


enum class TLSVersion {
    TLSv1_2,
    TLSv1_3
};


enum class State {
    COMPLETED,
    FAILED,
    IN_PROGRESS,
    NOT_INITIALIZED
};


class Attempt {
private:
    using Handle_t = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

    std::shared_ptr<const Policy> policy{};
    Handle_t handle{};
    State state{State::NOT_INITIALIZED};

    size_t read_index{0};
    std::vector<std::vector<std::byte>> read_buffer{};
    std::vector<std::vector<std::byte>> write_buffer{};
    std::vector<std::vector<std::byte>> header_buffer{};

public:
    struct Policy {
        // Timeouts
        std::optional<std::chrono::milliseconds> connect_timeout{};
        std::optional<std::chrono::milliseconds> total_timeout{};
        std::optional<std::chrono::seconds> low_speed_time{};
        std::optional<size_t> low_speed_bytes_per_sec{};

        // Protocol control
        // If unset, libcurl negotiates (HTTP/1.1 + upgrades).
        // If set, the specified version is forced with no fallback.
        std::optional<HTTPVersion> http_version{};

        // Redirect handling
        std::optional<bool> follow_redirects{};
        std::optional<size_t> max_redirects{};

        // TLS requirements
        std::optional<std::string> ca_bundle_path{};
        std::optional<std::string> client_cert_path{};
        std::optional<std::string> client_key_path{};
        std::optional<TLSVersion> tls_min_version{};
        std::optional<bool> verify_peer{};
        std::optional<bool> verify_host{};

        // Request behavior
        std::optional<bool> forbid_connection_reuse{};

        // Transfer behavior
        std::optional<bool> fail_on_http_error{}; // NOTE: often better left false

        // Debugging
        std::optional<bool> enable_debugging{};
    };

    explicit Attempt(std::shared_ptr<const Policy> policy_, CURLM* multi) : policy{std::move(policy_)} {
        if (!policy) { state = State::FAILED; throw std::runtime_error("Policy not provided."); }
        handle = curl_easy_init();
        if (!handle) { state = State::FAILED; throw std::runtime_error("Failed to create curl easy handle."); }

        auto SetOpt = [h=handle.get()](auto opt, auto val) {
            const CURLcode code = curl_easy_setopt(h, opt, val);
            if (code != CURLE_OK) {
                state = State::FAILED;
                throw std::runtime_error(std::string("curl_easy_setopt failed: ") + curl_easy_strerror(code));
            }
        };
        policy->connect_timeout        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_CONNECTTIMEOUT_MS, v.count()); });
        policy->total_timeout          .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_TIMEOUT_MS,        v.count()); });
        policy->low_speed_time         .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_LOW_SPEED_TIME,    v.count()); });
        policy->low_speed_bytes_per_sec.and_then([&SetOpt](auto v) { SetOpt(CURLOPT_LOW_SPEED_LIMIT,   static_cast<long>(v)); });
        policy->http_version           .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_HTTP_VERSION,
                                                                            v == HTTPVersion::HTTP1_1 ?
                                                                                CURL_HTTP_VERSION_1_1 :
                                                                                v == HTTPVersion::HTTP2 ?
                                                                                    CURL_HTTP_VERSION_2 :
                                                                                    CURL_HTTP_VERSION_3); });
        policy->follow_redirects       .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FOLLOWLOCATION,    v ? 1L : 0L); });
        policy->max_redirects          .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_MAXREDIRS,         static_cast<long>(v)); });
        policy->ca_bundle_path         .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_CAINFO,            v.c_str()); });
        policy->client_cert_path       .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLCERT,           v.c_str()); });
        policy->client_key_path        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLKEY,            v.c_str()); });
        policy->tls_min_version        .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSLVERSION,
                                                                            v == TLSVersion::TLSv1_2 ?
                                                                                CURL_SSLVERSION_TLSv1_2 :
                                                                                CURL_SSLVERSION_TLSv1_3); });
        policy->verify_peer            .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSL_VERIFYPEER,    v ? 1L : 0L); });
        policy->verify_host            .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_SSL_VERIFYHOST,    v ? 2L : 0L); });
        policy->forbid_connection_reuse.and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FORBID_REUSE,      v ? 1L : 0L); });
        policy->fail_on_http_error     .and_then([&SetOpt](auto v) { SetOpt(CURLOPT_FAILONERROR,       v ? 1L : 0L); });

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

        if (!multi) { state = State::FAILED; throw std::runtime_error("Failed to create curl multi handle."); }
        CURLMcode code = curl_multi_add_handle(multi, handle);
        if (code != CURLM_OK) {
            state = State::FAILED;
            throw std::runtime_error(std::string("curl_multi_add_handle failed: ") + curl_multi_strerror(code));
        }
        state = State::IN_PROGRESS;
    }

    Attempt() = delete;
    Attempt(const Attempt&) = delete;
    Attempt(Attempt&&) = delete;
    Attempt& operator=(const Attempt&) = delete;
    Attempt& operator=(Attempt&&) = delete;
    ~Attempt() = default;

    void Finalize(CURLcode result, CURLM* multi) {
        if (result != CURLE_OK) { state = State::FAILED; }
        if (multi) {
            curl_multi_remove_handle(multi, handle);
        }
        if (state != State::FAILED) {
            // handle final data reads from CURL* before deleting handle.
            // curl_easy_getinfo();
        }
        handle.reset();
        if (state != State::FAILED) { state = State::COMPLETED; }
    }

    State GetState() const { return state; }
    bool IsDone() const { return state == State::COMPLETED || state == State::FAILED; }

private:
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
    using CurlMultiHandle_t = std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)>;

    inline static Global global{};

    std::shared_ptr<const Policy> policy{};
    CurlMultiHandle_t handle{nullptr, &curl_multi_cleanup};

public:
    struct Policy {
        std::optional<size_t> max_total_connections{};
        std::optional<size_t> max_http2_streams_per_connection{};
    };

    explicit Interface(std::shared_ptr<const Policy> policy_) : policy(policy_) {
        if (!policy) { throw std::runtime_error("Policy is not provided."); }
        handle.reset(curl_multi_init());
        if (!handle) { throw std::runtime_error("Failed to create curl multi handle."); }

        auto SetOpt = [h=handle.get()](auto opt, auto val) {
            const CURLMcode code = curl_multi_setopt(h, opt, val);
            if (code != CURLM_OK) {
                throw std::runtime_error(std::string("curl_multi_setopt failed: ") + curl_multi_strerror(code));
            }
        };
        policy->max_total_connections.and_then([&SetOpt](auto v) {
            SetOpt(CURLMOPT_MAX_TOTAL_CONNECTIONS, static_cast<long>(v)); });
        policy->max_http2_streams_per_connection.and_then([&SetOpt](auto v) {
            SetOpt(CURLMOPT_MAX_CONCURRENT_STREAMS, static_cast<long>(v)); });
    }

    Interface() = delete;
    Interface(const Interface&) = delete;
    Interface(Interface&&) = default;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(Interface&&) = default;

    ~Interface() = default;

    Attempt MakeAttempt(Attempt::Policy policy = {}) const { return Attempt(policy, handle); }
    Attempt MakeAttemptSync(Attempt::Policy policy = {}) const {
        Attempt attempt(policy, handle);
        while (!attempt.IsDone()) { ExecOnce(); }
        return attempt;
    }

    void ExecOnce() {
        int still_running = 0;
        CURLMcode code = curl_multi_perform(handle, &still_running);
        if (code != CURLM_OK) { throw std::runtime_error("Failed to perform curl multi."); }

        CURLMsg* msg;
        int msgs_left;
        while ((msg = curl_multi_info_read(handle, &msgs_left))) {
            if (msg->msg != CURLMSG_DONE) { continue; }
            void* raw_ptr = nullptr;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &raw_ptr);
            Attempt* attempt = static_cast<Attempt*>(raw_ptr);
            attempt->Finalize(msg->data.result, handle.get());
        }
    }

private:
    struct Global {
        Global() { curl_global_init(CURL_GLOBAL_DEFAULT); }
        ~Global() { curl_global_cleanup(); }
    };
};


} // namespace jai::llm::curl
