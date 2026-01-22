#pragma once

#include "../../interface/providers/gemini_2_5.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::curl { struct Response; }

namespace jai::llm::gemini_2_5 {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
