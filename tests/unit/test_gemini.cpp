#include <vector>
#include <string>
#include <cassert>
#include <print>

#include "../../interface/protocols/gemini/generate_content.hpp"
#include "../../src/protocols/gemini.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple Gemini Request Serialization...");

    gemini::Request req;
    
    gemini::RequestContent content;
    content.role = gemini::Role::USER;
    
    gemini::RequestContent::RequestPart part;
    part.data = gemini::Text{"Hello, Gemini!"};
    content.parts.push_back(part);
    
    req.contents.push_back(content);

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    assert(json_str.find("\"role\":\"user\"") != std::string::npos);
    assert(json_str.find("\"text\":\"Hello, Gemini!\"") != std::string::npos);
    assert(json_str.find("\"parts\":[") != std::string::npos);
    assert(json_str.find("\"contents\":[") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

int main() {
    try {
        test_simple_serialization();
        std::println("ALL GEMINI UNIT TESTS PASSED");
        return 0;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
