/***
 * Provider-agnostic bridge functions — Call and CreateClient.
 *
 * Internal plumbing. Users interact via Instance::CreateClient and
 * ClientHandle::CallAsync / CallCoro / CallSync; these free-function
 * templates are the customization points that endpoint translation
 * units specialize.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include <cstddef>
#include <string>

#include "policy.hpp"
#include "results.hpp"


namespace jai::llm {


template <typename Endpoint, typename Auth>
size_t CreateClientImpl(Auth auth, std::string model, const ClientPolicy& policy);

template <typename Endpoint>
AsyncResult<typename Endpoint::Response_t> CallAsync(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);

template <typename Endpoint>
CoroAsyncResult<typename Endpoint::Response_t> CallCoro(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);

template <typename Endpoint>
typename Endpoint::Response_t CallSync(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);


}
