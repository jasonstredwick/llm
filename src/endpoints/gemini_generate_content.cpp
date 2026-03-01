/***
 * Gemini GenerateContent endpoint — CreateClient and Dispatch specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/gemini_generate_content.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"

#include "../../interface/core/error.hpp"

#include <string>
#include <utility>


namespace jai::llm {


namespace gemini {

// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);

}


// ----- Endpoint defaults -----

constexpr std::string_view API_KEY_BASE =
    "https://generativelanguage.googleapis.com/v1beta";

// Vertex AI global endpoint (no region prefix).
constexpr std::string_view VERTEX_GLOBAL_BASE =
    "https://aiplatform.googleapis.com/v1";

// Vertex AI regional endpoint template: {location}-aiplatform.googleapis.com
constexpr std::string_view VERTEX_REGIONAL_FMT =
    "https://{}-aiplatform.googleapis.com/v1";


// -----Functionality -----

// ----- Auth identity (for QueueKey) -----
template <>
std::string AuthIdentity<gemini::GenerateContent>(const gemini::ApiKeyAuth& auth) { return auth.api_key; }


// ----- Auth identity (for QueueKey) -----
template <>
std::string AuthIdentity<gemini::GenerateContent>(const gemini::VertexAuth& auth) { return auth.project; }


template <>
http::RequestHeaders BuildRequestHeaders<gemini::GenerateContent>(const gemini::ApiKeyAuth&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"}
    }};
}

template <>
http::RequestHeaders BuildRequestHeaders<gemini::GenerateContent>(const gemini::VertexAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"Authorization", std::format("Bearer {}", auth.access_token)}
    }};
}


template <>
std::string BuildUrl<gemini::GenerateContent>(const gemini::ApiKeyAuth& auth, std::string_view model) {
    return std::format("{}/models/{}:generateContent?key={}",
                       API_KEY_BASE, model, auth.api_key);
}


template <>
std::string BuildUrl<gemini::GenerateContent>(const gemini::VertexAuth& auth, std::string_view model) {
    // The global endpoint uses a different host than regional endpoints.
    // Global: https://aiplatform.googleapis.com/v1/projects/{p}/locations/global/...
    // Regional: https://{region}-aiplatform.googleapis.com/v1/projects/{p}/locations/{region}/...
    std::string_view base = (auth.location == gemini::LOCATION_GLOBAL)
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


template <>
typename gemini::GenerateContent::Response_t Deserialize<gemini::GenerateContent>(const curl::Response& curl_response) {
    return gemini::Deserialize(curl_response);
}


// Wrapper matching AsyncResult<Response>::DeserializeFn signature.
// Borrows the response from the orchestrator, deserializes, and releases.
template <>
typename gemini::GenerateContent::Response_t DeserializeAndRelease<gemini::GenerateContent>(Orchestrator* orch, size_t ticket) {
    const auto& resp = orch->GetResponse(ticket);
    auto data = gemini::Deserialize(resp);
    orch->ReleaseSlot(ticket);
    return data;
}


// ----- Endpoint base for QueueKey -----
template <>
std::string EndpointBase<gemini::GenerateContent>(const gemini::ApiKeyAuth&) {
    return std::string{API_KEY_BASE};
}


// ----- Endpoint base for QueueKey -----
template <>
std::string EndpointBase<gemini::GenerateContent>(const gemini::VertexAuth& auth) {
    if (auth.location == gemini::LOCATION_GLOBAL) {
        return std::format("{}/projects/{}/locations/global",
                           VERTEX_GLOBAL_BASE, auth.project);
    }
    return std::format("{}/projects/{}/locations/{}",
                       std::format(VERTEX_REGIONAL_FMT, auth.location),
                       auth.project, auth.location);
}


template <>
std::string ModelGroup<gemini::GenerateContent>(std::string_view model) {
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


template <>
std::vector<std::byte> Serialize<gemini::GenerateContent>(const gemini::GenerateContent::Request_t& request) {
    return gemini::Serialize(request);
}


// ----- CreateClientImpl specializations -----

template <>
size_t CreateClientImpl<gemini::GenerateContent, gemini::ApiKeyAuth>(
    gemini::ApiKeyAuth auth, std::string model, const ClientPolicy& policy)
{
    return FindOrCreateClient(
        AuthIdentity<gemini::GenerateContent>(auth),
        EndpointBase<gemini::GenerateContent>(auth),
        ModelGroup<gemini::GenerateContent>(model),
        policy,
        std::move(model),
        BuildRequestHeaders<gemini::GenerateContent>(auth),
        BuildUrl<gemini::GenerateContent>(auth, model));
}


template <>
size_t CreateClientImpl<gemini::GenerateContent, gemini::VertexAuth>(
    gemini::VertexAuth auth, std::string model, const ClientPolicy& policy)
{
    return FindOrCreateClient(
        AuthIdentity<gemini::GenerateContent>(auth),
        EndpointBase<gemini::GenerateContent>(auth),
        ModelGroup<gemini::GenerateContent>(model),
        policy,
        std::move(model),
        BuildRequestHeaders<gemini::GenerateContent>(auth),
        BuildUrl<gemini::GenerateContent>(auth, model));
}


// ----- CallAsync -----

template <>
AsyncResult<gemini::GenerateContent::Response_t> CallAsync<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<gemini::GenerateContent>(request), policy);
    return AsyncResult<gemini::GenerateContent::Response_t>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<gemini::GenerateContent>,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<gemini::GenerateContent::Response_t> CallCoro<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<gemini::GenerateContent>(request), policy);

    while (true) {
        co_await SyncAwaiter{sr.sync};

        if (!sr.sync->succeeded) {
            throw AnnotatedException{sr.sync->error_msg};
        }

        try {
            auto data = Deserialize<gemini::GenerateContent>(sr.GetResponse());
            sr.ReleaseSlot();
            co_return std::move(data);
        } catch (...) {
            if (!sr.RetrySlot()) {
                throw;  // retry budget exhausted
            }
        }
    }
}


// ----- CallSync -----

template <>
gemini::GenerateContent::Response_t CallSync<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto result = CallAsync<gemini::GenerateContent>(client_id, request, policy);
    result.RethrowIfException();
    if (result.HasError()) {
        throw AnnotatedException{result.Error()};
    }
    return std::move(result.Data());
}


}
