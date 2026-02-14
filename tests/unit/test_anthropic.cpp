#include <vector>
#include <string>
#include "test_assert.hpp"
#include <print>
#include <cstring>
#include <simdjson.h>

#include "../../interface/core/error.hpp"
#include "../../interface/protocols/anthropic/messages.hpp"
#include "../../src/protocols/anthropic.hpp"
#include "../../src/curl.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple Anthropic Request Serialization...");

    anthropic::Request req{
        .max_tokens = 1024,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"Hello, Claude!"}},
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    REQUIRE(json_str.find("\"model\":\"claude-3-5-sonnet-20240620\"") != std::string::npos);
    REQUIRE(json_str.find("\"max_tokens\":1024") != std::string::npos);
    REQUIRE(json_str.find("\"role\":\"user\"") != std::string::npos);
    REQUIRE(json_str.find("\"content\":\"Hello, Claude!\"") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

void test_complex_serialization() {
    std::println("Testing Complex Anthropic Request Serialization...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"What is the weather?"}},
                .role = anthropic::Role::USER
            },
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"I need to check the weather. Which city?"}},
                .role = anthropic::Role::ASSISTANT
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };
    req.temperature = 0.7;
    req.system = std::string{"You are a helpful assistant."};

    anthropic::Metadata meta;
    meta.user_id = "user_1234";
    req.metadata = meta;

    json::Object properties;
    
    json::Object location_prop;
    location_prop["type"] = "string";
    location_prop["description"] = "The city and state, e.g. San Francisco, CA";
    
    properties["location"] = location_prop;

    anthropic::Tool get_weather{
        .input_schema = anthropic::Tool::InputSchema{
            .type = {{}},
            .properties = properties,
            .required = std::vector<std::string>{"location"}
        },
        .name = std::string{"get_weather"},
        .description = std::string{"Get the current weather in a given location"}
    };
    
    req.tools = std::vector<anthropic::ToolUnion>{get_weather};

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    REQUIRE(json_str.find("\"system\":\"You are a helpful assistant.\"") != std::string::npos);
    REQUIRE(json_str.find("\"metadata\":{\"user_id\":\"user_1234\"}") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    REQUIRE(json_str.find("\"location\":") != std::string::npos);
    REQUIRE(json_str.find("\"temperature\":0.7") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_part_serialization() {
    std::println("Testing Anthropic Part Serialization...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{
                    std::vector<anthropic::ContentBlockParam>{
                        anthropic::TextBlockParam{
                            .type = {{}},
                            .text = std::string{"Here is an image and a tool result."}
                        },
                        anthropic::ImageBlockParam{
                            .type = {{}},
                            .source = anthropic::ImageSource{
                                anthropic::Base64ImageSource{
                                    .type = {{}},
                                    .data = std::string{"YmFzZTY0X2RhdGE="},
                                    .media_type = anthropic::ImageMediaType::IMAGE_PNG
                                }
                            }
                        },
                        anthropic::ToolResultBlockParam{
                            .type = {{}},
                            .tool_use_id = std::string{"tool_123"},
                            .content = anthropic::ToolResultBlockParam::Content{std::string{"Tool execution successful."}}
                        }
                    }
                },
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"type\":\"text\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"image\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"tool_result\"") != std::string::npos);
    REQUIRE(json_str.find("\"tool_use_id\":\"tool_123\"") != std::string::npos);
    REQUIRE(json_str.find("\"media_type\":\"image/png\"") != std::string::npos);

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

    REQUIRE(resp.id.Value() == "msg_01Xv6Xk");
    REQUIRE(resp.model.Value() == "claude-3-5-sonnet-20240620");
    auto content = resp.content.Value();
    REQUIRE(content.size() == 2);
    
    // Check text block
    auto* text_block = std::get_if<anthropic::TextBlock>(&content[0]);
    REQUIRE(text_block != nullptr);
    REQUIRE(text_block->text.Value() == "Hello! I can help with that.");

    // Check tool use block
    auto* tool_use = std::get_if<anthropic::ToolUseBlock>(&content[1]);
    REQUIRE(tool_use != nullptr);
    REQUIRE(tool_use->name.Value() == "get_weather");
    REQUIRE(tool_use->id.Value() == "toolu_01A09z9HS");
    REQUIRE(std::get<std::string>(tool_use->input.Value().at("location").data) == "San Francisco, CA");

    REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::TOOL_USE);
    REQUIRE(resp.usage.Value().input_tokens.Value() == 15);
    REQUIRE(resp.usage.Value().output_tokens.Value() == 40);

    std::println("[SUCCESS] Response Deserialization passed.");
}

