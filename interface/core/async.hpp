#pragma once


#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "error.hpp"


namespace jai::llm {


class Orchestrator;
namespace curl { struct Response; }


/***
 * ResultSync — shared signaling block between the orchestrator and a
 * Result (blocking) or CoroAsyncResult (coroutine).
 *
 * Heap-allocated (via shared_ptr) so the address remains stable across
 * Result moves. The orchestrator holds a shared_ptr and signals readiness
 * by setting ready/succeeded/error_msg and notifying the cv. The Result
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
    // (CallAsync/Result path). Both are always fired so mixed usage is safe.
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

    // Reset for retry — called by Result or CoroAsyncResult after deserialization failure.
    void Reset() {
        std::lock_guard lock(mtx);
        ready = false;
        succeeded = false;
        error_msg.clear();
    }
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
    std::shared_ptr<ResultSync> sync;

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


// Non-template bridge functions — defined in async.cpp where Orchestrator
// and curl::Response are complete types. Called by AsyncResult::Resolve()
// and CoroAsyncResult's coroutine body.
//
// GetResponseRef borrows the response from a completed slot.
// ReleaseSlotRequest releases the slot back to the free list.
// RetrySlotRequest re-queues for retry; returns false if budget exhausted.
const curl::Response& GetResponseRef(Orchestrator* orch, size_t ticket);

void ReleaseSlotRequest(Orchestrator* orch, size_t ticket);

bool RetrySlotRequest(Orchestrator* orch,
                      size_t ticket,
                      std::shared_ptr<ResultSync> sync);


/***
 * AsyncResult — blocking pull-based result container for LLM API calls.
 *
 * Holds a deserialization function, an orchestrator reference, and a ticket
 * identifying the completed slot. The orchestrator signals readiness through
 * a heap-stable ResultSync block; accessors block until signaled, then pull
 * the curl::Response from the orchestrator (via GetResponse), deserialize on
 * the caller's thread, and cache the result.
 *
 * Pull model benefits:
 *   - Data/error slots are only written after AsyncResult is in its final
 *     location (no dangling pointers from moves).
 *   - Deserialization runs on the user's thread, freeing the orchestrator.
 *   - curl::Response is moved (cheap — pointer swaps), not copied.
 *
 * Non-copyable, movable. Move transfers ownership of the ResultSync block
 * (heap-stable) so the orchestrator's signal pointer remains valid.
 */
template <typename T, typename E = std::string>
class AsyncResult {
public:
    using Data_t = T;
    using Error_t = E;

    // Typed deserialization: curl::Response → Data_t.
    // Provided by the client (e.g., anthropic::Deserialize).
    using DeserializeFn = Data_t (*)(const curl::Response&);

private:
    std::optional<Data_t> data{};
    std::optional<Error_t> error{};
    std::exception_ptr eptr{};

    // Pull-model wiring — set by the client at CallAsync time.
    Orchestrator* orchestrator{nullptr};
    size_t ticket{0};
    DeserializeFn deserialize_fn{nullptr};

    std::shared_ptr<ResultSync> sync{std::make_shared<ResultSync>()};

    bool resolved{false};  // true after first accessor has pulled + deserialized

    // Block until the orchestrator signals readiness.
    void Wait() const {
        std::unique_lock lock(sync->mtx);
        sync->cv.wait(lock, [this] { return sync->ready; });
    }

    // Pull the response from the orchestrator and deserialize. Called once,
    // on the first blocking accessor, on the user's thread.
    //
    // On deserialization failure with retry_on_deserialize_failure enabled,
    // the orchestrator re-queues the slot and signals again on the next
    // HTTP completion. The loop sleeps between attempts via Wait().
    void Resolve() {
        if (resolved) { return; }

        while (true) {
            Wait();

            // HTTP-level failure (retries exhausted by orchestrator).
            if (!sync->succeeded) {
                error.emplace(std::move(sync->error_msg));
                break;
            }

            // Borrow the response from the completed slot and attempt
            // deserialization. The slot stays in COMPLETED state — the
            // Attempt still owns the Response data.
            try {
                const curl::Response& resp = GetResponseRef(orchestrator, ticket);
                data.emplace(deserialize_fn(resp));
                ReleaseSlotRequest(orchestrator, ticket);
                break;  // success
            } catch (...) {
                eptr = std::current_exception();
            }

            // Deserialization failed — ask orchestrator to retry.
            sync->Reset();
            if (!RetrySlotRequest(orchestrator, ticket, sync)) {
                // Retry budget exhausted — slot already released by RetrySlot.
                break;
            }
            // Loop back to Wait() for the next attempt.
        }

        resolved = true;
    }

public:
    AsyncResult() = default;

    // Constructed by the client in CallAsync.
    AsyncResult(Orchestrator& orch, size_t tkt, DeserializeFn fn, std::shared_ptr<ResultSync> s)
        : orchestrator{&orch}
        , ticket{tkt}
        , deserialize_fn{fn}
        , sync{std::move(s)}
    {}

