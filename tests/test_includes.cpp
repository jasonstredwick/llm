#include <iostream>
#include <zlib.h>
#include <curl/curl.h>
#include <nghttp2/nghttp2.h>

#include "../src/providers/base.hpp"
#include "../src/providers/openai_4.hpp"
#include "../src/providers/openai_4o.hpp"
#include "../src/providers/openai_5.hpp"
#include "../src/providers/anthropic_4_5_opus.hpp"
#include "../src/providers/anthropic_4_5_sonnet.hpp"
#include "../src/providers/gemini_2_5.hpp"
#include "../src/providers/gemini_3.hpp"

int main() {
    std::cout << "Dependency Header Verification:" << std::endl;
    std::cout << " - zlib version: " << ZLIB_VERSION << std::endl;
    std::cout << " - curl version: " << LIBCURL_VERSION << std::endl;
    std::cout << " - nghttp2 version: " << NGHTTP2_VERSION << std::endl;
    
    std::cout << "\nGemini Header Verification:" << std::endl;
    jai::llm::providers::gemini::GenerateContentRequest request_g3;
    std::cout << " - Gemini 3.0 ok." << std::endl;

    jai::llm::providers::gemini_2_5::GenerateContentRequest request_g25;
    std::cout << " - Gemini 2.5 ok." << std::endl;

    std::cout << "\nOpenAI Header Verification:" << std::endl;
    jai::llm::providers::openai_5::ChatCompletionRequest req_oa5;
    std::cout << " - OpenAI 5.2 ok." << std::endl;

    std::cout << "\nAnthropic 4.5 Header Verification:" << std::endl;
    
    // Opus 4.5
    jai::llm::providers::anthropic_4_5_opus::MessageRequest req_opus;
    req_opus.model = "claude-opus-4-5";
    req_opus.thinking = {.budget_tokens = 4000};
    req_opus.effort = "high";
    std::cout << " - Anthropic Opus 4.5 ok (Effort: " << *req_opus.effort << ")." << std::endl;

    // Sonnet 4.5
    jai::llm::providers::anthropic_4_5_sonnet::MessageRequest req_sonnet;
    req_sonnet.model = "claude-sonnet-4-5";
    req_sonnet.thinking = {.budget_tokens = 2000};
    std::cout << " - Anthropic Sonnet 4.5 ok." << std::endl;

    std::cout << "\nSUCCESS: All providers (Gemini/OpenAI/Anthropic) initialized correctly." << std::endl;
    return 0;
}
