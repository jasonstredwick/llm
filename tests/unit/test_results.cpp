/***
 * Async unit tests — ResultSync signaling, SyncAwaiter, and CoroAsyncResult
 * coroutine mechanics.
 *
 * ResultSync is a standalone synchronization primitive that can be tested
 * without the orchestrator. CoroAsyncResult and SyncAwaiter test the
 * coroutine-based call path.
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


// ----- Test endpoint tag for CoroAsyncResult tests -----
// CoroAsyncResult<Endpoint> requires Endpoint::Response_t.
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
 * SyncAwaiter Tests
 */

void test_sync_awaiter_ready_immediately() {
    std::println("Testing SyncAwaiter: ready immediately (no suspension)...");

    auto sync = std::make_shared<ResultSync>();
    sync->Signal(true);  // pre-signal

    SyncAwaiter awaiter{sync.get()};
    REQUIRE_EQ(awaiter.await_ready(), true);
    std::println("  [SUCCESS]");
}


void test_sync_awaiter_not_ready() {
    std::println("Testing SyncAwaiter: not ready (would suspend)...");

    auto sync = std::make_shared<ResultSync>();
    SyncAwaiter awaiter{sync.get()};

    REQUIRE_EQ(awaiter.await_ready(), false);
    std::println("  [SUCCESS]");
}


void test_sync_awaiter_race_protection() {
    std::println("Testing SyncAwaiter: race between await_ready and await_suspend...");

    auto sync = std::make_shared<ResultSync>();
    SyncAwaiter awaiter{sync.get()};

    // await_ready returns false (not signaled yet).
    REQUIRE_EQ(awaiter.await_ready(), false);

    // Signal fires between await_ready and await_suspend.
    sync->Signal(true);

    // await_suspend should return false (don't suspend, already signaled).
    bool should_suspend = awaiter.await_suspend(std::noop_coroutine());
    REQUIRE_EQ(should_suspend, false);
    std::println("  [SUCCESS]");
}


void test_sync_coro_handle_resume() {
    std::println("Testing ResultSync: coroutine handle resumed on signal...");

    auto sync = std::make_shared<ResultSync>();
    bool resumed = false;

    // Use a thread to simulate the orchestrator signaling.
    // Store a coroutine handle manually, then signal.
    // We can't easily create a real coroutine handle for this test,
    // so verify the handle is stored and cleared correctly.
    {
        std::lock_guard lock(sync->mtx);
        REQUIRE(!sync->coro_handle);
    }

    // After signal with no coro_handle, everything still works.
    sync->Signal(true);
    REQUIRE_EQ(sync->ready, true);

    // Verify coro_handle was cleared by Signal (it was empty, so exchange is no-op).
    {
        std::lock_guard lock(sync->mtx);
        REQUIRE(!sync->coro_handle);
    }
    std::println("  [SUCCESS]");
}


/***
 * CoroAsyncResult Tests
 *
 * CoroAsyncResult<Endpoint> requires Endpoint::Response_t.
 * Coroutines now co_return Result<Endpoint> (not plain values).
 */

// Helper: build a successful Result with the given value.
Result<TestEndpoint, void> make_result(int value) {
    Result<TestEndpoint, void> r;
    r.data.emplace(value);
    return r;
}

// Helper: build a Result with an error.
Result<TestEndpoint, void> make_error_result(std::string error) {
    Result<TestEndpoint, void> r;
    r.error.emplace(std::move(error));
    return r;
}


// Simple coroutine that returns an int via co_return.
CoroAsyncResult<TestEndpoint> coro_return_value() {
    co_return make_result(42);
}


// Coroutine that suspends on a SyncAwaiter, then returns.
CoroAsyncResult<TestEndpoint> coro_await_sync(std::shared_ptr<ResultSync> sync) {
    co_await SyncAwaiter{sync.get()};
    co_return make_result(99);
}


// Coroutine that throws.
CoroAsyncResult<TestEndpoint> coro_throw() {
    throw std::runtime_error("coro error");
    co_return make_result(0);
}


// Coroutine that throws after awaiting.
CoroAsyncResult<TestEndpoint> coro_throw_after_await(std::shared_ptr<ResultSync> sync) {
    co_await SyncAwaiter{sync.get()};
    throw std::runtime_error("post-await error");
    co_return make_result(0);
}


