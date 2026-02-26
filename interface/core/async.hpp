#pragma once


#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "error.hpp"


namespace jai::llm {


class Orchestrator;
namespace curl { struct Response; }


/***
 * ResultSync — shared signaling block between the orchestrator and a Result.
 *
 * Heap-allocated (via shared_ptr) so the address remains stable across
 * Result moves. The orchestrator holds a shared_ptr and signals readiness
 * by setting ready/succeeded/error_msg and notifying the cv. The Result
 * waits on the cv in its blocking accessors.
 *
 * Also used for the retry handshake: if deserialization fails on the
 * caller's thread, the caller resets ready/succeeded and the orchestrator
 * re-queues the slot and signals again on the next HTTP completion.
 */
struct ResultSync {
    std::mutex mtx{};
    std::condition_variable cv{};
    bool ready{false};         // orchestrator has finished (success or failure)
    bool succeeded{false};     // true = response available via GetResponse, false = error
    std::string error_msg{};   // populated on failure before signaling

    // Signal readiness — called by the orchestrator from its worker thread.
    void Signal(bool success, std::string err = {}) {
        {
            std::lock_guard lock(mtx);
            succeeded = success;
            error_msg = std::move(err);
            ready = true;
        }
        cv.notify_one();
    }

    // Reset for retry — called by the Result after deserialization failure.
    void Reset() {
        std::lock_guard lock(mtx);
        ready = false;
        succeeded = false;
        error_msg.clear();
    }
};


// Non-template bridge functions — defined in async.cpp where Orchestrator
// and curl::Response are complete types. Called by Result::Resolve().
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
 * Result — pull-based result container for LLM API calls.
 *
 * Holds a deserialization function, an orchestrator reference, and a ticket
 * identifying the completed slot. The orchestrator signals readiness through
 * a heap-stable ResultSync block; accessors block until signaled, then pull
 * the curl::Response from the orchestrator (via GetResponse), deserialize on
 * the caller's thread, and cache the result.
 *
 * Pull model benefits:
 *   - Data/error slots are only written after Result is in its final location
 *     (no dangling pointers from moves).
 *   - Deserialization runs on the user's thread, freeing the orchestrator.
 *   - curl::Response is moved (cheap — pointer swaps), not copied.
 *
 * Non-copyable, movable. Move transfers ownership of the ResultSync block
 * (heap-stable) so the orchestrator's signal pointer remains valid.
 */
template <typename T, typename E = std::string>
class Result {
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
    Result() = default;

    // Constructed by the client in CallAsync.
    Result(Orchestrator& orch, size_t tkt, DeserializeFn fn, std::shared_ptr<ResultSync> s)
        : orchestrator{&orch}
        , ticket{tkt}
        , deserialize_fn{fn}
        , sync{std::move(s)}
    {}

    Result(Result const&) = delete;
    Result(Result&&) = default;
    Result& operator=(Result const&) = delete;
    Result& operator=(Result&&) = default;
    ~Result() = default;

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
