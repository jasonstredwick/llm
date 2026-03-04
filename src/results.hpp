#pragma once

#include "../interface/core/results.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>


namespace jai::llm {


class Orchestrator;

namespace curl { struct Response; }


// Build transport-level AttemptMetadata from a completed curl::Response.
// Defined in results.cpp. Caller overrides outcome and error as needed.
AttemptMetadata BuildTransportMetadata(const curl::Response& resp);


/***
 * ResultSync — shared signaling block between the orchestrator and AsyncResult.
 *
 * Heap-allocated (via shared_ptr) so the address remains stable across
 * AsyncResult moves. The orchestrator holds a shared_ptr and signals readiness
 * by setting ready/succeeded/error_msg and notifying the cv. The AsyncResult
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
    // Notifies the condition variable so that AsyncResult's blocking Resolve()
    // can proceed.
    void Signal(bool success, std::string err = {}) {
        {
            std::lock_guard lock(mtx);
            succeeded = success;
            error_msg = std::move(err);
            ready = true;
        }
        cv.notify_one();
    }

    // Reset for retry — called by AsyncResult after deserialization failure.
    void Reset() {
        std::lock_guard lock(mtx);
        ready = false;
        succeeded = false;
        error_msg.clear();
    }
};


/***
 * SubmitResult - Returned by SubmitRequest.
 * Wraps the orchestrator interaction so that endpoint code never calls
 * Orchestrator directly. Forward-declared in client.hpp; defined here
 * where Orchestrator is complete.
 */
struct SubmitResult {
    Orchestrator* orchestrator;
    size_t ticket;
    std::shared_ptr<ResultSync> sync;

    // Borrow the completed HTTP response from the orchestrator.
    const curl::Response& GetResponse() const;

    // Release the completed slot back to the free list.
    void ReleaseSlot();

    // Reset the sync block and re-queue for retry.
    // Returns false if the retry budget is exhausted (slot released).
    bool RetrySlot();
};


}
