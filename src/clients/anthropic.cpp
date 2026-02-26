/***
 * Anthropic client — implementation.
 *
 * Protocol-level helpers (URL building, header construction, model grouping)
 * are file-local. Serialize/Deserialize are defined in the separate
 * src/protocols/serialize/ and src/protocols/deserialize/ translation units.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/clients/anthropic.hpp"

#include "../orchestrator.hpp"
#include "../curl.hpp"
#include "../http.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>


namespace jai::llm::anthropic {


// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Message Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


// ----- ModelGroup (public) -----

std::string ModelGroup(std::string_view model) {
    // Family names are stable across both naming conventions:
    //   New: claude-{family}-{version}[-{date}]
    //   Old: claude-{major}-{minor}-{family}[-{date}]
    // A simple substring check is robust to both.
    if (model.find("opus")   != std::string_view::npos) return "opus";
    if (model.find("sonnet") != std::string_view::npos) return "sonnet";
    if (model.find("haiku")  != std::string_view::npos) return "haiku";
    return std::string{model};
}


namespace {


// ----- Endpoint defaults -----

constexpr std::string_view DEFAULT_ENDPOINT =
    "https://api.anthropic.com/v1/messages";


// ----- Request headers -----

http::RequestHeaders BuildRequestHeaders(const ApiKeyAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"x-api-key", auth.api_key},
        {"anthropic-version", auth.version}
    }};
}


// ----- Auth identity (for QueueKey) -----

std::string AuthIdentity(const ApiKeyAuth& auth) { return auth.api_key; }


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
        .endpoint_url = std::string{DEFAULT_ENDPOINT},
        .model_group = ModelGroup(model)
    });
    registration_index = token.index;
}


// ----- CallAsync -----

Result<Message> Client::CallAsync(const Request& r,
                                  const AttemptPolicy& call_policy) const {
    auto headers = std::visit([](const auto& a) {
        return BuildRequestHeaders(a);
    }, auth);

    http::Request http_request{
        .headers = std::move(headers),
        .method = http::Method::POST,
        .url = std::string{DEFAULT_ENDPOINT},
        .body = Serialize(r)
    };

    auto sync = std::make_shared<ResultSync>();

    size_t ticket = orchestrator.Submit(
        Orchestrator::RegistrationToken{registration_index},
        std::move(http_request),
        call_policy,
        sync
    );

    return Result<Message>{orchestrator, ticket, &Deserialize, std::move(sync)};
}


// ----- CallSync -----

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
