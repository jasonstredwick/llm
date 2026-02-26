/***
 * Gemini client — implementation.
 *
 * Protocol-level helpers (URL building, header construction, model grouping)
 * are file-local. Serialize/Deserialize are defined in the separate
 * src/protocols/serialize/ and src/protocols/deserialize/ translation units.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/clients/gemini.hpp"

#include "../orchestrator.hpp"
#include "../curl.hpp"
#include "../http.hpp"

#include <cstddef>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>


namespace jai::llm::gemini {


// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


// ----- ModelGroup (public) -----

std::string ModelGroup(std::string_view model) {
    // Strip from "-preview" or "-exp" onwards (includes any trailing date).
    if (auto pos = model.find("-preview"); pos != std::string_view::npos) {
        return std::string{model.substr(0, pos)};
    }
    if (auto pos = model.find("-exp"); pos != std::string_view::npos) {
        return std::string{model.substr(0, pos)};
    }

    // Strip trailing point-release suffix: -NNN (all digits after last dash).
    if (auto pos = model.rfind('-'); pos != std::string_view::npos && pos + 1 < model.size()) {
        auto tail = model.substr(pos + 1);
        bool all_digits = true;
        for (char c : tail) {
            if (c < '0' || c > '9') { all_digits = false; break; }
        }
        if (all_digits) {
            return std::string{model.substr(0, pos)};
        }
    }

    return std::string{model};
}


namespace {


// ----- Endpoint defaults -----

constexpr std::string_view API_KEY_BASE =
    "https://generativelanguage.googleapis.com/v1beta";

// Vertex AI global endpoint (no region prefix).
constexpr std::string_view VERTEX_GLOBAL_BASE =
    "https://aiplatform.googleapis.com/v1";

// Vertex AI regional endpoint template: {location}-aiplatform.googleapis.com
constexpr std::string_view VERTEX_REGIONAL_FMT =
    "https://{}-aiplatform.googleapis.com/v1";


// ----- URL building -----

std::string BuildUrl(const ApiKeyAuth& auth, std::string_view model) {
    return std::format("{}/models/{}:generateContent?key={}",
                       API_KEY_BASE, model, auth.api_key);
}

std::string BuildUrl(const VertexAuth& auth, std::string_view model) {
    // The global endpoint uses a different host than regional endpoints.
    // Global: https://aiplatform.googleapis.com/v1/projects/{p}/locations/global/...
    // Regional: https://{region}-aiplatform.googleapis.com/v1/projects/{p}/locations/{region}/...
    std::string_view base = (auth.location == LOCATION_GLOBAL)
        ? VERTEX_GLOBAL_BASE
        : std::string_view{};

    if (base.empty()) {
        return std::format(
            "{}/projects/{}/locations/{}"
            "/publishers/google/models/{}:generateContent",
            std::format(VERTEX_REGIONAL_FMT, auth.location),
            auth.project, auth.location, model);
    }

    return std::format(
        "{}/projects/{}/locations/{}"
        "/publishers/google/models/{}:generateContent",
        base, auth.project, auth.location, model);
}


// ----- Endpoint base for QueueKey -----

std::string EndpointBase(const ApiKeyAuth&) {
    return std::string{API_KEY_BASE};
}

std::string EndpointBase(const VertexAuth& auth) {
    if (auth.location == LOCATION_GLOBAL) {
        return std::format("{}/projects/{}/locations/global",
                           VERTEX_GLOBAL_BASE, auth.project);
    }
    return std::format("{}/projects/{}/locations/{}",
                       std::format(VERTEX_REGIONAL_FMT, auth.location),
                       auth.project, auth.location);
}


// ----- Request headers -----

http::RequestHeaders BuildRequestHeaders(const ApiKeyAuth&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"}
    }};
}

http::RequestHeaders BuildRequestHeaders(const VertexAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"Authorization", std::format("Bearer {}", auth.access_token)}
    }};
}


// ----- Auth identity (for QueueKey) -----

std::string AuthIdentity(const ApiKeyAuth& auth) { return auth.api_key; }
std::string AuthIdentity(const VertexAuth& auth) { return auth.project; }


} // anonymous namespace


// ----- Construction -----

Client::Client(Orchestrator& orchestrator_,
               ApiKeyAuth auth_,
               std::string model_,
               const ClientPolicy& client_policy)
    : orchestrator{orchestrator_}
    , auth{std::move(auth_)}
    , model{std::move(model_)}
{
    const auto& a = std::get<ApiKeyAuth>(auth);
    auto token = orchestrator.Register(client_policy, QueueKey{
        .auth_identity = AuthIdentity(a),
        .endpoint_url = EndpointBase(a),
        .model_group = ModelGroup(model)
    });
    registration_index = token.index;
}


Client::Client(Orchestrator& orchestrator_,
               VertexAuth auth_,
               std::string model_,
               const ClientPolicy& client_policy)
    : orchestrator{orchestrator_}
    , auth{std::move(auth_)}
    , model{std::move(model_)}
{
    const auto& a = std::get<VertexAuth>(auth);
    auto token = orchestrator.Register(client_policy, QueueKey{
        .auth_identity = AuthIdentity(a),
        .endpoint_url = EndpointBase(a),
        .model_group = ModelGroup(model)
    });
    registration_index = token.index;
}


// ----- CallAsync -----

AsyncResult<Response> Client::CallAsync(const Request& r,
                                   const AttemptPolicy& call_policy) const {
    auto [headers, url] = std::visit([&](const auto& a) {
        return std::pair{
            BuildRequestHeaders(a),
            BuildUrl(a, model)
        };
    }, auth);

    http::Request http_request{
        .headers = std::move(headers),
        .method = http::Method::POST,
        .url = std::move(url),
        .body = Serialize(r)
    };

    auto sync = std::make_shared<ResultSync>();

    size_t ticket = orchestrator.Submit(
        Orchestrator::RegistrationToken{registration_index},
        std::move(http_request),
        call_policy,
        sync
    );

    return AsyncResult<Response>{orchestrator, ticket, &Deserialize, std::move(sync)};
}


// ----- CallSync -----

Response Client::CallSync(const Request& r,
                          const AttemptPolicy& call_policy) const {
    auto result = CallAsync(r, call_policy);

    result.RethrowIfException();

    if (result.HasError()) {
        throw AnnotatedException{result.Error()};
    }

    return std::move(result.Data());
}


// ----- CallCoro -----

CoroAsyncResult<Response> Client::CallCoro(const Request& r,
                                      const AttemptPolicy& call_policy) const {
    // Capture before first suspension point so the coroutine frame holds
    // these directly, not through `this`. Matches Result's lifetime
    // semantics: only the Orchestrator must outlive the CoroAsyncResult.
    Orchestrator* orch = &orchestrator;
    size_t reg_index = registration_index;

    auto [headers, url] = std::visit([&](const auto& a) {
        return std::pair{
            BuildRequestHeaders(a),
            BuildUrl(a, model)
        };
    }, auth);

    http::Request http_request{
        .headers = std::move(headers),
        .method = http::Method::POST,
        .url = std::move(url),
        .body = Serialize(r)
    };

    auto sync = std::make_shared<ResultSync>();

    size_t ticket = orch->Submit(
        Orchestrator::RegistrationToken{reg_index},
        std::move(http_request),
        call_policy,
        sync
    );

    while (true) {
        co_await SyncAwaiter{sync};

        if (!sync->succeeded) {
            throw AnnotatedException{sync->error_msg};
        }

        try {
            const auto& resp = GetResponseRef(orch, ticket);
            auto data = Deserialize(resp);
            ReleaseSlotRequest(orch, ticket);
            co_return std::move(data);
        } catch (...) {
            sync->Reset();
            if (!RetrySlotRequest(orch, ticket, sync)) {
                throw;  // retry budget exhausted
            }
        }
    }
}


}
