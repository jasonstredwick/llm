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
struct ResultSync;


using Ticket = size_t;


/***
 * AsyncResultBase — non-template base for AsyncResult.
 *
 * Owns the wait/retry loop (Resolve). Deserialization and error storage
 * are delegated to pure virtual methods overridden by AsyncResult<T, E>.
 *
 * Non-copyable, movable.
 */
class AsyncResultBase {
protected:
    // Type-dependent hooks — overridden by AsyncResult<T, E>.
    virtual void ApplyDeserialization() = 0;
    virtual void ApplyError() = 0;

    Orchestrator* orchestrator;
    Ticket ticket;
    std::shared_ptr<ResultSync> sync;
    std::exception_ptr eptr;
    std::string error_msg; // populated by Resolve() before calling ApplyError()
    bool resolved;

    // Wait/retry loop
    void Resolve();

public:
    AsyncResultBase(Orchestrator&, Ticket, std::shared_ptr<ResultSync>);

    AsyncResultBase(AsyncResultBase const&) = delete;
    AsyncResultBase(AsyncResultBase&&) = default;
    AsyncResultBase& operator=(AsyncResultBase const&) = delete;
    AsyncResultBase& operator=(AsyncResultBase&&) = default;
    virtual ~AsyncResultBase() = default;

    std::shared_ptr<ResultSync>& SyncBlock() { return sync; }

    // Non-blocking check. Safe to call from any thread.
    bool IsDone() const;

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
 * Adds typed data/error storage to AsyncResultBase and implements the
 * virtual hooks for deserialization and error emplacement.
 *
 * Non-copyable, movable.
 */
template <typename T, typename E = std::string>
class AsyncResult : public AsyncResultBase {
public:
    using Data_t = T;
    using Error_t = E;

    using DeserializeFn = Data_t (*)(Orchestrator*, Ticket);

private:
    std::optional<Data_t> data{};
    std::optional<Error_t> error{};
    DeserializeFn deserialize_fn{nullptr};

    void ApplyDeserialization() override {
        data.emplace(deserialize_fn(orchestrator, ticket));
    }

    void ApplyError() override {
        error.emplace(std::move(error_msg));
    }

public:
    AsyncResult(Orchestrator& orch, Ticket tkt, DeserializeFn fn, std::shared_ptr<ResultSync> s)
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
 * The coroutine starts eagerly and suspends until the result is ready.
 * The caller retrieves the result via co_await or by polling
 * IsReady()/Data() after the event loop completes.
 *
 * Non-copyable, move-constructible.
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
        std::coroutine_handle<> waiting{};

        CoroAsyncResult get_return_object() {
            return CoroAsyncResult{coro_handle::from_promise(*this)};
        }

        static auto initial_suspend() noexcept { return std::suspend_never{}; }

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
