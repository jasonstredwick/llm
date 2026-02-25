/***
 * Anthropic client — implementation.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/clients/anthropic.hpp"

#include "../orchestrator.hpp"
#include "../protocols/anthropic.hpp"

#include <utility>


namespace jai::llm::anthropic {


//----- Construction -----

Client::Client(Orchestrator& orchestrator_,
               std::string api_key_,
               std::string model_)
    : Client{orchestrator_, std::move(api_key_), std::move(model_), ClientPolicy{}}
{}


Client::Client(Orchestrator& orchestrator_,
               std::string api_key_,
               std::string model_,
               const ClientPolicy& client_policy)
    : orchestrator{orchestrator_}
    , api_key{std::move(api_key_)}
    , model{std::move(model_)}
{
    auto token = orchestrator.Register(client_policy, QueueKey{
        .auth_identity = api_key,
        .endpoint_url = endpoint_url,
        .model_group = ModelGroup(model)
    });
    registration_index = token.index;
}


Client::Client(Orchestrator& orchestrator_,
               std::string api_key_,
               std::string model_,
               const ClientPolicy& client_policy,
               std::string endpoint_url_)
    : orchestrator{orchestrator_}
    , api_key{std::move(api_key_)}
    , model{std::move(model_)}
    , endpoint_url{std::move(endpoint_url_)}
{
    auto token = orchestrator.Register(client_policy, QueueKey{
        .auth_identity = api_key,
        .endpoint_url = endpoint_url,
        .model_group = ModelGroup(model)
    });
    registration_index = token.index;
}


//----- CallAsync -----

Result<Message> Client::CallAsync(const Request& r,
                                  const AttemptPolicy& call_policy) const {
    // Build the HTTP request from protocol-specific functions.
    // TODO: GenRequestHeaders/GenUrl need api_key, model, and endpoint_url
    //       to inject auth headers and build the full URL. Update Gen*
    //       signatures to accept client config once the protocol layer is
    //       finalized.
    http::Request http_request{
        .headers = GenRequestHeaders(r),
        .method = GenMethod(r),
        .url = GenUrl(r),
        .body = Serialize(r)
    };

    // Create the sync block first — it's heap-stable and shared between
    // the Result and the orchestrator's slot.
    auto sync = std::make_shared<ResultSync>();

    size_t ticket = orchestrator.Submit(
        Orchestrator::RegistrationToken{registration_index},
        std::move(http_request),
        call_policy,
        sync
    );

    return Result<Message>{orchestrator, ticket, &Deserialize, std::move(sync)};
}


//----- CallSync -----

Message Client::CallSync(const Request& r,
                         const AttemptPolicy& call_policy) const {
    auto result = CallAsync(r, call_policy);

    result.RethrowIfException();

    if (result.HasError()) {
        throw AnnotatedException{result.Error()};
    }

    return std::move(result.Data());
}


}
