#pragma once

#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace jai::llm {


class Orchestrator;
struct ResultSync;


using Ticket = size_t;


/***
 * TokenUsage — normalized token counts across all providers.
 *
 * Fields that a given provider does not report are left as nullopt.
 * All values are int64_t (providers report integers; any double→int64_t
 * conversion happens in the per-endpoint extraction specializations).
 */
struct TokenUsage {
    std::optional<int64_t> input_tokens{};
    std::optional<int64_t> output_tokens{};
    std::optional<int64_t> total_tokens{};
    std::optional<int64_t> cache_creation_tokens{};
    std::optional<int64_t> cache_read_tokens{};
    std::optional<int64_t> reasoning_tokens{};
    std::optional<int64_t> tool_use_tokens{};
};


/***
 * AttemptOutcome — classification of how an individual HTTP attempt resolved.
 */
enum class AttemptOutcome : uint32_t {
    SUCCESS,
    TRANSPORT_ERROR,
    HTTP_ERROR,
    DESERIALIZATION_ERROR
};


/***
 * AttemptMetadata — per-attempt transport and envelope diagnostics.
 *
 * Transport fields (status_code, duration_us, bytes_*) are always populated.
 * Envelope-extracted fields (usage, model, stop_reason) are populated only
 * on successful deserialization.
 */
struct AttemptMetadata {
    int64_t status_code{-1};
    int64_t duration_us{-1};
    int64_t bytes_downloaded{-1};
    int64_t bytes_uploaded{-1};
    AttemptOutcome outcome{AttemptOutcome::SUCCESS};
    std::string error{};

    // Envelope-extracted (only on successful deserialization)
    std::optional<TokenUsage> usage{};
    std::optional<std::string> model{};
    std::optional<std::string> stop_reason{};
};


/***
 * Result — aggregate result for an LLM API call.
 *
 * Carries the deserialized response data (if successful), an error string
 * (if failed), and a chronological list of per-attempt metadata covering
 * every counted attempt (silent retries like 429 are excluded).
 */
template <typename Endpoint, typename Data = void>
struct Result {
    using Data_t = Data;
    using Response_t = typename Endpoint::Response_t;

    std::optional<Data> data{};
    std::optional<Response_t> response{};
    std::optional<std::string> error{};
    std::vector<AttemptMetadata> attempts{};
};


template <typename Endpoint>
struct Result<Endpoint, void> {
    using Response_t = typename Endpoint::Response_t;

    std::optional<Response_t> data{};
    std::optional<std::string> error{};
    std::vector<AttemptMetadata> attempts{};
};


/***
 * AsyncResultArgs - a structure to be filled with information used to
 *                   construct AsyncResult.
 */
template <typename Endpoint>
struct AsyncResultArgs {
    using Deserialize_f = typename Endpoint::Response_t (*)(Orchestrator*, Ticket);
    using Extract_f = void (*)(const typename Endpoint::Response_t&, AttemptMetadata&);

    Orchestrator* orch;
    Ticket ticket;
    Deserialize_f deserialize_fn;
    Extract_f extract_fn;
    std::shared_ptr<ResultSync> result_sync;
};


/***
 * AsyncResultBase — non-template base for AsyncResult.
 *
 * Owns the wait/retry loop (Resolve). Deserialization, error storage,
 * and envelope metadata extraction are delegated to pure virtual methods
 * overridden by AsyncResult<Endpoint>.
 *
 * Non-copyable, movable.
 */
class AsyncResultBase {
protected:
    // Type-dependent hooks — overridden by AsyncResult<Endpoint>.
    virtual void ApplyDeserialization() = 0;
    virtual void ApplyExtraction(AttemptMetadata&) = 0;
    virtual void ApplyError(std::string error_msg) = 0;
    virtual std::vector<AttemptMetadata>& Attempts() = 0;

    Orchestrator* orchestrator;
    Ticket ticket;
    std::shared_ptr<ResultSync> sync;
    bool resolved;

    // Wait/retry loop
    void Resolve();

    AsyncResultBase(Orchestrator&, Ticket, std::shared_ptr<ResultSync>);

public:
    AsyncResultBase(AsyncResultBase const&) = delete;
    AsyncResultBase(AsyncResultBase&&) = default;
    AsyncResultBase& operator=(AsyncResultBase const&) = delete;
    AsyncResultBase& operator=(AsyncResultBase&&) = default;
    virtual ~AsyncResultBase() = default;

    std::shared_ptr<ResultSync>& SyncBlock() { return sync; }

protected:
    // Non-blocking check. Safe to call from any thread.
    bool IsReadyBase() const;
};


