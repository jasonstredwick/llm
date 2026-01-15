#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../interface/policy.hpp"
#include "../interface/http.hpp"
#include "curl.hpp"
#include "http.hpp"
#include "memory.hpp"


namespace jai::llm {


class Orchestrator {
private:
    SlotPool<Attempt> attempt_pool;
    curl::Interface interface;

public:
    explicit Orchestrator(const ConnectionPolicy& policy) : interface{policy} {}

private:
    Attempt MakeAttempt(const AttemptPolicy& policy,
                        http::Method method,
                        const std::string& url,
                        const HeaderList& header_list,
                        const std::vector<std::byte>& body) const
    {
        return curl::Attempt{interface, policy, method, url, header_list, body};
    }

    std::vector<Attempt*> SyncAttempt(Attempt& attempt);
};


}
