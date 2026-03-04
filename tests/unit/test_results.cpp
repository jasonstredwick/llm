/***
 * Async unit tests — ResultSync signaling and Result type mechanics.
 *
 * ResultSync is a standalone synchronization primitive that can be tested
 * without the orchestrator.
 *
 * AsyncResult<Endpoint> tests require a live Orchestrator + curl::Response, so
 * they belong in the integration test suite.
 */

#include <atomic>
#include <print>
#include <string>
#include <thread>

#include "test_assert.hpp"
#include "../../src/results.hpp"


using namespace jai::llm;
using namespace std::chrono_literals;


// ----- Test endpoint tag -----
struct TestEndpoint {
    using Request_t = int;    // unused in unit tests
    using Response_t = int;
};


/***
 * ResultSync Tests
 */

void test_sync_initial_state() {
    std::println("Testing ResultSync: initial state...");

    ResultSync sync{};

    REQUIRE_EQ(sync.ready, false);
    REQUIRE_EQ(sync.succeeded, false);
    REQUIRE(sync.error_msg.empty());
    std::println("  [SUCCESS]");
}


void test_sync_signal_success() {
    std::println("Testing ResultSync: signal success...");

    ResultSync sync{};
    sync.Signal(true);

    REQUIRE_EQ(sync.ready, true);
    REQUIRE_EQ(sync.succeeded, true);
    REQUIRE(sync.error_msg.empty());
    std::println("  [SUCCESS]");
}


void test_sync_signal_failure() {
    std::println("Testing ResultSync: signal failure with error...");

    ResultSync sync{};
    sync.Signal(false, "HTTP 500 after 3 retries");

    REQUIRE_EQ(sync.ready, true);
    REQUIRE_EQ(sync.succeeded, false);
    REQUIRE_EQ(sync.error_msg, std::string{"HTTP 500 after 3 retries"});
    std::println("  [SUCCESS]");
}


void test_sync_reset_clears_state() {
    std::println("Testing ResultSync: reset clears state...");

    ResultSync sync{};
    sync.Signal(true);
    REQUIRE_EQ(sync.ready, true);

    sync.Reset();

    REQUIRE_EQ(sync.ready, false);
    REQUIRE_EQ(sync.succeeded, false);
    REQUIRE(sync.error_msg.empty());
    std::println("  [SUCCESS]");
}


void test_sync_signal_after_reset() {
    std::println("Testing ResultSync: signal after reset (retry cycle)...");

    ResultSync sync{};

    // First signal.
    sync.Signal(false, "transient error");
    REQUIRE_EQ(sync.ready, true);
    REQUIRE_EQ(sync.succeeded, false);

    // Reset for retry.
    sync.Reset();
    REQUIRE_EQ(sync.ready, false);

    // Second signal (success this time).
    sync.Signal(true);
    REQUIRE_EQ(sync.ready, true);
    REQUIRE_EQ(sync.succeeded, true);
    REQUIRE(sync.error_msg.empty());
    std::println("  [SUCCESS]");
}


void test_sync_threaded_wait() {
    std::println("Testing ResultSync: threaded wait/signal...");

    auto sync = std::make_shared<ResultSync>();
    std::atomic<bool> thread_done{false};

    // Waiter thread.
    std::thread waiter([&]() {
        std::unique_lock lock(sync->mtx);
        sync->cv.wait(lock, [&] { return sync->ready; });
        thread_done.store(true);
    });

    // Give the waiter time to block.
    std::this_thread::sleep_for(10ms);
    REQUIRE_EQ(thread_done.load(), false);

    // Signal from "orchestrator" thread.
    sync->Signal(true);

    waiter.join();
    REQUIRE_EQ(thread_done.load(), true);
    REQUIRE_EQ(sync->succeeded, true);
    std::println("  [SUCCESS]");
}


void test_sync_threaded_wait_failure() {
    std::println("Testing ResultSync: threaded wait with failure signal...");

    auto sync = std::make_shared<ResultSync>();
    std::string captured_error{};

    std::thread waiter([&]() {
        std::unique_lock lock(sync->mtx);
        sync->cv.wait(lock, [&] { return sync->ready; });
        captured_error = sync->error_msg;
    });

    std::this_thread::sleep_for(10ms);
    sync->Signal(false, "Transport error: connection refused");

    waiter.join();
    REQUIRE_EQ(captured_error, std::string{"Transport error: connection refused"});
    std::println("  [SUCCESS]");
}


void test_sync_threaded_retry_cycle() {
    std::println("Testing ResultSync: threaded retry cycle...");

    auto sync = std::make_shared<ResultSync>();
    int signal_count = 0;

    std::thread waiter([&]() {
        // Wait for first signal (failure).
        {
            std::unique_lock lock(sync->mtx);
            sync->cv.wait(lock, [&] { return sync->ready; });
        }
        ++signal_count;

        // Simulate reset for retry.
        sync->Reset();

        // Wait for second signal (success).
        {
            std::unique_lock lock(sync->mtx);
            sync->cv.wait(lock, [&] { return sync->ready; });
        }
        ++signal_count;
    });

    // First signal: failure.
    std::this_thread::sleep_for(10ms);
    sync->Signal(false, "HTTP 503");

    // Wait for waiter to process and reset.
    std::this_thread::sleep_for(20ms);

    // Second signal: success.
    sync->Signal(true);

    waiter.join();
    REQUIRE_EQ(signal_count, 2);
    REQUIRE_EQ(sync->succeeded, true);
    std::println("  [SUCCESS]");
}


/***
 * TokenUsage Tests
 */

void test_token_usage_default() {
    std::println("Testing TokenUsage: default construction...");

    TokenUsage usage{};
    REQUIRE(!usage.input_tokens.has_value());
    REQUIRE(!usage.output_tokens.has_value());
    REQUIRE(!usage.total_tokens.has_value());
    REQUIRE(!usage.cache_creation_tokens.has_value());
    REQUIRE(!usage.cache_read_tokens.has_value());
    REQUIRE(!usage.reasoning_tokens.has_value());
    REQUIRE(!usage.tool_use_tokens.has_value());
    std::println("  [SUCCESS]");
}


void test_attempt_metadata_default() {
    std::println("Testing AttemptMetadata: default construction...");

    AttemptMetadata am{};
    REQUIRE_EQ(am.status_code, static_cast<int64_t>(-1));
    REQUIRE_EQ(am.duration_us, static_cast<int64_t>(-1));
    REQUIRE_EQ(am.outcome, AttemptOutcome::SUCCESS);
    REQUIRE(am.error.empty());
    REQUIRE(!am.usage.has_value());
    REQUIRE(!am.model.has_value());
    REQUIRE(!am.stop_reason.has_value());
    std::println("  [SUCCESS]");
}


void test_result_default() {
    std::println("Testing Result<Endpoint, void>: default construction...");

    Result<TestEndpoint, void> result{};
    REQUIRE(!result.data.has_value());
    REQUIRE(!result.error.has_value());
    REQUIRE(result.attempts.empty());
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

    std::println("===== ResultSync Tests =====");
    run(test_sync_initial_state);
    run(test_sync_signal_success);
    run(test_sync_signal_failure);
    run(test_sync_reset_clears_state);
    run(test_sync_signal_after_reset);
    run(test_sync_threaded_wait);
    run(test_sync_threaded_wait_failure);
    run(test_sync_threaded_retry_cycle);

    std::println("\n===== New Type Tests =====");
    run(test_token_usage_default);
    run(test_attempt_metadata_default);
    run(test_result_default);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
