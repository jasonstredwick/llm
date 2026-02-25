/***
 * Anthropic client — typed adapter between anthropic::Request/Message
 * and the orchestrator's HTTP transport engine.
 *
 * Each client is bound to a single (auth, endpoint, model) configuration.
 * Registration with the orchestrator happens at construction.
 *
 * Lifetime contract: the Orchestrator must outlive the Client, and the Client
 * (or at minimum the Result it returns) must outlive any in-flight requests.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "policy.hpp"
#include "../protocols/anthropic/messages.hpp"
#include "../core/async.hpp"

#include <string>
#include <string_view>


namespace jai::llm { class Orchestrator; }


namespace jai::llm::anthropic {


class Client {
private:
    Orchestrator& orchestrator;
    size_t registration_index{};  // opaque token from Orchestrator::Register
    std::string api_key{};
    std::string model{};
    std::string endpoint_url{"https://api.anthropic.com/v1/messages"};

public:
    // Construct with default policies.
    Client(Orchestrator& orchestrator,
           std::string api_key,
           std::string model);

    // Construct with client policy overrides.
    Client(Orchestrator& orchestrator,
           std::string api_key,
           std::string model,
           const ClientPolicy& client_policy);

    // Construct with client policy overrides and custom endpoint.
    Client(Orchestrator& orchestrator,
           std::string api_key,
           std::string model,
           const ClientPolicy& client_policy,
           std::string endpoint_url);

    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;

    // Async call — submits the request to the orchestrator and returns
    // immediately. The returned Result blocks on access (Data(), Error())
    // until the orchestrator has emplaced a result. Use IsDone() to poll
    // without blocking.
    Result<Message> CallAsync(const Request& r,
                              const AttemptPolicy& call_policy = {}) const;

    // Sync call — blocks until the response is available.
    // Internally calls CallAsync and waits for completion.
    Message CallSync(const Request& r,
                     const AttemptPolicy& call_policy = {}) const;

    // Accessors
    std::string_view GetModel() const { return model; }
    std::string_view GetEndpoint() const { return endpoint_url; }
};


}
