#include <chrono>
#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../interface/clients/policy.hpp"

using namespace jai::llm;
using namespace std::chrono_literals;


/***
 * AttemptPolicy Merge Tests
 */

void test_merge_attempt_empty() {
    std::println("Testing AttemptPolicy Merge: all empty...");

    AttemptPolicy a{};
    AttemptPolicy b{};
    auto result = Merge(a, b);

    REQUIRE(!result.connect_timeout.has_value());
    REQUIRE(!result.total_timeout.has_value());
    REQUIRE(!result.http_version.has_value());
    REQUIRE(!result.verify_peer.has_value());
    REQUIRE(!result.enable_debugging.has_value());

    std::println("  [SUCCESS]");
}


void test_merge_attempt_base_only() {
    std::println("Testing AttemptPolicy Merge: base only...");

    AttemptPolicy base{};
    base.connect_timeout = 5000ms;
    base.verify_peer = true;

    AttemptPolicy override{};
    auto result = Merge(base, override);

    REQUIRE(result.connect_timeout.has_value());
    REQUIRE_EQ(*result.connect_timeout, 5000ms);
    REQUIRE(result.verify_peer.has_value());
    REQUIRE_EQ(*result.verify_peer, true);
    REQUIRE(!result.total_timeout.has_value());

    std::println("  [SUCCESS]");
}


void test_merge_attempt_override_wins() {
    std::println("Testing AttemptPolicy Merge: override wins...");

    AttemptPolicy base{};
    base.connect_timeout = 5000ms;
    base.verify_peer = true;

    AttemptPolicy override{};
    override.connect_timeout = 10000ms;
    override.verify_peer = false;

    auto result = Merge(base, override);

    REQUIRE_EQ(*result.connect_timeout, 10000ms);
    REQUIRE_EQ(*result.verify_peer, false);

    std::println("  [SUCCESS]");
}


void test_merge_attempt_three_level() {
    std::println("Testing AttemptPolicy Merge: three-level cascade...");

    AttemptPolicy orchestrator{};
    orchestrator.connect_timeout = 5000ms;
    orchestrator.total_timeout = 30000ms;
    orchestrator.verify_peer = true;
    orchestrator.enable_debugging = false;

    AttemptPolicy client{};
    client.total_timeout = 60000ms;  // override orchestrator
    client.http_version = HTTPVersion::HTTP2;

    AttemptPolicy call_site{};
    call_site.enable_debugging = true;  // override orchestrator

    auto result = Merge(orchestrator, client, call_site);

    // orchestrator value (not overridden)
    REQUIRE_EQ(*result.connect_timeout, 5000ms);
    // client overrides orchestrator
    REQUIRE_EQ(*result.total_timeout, 60000ms);
    // client value (not overridden by call_site)
    REQUIRE_EQ(*result.http_version, HTTPVersion::HTTP2);
    // orchestrator value (verify_peer not overridden)
    REQUIRE_EQ(*result.verify_peer, true);
    // call_site overrides orchestrator
    REQUIRE_EQ(*result.enable_debugging, true);
    // never set at any level
    REQUIRE(!result.ca_bundle_path.has_value());

    std::println("  [SUCCESS]");
}


void test_merge_attempt_call_site_over_client_over_orchestrator() {
    std::println("Testing AttemptPolicy Merge: all three set same field...");

    AttemptPolicy orchestrator{};
    orchestrator.connect_timeout = 1000ms;

    AttemptPolicy client{};
    client.connect_timeout = 2000ms;

    AttemptPolicy call_site{};
    call_site.connect_timeout = 3000ms;

    auto result = Merge(orchestrator, client, call_site);

    // Most specific (call_site) wins
    REQUIRE_EQ(*result.connect_timeout, 3000ms);

    std::println("  [SUCCESS]");
}


void test_merge_attempt_two_level_matches_three_level_empty_call_site() {
    std::println("Testing AttemptPolicy Merge: two-level == three-level with empty call_site...");

    AttemptPolicy orchestrator{};
    orchestrator.connect_timeout = 5000ms;
    orchestrator.verify_peer = true;

    AttemptPolicy client{};
    client.total_timeout = 60000ms;

    auto two_level = Merge(orchestrator, client);

    AttemptPolicy empty_call_site{};
    auto three_level = Merge(orchestrator, client, empty_call_site);

    REQUIRE_EQ(two_level.connect_timeout, three_level.connect_timeout);
    REQUIRE_EQ(two_level.total_timeout, three_level.total_timeout);
    REQUIRE_EQ(two_level.verify_peer, three_level.verify_peer);
    REQUIRE_EQ(two_level.enable_debugging, three_level.enable_debugging);

    std::println("  [SUCCESS]");
}


/***
 * RetryPolicy Resolve Tests
 */

void test_resolve_retry_defaults() {
    std::println("Testing RetryPolicy Resolve: all defaults...");

    RetryPolicy orchestrator{};
    RetryPolicy client{};
    auto result = Resolve(orchestrator, client);

    ResolvedRetryPolicy defaults{};
    REQUIRE_EQ(result.max_retries, defaults.max_retries);
    REQUIRE_EQ(result.retry_on_deserialize_failure, defaults.retry_on_deserialize_failure);
    REQUIRE_EQ(result.retryable_status_codes.size(), defaults.retryable_status_codes.size());

    std::println("  [SUCCESS]");
}


