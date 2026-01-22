#pragma once

#include "../../interface/providers/openai_4o.hpp"

#include "../http.hpp"

#include <cstddef>
#include <string>
#include <vector>


namespace jai::llm::curl { struct Response; }

namespace jai::llm::openai_4o {


http::Method GenMethod(const Request&);
http::RequestHeaders GenRequestHeaders(const Request&);
std::string GenUrl(const Request&);
Response Deserialize(const curl::Response& response);
std::vector<std::byte> Serialize(const Request&);


}
