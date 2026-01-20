#pragma once


#include "../interface/policy.hpp"
#include "http.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>


namespace jai::llm::curl {


class Interface;


class HeaderList {
private:
    using Handle_t = std::unique_ptr<void, void(*)(void*)>;

    Handle_t list;

public:
    explicit HeaderList(const http::RequestHeaders& headers);

    HeaderList(const HeaderList&) = delete;
    HeaderList(HeaderList&&) noexcept = default;
    HeaderList& operator=(const HeaderList&) = delete;
    HeaderList& operator=(HeaderList&&) noexcept = default;
    ~HeaderList() noexcept = default;

    void* Get() const noexcept { return list.get(); }
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
        UPLOAD_COMPLETE,   //
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
    using Handle_t = std::unique_ptr<void, void(*)(void*)>;

private:
    Handle_t handle;
    Response response{};

    bool cancel_requested{false};
    bool header_open{false};
    bool unhooked{true};

public:
    explicit Attempt(Interface& interface,
                     const AttemptPolicy& policy,
                     http::Method method,
                     const std::string& url,
                     const HeaderList& header_list,
                     const std::vector<std::byte>& body);

    // No copy/move constructors; this pointer locked by libcurl.
    Attempt(const Attempt&) = delete;
    Attempt(Attempt&&) noexcept = delete;
    Attempt& operator=(const Attempt&) = delete;
    Attempt& operator=(Attempt&&) noexcept = delete;
    ~Attempt() noexcept = default;

    void Finalize(Interface& interface, const std::string& result_error_str) noexcept;

    const std::string& GetErrorMessage() const { return response.error_message; }
    const Response& GetResponse() const { return response; }
    Response::State GetState() const { return response.state; }
    bool IsCompleted() const { return response.state == Response::State::COMPLETED; }
    bool IsDone() const { return response.state == Response::State::COMPLETED ||
                                 response.state == Response::State::FAILED; }
    bool IsFailed() const { return response.state == Response::State::FAILED; }
    bool IsUnhooked() const { return unhooked; }

private:
    void ExtractMetadata() noexcept;

    void Fail(const std::string message, bool should_raise = false) {
        response.state = Response::State::FAILED;
        response.error_message = message;
        if (should_raise) { throw std::runtime_error(response.error_message); }
    }

    // Callback Handlers
    struct RawHandlers;
    friend struct RawHandlers;

    size_t OnHeader(std::span<const std::byte> ptr);
    size_t OnRead(std::span<const std::byte> ptr);
    size_t OnWrite(std::span<const std::byte> ptr);
    int OnXferInfo(int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow);
};


class Interface {
private:
    using Handle_t = std::unique_ptr<void, void(*)(void*)>;

    struct Global {
        Global();
        ~Global() noexcept;
    };

private:
    static Global global;

    Handle_t handle;

public:
    explicit Interface(const ConnectionPolicy& policy);

    Interface() = delete;
    Interface(const Interface&) = delete;
    Interface(Interface&&) noexcept = delete;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(Interface&&) noexcept = delete;
    ~Interface() noexcept = default;

    std::string AddHandle(void* curl_easy_handle);
    std::vector<Attempt*> ExecOnce();
    std::string RemoveHandle(void* curl_easy_handle) noexcept;
};


}
