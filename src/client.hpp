/***
 * Client — internal client record stored by Instance.
 *
 * Holds the orchestrator registration index and the pre-computed
 * request details (headers, URL, model) needed to submit HTTP requests.
 * Constructed by per-endpoint specializations and stored as
 * unique_ptr<Client> in Instance::Impl.
 *
 * Also declares the per-endpoint customization point templates.
 * Each endpoint .cpp provides explicit specializations.
 *
 * NOT a public header. Must not be included from interface/.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "curl.hpp"
#include "http.hpp"
#include "orchestrator.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace jai::llm {


// ----- Per-endpoint customization points -----
// Primary template declarations. Specializations live in
// per-endpoint translation units (src/endpoints/).

template <typename Endpoint, typename Auth>
std::string AuthIdentity(const Auth&);

template <typename Endpoint, typename Auth>
http::RequestHeaders BuildRequestHeaders(const Auth&);

template <typename Endpoint, typename Auth>
std::string BuildUrl(const Auth&, std::string_view model);

template <typename Endpoint>
typename Endpoint::Response_t Deserialize(const curl::Response&);

template <typename Endpoint>
typename Endpoint::Response_t DeserializeAndRelease(Orchestrator* orch, size_t ticket);

template <typename Endpoint, typename Auth>
std::string EndpointBase(const Auth&);

template <typename Endpoint>
std::string ModelGroup(std::string_view model);

template <typename Endpoint>
std::vector<std::byte> Serialize(const typename Endpoint::Request_t&);


// ----- Client identity -----
// Uniquely identifies a client by its queue placement and policy.
// Two clients with the same ClientKey are the same logical client.

struct ClientKey {
    QueueKey queue_key;
    ClientPolicy policy;

    friend auto operator<=>(const ClientKey&, const ClientKey&) = default;
    friend bool operator==(const ClientKey&, const ClientKey&) = default;
};


// ----- Client record -----

struct Client {
    size_t registration_index;
    std::string model;
    http::RequestHeaders request_headers;
    std::string url;
};


// ----- Bridge functions -----
// Defined in instance.cpp. Allow endpoint .cpp files to interact
// with the orchestrator and client storage without including
// instance_impl.hpp.

// Returns existing client_id if a client with the same key exists,
// otherwise registers with the orchestrator, constructs and stores
// the client, and returns the new client_id. Thread-safe.
//
// Takes the five values that only endpoint-specific code can compute:
//   auth_identity, endpoint_url, model_group — form the QueueKey
//   headers, url — pre-computed request details for the Client
// Plus model and policy which pass through from the caller.
size_t FindOrCreateClient(std::string auth_identity, std::string endpoint_url,
                          std::string model_group, const ClientPolicy& policy,
                          std::string model, http::RequestHeaders headers,
                          std::string url);


// Forward declaration — defined in instance_impl.hpp where Orchestrator
// is a complete type, allowing inline method definitions.
struct SubmitResult;

SubmitResult SubmitRequest(size_t client_id, std::vector<std::byte> body,
                           const AttemptPolicy& policy);


}
