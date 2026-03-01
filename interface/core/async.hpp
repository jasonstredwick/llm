#pragma once


#include <coroutine>
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>


namespace jai::llm {


class Orchestrator;
struct ResultSync;  // defined in src/sync.hpp — implementation detail


// ResultSync bridge — defined in async.cpp. Keeps the public header free
// of the full ResultSync definition.
std::shared_ptr<ResultSync> MakeResultSync();
bool IsSyncReady(std::shared_ptr<ResultSync> const& sync);


/***
 * AsyncResultBase — non-template base for AsyncResult.
 *
 * Owns the orchestrator wiring (ticket, sync block) and the wait/retry
 * loop (Resolve). The two type-dependent steps — deserialization and
 * error storage — are delegated to pure virtual methods overridden by
 * the derived AsyncResult<T, E>.
 *
 * Resolve() is defined in async.cpp, keeping curl::Response, ResultSync
 * internals, and the orchestrator API behind the interface boundary.
 *
 * Non-copyable, movable. Move transfers ownership of the ResultSync block
 * (heap-stable) so the orchestrator's signal pointer remains valid.
 */
class AsyncResultBase {
protected:
    // Type-dependent hooks — overridden by AsyncResult<T, E>.
    virtual void ApplyDeserialization() = 0;
    virtual void ApplyError() = 0;

    Orchestrator* orchestrator{nullptr};
    size_t ticket{0};
    std::shared_ptr<ResultSync> sync{MakeResultSync()};
    std::exception_ptr eptr{};
    std::string error_msg{};   // populated by Resolve() before calling ApplyError()
    bool resolved{false};

    // Wait/retry loop — defined in async.cpp.
    void Resolve();

public:
    AsyncResultBase(Orchestrator& orch, size_t tkt, std::shared_ptr<ResultSync> s)
        : orchestrator{&orch}
        , ticket{tkt}
        , sync{std::move(s)}
    {}

    AsyncResultBase(AsyncResultBase const&) = delete;
    AsyncResultBase(AsyncResultBase&&) = default;
    AsyncResultBase& operator=(AsyncResultBase const&) = delete;
    AsyncResultBase& operator=(AsyncResultBase&&) = default;
    virtual ~AsyncResultBase() = default;

    // Sync block access — the client passes this to the orchestrator at
    // submit time so the orchestrator can signal readiness.
    std::shared_ptr<ResultSync>& SyncBlock() { return sync; }

    // Non-blocking check. Safe to call from any thread.
    bool IsDone() const { return IsSyncReady(sync); }

    // Blocks until resolved.
    bool HasException() { Resolve(); return eptr != nullptr; }

    // Rethrow the captured exception if one exists. Blocks until resolved.
    void RethrowIfException() {
        Resolve();
        if (eptr) { std::rethrow_exception(eptr); }
    }
};


/***
 * AsyncResult — blocking pull-based result container for LLM API calls.
 *
 * Derives from AsyncResultBase, which owns the wait/retry loop. This
 * template adds typed data/error storage and implements the two virtual
 * hooks for deserialization and error emplacement.
 *
 * Pull model benefits:
 *   - Data/error slots are only written after AsyncResult is in its final
 *     location (no dangling pointers from moves).
 *   - Deserialization runs on the user's thread, freeing the orchestrator.
 *
 * Non-copyable, movable.
 */
template <typename T, typename E = std::string>
class AsyncResult : public AsyncResultBase {
public:
    using Data_t = T;
    using Error_t = E;

    // Typed deserialization function. Provided by the client at CallAsync
    // time. Takes the orchestrator and ticket to borrow the completed
    // response internally — the caller never sees curl::Response.
    using DeserializeFn = Data_t (*)(Orchestrator*, size_t);

private:
    std::optional<Data_t> data{};
    std::optional<Error_t> error{};
    DeserializeFn deserialize_fn{nullptr};

    // Called by AsyncResultBase::Resolve() on successful HTTP completion.
    void ApplyDeserialization() override {
        data.emplace(deserialize_fn(orchestrator, ticket));
    }

    // Called by AsyncResultBase::Resolve() on HTTP failure.
    // Reads error_msg populated by the base class Resolve().
    void ApplyError() override {
        error.emplace(std::move(error_msg));
    }

public:
    // Constructed by the client in CallAsync.
    AsyncResult(Orchestrator& orch, size_t tkt, DeserializeFn fn, std::shared_ptr<ResultSync> s)
        : AsyncResultBase{orch, tkt, std::move(s)}
        , deserialize_fn{fn}
    {}

    AsyncResult(AsyncResult const&) = delete;
    AsyncResult(AsyncResult&&) = default;
    AsyncResult& operator=(AsyncResult const&) = delete;
    AsyncResult& operator=(AsyncResult&&) = default;
    ~AsyncResult() override = default;

    // ----- User interface -----

    // State queries — block until resolved.
    bool HasData() { Resolve(); return data.has_value(); }
    bool HasError() { Resolve(); return error.has_value(); }

    // Result access — blocks until resolved, then returns cached result.
    Data_t& Data() { Resolve(); return *data; }
    Error_t& Error() { Resolve(); return *error; }
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


}