void test_anthropic_doc_examples() {
    std::println("Testing Anthropic Documentation Examples...");

    // 1. Vision Request (from docs)
    {
        anthropic::Request req{
            .max_tokens = 1024,
            .messages = std::vector<anthropic::MessageParam>{
                anthropic::MessageParam{
                    .content = anthropic::MessageParam::Content{
                        std::vector<anthropic::ContentBlockParam>{
                            anthropic::ImageBlockParam{
                                .type = {{}},
                                .source = anthropic::ImageSource{
                                    anthropic::Base64ImageSource{
                                        .type = {{}},
                                        .data = std::string{"YmFzZTY0X2RhdGE="},
                                        .media_type = anthropic::ImageMediaType::IMAGE_JPEG
                                    }
                                }
                            },
                            anthropic::TextBlockParam{
                                .type = {{}},
                                .text = std::string{"What is in the above image?"}
                            }
                        }
                    },
                    .role = anthropic::Role::USER
                }
            },
            .model = std::string{"claude-3-5-sonnet-20240620"}
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        REQUIRE(json_str.find("\"type\":\"image\"") != std::string::npos);
        REQUIRE(json_str.find("\"media_type\":\"image/jpeg\"") != std::string::npos);
        REQUIRE(json_str.find("\"type\":\"text\"") != std::string::npos);
    }

    // 2. Tool Use Response (from docs)
    {
        std::string json_response = R"({
            "id": "msg_01A09z9HS",
            "type": "message",
            "role": "assistant",
            "content": [
                {
                    "type": "text",
                    "text": "I'll check the weather for you.",
                    "citations": []
                },
                {
                    "type": "tool_use",
                    "id": "toolu_01A09z9HS",
                    "name": "get_weather",
                    "input": {"location": "San Francisco, CA"}
                }
            ],
            "model": "claude-3-5-sonnet-20240620",
            "stop_reason": "tool_use",
            "usage": {
                "input_tokens": 15, 
                "output_tokens": 40,
                "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
                "cache_creation_input_tokens": 0,
                "cache_read_input_tokens": 0,
                "server_tool_use": {"web_search_requests": 0},
                "service_tier": "standard"
            }
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = anthropic::Deserialize(res);

        REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::TOOL_USE);
        auto content = resp.content.Value();
        REQUIRE(content.size() == 2);
        
        auto* tool_use = std::get_if<anthropic::ToolUseBlock>(&content[1]);
        REQUIRE(tool_use != nullptr);
        REQUIRE(tool_use->name.Value() == "get_weather");
        REQUIRE(tool_use->id.Value() == "toolu_01A09z9HS");
        
        auto loc_it = tool_use->input.Value().find("location");
        REQUIRE(loc_it != tool_use->input.Value().end());
        REQUIRE(std::get<std::string>(loc_it->second.data) == "San Francisco, CA");
        std::println("  [SUCCESS] Anthropic Documentation Examples passed.");
    }

    // 3. Parallel Tool Use & Advanced Usage (from docs)
    {
        std::println("Testing Anthropic Advanced Documentation Examples...");
        std::string json_response = R"({
            "id": "msg_01A09z9HS",
            "type": "message",
            "role": "assistant",
            "content": [
                {
                    "type": "text",
                    "text": "I'll fetch the weather and your calendar.",
                    "citations": []
                },
                {
                    "type": "tool_use",
                    "id": "toolu_01",
                    "name": "get_weather",
                    "input": {"location": "San Francisco, CA"}
                },
                {
                    "type": "tool_use",
                    "id": "toolu_02",
                    "name": "get_calendar",
                    "input": {"date": "2024-03-20"}
                }
            ],
            "model": "claude-3-5-sonnet-20240620",
            "stop_reason": "tool_use",
            "usage": {
                "input_tokens": 150, 
                "output_tokens": 500,
                "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
                "cache_creation_input_tokens": 100,
                "cache_read_input_tokens": 50,
                "server_tool_use": {"web_search_requests": 0},
                "service_tier": "priority"
            }
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = anthropic::Deserialize(res);

        auto content = resp.content.Value();
        REQUIRE(content.size() == 3);
        REQUIRE(std::holds_alternative<anthropic::TextBlock>(content[0]));
        REQUIRE(std::holds_alternative<anthropic::ToolUseBlock>(content[1]));
        REQUIRE(std::holds_alternative<anthropic::ToolUseBlock>(content[2]));
        
        auto& tool1 = std::get<anthropic::ToolUseBlock>(content[1]);
        REQUIRE(tool1.name.Value() == "get_weather");
        
        auto& tool2 = std::get<anthropic::ToolUseBlock>(content[2]);
        REQUIRE(tool2.name.Value() == "get_calendar");

        REQUIRE(resp.usage.Value().input_tokens.Value() == 150);
        REQUIRE(resp.usage.Value().cache_read_input_tokens.Value() == 50);
        REQUIRE(resp.usage.Value().service_tier.Value() == anthropic::UsageServiceTier::PRIORITY);

        std::println("  [SUCCESS] Anthropic Advanced Documentation Examples passed.");
    }
}

void test_thinking_block_deserialization() {
    std::println("Testing Anthropic ThinkingBlock/RedactedThinkingBlock Deserialization...");

    std::string json_response = R"({
        "id": "msg_thinking_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "thinking",
                "signature": "sig_abc123",
                "thinking": "Let me reason through this step by step..."
            },
            {
                "type": "redacted_thinking",
                "data": "ZW5jcnlwdGVkX2RhdGE="
            },
            {
                "type": "text",
                "text": "Based on my analysis, the answer is 42.",
                "citations": []
            }
        ],
        "model": "claude-sonnet-4-20250514",
        "stop_reason": "end_turn",
        "usage": {
            "input_tokens": 20,
            "output_tokens": 100,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    auto content = resp.content.Value();
    REQUIRE(content.size() == 3);

    // ThinkingBlock
    REQUIRE(std::holds_alternative<anthropic::ThinkingBlock>(content[0]));
    auto& thinking = std::get<anthropic::ThinkingBlock>(content[0]);
    REQUIRE(thinking.signature.Value() == "sig_abc123");
    REQUIRE(thinking.thinking.Value() == "Let me reason through this step by step...");

    // RedactedThinkingBlock
    REQUIRE(std::holds_alternative<anthropic::RedactedThinkingBlock>(content[1]));
    auto& redacted = std::get<anthropic::RedactedThinkingBlock>(content[1]);
    REQUIRE(redacted.data.Value() == "ZW5jcnlwdGVkX2RhdGE=");

    // TextBlock (still present after thinking)
    REQUIRE(std::holds_alternative<anthropic::TextBlock>(content[2]));
    auto& text = std::get<anthropic::TextBlock>(content[2]);
    REQUIRE(text.text.Value() == "Based on my analysis, the answer is 42.");

    std::println("[SUCCESS] ThinkingBlock/RedactedThinkingBlock Deserialization passed.");
}