/***
 * AsyncResult — a pure delivery mechanism — blocking pull-based result container.
 */
template <typename Endpoint, typename Data = void>
class AsyncResult : public AsyncResultBase {
public:
    using Response_t = typename Endpoint::Response_t;

    using Deserialize_f = Response_t (*)(Orchestrator*, Ticket);
    using Extract_f = void (*)(const Response_t&, AttemptMetadata&);
    using Transform_f = Data (*)(const Response_t&);

private:
    Result<Endpoint, Data> result{};
    Deserialize_f deserialize_fn{nullptr};
    Extract_f extract_fn{nullptr};
    Transform_f transform_fn{nullptr};

    void ApplyDeserialization() override {
        if constexpr (std::is_void_v<Data>) {
            if (deserialize_fn) {
                result.data.emplace(deserialize_fn(orchestrator, ticket));
            }
        } else {
            if (deserialize_fn) {
                result.response.emplace(deserialize_fn(orchestrator, ticket));
                if (transform_fn) {
                    result.data.emplace(transform_fn(*result.response));
                }
            }
        }
    }

    void ApplyExtraction(AttemptMetadata& am) override {
        if constexpr (std::is_void_v<Data>) {
            if (extract_fn && result.data) {
                extract_fn(*result.data, am);
            }
        } else {
            if (extract_fn && result.response) {
                extract_fn(*result.response, am);
            }
        }
    }

    void ApplyError(std::string error_msg) override {
        result.error.emplace(std::move(error_msg));
    }

    std::vector<AttemptMetadata>& Attempts() override {
        return result.attempts;
    }

public:
    AsyncResult(AsyncResultArgs<Endpoint>&& args, Transform_f tfn)
        : AsyncResultBase{args.orch, args.ticket, std::move(args.result_sync)}
        , deserialize_fn{args.deserialize_fn}
        , extract_fn{args.extract_fn}
        , transform_fn{tfn}
    {}

    AsyncResult(AsyncResult const&) = delete;
    AsyncResult(AsyncResult&&) = default;
    AsyncResult& operator=(AsyncResult const&) = delete;
    AsyncResult& operator=(AsyncResult&&) = default;
    ~AsyncResult() override = default;


    // Non-blocking check. Safe to call from any thread.
    bool IsReady() const { return IsReadyBase(); }

    // Blocking access — resolves on first call, then returns cached result.
    const Result<Endpoint, Data>& Get() {
        Resolve();
        return result;
    }

    // Move the result out — blocks until resolved.
    // After Take(), the internal result is in a moved-from state.
    Result<Endpoint, Data> Take() {
        Resolve();
        return std::move(result);
    }
};


/***
 * CoroResult — a pure delivery mechanism — coroutine-based result container
 *
 * The coroutine starts eagerly and suspends until the result is ready.
 * The caller retrieves the result via co_await or by polling
 * IsReady()/Get() after the event loop completes.
 *
 * The coroutine body (CallCoro) builds and co_returns Result,
 */
template <typename Endpoint, typename Data = void>
class CoroResult {
public:
    class Promise_t;
    using promise_type = Promise_t;

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

        Result<Endpoint, Data> result{};
        std::coroutine_handle<> waiting{};

        CoroResult get_return_object() {
            return CoroResult{coro_handle::from_promise(*this)};
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

        void return_value(Result<Endpoint, Data> val) { result = std::move(val); }

        void unhandled_exception() {
            // All exceptions should be caught in CallCoro and stored
            // on the Result as error information. This is a last resort.
            result.error.emplace("Unhandled exception in coroutine");
        }

        template <typename Awaitable>
        auto&& await_transform(Awaitable&& a) { return std::forward<Awaitable>(a); }
    };

private:
    using coro_handle = typename Promise_t::coro_handle;
    coro_handle handle;

public:
    CoroResult(coro_handle h) : handle{h} {}

    CoroResult(const CoroResult&) = delete;
    CoroResult(CoroResult&& other) noexcept : handle{other.handle} {
        other.handle = nullptr;
    }
    CoroResult& operator=(const CoroResult&) = delete;
    CoroResult& operator=(CoroResult&&) = delete;
    ~CoroResult() { if (handle) { handle.destroy(); } }

    // Allows:  auto result = co_await client.CallCoro(request);
    auto operator co_await() {
        struct Awaiter {
            CoroResult& task;

            bool await_ready() const noexcept { return task.handle.done(); }

            void await_suspend(std::coroutine_handle<> caller) noexcept {
                task.handle.promise().waiting = caller;
            }

            Result<Endpoint, Data> await_resume() {
                return std::move(task.handle.promise().result);
            }
        };
        return Awaiter{*this};
    }
};


}
