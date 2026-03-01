/***
 * async.cpp — AsyncResultBase::Resolve() and bridge functions.
 *
 * The wait/retry loop lives here so that Orchestrator, curl::Response,
 * and ResultSync are complete types when compiled. The public header
 * (async.hpp) only forward-declares ResultSync and never references
 * curl::Response.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../interface/core/async.hpp"

#include "orchestrator.hpp"
#include "curl.hpp"
#include "sync.hpp"

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

void ResetSync(std::shared_ptr<ResultSync> const& sync) {
    std::lock_guard lock(sync->mtx);
    sync->ready = false;
    sync->succeeded = false;
    sync->error_msg.clear();
}

} // anonymous namespace


// ----- Public bridge functions (declared in async.hpp) -----

std::shared_ptr<ResultSync> MakeResultSync() {
    return std::make_shared<ResultSync>();
}


bool IsSyncReady(std::shared_ptr<ResultSync> const& sync) {
    std::lock_guard lock(sync->mtx);
    return sync->ready;
}


// ----- AsyncResultBase::Resolve() -----

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
        ResetSync(sync);
        if (!orchestrator->RetrySlot(ticket, sync)) {
            // Retry budget exhausted — slot already released by RetrySlot.
            break;
        }
        // Loop back to WaitForSync() for the next attempt.
    }

    resolved = true;
}


}