void test_end_turn_response() {
    std::println("Testing Anthropic end_turn Response Deserialization...");

    std::string json_response = R"({
        "id": "msg_end_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "text",
                "text": "Hello! How can I help you today?",
                "citations": []
            }
        ],
        "model": "claude-3-5-sonnet-20240620",
        "stop_reason": "end_turn",
        "usage": {
            "input_tokens": 8,
            "output_tokens": 12,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    REQUIRE(resp.id.Value() == "msg_end_001");
    REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::END_TURN);
    REQUIRE(resp.role.Value() == anthropic::ResponseRole::ASSISTANT);

    auto content = resp.content.Value();
    REQUIRE(content.size() == 1);
    auto* text = std::get_if<anthropic::TextBlock>(&content[0]);
    REQUIRE(text != nullptr);
    REQUIRE(text->text.Value() == "Hello! How can I help you today?");

    REQUIRE(resp.usage.Value().input_tokens.Value() == 8);
    REQUIRE(resp.usage.Value().output_tokens.Value() == 12);

    std::println("[SUCCESS] end_turn Response Deserialization passed.");
}

void test_server_tool_use_deserialization() {
    std::println("Testing Anthropic ServerToolUseBlock Deserialization...");

    std::string json_response = R"({
        "id": "msg_server_tool_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "server_tool_use",
                "id": "srvtoolu_01ABC",
                "name": "web_search",
                "input": {"query": "latest Claude model release date"}
            },
            {
                "type": "text",
                "text": "Based on my search results...",
                "citations": []
            }
        ],
        "model": "claude-sonnet-4-20250514",
        "stop_reason": "end_turn",
        "usage": {
            "input_tokens": 30,
            "output_tokens": 80,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 1},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    auto content = resp.content.Value();
    REQUIRE(content.size() == 2);

    // ServerToolUseBlock
    REQUIRE(std::holds_alternative<anthropic::ServerToolUseBlock>(content[0]));
    auto& server_tool = std::get<anthropic::ServerToolUseBlock>(content[0]);
    REQUIRE(server_tool.id.Value() == "srvtoolu_01ABC");
    REQUIRE(server_tool.name.Value() == anthropic::WebSearchName::WEB_SEARCH);
    auto query_it = server_tool.input.Value().find("query");
    REQUIRE(query_it != server_tool.input.Value().end());
    REQUIRE(std::get<std::string>(query_it->second.data) == "latest Claude model release date");

    // TextBlock follows
    REQUIRE(std::holds_alternative<anthropic::TextBlock>(content[1]));

    // Verify server_tool_use usage
    REQUIRE(resp.usage.Value().server_tool_use.Value().web_search_requests.Value() == 1);

    std::println("[SUCCESS] ServerToolUseBlock Deserialization passed.");
}


void test_web_search_result_deserialization() {
    std::println("Testing Anthropic WebSearchToolResultBlock Deserialization...");

    // Test 1: Successful results (array content)
    {
        std::string json_response = R"({
            "id": "msg_websearch_001",
            "type": "message",
            "role": "assistant",
            "content": [
                {
                    "type": "web_search_tool_result",
                    "tool_use_id": "srvtoolu_01ABC",
                    "content": [
                        {
                            "type": "web_search_result_location",
                            "encrypted_content": "enc_content_abc",
                            "page_age": "2 days ago",
                            "title": "Claude 4 Release Notes",
                            "url": "https://example.com/claude4"
                        },
                        {
                            "type": "web_search_result_location",
                            "encrypted_content": "enc_content_def",
                            "page_age": "1 week ago",
                            "title": "AI Model Comparison 2025",
                            "url": "https://example.com/comparison"
                        }
                    ]
                }
            ],
            "model": "claude-sonnet-4-20250514",
            "stop_reason": "end_turn",
            "usage": {
                "input_tokens": 50,
                "output_tokens": 120,
                "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
                "cache_creation_input_tokens": 0,
                "cache_read_input_tokens": 0,
                "server_tool_use": {"web_search_requests": 1},
                "service_tier": "standard"
            }
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = anthropic::Deserialize(res);

        auto content = resp.content.Value();
        REQUIRE(content.size() == 1);

        REQUIRE(std::holds_alternative<anthropic::WebSearchToolResultBlock>(content[0]));
        auto& ws_result = std::get<anthropic::WebSearchToolResultBlock>(content[0]);
        REQUIRE(ws_result.tool_use_id.Value() == "srvtoolu_01ABC");

        // Content should be the vector<WebSearchResultBlock> arm
        auto* results = std::get_if<std::vector<anthropic::WebSearchToolResultBlock::WebSearchResultBlock>>(&ws_result.content.Value());
        REQUIRE(results != nullptr);
        REQUIRE(results->size() == 2);
        REQUIRE((*results)[0].title.Value() == "Claude 4 Release Notes");
        REQUIRE((*results)[0].encrypted_content.Value() == "enc_content_abc");
        REQUIRE((*results)[0].page_age.Value() == "2 days ago");
        REQUIRE((*results)[1].title.Value() == "AI Model Comparison 2025");
    }

    // Test 2: Error result (object content)
    {
        std::string json_response = R"({
            "id": "msg_websearch_err",
            "type": "message",
            "role": "assistant",
            "content": [
                {
                    "type": "web_search_tool_result",
                    "tool_use_id": "srvtoolu_02DEF",
                    "content": {
                        "type": "web_search_tool_result_error",
                        "error_code": "max_uses_exceeded"
                    }
                }
            ],
            "model": "claude-sonnet-4-20250514",
            "stop_reason": "end_turn",
            "usage": {
                "input_tokens": 20,
                "output_tokens": 10,
                "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
                "cache_creation_input_tokens": 0,
                "cache_read_input_tokens": 0,
                "server_tool_use": {"web_search_requests": 0},
                "service_tier": "standard"
            }
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = anthropic::Deserialize(res);

        auto content = resp.content.Value();
        REQUIRE(content.size() == 1);

        auto& ws_result = std::get<anthropic::WebSearchToolResultBlock>(content[0]);
        REQUIRE(ws_result.tool_use_id.Value() == "srvtoolu_02DEF");

        // Content should be the error arm
        auto* error = std::get_if<anthropic::WebSearchToolResultBlock::WebSearchToolResultError>(&ws_result.content.Value());
        REQUIRE(error != nullptr);
        REQUIRE(error->error_code.Value() == anthropic::WebSearchToolResultErrorCode::MAX_USES_EXCEEDED);
    }

    std::println("[SUCCESS] WebSearchToolResultBlock Deserialization passed.");
}