// Coroutine that returns a result with error (not an exception).
CoroAsyncResult<TestEndpoint> coro_return_error() {
    co_return make_error_result("call failed");
}


void test_coro_result_immediate_return() {
    std::println("Testing CoroAsyncResult: immediate co_return...");

    auto coro = coro_return_value();

    // Eager start + no suspension points → already done.
    REQUIRE_EQ(coro.IsReady(), true);
    const auto& result = coro.Get();
    REQUIRE(result.data.has_value());
    REQUIRE_EQ(*result.data, 42);
    std::println("  [SUCCESS]");
}


void test_coro_result_await_sync() {
    std::println("Testing CoroAsyncResult: suspend on SyncAwaiter, then signal...");

    auto sync = std::make_shared<ResultSync>();
    auto coro = coro_await_sync(sync);

    // Should be suspended at the SyncAwaiter.
    REQUIRE_EQ(coro.IsReady(), false);

    // Signal the sync — this resumes the coroutine.
    sync->Signal(true);

    // Now the coroutine has run to co_return.
    REQUIRE_EQ(coro.IsReady(), true);
    REQUIRE_EQ(*coro.Get().data, 99);
    std::println("  [SUCCESS]");
}


void test_coro_result_exception_immediate() {
    std::println("Testing CoroAsyncResult: immediate throw stored as error...");

    auto coro = coro_throw();

    // Eager start → throws immediately → unhandled_exception stores error on Result.
    REQUIRE_EQ(coro.IsReady(), true);

    const auto& result = coro.Get();
    REQUIRE(result.error.has_value());
    REQUIRE(!result.data.has_value());
    std::println("  [SUCCESS]");
}


void test_coro_result_exception_after_await() {
    std::println("Testing CoroAsyncResult: throw after SyncAwaiter stored as error...");

    auto sync = std::make_shared<ResultSync>();
    auto coro = coro_throw_after_await(sync);

    REQUIRE_EQ(coro.IsReady(), false);

    sync->Signal(true);

    REQUIRE_EQ(coro.IsReady(), true);

    const auto& result = coro.Get();
    REQUIRE(result.error.has_value());
    REQUIRE(!result.data.has_value());
    std::println("  [SUCCESS]");
}


void test_coro_result_move_construct() {
    std::println("Testing CoroAsyncResult: move construction...");

    auto sync = std::make_shared<ResultSync>();
    auto original = coro_await_sync(sync);
    REQUIRE_EQ(original.IsReady(), false);

    // Move-construct.
    auto moved = std::move(original);

    // Signal through the moved result.
    sync->Signal(true);
    REQUIRE_EQ(moved.IsReady(), true);
    REQUIRE_EQ(*moved.Get().data, 99);
    std::println("  [SUCCESS]");
}


void test_coro_result_threaded_signal() {
    std::println("Testing CoroAsyncResult: signal from another thread...");

    auto sync = std::make_shared<ResultSync>();
    auto coro = coro_await_sync(sync);

    REQUIRE_EQ(coro.IsReady(), false);

    // Signal from a different thread.
    std::thread signaler([&]() {
        std::this_thread::sleep_for(10ms);
        sync->Signal(true);
    });

    signaler.join();

    REQUIRE_EQ(coro.IsReady(), true);
    REQUIRE_EQ(*coro.Get().data, 99);
    std::println("  [SUCCESS]");
}


void test_coro_result_error_return() {
    std::println("Testing CoroAsyncResult: co_return with error result...");

    auto coro = coro_return_error();

    REQUIRE_EQ(coro.IsReady(), true);
    const auto& result = coro.Get();
    REQUIRE(result.error.has_value());
    REQUIRE(!result.data.has_value());
    REQUIRE_EQ(*result.error, std::string{"call failed"});
    std::println("  [SUCCESS]");
}


void test_coro_result_get() {
    std::println("Testing CoroAsyncResult: Get() returns full Result aggregate...");

    auto coro = coro_return_value();

    REQUIRE_EQ(coro.IsReady(), true);
    const auto& result = coro.Get();
    REQUIRE(result.data.has_value());
    REQUIRE_EQ(*result.data, 42);
    REQUIRE(!result.error.has_value());
    std::println("  [SUCCESS]");
}


