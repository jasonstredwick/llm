#include <vector>
#include <string>
#include <cassert>
#include <print>
#include <cstring>
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
    std::println("Testing OpenAI Content Serialization...");

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
        "reasoning": {"effort": "low", "summary": "auto"},
        "safety_identifier": "none",
        "service_tier": "default",
        "status": "completed",
        "temperature": 1.0,
        "text": {"format": {"type": "text"}, "verbosity": "high"},
        "tool_choice": "auto",
        "tools": [],
        "top_logprobs": 0,
        "top_p": 1.0,
        "truncation": "auto",
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

    auto resp = openai::Deserialize(res);

    assert(resp.id == "res_123");
    assert(resp.model == "gpt-4o");
    assert(resp.output.size() == 1);
    
    auto* item_ptr = &resp.output[0];
    auto* msg_item = std::get_if<openai::response::InputTypes::Item::OutputMessage>(item_ptr);
    if (!msg_item) {
        std::println("[ERROR] Variant is NOT OutputMessage! It is index: {}", item_ptr->index());
        return;
    }
    assert(msg_item->role == openai::RoleAssistant::ASSISTANT);
    
    auto* text_content = std::get_if<openai::response::ContentTypes::OutputText>(&msg_item->content[0]);
    if (!text_content) {
        std::println("[ERROR] Content is NOT OutputText!");
        return;
    }
    assert(text_content->text == "Hello! I am OpenAI assistant.");

    assert(resp.usage.total_tokens == 30);

    std::println("[SUCCESS] Response Deserialization passed.");
}

void test_openai_doc_examples() {
    std::println("Testing OpenAI Documentation Examples...");

    // 1. Tool Call Response (mapped to semantic model)
    {
        std::string json_response = R"({
            "id": "resp_123",
            "object": "response",
            "background": false,
            "completed_at": 1677652288.0,
            "conversation": {"id": "conv_123"},
            "created_at": 1677652288.0,
            "error": {"code": "none", "message": "none"},
            "incomplete_details": {"reason": "none"},
            "instructions": "",
            "max_output_tokens": 1000,
            "max_tool_calls": 10,
            "metadata": {},
            "model": "gpt-4o",
            "output": [
                {
                    "type": "function_call",
                    "id": "item_123",
                    "call_id": "call_abc123",
                    "name": "get_current_weather",
                    "arguments": "{\n\"location\": \"Boston, MA\"\n}",
                    "status": "completed"
                }
            ],
            "parallel_tool_calls": true,
            "previous_response_id": "none",
            "prompt": {"id": "prompt_123"},
            "prompt_cache_key": "none",
            "prompt_cache_retention": "none",
            "reasoning": {"effort": "low", "summary": "auto"},
            "safety_identifier": "none",
            "service_tier": "default",
            "status": "completed",
            "temperature": 1.0,
            "text": {"format": {"type": "text"}, "verbosity": "high"},
            "tool_choice": "auto",
            "tools": [],
            "top_logprobs": 0,
            "top_p": 1.0,
            "truncation": "auto",
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

        auto resp = openai::Deserialize(res);

        assert(resp.output.size() == 1);
        auto* call = std::get_if<openai::response::InputTypes::Item::FunctionToolCall>(&resp.output[0]);
        assert(call != nullptr);
        assert(call->call_id == "call_abc123");
        assert(call->name == "get_current_weather");
    }

    // 2. Structured Output Request (mapped to semantic model)
    {
        openai::Request req;
        req.model = "gpt-4o-2024-08-06";
        req.background = false;
        req.input = "Analyze this data.";

        openai::TextConfig tc;
        openai::TextConfig::FormatJsonSchema schema_fmt;
        schema_fmt.name = "analysis";
        schema_fmt.strict = true;
        schema_fmt.description = "Analysis schema";
        
        json::Object schema_obj;
        schema_obj["type"] = "object";
        schema_fmt.schema = schema_obj;

        tc.format = schema_fmt;
        tc.verbosity = openai::Verbosity::MEDIUM;
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        assert(json_str.find("\"name\":\"analysis\"") != std::string::npos);
        assert(json_str.find("\"strict\":true") != std::string::npos);
        std::println("[SUCCESS] OpenAI Documentation Examples passed.");
    }

    // 3. Advanced Structured Output (Recursive/Nested Schema)
    {
        std::println("Testing OpenAI Advanced Documentation Examples (Nested Schema)...");
        openai::Request req;
        req.model = "gpt-4o-2024-08-06";
        req.background = false;

        openai::TextConfig tc;
        openai::TextConfig::FormatJsonSchema schema_fmt;
        schema_fmt.name = "math_solution";
        schema_fmt.strict = true;
        schema_fmt.description = "A detailed math solution";
        
        json::Object schema_obj;
        schema_obj["type"] = "object";
        json::Object props;
        props["steps"] = json::Object{{"type", "array"}, {"items", json::Object{{"type", "string"}}}};
        props["final_answer"] = json::Object{{"type", "string"}};
        schema_obj["properties"] = props;
        schema_obj["required"] = json::Array{"steps", "final_answer"};
        schema_obj["additionalProperties"] = false;
        
        schema_fmt.schema = schema_obj;
        tc.format = schema_fmt;
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        assert(json_str.find("\"name\":\"math_solution\"") != std::string::npos);
        assert(json_str.find("\"steps\"") != std::string::npos);
        assert(json_str.find("\"additionalProperties\":false") != std::string::npos);

        std::println("[SUCCESS] OpenAI Advanced Documentation Examples passed.");
    }
}

int main() {
    std::println("Starting OpenAI Tests...");
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_content_serialization();
        test_response_deserialization();
        test_openai_doc_examples();
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
