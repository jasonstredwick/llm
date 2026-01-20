#pragma once

#include "../../interface/providers/gemini_2_5.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::gemini_2_5 {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const std::vector<std::byte>& raw_response_bytes);
std::vector<std::byte> Serialize(const Request&);


}
