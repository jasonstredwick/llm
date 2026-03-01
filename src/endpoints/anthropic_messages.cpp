/***
 * Anthropic Messages endpoint — CreateClient and Call specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/anthropic_messages.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"

#include "../../interface/core/error.hpp"

#include <string>
#include <utility>


namespace jai::llm {


namespace anthropic {

// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Message Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);

}


// ----- Endpoint defaults -----

constexpr std::string_view ANTHROPIC_ENDPOINT =
    "https://api.anthropic.com/v1/messages";


// ----- Functionality -----

// ----- Auth identity (for QueueKey) -----
template <>
std::string AuthIdentity<anthropic::Messages>(const anthropic::ApiKeyAuth& auth) {
    return auth.api_key;
}


template <>
http::RequestHeaders BuildRequestHeaders<anthropic::Messages>(const anthropic::ApiKeyAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"x-api-key", auth.api_key},
        {"anthropic-version", auth.version}
    }};
}


template <>
std::string BuildUrl<anthropic::Messages>(const anthropic::ApiKeyAuth&, std::string_view) {
    return std::string{ANTHROPIC_ENDPOINT};
}


template <>
typename anthropic::Messages::Response_t Deserialize<anthropic::Messages>(const curl::Response& curl_response) {
    return anthropic::Deserialize(curl_response);
}


template <>
typename anthropic::Messages::Response_t DeserializeAndRelease<anthropic::Messages>(Orchestrator* orch, size_t ticket) {
    const auto& resp = orch->GetResponse(ticket);
    auto data = anthropic::Deserialize(resp);
    orch->ReleaseSlot(ticket);
    return data;
}


template <>
std::string EndpointBase<anthropic::Messages>(const anthropic::ApiKeyAuth&) {
    return std::string{ANTHROPIC_ENDPOINT};
}


template <>
std::string ModelGroup<anthropic::Messages>(std::string_view model) {
    if (model.find("opus")   != std::string_view::npos) return "opus";
    if (model.find("sonnet") != std::string_view::npos) return "sonnet";
    if (model.find("haiku")  != std::string_view::npos) return "haiku";
    return std::string{model};
}


template <>
std::vector<std::byte> Serialize<anthropic::Messages>(const anthropic::Messages::Request_t& request) {
    return anthropic::Serialize(request);
}


// ----- CreateClientImpl specialization -----

template <>
size_t CreateClientImpl<anthropic::Messages, anthropic::ApiKeyAuth>(
    anthropic::ApiKeyAuth auth, std::string model, const ClientPolicy& policy)
{
    return FindOrCreateClient(
        AuthIdentity<anthropic::Messages>(auth),
        EndpointBase<anthropic::Messages>(auth),
        ModelGroup<anthropic::Messages>(model),
        policy,
        std::move(model),
        BuildRequestHeaders<anthropic::Messages>(auth),
        BuildUrl<anthropic::Messages>(auth, model));
}


// ----- CallAsync -----

template <>
AsyncResult<anthropic::Messages::Response_t> CallAsync<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<anthropic::Messages>(request), policy);
    return AsyncResult<anthropic::Messages::Response_t>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<anthropic::Messages>,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<anthropic::Messages::Response_t> CallCoro<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<anthropic::Messages>(request), policy);

    while (true) {
        co_await SyncAwaiter{sr.sync};

        if (!sr.sync->succeeded) {
            throw AnnotatedException{sr.sync->error_msg};
        }

        try {
            auto data = Deserialize<anthropic::Messages>(sr.GetResponse());
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
anthropic::Messages::Response_t CallSync<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto result = CallAsync<anthropic::Messages>(client_id, request, policy);
    result.RethrowIfException();
    if (result.HasError()) {
        throw AnnotatedException{result.Error()};
    }
    return std::move(result.Data());
}


}
