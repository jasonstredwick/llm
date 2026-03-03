/***
 * Anthropic Messages endpoint — CreateClient and Call specializations.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/endpoints/anthropic_messages.hpp"
#include "../../interface/core/auth.hpp"
#include "../../interface/core/call.hpp"

#include "../instance_impl.hpp"
#include "../results.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>


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


// Deserialize from a live slot (does NOT release — caller handles release).
template <>
typename anthropic::Messages::Response_t DeserializeAndRelease<anthropic::Messages>(Orchestrator* orch, Ticket ticket) {
    return anthropic::Deserialize(orch->GetResponse(ticket));
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


// ----- Envelope metadata extraction -----

template <>
TokenUsage ExtractUsage<anthropic::Messages>(const anthropic::Message& msg) {
    TokenUsage usage{};
    if (msg.usage) {
        auto& u = *msg.usage;
        if (u.input_tokens)               usage.input_tokens = static_cast<int64_t>(*u.input_tokens);
        if (u.output_tokens)              usage.output_tokens = static_cast<int64_t>(*u.output_tokens);
        if (u.cache_creation_input_tokens) usage.cache_creation_tokens = static_cast<int64_t>(*u.cache_creation_input_tokens);
        if (u.cache_read_input_tokens)     usage.cache_read_tokens = static_cast<int64_t>(*u.cache_read_input_tokens);
        // Anthropic does not report total_tokens, reasoning_tokens, or tool_use_tokens.
    }
    return usage;
}


template <>
std::optional<std::string> ExtractModel<anthropic::Messages>(const anthropic::Message& msg) {
    if (!msg.model) return std::nullopt;
    return std::visit([](const auto& v) -> std::string {
        using V = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<V, std::string>) {
            return v;
        } else {
            // ModelValues enum — return empty string as placeholder.
            // In practice, JSON deserialization yields the string variant.
            return {};
        }
    }, *msg.model);
}


template <>
std::optional<std::string> ExtractStopReason<anthropic::Messages>(const anthropic::Message& msg) {
    if (!msg.stop_reason) return std::nullopt;
    switch (*msg.stop_reason) {
        case anthropic::Message::StopReason::END_TURN:      return "end_turn";
        case anthropic::Message::StopReason::MAX_TOKENS:    return "max_tokens";
        case anthropic::Message::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case anthropic::Message::StopReason::TOOL_USE:      return "tool_use";
        case anthropic::Message::StopReason::PAUSE_TURN:    return "pause_turn";
        case anthropic::Message::StopReason::REFUSAL:       return "refusal";
    }
    return std::nullopt;
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


// ----- Extract wrapper (bridges internal extraction into ExtractFn pointer) -----

namespace {

void ExtractEnvelope(const anthropic::Message& data, AttemptMetadata& am) {
    am.usage.emplace(ExtractUsage<anthropic::Messages>(data));
    am.model = ExtractModel<anthropic::Messages>(data);
    am.stop_reason = ExtractStopReason<anthropic::Messages>(data);
}

} // anonymous namespace


// ----- CallAsync -----

template <>
AsyncResult<anthropic::Messages> CallAsync<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<anthropic::Messages>(request), policy);
    return AsyncResult<anthropic::Messages>{
        *sr.orchestrator, sr.ticket,
        &DeserializeAndRelease<anthropic::Messages>,
        &ExtractEnvelope,
        std::move(sr.sync)};
}


// ----- CallCoro -----

template <>
CoroAsyncResult<anthropic::Messages> CallCoro<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto sr = SubmitRequest(client_id, Serialize<anthropic::Messages>(request), policy);
    Result<anthropic::Messages, void> result;

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
            result.data.emplace(Deserialize<anthropic::Messages>(response));
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
Result<anthropic::Messages, void> CallSync<anthropic::Messages>(
    size_t client_id,
    const anthropic::Messages::Request_t& request,
    const AttemptPolicy& policy)
{
    auto ar = CallAsync<anthropic::Messages>(client_id, request, policy);
    return ar.Take();
}


}
