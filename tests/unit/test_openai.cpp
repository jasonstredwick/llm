#include <vector>
#include <string>
#include "test_assert.hpp"
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
    REQUIRE(json_str.find("\"model\":\"gpt-4o\"") != std::string::npos);
    REQUIRE(json_str.find("\"input\":\"Hello, OpenAI!\"") != std::string::npos);

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

    openai::request::ToolTypes::Function func{
        .type = {{}},
        .name = std::string{"get_stock_price"},
        .parameters = params,
        .strict = true,
        .description = std::string{"Get the current stock price for a given symbol"}
    };
    
    openai::request::Tool get_stock_price = func;
    
    req.tools = std::vector<openai::request::Tool>{get_stock_price};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    REQUIRE(json_str.find("\"max_output_tokens\":1000") != std::string::npos);
    REQUIRE(json_str.find("\"environment\":\"production\"") != std::string::npos);
    REQUIRE(json_str.find("\"tool_choice\":\"auto\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_stock_price\"") != std::string::npos);
    REQUIRE(json_str.find("\"symbol\":") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_content_serialization() {
    std::println("Testing OpenAI Content Serialization...");

    openai::Request req;

    req.background = false;
    req.model = "gpt-4o";
    
    openai::request::ContentTypes::Text text_unit{
        .type = {{}},
        .text = std::string{"Here is an image."}
    };

    openai::request::ContentTypes::Image image_unit{
        .type = {{}},
        .detail = openai::Detail::HIGH,
        .image_url = EncodedUrl(std::string_view("https://example.com/image.png"))
    };

    openai::request::InputTypes::Message msg{
        .type = {{}},
        .content = openai::request::InputTypes::Message::Content{
            std::vector<openai::request::InputTypes::MessageContentUnit>{text_unit, image_unit}
        },
        .role = openai::RoleInputMessage::USER
    };
    
    req.input = std::vector<openai::request::InputItemList>{msg};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"type\":\"input_text\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"input_image\"") != std::string::npos);
    REQUIRE(json_str.find("\"image_url\":\"https://example.com/image.png\"") != std::string::npos);

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

    REQUIRE(resp.id.Value() == "res_123");
    REQUIRE(resp.model.Value() == "gpt-4o");
    auto const& output_vec = resp.output.Value();
    REQUIRE(output_vec.size() == 1);
    
    auto const* item_ptr = &output_vec[0];
    auto const* msg_item = std::get_if<openai::response::InputTypes::Item::OutputMessage>(item_ptr);
    if (!msg_item) {
        std::println("[ERROR] Variant is NOT OutputMessage! It is index: {}", item_ptr->index());
        return;
    }
    REQUIRE(msg_item->id.Value() == "msg_456");
    REQUIRE(msg_item->role.Value() == openai::RoleAssistant::ASSISTANT);
    REQUIRE(msg_item->status.Value() == openai::ItemStatus::COMPLETED);
    
    auto const& content_vec = msg_item->content.Value();
    auto const* text_content = std::get_if<openai::response::ContentTypes::OutputText>(&content_vec[0]);
    if (!text_content) {
        std::println("[ERROR] Content is NOT OutputText!");
        return;
    }
    REQUIRE(text_content->text.Value() == "Hello! I am OpenAI assistant.");

    REQUIRE(resp.usage.Value().total_tokens.Value() == 30);

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
        auto const& output = resp.output.Value();

        REQUIRE(output.size() == 1);
        auto const* call = std::get_if<openai::response::InputTypes::Item::FunctionToolCall>(&output[0]);
        REQUIRE(call != nullptr);
        REQUIRE(call->id.Value() == "item_123");
        REQUIRE(call->call_id.Value() == "call_abc123");
        REQUIRE(call->name.Value() == "get_current_weather");
        REQUIRE(call->status.Value() == openai::ItemStatus::COMPLETED);
    }

    // 2. Structured Output Request (mapped to semantic model)
    {
        openai::Request req;
        req.model = "gpt-4o-2024-08-06";
        req.background = false;
        req.input = "Analyze this data.";

        json::Object schema_obj;
        schema_obj["type"] = "object";

        openai::TextConfig tc;
        tc.format = openai::TextConfig::FormatJsonSchema{
            .type = {{}},
            .name = std::string{"analysis"},
            .schema = schema_obj,
            .description = std::string{"Analysis schema"},
            .strict = true
        };
        tc.verbosity = openai::Verbosity::MEDIUM;
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        REQUIRE(json_str.find("\"name\":\"analysis\"") != std::string::npos);
        REQUIRE(json_str.find("\"strict\":true") != std::string::npos);
        std::println("[SUCCESS] OpenAI Documentation Examples passed.");
    }

    // 3. Advanced Structured Output (Recursive/Nested Schema)
    {
        std::println("Testing OpenAI Advanced Documentation Examples (Nested Schema)...");
        openai::Request req;
        req.model = "gpt-4o-2024-08-06";
        req.background = false;

        json::Object schema_obj;
        schema_obj["type"] = "object";
        json::Object props;
        props["steps"] = json::Object{{"type", "array"}, {"items", json::Object{{"type", "string"}}}};
        props["final_answer"] = json::Object{{"type", "string"}};
        schema_obj["properties"] = props;
        schema_obj["required"] = json::Array{"steps", "final_answer"};
        schema_obj["additionalProperties"] = false;

        openai::TextConfig tc;
        tc.format = openai::TextConfig::FormatJsonSchema{
            .type = {{}},
            .name = std::string{"math_solution"},
            .schema = schema_obj,
            .description = std::string{"A detailed math solution"},
            .strict = true
        };
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        REQUIRE(json_str.find("\"name\":\"math_solution\"") != std::string::npos);
        REQUIRE(json_str.find("\"steps\"") != std::string::npos);
        REQUIRE(json_str.find("\"additionalProperties\":false") != std::string::npos);

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
