/***
 * OpenAI Responses endpoint — CreateClient and Call specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/openai_responses.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"
#include "../results.hpp"

#include <format>
#include <optional>
#include <string>
#include <utility>
#include <variant>


namespace jai::llm {


namespace openai {

// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);

}


// ----- Endpoint defaults -----

constexpr std::string_view OPENAI_ENDPOINT =
    "https://api.openai.com/v1/responses";


// ----- Functionality -----

// ----- Auth identity (for QueueKey) -----
template <>
std::string AuthIdentity<openai::Responses>(const openai::ApiKeyAuth& auth) {
    return auth.api_key;
}

template <>
std::string AuthIdentity<openai::Responses>(const openai::AzureAuth& auth) {
    return std::format("{}:{}", auth.resource_name, auth.deployment);
}


// ----- Request headers -----
template <>
http::RequestHeaders BuildRequestHeaders<openai::Responses>(const openai::ApiKeyAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"Authorization", std::format("Bearer {}", auth.api_key)}
    }};
}

template <>
http::RequestHeaders BuildRequestHeaders<openai::Responses>(const openai::AzureAuth& auth) {
    std::vector<std::pair<std::string, std::string>> headers{
        {"Content-Type", "application/json"}
    };
    if (auth.use_bearer_token) {
        headers.emplace_back("Authorization", std::format("Bearer {}", auth.api_key));
    } else {
        headers.emplace_back("api-key", auth.api_key);
    }
    return http::RequestHeaders{std::move(headers)};
}


// ----- URL construction -----
template <>
std::string BuildUrl<openai::Responses>(const openai::ApiKeyAuth&, std::string_view) {
    return std::string{OPENAI_ENDPOINT};
}

template <>
std::string BuildUrl<openai::Responses>(const openai::AzureAuth& auth, std::string_view) {
    return std::format(
        "https://{}.openai.azure.com/openai/deployments/{}/responses?api-version={}",
        auth.resource_name, auth.deployment, auth.api_version);
}


template <>
typename openai::Responses::Response_t Deserialize<openai::Responses>(const curl::Response& curl_response) {
    return openai::Deserialize(curl_response);
}


// Deserialize from a live slot (does NOT release — caller handles release).
template <>
typename openai::Responses::Response_t DeserializeAndRelease<openai::Responses>(Orchestrator* orch, Ticket ticket) {
    return openai::Deserialize(orch->GetResponse(ticket));
}


template <>
std::string EndpointBase<openai::Responses>(const openai::ApiKeyAuth&) {
    return std::string{OPENAI_ENDPOINT};
}

template <>
std::string EndpointBase<openai::Responses>(const openai::AzureAuth& auth) {
    return std::format(
        "https://{}.openai.azure.com/openai/deployments/{}",
        auth.resource_name, auth.deployment);
}


template <>
std::string ModelGroup<openai::Responses>(std::string_view model) {
    // Date suffix pattern: -DDDD-DD-DD at the end (exactly 11 chars).
    if (model.size() > 11) {
        auto tail = model.substr(model.size() - 11);
        if (tail[0] == '-' &&
            tail[5] == '-' &&
            tail[8] == '-') {
            bool all_digits = true;
            for (size_t i : {1, 2, 3, 4, 6, 7, 9, 10}) {
                if (tail[i] < '0' || tail[i] > '9') { all_digits = false; break; }
            }
            if (all_digits) {
                return std::string{model.substr(0, model.size() - 11)};
            }
        }
    }
    return std::string{model};
}


template <>
std::vector<std::byte> Serialize<openai::Responses>(const openai::Responses::Request_t& request) {
    return openai::Serialize(request);
}


// ----- Envelope metadata extraction -----

template <>
TokenUsage ExtractUsage<openai::Responses>(const openai::Response& resp) {
    TokenUsage usage{};
    if (resp.usage) {
        auto& u = *resp.usage;
        if (u.input_tokens)  usage.input_tokens  = static_cast<int64_t>(*u.input_tokens);
        if (u.output_tokens) usage.output_tokens = static_cast<int64_t>(*u.output_tokens);
        if (u.total_tokens)  usage.total_tokens  = static_cast<int64_t>(*u.total_tokens);
        if (u.input_tokens_details && u.input_tokens_details->cached_tokens) {
            usage.cache_read_tokens = static_cast<int64_t>(*u.input_tokens_details->cached_tokens);
        }
        if (u.output_tokens_details && u.output_tokens_details->reasoning_tokens) {
            usage.reasoning_tokens = static_cast<int64_t>(*u.output_tokens_details->reasoning_tokens);
        }
        // OpenAI does not report cache_creation_tokens or tool_use_tokens.
    }
    return usage;
}


template <>
std::optional<std::string> ExtractModel<openai::Responses>(const openai::Response& resp) {
    if (!resp.model) return std::nullopt;
    return std::visit([](const auto& v) -> std::string {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, std::string>) {
            return v;
        } else {
            // ResponsesModelValues enum — return empty string as placeholder.
            // In practice, JSON deserialization yields the string variant.
            return {};
        }
    }, *resp.model);
}


template <>
std::optional<std::string> ExtractStopReason<openai::Responses>(const openai::Response& resp) {
    if (!resp.status) return std::nullopt;
    switch (*resp.status) {
        case openai::Response::Status::COMPLETED:   return "completed";
        case openai::Response::Status::FAILED:       return "failed";
        case openai::Response::Status::IN_PROGRESS:  return "in_progress";
        case openai::Response::Status::CANCELLED:    return "cancelled";
        case openai::Response::Status::QUEUED:       return "queued";
        case openai::Response::Status::INCOMPLETE:   return "incomplete";
    }
    return std::nullopt;
}


// ----- CreateClientImpl specialization -----

template <>
size_t CreateClientImpl<openai::Responses, openai::ApiKeyAuth>(
    openai::ApiKeyAuth auth, std::string model, const ClientPolicy& policy)
{
    return FindOrCreateClient(
        AuthIdentity<openai::Responses>(auth),
        EndpointBase<openai::Responses>(auth),
        ModelGroup<openai::Responses>(model),
        policy,
        std::move(model),
        BuildRequestHeaders<openai::Responses>(auth),
        BuildUrl<openai::Responses>(auth, model));
}

template <>
size_t CreateClientImpl<openai::Responses, openai::AzureAuth>(
    openai::AzureAuth auth, std::string model, const ClientPolicy& policy)
{
    return FindOrCreateClient(
        AuthIdentity<openai::Responses>(auth),
        EndpointBase<openai::Responses>(auth),
        ModelGroup<openai::Responses>(model),
        policy,
        std::move(model),
        BuildRequestHeaders<openai::Responses>(auth),
        BuildUrl<openai::Responses>(auth, model));
}


// ----- Extract wrapper (bridges internal extraction into ExtractFn pointer) -----

namespace {

void ExtractEnvelope(const openai::Response& data, AttemptMetadata& am) {
    am.usage.emplace(ExtractUsage<openai::Responses>(data));
    am.model = ExtractModel<openai::Responses>(data);
    am.stop_reason = ExtractStopReason<openai::Responses>(data);
}

} // anonymous namespace


// ----- CallAsync -----

template <>
AsyncResult<openai::Responses> CallAsync<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<openai::Responses>(request), policy);
    return AsyncResult<openai::Responses>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<openai::Responses>,
        &ExtractEnvelope,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<openai::Responses> CallCoro<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<openai::Responses>(request), policy);
    Result<openai::Responses, void> result;

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
            result.data.emplace(Deserialize<openai::Responses>(response));
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
Result<openai::Responses, void> CallSync<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto ar = CallAsync<openai::Responses>(client_id, request, policy);
    return ar.Take();
}


}
