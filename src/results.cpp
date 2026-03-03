/***
 * results.cpp — AsyncResultBase and SubmitResult implementations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "results.hpp"

#include "client.hpp"
#include "curl.hpp"
#include "orchestrator.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>


namespace jai::llm {


namespace {

void WaitForSync(std::shared_ptr<ResultSync> const& sync) {
    std::unique_lock lock(sync->mtx);
    sync->cv.wait(lock, [&sync] { return sync->ready; });
}


bool SyncSucceeded(std::shared_ptr<ResultSync> const& sync) {
    std::lock_guard lock(sync->mtx);
    return sync->succeeded;
}


std::string TakeSyncError(std::shared_ptr<ResultSync> const& sync) {
    std::lock_guard lock(sync->mtx);
    return std::move(sync->error_msg);
}


} // anonymous namespace


// ----- BuildTransportMetadata -----

AttemptMetadata BuildTransportMetadata(const curl::Response& resp) {
    return AttemptMetadata{
        .status_code = resp.status_code,
        .duration_us = resp.total_time_us,
        .bytes_downloaded = resp.total_wire_bytes_downloaded,
        .bytes_uploaded = resp.total_wire_bytes_uploaded,
        .outcome = AttemptOutcome::SUCCESS,  // caller overrides if needed
        .error = {}
    };
}


// ----- AsyncResultBase -----

AsyncResultBase::AsyncResultBase(Orchestrator& orch, Ticket tkt, std::shared_ptr<ResultSync> s)
    : orchestrator{&orch}
    , ticket{tkt}
    , sync{std::move(s)}
    , resolved{false}
{}


bool AsyncResultBase::IsReadyBase() const {
    std::lock_guard lock(sync->mtx);
    return sync->ready;
}


void AsyncResultBase::Resolve() {
    if (resolved) { return; }

    while (true) {
        WaitForSync(sync);

        // Extract transport metadata from the completed response.
        // The attempt is kept alive by the orchestrator for all signal paths
        // (success AND failure) so GetResponse() is valid here.
        const auto& response = orchestrator->GetResponse(ticket);
        auto am = BuildTransportMetadata(response);

        // --- Failure signal (transport error, retryable 5xx, non-retryable 4xx) ---
        if (!SyncSucceeded(sync)) {
            am.error = TakeSyncError(sync);
            am.outcome = (response.state == curl::Response::State::FAILED)
                ? AttemptOutcome::TRANSPORT_ERROR
                : AttemptOutcome::HTTP_ERROR;
            Attempts().push_back(std::move(am));

            // Try caller-driven retry (checks budget internally).
            if (!orchestrator->RetrySlot(ticket, sync)) {
                // Retry budget exhausted or non-retryable — set final error.
                ApplyError(Attempts().back().error);
                break;
            }
            // Loop back to WaitForSync() for the next attempt.
            continue;
        }

        // --- Success signal: attempt deserialization ---
        try {
            ApplyDeserialization();

            // Extract envelope metadata (usage, model, stop_reason)
            // via the derived class override.
            am.outcome = AttemptOutcome::SUCCESS;
            ApplyExtraction(am);

            // Accumulate token usage into Instance totals.
            if (am.usage) {
                AccumulateUsage(*am.usage);
            }

            Attempts().push_back(std::move(am));

            orchestrator->ReleaseSlot(ticket);
            break;  // success
        } catch (const std::exception& e) {
            am.outcome = AttemptOutcome::DESERIALIZATION_ERROR;
            am.error = e.what();
            Attempts().push_back(std::move(am));

            // Deserialization failed — ask orchestrator to retry.
            if (!orchestrator->RetrySlot(ticket, sync)) {
                // Retry budget exhausted — set final error from last attempt.
                ApplyError(Attempts().back().error);
                break;
            }
            // Loop back to WaitForSync() for the next attempt.
        }
    }

    resolved = true;
}


// ----- SubmitResult -----

const curl::Response& SubmitResult::GetResponse() const {
    return orchestrator->GetResponse(ticket);
}


void SubmitResult::ReleaseSlot() {
    orchestrator->ReleaseSlot(ticket);
}


bool SubmitResult::RetrySlot() {
    sync->Reset();
    return orchestrator->RetrySlot(ticket, sync);
}


}
