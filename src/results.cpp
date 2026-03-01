/***
 * results.cpp — AsyncResultBase and SubmitResult implementations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "results.hpp"

#include "curl.hpp"
#include "orchestrator.hpp"

#include <condition_variable>
#include <exception>
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


// ----- AsyncResultBase -----

AsyncResultBase::AsyncResultBase(Orchestrator& orch, Ticket tkt, std::shared_ptr<ResultSync> s)
    : orchestrator{&orch}
    , ticket{tkt}
    , sync{std::move(s)}
    , eptr{}
    , error_msg{}
    , resolved{false}
{}


bool AsyncResultBase::IsDone() const {
    std::lock_guard lock(sync->mtx);
    return sync->ready;
}


void AsyncResultBase::Resolve() {
    if (resolved) { return; }

    while (true) {
        WaitForSync(sync);

        // HTTP-level failure (retries exhausted by orchestrator).
        if (!SyncSucceeded(sync)) {
            error_msg = TakeSyncError(sync);
            ApplyError();
            break;
        }

        // Attempt deserialization via the derived class override.
        // The override borrows the response from the orchestrator
        // and releases the slot on success.
        try {
            ApplyDeserialization();
            break;  // success
        } catch (...) {
            eptr = std::current_exception();
        }

        // Deserialization failed — ask orchestrator to retry.
        sync->Reset();
        if (!orchestrator->RetrySlot(ticket, sync)) {
            // Retry budget exhausted — slot already released by RetrySlot.
            break;
        }
        // Loop back to WaitForSync() for the next attempt.
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
