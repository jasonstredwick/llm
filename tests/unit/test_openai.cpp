#include <vector>
#include <string>
#include "test_assert.hpp"
#include <print>
#include <cstring>
#include <simdjson.h>

#include "../../src/curl.hpp"

#include "../../interface/core/error.hpp"
#include "../../interface/protocols/openai/responses.hpp"
#include "../../interface/protocols/openai/responses_strings.hpp"
#include "../../src/protocols/openai.hpp"

using namespace jai::llm;

// ─── Type aliases for readability ────────────────────────────────────────────

// Request types
using Req = openai::Request;
using FunctionTool = Req::FunctionTool;
using ReqTool = Req::Tool;
using ReqMessage = Req::EasyInputMessage;
using ReqInputText = Req::EasyInputMessage::ResponseInputText;
using ReqInputImage = Req::EasyInputMessage::ResponseInputImage;
using ReqInputFile = Req::EasyInputMessage::ResponseInputFile;
using ReqInputItem = Req::ResponseInputItem;
using ReqReasoning = Req::Reasoning;
using ReqTextConfig = Req::ResponseTextConfig;
using ReqJsonSchema = Req::ResponseTextConfig::ResponseFormatTextJSONSchemaConfig;

// Response types
using Resp = openai::Response;
using RespOutputMsg = Resp::ResponseOutputMessage;
using RespOutputText = Resp::ResponseOutputMessage::ResponseOutputText;
using RespOutputRefusal = Resp::ResponseOutputMessage::ResponseOutputRefusal;
using RespFuncCall = Resp::ResponseFunctionToolCall;
using RespCodeInterp = Resp::ResponseCodeInterpreterToolCall;
using RespMcpCall = Resp::McpCall;
using RespOutputItem = Resp::ResponseOutputItem;

// ─── Serialization tests ─────────────────────────────────────────────────────

void test_simple_serialization() {
    std::println("Testing Simple OpenAI Request Serialization...");

    Req req;
    req.background = false;
    req.model = "gpt-4o";
    req.input = Req::Input{std::string{"Hello, OpenAI!"}};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"model\":\"gpt-4o\"") != std::string::npos);
    REQUIRE(json_str.find("\"input\":\"Hello, OpenAI!\"") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

void test_complex_serialization() {
    std::println("Testing Complex OpenAI Request Serialization...");

    Req req;
    req.background = false;
    req.model = "gpt-4o";
    req.max_output_tokens = 1000;

    // Tools — function tool
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

    FunctionTool func{
        .name = std::string{"get_stock_price"},
        .parameters = params,
        .strict = true,
        .type = {{}},
        .description = std::string{"Get the current stock price for a given symbol"}
    };

    req.tools = std::vector<ReqTool>{func};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"max_output_tokens\":1000") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_stock_price\"") != std::string::npos);
    REQUIRE(json_str.find("\"symbol\":") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_content_serialization() {
    std::println("Testing OpenAI Content Serialization...");

    Req req;
    req.background = false;
    req.model = "gpt-4o";

    ReqInputText text_unit{
        .text = std::string{"Here is an image."},
        .type = {{}}
    };

    ReqInputImage image_unit{
        .detail = ReqInputImage::Detail::HIGH,
        .type = {{}},
        .image_url = std::string{"https://example.com/image.png"}
    };

    using ReqInputContent = ReqMessage::ResponseInputContent;
    ReqMessage msg{
        .content = ReqMessage::Content{
            std::vector<ReqInputContent>{text_unit, image_unit}
        },
        .role = ReqMessage::Role::USER,
        .type = {{}}
    };

    req.input = Req::Input{std::vector<ReqInputItem>{msg}};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"type\":\"input_text\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"input_image\"") != std::string::npos);
    REQUIRE(json_str.find("\"image_url\":\"https://example.com/image.png\"") != std::string::npos);

    std::println("[SUCCESS] Content Serialization passed.");
}

void test_openai_full_request_serialization() {
    std::println("Testing OpenAI Full Request Serialization...");

    Req req;
    req.model = "o1-preview";
    req.background = true;

    // Reasoning
    ReqReasoning r;
    r.effort = ReqReasoning::Effort::HIGH;
    r.summary = ReqReasoning::Summary::AUTO;
    req.reasoning = r;

    // Service Tier
    req.service_tier = Req::ServiceTier::DEFAULT;

    // Stream
    req.stream = true;

    // Prompt Caching
    req.prompt_cache_key = "cache_key_123";

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"effort\":\"high\"") != std::string::npos);
    REQUIRE(json_str.find("\"stream\":true") != std::string::npos);
    REQUIRE(json_str.find("\"prompt_cache_key\":\"cache_key_123\"") != std::string::npos);

    std::println("[SUCCESS] Full Request Serialization passed.");
}

