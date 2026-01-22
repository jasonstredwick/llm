#pragma once

#include "../../interface/providers/anthropic_4_5_opus.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::curl { struct Response; }

namespace jai::llm::anthropic_4_5_opus {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
