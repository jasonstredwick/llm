#include "curl.hpp"


#include <curl/curl.h>
#if defined(_WIN32)
#   undef min
#   undef max
#   undef ERROR
#   undef DELETE
#endif


#include <algorithm>
#include <ranges>


namespace jai::llm::curl {


template <typename T> T* ToCurl(void* ptr) { return static_cast<T*>(ptr); }


/**
 * Raw callbacks
 */
 struct Attempt::RawHandlers {
    static int DebugCallback(CURL* debug_handle, curl_infotype type, char* data, size_t size, void* userdata) {
        auto OnDebug = []([[maybe_unused]] Attempt* attempt,
                        [[maybe_unused]] CURL* debug_handle,
                        [[maybe_unused]] curl_infotype type,
                        [[maybe_unused]] std::span<const std::byte> data) -> int
        {
            // See https://curl.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
            return 0;
        };
        auto attempt = static_cast<Attempt*>(userdata);
        std::span<char> buffer(data, size);
        return OnDebug(attempt, debug_handle, type, std::as_bytes(buffer));
    }


    static size_t HeaderCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        std::span<char> buffer(ptr, size * nmemb);
        return attempt->OnHeader(std::as_bytes(buffer));
    }


    static size_t ReadCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        std::span<char> buffer(ptr, size * nmemb);
        return attempt->OnRead(std::as_bytes(buffer));
    }


    static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        auto attempt = static_cast<Attempt*>(userdata);
        std::span<char> buffer(ptr, size * nmemb);
        return attempt->OnWrite(std::as_bytes(buffer));
    }


    static int XferInfoCallback(void* userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        auto attempt = static_cast<Attempt*>(userdata);
        return attempt->OnXferInfo(static_cast<int64_t>(dltotal),
                                static_cast<int64_t>(dlnow),
                                static_cast<int64_t>(ultotal),
                                static_cast<int64_t>(ulnow));
    }
 };


/**
 * HeaderList
 */
void DeleteHeaderListHandle(void* ptr) { curl_slist_free_all(ToCurl<curl_slist>(ptr)); }


HeaderList::HeaderList(const http::Headers& headers) : list{nullptr, &DeleteHeaderListHandle} {
    for (const auto& header : headers.GetHeaders()) {
        auto* new_list = curl_slist_append(ToCurl<curl_slist>(list.get()), header.c_str());
        if (!new_list) { throw std::runtime_error("Failed to create HeaderList."); }
        list.reset(static_cast<void*>(new_list));
    }
}


/**
 * Attempt
 */
void AttemptEasyCleanup(void* ptr) { curl_easy_cleanup(ToCurl<CURL>(ptr)); }


Attempt::Attempt(Interface& interface,
                 const AttemptPolicy& policy,
                 http::Method method,
                 const std::string& url,
                 const HeaderList& header_list,
                 const std::vector<std::byte>& body)
