/***
 * OpenAI Responses endpoint — CreateClient and Call specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/openai_responses.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"

#include "../../interface/core/error.hpp"

#include <format>
#include <string>
#include <utility>


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


template <>
typename openai::Responses::Response_t DeserializeAndRelease<openai::Responses>(Orchestrator* orch, Ticket ticket) {
    const auto& resp = orch->GetResponse(ticket);
    auto data = openai::Deserialize(resp);
    orch->ReleaseSlot(ticket);
    return data;
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


// ----- CallAsync -----

template <>
AsyncResult<openai::Responses::Response_t> CallAsync<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<openai::Responses>(request), policy);
    return AsyncResult<openai::Responses::Response_t>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<openai::Responses>,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<openai::Responses::Response_t> CallCoro<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<openai::Responses>(request), policy);

    while (true) {
        co_await SyncAwaiter{sr.sync.get()};

        if (!sr.sync->succeeded) {
            throw AnnotatedException{sr.sync->error_msg};
        }

        try {
            auto data = Deserialize<openai::Responses>(sr.GetResponse());
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
openai::Responses::Response_t CallSync<openai::Responses>(
    size_t client_id,
    const openai::Responses::Request_t& request,
    const AttemptPolicy& policy)
{
    auto result = CallAsync<openai::Responses>(client_id, request, policy);
    result.RethrowIfException();
    if (result.HasError()) {
        throw AnnotatedException{result.Error()};
    }
    return std::move(result.Data());
}


}
