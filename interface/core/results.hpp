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
template <typename Endpoint, typename Data>
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
 * TransformResult — apply a user-provided transform to a Tier 1 result.
 *
 * Converts Result<Endpoint, void> into Result<Endpoint, Data> by running
 * the transform function on the response data. The original Response_t is
 * preserved in result.response regardless of whether the transform succeeds.
 * If the transform throws, the error is captured on the result.
 */
template <typename Endpoint, typename Data>
Result<Endpoint, Data> TransformResult(Result<Endpoint, void>&& tier1,
                                        Data (*transform)(const typename Endpoint::Response_t&)) {
    Result<Endpoint, Data> result;
    result.error = std::move(tier1.error);
    result.attempts = std::move(tier1.attempts);
    if (tier1.data) {
        try {
            result.data.emplace(transform(*tier1.data));
        } catch (const std::exception& e) {
            result.error.emplace(std::string{"User transform failed: "} + e.what());
        }
        result.response.emplace(std::move(*tier1.data));
    }
    return result;
}


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

public:
    AsyncResultBase(Orchestrator&, Ticket, std::shared_ptr<ResultSync>);

    AsyncResultBase(AsyncResultBase const&) = delete;
    AsyncResultBase(AsyncResultBase&&) = default;
    AsyncResultBase& operator=(AsyncResultBase const&) = delete;
    AsyncResultBase& operator=(AsyncResultBase&&) = default;
    virtual ~AsyncResultBase() = default;

    std::shared_ptr<ResultSync>& SyncBlock() { return sync; }

    // Non-blocking check. Safe to call from any thread.
    bool IsReadyBase() const;
};


// Forward declaration — two-param template with void default.
template <typename Endpoint, typename Data = void>
class AsyncResult;


/***
 * AsyncResult<Endpoint, void> — blocking pull-based result container (Tier 1).
 *
 * Parameterized on an Endpoint tag type (e.g. anthropic::Messages).
 * Stores a Result<Endpoint, void> aggregate with data, error, and per-attempt
 * metadata. Implements the virtual hooks for deserialization, envelope
 * metadata extraction, and error emplacement.
 *
 * AsyncResult is a pure delivery mechanism — all outcome information
 * (data, error, diagnostics) lives on the Result itself.
 *
 * Non-copyable, movable.
 */
template <typename Endpoint>
class AsyncResult<Endpoint, void> : public AsyncResultBase {
public:
    using Response_t = typename Endpoint::Response_t;

    using DeserializeFn = Response_t (*)(Orchestrator*, Ticket);
    using ExtractFn = void (*)(const Response_t&, AttemptMetadata&);

private:
    Result<Endpoint, void> result{};
    DeserializeFn deserialize_fn{nullptr};
    ExtractFn extract_fn{nullptr};

    void ApplyDeserialization() override {
        result.data.emplace(deserialize_fn(orchestrator, ticket));
    }

    void ApplyExtraction(AttemptMetadata& am) override {
        if (extract_fn && result.data) {
            extract_fn(*result.data, am);
        }
    }

    void ApplyError(std::string error_msg) override {
        result.error.emplace(std::move(error_msg));
    }

    std::vector<AttemptMetadata>& Attempts() override {
        return result.attempts;
    }

public:
    AsyncResult(Orchestrator& orch, Ticket tkt, DeserializeFn fn,
                ExtractFn efn, std::shared_ptr<ResultSync> s)
        : AsyncResultBase{orch, tkt, std::move(s)}
        , deserialize_fn{fn}
        , extract_fn{efn}
    {}

    AsyncResult(AsyncResult const&) = delete;
    AsyncResult(AsyncResult&&) = default;
    AsyncResult& operator=(AsyncResult const&) = delete;
    AsyncResult& operator=(AsyncResult&&) = default;
    ~AsyncResult() override = default;

    // ----- User interface -----

    // Non-blocking check. Safe to call from any thread.
    bool IsReady() const { return IsReadyBase(); }

    // Blocking access — resolves on first call, then returns cached result.
    const Result<Endpoint, void>& Get() {
        Resolve();
        return result;
    }

    // Move the result out — blocks until resolved.
    // After Take(), the internal result is in a moved-from state.
    Result<Endpoint, void> Take() {
        Resolve();
        return std::move(result);
    }
};


/***
 * AsyncResult<Endpoint, Data> — blocking pull-based result container (Tier 2).
 *
 * Wraps an AsyncResult<Endpoint, void> and applies a user-provided transform
 * function to convert Response_t into Data. The original Response_t is
 * preserved in result.response.
 *
 * Does NOT inherit from AsyncResultBase — delegates to the inner Tier 1
 * AsyncResult for all orchestrator interaction.
 *
 * Non-copyable, movable.
 */
template <typename Endpoint, typename Data>
class AsyncResult {
    using Response_t = typename Endpoint::Response_t;

    AsyncResult<Endpoint, void> inner;
    Data (*transform_fn)(const Response_t&);
    Result<Endpoint, Data> result{};
    bool resolved{false};

