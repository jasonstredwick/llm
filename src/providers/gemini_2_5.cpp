#include "gemini_2_5.hpp"

#include <string_view>


namespace jai::llm::gemini_2_5 {


constexpr std::string_view ENDPOINT_BASE = "https://generativelanguage.googleapis.com/v1beta/models/{MODEL_ID}:generateContent";
constexpr std::string_view ENDPOINT_VERTEX_BASE = "https://{REGION}-aiplatform.googleapis.com/v1/projects/{PROJECT_ID}/locations/{REGION}/publishers/google/models/{MODEL_ID}:generateContent";


}
