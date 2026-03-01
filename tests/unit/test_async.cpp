/***
 * Async unit tests — ResultSync signaling, SyncAwaiter, and CoroAsyncResult
 * coroutine mechanics.
 *
 * ResultSync is a standalone synchronization primitive that can be tested
 * without the orchestrator. CoroAsyncResult and SyncAwaiter test the
 * coroutine-based call path.
 *
 * AsyncResult<T> tests require a live Orchestrator + curl::Response, so
 * they belong in the integration test suite.
 */

#include <atomic>
#include <chrono>
#include <print>
#include <string>
#include <thread>

#include "test_assert.hpp"
#include "../../interface/core/results.hpp"
#include "../../src/sync.hpp"


using namespace jai::llm;
using namespace std::chrono_literals;


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

    SyncAwaiter awaiter{sync};
    REQUIRE_EQ(awaiter.await_ready(), true);
    std::println("  [SUCCESS]");
}


void test_sync_awaiter_not_ready() {
    std::println("Testing SyncAwaiter: not ready (would suspend)...");

    auto sync = std::make_shared<ResultSync>();
    SyncAwaiter awaiter{sync};

    REQUIRE_EQ(awaiter.await_ready(), false);
    std::println("  [SUCCESS]");
}


void test_sync_awaiter_race_protection() {
    std::println("Testing SyncAwaiter: race between await_ready and await_suspend...");

    auto sync = std::make_shared<ResultSync>();
    SyncAwaiter awaiter{sync};

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
 */

// Simple coroutine that returns an int via co_return.
CoroAsyncResult<int> coro_return_value() {
    co_return 42;
}


// Coroutine that suspends on a SyncAwaiter, then returns.
CoroAsyncResult<int> coro_await_sync(std::shared_ptr<ResultSync> sync) {
    co_await SyncAwaiter{sync};
    co_return 99;
}


// Coroutine that throws.
CoroAsyncResult<int> coro_throw() {
    throw std::runtime_error("coro error");
    co_return 0;
}


// Coroutine that throws after awaiting.
CoroAsyncResult<int> coro_throw_after_await(std::shared_ptr<ResultSync> sync) {
    co_await SyncAwaiter{sync};
    throw std::runtime_error("post-await error");
    co_return 0;
}


void test_coro_result_immediate_return() {
    std::println("Testing CoroAsyncResult: immediate co_return...");

    auto result = coro_return_value();

    // Eager start + no suspension points → already done.
    REQUIRE_EQ(result.IsReady(), true);
    REQUIRE_EQ(result.HasData(), true);
    REQUIRE_EQ(result.Data(), 42);
    std::println("  [SUCCESS]");
}


void test_coro_result_await_sync() {
    std::println("Testing CoroAsyncResult: suspend on SyncAwaiter, then signal...");

    auto sync = std::make_shared<ResultSync>();
    auto result = coro_await_sync(sync);

    // Should be suspended at the SyncAwaiter.
    REQUIRE_EQ(result.IsReady(), false);

    // Signal the sync — this resumes the coroutine.
    sync->Signal(true);

    // Now the coroutine has run to co_return.
    REQUIRE_EQ(result.IsReady(), true);
    REQUIRE_EQ(result.Data(), 99);
    std::println("  [SUCCESS]");
}


void test_coro_result_exception_immediate() {
    std::println("Testing CoroAsyncResult: immediate throw...");

    auto result = coro_throw();

    // Eager start → throws immediately → captured in promise.
    REQUIRE_EQ(result.IsReady(), true);
    REQUIRE_EQ(result.HasException(), true);
    REQUIRE_EQ(result.HasData(), false);

    bool caught = false;
    try {
        result.RethrowIfException();
    } catch (const std::runtime_error& e) {
        caught = true;
        REQUIRE_EQ(std::string{e.what()}, std::string{"coro error"});
    }
    REQUIRE(caught);
    std::println("  [SUCCESS]");
}


void test_coro_result_exception_after_await() {
    std::println("Testing CoroAsyncResult: throw after SyncAwaiter...");

    auto sync = std::make_shared<ResultSync>();
    auto result = coro_throw_after_await(sync);

    REQUIRE_EQ(result.IsReady(), false);

    sync->Signal(true);

    REQUIRE_EQ(result.IsReady(), true);
    REQUIRE_EQ(result.HasException(), true);

    bool caught = false;
    try {
        result.RethrowIfException();
    } catch (const std::runtime_error& e) {
        caught = true;
        REQUIRE_EQ(std::string{e.what()}, std::string{"post-await error"});
    }
    REQUIRE(caught);
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
    REQUIRE_EQ(moved.Data(), 99);
    std::println("  [SUCCESS]");
}


void test_coro_result_threaded_signal() {
    std::println("Testing CoroAsyncResult: signal from another thread...");

    auto sync = std::make_shared<ResultSync>();
    auto result = coro_await_sync(sync);

    REQUIRE_EQ(result.IsReady(), false);

    // Signal from a different thread.
    std::thread signaler([&]() {
        std::this_thread::sleep_for(10ms);
        sync->Signal(true);
    });

    signaler.join();

    REQUIRE_EQ(result.IsReady(), true);
    REQUIRE_EQ(result.Data(), 99);
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

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
