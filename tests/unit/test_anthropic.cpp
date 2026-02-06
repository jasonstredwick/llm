#include <vector>
#include <string>
#include <cassert>
#include <print>
#include <cstring>
#include <simdjson.h>

#include "../../interface/protocols/anthropic/messages.hpp"
#include "../../src/protocols/anthropic.hpp"
#include "../../src/curl.hpp"

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

void test_complex_serialization() {
    std::println("Testing Complex Anthropic Request Serialization...");

    anthropic::Request req;
    req.model = "claude-3-5-sonnet-20240620";
    req.max_tokens = 4096;
    req.temperature = 0.7;
    
    // System prompt
    req.system = "You are a helpful assistant.";

    // Metadata
    anthropic::Metadata meta;
    meta.user_id = "user_1234";
    req.metadata = meta;

    // Messages
    anthropic::MessageParam msg1;
    msg1.role = anthropic::Role::USER;
    msg1.content = "What is the weather?";
    req.messages.push_back(msg1);

    anthropic::MessageParam msg2;
    msg2.role = anthropic::Role::ASSISTANT;
    msg2.content = "I need to check the weather. Which city?";
    req.messages.push_back(msg2);

    // Tools
    anthropic::Tool get_weather;
    get_weather.name = "get_weather";
    get_weather.description = "Get the current weather in a given location";
    
    anthropic::Tool::InputSchema schema;
    schema.type = anthropic::KindJsonSchemaType{};
    
    json::Object properties;
    
    json::Object location_prop;
    location_prop["type"] = "string";
    location_prop["description"] = "The city and state, e.g. San Francisco, CA";
    
    properties["location"] = location_prop;
    schema.properties = properties;
    schema.required = std::vector<std::string>{"location"};
    
    get_weather.input_schema = schema;
    
    req.tools = std::vector<anthropic::ToolUnion>{get_weather};

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    assert(json_str.find("\"system\":\"You are a helpful assistant.\"") != std::string::npos);
    assert(json_str.find("\"metadata\":{\"user_id\":\"user_1234\"}") != std::string::npos);
    assert(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    assert(json_str.find("\"location\":{\"type\":\"string\"") != std::string::npos);
    assert(json_str.find("\"temperature\":0.7") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_part_serialization() {
    std::println("Testing Anthropic Part Serialization...");

    anthropic::Request req;
    req.model = "claude-3-5-sonnet-20240620";
    req.max_tokens = 4096;

    anthropic::MessageParam msg;
    msg.role = anthropic::Role::USER;

    // Text part
    anthropic::TextBlockParam text_part;
    text_part.text = "Here is an image and a tool result.";
    
    // Image part
    anthropic::ImageBlockParam image_part;
    anthropic::Base64ImageSource img_src;
    img_src.data = "YmFzZTY0X2RhdGE=";
    img_src.media_type = anthropic::ImageMediaType::IMAGE_PNG;
    image_part.source = img_src;

    // Tool result part
    anthropic::ToolResultBlockParam tool_result;
    tool_result.tool_use_id = "tool_123";
    tool_result.content = "Tool execution successful.";

    msg.content = std::vector<anthropic::ContentBlockParam>{text_part, image_part, tool_result};
    req.messages.push_back(msg);

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    assert(json_str.find("\"type\":\"text\"") != std::string::npos);
    assert(json_str.find("\"type\":\"image\"") != std::string::npos);
    assert(json_str.find("\"type\":\"tool_result\"") != std::string::npos);
    assert(json_str.find("\"tool_use_id\":\"tool_123\"") != std::string::npos);
    assert(json_str.find("\"media_type\":\"image/png\"") != std::string::npos);

    std::println("[SUCCESS] Part Serialization passed.");
}

void test_response_deserialization() {
    std::println("Testing Anthropic Response Deserialization...");

    std::string json_response = R"({
        "id": "msg_01Xv6Xk",
        "type": "message",
        "role": "assistant",
        "model": "claude-3-5-sonnet-20240620",
        "stop_reason": "tool_use",
        "usage": {
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "input_tokens": 15,
            "output_tokens": 40,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        },
        "content": [
            {
                "type": "text",
                "text": "Hello! I can help with that.",
                "citations": []
            },
            {
                "type": "tool_use",
                "id": "toolu_01A09z9HS",
                "name": "get_weather",
                "input": {"location": "San Francisco, CA"}
            }
        ]
    })";


    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    assert(resp.id == "msg_01Xv6Xk");
    assert(resp.model == "claude-3-5-sonnet-20240620");
    assert(resp.content.size() == 2);
    
    // Check text block
    auto* text_block = std::get_if<anthropic::TextBlock>(&resp.content[0]);
    assert(text_block != nullptr);
    assert(text_block->text == "Hello! I can help with that.");

    // Check tool use block
    auto* tool_use = std::get_if<anthropic::ToolUseBlock>(&resp.content[1]);
    assert(tool_use != nullptr);
    assert(tool_use->name == "get_weather");
    assert(tool_use->id == "toolu_01A09z9HS");
    assert(std::get<std::string>(tool_use->input.at("location").data) == "San Francisco, CA");

    assert(resp.stop_reason == anthropic::StopReason::TOOL_USE);
    assert(resp.usage.input_tokens == 15);
    assert(resp.usage.output_tokens == 40);

    std::println("[SUCCESS] Response Deserialization passed.");
}

int main() {
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_part_serialization();
        test_response_deserialization();
        std::println("ALL ANTHROPIC UNIT TESTS PASSED");
        return 0;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    }
}
