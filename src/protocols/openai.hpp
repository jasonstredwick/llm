#pragma once

#include "../../interface/protocols/openai/responses.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>


namespace jai::llm::curl { struct Response; }


namespace jai::llm::openai {


// Normalize a model string to its rate limit group.
// OpenAI groups models into "shared limit" families.
//   "gpt-4o-2024-08-06" → "gpt-4o"
//   "o3-2025-04-16"     → "o3"
//   "o1-mini"           → "o1-mini"
// Returns the base model family name (strips date suffixes).
std::string ModelGroup(std::string_view model);

http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
