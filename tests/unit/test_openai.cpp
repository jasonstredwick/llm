#include <vector>
#include <string>
#include <cassert>
#include <print>

#include "../../interface/protocols/openai/responses.hpp"
#include "../../src/protocols/openai.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple OpenAI Request Serialization...");

    openai::Request req;
    req.model = "gpt-4o";
    req.input = "Hello, OpenAI!";

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    assert(json_str.find("\"model\":\"gpt-4o\"") != std::string::npos);
    assert(json_str.find("\"input\":\"Hello, OpenAI!\"") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

int main() {
    try {
        test_simple_serialization();
        std::println("ALL OPENAI UNIT TESTS PASSED");
        return 0;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
