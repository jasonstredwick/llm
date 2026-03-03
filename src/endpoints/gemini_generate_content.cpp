/***
 * Gemini GenerateContent endpoint — CreateClient and Dispatch specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/gemini_generate_content.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"
#include "../results.hpp"

#include <optional>
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


// Deserialize from a live slot (does NOT release — caller handles release).
template <>
typename gemini::GenerateContent::Response_t DeserializeAndRelease<gemini::GenerateContent>(Orchestrator* orch, Ticket ticket) {
    return gemini::Deserialize(orch->GetResponse(ticket));
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


// ----- Envelope metadata extraction -----

template <>
TokenUsage ExtractUsage<gemini::GenerateContent>(const gemini::GenerateContentResponse& resp) {
    TokenUsage usage{};
    if (resp.usageMetadata) {
        auto& u = *resp.usageMetadata;
        if (u.promptTokenCount)        usage.input_tokens         = *u.promptTokenCount;
        if (u.candidatesTokenCount)    usage.output_tokens        = *u.candidatesTokenCount;
        if (u.totalTokenCount)         usage.total_tokens         = *u.totalTokenCount;
        if (u.cachedContentTokenCount) usage.cache_read_tokens    = *u.cachedContentTokenCount;
        if (u.thoughtsTokenCount)      usage.reasoning_tokens     = *u.thoughtsTokenCount;
        if (u.toolUsePromptTokenCount) usage.tool_use_tokens      = *u.toolUsePromptTokenCount;
        // Gemini does not report cache_creation_tokens.
    }
    return usage;
}


template <>
std::optional<std::string> ExtractModel<gemini::GenerateContent>(const gemini::GenerateContentResponse& resp) {
    return resp.modelVersion;
}


template <>
std::optional<std::string> ExtractStopReason<gemini::GenerateContent>(const gemini::GenerateContentResponse& resp) {
    if (!resp.candidates || resp.candidates->empty()) return std::nullopt;
    auto& first = resp.candidates->front();
    if (!first.finishReason) return std::nullopt;
    switch (*first.finishReason) {
        case gemini::FinishReason::FINISH_REASON_UNSPECIFIED: return "unspecified";
        case gemini::FinishReason::STOP:                      return "stop";
        case gemini::FinishReason::MAX_TOKENS:                return "max_tokens";
        case gemini::FinishReason::SAFETY:                    return "safety";
        case gemini::FinishReason::RECITATION:                return "recitation";
        case gemini::FinishReason::LANGUAGE:                  return "language";
        case gemini::FinishReason::OTHER:                     return "other";
        case gemini::FinishReason::BLOCKLIST:                 return "blocklist";
        case gemini::FinishReason::PROHIBITED_CONTENT:        return "prohibited_content";
        case gemini::FinishReason::SPII:                      return "spii";
        case gemini::FinishReason::MALFORMED_FUNCTION_CALL:   return "malformed_function_call";
        case gemini::FinishReason::IMAGE_SAFETY:              return "image_safety";
        case gemini::FinishReason::IMAGE_PROHIBITED_CONTENT:  return "image_prohibited_content";
        case gemini::FinishReason::IMAGE_OTHER:               return "image_other";
        case gemini::FinishReason::NO_IMAGE:                  return "no_image";
        case gemini::FinishReason::IMAGE_RECITATION:          return "image_recitation";
        case gemini::FinishReason::UNEXPECTED_TOOL_CALL:      return "unexpected_tool_call";
        case gemini::FinishReason::TOO_MANY_TOOL_CALLS:       return "too_many_tool_calls";
        case gemini::FinishReason::MISSING_THOUGHT_SIGNATURE: return "missing_thought_signature";
        case gemini::FinishReason::MALFORMED_RESPONSE:        return "malformed_response";
    }
    return std::nullopt;
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


// ----- Extract wrapper (bridges internal extraction into ExtractFn pointer) -----

namespace {

void ExtractEnvelope(const gemini::GenerateContentResponse& data, AttemptMetadata& am) {
    am.usage.emplace(ExtractUsage<gemini::GenerateContent>(data));
    am.model = ExtractModel<gemini::GenerateContent>(data);
    am.stop_reason = ExtractStopReason<gemini::GenerateContent>(data);
}

} // anonymous namespace


// ----- CallAsync -----

template <>
AsyncResult<gemini::GenerateContent> CallAsync<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<gemini::GenerateContent>(request), policy);
    return AsyncResult<gemini::GenerateContent>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<gemini::GenerateContent>,
        &ExtractEnvelope,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<gemini::GenerateContent> CallCoro<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<gemini::GenerateContent>(request), policy);
    Result<gemini::GenerateContent, void> result;

    while (true) {
        co_await SyncAwaiter{sr.sync.get()};

        const auto& response = sr.GetResponse();
        auto am = BuildTransportMetadata(response);

        if (!sr.sync->succeeded) {
            am.error = sr.sync->error_msg;
            am.outcome = (response.state == curl::Response::State::FAILED)
                ? AttemptOutcome::TRANSPORT_ERROR
                : AttemptOutcome::HTTP_ERROR;
            result.attempts.push_back(std::move(am));

            if (!sr.RetrySlot()) {
                result.error.emplace(result.attempts.back().error);
                co_return std::move(result);
            }
            continue;
        }

        try {
            result.data.emplace(Deserialize<gemini::GenerateContent>(response));
            am.outcome = AttemptOutcome::SUCCESS;
            ExtractEnvelope(*result.data, am);
            if (am.usage) { AccumulateUsage(*am.usage); }
            result.attempts.push_back(std::move(am));
            sr.ReleaseSlot();
            co_return std::move(result);
        } catch (const std::exception& e) {
            am.outcome = AttemptOutcome::DESERIALIZATION_ERROR;
            am.error = e.what();
            result.attempts.push_back(std::move(am));
            if (!sr.RetrySlot()) {
                result.error.emplace(result.attempts.back().error);
                co_return std::move(result);
            }
        }
    }
}


// ----- CallSync -----

template <>
Result<gemini::GenerateContent, void> CallSync<gemini::GenerateContent>(
    size_t client_id,
    const gemini::GenerateContent::Request_t& request,
    const AttemptPolicy& policy)
{
    auto ar = CallAsync<gemini::GenerateContent>(client_id, request, policy);
    return ar.Take();
}


}
