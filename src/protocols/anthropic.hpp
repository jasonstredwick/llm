#pragma once

#include "../../interface/protocols/anthropic/messages.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>


namespace jai::llm::curl { struct Response; }


namespace jai::llm::anthropic {


// Normalize a model string to its rate limit family.
// Anthropic groups rate limits by model family (e.g., all Opus versions
// share one pool, all Sonnet versions share another).
//   "claude-opus-4-20250514"   → "opus"
//   "claude-sonnet-4-20250514" → "sonnet"
//   "claude-haiku-4-20250514"  → "haiku"
// Returns the model string as-is if the family cannot be determined.
std::string ModelGroup(std::string_view model);

http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Message Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