void test_openai_doc_examples() {
    std::println("Testing OpenAI Documentation Examples...");

    // Structured Output Request
    {
        Req req;
        req.model = "gpt-4o-2024-08-06";
        req.background = false;
        req.input = Req::Input{std::string{"Analyze this data."}};

        json::Object schema_obj;
        schema_obj["type"] = "object";

        ReqTextConfig tc;
        tc.format = ReqJsonSchema{
            .name = std::string{"analysis"},
            .schema = schema_obj,
            .type = {{}},
            .description = std::string{"Analysis schema"},
            .strict = true
        };
        tc.verbosity = ReqTextConfig::Verbosity::MEDIUM;
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"name\":\"analysis\"") != std::string::npos);
        REQUIRE(json_str.find("\"strict\":true") != std::string::npos);
    }

    // Advanced Structured Output (Nested Schema)
    {
        Req req;
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

        ReqTextConfig tc;
        tc.format = ReqJsonSchema{
            .name = std::string{"math_solution"},
            .schema = schema_obj,
            .type = {{}},
            .description = std::string{"A detailed math solution"},
            .strict = true
        };
        req.text = tc;

        auto serialized = openai::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"name\":\"math_solution\"") != std::string::npos);
        REQUIRE(json_str.find("\"steps\"") != std::string::npos);
        REQUIRE(json_str.find("\"additionalProperties\":false") != std::string::npos);
    }

    std::println("[SUCCESS] OpenAI Documentation Examples passed.");
}

// ─── Deserialization tests ───────────────────────────────────────────────────

void test_simple_response_deserialization() {
    std::println("Testing OpenAI Response Deserialization...");

    std::string json_response = R"({
        "object": "response",
        "background": false,
        "completed_at": 1677652288,
        "conversation": {"id": "conv_123"},
        "created_at": 1677652288,
        "id": "res_123",
        "incomplete_details": {},
        "instructions": "",
        "max_output_tokens": 1000,
        "max_tool_calls": 10,
        "model": "gpt-4o",
        "output": [
            {
                "type": "message",
                "id": "msg_456",
                "role": "assistant",
                "status": "completed",
                "content": {
                    "type": "output_text",
                    "text": "Hello! I am OpenAI assistant.",
                    "logprobs": []
                }
            }
        ],
        "parallel_tool_calls": true,
        "prompt": {"id": "prompt_123"},
        "reasoning": {"effort": "low", "summary": "auto"},
        "service_tier": "default",
        "status": "completed",
        "temperature": 1.0,
        "text": {"format": {"type": "text"}, "verbosity": "high"},
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

    REQUIRE(resp.id.value() == "res_123");
    REQUIRE(resp.model.value() == "gpt-4o");
    auto const& output_vec = resp.output.value();
    REQUIRE(output_vec.size() == 1);

    auto const* msg_item = std::get_if<RespOutputMsg>(&output_vec[0]);
    REQUIRE(msg_item != nullptr);
    REQUIRE(msg_item->id.value() == "msg_456");

    auto const& content_variant = msg_item->content.value();
    auto const* text_content = std::get_if<RespOutputText>(&content_variant);
    REQUIRE(text_content != nullptr);
    REQUIRE(text_content->text.value() == "Hello! I am OpenAI assistant.");

    REQUIRE(resp.usage.value().total_tokens.value() == 30);

    std::println("[SUCCESS] Response Deserialization passed.");
}

