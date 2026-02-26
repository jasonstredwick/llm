/***
 * Anthropic client — typed adapter between anthropic::Request/Message
 * and the orchestrator's HTTP transport engine.
 *
 * Currently supports API key authentication (direct Anthropic API).
 * Future auth modes (AWS Bedrock, Vertex AI) can be added to the
 * auth variant without changing the public interface.
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
#include <variant>


namespace jai::llm { class Orchestrator; }


namespace jai::llm::anthropic {


// API key authentication for the direct Anthropic API.
// The key is passed in the x-api-key header.
// The version field controls the anthropic-version header; override it
// to opt into beta features (e.g., extended thinking, computer use).
struct ApiKeyAuth {
    std::string api_key;
    std::string version{"2023-06-01"};
};


// Normalize a model string to its rate limit family.
// Anthropic groups rate limits by model family — all Opus versions share
// one pool, all Sonnet versions share another, etc.
//   "claude-opus-4-20250514"     → "opus"
//   "claude-sonnet-4.5-20250929" → "sonnet"
//   "claude-3-5-sonnet-20241022" → "sonnet"  (legacy naming)
// Returns the model string as-is if the family cannot be determined.
std::string ModelGroup(std::string_view model);


class Client {
private:
    Orchestrator& orchestrator;
    size_t registration_index{};
    std::variant<ApiKeyAuth> auth;
    std::string model;

public:
    // API Key authentication (default Anthropic endpoint).
    Client(Orchestrator& orchestrator,
           ApiKeyAuth auth,
           std::string model,
           const ClientPolicy& client_policy = {});

    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;

    Result<Message> CallAsync(const Request& r,
                              const AttemptPolicy& call_policy = {}) const;

    Message CallSync(const Request& r,
                     const AttemptPolicy& call_policy = {}) const;

    std::string_view GetModel() const { return model; }
};


}