    void ResolveTransform() {
        if (resolved) { return; }
        result = TransformResult<Endpoint, Data>(inner.Take(), transform_fn);
        resolved = true;
    }

public:
    AsyncResult(AsyncResult<Endpoint, void> inner_, Data (*fn)(const Response_t&))
        : inner{std::move(inner_)}, transform_fn{fn} {}

    AsyncResult(const AsyncResult&) = delete;
    AsyncResult(AsyncResult&&) = default;
    AsyncResult& operator=(const AsyncResult&) = delete;
    AsyncResult& operator=(AsyncResult&&) = default;
    ~AsyncResult() = default;

    // ----- User interface -----

    // Non-blocking check. Safe to call from any thread.
    bool IsReady() const { return inner.IsReady(); }

    // Blocking access — resolves on first call, then returns cached result.
    const Result<Endpoint, Data>& Get() {
        ResolveTransform();
        return result;
    }

    // Move the result out — blocks until resolved.
    // After Take(), the internal result is in a moved-from state.
    Result<Endpoint, Data> Take() {
        ResolveTransform();
        return std::move(result);
    }
};


// Forward declaration — two-param template with void default.
template <typename Endpoint, typename Data = void>
class CoroAsyncResult;


/***
 * CoroAsyncResult<Endpoint, void> — coroutine-based result container (Tier 1).
 *
 * Parameterized on an Endpoint tag type (e.g. anthropic::Messages).
 * The coroutine starts eagerly and suspends until the result is ready.
 * The caller retrieves the result via co_await or by polling
 * IsReady()/Get() after the event loop completes.
 *
 * The coroutine body (CallCoro) builds and co_returns a Result<Endpoint, void>,
 * which includes per-attempt metadata. All outcome information lives on the
 * Result — CoroAsyncResult is a pure delivery mechanism.
 *
 * Non-copyable, move-constructible.
 */
template <typename Endpoint>
class CoroAsyncResult<Endpoint, void> {
public:
    class Promise_t;
    using promise_type = Promise_t;

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

        Result<Endpoint, void> result{};
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

        void return_value(Result<Endpoint, void> val) { result = std::move(val); }

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
    CoroAsyncResult(coro_handle h) : handle{h} {}

    CoroAsyncResult(const CoroAsyncResult&) = delete;
    CoroAsyncResult(CoroAsyncResult&& other) noexcept : handle{other.handle} {
        other.handle = nullptr;
    }
    CoroAsyncResult& operator=(const CoroAsyncResult&) = delete;
    CoroAsyncResult& operator=(CoroAsyncResult&&) = delete;
    ~CoroAsyncResult() { if (handle) { handle.destroy(); } }

    // ----- Polling interface (after RunOnce/RunUntilComplete) -----

    // Non-blocking check. Safe to call from any thread.
    bool IsReady() const { return handle.done(); }

    // Access the result.
    // Caller must ensure IsReady() before calling.
    const Result<Endpoint, void>& Get() const {
        return handle.promise().result;
    }

    // Move the result out.
    // Caller must ensure IsReady() before calling.
    // After Take(), the internal result is in a moved-from state.
    Result<Endpoint, void> Take() {
        return std::move(handle.promise().result);
    }

    // ----- Awaitable interface -----
    // Allows:  Result<Endpoint, void> result = co_await client.CallCoro(request);

    auto operator co_await() {
        struct Awaiter {
            CoroAsyncResult& task;

            bool await_ready() const noexcept { return task.handle.done(); }

            void await_suspend(std::coroutine_handle<> caller) noexcept {
                task.handle.promise().waiting = caller;
            }

            Result<Endpoint, void> await_resume() {
                return std::move(task.handle.promise().result);
            }
        };
        return Awaiter{*this};
    }
};


/***
 * CoroAsyncResult<Endpoint, Data> — coroutine-based result container (Tier 2).
 *
 * Used by the Tier 2 CallCoro overload, which co_awaits the Tier 1
 * CoroAsyncResult<Endpoint, void> and applies a user transform via
 * TransformResult. The result carries both the user Data and the
 * original Response_t.
 *
 * Non-copyable, move-constructible.
 */
template <typename Endpoint, typename Data>
class CoroAsyncResult {
public:
    class Promise_t;
    using promise_type = Promise_t;

    class Promise_t {
    public:
        using coro_handle = std::coroutine_handle<Promise_t>;

        Result<Endpoint, Data> result{};
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

        void return_value(Result<Endpoint, Data> val) { result = std::move(val); }

        void unhandled_exception() {
            result.error.emplace("Unhandled exception in coroutine");
        }

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

    const Result<Endpoint, Data>& Get() const {
        return handle.promise().result;
    }

    Result<Endpoint, Data> Take() {
        return std::move(handle.promise().result);
    }

    // ----- Awaitable interface -----
    // Allows:  Result<Endpoint, Data> result = co_await client.CallCoro(request, &transform);

    auto operator co_await() {
        struct Awaiter {
            CoroAsyncResult& task;

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
