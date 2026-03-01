/***
 * Policy cascade resolution — internal header.
 *
 * Resolved policy types (no optionals) and Merge/Resolve function
 * declarations. Used by the orchestrator to collapse the
 * orchestrator → client → call-site cascade into concrete values.
 *
 * NOT a public header. Must not be included from interface/.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "../interface/core/policy.hpp"


namespace jai::llm {


//----- Resolved policies -----
// Fully resolved forms with no optionals. Every field has a concrete value.
// Produced by merging the cascade and filling remaining gaps with hardcoded defaults.
// These govern the library's own logic (retry, rate limiting) where every field
// needs a concrete value to operate.
//
// AttemptPolicy and ConnectionPolicy do NOT have resolved forms because unset
// fields defer to libcurl's internal defaults — "not set" is a valid final state.
// Members ordered for packing: 8-byte, 4-byte, 1-byte.

// Hardcoded defaults for retry behavior. Used to fill unset fields after cascade resolution.
struct ResolvedRetryPolicy {
    std::vector<int64_t> retryable_status_codes{500, 502, 503, 504};  // 429 handled separately (rate-limit path)
    size_t max_retries{3};
    bool retry_on_deserialize_failure{false};
};


// Hardcoded defaults for rate limiting. Used to fill unset fields after cascade resolution.
struct ResolvedRateLimitPolicy {
    std::chrono::milliseconds backoff_floor{100};
    size_t initial_max_concurrent{8};
    size_t min_remaining_before_backoff{2};
    bool use_provider_headers{true};
};


//----- Merge functions -----
// Merge the cascade: for each field, use the most specific non-nullopt value.
// AttemptPolicy and ConnectionPolicy remain in optional form (unset = curl default).
// RetryPolicy and RateLimitPolicy are fully resolved (unset = hardcoded default).

AttemptPolicy Merge(const AttemptPolicy& orchestrator,
                    const AttemptPolicy& client,
                    const AttemptPolicy& call_site);

AttemptPolicy Merge(const AttemptPolicy& orchestrator,
                    const AttemptPolicy& client);

ResolvedRetryPolicy Resolve(const RetryPolicy& orchestrator,
                            const RetryPolicy& client);

ResolvedRateLimitPolicy Resolve(const RateLimitPolicy& orchestrator,
                                const RateLimitPolicy& client);


}
