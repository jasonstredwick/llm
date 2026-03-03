/***
 * Orchestrator — managed HTTP transport engine for LLM API calls.
 *
 * Provider-agnostic. Accepts request data (headers, URL, body) and produces curl::Response.
 * Manages concurrency, rate limiting, retry, and connection pooling.
 *
 * Pull-based design: clients submit requests with a shared ResultSync block.
 * The orchestrator drives the curl event loop and signals the ResultSync block
 * on HTTP completion (success or failure). Deserialization happens on the
 * caller's thread via GetResponse; if it fails, the caller requests a retry
 * via RetrySlot. On success the caller calls ReleaseSlot to free resources.
 *
 * Slot management uses intrusive doubly-linked lists. Each slot carries prev/next
 * indices and belongs to exactly one of four lists:
 *   - awaiting:   pending dispatch (new work at back, retries at front) [per-queue]
 *   - active:     in-flight with curl [per-queue]
 *   - completed:  HTTP done, awaiting GetResponse/ReleaseSlot/RetrySlot [per-queue]
 * Plus a global free list for recycled slots.
 *
 * Flow:
 *   1. Client registers once → gets a RegistrationToken
 *   2. Client calls Submit → slot allocated from free list → back of awaiting
 *   3. Orchestrator dispatches: front of awaiting → active (launch curl::Attempt)
 *   4. On HTTP success:      active → completed, signal ResultSync
 *   5. On HTTP retry:        active → front of awaiting (priority over new work)
 *   6. On permanent failure: active → completed, signal ResultSync with error
 *   7. Caller GetResponse:   borrows const ref to Response (slot stays completed)
 *   8. Caller ReleaseSlot:   completed → free (deserialization succeeded)
 *   9. Caller RetrySlot:     completed → front of awaiting (deserialization retry)
 *
 * Backoff is a queue-level concern, not per-slot. The dispatch gate checks
 * rate limit watermarks; individual retries don't carry their own delay.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once

#include "curl.hpp"
#include "http.hpp"
#include "policy.hpp"
#include "results.hpp"

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
        COMPLETED,  // HTTP done, awaiting GetResponse/ReleaseSlot/RetrySlot
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
        // Headers and URL are borrowed from Client (which outlives all Slots).
        // Body is owned because it is unique to each request.
        const std::string* url;
        http::Method method;
        std::vector<std::byte> body;
        AttemptPolicy attempt_policy;
        size_t registration_index{};

        // --- Signaling ---
        std::shared_ptr<ResultSync> sync{};

        // --- Managed by orchestrator ---
        SlotState state{SlotState::FREE};
        size_t retry_count{0};

        // Prepared curl data: constructed once from headers, reused across retries.
        curl::HeaderList header_list;

        // The live attempt. Emplaced when dispatched, reset on completion.
        // Stays alive in COMPLETED state so GetResponse can borrow the Response.
        std::optional<curl::Attempt> attempt{};

        Slot(const http::RequestHeaders& headers,
             const std::string& url_ref,
             http::Method method_,
             std::vector<std::byte> body_,
             AttemptPolicy policy,
             size_t reg_index,
             std::shared_ptr<ResultSync> sync_block)
            : url{&url_ref}
            , method{method_}
            , body{std::move(body_)}
            , attempt_policy{std::move(policy)}
            , registration_index{reg_index}
            , sync{std::move(sync_block)}
            , state{SlotState::AWAITING}
            , header_list{headers}
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
        ListHead completed{};   // HTTP done, awaiting retrieval or retry

        // Adaptive rate limit watermarks (from provider response headers)
        std::optional<int64_t> remaining_requests{};
        std::optional<int64_t> remaining_tokens{};
        std::optional<std::chrono::steady_clock::time_point> reset_time{};

        // Client-side resource pressure backoff.
        // Set when curl::Attempt construction fails (allocation, handle exhaustion).
        // Pauses all dispatches on this queue until the backoff expires.
        // Escalates: 100ms → 200ms → 400ms, capped at 400ms.
        std::optional<std::chrono::steady_clock::time_point> resource_backoff_until{};
        size_t resource_backoff_count{0};
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
    // GetResponse, ReleaseSlot, and RetrySlot. The ResultSync block is
    // signaled on completion. Headers and URL are borrowed from Client
    // (which outlives all Slots); body is moved.
    size_t Submit(RegistrationToken token,
                  const http::RequestHeaders& headers,
                  const std::string& url,
                  http::Method method,
                  std::vector<std::byte> body,
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

    // Block until network activity or timeout. For use by the loop thread.
    int WaitForActivity(int timeout_ms);

    // Interrupt a blocked WaitForActivity() from another thread.
    // Thread-safe relative to WaitForActivity().
    void Wakeup();

    // Signal all non-free, non-completed slots as failed with the given error.
    // Used when the event loop thread encounters an unrecoverable exception
    // so that all waiting callers unblock instead of hanging forever.
    void DrainAll(const std::string& error);

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
    size_t AllocateSlot(const http::RequestHeaders& headers,
                        const std::string& url,
                        http::Method method,
                        std::vector<std::byte> body,
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
