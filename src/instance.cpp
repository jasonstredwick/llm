/***
 * Instance — core implementation.
 *
 * Construction, destruction, event loop, and observability.
 * Client factory and dispatch specializations live in per-endpoint
 * translation units (src/endpoints/).
 *
 * @author jason.stredwick@gmail.com
 */

#include "core/error.hpp"
#include "instance_impl.hpp"

#include <thread>


namespace jai::llm {


// ----- Static singleton state -----

std::atomic<bool> Instance::alive{false};
Instance* Instance::self{nullptr};

Instance* Instance::Get() { return self; }


// ----- Construction / Destruction -----

Instance::Instance() : Instance(Config{}) {}


Instance::Instance(const Config& config) {
    bool expected = false;
    if (!alive.compare_exchange_strong(expected, true)) {
        throw AnnotatedException{
            "jai::llm::Instance: only one Instance may exist per process."
        };
    }

    try {
        impl = std::make_unique<Impl>(config);
    } catch (const AnnotatedException&) {
        alive.store(false);
        throw;
    } catch (const std::exception& e) {
        alive.store(false);
        throw AnnotatedException{e.what()};
    }

    self = this;
}


Instance::~Instance() noexcept {
    self = nullptr;
    try {
        Stop();
        impl.reset();
        alive.store(false);
    } catch (const std::exception&) {}
}


// ----- Event loop -----

size_t Instance::ExecOnce() {
    if (!self || !impl) { return 0; }
    if (impl->config.threading == ThreadingMode::INTERNAL) {
        return 0;  // no-op in threaded mode
    }
    if (impl->dead.load(std::memory_order_acquire)) {
        throw FatalInstanceError{
            "jai::llm::Instance::ExecOnce: instance is dead.",
            impl->fatal_error
        };
    }
    try {
        return impl->orchestrator.RunOnce();
    } catch (const std::exception& e) {
        // Unrecoverable: drain all waiting callers and mark instance dead.
        std::string error = std::string{"Event loop fatal: "} + e.what();
        impl->orchestrator.DrainAll(error);
        impl->fatal_error = error;
        impl->dead.store(true, std::memory_order_release);
        throw FatalInstanceError{error};
    }
}


void Instance::Start() {
    if (!self || !impl) {
        throw AnnotatedException{
            "jai::llm::Instance::Start(): Instance is not available."
        };
    }
    if (impl->config.threading != ThreadingMode::INTERNAL) {
        throw AnnotatedException{
            "jai::llm::Instance::Start(): only valid in INTERNAL threading mode."
        };
    }
    if (impl->started) { return; }

    try {
        impl->started = true;
        impl->loop_thread = std::jthread{[this](std::stop_token token) {
            try {
                while (!token.stop_requested()) {
                    impl->orchestrator.RunOnce();
                    // Block until network activity or new submissions (via Wakeup).
                    // Timeout ensures periodic dispatch of queued work even without
                    // network events. Short enough to feel responsive, long enough
                    // to avoid busy-waiting.
                    impl->orchestrator.WaitForActivity(50);
                }
            } catch (const std::exception& e) {
                // Unrecoverable: the orchestrator or curl multi handle is broken.
                // Signal all waiting callers so they unblock with an error
                // instead of hanging, then mark the instance as dead so new
                // submissions fail immediately.
                std::string error = std::string{"Event loop fatal: "} + e.what();
                impl->orchestrator.DrainAll(error);

                // Brief cooldown before marking dead. Gives the system time
                // to reclaim resources (memory, file descriptors) before the
                // caller sees FatalInstanceError and attempts to rebuild.
                std::this_thread::sleep_for(std::chrono::milliseconds{500});

                impl->fatal_error = std::move(error);
                impl->dead.store(true, std::memory_order_release);
            }
        }};
    } catch (const AnnotatedException&) {
        impl->started = false;
        throw;
    } catch (const std::exception& e) {
        impl->started = false;
        throw AnnotatedException{e.what()};
    }
}


void Instance::Stop() {
    if (!impl || !impl->started) { return; }
    impl->loop_thread.request_stop();
    // Wake the loop thread if it's blocked in WaitForActivity so it
    // can observe the stop request promptly.
    impl->orchestrator.Wakeup();
    if (impl->loop_thread.joinable()) {
        impl->loop_thread.join();
    }
    impl->started = false;
}


// ----- Impl accessor -----
// Single friend of Instance. All bridge functions go through this
// instead of needing individual friend declarations in llm.hpp.

Instance::Impl* GetImpl() {
    Instance* instance = Instance::Get();
    if (!instance) {
        throw AnnotatedException{
            "jai::llm: no Instance exists."
        };
    }
    return instance->impl.get();
}


// ----- Bridge functions -----
// Allow per-endpoint .cpp files to register with the orchestrator and
// store clients without including instance_impl.hpp.

size_t FindOrCreateClient(std::string auth_identity, std::string endpoint_url,
                          std::string model_group, const ClientPolicy& policy,
                          std::string model, http::RequestHeaders headers,
                          std::string url)
{
    auto& impl = *GetImpl();

    ClientKey key{
        .queue_key = QueueKey{
            .auth_identity = std::move(auth_identity),
            .endpoint_url = std::move(endpoint_url),
            .model_group = std::move(model_group)
        },
        .policy = policy
    };

    // Fast path: check if a client with this key already exists.
    {
        std::shared_lock lock(impl.clients_mtx);
        if (auto it = impl.client_map.find(key); it != impl.client_map.end()) {
            return it->second;
        }
    }

    // Slow path: acquire exclusive lock, double-check, then create.
    std::unique_lock lock(impl.clients_mtx);
    if (auto it = impl.client_map.find(key); it != impl.client_map.end()) {
        return it->second;  // another thread beat us
    }

    // Register with the orchestrator and construct the client.
    auto token = impl.orchestrator.Register(key.policy, key.queue_key);

    size_t id = impl.clients.size();
    impl.clients.emplace_back(std::make_unique<Client>(Client{
        .registration_index = token.index,
        .model = std::move(model),
        .request_headers = std::move(headers),
        .url = std::move(url)
    }));
    impl.client_map.emplace(std::move(key), id);
    return id;
}


SubmitResult SubmitRequest(size_t client_id, std::vector<std::byte> body,
                           const AttemptPolicy& policy)
{
    try {
        auto& impl = *GetImpl();

        if (impl.dead.load(std::memory_order_acquire)) {
            throw FatalInstanceError{
                "jai::llm::SubmitRequest: instance is dead.",
                impl.fatal_error
            };
        }

        // No lock needed: ClientHandle can only exist after FindOrCreateClient
        // completes, and clients are never removed. Direct free-function callers
        // with invalid IDs hit the bounds check and get an exception.
        if (client_id >= impl.clients.size()) {
            throw AnnotatedException{
                "jai::llm::SubmitRequest: invalid client ID."
            };
        }
        Client* client = impl.clients[client_id].get();

        auto sync = std::make_shared<ResultSync>();

        size_t ticket = impl.orchestrator.Submit(
            Orchestrator::RegistrationToken{client->registration_index},
            client->request_headers,
            client->url,
            http::Method::POST,
            std::move(body),
            policy,
            sync
        );

        return SubmitResult{
            .orchestrator = &impl.orchestrator,
            .ticket = ticket,
            .sync = std::move(sync)
        };
    } catch (const AnnotatedException&) {
        throw;
    } catch (const std::exception& e) {
        throw AnnotatedException{e.what()};
    }
}


// ----- Token usage accumulation -----

namespace {

void AccumulateField(std::optional<int64_t>& dst, const std::optional<int64_t>& src) {
    if (src) {
        dst = dst.value_or(0) + *src;
    }
}

} // anonymous namespace


void AccumulateUsage(const TokenUsage& usage) {
    auto& impl = *GetImpl();
    std::lock_guard lock(impl.usage_mtx);
    AccumulateField(impl.total_usage.input_tokens, usage.input_tokens);
    AccumulateField(impl.total_usage.output_tokens, usage.output_tokens);
    AccumulateField(impl.total_usage.total_tokens, usage.total_tokens);
    AccumulateField(impl.total_usage.cache_creation_tokens, usage.cache_creation_tokens);
    AccumulateField(impl.total_usage.cache_read_tokens, usage.cache_read_tokens);
    AccumulateField(impl.total_usage.reasoning_tokens, usage.reasoning_tokens);
    AccumulateField(impl.total_usage.tool_use_tokens, usage.tool_use_tokens);
}


// ----- Observability -----

TokenUsage Instance::TotalUsage() const {
    if (!self || !impl) { return {}; }
    std::lock_guard lock(impl->usage_mtx);
    return impl->total_usage;
}


size_t Instance::PendingCount() const {
    if (!self || !impl) { return 0; }
    return impl->orchestrator.PendingCount();
}


bool Instance::IsRunning() const {
    return self && impl && impl->started;
}


}
