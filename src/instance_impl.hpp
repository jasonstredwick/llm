/***
 * Impl — shared internal definition.
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

#include <atomic>
#include <flat_map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <deque>


namespace jai::llm {


struct Impl {
    // ----- Singleton state (process-wide) -----
    static std::atomic<bool> alive;
    static Impl* self;

    // Returns the live Impl* or throws if no Instance exists.
    static Impl* GetImpl() {
        if (!self) {
            throw AnnotatedException{"jai::llm: no Instance exists."};
        }
        return self;
    }

    Instance::Config config;
    Orchestrator orchestrator;

    // Client storage — dedup map + indexed deque.
    // clients_mtx guards all container access: shared for reads,
    // exclusive for creation (FindOrCreateClient).
    std::shared_mutex clients_mtx{};
    std::flat_map<ClientKey, size_t> client_map{};
    std::deque<std::unique_ptr<Client>> clients{};

    // Cumulative token usage across all calls — thread-safe.
    std::mutex usage_mtx{};
    TokenUsage total_usage{};

    // Guards Start/Stop state transitions and started flag.
    std::mutex lifecycle_mtx{};
    std::jthread loop_thread{};
    bool started{false};

    // Set when the event loop thread exits due to an unrecoverable exception.
    // Checked by SubmitRequest and ExecOnce to fail fast instead of hanging.
    std::atomic<bool> dead{false};
    std::string fatal_error{};

    explicit Impl(const Instance::Config& cfg)
        : config{cfg}
        , orchestrator{cfg.policy}
    {}
};


}
