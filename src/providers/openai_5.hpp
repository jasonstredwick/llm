#pragma once

#include "../../interface/providers/openai_5.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::openai_5 {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const std::vector<std::byte>& raw_response_bytes);
std::vector<std::byte> Serialize(const Request&);


}
