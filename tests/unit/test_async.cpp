/***
 * Async unit tests — ResultSync signaling and AsyncTask coroutine mechanics.
 *
 * ResultSync is a standalone synchronization primitive that can be tested
 * without the orchestrator. AsyncTask is a coroutine wrapper testable
 * by stepping through coroutine frames.
 *
 * Result<T> tests require a live Orchestrator + curl::Response, so they
 * belong in the integration test suite.
 */

#include <atomic>
#include <chrono>
#include <print>
#include <string>
#include <thread>

#include "test_assert.hpp"
#include "../../interface/core/async.hpp"


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
 * AsyncTask Tests
 */

// Simple coroutine that yields once, then completes.
AsyncTask<int> simple_coro() {
    co_await std::suspend_always{};
    co_return;
}


// Coroutine that yields multiple times.
AsyncTask<int> multi_step_coro() {
    co_await std::suspend_always{};
    co_await std::suspend_always{};
    co_await std::suspend_always{};
    co_return;
}


// Coroutine that throws.
AsyncTask<int> throwing_coro() {
    co_await std::suspend_always{};
    throw std::runtime_error("coroutine error");
    co_return;
}


void test_async_task_initial_state() {
    std::println("Testing AsyncTask: initial state...");

    auto task = simple_coro();

    REQUIRE_EQ(task.IsReady(), false);
    REQUIRE_EQ(task.HasData(), false);
    REQUIRE_EQ(task.HasError(), false);
    std::println("  [SUCCESS]");
}


void test_async_task_step() {
    std::println("Testing AsyncTask: manual step...");

    auto task = simple_coro();

    // First step: resume from initial_suspend to first co_await.
    bool more = task();
    REQUIRE_EQ(more, true);  // suspended at co_await

    // Second step: resume from co_await to co_return.
    more = task();
    REQUIRE_EQ(more, false);  // done
    std::println("  [SUCCESS]");
}


void test_async_task_multi_step() {
    std::println("Testing AsyncTask: multi-step coroutine...");

    auto task = multi_step_coro();

    int steps = 0;
    while (task()) {
        ++steps;
    }

    // initial_suspend (step 1) + 3 co_awaits (steps 2,3,4) = 4 resumes,
    // last resume returns false. So we count 3 true-returning steps
    // (initial_suspend is the first resume, then 3 suspensions, then final).
    // Actually: initial_suspend → resume(1, more=true) → co_await1 → resume(2, more=true)
    //           → co_await2 → resume(3, more=true) → co_await3 → resume(4, more=false)
    // So steps where task() returns true = 3.
    REQUIRE_EQ(steps, 3);
    std::println("  [SUCCESS]");
}


void test_async_task_disable_suspension() {
    std::println("Testing AsyncTask: disable_suspension runs to completion...");

    auto task = multi_step_coro();

    task.disable_suspension();

    // After disable_suspension, task should have run to completion.
    // More steps should return false.
    bool more = task();
    REQUIRE_EQ(more, false);
    std::println("  [SUCCESS]");
}


void test_async_task_data_slot() {
    std::println("Testing AsyncTask: DataSlot emplacement...");

    auto task = simple_coro();

    REQUIRE_EQ(task.HasData(), false);

    task.DataSlot().emplace(42);

    REQUIRE_EQ(task.HasData(), true);
    REQUIRE_EQ(task.IsReady(), true);
    REQUIRE_EQ(task.Data(), 42);
    std::println("  [SUCCESS]");
}


void test_async_task_error_slot() {
    std::println("Testing AsyncTask: ErrorSlot emplacement...");

    auto task = simple_coro();

    REQUIRE_EQ(task.HasError(), false);

    task.ErrorSlot().emplace("some error");

    REQUIRE_EQ(task.HasError(), true);
    REQUIRE_EQ(task.IsReady(), true);
    REQUIRE_EQ(task.Error(), std::string{"some error"});
    std::println("  [SUCCESS]");
}


void test_async_task_exception_capture() {
    std::println("Testing AsyncTask: exception capture...");

    auto task = throwing_coro();

    // Step past initial_suspend.
    task();

    // This resume hits the throw.
    task();

    REQUIRE_EQ(task.HasException(), true);

    bool caught = false;
    try {
        task.RethrowIfException();
    } catch (const std::runtime_error& e) {
        caught = true;
        REQUIRE_EQ(std::string{e.what()}, std::string{"coroutine error"});
    }
    REQUIRE(caught);
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

    std::println("\n===== AsyncTask Tests =====");
    run(test_async_task_initial_state);
    run(test_async_task_step);
    run(test_async_task_multi_step);
    run(test_async_task_disable_suspension);
    run(test_async_task_data_slot);
    run(test_async_task_error_slot);
    run(test_async_task_exception_capture);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
