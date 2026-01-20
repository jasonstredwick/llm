/***
 * Primary interface for LLM usage management.
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include <chrono>
#include <filesystem>
#include <optional>


namespace jai::llm {


enum class HTTPVersion {
    HTTP1_1,
    HTTP2//,
    //HTTP3
};


enum class TLSVersion {
    TLSv1_2,
    TLSv1_3
};



struct AttemptPolicy {
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
    std::optional<std::filesystem::path> ca_bundle_path{};
    std::optional<std::filesystem::path> client_cert_path{};
    std::optional<std::filesystem::path> client_key_path{};
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


struct ConnectionPolicy {
    std::optional<size_t> max_total_connections{};
    std::optional<size_t> max_http2_streams_per_connection{};
};


struct ClientPolicy {
    AttemptPolicy attempt_policy{};
    ConnectionPolicy connection_policy{};
};


}