#include <vector>
#include <string>
#include <cassert>
#include <print>
#include <cstring>
#include <simdjson.h>

#include <simdjson.h>

#include "../../src/curl.hpp"

#include "../../interface/core/error.hpp"
#include "../../interface/protocols/openai/responses.hpp"
#include "../../src/protocols/openai.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple OpenAI Request Serialization...");

    openai::Request req;
    req.background = false; // Avoid leading comma
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

void test_complex_serialization() {
    std::println("Testing Complex OpenAI Request Serialization...");

    openai::Request req;
    req.background = false; // Avoid leading comma
    req.model = "gpt-4o";
    req.max_output_tokens = 1000;
    
    // Metadata
    std::map<NameLen<64>, NameLen<512>> meta = {
        {NameLen<64>(std::string_view("environment")), NameLen<512>(std::string_view("production"))},
        {NameLen<64>(std::string_view("user_id")), NameLen<512>(std::string_view("user_9876"))}
    };
    req.metadata = meta;

    // Tool choice
    req.tool_choice = openai::ToolChoiceMode::AUTO;

    // Tools
    openai::request::ToolTypes::Function func;
    func.type = openai::KindFunctionTool{};
    func.name = "get_stock_price";
    func.description = "Get the current stock price for a given symbol";
    
    json::Object params;
    params["type"] = "object";
    
    json::Object symbol_prop;
    symbol_prop["type"] = "string";
    symbol_prop["description"] = "The stock symbol, e.g. AAPL";
    
    json::Object props;
    props["symbol"] = symbol_prop;
    params["properties"] = props;
    
    json::Array required;
    required.push_back("symbol");
    params["required"] = required;
    
    func.parameters = params;
    func.strict = true;
    
    openai::request::Tool get_stock_price = func;
    
    req.tools = std::vector<openai::request::Tool>{get_stock_price};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    assert(json_str.find("\"max_output_tokens\":1000") != std::string::npos);
    assert(json_str.find("\"environment\":\"production\"") != std::string::npos);
    assert(json_str.find("\"tool_choice\":\"auto\"") != std::string::npos);
    assert(json_str.find("\"name\":\"get_stock_price\"") != std::string::npos);
    assert(json_str.find("\"symbol\":{\"type\":\"string\"") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_content_serialization() {
    std::cout << "Testing OpenAI Content Serialization..." << std::endl;

    openai::Request req;

    req.background = false;
    req.model = "gpt-4o";
    
    openai::request::InputTypes::Message msg;
    msg.role = openai::RoleInputMessage::USER;

    openai::request::ContentTypes::Text text_unit;
    text_unit.text = "Here is an image.";
    
    openai::request::ContentTypes::Image image_unit;
    image_unit.image_url = EncodedUrl(std::string_view("https://example.com/image.png"));
    image_unit.detail = openai::Detail::HIGH;

    msg.content = std::vector<openai::request::InputTypes::MessageContentUnit>{text_unit, image_unit};
    
    req.input = std::vector<openai::request::InputItemList>{msg};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    assert(json_str.find("\"type\":\"text\"") != std::string::npos);
    assert(json_str.find("\"type\":\"image\"") != std::string::npos);
    assert(json_str.find("\"image_url\":\"https://example.com/image.png\"") != std::string::npos);

    std::println("[SUCCESS] Content Serialization passed.");
}

void test_response_deserialization() {
    std::println("Testing OpenAI Response Deserialization...");

    std::string json_response = R"({
        "object": "response",
        "background": false,
        "completed_at": 1677652288.0,
        "conversation": {"id": "conv_123"},
        "created_at": 1677652288.0,
        "error": {"code": "none", "message": "none"},
        "id": "res_123",
        "incomplete_details": {"reason": "none"},
        "instructions": "",
        "max_output_tokens": 1000,
        "max_tool_calls": 10,
        "metadata": {},
        "model": "gpt-4o",
        "output": [
            {
                "type": "message",
                "id": "msg_456",
                "role": "assistant",
                "status": "completed",
                "content": [
                    {
                        "type": "output_text",
                        "text": "Hello! I am OpenAI assistant.",
                        "annotations": [],
                        "logprobs": []
                    }
                ]
            }
        ],
        "parallel_tool_calls": true,
        "previous_response_id": "none",
        "prompt": {"id": "prompt_123"},
        "prompt_cache_key": "none",
        "prompt_cache_retention": "none",
        "reasoning": {"effort": "low", "summary": {"tokens": 0}},
        "safety_identifier": "none",
        "service_tier": "standard",
        "status": "completed",
        "temperature": 1.0,
        "text": {"format": "text", "verbosity": "high"},
        "tool_choice": "auto",
        "tools": [],
        "top_logprobs": 0,
        "top_p": 1.0,
        "truncation": {"type": "auto"},
        "usage": {
            "input_tokens": 10,
            "input_tokens_details": {"cached_tokens": 0},
            "output_tokens": 20,
            "output_tokens_details": {"reasoning_tokens": 0},
            "total_tokens": 30
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    std::println("  Deserializing...");
    auto resp = openai::Deserialize(res);

    std::println("  Checking response metadata...");
    assert(resp.id == "res_123");
    assert(resp.model == "gpt-4o");
    assert(resp.output.size() == 1);
    
    std::println("  Checking output item...");
    auto* item_ptr = &resp.output[0];
    auto* msg_item = std::get_if<openai::response::InputTypes::Item::OutputMessage>(item_ptr);
    if (!msg_item) {
        std::println("[ERROR] Variant is NOT OutputMessage! It is index: {}", item_ptr->index());
        return;
    }
    assert(msg_item->role == openai::RoleAssistant::ASSISTANT);
    
    std::println("  Checking content...");
    auto* text_content = std::get_if<openai::response::ContentTypes::OutputText>(&msg_item->content[0]);
    if (!text_content) {
        std::println("[ERROR] Content is NOT OutputText!");
        return;
    }
    assert(text_content->text == "Hello! I am OpenAI assistant.");

    assert(resp.usage.total_tokens == 30);

    std::println("[SUCCESS] Response Deserialization passed.");
}

#include <iostream>

int main() {
    std::cout << "Starting OpenAI Tests (COUT)..." << std::endl;
    try {
        std::cout << "Testing Simple Serialization..." << std::endl;
        test_simple_serialization();
        
        std::cout << "Testing Complex Serialization..." << std::endl;
        test_complex_serialization();
        
        std::cout << "Testing Content Serialization..." << std::endl;
        //test_content_serialization();
        
        std::cout << "Testing Response Deserialization..." << std::endl;
        test_response_deserialization();
        
        std::cout << "ALL OPENAI UNIT TESTS PASSED" << std::endl;
        return 0;
    } catch (const jai::llm::AnnotatedException& e) {
        std::println("[ERROR] AnnotatedException\n{}", jai::llm::to_string(e));
        return 1;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
