#pragma once


#include "policy.hpp"
#include "../core/async.hpp"
#include "../protocols/gemini/generate_content.hpp"



namespace jai::llm::gemini {


/***
 * Client
 */
class Client {
private:
    ClientPolicy policy;

public:
    Client(const ClientPolicy& client_policy) : policy{client_policy} {}
    Client(ClientPolicy&& client_policy) : policy{std::move(client_policy)} {}
    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = default;
    Client& operator=(Client&&) noexcept = default;

    AsyncTask<Response> CallAsync(const Request& r) const;
    Response CallSync(const Request& r) const;
};


}