    AsyncResult(AsyncResult const&) = delete;
    AsyncResult(AsyncResult&&) = default;
    AsyncResult& operator=(AsyncResult const&) = delete;
    AsyncResult& operator=(AsyncResult&&) = default;
    ~AsyncResult() = default;

    // Sync block access — the client passes this to the orchestrator at
    // submit time so the orchestrator can signal readiness.
    std::shared_ptr<ResultSync>& SyncBlock() { return sync; }

    // ----- User interface -----

    // Non-blocking check. Safe to call from any thread.
    bool IsDone() const {
        std::lock_guard lock(sync->mtx);
        return sync->ready;
    }

    // State queries — block until resolved.
    bool HasData() { Resolve(); return data.has_value(); }
    bool HasError() { Resolve(); return error.has_value(); }
    bool HasException() { Resolve(); return eptr != nullptr; }

    // Result access — blocks until resolved, then returns cached result.
    Data_t& Data() { Resolve(); return *data; }
    Error_t& Error() { Resolve(); return *error; }

    // Rethrow the captured exception if one exists. Blocks until resolved.
    void RethrowIfException() {
        Resolve();
        if (eptr) { std::rethrow_exception(eptr); }
    }
};


/***
 * CoroAsyncResult — coroutine-based result container for LLM API calls.
 *
 * Returned by Client::CallCoro(). The coroutine body starts eagerly,
 * submits the request to the orchestrator, and suspends at a SyncAwaiter
 * until the orchestrator signals completion. On resumption, the coroutine
 * deserializes the response, stores the result in the promise (coroutine
 * frame), and finishes. The caller retrieves the result via co_await or
 * by polling IsReady()/Data() after the event loop completes.
 *
 * Data lives in the coroutine frame (promise). The caller gets a copy
 * via co_await's await_resume — one extra hop, intentionally simple.
 *
 * Non-copyable, move-constructible. Destroying a CoroAsyncResult destroys
 * the coroutine frame.
 */
template <typename T>
class CoroAsyncResult {
public:
    using Data_t = T;

    class Promise_t;
    using promise_type = Promise_t;

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

        std::optional<Data_t> data{};
        std::exception_ptr eptr{};
        std::coroutine_handle<> waiting{};  // caller co_await-ing on this CoroAsyncResult

        CoroAsyncResult get_return_object() {
            return CoroAsyncResult{coro_handle::from_promise(*this)};
        }

        // Eager start: the coroutine begins immediately and runs until
        // the first co_await (SyncAwaiter), where it suspends.
        static auto initial_suspend() noexcept { return std::suspend_never{}; }

        // Suspend at final to keep the frame alive for the caller to read.
        // If a caller is co_await-ing on us, resume them.
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                bool await_ready() const noexcept { return false; }

                void await_suspend(coro_handle h) noexcept {
                    if (h.promise().waiting) {
                        h.promise().waiting.resume();
                    }
                }

                void await_resume() noexcept {}
            };
            return FinalAwaiter{};
        }

        void return_value(Data_t val) { data.emplace(std::move(val)); }

        void unhandled_exception() { eptr = std::current_exception(); }

        // Forward all co_await expressions unchanged (SyncAwaiter, etc.).
        template <typename Awaitable>
        auto&& await_transform(Awaitable&& a) { return std::forward<Awaitable>(a); }
    };

private:
    using coro_handle = typename Promise_t::coro_handle;
    coro_handle handle;

public:
    CoroAsyncResult(coro_handle h) : handle{h} {}

    CoroAsyncResult(const CoroAsyncResult&) = delete;
    CoroAsyncResult(CoroAsyncResult&& other) noexcept : handle{other.handle} {
        other.handle = nullptr;
    }
    CoroAsyncResult& operator=(const CoroAsyncResult&) = delete;
    CoroAsyncResult& operator=(CoroAsyncResult&&) = delete;
    ~CoroAsyncResult() { if (handle) { handle.destroy(); } }

    // ----- Polling interface (after RunOnce/RunUntilComplete) -----

    bool IsReady() const { return handle.done(); }
    bool HasData() const { return handle.promise().data.has_value(); }
    bool HasException() const { return handle.promise().eptr != nullptr; }

    Data_t& Data() { return *handle.promise().data; }
    const Data_t& Data() const { return *handle.promise().data; }

    void RethrowIfException() const {
        if (handle.promise().eptr) {
            std::rethrow_exception(handle.promise().eptr);
        }
    }

    // ----- Awaitable interface -----
    // Allows:  Message msg = co_await client.CallCoro(request);

    auto operator co_await() {
        struct Awaiter {
            CoroAsyncResult& task;

            bool await_ready() const noexcept { return task.handle.done(); }

            void await_suspend(std::coroutine_handle<> caller) noexcept {
                task.handle.promise().waiting = caller;
            }

            Data_t await_resume() {
                task.RethrowIfException();
                return std::move(*task.handle.promise().data);
            }
        };
        return Awaiter{*this};
    }
};


