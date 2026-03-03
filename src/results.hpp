#pragma once

#include "../interface/core/results.hpp"

#include <condition_variable>
#include <coroutine>
#include <memory>
#include <mutex>
#include <string>
#include <utility>


namespace jai::llm {


class Orchestrator;

namespace curl { struct Response; }


// Build transport-level AttemptMetadata from a completed curl::Response.
// Defined in results.cpp. Caller overrides outcome and error as needed.
AttemptMetadata BuildTransportMetadata(const curl::Response& resp);


/***
 * ResultSync — shared signaling block between the orchestrator and a
 * AsyncResult (blocking) or CoroAsyncResult (coroutine).
 *
 * Heap-allocated (via shared_ptr) so the address remains stable across
 * AsyncResult moves. The orchestrator holds a shared_ptr and signals readiness
 * by setting ready/succeeded/error_msg and notifying the cv. The AsyncResult
 * waits on the cv in its blocking accessors.
 *
 * For the coroutine path (CoroAsyncResult / CallCoro), a SyncAwaiter stores
 * a coroutine_handle in coro_handle before suspending. Signal() resumes
 * that handle after releasing the lock, driving the coroutine forward.
 * Both cv and coroutine paths fire on every Signal so mixed usage is safe.
 *
 * Also used for the retry handshake: if deserialization fails on the
 * caller's thread, the caller resets ready/succeeded and the orchestrator
 * re-queues the slot and signals again on the next HTTP completion.
 */
struct ResultSync {
    std::mutex mtx{};
    std::condition_variable cv{};
    std::coroutine_handle<> coro_handle{};  // coroutine to resume on signal (CallCoro path)
    bool ready{false};         // orchestrator has finished (success or failure)
    bool succeeded{false};     // true = response available via GetResponse, false = error
    std::string error_msg{};   // populated on failure before signaling

    // Signal readiness — called by the orchestrator from its worker thread.
    // If a coroutine handle is stored (CallCoro path), resumes it after
    // releasing the lock. Otherwise notifies the condition variable
    // (CallAsync/AsyncResult path). Both are always fired so mixed usage is safe.
    void Signal(bool success, std::string err = {}) {
        std::coroutine_handle<> h{};
        {
            std::lock_guard lock(mtx);
            succeeded = success;
            error_msg = std::move(err);
            ready = true;
            h = std::exchange(coro_handle, {});
        }
        cv.notify_one();
        if (h) { h.resume(); }
    }

    // Reset for retry — called by AsyncResult or CoroAsyncResult after deserialization failure.
    void Reset() {
        std::lock_guard lock(mtx);
        ready = false;
        succeeded = false;
        error_msg.clear();
    }
};


/***
 * SubmitResult - Returned by SubmitRequest.
 * Wraps the orchestrator interaction so that endpoint code (CallCoro, etc.)
 * never calls Orchestrator directly. Forward-declared in client.hpp; defined
 * here where Orchestrator is complete.
 */
struct SubmitResult {
    Orchestrator* orchestrator;
    size_t ticket;
    std::shared_ptr<ResultSync> sync;

    // Borrow the completed HTTP response from the orchestrator.
    const curl::Response& GetResponse() const;

    // Release the completed slot back to the free list.
    void ReleaseSlot();

    // Reset the sync block and re-queue for retry.
    // Returns false if the retry budget is exhausted (slot released).
    bool RetrySlot();
};


/***
 * SyncAwaiter — suspends a coroutine until a ResultSync block is signaled.
 *
 * Used by CoroAsyncResult's coroutine body (CallCoro) to suspend at the
 * orchestrator boundary. Stores the coroutine handle in the ResultSync
 * so that Signal() can resume it directly.
 *
 * Race-safe: if Signal() fires between await_ready and await_suspend,
 * await_suspend returns false (don't suspend) and the coroutine continues.
 */
struct SyncAwaiter {
    ResultSync* sync;

    bool await_ready() const {
        std::lock_guard lock(sync->mtx);
        return sync->ready;
    }

    bool await_suspend(std::coroutine_handle<> h) {
        std::lock_guard lock(sync->mtx);
        if (sync->ready) { return false; }  // already signaled, don't suspend
        sync->coro_handle = h;
        return true;
    }

    void await_resume() const noexcept {}
};


}
