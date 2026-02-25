#pragma once

#include "../../interface/protocols/gemini/generate_content.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>


namespace jai::llm::curl { struct Response; }


namespace jai::llm::gemini {


// Normalize a model string to its rate limit group.
// Gemini has per-model rate limits (no shared families), so the model
// name is returned with the version stripped but variant preserved.
//   "gemini-2.5-pro-preview-05-06" → "gemini-2.5-pro"
//   "gemini-2.5-flash"             → "gemini-2.5-flash"
//   "gemini-2.5-flash-lite"        → "gemini-2.5-flash-lite"
// Returns the base model identifier.
std::string ModelGroup(std::string_view model);

http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
