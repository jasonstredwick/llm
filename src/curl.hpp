#pragma once


#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../interface/http.hpp"
#include "../interface/policy.hpp"
#include "http.hpp"


struct CURL;
struct CURLM;
struct curl_slist;


namespace jai::llm::curl {


class HeaderList {
private:
    using Handle_t = std::unique_ptr<curl_slist, void(*)(curl_slist*)>;

    Handle_t list;

public:
    explicit HeaderList(const http::Headers& headers);

    HeaderList(const HeaderList&) = delete;
    HeaderList(HeaderList&&) noexcept = default;
    HeaderList& operator=(const HeaderList&) = delete;
    HeaderList& operator=(HeaderList&&) noexcept = default;
    ~HeaderList() noexcept = default;

    curl_slist* Get() const noexcept { return list.get(); }
};


struct Response {
    enum class State : uint32_t {
        COMPLETED,
        FAILED,
        IN_PROGRESS,
        NOT_INITIALIZED
    };

    // Availability is monotonic and only moves forward
    enum class Availability : uint32_t {
        NOT_INITIALIZED,   // available: state, data_state, error_message
        STARTED,           // available: current_leg_* // estimates
        UPLOAD_COMPLETE,   
        DOWNLOAD_COMPLETE, // available: body, headers, abnormal_headers
        FINAL              // available: status_code, http_version, redirect_count, total_time_us, effective_url,
                           //            total_wire_bytes_downloaded, total_wire_bytes_uploaded
    };

    State state{State::NOT_INITIALIZED};
    Availability availability{Availability::NOT_INITIALIZED};
    int64_t status_code{-1};
    int64_t http_version{-1};
    int64_t redirect_count{-1};
    int64_t current_leg_download_progress_bytes{-1};
    int64_t current_leg_download_total_estimate_bytes{-1};
    int64_t current_leg_upload_progress_bytes{-1};
    int64_t current_leg_upload_total_estimate_bytes{-1};
    int64_t total_time_us{-1};
    int64_t total_wire_bytes_downloaded{-1};
    int64_t total_wire_bytes_uploaded{-1};
    std::string effective_url{};
    std::vector<std::byte> body{};
    std::vector<std::vector<std::byte>> headers{};
    std::vector<std::vector<std::byte>> abnormal_headers{std::vector<std::byte>{}}; // num headers + 1
    std::string error_message{};
};


class Attempt {
private:
    using Handle_t = std::unique_ptr<CURL, void(*)(CURL*)>;

private:
    Handle_t handle;
    Response response{};

    bool cancel_requested{false};
    bool header_open{false};
    bool unhooked{true};

public:
    explicit Attempt(CURLM* multi,
                     const AttemptPolicy& policy,
                     http::Method method,
                     const std::string& url,
                     const HeaderList& header_list,
                     const std::vector<std::byte>& body);

    Attempt() = delete;
    Attempt(const Attempt&) = delete;
    Attempt(Attempt&&) noexcept = delete;
    Attempt& operator=(const Attempt&) = delete;
    Attempt& operator=(Attempt&&) noexcept = delete;
    ~Attempt() noexcept = default;

    void Finalize(CURLM* multi, CURLcode result) noexcept;

    const std::string& GetErrorMessage() const { return response.error_message; }
    const Response& GetResponse() const { return response; }
    Response::State GetState() const { return response.state; }
    bool IsCompleted() const { return response.state == Response::State::COMPLETED; }
    bool IsDone() const { return response.state == Response::State::COMPLETED ||
                                 response.state == Response::State::FAILED; }
    bool IsFailed() const { return response.state == Response::State::FAILED; }
    bool IsUnhooked() const { return unhooked; }

private:
    void ExtractMetadata();

    void Fail(const std::string message, bool should_raise = false) {
        response.state = Response::State::FAILED;
        response.error_message = message;
        if (should_raise) { throw std::runtime_error(response.error_message); }
    }

    // Callback Handlers
    int OnDebug(CURL* debug_handle, curl_infotype type, std::byte *data, size_t size);
    size_t OnHeader(std::byte* ptr, size_t size, size_t nmemb);
    size_t OnRead(std::byte* ptr, size_t size, size_t nmemb);
    size_t OnWrite(std::byte* ptr, size_t size, size_t nmemb);
    int OnXferInfo(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

private:
    // Raw callbacks
    static int DebugCallback(CURL* debug_handle, curl_infotype type, char* data, size_t size, void* userdata) {
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

    static int XferInfoCallback(void* userdata,
                                curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnXferInfo(dltotal, dlnow, ultotal, ulnow);
    }
};


class Interface {
private:
    using Handle_t = std::unique_ptr<CURLM, decltype(&curl_multi_cleanup)>;

private:
    inline static Global global{};

    Handle_t handle;

public:
    explicit Interface(const ConnectionPolicy& policy_);

    Interface() = delete;
    Interface(const Interface&) = delete;
    Interface(Interface&&) noexcept = delete;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(Interface&&) noexcept = delete;
    ~Interface() noexcept = default;

    Attempt MakeAttempt(const AttemptPolicy& policy,
                        http::Method method,
                        const std::string& url,
                        const HeaderList& header_list,
                        const std::vector<std::byte>& body) const
    {
        return Attempt{handle.get(), policy, method, url, header_list, body};
    }

    Attempt MakeAttemptSync(const AttemptPolicy& policy,
                            http::Method method,
                            const std::string& url,
                            const HeaderList& header_list,
                            const std::vector<std::byte>& body) const
    {
        Attempt attempt{handle.get(), policy, method, url, header_list, body};
        while (!attempt.IsDone()) { ExecOnce(); }
        return attempt;
    }

    std::vector<Attempt*> ExecOnce();

private:
    struct Global {
        Global();
        ~Global() noexcept;
    };
};


} // namespace jai::llm::curl
