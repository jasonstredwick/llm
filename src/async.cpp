/***
 * async.cpp — bridge functions for Result::Resolve().
 *
 * These non-template functions live here so that Orchestrator and
 * curl::Response are complete types when compiled. The template
 * Result<T>::Resolve() calls these through declared-only signatures
 * in async.hpp.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../interface/core/async.hpp"

#include "orchestrator.hpp"
#include "curl.hpp"

#include <memory>


namespace jai::llm {


const curl::Response& GetResponseRef(Orchestrator* orch, size_t ticket) {
    return orch->GetResponse(ticket);
}


void ReleaseSlotRequest(Orchestrator* orch, size_t ticket) {
    orch->ReleaseSlot(ticket);
}


bool RetrySlotRequest(Orchestrator* orch,
                      size_t ticket,
                      std::shared_ptr<ResultSync> sync) {
    return orch->RetrySlot(ticket, std::move(sync));
}


}