void test_citations_deserialization() {
    std::println("Testing Anthropic TextCitation Deserialization (all 5 types)...");

    std::string json_response = R"({
        "id": "msg_cite_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "text",
                "text": "According to the documents, the answer is 42.",
                "citations": [
                    {
                        "type": "char_location",
                        "cited_text": "the answer is 42",
                        "document_index": 0,
                        "document_title": "Guide to Life",
                        "end_char_index": 150,
                        "start_char_index": 134,
                        "file_id": "file_001"
                    },
                    {
                        "type": "page_location",
                        "cited_text": "on page seven",
                        "document_index": 1,
                        "document_title": "Reference Manual",
                        "end_page_number": 8,
                        "start_page_number": 7,
                        "file_id": "file_002"
                    },
                    {
                        "type": "content_block_location",
                        "cited_text": "block-level citation",
                        "document_index": 2,
                        "document_title": "Structured Doc",
                        "end_block_index": 5,
                        "start_block_index": 3,
                        "file_id": "file_003"
                    },
                    {
                        "type": "web_search_result_location",
                        "cited_text": "from search results",
                        "encrypted_index": "enc_idx_abc",
                        "title": "Web Result Title",
                        "url": "https://example.com/result"
                    },
                    {
                        "type": "search_result_location",
                        "cited_text": "from custom search",
                        "end_block_index": 10,
                        "search_result_index": 3,
                        "source": "knowledge_base",
                        "start_block_index": 8,
                        "title": "KB Article"
                    }
                ]
            }
        ],
        "model": "claude-sonnet-4-20250514",
        "stop_reason": "end_turn",
        "usage": {
            "input_tokens": 100,
            "output_tokens": 50,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    auto content = resp.content.Value();
    REQUIRE(content.size() == 1);

    auto& text_block = std::get<anthropic::TextBlock>(content[0]);
    REQUIRE(text_block.text.Value() == "According to the documents, the answer is 42.");

    auto& citations = text_block.citations.Value();
    REQUIRE(citations.size() == 5);

    // 1. char_location
    REQUIRE(std::holds_alternative<anthropic::CitationCharLocation>(citations[0]));
    auto& char_loc = std::get<anthropic::CitationCharLocation>(citations[0]);
    REQUIRE(char_loc.cited_text.Value() == "the answer is 42");
    REQUIRE(char_loc.document_index.Value() == 0);
    REQUIRE(char_loc.document_title.Value() == "Guide to Life");
    REQUIRE(char_loc.end_char_index.Value() == 150);
    REQUIRE(char_loc.start_char_index.Value() == 134);
    REQUIRE(char_loc.file_id.Value() == "file_001");

    // 2. page_location
    REQUIRE(std::holds_alternative<anthropic::CitationPageLocation>(citations[1]));
    auto& page_loc = std::get<anthropic::CitationPageLocation>(citations[1]);
    REQUIRE(page_loc.cited_text.Value() == "on page seven");
    REQUIRE(page_loc.end_page_number.Value() == 8);
    REQUIRE(page_loc.start_page_number.Value() == 7);
    REQUIRE(page_loc.file_id.Value() == "file_002");

    // 3. content_block_location
    REQUIRE(std::holds_alternative<anthropic::CitationContentBlockLocation>(citations[2]));
    auto& block_loc = std::get<anthropic::CitationContentBlockLocation>(citations[2]);
    REQUIRE(block_loc.cited_text.Value() == "block-level citation");
    REQUIRE(block_loc.end_block_index.Value() == 5);
    REQUIRE(block_loc.start_block_index.Value() == 3);
    REQUIRE(block_loc.file_id.Value() == "file_003");

    // 4. web_search_result_location
    REQUIRE(std::holds_alternative<anthropic::CitationsWebSearchResultLocation>(citations[3]));
    auto& web_loc = std::get<anthropic::CitationsWebSearchResultLocation>(citations[3]);
    REQUIRE(web_loc.cited_text.Value() == "from search results");
    REQUIRE(web_loc.encrypted_index.Value() == "enc_idx_abc");
    REQUIRE(web_loc.title.Value() == "Web Result Title");

    // 5. search_result_location
    REQUIRE(std::holds_alternative<anthropic::CitationsSearchResultLocation>(citations[4]));
    auto& search_loc = std::get<anthropic::CitationsSearchResultLocation>(citations[4]);
    REQUIRE(search_loc.cited_text.Value() == "from custom search");
    REQUIRE(search_loc.search_result_index.Value() == 3);
    REQUIRE(search_loc.source.Value() == "knowledge_base");
    REQUIRE(search_loc.title.Value() == "KB Article");

    std::println("[SUCCESS] TextCitation Deserialization (all 5 types) passed.");
}


