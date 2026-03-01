/***
 * Orchestrator — implementation.
 *
 * @author jason.stredwick@gmail.com
 */

#include "orchestrator.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <thread>


namespace jai::llm {


//----- Construction / Destruction -----

Orchestrator::Orchestrator(const OrchestratorPolicy& policy_)
    : policy{policy_}
    , interface{policy_.connection_policy}
{}


Orchestrator::~Orchestrator() noexcept {
    // Reset all in-flight attempts before Interface is destroyed.
    for (auto& slot : slots) {
        slot.attempt.reset();
    }
}


//----- Client registration -----

Orchestrator::RegistrationToken
Orchestrator::Register(const ClientPolicy& client_policy, QueueKey queue_key) {
    size_t index = registrations.size();
    registrations.push_back(ClientRegistration{
        .queue_key = std::move(queue_key),
        .attempt_policy = Merge(policy.attempt_policy, client_policy.attempt_policy),
        .retry_policy = Resolve(policy.retry_policy, client_policy.retry_policy),
        .rate_limit_policy = Resolve(policy.rate_limit_policy, client_policy.rate_limit_policy)
    });

    queues.try_emplace(registrations.back().queue_key);

    return RegistrationToken{index};
}


//----- Submission -----

size_t Orchestrator::Submit(RegistrationToken token,
                            http::Request request,
                            const AttemptPolicy& call_site_policy,
                            std::shared_ptr<ResultSync> sync) {
    const auto& reg = GetClientRegistration(token);

    AttemptPolicy merged_attempt = Merge(reg.attempt_policy, call_site_policy);

    size_t slot_index = AllocateSlot(
        std::move(request),
        std::move(merged_attempt),
        token.index,
        std::move(sync)
    );

    // New submissions go to the back of awaiting.
    auto& queue = queues[reg.queue_key];
    ListPushBack(queue.awaiting, slot_index);
    slots[slot_index].state = SlotState::AWAITING;

    // Attempt immediate dispatch if rate limits permit.
    DispatchFromQueue(queue, reg.rate_limit_policy);

    return slot_index;
}


//----- Caller retrieval (called from user's thread) -----

const curl::Response& Orchestrator::GetResponse(size_t ticket) const {
    assert(ticket < slots.size());
    const auto& slot = slots[ticket];
    assert(slot.state == SlotState::COMPLETED);
    assert(slot.attempt.has_value());

    return slot.attempt->GetResponse();
}


void Orchestrator::ReleaseSlot(size_t ticket) {
    assert(ticket < slots.size());
    auto& slot = slots[ticket];
    assert(slot.state == SlotState::COMPLETED);

    const auto& reg = registrations[slot.registration_index];
    auto& queue = queues[reg.queue_key];

    slot.attempt.reset();
    ListRemove(queue.completed, ticket);

    // Reset and return to free list.
    slot.state = SlotState::FREE;
    slot.retry_count = 0;
    slot.sync.reset();
    ListPushBack(free_list, ticket);
}


bool Orchestrator::RetrySlot(size_t ticket, std::shared_ptr<ResultSync> sync) {
    assert(ticket < slots.size());
    auto& slot = slots[ticket];
    assert(slot.state == SlotState::COMPLETED);

    const auto& reg = registrations[slot.registration_index];
    const auto& retry_policy = reg.retry_policy;
    auto& queue = queues[reg.queue_key];

    // Check retry budget.
    if (slot.retry_count >= retry_policy.max_retries) {
        ReleaseSlot(ticket);
        return false;
    }

    // Re-queue for retry.
    ++slot.retry_count;
    slot.attempt.reset();
    slot.sync = std::move(sync);  // re-attach the (reset) sync block

    ListRemove(queue.completed, ticket);
    slot.state = SlotState::AWAITING;
    ListPushFront(queue.awaiting, ticket);  // priority over new work

    return true;
}


//----- Event loop -----

size_t Orchestrator::RunOnce() {
    // 1. Drive curl.
    auto completed = interface.ExecOnce();

    // 2. Process completed attempts.
    for (auto* attempt_ptr : completed) {
        auto it = attempt_to_slot.find(attempt_ptr);
        if (it == attempt_to_slot.end()) { continue; }

        size_t slot_index = it->second;
        ProcessCompletion(slots[slot_index], slot_index);
    }

    // 3. Dispatch newly eligible work across all queues.
    for (auto&& [key, queue] : queues) {
        for (const auto& reg : registrations) {
            if (reg.queue_key == key) {
                DispatchFromQueue(queue, reg.rate_limit_policy);
                break;
            }
        }
    }

    return PendingCount();
}


void Orchestrator::RunUntilComplete() {
    while (PendingCount() > 0) {
        RunOnce();
        if (PendingCount() > 0) {
            WaitForActivity(ActiveCount() > 0 ? 50 : 1);
        }
    }
}


int Orchestrator::WaitForActivity(int timeout_ms) {
    return interface.WaitForActivity(timeout_ms);
}


void Orchestrator::Wakeup() {
    interface.Wakeup();
}


//----- Observability -----

size_t Orchestrator::AwaitingCount() const {
    size_t total = 0;
    for (auto&& [key, queue] : queues) {
        total += queue.awaiting.count;
    }
    return total;
}


size_t Orchestrator::ActiveCount() const {
    size_t total = 0;
    for (auto&& [key, queue] : queues) {
        total += queue.active.count;
    }
    return total;
}


size_t Orchestrator::CompletedCount() const {
    size_t total = 0;
    for (auto&& [key, queue] : queues) {
        total += queue.completed.count;
    }
    return total;
}


size_t Orchestrator::PendingCount() const {
    return AwaitingCount() + ActiveCount();
}


//----- Intrusive list operations -----

void Orchestrator::ListPushBack(ListHead& list, size_t index) {
    auto& slot = slots[index];
    slot.prev = list.tail;
    slot.next = NONE;

    if (list.tail != NONE) {
        slots[list.tail].next = index;
    } else {
        list.head = index;
    }
    list.tail = index;
    ++list.count;
}


void Orchestrator::ListPushFront(ListHead& list, size_t index) {
    auto& slot = slots[index];
    slot.prev = NONE;
    slot.next = list.head;

    if (list.head != NONE) {
        slots[list.head].prev = index;
    } else {
        list.tail = index;
    }
    list.head = index;
    ++list.count;
}


size_t Orchestrator::ListPopFront(ListHead& list) {
    assert(!list.Empty());
    size_t index = list.head;
    auto& slot = slots[index];

    list.head = slot.next;
    if (list.head != NONE) {
        slots[list.head].prev = NONE;
    } else {
        list.tail = NONE;
    }

    slot.prev = NONE;
    slot.next = NONE;
    --list.count;
    return index;
}


void Orchestrator::ListRemove(ListHead& list, size_t index) {
    auto& slot = slots[index];

    if (slot.prev != NONE) {
        slots[slot.prev].next = slot.next;
    } else {
        list.head = slot.next;
    }

    if (slot.next != NONE) {
        slots[slot.next].prev = slot.prev;
    } else {
        list.tail = slot.prev;
    }

    slot.prev = NONE;
    slot.next = NONE;
    --list.count;
}


//----- Internal: registration access -----

const Orchestrator::ClientRegistration&
Orchestrator::GetClientRegistration(RegistrationToken token) const {
    assert(token.index < registrations.size());
    return registrations[token.index];
}


//----- Internal: slot allocation -----

size_t Orchestrator::AllocateSlot(http::Request request,
                                   AttemptPolicy policy,
                                   size_t reg_index,
                                   std::shared_ptr<ResultSync> sync) {
    if (!free_list.Empty()) {
        size_t index = ListPopFront(free_list);
        slots[index].~Slot();
        new (&slots[index]) Slot(
            std::move(request),
            std::move(policy),
            reg_index,
            std::move(sync)
        );
        return index;
    }

    size_t index = slots.size();
    slots.emplace_back(
        std::move(request),
        std::move(policy),
        reg_index,
        std::move(sync)
    );
    return index;
}


//----- Internal: dispatch -----

void Orchestrator::DispatchFromQueue(QueueState& queue,
                                      const ResolvedRateLimitPolicy& rl_policy) {
    while (!queue.awaiting.Empty()) {
        // Concurrency gate
        if (queue.active.count >= rl_policy.initial_max_concurrent) {
            break;
        }

        // Rate limit gate
        if (rl_policy.use_provider_headers && queue.remaining_requests.has_value()) {
            if (*queue.remaining_requests <= static_cast<int64_t>(rl_policy.min_remaining_before_backoff)) {
                if (queue.reset_time.has_value() &&
                    std::chrono::steady_clock::now() < *queue.reset_time) {
                    break;
                }
                queue.remaining_requests.reset();
                queue.reset_time.reset();
            }
        }

        size_t slot_index = ListPopFront(queue.awaiting);
        auto& slot = slots[slot_index];

        LaunchAttempt(slot, slot_index);

        slot.state = SlotState::ACTIVE;
        ListPushBack(queue.active, slot_index);
    }
}


void Orchestrator::LaunchAttempt(Slot& slot, size_t slot_index) {
    slot.attempt.emplace(
        interface,
        slot.attempt_policy,
        slot.request.method,
        slot.request.url,
        slot.header_list,
        slot.request.body
    );
    attempt_to_slot[&(*slot.attempt)] = slot_index;
}


//----- Internal: completion processing (retry boundary) -----

void Orchestrator::ProcessCompletion(Slot& slot, size_t slot_index) {
    const auto& reg = registrations[slot.registration_index];
    const auto& retry_policy = reg.retry_policy;

    auto& queue = queues[reg.queue_key];

    // Remove from active list and lookup map.
    ListRemove(queue.active, slot_index);
    attempt_to_slot.erase(&(*slot.attempt));

    assert(slot.attempt.has_value());
    const auto& response = slot.attempt->GetResponse();

    UpdateRateLimits(queue, response);

    // --- Transport-level failure ---
    if (slot.attempt->IsFailed()) {
        if (Requeue(slot, slot_index)) {
            slot.attempt.reset();
            return;
        }
        std::string error = "Transport error: " + slot.attempt->GetErrorMessage();
        slot.attempt.reset();
        slot.state = SlotState::COMPLETED;
        ListPushBack(queue.completed, slot_index);
        slot.sync->Signal(false, std::move(error));
        return;
    }

    int64_t status = response.status_code;

    // --- 429: rate-limit re-queue, does NOT count against retry limit ---
    if (status == 429) {
        slot.attempt.reset();
        RequeueRateLimit(slot, slot_index);
        return;
    }

    // --- Retryable HTTP status (5xx): counts against retry limit ---
    if (IsRetryable(status, retry_policy)) {
        if (Requeue(slot, slot_index)) {
            slot.attempt.reset();
            return;
        }
        std::string error = "HTTP " + std::to_string(status) + " after "
                          + std::to_string(slot.retry_count) + " retries";
        slot.attempt.reset();
        slot.state = SlotState::COMPLETED;
        ListPushBack(queue.completed, slot_index);
        slot.sync->Signal(false, std::move(error));
        return;
    }

    // --- Non-retryable HTTP error ---
    if (status >= 400) {
        std::string error = "HTTP " + std::to_string(status);
        slot.attempt.reset();
        slot.state = SlotState::COMPLETED;
        ListPushBack(queue.completed, slot_index);
        slot.sync->Signal(false, std::move(error));
        return;
    }

    // --- HTTP success: move to completed, signal caller. ---
    // The attempt (and its curl::Response) stays alive so the caller
    // can borrow the response via GetResponse on their thread.
    slot.state = SlotState::COMPLETED;
    ListPushBack(queue.completed, slot_index);
    slot.sync->Signal(true);
}


//----- Internal: re-queue for retry -----

bool Orchestrator::Requeue(Slot& slot, size_t slot_index) {
    const auto& reg = registrations[slot.registration_index];
    const auto& retry_policy = reg.retry_policy;

    if (slot.retry_count >= retry_policy.max_retries) {
        return false;
    }

    ++slot.retry_count;
    slot.state = SlotState::AWAITING;

    // Retries go to front of awaiting — priority over new submissions.
    auto& queue = queues[reg.queue_key];
    ListPushFront(queue.awaiting, slot_index);

    return true;
}


void Orchestrator::RequeueRateLimit(Slot& slot, size_t slot_index) {
    const auto& reg = registrations[slot.registration_index];

    // Does NOT increment retry_count — this is a rate-limit signal, not a fault.
    slot.state = SlotState::AWAITING;

    // Also goes to front of awaiting — it was already dispatched and deserves
    // priority, plus the queue's rate limit gate will hold it until the window resets.
    auto& queue = queues[reg.queue_key];
    ListPushFront(queue.awaiting, slot_index);
}




//----- Internal: rate limit tracking -----

void Orchestrator::UpdateRateLimits(QueueState& queue, const curl::Response& response) {
    // TODO: Parse rate limit headers from response.
    // OpenAI:    x-ratelimit-remaining-requests, x-ratelimit-reset-requests
    // Anthropic: anthropic-ratelimit-requests-remaining
    // Gemini:    no rate limit headers — relies on 429 + Retry-After
    (void)queue;
    (void)response;
}


//----- Internal: retry policy helpers -----

bool Orchestrator::IsRetryable(int64_t status_code,
                                const ResolvedRetryPolicy& retry_policy) {
    const auto& codes = retry_policy.retryable_status_codes;
    return std::find(codes.begin(), codes.end(), status_code) != codes.end();
}


}