: handle{static_cast<void*>(curl_easy_init()), &AttemptEasyCleanup}
{
    if (!handle) { Fail("Failed to construct Attempt.", true); }
    CURL* curl_ptr = ToCurl<CURL>(handle.get());

    try {
        const size_t buffer_size = 131072; // 128 KB
        response.body.reserve(buffer_size);
    } catch(...) {
        Fail(std::string("Failed to create Attempt; failed to allocate response buffers."), true);
    }

    auto SetOpt = [this, h=curl_ptr](auto opt, auto val) {
        const CURLcode code = curl_easy_setopt(h, opt, val);
        if (code != CURLE_OK) {
            Fail(std::string("Failed to create Attempt; failed to apply policy: ") + curl_easy_strerror(code), true);
        }
    };

    // POLICY
    if (policy.connect_timeout)           { SetOpt(CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(policy.connect_timeout->count())); }
    if (policy.total_timeout)             { SetOpt(CURLOPT_TIMEOUT_MS,        static_cast<long>(policy.total_timeout->count())); }
    if (policy.low_speed_time)            { SetOpt(CURLOPT_LOW_SPEED_TIME,    static_cast<long>(policy.low_speed_time->count())); }
    if (policy.low_speed_bytes_per_sec)   { SetOpt(CURLOPT_LOW_SPEED_LIMIT,   static_cast<long>(policy.low_speed_bytes_per_sec.value())); }
    if (policy.http_version)              { SetOpt(CURLOPT_HTTP_VERSION,
                                                   policy.http_version.value() == HTTPVersion::HTTP1_1 ? CURL_HTTP_VERSION_1_1 :
                                                   policy.http_version.value() == HTTPVersion::HTTP2   ? CURL_HTTP_VERSION_2 :
                                                                                                         CURL_HTTP_VERSION_3); }
    if (policy.follow_redirects)          { SetOpt(CURLOPT_FOLLOWLOCATION, policy.follow_redirects.value() ? 1L : 0L); }
    if (policy.max_redirects)             { SetOpt(CURLOPT_MAXREDIRS, static_cast<long>(policy.max_redirects.value())); }
    if (policy.ca_bundle_path)            { SetOpt(CURLOPT_CAINFO, policy.ca_bundle_path.value().c_str()); }
    if (policy.client_cert_path)          { SetOpt(CURLOPT_SSLCERT, policy.client_cert_path.value().c_str()); }
    if (policy.client_key_path)           { SetOpt(CURLOPT_SSLKEY, policy.client_key_path.value().c_str()); }
    if (policy.tls_min_version)           { SetOpt(CURLOPT_SSLVERSION,
                                                   policy.tls_min_version.value() == TLSVersion::TLSv1_2 ?
                                                   CURL_SSLVERSION_TLSv1_2 : CURL_SSLVERSION_TLSv1_3); }
    if (policy.verify_peer)               { SetOpt(CURLOPT_SSL_VERIFYPEER, policy.verify_peer.value() ? 1L : 0L); }
    if (policy.verify_host)               { SetOpt(CURLOPT_SSL_VERIFYHOST, policy.verify_host.value() ? 2L : 0L); }
    if (policy.forbid_connection_reuse)   { SetOpt(CURLOPT_FORBID_REUSE, policy.forbid_connection_reuse.value() ? 1L : 0L); }
    if (policy.fail_on_http_error)        { SetOpt(CURLOPT_FAILONERROR, policy.fail_on_http_error.value() ? 1L : 0L); }

    // REQUEST DATA
    bool allow_body = true;
    switch (method) {
        case http::Method::POST:
            SetOpt(CURLOPT_POST, 1L);
            break;
        case http::Method::PUT:
            SetOpt(CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case http::Method::DELETE:
            SetOpt(CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case http::Method::PATCH:
            SetOpt(CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case http::Method::HEAD:
            SetOpt(CURLOPT_CUSTOMREQUEST, "HEAD");
            SetOpt(CURLOPT_NOBODY, 1L);
            allow_body = false;
            break;
        case http::Method::OPTIONS:
            SetOpt(CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        case http::Method::TRACE:
            SetOpt(CURLOPT_CUSTOMREQUEST, "TRACE");
            break;
        default:
            break; // Default to GET: no action
    }
    SetOpt(CURLOPT_URL, url.c_str());
    SetOpt(CURLOPT_HTTPHEADER, ToCurl<curl_slist>(header_list.Get()));
    if (allow_body && !body.empty()) {
        SetOpt(CURLOPT_POSTFIELDS, body.data());
        SetOpt(CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }
    SetOpt(CURLOPT_NOSIGNAL, 1L);

    // CALLBACKS
    SetOpt(CURLOPT_WRITEFUNCTION, RawHandlers::WriteCallback);
    SetOpt(CURLOPT_WRITEDATA, this);
    SetOpt(CURLOPT_HEADERFUNCTION, RawHandlers::HeaderCallback);
    SetOpt(CURLOPT_HEADERDATA, this);
    // SetOpt(CURLOPT_READFUNCTION, RawHandlers::ReadCallback); // streaming read
    // SetOpt(CURLOPT_READDATA, this);
    SetOpt(CURLOPT_XFERINFOFUNCTION, RawHandlers::XferInfoCallback);
    SetOpt(CURLOPT_XFERINFODATA, this);
    SetOpt(CURLOPT_NOPROGRESS, 0L);
    if (policy.enable_debugging && *policy.enable_debugging) {
        SetOpt(CURLOPT_DEBUGFUNCTION, RawHandlers::DebugCallback);
        SetOpt(CURLOPT_DEBUGDATA, this);
        SetOpt(CURLOPT_VERBOSE, 1L);
    }

    // SELF REFERENCE FOR CALLBACKS
    SetOpt(CURLOPT_PRIVATE, this);

    std::string add_handle_error_str = interface.AddHandle(handle.get());
    if (!add_handle_error_str.empty()) {
        Fail(add_handle_error_str, true);
    }
    unhooked = false;

    response.state = Response::State::IN_PROGRESS;
    response.availability = Response::Availability::STARTED;
}


void Attempt::Finalize(Interface& interface, const std::string& result_error_str) noexcept {
    std::string remove_handle_error_str = interface.RemoveHandle(handle.get());
    if (!remove_handle_error_str.empty()) { Fail(remove_handle_error_str); }
    else { unhooked = true; }
    if (!result_error_str.empty()) { Fail(result_error_str); }
    ExtractMetadata();
    handle.reset();
    response.availability = Response::Availability::FINAL;
    if (response.state != Response::State::FAILED) { response.state = Response::State::COMPLETED; }
}


void Attempt::ExtractMetadata() {
    CURL* curl_ptr = ToCurl<CURL>(handle.get());

    char* url{nullptr};
    CURLcode code = curl_easy_getinfo(curl_ptr, CURLINFO_EFFECTIVE_URL, &url);
    if (code == CURLE_OK && url) { response.effective_url = std::string(url); }

    long long_out{-1};
    code = curl_easy_getinfo(curl_ptr, CURLINFO_RESPONSE_CODE, &long_out);
    if (code == CURLE_OK) { response.status_code = static_cast<int64_t>(long_out); }

    code = curl_easy_getinfo(curl_ptr, CURLINFO_HTTP_VERSION, &long_out);
    if (code == CURLE_OK) { response.http_version = static_cast<int64_t>(long_out); }

    code = curl_easy_getinfo(curl_ptr, CURLINFO_REDIRECT_COUNT, &long_out);
    if (code == CURLE_OK) { response.redirect_count = static_cast<int64_t>(long_out); }

    curl_off_t off_out{-1};
    code = curl_easy_getinfo(curl_ptr, CURLINFO_SIZE_DOWNLOAD_T, &off_out);
    if (code == CURLE_OK) { response.total_wire_bytes_downloaded = static_cast<int64_t>(off_out); }

    code = curl_easy_getinfo(curl_ptr, CURLINFO_SIZE_UPLOAD_T, &off_out);
    if (code == CURLE_OK) { response.total_wire_bytes_uploaded = static_cast<int64_t>(off_out); }

    code = curl_easy_getinfo(curl_ptr, CURLINFO_TOTAL_TIME_T, &off_out);
    if (code == CURLE_OK) { response.total_time_us = static_cast<int64_t>(off_out); }
}


size_t Attempt::OnHeader(std::span<const std::byte> header_raw) {
    if (cancel_requested) { return 0; }

    auto it = std::ranges::find_if_not(header_raw.rbegin(), header_raw.rend(), [](auto b) {
        return b == std::byte{'\r'} || b == std::byte{'\n'};
    });
    const size_t header_size_bytes = static_cast<size_t>(std::ranges::distance(it, header_raw.rend()));
    auto header = header_raw.subspan(0, header_size_bytes);

    // Note: libcurl normalizes HTTP/2 and HTTP/3 headers into
    // HTTP/1.x-style lines before invoking the header callback.
    if (header.empty()) {
        header_open = false;
    } else if (!header_open) {
        if (header_size_bytes >= 5 && // still could be wrong, but not fully validating status header.
            (header[0] == std::byte{'H'} || header[0] == std::byte{'h'}) &&
            (header[1] == std::byte{'T'} || header[1] == std::byte{'t'}) &&
            (header[2] == std::byte{'T'} || header[2] == std::byte{'t'}) &&
            (header[3] == std::byte{'P'} || header[3] == std::byte{'p'}) &&
            (header[4] == std::byte{'/'}))
        {
            try {
                header_open = true;
                response.headers.emplace_back();
                response.headers.back().reserve(1024); // 1 KB
                response.abnormal_headers.emplace_back();
            } catch (...) {
                return 0;
            }
        } else {
            response.abnormal_headers.back().append_range(header);
            response.abnormal_headers.back().push_back(std::byte{'\n'});
        }
    } else {
        response.headers.back().append_range(header);
        response.headers.back().push_back(std::byte{'\n'});
    }

    return header_raw.size();
}


size_t Attempt::OnRead(std::span<const std::byte> data) {
    // streaming read
    return data.size();
}


size_t Attempt::OnWrite(std::span<const std::byte> data) {
    if (cancel_requested) { return 0; }
    try {
        response.body.append_range(data);
    } catch (...) { return 0; }
    return data.size();
}

int Attempt::OnXferInfo(int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) {
    response.current_leg_download_progress_bytes = dlnow;
    response.current_leg_download_total_estimate_bytes = dltotal;
    response.current_leg_upload_progress_bytes = ulnow;
    response.current_leg_upload_total_estimate_bytes = ultotal;
    if (response.availability == Response::Availability::STARTED && ultotal > 0 && ulnow >= ultotal) {
        response.availability = Response::Availability::UPLOAD_COMPLETE;
    }
    if ((response.availability == Response::Availability::STARTED ||
            response.availability == Response::Availability::UPLOAD_COMPLETE) &&
        dlnow >= dltotal) {
        response.availability = Response::Availability::DOWNLOAD_COMPLETE;
    }
    return cancel_requested ? 1 : 0;
}


/**
 * Interface
 */
void InterfaceMultiCleanup(void* ptr) { curl_multi_cleanup(ToCurl<CURLM>(ptr)); }


Interface::Interface(const ConnectionPolicy& policy)
: handle{static_cast<void*>(curl_multi_init()), &InterfaceMultiCleanup}
{
    if (!handle) { throw std::runtime_error("Failed to create curl::Interface; curl_multi_init failed."); }
    CURLM* curlm_ptr = ToCurl<CURLM>(handle.get());

    auto SetOpt = [h=curlm_ptr](auto opt, auto val) {
        const CURLMcode code = curl_multi_setopt(h, opt, val);
        if (code != CURLM_OK) {
            throw std::runtime_error(std::string("Failed to create curl::Interface; failed to apply policy: ") +
                                     curl_multi_strerror(code));
        }
    };

    if (policy.max_total_connections) {
        SetOpt(CURLMOPT_MAX_TOTAL_CONNECTIONS, static_cast<long>(policy.max_total_connections.value()));
    }
    if (policy.max_http2_streams_per_connection) {
        SetOpt(CURLMOPT_MAX_CONCURRENT_STREAMS, static_cast<long>(policy.max_http2_streams_per_connection.value()));
    }
}


std::string Interface::AddHandle(void* curl_easy_handle) {
    CURLM* curlm_ptr = ToCurl<CURLM>(handle.get());
    CURL* curl_ptr = ToCurl<CURL>(curl_easy_handle);
    CURLMcode code = curl_multi_add_handle(curlm_ptr, curl_ptr);
    if (code != CURLM_OK) {
        return std::string("Failed to register attempt: ") + curl_multi_strerror(code);
    }
    return "";
}


std::vector<Attempt*> Interface::ExecOnce() {
    CURLM* curlm_ptr = ToCurl<CURLM>(handle.get());

    int still_running{0};
    CURLMcode mcode = curl_multi_perform(curlm_ptr, &still_running);
    if (mcode != CURLM_OK) {
        throw std::runtime_error(std::string{"curl::Interface::ExecOnce curl_multi_perform failed with code "} +
                                 curl_multi_strerror(mcode));
    }

    std::vector<Attempt*> attempts{};

    CURLMsg* msg{nullptr};
    int msgs_left{0};
    while ((msg = curl_multi_info_read(curlm_ptr, &msgs_left))) {
        if (msg->msg != CURLMSG_DONE) { continue; }

        void* raw_ptr{nullptr};
        CURLcode ecode = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &raw_ptr);
        if (ecode != CURLE_OK) {
            throw std::runtime_error(std::string{"curl::Interface::ExecOnce failed to retrieve attempt: "} +
                                     curl_easy_strerror(ecode));
        }

        Attempt* attempt = static_cast<Attempt*>(raw_ptr);
        attempts.push_back(attempt);

        const auto result = msg->data.result;
        std::string error_str{};
        if (result != CURLE_OK) {
            error_str = std::string{"Attempt failed: "} + curl_easy_strerror(result);
        }

        attempt->Finalize(curlm_ptr, error_str);
    }

    return attempts;
}


std::string Interface::RemoveHandle(void* curl_easy_handle) {
    CURLM* curlm_ptr = ToCurl<CURLM>(handle.get());
    CURL* curl_ptr = ToCurl<CURL>(curl_easy_handle);
    CURLMcode code = curl_multi_remove_handle(curlm_ptr, curl_ptr);
    if (code != CURLM_OK) {
        return std::string("Failed to unregister attempt: ") + curl_multi_strerror(code);
    }
    return "";
}


Interface::Global::Global() { curl_global_init(CURL_GLOBAL_DEFAULT); }
Interface::Global::~Global() noexcept { curl_global_cleanup(); }
Interface::Global Interface::global{};


} // namespace jai::llm::curl