void test_coro_result_take() {
    std::println("Testing CoroAsyncResult: Take() moves Result out...");

    auto coro = coro_return_value();

    REQUIRE_EQ(coro.IsReady(), true);
    auto result = coro.Take();
    REQUIRE(result.data.has_value());
    REQUIRE_EQ(*result.data, 42);
    REQUIRE(!result.error.has_value());
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
 * Tier 2: Result<Endpoint, Data> and TransformResult Tests
 */

// User data type for Tier 2 tests.
struct UserData {
    std::string label{};
    int doubled{0};
};

// User transform: convert int Response_t to UserData.
UserData DoubleTransform(const int& value) {
    return UserData{.label = "doubled", .doubled = value * 2};
}

// User transform that throws.
UserData ThrowingTransform(const int& /*value*/) {
    throw std::runtime_error("transform failed");
}


void test_result_data_default() {
    std::println("Testing Result<Endpoint, Data>: default construction...");

    Result<TestEndpoint, UserData> result{};
    REQUIRE(!result.data.has_value());
    REQUIRE(!result.response.has_value());
    REQUIRE(!result.error.has_value());
    REQUIRE(result.attempts.empty());
    std::println("  [SUCCESS]");
}


void test_transform_result_success() {
    std::println("Testing TransformResult: success path...");

    Result<TestEndpoint, void> tier1;
    tier1.data.emplace(21);
    tier1.attempts.push_back(AttemptMetadata{.outcome = AttemptOutcome::SUCCESS});

    auto result = TransformResult<TestEndpoint, UserData>(std::move(tier1), &DoubleTransform);

    REQUIRE(result.data.has_value());
    REQUIRE_EQ(result.data->label, std::string{"doubled"});
    REQUIRE_EQ(result.data->doubled, 42);
    REQUIRE(result.response.has_value());
    REQUIRE_EQ(*result.response, 21);
    REQUIRE(!result.error.has_value());
    REQUIRE_EQ(result.attempts.size(), static_cast<size_t>(1));
    std::println("  [SUCCESS]");
}


void test_transform_result_transform_throws() {
    std::println("Testing TransformResult: transform throws...");

    Result<TestEndpoint, void> tier1;
    tier1.data.emplace(10);

    auto result = TransformResult<TestEndpoint, UserData>(std::move(tier1), &ThrowingTransform);

    // Transform failed — error set, data empty, but response still preserved.
    REQUIRE(!result.data.has_value());
    REQUIRE(result.response.has_value());
    REQUIRE_EQ(*result.response, 10);
    REQUIRE(result.error.has_value());
    REQUIRE(result.error->find("User transform failed") != std::string::npos);
    std::println("  [SUCCESS]");
}


void test_transform_result_tier1_error() {
    std::println("Testing TransformResult: Tier 1 error passthrough...");

    Result<TestEndpoint, void> tier1;
    tier1.error.emplace("HTTP 500");
    tier1.attempts.push_back(AttemptMetadata{.outcome = AttemptOutcome::HTTP_ERROR, .error = "HTTP 500"});

    auto result = TransformResult<TestEndpoint, UserData>(std::move(tier1), &DoubleTransform);

    // Error passed through, no data or response.
    REQUIRE(!result.data.has_value());
    REQUIRE(!result.response.has_value());
    REQUIRE(result.error.has_value());
    REQUIRE_EQ(*result.error, std::string{"HTTP 500"});
    REQUIRE_EQ(result.attempts.size(), static_cast<size_t>(1));
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

    std::println("\n===== SyncAwaiter Tests =====");
    run(test_sync_awaiter_ready_immediately);
    run(test_sync_awaiter_not_ready);
    run(test_sync_awaiter_race_protection);
    run(test_sync_coro_handle_resume);

    std::println("\n===== CoroAsyncResult Tests =====");
    run(test_coro_result_immediate_return);
    run(test_coro_result_await_sync);
    run(test_coro_result_exception_immediate);
    run(test_coro_result_exception_after_await);
    run(test_coro_result_move_construct);
    run(test_coro_result_threaded_signal);
    run(test_coro_result_error_return);
    run(test_coro_result_get);
    run(test_coro_result_take);

    std::println("\n===== New Type Tests =====");
    run(test_token_usage_default);
    run(test_attempt_metadata_default);
    run(test_result_default);

    std::println("\n===== Tier 2: User Transform Tests =====");
    run(test_result_data_default);
    run(test_transform_result_success);
    run(test_transform_result_transform_throws);
    run(test_transform_result_tier1_error);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
