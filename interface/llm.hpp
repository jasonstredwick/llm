/***
 * Instance — top-level user-facing entry point for the jai::llm library.
 *
 * Provider-agnostic. Include this header for the Instance class, then
 * include the endpoint header(s) you need (e.g., endpoints/anthropic.hpp)
 * to register the provider-specific CreateClient specializations.
 *
 * Two execution modes:
 *   - Manual:   the user drives the event loop by calling ExecOnce() in their
 *               own loop. Single-threaded; no synchronization overhead.
 *   - Threaded: the Instance spawns a dedicated loop thread via Start().
 *               Submissions and result retrieval are synchronized internally.
 *               ExecOnce() is a no-op in this mode.
 *
 * Only one Instance may exist per process. Attempting to construct a second
 * while one is alive will throw.
 *
 * Lifetime: Instance must outlive all ClientHandles created from it and all
 * outstanding AsyncResult / CoroAsyncResult objects.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "core/async.hpp"
#include "core/call.hpp"
#include "core/policy.hpp"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>


namespace jai::llm {


class Instance {
public:
    enum class ThreadingMode {
        MANUAL,   // user calls ExecOnce() in their own loop
        INTERNAL  // Instance spawns and manages the loop thread
    };

    struct Config {
        ThreadingMode threading{ThreadingMode::MANUAL};
        OrchestratorPolicy policy{};
    };

    template <typename Endpoint>
    class ClientHandle {
    public:
        using Request_t = typename Endpoint::Request_t;
        using Response_t = typename Endpoint::Response_t;

    private:
        size_t client_id{0};

        explicit ClientHandle(size_t id) : client_id{id} {}

    public:
        ClientHandle(const ClientHandle&) = default;
        ClientHandle(ClientHandle&&) noexcept = default;
        ClientHandle& operator=(const ClientHandle&) = default;
        ClientHandle& operator=(ClientHandle&&) noexcept = default;
        ~ClientHandle() = default;

        AsyncResult<Response_t> CallAsync(const Request_t& request, const AttemptPolicy& policy = {}) const {
            return jai::llm::CallAsync<Endpoint>(client_id, request, policy);
        }

        CoroAsyncResult<Response_t> CallCoro(const Request_t& request, const AttemptPolicy& policy = {}) const {
            return jai::llm::CallCoro<Endpoint>(client_id, request, policy);
        }

        Response_t CallSync(const Request_t& request, const AttemptPolicy& policy = {}) const {
            return jai::llm::CallSync<Endpoint>(client_id, request, policy);
        }
    };

private:
    static std::atomic<bool> alive;
    static Instance* self;

    struct Impl;
    std::unique_ptr<Impl> impl{nullptr};

public:
    Instance();
    explicit Instance(const Config& config);

    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;
    ~Instance() noexcept;

    //----- Event loop -----

    // Manual mode: drive one iteration of the event loop.
    // Threaded mode: no-op.
    size_t ExecOnce();

    // Threaded mode: start the internal loop thread.
    // Manual mode: throws.
    void Start();

    // Threaded mode: signal the loop thread to stop and join it.
    // Drains in-flight work before returning.
    // Manual mode: no-op.
    void Stop();

    //----- Client factory -----
    // Returns a lightweight ClientHandle parameterized on the endpoint tag.
    // Delegates to the free-function CreateClientImpl (declared in core/call.hpp),
    // which each endpoint translation unit specializes.
    //
    // Endpoint is specified explicitly; Auth is deduced from the argument:
    //   auto client = inst.CreateClient<anthropic::Messages>(auth, model);

    template <typename Endpoint, typename Auth>
    ClientHandle<Endpoint> CreateClient(Auth auth,
                                        std::string model,
                                        const ClientPolicy& policy = {})
    {
        auto id = jai::llm::CreateClientImpl<Endpoint, Auth>(std::move(auth), std::move(model), policy);
        return ClientHandle<Endpoint>{id};
    }

    //----- Observability -----

    size_t PendingCount() const;
    bool IsRunning() const;

private:
    //----- Bridge function access -----
    static Instance* Get();

    // Single friend accessor — returns the Impl* for the live singleton.
    // All bridge functions (FindOrCreateClient, SubmitRequest, etc.) call
    // this instead of needing individual friend declarations.
    friend Impl* GetImpl();
};


}