void test_resolve_retry_orchestrator_override() {
    std::println("Testing RetryPolicy Resolve: orchestrator override...");

    RetryPolicy orchestrator{};
    orchestrator.max_retries = 5;
    orchestrator.retry_on_deserialize_failure = true;

    RetryPolicy client{};
    auto result = Resolve(orchestrator, client);

    REQUIRE_EQ(result.max_retries, 5u);
    REQUIRE_EQ(result.retry_on_deserialize_failure, true);

    std::println("  [SUCCESS]");
}


void test_resolve_retry_client_wins() {
    std::println("Testing RetryPolicy Resolve: client overrides orchestrator...");

    RetryPolicy orchestrator{};
    orchestrator.max_retries = 5;
    orchestrator.retryable_status_codes = std::vector<int64_t>{500, 502};

    RetryPolicy client{};
    client.max_retries = 10;
    client.retryable_status_codes = std::vector<int64_t>{500, 502, 503, 504, 599};

    auto result = Resolve(orchestrator, client);

    REQUIRE_EQ(result.max_retries, 10u);
    REQUIRE_EQ(result.retryable_status_codes.size(), 5u);

    std::println("  [SUCCESS]");
}


void test_resolve_retry_client_partial() {
    std::println("Testing RetryPolicy Resolve: client overrides some, orchestrator provides rest...");

    RetryPolicy orchestrator{};
    orchestrator.max_retries = 5;
    orchestrator.retry_on_deserialize_failure = true;

    RetryPolicy client{};
    client.max_retries = 2;  // override
    // retry_on_deserialize_failure not set — falls through to orchestrator

    auto result = Resolve(orchestrator, client);

    REQUIRE_EQ(result.max_retries, 2u);
    REQUIRE_EQ(result.retry_on_deserialize_failure, true);  // from orchestrator

    std::println("  [SUCCESS]");
}


/***
 * RateLimitPolicy Resolve Tests
 */

void test_resolve_ratelimit_defaults() {
    std::println("Testing RateLimitPolicy Resolve: all defaults...");

    RateLimitPolicy orchestrator{};
    RateLimitPolicy client{};
    auto result = Resolve(orchestrator, client);

    ResolvedRateLimitPolicy defaults{};
    REQUIRE_EQ(result.backoff_floor, defaults.backoff_floor);
    REQUIRE_EQ(result.initial_max_concurrent, defaults.initial_max_concurrent);
    REQUIRE_EQ(result.min_remaining_before_backoff, defaults.min_remaining_before_backoff);
    REQUIRE_EQ(result.use_provider_headers, defaults.use_provider_headers);

    std::println("  [SUCCESS]");
}


void test_resolve_ratelimit_client_wins() {
    std::println("Testing RateLimitPolicy Resolve: client overrides...");

    RateLimitPolicy orchestrator{};
    orchestrator.initial_max_concurrent = 16;
    orchestrator.use_provider_headers = true;

    RateLimitPolicy client{};
    client.initial_max_concurrent = 4;
    client.backoff_floor = 200ms;

    auto result = Resolve(orchestrator, client);

    REQUIRE_EQ(result.initial_max_concurrent, 4u);
    REQUIRE_EQ(result.backoff_floor, 200ms);
    REQUIRE_EQ(result.use_provider_headers, true);  // from orchestrator

    std::println("  [SUCCESS]");
}


void test_resolve_ratelimit_orchestrator_only() {
    std::println("Testing RateLimitPolicy Resolve: orchestrator provides all...");

    RateLimitPolicy orchestrator{};
    orchestrator.backoff_floor = 500ms;
    orchestrator.initial_max_concurrent = 32;
    orchestrator.min_remaining_before_backoff = 5;
    orchestrator.use_provider_headers = false;

    RateLimitPolicy client{};
    auto result = Resolve(orchestrator, client);

    REQUIRE_EQ(result.backoff_floor, 500ms);
    REQUIRE_EQ(result.initial_max_concurrent, 32u);
    REQUIRE_EQ(result.min_remaining_before_backoff, 5u);
    REQUIRE_EQ(result.use_provider_headers, false);

    std::println("  [SUCCESS]");
}


/***
 * Main
 */

int main() {
    int failed = 0;

    auto run = [&](auto fn) {
        try { fn(); }
        catch (const std::exception& e) {
            std::println("  [FAILED] {}", e.what());
            ++failed;
        }
    };

    std::println("===== AttemptPolicy Merge Tests =====");
    run(test_merge_attempt_empty);
    run(test_merge_attempt_base_only);
    run(test_merge_attempt_override_wins);
    run(test_merge_attempt_three_level);
    run(test_merge_attempt_call_site_over_client_over_orchestrator);
    run(test_merge_attempt_two_level_matches_three_level_empty_call_site);

    std::println("\n===== RetryPolicy Resolve Tests =====");
    run(test_resolve_retry_defaults);
    run(test_resolve_retry_orchestrator_override);
    run(test_resolve_retry_client_wins);
    run(test_resolve_retry_client_partial);

    std::println("\n===== RateLimitPolicy Resolve Tests =====");
    run(test_resolve_ratelimit_defaults);
    run(test_resolve_ratelimit_client_wins);
    run(test_resolve_ratelimit_orchestrator_only);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
