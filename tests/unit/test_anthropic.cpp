#include <vector>
#include <string>
#include <cassert>
#include <print>

#include "../../interface/protocols/anthropic/messages.hpp"
#include "../../src/protocols/anthropic.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple Anthropic Request Serialization...");

    anthropic::Request req;
    req.model = "claude-3-5-sonnet-20240620";
    req.max_tokens = 1024;
    
    anthropic::MessageParam msg;
    msg.role = anthropic::Role::USER;
    msg.content = "Hello, Claude!";
    req.messages.push_back(msg);

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    assert(json_str.find("\"model\":\"claude-3-5-sonnet-20240620\"") != std::string::npos);
    assert(json_str.find("\"max_tokens\":1024") != std::string::npos);
    assert(json_str.find("\"role\":\"user\"") != std::string::npos);
    assert(json_str.find("\"content\":\"Hello, Claude!\"") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

int main() {
    try {
        test_simple_serialization();
        std::println("ALL ANTHROPIC UNIT TESTS PASSED");
        return 0;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    }
}
