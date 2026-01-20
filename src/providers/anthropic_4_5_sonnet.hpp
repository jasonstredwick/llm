#pragma once

#include "../../interface/providers/anthropic_4_5_sonnet.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::anthropic_4_5_sonnet {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const std::vector<std::byte>& raw_response_bytes);
std::vector<std::byte> Serialize(const Request&);


}
