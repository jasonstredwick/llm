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
 * outstanding AsyncResult objects.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "core/call.hpp"
#include "core/error.hpp"
#include "core/policy.hpp"
#include "core/results.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>


namespace jai::llm {


// Forward declarations
struct Impl;
void RunUntilDoneImpl(const std::shared_ptr<ResultSync>&);


// Primary entry point to library.
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

        template <typename T>
        using Transform_f = MoveFunction<T(const Response_t&)>;

    private:
        size_t client_id{0};

        explicit ClientHandle(size_t id) : client_id{id} {}

    public:
        ClientHandle(const ClientHandle&) = default;
        ClientHandle(ClientHandle&&) noexcept = default;
        ClientHandle& operator=(const ClientHandle&) = default;
        ClientHandle& operator=(ClientHandle&&) noexcept = default;
        ~ClientHandle() = default;

        AsyncResult<Endpoint> CallAsync(const Request_t&, const AttemptPolicy& = {}) const;
        Result<Endpoint> CallSync(const Request_t&, const AttemptPolicy& = {}) const;

        template <typename Data>
        AsyncResult<Endpoint, Data> CallAsync(const Request_t&, Transform_f<Data>, const AttemptPolicy& = {}) const;

        template <typename Data>
        Result<Endpoint, Data> CallSync(const Request_t&, Transform_f<Data>, const AttemptPolicy& = {}) const;
    };

private:
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

    template <typename Endpoint, typename Auth>
    ClientHandle<Endpoint> CreateClient(Auth, std::string model, const ClientPolicy& = {});

    //----- Observability -----

    size_t PendingCount() const;
    bool IsRunning() const;
    TokenUsage TotalUsage() const;
};


template <typename Endpoint>
AsyncResult<Endpoint>
Instance::ClientHandle<Endpoint>::CallAsync(
    const Request_t& request,
    const AttemptPolicy& policy) const
{
    return AsyncResult<Endpoint, void>{
        PrepareAsync<Endpoint>(client_id, request, policy),
        nullptr
    };
}


template <typename Endpoint>
Result<Endpoint>
Instance::ClientHandle<Endpoint>::CallSync(
    const Request_t& request,
    const AttemptPolicy& policy) const
{
    auto aresult = AsyncResult<Endpoint, void>{
        PrepareAsync<Endpoint>(client_id, request, policy),
        nullptr
    };
    RunUntilDoneImpl(aresult.SyncBlock());
    return aresult.Take();
}


template <typename Endpoint>
template <typename Data>
AsyncResult<Endpoint, Data>
Instance::ClientHandle<Endpoint>::CallAsync(
    const Request_t& request,
    Transform_f<Data> transform,
    const AttemptPolicy& policy) const
{
    return AsyncResult<Endpoint, Data>{
        PrepareAsync<Endpoint>(client_id, request, policy),
        std::move(transform)
    };
}


template <typename Endpoint>
template <typename Data>
Result<Endpoint, Data>
Instance::ClientHandle<Endpoint>::CallSync(
    const Request_t& request,
    Transform_f<Data> transform,
    const AttemptPolicy& policy) const
{
    auto aresult = AsyncResult<Endpoint, Data>{
        PrepareAsync<Endpoint>(client_id, request, policy),
        std::move(transform)
    };
    RunUntilDoneImpl(aresult.SyncBlock());
    return aresult.Take();
}


template <typename Endpoint, typename Auth>
Instance::ClientHandle<Endpoint> Instance::CreateClient(Auth auth,
                                                        std::string model,
                                                        const ClientPolicy& policy)
{
    try {
        auto id = jai::llm::CreateClientImpl<Endpoint, Auth>(std::move(auth), std::move(model), policy);
        return ClientHandle<Endpoint>{id};
    } catch (const AnnotatedException&) {
        throw;
    } catch (const std::exception& e) {
        throw AnnotatedException{e.what()};
    }
}


}
