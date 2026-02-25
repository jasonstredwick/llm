/***
 * Orchestrator — managed HTTP transport engine for LLM API calls.
 *
 * Provider-agnostic. Operates on http::Request → curl::Response.
 * Manages concurrency, rate limiting, retry, and connection pooling.
 *
 * Pull-based design: clients submit requests with a shared ResultSync block.
 * The orchestrator drives the curl event loop and signals the ResultSync block
 * on HTTP completion (success or failure). Deserialization happens on the
 * caller's thread via Checkout; if it fails, the caller requests a retry
 * via RetrySlot.
 *
 * Slot management uses intrusive doubly-linked lists. Each slot carries prev/next
 * indices and belongs to exactly one of four lists:
 *   - awaiting:   pending dispatch (new work at back, retries at front) [per-queue]
 *   - active:     in-flight with curl [per-queue]
 *   - completed:  HTTP done, awaiting Checkout or RetrySlot from caller [per-queue]
 * Plus a global free list for recycled slots.
 *
 * Flow:
 *   1. Client registers once → gets a RegistrationToken
 *   2. Client calls Submit → slot allocated from free list → back of awaiting
 *   3. Orchestrator dispatches: front of awaiting → active (launch curl::Attempt)
 *   4. On HTTP success:      active → completed, signal ResultSync
 *   5. On HTTP retry:        active → front of awaiting (priority over new work)
 *   6. On permanent failure: active → completed, signal ResultSync with error
 *   7. Caller Checkout:      completed → free (moves curl::Response out)
 *   8. Caller RetrySlot:     completed → front of awaiting (deserialization retry)
 *
 * Backoff is a queue-level concern, not per-slot. The dispatch gate checks
 * rate limit watermarks; individual retries don't carry their own delay.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once

#include "../interface/clients/policy.hpp"
#include "../interface/core/async.hpp"
#include "curl.hpp"
#include "http.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <flat_map>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>


namespace jai::llm {


// Queue identity — requests sharing a queue share rate limit state.
// Keyed on (auth, endpoint, model) because providers scope rate limits
// per-model or per-model-family. The client normalizes model_group
// appropriately (e.g., "opus" for Claude Opus 4.5/4.6 which share limits).
struct QueueKey {
    std::string auth_identity{};   // API key or credential identifier
    std::string endpoint_url{};    // base endpoint URL
    std::string model_group{};     // rate limit group (model or model family)

    friend auto operator<=>(const QueueKey&, const QueueKey&) = default;
    friend bool operator==(const QueueKey&, const QueueKey&) = default;
};


class Orchestrator {
public:
    struct RegistrationToken {
        size_t index{};

        friend auto operator<=>(const RegistrationToken&, const RegistrationToken&) = default;
        friend bool operator==(const RegistrationToken&, const RegistrationToken&) = default;
    };

private:
    static constexpr size_t NONE = std::numeric_limits<size_t>::max();

    enum class SlotState : uint32_t {
        FREE,       // in the global free list
        AWAITING,   // in a queue's awaiting list (pending dispatch)
        ACTIVE,     // in a queue's active list (curl::Attempt in-flight)
        COMPLETED,  // HTTP done, awaiting Checkout or RetrySlot from caller
    };

    struct ClientRegistration {
        QueueKey queue_key;
        AttemptPolicy attempt_policy;
        ResolvedRetryPolicy retry_policy;
        ResolvedRateLimitPolicy rate_limit_policy;
    };

    // Per-request state managed by the orchestrator.
    // Lives in stable storage (vector, never resized down). Pinned once an
    // Attempt is constructed because libcurl locks the this pointer.
    struct Slot {
        // --- Intrusive list linkage ---
        size_t prev{NONE};
        size_t next{NONE};

        // --- Provided at Submit ---
        http::Request request;
        AttemptPolicy attempt_policy;
        size_t registration_index{};

        // --- Signaling ---
        std::shared_ptr<ResultSync> sync{};

        // --- Managed by orchestrator ---
        SlotState state{SlotState::FREE};
        size_t retry_count{0};

        // Prepared curl data: constructed once from request, reused across retries.
        curl::HeaderList header_list;

        // The live attempt. Emplaced when dispatched, reset on completion.
        // Stays alive in COMPLETED state so Checkout can move the Response out.
        std::optional<curl::Attempt> attempt{};

        Slot(http::Request req,
             AttemptPolicy policy,
             size_t reg_index,
             std::shared_ptr<ResultSync> sync_block)
            : request{std::move(req)}
            , attempt_policy{std::move(policy)}
            , registration_index{reg_index}
            , sync{std::move(sync_block)}
            , state{SlotState::AWAITING}
            , header_list{request.headers}  // must follow request in member decl order
        {}
    };

    // Intrusive doubly-linked list head/tail. O(1) push_front, push_back, remove.
    struct ListHead {
        size_t head{NONE};
        size_t tail{NONE};
        size_t count{0};

        bool Empty() const { return head == NONE; }
    };

    // Per-queue state.
    struct QueueState {
        ListHead awaiting{};    // pending dispatch
        ListHead active{};      // in-flight with curl
        ListHead completed{};   // HTTP done, awaiting Checkout or RetrySlot

        // Adaptive rate limit watermarks (from provider response headers)
        std::optional<int64_t> remaining_requests{};
        std::optional<int64_t> remaining_tokens{};
        std::optional<std::chrono::steady_clock::time_point> reset_time{};
    };

private:
    OrchestratorPolicy policy{};

    curl::Interface interface;

    std::vector<ClientRegistration> registrations{};

    // Slot pool: stable storage. Grows as needed, never shrinks.
    // deque because Slot is non-movable (curl::Attempt deletes move ctor
    // since libcurl locks the this pointer). deque never relocates elements.
    std::deque<Slot> slots{};
    ListHead free_list{};

    std::flat_map<QueueKey, QueueState> queues{};
    std::flat_map<const curl::Attempt*, size_t> attempt_to_slot{};

public:
    explicit Orchestrator(const OrchestratorPolicy& policy);

    Orchestrator() = delete;
    Orchestrator(const Orchestrator&) = delete;
    Orchestrator(Orchestrator&&) = delete;
    Orchestrator& operator=(const Orchestrator&) = delete;
    Orchestrator& operator=(Orchestrator&&) = delete;
    ~Orchestrator() noexcept;

    //----- Client registration -----

    RegistrationToken Register(const ClientPolicy& client_policy, QueueKey queue_key);

    //----- Submission -----

    // Submit returns a ticket (slot index) that the caller uses with
    // Checkout and RetrySlot. The ResultSync block is signaled on completion.
    size_t Submit(RegistrationToken token,
                  http::Request request,
                  const AttemptPolicy& call_site_policy,
                  std::shared_ptr<ResultSync> sync);

    //----- Caller retrieval (called from user's thread) -----

    // Borrow the response from a completed slot. The slot stays in COMPLETED
    // state and the Attempt continues to own the Response. The reference is
    // valid until ReleaseSlot or RetrySlot is called for this ticket.
    const curl::Response& GetResponse(size_t ticket) const;

    // Release a completed slot back to the free list.
    // The slot transitions: COMPLETED → FREE.
    void ReleaseSlot(size_t ticket);

    // Re-queue a completed slot for retry (deserialization failure).
    // Returns false if the retry budget is exhausted (slot is released).
    // The slot transitions: COMPLETED → AWAITING (front, priority) on success,
    //                       COMPLETED → FREE on exhaustion.
    bool RetrySlot(size_t ticket, std::shared_ptr<ResultSync> sync);

    //----- Event loop -----

    size_t RunOnce();
    void RunUntilComplete();

    //----- Observability -----

    size_t AwaitingCount() const;
    size_t ActiveCount() const;
    size_t CompletedCount() const;
    size_t PendingCount() const;

private:
    //----- Intrusive list operations -----

    void ListPushBack(ListHead& list, size_t index);
    void ListPushFront(ListHead& list, size_t index);
    size_t ListPopFront(ListHead& list);
    void ListRemove(ListHead& list, size_t index);

    //----- Internal operations -----

    const ClientRegistration& GetClientRegistration(RegistrationToken token) const;

    // Allocate a slot: take from free list or grow the pool.
    size_t AllocateSlot(http::Request request,
                        AttemptPolicy policy,
                        size_t reg_index,
                        std::shared_ptr<ResultSync> sync);

    void DispatchFromQueue(QueueState& queue, const ResolvedRateLimitPolicy& rl_policy);
    void LaunchAttempt(Slot& slot, size_t slot_index);
    void ProcessCompletion(Slot& slot, size_t slot_index);

    // Re-queue for retry. Increments retry_count. Returns false if exhausted.
    bool Requeue(Slot& slot, size_t slot_index);

    // Re-queue for rate-limit retry. Does NOT increment retry_count.
    void RequeueRateLimit(Slot& slot, size_t slot_index);

    void UpdateRateLimits(QueueState& queue, const curl::Response& response);

    static bool IsRetryable(int64_t status_code, const ResolvedRetryPolicy& retry_policy);
};


}
