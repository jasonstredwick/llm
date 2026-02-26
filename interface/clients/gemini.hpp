/***
 * Gemini client — typed adapter between gemini::Request/Response
 * and the orchestrator's HTTP transport engine.
 *
 * Supports two authentication modes:
 *   - ApiKeyAuth:  Google AI Studio (Generative Language API)
 *   - VertexAuth:  Google Cloud Vertex AI
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
#include "../protocols/gemini/generate_content.hpp"
#include "../core/async.hpp"

#include <string>
#include <string_view>
#include <variant>


namespace jai::llm { class Orchestrator; }


namespace jai::llm::gemini {


// Well-known Vertex AI location for the global endpoint.
// Routes requests to the most available region automatically.
// See: https://cloud.google.com/vertex-ai/generative-ai/docs/learn/locations
inline constexpr std::string_view LOCATION_GLOBAL = "global";


// API key authentication for Google AI Studio (Generative Language API).
// The key is passed as a URL query parameter.
struct ApiKeyAuth {
    std::string api_key;
};


// Vertex AI authentication via short-lived OAuth2 bearer token.
// The token is passed in the Authorization header.
// Location defaults to the global endpoint; set to a specific region
// (e.g., "us-central1") for data residency requirements.
struct VertexAuth {
    std::string access_token;
    std::string project;
    std::string location{LOCATION_GLOBAL};
};


// Normalize a model string to its rate limit group.
// Gemini has per-model rate limits. Strips preview/experimental/point-release
// suffixes but preserves variant names.
//   "gemini-2.5-pro-preview-05-06" → "gemini-2.5-pro"
//   "gemini-2.5-flash-lite"        → "gemini-2.5-flash-lite"
//   "gemini-2.5-flash-001"         → "gemini-2.5-flash"
std::string ModelGroup(std::string_view model);


class Client {
private:
    Orchestrator& orchestrator;
    size_t registration_index{};
    std::variant<ApiKeyAuth, VertexAuth> auth;
    std::string model;

public:
    // API Key authentication (Google AI Studio).
    Client(Orchestrator& orchestrator,
           ApiKeyAuth auth,
           std::string model,
           const ClientPolicy& client_policy = {});

    // Vertex AI authentication.
    Client(Orchestrator& orchestrator,
           VertexAuth auth,
           std::string model,
           const ClientPolicy& client_policy = {});

    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;

    AsyncResult<Response> CallAsync(const Request& r,
                               const AttemptPolicy& call_policy = {}) const;

    Response CallSync(const Request& r,
                      const AttemptPolicy& call_policy = {}) const;

    // Coroutine-based call. Returns a CoroAsyncResult that can be co_await-ed
    // or polled after the event loop completes.
    //
    // Unlike CallAsync (where deserialization runs on the caller's thread),
    // deserialization runs on the orchestrator's thread inside RunOnce().
    // The data is then copied out to the caller via co_await or Data().
    CoroAsyncResult<Response> CallCoro(const Request& r,
                                  const AttemptPolicy& call_policy = {}) const;

    std::string_view GetModel() const { return model; }
};


}