template <typename T, typename E = std::string>
class AsyncTask {
public:
    using Data_t = T;
    using Error_t = E;

    class Promise_t;
    using promise_type = Promise_t;

    struct Awaiter_t {
        bool ready{false};

        Awaiter_t(bool ready_) : ready{ready_} {}
        bool await_ready() const noexcept { return ready; }
        static void await_resume() noexcept {}
        static void await_suspend(std::coroutine_handle<>) noexcept {}
    };

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

    private:
        bool ready{true};

    public:
        static auto final_suspend() noexcept { return std::suspend_always(); }
        static auto initial_suspend() { return std::suspend_always(); }
        static void return_void() {}

        // Capture unhandled exceptions from the coroutine body rather than
        // terminating. The caller can inspect via HasException()/Exception().
        void unhandled_exception() { eptr = std::current_exception(); }

        auto get_return_object() { return coro_handle::from_promise(*this); }

        // Transform co_await std::suspend_always{} into our controllable awaiter.
        auto await_transform(std::suspend_always) { return Awaiter_t(!ready); }

        // Forward all other co_await expressions unchanged — allows
        // co_await on any awaitable type (e.g. another AsyncTask).
        template <typename Awaitable>
        auto&& await_transform(Awaitable&& a) { return std::forward<Awaitable>(a); }

        void disable_suspension() { ready = false; }

        std::exception_ptr eptr{};
    };

private:
    Promise_t::coro_handle handle;

    // Result storage — exactly one of these is engaged after completion,
    // both empty while in-flight. ProcessFn emplaces data on success,
    // FailFn emplaces error on permanent failure.
    std::optional<Data_t> data{};
    std::optional<Error_t> error{};

public:
    // Born from a coroutine — the compiler calls this via promise_type.
    AsyncTask(Promise_t::coro_handle h) : handle{h} {}

    // Born from a regular function (e.g. CallAsync). No coroutine frame.
    // The suspend/step interface is inactive; awaitable and polling work normally.
    AsyncTask() : handle{nullptr} {}

    AsyncTask(AsyncTask const&) = delete;
    AsyncTask(AsyncTask&&) = delete;
    AsyncTask& operator=(AsyncTask const&) = delete;
    AsyncTask& operator=(AsyncTask&&) = delete;
    ~AsyncTask() { if (handle) { handle.destroy(); } }

    // Convert to synchronous execution — all subsequent co_await points
    // become no-ops and the coroutine runs straight through.
    void disable_suspension() const {
        if (handle.done()) { return; }
        handle.promise().disable_suspension();
        handle();
    }

    // Manual single-step resumption. Returns true if more work remains.
    bool operator()() {
        if (!handle.done()) { handle(); }
        return !handle.done();
    }

    // State queries — three states: in-flight, success, failure.
    // Exception is a fourth abnormal state (bug in coroutine body).
    bool IsReady() const { return data.has_value() || error.has_value(); }
    bool HasData() const { return data.has_value(); }
    bool HasError() const { return error.has_value(); }
    bool HasException() const { return handle.promise().eptr != nullptr; }

    // Result access — caller checks HasData()/HasError() after resume.
    const Data_t& Data() const { return *data; }
    Data_t& Data() { return *data; }
    const Error_t& Error() const { return *error; }
    Error_t& Error() { return *error; }

    // Rethrow the captured exception if one exists.
    void RethrowIfException() const {
        if (auto& eptr = handle.promise().eptr) {
            std::rethrow_exception(eptr);
        }
    }

    // Mutable access for callbacks to emplace results through a pointer.
    std::optional<Data_t>& DataSlot() { return data; }
    std::optional<Error_t>& ErrorSlot() { return error; }

    //----- Awaitable interface -----
    // Allows: Message msg = co_await task;

    // Resume the coroutine that is co_await-ing on this task.
    // Called after data or error has been emplaced (e.g. by the orchestrator
    // callback). No-op if nobody is waiting.
    void Resume() {
        if (waiting) { waiting.resume(); }
    }

    // Makes this task co_await-able. Returns a lightweight awaiter that
    // holds a reference back to this task.
    auto operator co_await() {
        struct TaskAwaiter {
            AsyncTask& task;

            // If the result is already available, don't suspend.
            bool await_ready() const noexcept { return task.IsReady(); }

            // Store the caller's coroutine handle so Resume() can wake them.
            void await_suspend(std::coroutine_handle<> caller) noexcept {
                task.waiting = caller;
            }

            // Called when the caller is resumed — hand them a copy of the result.
            // Returns by value so `auto result = co_await CallAsync(...)` is safe
            // even when the AsyncTask is a temporary.
            Data_t await_resume() {
                task.RethrowIfException();
                return *task.data;
            }
        };
        return TaskAwaiter{*this};
    }

private:
    // Handle of the coroutine that is co_await-ing on this task.
    // Set by await_suspend, used by Resume().
    std::coroutine_handle<> waiting{};
};


}
