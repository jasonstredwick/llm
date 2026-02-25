/***
 * Policy cascade — Merge and Resolve implementations.
 *
 * Pattern: for each field, use the most specific non-nullopt value.
 * The "most specific" is the last argument (call_site > client > orchestrator).
 *
 * @author jason.stredwick@gmail.com
 */

#include "../interface/clients/policy.hpp"


namespace jai::llm {


//----- Helper: pick the most specific value -----

template <typename T>
static const std::optional<T>& Pick(const std::optional<T>& base,
                                     const std::optional<T>& override) {
    return override.has_value() ? override : base;
}

template <typename T>
static const std::optional<T>& Pick(const std::optional<T>& base,
                                     const std::optional<T>& mid,
                                     const std::optional<T>& top) {
    return top.has_value() ? top : (mid.has_value() ? mid : base);
}


//----- AttemptPolicy Merge (two-level: orchestrator + client) -----

AttemptPolicy Merge(const AttemptPolicy& orchestrator,
                    const AttemptPolicy& client) {
    return AttemptPolicy{
        .connect_timeout         = Pick(orchestrator.connect_timeout,         client.connect_timeout),
        .total_timeout           = Pick(orchestrator.total_timeout,           client.total_timeout),
        .low_speed_time          = Pick(orchestrator.low_speed_time,          client.low_speed_time),
        .low_speed_bytes_per_sec = Pick(orchestrator.low_speed_bytes_per_sec, client.low_speed_bytes_per_sec),
        .http_version            = Pick(orchestrator.http_version,            client.http_version),
        .follow_redirects        = Pick(orchestrator.follow_redirects,        client.follow_redirects),
        .max_redirects           = Pick(orchestrator.max_redirects,           client.max_redirects),
        .ca_bundle_path          = Pick(orchestrator.ca_bundle_path,          client.ca_bundle_path),
        .client_cert_path        = Pick(orchestrator.client_cert_path,        client.client_cert_path),
        .client_key_path         = Pick(orchestrator.client_key_path,         client.client_key_path),
        .tls_min_version         = Pick(orchestrator.tls_min_version,         client.tls_min_version),
        .verify_peer             = Pick(orchestrator.verify_peer,             client.verify_peer),
        .verify_host             = Pick(orchestrator.verify_host,             client.verify_host),
        .forbid_connection_reuse = Pick(orchestrator.forbid_connection_reuse, client.forbid_connection_reuse),
        .fail_on_http_error      = Pick(orchestrator.fail_on_http_error,      client.fail_on_http_error),
        .enable_debugging        = Pick(orchestrator.enable_debugging,        client.enable_debugging),
    };
}


//----- AttemptPolicy Merge (three-level: orchestrator + client + call_site) -----

AttemptPolicy Merge(const AttemptPolicy& orchestrator,
                    const AttemptPolicy& client,
                    const AttemptPolicy& call_site) {
    return AttemptPolicy{
        .connect_timeout         = Pick(orchestrator.connect_timeout,         client.connect_timeout,         call_site.connect_timeout),
        .total_timeout           = Pick(orchestrator.total_timeout,           client.total_timeout,           call_site.total_timeout),
        .low_speed_time          = Pick(orchestrator.low_speed_time,          client.low_speed_time,          call_site.low_speed_time),
        .low_speed_bytes_per_sec = Pick(orchestrator.low_speed_bytes_per_sec, client.low_speed_bytes_per_sec, call_site.low_speed_bytes_per_sec),
        .http_version            = Pick(orchestrator.http_version,            client.http_version,            call_site.http_version),
        .follow_redirects        = Pick(orchestrator.follow_redirects,        client.follow_redirects,        call_site.follow_redirects),
        .max_redirects           = Pick(orchestrator.max_redirects,           client.max_redirects,           call_site.max_redirects),
        .ca_bundle_path          = Pick(orchestrator.ca_bundle_path,          client.ca_bundle_path,          call_site.ca_bundle_path),
        .client_cert_path        = Pick(orchestrator.client_cert_path,        client.client_cert_path,        call_site.client_cert_path),
        .client_key_path         = Pick(orchestrator.client_key_path,         client.client_key_path,         call_site.client_key_path),
        .tls_min_version         = Pick(orchestrator.tls_min_version,         client.tls_min_version,         call_site.tls_min_version),
        .verify_peer             = Pick(orchestrator.verify_peer,             client.verify_peer,             call_site.verify_peer),
        .verify_host             = Pick(orchestrator.verify_host,             client.verify_host,             call_site.verify_host),
        .forbid_connection_reuse = Pick(orchestrator.forbid_connection_reuse, client.forbid_connection_reuse, call_site.forbid_connection_reuse),
        .fail_on_http_error      = Pick(orchestrator.fail_on_http_error,      client.fail_on_http_error,      call_site.fail_on_http_error),
        .enable_debugging        = Pick(orchestrator.enable_debugging,        client.enable_debugging,        call_site.enable_debugging),
    };
}


//----- RetryPolicy Resolve -----

template <typename T>
static T ResolveField(const std::optional<T>& orchestrator,
                      const std::optional<T>& client,
                      const T& fallback) {
    if (client.has_value()) { return *client; }
    if (orchestrator.has_value()) { return *orchestrator; }
    return fallback;
}


ResolvedRetryPolicy Resolve(const RetryPolicy& orchestrator,
                            const RetryPolicy& client) {
    ResolvedRetryPolicy defaults{};
    return ResolvedRetryPolicy{
        .retryable_status_codes     = ResolveField(orchestrator.retryable_status_codes,     client.retryable_status_codes,     defaults.retryable_status_codes),
        .max_retries                = ResolveField(orchestrator.max_retries,                client.max_retries,                defaults.max_retries),
        .retry_on_deserialize_failure = ResolveField(orchestrator.retry_on_deserialize_failure, client.retry_on_deserialize_failure, defaults.retry_on_deserialize_failure),
    };
}


//----- RateLimitPolicy Resolve -----

ResolvedRateLimitPolicy Resolve(const RateLimitPolicy& orchestrator,
                                const RateLimitPolicy& client) {
    ResolvedRateLimitPolicy defaults{};
    return ResolvedRateLimitPolicy{
        .backoff_floor               = ResolveField(orchestrator.backoff_floor,               client.backoff_floor,               defaults.backoff_floor),
        .initial_max_concurrent      = ResolveField(orchestrator.initial_max_concurrent,      client.initial_max_concurrent,      defaults.initial_max_concurrent),
        .min_remaining_before_backoff = ResolveField(orchestrator.min_remaining_before_backoff, client.min_remaining_before_backoff, defaults.min_remaining_before_backoff),
        .use_provider_headers        = ResolveField(orchestrator.use_provider_headers,        client.use_provider_headers,        defaults.use_provider_headers),
    };
}


}
