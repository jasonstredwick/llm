#include "curl.hpp"


#include <curl/curl.h>
#if defined(_WIN32)
#   undef min
#   undef max
#   undef ERROR
#endif


#include <algorithm>
#include <ranges>
#include <span>


namespace jai::llm::curl {


HeaderList::HeaderList(const http::Headers& headers) : list{nullptr, &curl_slist_free_all} {
    for (const auto& header : headers.GetHeaders()) {
        auto* next = curl_slist_append(list.get(), header.c_str());
        if (!next) { throw std::runtime_error("Failed to create HeaderList."); }
        list.reset(next);
    }
}


Attempt::Attempt(CURLM* multi,
                 const AttemptPolicy& policy,
                 http::Method method,
                 const std::string& url,
                 const HeaderList& header_list,
                 const std::vector<std::byte>& body)
: handle{curl_easy_init(), &curl_easy_cleanup}
{
    if (!handle) { Fail("Failed to construct Attempt.", true); }

    try {
        const size_t buffer_size = 131072; // 128 KB
        response.body.reserve(buffer_size);
    } catch(...) {
        Fail(std::string("Failed to create Attempt; failed to allocate response buffers."), true);
    }

    auto SetOpt = [h=handle.get()](auto opt, auto val) {
        const CURLcode code = curl_easy_setopt(h, opt, val);
        if (code != CURLE_OK) {
            Fail(std::string("Failed to create Attempt; failed to apply policy: ") + curl_easy_strerror(code), true);
        }
    };

    // POLICY
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
    SetOpt(CURLOPT_HTTPHEADER, header_list.Get());
    if (allow_body && !body.empty()) {
        SetOpt(CURLOPT_POSTFIELDS, body.data());
        SetOpt(CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    }
    SetOpt(CURLOPT_NOSIGNAL, 1L);

    // CALLBACKS
    SetOpt(CURLOPT_WRITEFUNCTION, WriteCallback);
    SetOpt(CURLOPT_WRITEDATA, this);
    SetOpt(CURLOPT_HEADERFUNCTION, HeaderCallback);
    SetOpt(CURLOPT_HEADERDATA, this);
    // SetOpt(CURLOPT_READFUNCTION, ReadCallback); // streaming read
    // SetOpt(CURLOPT_READDATA, this);
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

    // SELF REFERENCE FOR CALLBACKS
    SetOpt(CURLOPT_PRIVATE, this);

    CURLMcode code = curl_multi_add_handle(multi, handle.get());
    if (code != CURLM_OK) {
        Fail(std::string("Failed to create Attempt; parent registration failed: ") + curl_multi_strerror(code), true);
    }
    unhooked = false;

    response.state = Response::State::IN_PROGRESS;
    response.availability = Response::Availability::STARTED;
}


void Attempt::Finalize(CURLM* multi, CURLcode result) noexcept {
    CURLMcode unhook_code = curl_multi_remove_handle(multi, handle.get());
    if (unhook_code == CURLM_OK) { unhooked = true; }
    if (result != CURLE_OK) { Fail(std::string("Attempt failed: ") + curl_easy_strerror(result)); }
    ExtractMetadata();
    handle.reset();
    response.availability = Response::Availability::FINAL;
    if (response.state != Response::State::FAILED) { response.state = Response::State::COMPLETED; }
}


void Attempt::ExtractMetadata() {
    char* url{nullptr};
    CURLcode code = curl_easy_getinfo(handle.get(), CURLINFO_EFFECTIVE_URL, &url);
    if (code == CURLE_OK && url) { response.effective_url = std::string(url); }

    long long_out{-1};
    code = curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &long_out);
    if (code == CURLE_OK) { response.status_code = static_cast<int64_t>(long_out); }

    code = curl_easy_getinfo(handle.get(), CURLINFO_HTTP_VERSION, &long_out);
    if (code == CURLE_OK) { response.http_version = static_cast<int64_t>(long_out); }

    code = curl_easy_getinfo(handle.get(), CURLINFO_REDIRECT_COUNT, &long_out);
    if (code == CURLE_OK) { response.redirect_count = static_cast<int64_t>(long_out); }

    curl_off_t off_out{-1};
    code = curl_easy_getinfo(handle.get(), CURLINFO_SIZE_DOWNLOAD_T, &off_out);
    if (code == CURLE_OK) { response.total_wire_bytes_downloaded = static_cast<int64_t>(off_out); }

    code = curl_easy_getinfo(handle.get(), CURLINFO_SIZE_UPLOAD_T, &off_out);
    if (code == CURLE_OK) { response.total_wire_bytes_uploaded = static_cast<int64_t>(off_out); }

    code = curl_easy_getinfo(handle.get(), CURLINFO_TOTAL_TIME_T, &off_out);
    if (code == CURLE_OK) { response.total_time_us = static_cast<int64_t>(off_out); }
}


int Attempt::OnDebug(CURL* debug_handle, curl_infotype type, std::byte *data, size_t size) {
    // See https://curl.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
    return 0;
}


size_t Attempt::OnHeader(std::byte* ptr, size_t size, size_t nmemb) {
    if (cancel_requested) { return 0; }

    const size_t raw_size_in_bytes = size * nmemb;
    std::span<const std::byte> header_raw{ptr, raw_size_in_bytes};
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

    return raw_size_in_bytes;
}


size_t Attempt::OnRead(std::byte* ptr, size_t size, size_t nmemb) {
    // streaming read
    return size * nmemb;
}

size_t Attempt::OnWrite(std::byte* ptr, size_t size, size_t nmemb) {
    if (cancel_requested) { return 0; }
    const size_t size_bytes = size * nmemb;
    try {
        response.body.append_range(std::span{ptr, size_bytes});
    } catch (...) { return 0; }
    return size_bytes;
}

int Attempt::OnXferInfo(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    response.current_leg_download_progress_bytes = static_cast<int64_t>(dlnow);
    response.current_leg_download_total_estimate_bytes = static_cast<int64_t>(dltotal);
    response.current_leg_upload_progress_bytes = static_cast<int64_t>(ulnow);
    response.current_leg_upload_total_estimate_bytes = static_cast<int64_t>(ultotal);
    if (response.availability == Attempt::Availability::STARTED && ultotal > 0 && ulnow >= ultotal) {
        response.availability = Attempt::Availability::UPLOAD_COMPLETE;
    }
    if ((response.availability == Attempt::Availability::STARTED ||
            response.availability == Attempt::Availability::UPLOAD_COMPLETE) &&
        dlnow >= dltotal) {
        response.availability = Attempt::Availability::DOWNLOAD_COMPLETE;
    }
    return cancel_requested ? 1 : 0;
}


Interface::Interface(const ConnectionPolicy& policy_) : handle{curl_multi_init(), &curl_multi_cleanup} {
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


std::vector<Attempt*> Interface::ExecOnce() {
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
            throw std::runtime_error("curl::Interface::ExecOnce failed to retrieve attempt: " + std::to_string(code));
        }

        Attempt* attempt = static_cast<Attempt*>(raw_ptr);
        attempts.push_back(attempt);
        attempt->Finalize(handle.get(), msg->data.result);
    }

    return attempts;
}


Interface::Global::Global() { curl_global_init(CURL_GLOBAL_DEFAULT); }
Interface::Global::~Global() noexcept { curl_global_cleanup(); }


} // namespace jai::llm::curl
