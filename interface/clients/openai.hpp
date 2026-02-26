/***
 * OpenAI client — typed adapter between openai::Request/Response
 * and the orchestrator's HTTP transport engine.
 *
 * Currently supports API key authentication (direct OpenAI API).
 * Future auth modes (Azure OpenAI) can be added to the auth variant
 * without changing the public interface.
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
#include "../protocols/openai/responses.hpp"
#include "../core/async.hpp"

#include <string>
#include <string_view>
#include <variant>


namespace jai::llm { class Orchestrator; }


namespace jai::llm::openai {


// API key authentication for the direct OpenAI API.
// The key is passed as a Bearer token in the Authorization header.
struct ApiKeyAuth {
    std::string api_key;
};


// Normalize a model string to its rate limit group.
// OpenAI groups models by base name — strips trailing -YYYY-MM-DD date suffix.
//   "gpt-4o-2024-08-06" → "gpt-4o"
//   "o3-2025-04-16"     → "o3"
//   "o3-mini"           → "o3-mini"  (mini is a variant, not a date)
std::string ModelGroup(std::string_view model);


class Client {
private:
    Orchestrator& orchestrator;
    size_t registration_index{};
    std::variant<ApiKeyAuth> auth;
    std::string model;

public:
    // API Key authentication (default OpenAI endpoint).
    Client(Orchestrator& orchestrator,
           ApiKeyAuth auth,
           std::string model,
           const ClientPolicy& client_policy = {});

    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;

    Result<Response> CallAsync(const Request& r,
                               const AttemptPolicy& call_policy = {}) const;

    Response CallSync(const Request& r,
                      const AttemptPolicy& call_policy = {}) const;

    std::string_view GetModel() const { return model; }
};


}
