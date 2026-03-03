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

// ----- Tier 1: direct (per-endpoint specializations in src/endpoints/) -----

template <typename Endpoint>
AsyncResult<Endpoint> CallAsync(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);

template <typename Endpoint>
CoroAsyncResult<Endpoint> CallCoro(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);

template <typename Endpoint>
Result<Endpoint, void> CallSync(size_t client_id, const typename Endpoint::Request_t&, const AttemptPolicy&);


// ----- Tier 2: user transform (generic, wraps Tier 1) -----

template <typename Endpoint, typename Data>
AsyncResult<Endpoint, Data> CallAsync(size_t client_id,
                                       const typename Endpoint::Request_t& request,
                                       Data (*transform)(const typename Endpoint::Response_t&),
                                       const AttemptPolicy& policy) {
    return AsyncResult<Endpoint, Data>{
        CallAsync<Endpoint>(client_id, request, policy),
        transform
    };
}

template <typename Endpoint, typename Data>
CoroAsyncResult<Endpoint, Data> CallCoro(size_t client_id,
                                          const typename Endpoint::Request_t& request,
                                          Data (*transform)(const typename Endpoint::Response_t&),
                                          const AttemptPolicy& policy) {
    Result<Endpoint, void> tier1 = co_await CallCoro<Endpoint>(client_id, request, policy);
    co_return TransformResult<Endpoint, Data>(std::move(tier1), transform);
}

template <typename Endpoint, typename Data>
Result<Endpoint, Data> CallSync(size_t client_id,
                                 const typename Endpoint::Request_t& request,
                                 Data (*transform)(const typename Endpoint::Response_t&),
                                 const AttemptPolicy& policy) {
    return TransformResult<Endpoint, Data>(
        CallSync<Endpoint>(client_id, request, policy),
        transform
    );
}


}
