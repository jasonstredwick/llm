/***
 * Instance::Impl — shared internal definition.
 *
 * Included by instance.cpp and per-endpoint specialization files
 * (src/endpoints/) so they can access the orchestrator and
 * client storage.
 *
 * NOT a public header. Must not be included from interface/.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "../interface/llm.hpp"

#include "client.hpp"
#include "orchestrator.hpp"

#include <flat_map>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>


namespace jai::llm {


struct Instance::Impl {
    Instance::Config config;
    Orchestrator orchestrator;

    // Client storage — dedup map + indexed vector.
    // client_map maps a ClientKey to the client_id (index into clients).
    // clients provides O(1) lookup by client_id at submit time.
    // Protected by clients_mtx for thread-safe creation and access.
    mutable std::shared_mutex clients_mtx{};
    std::flat_map<ClientKey, size_t> client_map{};
    std::vector<std::unique_ptr<Client>> clients{};

    std::jthread loop_thread{};
    bool started{false};

    explicit Impl(const Config& cfg)
        : config{cfg}
        , orchestrator{cfg.policy}
    {}
};


// ----- SubmitResult -----
// Returned by SubmitRequest. Wraps the orchestrator interaction so
// that endpoint code (CallCoro, etc.) never calls Orchestrator directly.
// Forward-declared in client.hpp; defined here where Orchestrator is complete.

struct SubmitResult {
    Orchestrator* orchestrator;
    size_t ticket;
    std::shared_ptr<ResultSync> sync;

    // Borrow the completed HTTP response from the orchestrator.
    const curl::Response& GetResponse() const {
        return orchestrator->GetResponse(ticket);
    }

    // Release the completed slot back to the free list.
    void ReleaseSlot() {
        orchestrator->ReleaseSlot(ticket);
    }

    // Reset the sync block and re-queue for retry.
    // Returns false if the retry budget is exhausted (slot released).
    bool RetrySlot() {
        sync->Reset();
        return orchestrator->RetrySlot(ticket, sync);
    }
};


}