void test_max_tokens_stop_reason() {
    std::println("Testing Anthropic max_tokens Stop Reason Deserialization...");

    std::string json_response = R"({
        "id": "msg_max_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "text",
                "text": "This response was truncated because it hit the max tok",
                "citations": []
            }
        ],
        "model": "claude-3-5-sonnet-20240620",
        "stop_reason": "max_tokens",
        "usage": {
            "input_tokens": 10,
            "output_tokens": 4096,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::MAX_TOKENS);
    REQUIRE(!resp.stop_sequence.has_value());
    REQUIRE(resp.usage.Value().output_tokens.Value() == 4096);

    auto content = resp.content.Value();
    REQUIRE(content.size() == 1);
    auto& text = std::get<anthropic::TextBlock>(content[0]);
    REQUIRE(text.text.Value() == "This response was truncated because it hit the max tok");

    std::println("[SUCCESS] max_tokens Stop Reason Deserialization passed.");
}

void test_thinking_config_serialization() {
    std::println("Testing Anthropic ThinkingConfig Serialization...");

    // 1. ThinkingConfigEnabled
    {
        anthropic::Request req{
            .max_tokens = 16384,
            .messages = std::vector<anthropic::MessageParam>{
                anthropic::MessageParam{
                    .content = anthropic::MessageParam::Content{std::string{"Think step by step."}},
                    .role = anthropic::Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = anthropic::ThinkingConfig{
            anthropic::ThinkingConfigEnabled{
                .type = anthropic::ThinkingConfigType::ENABLED,
                .budget_tokens = 10000
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"budget_tokens\":10000") != std::string::npos);
        REQUIRE(json_str.find("\"type\":\"enabled\"") != std::string::npos);
    }

    // 2. ThinkingConfigDisabled
    {
        anthropic::Request req{
            .max_tokens = 4096,
            .messages = std::vector<anthropic::MessageParam>{
                anthropic::MessageParam{
                    .content = anthropic::MessageParam::Content{std::string{"Hello"}},
                    .role = anthropic::Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = anthropic::ThinkingConfig{
            anthropic::ThinkingConfigDisabled{
                .type = anthropic::ThinkingConfigType::DISABLED
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"type\":\"disabled\"") != std::string::npos);
        REQUIRE(json_str.find("\"budget_tokens\"") == std::string::npos);
    }

    // 3. ThinkingConfigAdaptive
    {
        anthropic::Request req{
            .max_tokens = 4096,
            .messages = std::vector<anthropic::MessageParam>{
                anthropic::MessageParam{
                    .content = anthropic::MessageParam::Content{std::string{"Hello"}},
                    .role = anthropic::Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = anthropic::ThinkingConfig{
            anthropic::ThinkingConfigDisabled{
                .type = anthropic::ThinkingConfigType::ADAPTIVE
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"type\":\"disabled\"") != std::string::npos);
        REQUIRE(json_str.find("\"budget_tokens\"") == std::string::npos);
    }

    std::println("[SUCCESS] ThinkingConfig Serialization passed.");
}


void test_tool_choice_serialization() {
    std::println("Testing Anthropic ToolChoice Serialization...");

    auto make_base_req = []() {
        return anthropic::Request{
            .max_tokens = 1024,
            .messages = std::vector<anthropic::MessageParam>{
                anthropic::MessageParam{
                    .content = anthropic::MessageParam::Content{std::string{"Hello"}},
                    .role = anthropic::Role::USER
                }
            },
            .model = std::string{"claude-3-5-sonnet-20240620"}
        };
    };

    // 1. ToolChoiceAuto
    {
        auto req = make_base_req();
        req.tool_choice = anthropic::ToolChoice{anthropic::ToolChoiceAuto{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"auto\"}") != std::string::npos);
    }

    // 2. ToolChoiceAny
    {
        auto req = make_base_req();
        req.tool_choice = anthropic::ToolChoice{anthropic::ToolChoiceAny{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"any\"}") != std::string::npos);
    }

    // 3. ToolChoiceTool
    {
        auto req = make_base_req();
        req.tool_choice = anthropic::ToolChoice{
            anthropic::ToolChoiceTool{
                .type = {{}},
                .name = std::string{"get_weather"}
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"type\":\"tool\"") != std::string::npos);
        REQUIRE(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    }

    // 4. ToolChoiceNone
    {
        auto req = make_base_req();
        req.tool_choice = anthropic::ToolChoice{anthropic::ToolChoiceNone{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"none\"}") != std::string::npos);
    }

    std::println("[SUCCESS] ToolChoice Serialization passed.");
}


void test_web_search_tool_serialization() {
    std::println("Testing Anthropic WebSearchTool20250305 Serialization...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"Search for recent AI news."}},
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    anthropic::WebSearchTool20250305 ws_tool{
        .type = {{}},
        .name = anthropic::WebSearchName::WEB_SEARCH,
        .allowed_domains = std::vector<std::string>{"example.com", "news.example.com"},
        .blocked_domains = std::vector<std::string>{"spam.example.com"},
        .max_uses = 5,
        .user_location = anthropic::WebSearchTool20250305::UserLocation{
            .type = anthropic::UserLocationType::APPROXIMATE,
            .city = std::string{"San Francisco"},
            .country = std::string{"US"},
            .region = std::string{"California"},
            .timezone = std::string{"America/Los_Angeles"}
        }
    };

    req.tools = std::vector<anthropic::ToolUnion>{ws_tool};

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"type\":\"web_search_20250305\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"web_search\"") != std::string::npos);
    REQUIRE(json_str.find("\"example.com\"") != std::string::npos);
    REQUIRE(json_str.find("\"spam.example.com\"") != std::string::npos);
    REQUIRE(json_str.find("\"max_uses\":5") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"approximate\"") != std::string::npos);
    REQUIRE(json_str.find("\"city\":\"San Francisco\"") != std::string::npos);
    REQUIRE(json_str.find("\"country\":\"US\"") != std::string::npos);

    std::println("[SUCCESS] WebSearchTool20250305 Serialization passed.");
}


void test_system_as_text_blocks_serialization() {
    std::println("Testing Anthropic System as vector<TextBlockParam> Serialization...");

    anthropic::Request req{
        .max_tokens = 1024,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"Hello"}},
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    req.system = anthropic::System{
        std::vector<anthropic::TextBlockParam>{
            anthropic::TextBlockParam{
                .type = {{}},
                .text = std::string{"You are a coding assistant."},
                .cache_control = anthropic::CacheControlEphemeral{.type = {{}}}
            },
            anthropic::TextBlockParam{
                .type = {{}},
                .text = std::string{"Always provide complete solutions."}
            }
        }
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"system\":[") != std::string::npos);
    REQUIRE(json_str.find("\"You are a coding assistant.\"") != std::string::npos);
    REQUIRE(json_str.find("\"Always provide complete solutions.\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"ephemeral\"") != std::string::npos);

    std::println("[SUCCESS] System as vector<TextBlockParam> Serialization passed.");
}


void test_computer_use_tools_serialization() {
    std::println("Testing Anthropic Computer Use Tools Serialization...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"Run a command."}},
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    anthropic::ToolBash20250124 bash_tool{
        .type = {{}},
        .name = anthropic::ToolBash20250124Name::BASH
    };

    anthropic::ToolTextEditor20250124 text_editor_124{
        .type = {{}},
        .name = anthropic::ReplaceEditor::STRING
    };

    anthropic::ToolTextEditor20250429 text_editor_429{
        .type = {{}},
        .name = anthropic::ReplaceBasedEditor::STRING
    };

    anthropic::ToolTextEditor20250728 text_editor_728{
        .type = {{}},
        .name = anthropic::ReplaceBasedEditor::STRING,
        .max_characters = 50000
    };

    req.tools = std::vector<anthropic::ToolUnion>{
        bash_tool,
        text_editor_124,
        text_editor_429,
        text_editor_728
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"type\":\"bash_20250124\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"bash\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"text_editor_20250124\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"str_replace_editor\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"text_editor_20250429\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"str_replace_based_edit_tool\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"text_editor_20250728\"") != std::string::npos);

    std::println("[SUCCESS] Computer Use Tools Serialization passed.");
}


void test_tool_result_with_content_blocks() {
    std::println("Testing Anthropic ToolResultBlockParam with content blocks and is_error...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{
                    std::vector<anthropic::ContentBlockParam>{
                        anthropic::ToolResultBlockParam{
                            .type = {{}},
                            .tool_use_id = std::string{"toolu_abc123"},
                            .content = anthropic::ToolResultBlockParam::Content{
                                std::vector<anthropic::ToolResultBlockParam::ContentUnit>{
                                    anthropic::TextBlockParam{
                                        .type = {{}},
                                        .text = std::string{"The result text here."}
                                    },
                                    anthropic::ImageBlockParam{
                                        .type = {{}},
                                        .source = anthropic::ImageSource{
                                            anthropic::Base64ImageSource{
                                                .type = {{}},
                                                .data = std::string{"aW1hZ2VfZGF0YQ=="},
                                                .media_type = anthropic::ImageMediaType::IMAGE_PNG
                                            }
                                        }
                                    }
                                }
                            },
                            .is_error = false
                        },
                        anthropic::ToolResultBlockParam{
                            .type = {{}},
                            .tool_use_id = std::string{"toolu_def456"},
                            .content = anthropic::ToolResultBlockParam::Content{
                                std::string{"Error: command not found"}
                            },
                            .is_error = true
                        }
                    }
                },
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    // First tool result has content array
    REQUIRE(json_str.find("\"tool_use_id\":\"toolu_abc123\"") != std::string::npos);
    REQUIRE(json_str.find("\"The result text here.\"") != std::string::npos);
    REQUIRE(json_str.find("\"is_error\":false") != std::string::npos);

    // Second tool result has string content and is_error=true
    REQUIRE(json_str.find("\"tool_use_id\":\"toolu_def456\"") != std::string::npos);
    REQUIRE(json_str.find("\"Error: command not found\"") != std::string::npos);
    REQUIRE(json_str.find("\"is_error\":true") != std::string::npos);

    std::println("[SUCCESS] ToolResultBlockParam with content blocks passed.");
}


void test_url_image_source_serialization() {
    std::println("Testing Anthropic URLImageSource Serialization...");

    anthropic::Request req{
        .max_tokens = 1024,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{
                    std::vector<anthropic::ContentBlockParam>{
                        anthropic::ImageBlockParam{
                            .type = {{}},
                            .source = anthropic::ImageSource{
                                anthropic::URLImageSource{
                                    .type = {{}},
                                    .url = jai::llm::EncodedUrl{std::string{"https://example.com/image.png"}}
                                }
                            }
                        },
                        anthropic::TextBlockParam{
                            .type = {{}},
                            .text = std::string{"Describe this image."}
                        }
                    }
                },
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"type\":\"url\"") != std::string::npos);
    REQUIRE(json_str.find("\"url\":\"https://example.com/image.png\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"image\"") != std::string::npos);

    std::println("[SUCCESS] URLImageSource Serialization passed.");
}


void test_cache_control_serialization() {
    std::println("Testing Anthropic cache_control Serialization...");

    anthropic::Request req{
        .max_tokens = 4096,
        .messages = std::vector<anthropic::MessageParam>{
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{
                    std::vector<anthropic::ContentBlockParam>{
                        anthropic::TextBlockParam{
                            .type = {{}},
                            .text = std::string{"Important cached text."},
                            .cache_control = anthropic::CacheControlEphemeral{
                                .type = {{}},
                                .ttl = anthropic::CacheControlTTL::TTL_5M
                            }
                        }
                    }
                },
                .role = anthropic::Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"cache_control\":{\"type\":\"ephemeral\"") != std::string::npos);
    REQUIRE(json_str.find("\"ttl\":\"5m\"") != std::string::npos);

    std::println("[SUCCESS] cache_control Serialization passed.");
}


void test_stop_sequence_deserialization() {
    std::println("Testing Anthropic stop_sequence Deserialization...");

    std::string json_response = R"({
        "id": "msg_stop_seq_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "text",
                "text": "Here is the list:\n1. Apple\n2. Banana\n---",
                "citations": []
            }
        ],
        "model": "claude-3-5-sonnet-20240620",
        "stop_reason": "stop_sequence",
        "stop_sequence": "---",
        "usage": {
            "input_tokens": 25,
            "output_tokens": 18,
            "cache_creation": {"ephemeral_1h_input_tokens": 0, "ephemeral_5m_input_tokens": 0},
            "cache_creation_input_tokens": 0,
            "cache_read_input_tokens": 0,
            "server_tool_use": {"web_search_requests": 0},
            "service_tier": "standard"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::STOP_SEQUENCE);
    REQUIRE(resp.stop_sequence.has_value());
    REQUIRE(resp.stop_sequence.value() == "---");

    auto content = resp.content.Value();
    REQUIRE(content.size() == 1);
    auto& text = std::get<anthropic::TextBlock>(content[0]);
    REQUIRE(text.text.Value().find("---") != std::string::npos);

    std::println("[SUCCESS] stop_sequence Deserialization passed.");
}

void test_full_request_serialization() {
    std::println("Testing Anthropic Full Request Serialization (kitchen sink)...");

    // Build a tool input_schema
    json::Object location_prop;
    location_prop["type"] = "string";
    location_prop["description"] = "City name";

    json::Object properties;
    properties["location"] = location_prop;

    anthropic::Tool custom_tool{
        .input_schema = anthropic::Tool::InputSchema{
            .type = {{}},
            .properties = properties,
            .required = std::vector<std::string>{"location"}
        },
        .name = std::string{"get_weather"},
        .description = std::string{"Get weather for a location"},
        .type = anthropic::KindCustomTool{{}}
    };

    // Build output_config with JSON schema
    json::Object schema;
    schema["type"] = "object";
    json::Object schema_props;
    json::Object answer_prop;
    answer_prop["type"] = "string";
    schema_props["answer"] = answer_prop;
    schema["properties"] = schema_props;

    anthropic::Request req{
        .max_tokens = 8192,
        .messages = std::vector<anthropic::MessageParam>{
            // User message with multi-type content blocks
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{
                    std::vector<anthropic::ContentBlockParam>{
                        anthropic::TextBlockParam{
                            .type = {{}},
                            .text = std::string{"Analyze this image and run a tool."},
                            .cache_control = anthropic::CacheControlEphemeral{
                                .type = {{}},
                                .ttl = anthropic::CacheControlTTL::TTL_1H
                            }
                        },
                        anthropic::ImageBlockParam{
                            .type = {{}},
                            .source = anthropic::ImageSource{
                                anthropic::URLImageSource{
                                    .type = {{}},
                                    .url = jai::llm::EncodedUrl{std::string{"https://example.com/photo.jpg"}}
                                }
                            }
                        }
                    }
                },
                .role = anthropic::Role::USER
            },
            // Assistant message with string content
            anthropic::MessageParam{
                .content = anthropic::MessageParam::Content{std::string{"I'll analyze that for you."}},
                .role = anthropic::Role::ASSISTANT
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    // Set ALL optional fields
    req.metadata = anthropic::Metadata{.user_id = std::string{"user_42"}};
    req.output_config = anthropic::OutputConfig{
        .format = anthropic::OutputConfig::Format{
            .type = {{}},
            .schema = schema
        }
    };
    req.service_tier = anthropic::RequestServiceTier::AUTO;
    req.stop_sequences = std::vector<std::string>{"---", "END"};
    req.stream = false;
    req.system = anthropic::System{
        std::vector<anthropic::TextBlockParam>{
            anthropic::TextBlockParam{
                .type = {{}},
                .text = std::string{"You are a helpful assistant."},
                .cache_control = anthropic::CacheControlEphemeral{.type = {{}}}
            }
        }
    };
    req.temperature = 0.8;
    req.thinking = anthropic::ThinkingConfig{
        anthropic::ThinkingConfigEnabled{
            .type = anthropic::ThinkingConfigType::ENABLED,
            .budget_tokens = 5000
        }
    };
    req.tool_choice = anthropic::ToolChoice{
        anthropic::ToolChoiceTool{
            .type = {{}},
            .name = std::string{"get_weather"}
        }
    };
    req.tools = std::vector<anthropic::ToolUnion>{custom_tool};
    req.top_k = 40;
    req.top_p = 0.95;

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    // Verify ALL fields are present
    REQUIRE(json_str.find("\"max_tokens\":8192") != std::string::npos);
    REQUIRE(json_str.find("\"model\":\"claude-sonnet-4-20250514\"") != std::string::npos);
    REQUIRE(json_str.find("\"user_id\":\"user_42\"") != std::string::npos);
    REQUIRE(json_str.find("\"json_schema\"") != std::string::npos);
    REQUIRE(json_str.find("\"service_tier\":\"auto\"") != std::string::npos);
    REQUIRE(json_str.find("\"---\"") != std::string::npos);
    REQUIRE(json_str.find("\"END\"") != std::string::npos);
    REQUIRE(json_str.find("\"stream\":false") != std::string::npos);
    REQUIRE(json_str.find("\"system\":[") != std::string::npos);
    REQUIRE(json_str.find("\"temperature\":0.8") != std::string::npos);
    REQUIRE(json_str.find("\"budget_tokens\":5000") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"enabled\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"tool\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    REQUIRE(json_str.find("\"top_k\":40") != std::string::npos);
    REQUIRE(json_str.find("\"top_p\":0.95") != std::string::npos);
    REQUIRE(json_str.find("\"ttl\":\"1h\"") != std::string::npos);
    REQUIRE(json_str.find("\"type\":\"url\"") != std::string::npos);
    REQUIRE(json_str.find("\"https://example.com/photo.jpg\"") != std::string::npos);

    std::println("[SUCCESS] Full Request Serialization (kitchen sink) passed.");
}


void test_full_response_deserialization() {
    std::println("Testing Anthropic Full Response Deserialization (kitchen sink)...");

    std::string json_response = R"({
        "id": "msg_full_001",
        "type": "message",
        "role": "assistant",
        "content": [
            {
                "type": "thinking",
                "signature": "EqoB...",
                "thinking": "Let me think about this step by step..."
            },
            {
                "type": "text",
                "text": "Here is my analysis based on the document.\n---",
                "citations": [
                    {
                        "type": "char_location",
                        "cited_text": "important finding",
                        "document_index": 0,
                        "document_title": "Research Paper",
                        "end_char_index": 150,
                        "start_char_index": 100,
                        "file_id": "file_abc123"
                    },
                    {
                        "type": "web_search_result_location",
                        "cited_text": "external source text",
                        "encrypted_index": "enc_idx_456",
                        "title": "Web Article",
                        "url": "https://example.com/article"
                    }
                ]
            },
            {
                "type": "tool_use",
                "id": "toolu_full_001",
                "input": {"location": "San Francisco", "units": "celsius"},
                "name": "get_weather"
            }
        ],
        "model": "claude-sonnet-4-20250514",
        "stop_reason": "stop_sequence",
        "stop_sequence": "---",
        "usage": {
            "input_tokens": 1500,
            "output_tokens": 350,
            "cache_creation": {
                "ephemeral_1h_input_tokens": 200,
                "ephemeral_5m_input_tokens": 100
            },
            "cache_creation_input_tokens": 300,
            "cache_read_input_tokens": 500,
            "server_tool_use": {"web_search_requests": 3},
            "service_tier": "priority"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = anthropic::Deserialize(res);

    // Top-level fields
    REQUIRE(resp.id.Value() == "msg_full_001");
    REQUIRE(resp.model.Value() == "claude-sonnet-4-20250514");
    REQUIRE(resp.role.Value() == anthropic::ResponseRole::ASSISTANT);
    REQUIRE(resp.stop_reason.Value() == anthropic::StopReason::STOP_SEQUENCE);
    REQUIRE(resp.stop_sequence.has_value());
    REQUIRE(resp.stop_sequence.value() == "---");

    // Content: 3 blocks (thinking, text, tool_use)
    auto content = resp.content.Value();
    REQUIRE(content.size() == 3);

    // Block 0: ThinkingBlock
    auto* thinking = std::get_if<anthropic::ThinkingBlock>(&content[0]);
    REQUIRE(thinking != nullptr);
    REQUIRE(thinking->signature.Value() == "EqoB...");
    REQUIRE(thinking->thinking.Value().find("step by step") != std::string::npos);

    // Block 1: TextBlock with 2 citations
    auto* text = std::get_if<anthropic::TextBlock>(&content[1]);
    REQUIRE(text != nullptr);
    REQUIRE(text->text.Value().find("analysis") != std::string::npos);
    REQUIRE(text->citations.Value().size() == 2);

    auto* char_cite = std::get_if<anthropic::CitationCharLocation>(&text->citations.Value()[0]);
    REQUIRE(char_cite != nullptr);
    REQUIRE(char_cite->cited_text.Value() == "important finding");
    REQUIRE(char_cite->document_title.Value() == "Research Paper");
    REQUIRE(char_cite->start_char_index.Value() == 100);
    REQUIRE(char_cite->end_char_index.Value() == 150);
    REQUIRE(char_cite->file_id.Value() == "file_abc123");

    auto* web_cite = std::get_if<anthropic::CitationsWebSearchResultLocation>(&text->citations.Value()[1]);
    REQUIRE(web_cite != nullptr);
    REQUIRE(web_cite->title.Value() == "Web Article");

    // Block 2: ToolUseBlock
    auto* tool_use = std::get_if<anthropic::ToolUseBlock>(&content[2]);
    REQUIRE(tool_use != nullptr);
    REQUIRE(tool_use->id.Value() == "toolu_full_001");
    REQUIRE(tool_use->name.Value() == "get_weather");

    // Usage — all sub-fields
    auto& usage = resp.usage.Value();
    REQUIRE(usage.input_tokens.Value() == 1500);
    REQUIRE(usage.output_tokens.Value() == 350);
    REQUIRE(usage.cache_creation_input_tokens.Value() == 300);
    REQUIRE(usage.cache_read_input_tokens.Value() == 500);
    REQUIRE(usage.cache_creation.Value().ephemeral_1h_input_tokens.Value() == 200);
    REQUIRE(usage.cache_creation.Value().ephemeral_5m_input_tokens.Value() == 100);
    REQUIRE(usage.server_tool_use.Value().web_search_requests.Value() == 3);
    REQUIRE(usage.service_tier.Value() == anthropic::UsageServiceTier::PRIORITY);

    std::println("[SUCCESS] Full Response Deserialization (kitchen sink) passed.");
}


int main() {
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_part_serialization();
        test_response_deserialization();
        test_anthropic_doc_examples();
        test_thinking_block_deserialization();
        test_end_turn_response();
        test_server_tool_use_deserialization();
        test_web_search_result_deserialization();
        test_citations_deserialization();
        test_max_tokens_stop_reason();
        test_thinking_config_serialization();
        test_tool_choice_serialization();
        test_web_search_tool_serialization();
        test_system_as_text_blocks_serialization();
        test_computer_use_tools_serialization();
        test_tool_result_with_content_blocks();
        test_url_image_source_serialization();
        test_cache_control_serialization();
        test_stop_sequence_deserialization();
        test_full_request_serialization();
        test_full_response_deserialization();
        std::println("ALL ANTHROPIC UNIT TESTS PASSED");
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
