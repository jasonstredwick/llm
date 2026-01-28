#include "gemini.hpp"

#include <format>
#include <string_view>

#include <simdjson.h>

#include "../curl.hpp"


namespace jai::llm::gemini {


constexpr std::string_view ENDPOINT_BASE = "https://generativelanguage.googleapis.com/v1beta/models/{MODEL_ID}:generateContent";
constexpr std::string_view ENDPOINT_VERTEX_BASE = "https://{REGION}-aiplatform.googleapis.com/v1/projects/{PROJECT_ID}/locations/{REGION}/publishers/google/models/{MODEL_ID}:generateContent";
constexpr http::Method ENDPOINT_METHOD = http::Method::POST;


http::Method GenMethod(const Request&) { return ENDPOINT_METHOD; }


http::RequestHeaders GenRequestHeaders(const Request&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"}
    }};
}


std::string GenUrl(const Request& r) {
    const std::string model_id;
    return std::format(ENDPOINT_BASE, model_id);
}


}
