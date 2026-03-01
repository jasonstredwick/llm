/***
 * Policy configuration for the jai::llm library.
 *
 * Policies cascade: orchestrator (global defaults) → client (overrides) → call site (per-call).
 * All fields are std::optional. Unset fields fall through to the next level up.
 * The orchestrator resolves unset fields to sane hardcoded defaults.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>


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


// Per-request curl settings. Applied when creating each curl::Attempt.
// Cascade: orchestrator default → client override → per-call override.
struct AttemptPolicy {
    // Timeouts
    std::optional<std::chrono::milliseconds> connect_timeout{};
    std::optional<std::chrono::milliseconds> total_timeout{};
    std::optional<std::chrono::seconds> low_speed_time{};
    std::optional<size_t> low_speed_bytes_per_sec{};

    // Protocol control
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

    friend auto operator<=>(const AttemptPolicy&, const AttemptPolicy&) = default;
    friend bool operator==(const AttemptPolicy&, const AttemptPolicy&) = default;
};


// Curl multi handle settings. Orchestrator only — not overridable per-client or per-call.
struct ConnectionPolicy {
    std::optional<size_t> max_total_connections{};
    std::optional<size_t> max_http2_streams_per_connection{};

    friend auto operator<=>(const ConnectionPolicy&, const ConnectionPolicy&) = default;
    friend bool operator==(const ConnectionPolicy&, const ConnectionPolicy&) = default;
};


// Governs retry behavior on transport/deserialization failures.
// Cascade: orchestrator default → client override.
struct RetryPolicy {
    std::optional<std::vector<int64_t>> retryable_status_codes{};
    std::optional<size_t> max_retries{};
    std::optional<bool> retry_on_deserialize_failure{};

    friend auto operator<=>(const RetryPolicy&, const RetryPolicy&) = default;
    friend bool operator==(const RetryPolicy&, const RetryPolicy&) = default;
};


// Configuration knobs for the adaptive rate limiting algorithm.
// The actual rate limit state (watermarks, send rate) is dynamic and
// maintained internally by the orchestrator per-queue at runtime.
// Cascade: orchestrator default → client override.
// Members ordered for packing: 8-byte, 1-byte.
struct RateLimitPolicy {
    std::optional<std::chrono::milliseconds> backoff_floor{};
    std::optional<size_t> initial_max_concurrent{};
    std::optional<size_t> min_remaining_before_backoff{};
    std::optional<bool> use_provider_headers{};

    friend auto operator<=>(const RateLimitPolicy&, const RateLimitPolicy&) = default;
    friend bool operator==(const RateLimitPolicy&, const RateLimitPolicy&) = default;
};


// Client-level overrides. All fields cascade from orchestrator defaults.
// A client with an empty ClientPolicy inherits all orchestrator defaults.
struct ClientPolicy {
    AttemptPolicy attempt_policy{};
    RetryPolicy retry_policy{};
    RateLimitPolicy rate_limit_policy{};

    friend auto operator<=>(const ClientPolicy&, const ClientPolicy&) = default;
    friend bool operator==(const ClientPolicy&, const ClientPolicy&) = default;
};


// Orchestrator-level configuration. Provides global defaults for all clients
// and owns the connection policy (not overridable per-client).
struct OrchestratorPolicy {
    ConnectionPolicy connection_policy{};
    AttemptPolicy attempt_policy{};
    RetryPolicy retry_policy{};
    RateLimitPolicy rate_limit_policy{};

    friend auto operator<=>(const OrchestratorPolicy&, const OrchestratorPolicy&) = default;
    friend bool operator==(const OrchestratorPolicy&, const OrchestratorPolicy&) = default;
};


}
