/***
 * OpenAI client — implementation.
 *
 * Protocol-level helpers (URL building, header construction, model grouping)
 * are file-local. Serialize/Deserialize are defined in the separate
 * src/protocols/serialize/ and src/protocols/deserialize/ translation units.
 *
 * @author jason.stredwick@gmail.com
 */

#include "../../interface/clients/openai.hpp"

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


namespace jai::llm::openai {


// Forward declarations — defined in src/protocols/{serialize,deserialize}/.
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


// ----- ModelGroup (public) -----

std::string ModelGroup(std::string_view model) {
    // Date suffix pattern: -DDDD-DD-DD at the end (exactly 11 chars).
    // Verify all digits are in the right positions before stripping.
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


namespace {


// ----- Endpoint defaults -----

constexpr std::string_view DEFAULT_ENDPOINT =
    "https://api.openai.com/v1/responses";


// ----- Request headers -----

http::RequestHeaders BuildRequestHeaders(const ApiKeyAuth& auth) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"Authorization", std::format("Bearer {}", auth.api_key)}
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

Result<Response> Client::CallAsync(const Request& r,
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

    return Result<Response>{orchestrator, ticket, &Deserialize, std::move(sync)};
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


}