void test_tool_call_response_deserialization() {
    std::println("Testing OpenAI Tool Call Response Deserialization...");

    std::string json_response = R"({
        "id": "resp_123",
        "object": "response",
        "background": false,
        "completed_at": 1677652288,
        "conversation": {"id": "conv_123"},
        "created_at": 1677652288,
        "incomplete_details": {},
        "instructions": "",
        "max_output_tokens": 1000,
        "max_tool_calls": 10,
        "model": "gpt-4o",
        "output": [
            {
                "type": "function_call",
                "id": "item_123",
                "call_id": "call_abc123",
                "name": "get_current_weather",
                "arguments": "{\"location\": \"Boston, MA\"}",
                "status": "completed"
            }
        ],
        "parallel_tool_calls": true,
        "prompt": {"id": "prompt_123"},
        "reasoning": {"effort": "low", "summary": "auto"},
        "service_tier": "default",
        "status": "completed",
        "temperature": 1.0,
        "text": {"format": {"type": "text"}, "verbosity": "high"},
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
    auto const& output = resp.output.value();

    REQUIRE(output.size() == 1);
    auto const* call = std::get_if<RespFuncCall>(&output[0]);
    REQUIRE(call != nullptr);
    REQUIRE(call->id.value() == "item_123");
    REQUIRE(call->call_id.value() == "call_abc123");
    REQUIRE(call->name.value() == "get_current_weather");

    std::println("[SUCCESS] Tool Call Response Deserialization passed.");
}

void test_advanced_response_deserialization() {
    std::println("Testing OpenAI Advanced Response Deserialization...");

    std::string json_response = R"json(
    {
        "id": "resp_advanced_123",
        "object": "response",
        "created_at": 1234567890,
        "model": "gpt-4o",
        "status": "completed",
        "background": false,
        "completed_at": 1234567899,
        "conversation": { "id": "conv_123" },
        "incomplete_details": { "reason": "none" },
        "instructions": "You are a helpful assistant.",
        "max_output_tokens": 4096,
        "max_tool_calls": 5,
        "output": [
            {
                "type": "message",
                "id": "msg_refusal",
                "role": "assistant",
                "status": "completed",
                "content": {
                    "type": "refusal",
                    "refusal": "I cannot answer that question."
                }
            },
            {
                "type": "message",
                "id": "msg_text_with_citation",
                "role": "assistant",
                "status": "completed",
                "content": {
                    "type": "output_text",
                    "text": "Here is the data [1].",
                    "annotations": {
                        "type": "file_citation",
                        "file_id": "file-123",
                        "filename": "data.csv",
                        "index": 0
                    },
                    "logprobs": [
                        {
                            "token": "Here",
                            "logprob": -0.1,
                            "bytes": [72, 101, 114, 101],
                            "top_logprobs": []
                        }
                    ]
                }
            }
        ],
        "parallel_tool_calls": true,
        "previous_response_id": "prev_resp",
        "prompt": { "id": "prompt_1" },
        "prompt_cache_key": "cache_key",
        "prompt_cache_retention": "24h",
        "reasoning": { "effort": "low", "summary": "auto" },
        "safety_identifier": "safe_id",
        "service_tier": "default",
        "temperature": 0.7,
        "text": { "type": "text" },
        "tools": [],
        "top_logprobs": 0,
        "top_p": 1.0,
        "truncation": "auto",
        "usage": {
            "input_tokens": 10,
            "input_tokens_details": { "cached_tokens": 0 },
            "output_tokens": 20,
            "output_tokens_details": { "reasoning_tokens": 0 },
            "total_tokens": 30
        }
    })json";

    curl::Response curl_resp;
    curl_resp.state = curl::Response::State::COMPLETED;
    curl_resp.status_code = 200;
    curl_resp.body_len = json_response.size();
    curl_resp.body.resize(curl_resp.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(curl_resp.body.data(), json_response.data(), curl_resp.body_len);

    auto resp = openai::Deserialize(curl_resp);

    auto& output = resp.output.value();
    bool found_refusal = false;
    bool found_citation = false;

    for (const auto& item : output) {
        auto const* msg = std::get_if<RespOutputMsg>(&item);
        if (!msg) continue;

        auto const& content = msg->content.value();
        if (auto const* refusal = std::get_if<RespOutputRefusal>(&content)) {
            if (refusal->refusal.value() == "I cannot answer that question.") {
                found_refusal = true;
            }
        } else if (auto const* out_text = std::get_if<RespOutputText>(&content)) {
            auto const& ann = out_text->annotations.value();
            using FileCitation = RespOutputText::FileCitation;
            if (auto const* cite = std::get_if<FileCitation>(&ann)) {
                if (cite->file_id.value() == "file-123") {
                    found_citation = true;
                }
            }
        }
    }

    REQUIRE(found_refusal);
    REQUIRE(found_citation);

    std::println("[SUCCESS] Advanced Response Deserialization passed.");
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    std::println("Starting OpenAI Tests...");
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_content_serialization();
        test_openai_full_request_serialization();
        test_openai_doc_examples();
        test_simple_response_deserialization();
        test_tool_call_response_deserialization();
        test_advanced_response_deserialization();
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
