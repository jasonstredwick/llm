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
#include <mutex>
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

    // Cumulative token usage across all calls — thread-safe.
    mutable std::mutex usage_mtx{};
    TokenUsage total_usage{};

    std::jthread loop_thread{};
    bool started{false};

    // Set when the event loop thread exits due to an unrecoverable exception.
    // Checked by SubmitRequest and ExecOnce to fail fast instead of hanging.
    std::atomic<bool> dead{false};
    std::string fatal_error{};

    explicit Impl(const Config& cfg)
        : config{cfg}
        , orchestrator{cfg.policy}
    {}
};


}
