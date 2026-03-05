#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <version>


namespace jai::llm {


// Prefer std::move_only_function (C++23) — move-only, SBO-friendly, no copy
// overhead. Falls back to std::function on toolchains that don't provide it yet.
#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
template <typename Sig>
using MoveFunction = std::move_only_function<Sig>;
#else
template <typename Sig>
using MoveFunction = std::function<Sig>;
#endif


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
    using Transform_f = MoveFunction<Data(const Response_t&)>;

private:
    Result<Endpoint, Data> result{};
    Deserialize_f deserialize_fn{nullptr};
    Extract_f extract_fn{nullptr};
    Transform_f transform_fn{};

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
        , transform_fn{std::move(tfn)}
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


}
