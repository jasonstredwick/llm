#include <vector>
#include <string>
#include "test_assert.hpp"
#include <print>
#include <cstring>
#include <simdjson.h>

#include "../../interface/core/error.hpp"
#include "../../src/protocols/anthropic.hpp"
#include "../../src/curl.hpp"

using namespace jai::llm;

// ── Request-side path aliases ──────────────────────────────
using Request            = anthropic::Request;
using MessageParam       = Request::MessageParam;
using TextBlockParam     = MessageParam::TextBlockParam;
using ImageBlockParam    = MessageParam::ImageBlockParam;
using ToolResultBlockParam = MessageParam::ToolResultBlockParam;
using ContentBlockParam  = MessageParam::ContentBlockParam;
using Base64ImageSource  = ImageBlockParam::Base64ImageSource;
using URLImageSource     = ImageBlockParam::URLImageSource;
using ImageSource        = ImageBlockParam::Source;
using ImageMediaType     = Base64ImageSource::MediaType;
using CacheControlEphemeral = TextBlockParam::CacheControlEphemeral;
using CacheControlTTL    = CacheControlEphemeral::Ttl;
using Role               = MessageParam::Role;
using Metadata           = Request::Metadata;
using OutputConfig       = Request::OutputConfig;
using Tool               = Request::Tool;
using ToolUnion          = Request::ToolUnion;
using ToolBash20250124   = Request::ToolBash20250124;
using ToolTextEditor20250124 = Request::ToolTextEditor20250124;
using ToolTextEditor20250429 = Request::ToolTextEditor20250429;
using ToolTextEditor20250728 = Request::ToolTextEditor20250728;
using WebSearchTool20250305  = Request::WebSearchTool20250305;
using ToolChoiceAuto     = Request::ToolChoiceAuto;
using ToolChoiceAny      = Request::ToolChoiceAny;
using ToolChoiceTool     = Request::ToolChoiceTool;
using ToolChoiceNone     = Request::ToolChoiceNone;
using ToolChoice         = Request::ToolChoice;
using ThinkingConfigEnabled  = Request::ThinkingConfigEnabled;
using ThinkingConfigDisabled = Request::ThinkingConfigDisabled;
using ThinkingConfigAdaptive = Request::ThinkingConfigAdaptive;
using ThinkingConfig     = Request::ThinkingConfigParam;
using System             = Request::System;
using SystemTextBlockParam = Request::TextBlockParam;
using RequestServiceTier = Request::ServiceTier;

// ── Response-side path aliases ─────────────────────────────
using RespMsg            = anthropic::Message;
using TextBlock          = RespMsg::TextBlock;
using ToolUseBlock       = RespMsg::ToolUseBlock;
using ThinkingBlock      = RespMsg::ThinkingBlock;
using RedactedThinkingBlock = RespMsg::RedactedThinkingBlock;
using ServerToolUseBlock = RespMsg::ServerToolUseBlock;
using WebSearchToolResultBlock = RespMsg::WebSearchToolResultBlock;
using StopReason         = RespMsg::StopReason;
using Usage              = RespMsg::Usage;
using UsageServiceTier   = Usage::ServiceTier;
using CitationCharLocation           = TextBlock::CitationCharLocation;
using CitationPageLocation           = TextBlock::CitationPageLocation;
using CitationContentBlockLocation   = TextBlock::CitationContentBlockLocation;
using CitationsWebSearchResultLocation = TextBlock::CitationsWebSearchResultLocation;
using CitationsSearchResultLocation  = TextBlock::CitationsSearchResultLocation;

void test_simple_serialization() {
    std::println("Testing Simple Anthropic Request Serialization...");

    Request req{
        .max_tokens = 1024,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{std::string{"Hello, Claude!"}},
                .role = Role::USER
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

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{std::string{"What is the weather?"}},
                .role = Role::USER
            },
            MessageParam{
                .content = MessageParam::Content{std::string{"I need to check the weather. Which city?"}},
                .role = Role::ASSISTANT
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };
    req.temperature = 0.7;
    req.system = std::string{"You are a helpful assistant."};

    Metadata meta;
    meta.user_id = "user_1234";
    req.metadata = meta;

    json::Object properties;

    json::Object location_prop;
    location_prop["type"] = "string";
    location_prop["description"] = "The city and state, e.g. San Francisco, CA";

    properties["location"] = location_prop;

    Tool get_weather{
        .input_schema = json::Object{},
        .name = std::string{"get_weather"},
        .description = std::string{"Get the current weather in a given location"}
    };
    get_weather.input_schema.value()["type"] = "object";
    get_weather.input_schema.value()["properties"] = properties;
    get_weather.input_schema.value()["required"] = json::Array{json::Value{std::string{"location"}}};

    req.tools = std::vector<ToolUnion>{get_weather};

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

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{
                    std::vector<ContentBlockParam>{
                        TextBlockParam{
                            .text = std::string{"Here is an image and a tool result."},
                            .type = {{}}
                        },
                        ImageBlockParam{
                            .source = ImageSource{
                                Base64ImageSource{
                                    .data = std::string{"YmFzZTY0X2RhdGE="},
                                    .media_type = ImageMediaType::IMAGE_PNG,
                                    .type = {{}}
                                }
                            },
                            .type = {{}}
                        },
                        ToolResultBlockParam{
                            .tool_use_id = std::string{"tool_123"},
                            .type = {{}},
                            .content = ToolResultBlockParam::Content{std::string{"Tool execution successful."}}
                        }
                    }
                },
                .role = Role::USER
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

    REQUIRE(resp.id.value() == "msg_01Xv6Xk");
    REQUIRE(resp.model.value() == "claude-3-5-sonnet-20240620");
    auto content = resp.content.value();
    REQUIRE(content.size() == 2);

    // Check text block
    auto* text_block = std::get_if<TextBlock>(&content[0]);
    REQUIRE(text_block != nullptr);
    REQUIRE(text_block->text.value() == "Hello! I can help with that.");

    // Check tool use block
    auto* tool_use = std::get_if<ToolUseBlock>(&content[1]);
    REQUIRE(tool_use != nullptr);
    REQUIRE(tool_use->name.value() == "get_weather");
    REQUIRE(tool_use->id.value() == "toolu_01A09z9HS");
    REQUIRE(std::get<std::string>(tool_use->input.value().at("location").data) == "San Francisco, CA");

    REQUIRE(resp.stop_reason.value() == StopReason::TOOL_USE);
    REQUIRE(resp.usage.value().input_tokens.value() == 15);
    REQUIRE(resp.usage.value().output_tokens.value() == 40);

    std::println("[SUCCESS] Response Deserialization passed.");
}

void test_anthropic_doc_examples() {
    std::println("Testing Anthropic Documentation Examples...");

    // 1. Vision Request (from docs)
    {
        Request req{
            .max_tokens = 1024,
            .messages = std::vector<MessageParam>{
                MessageParam{
                    .content = MessageParam::Content{
                        std::vector<ContentBlockParam>{
                            ImageBlockParam{
                                .source = ImageSource{
                                    Base64ImageSource{
                                        .data = std::string{"YmFzZTY0X2RhdGE="},
                                        .media_type = ImageMediaType::IMAGE_JPEG,
                                        .type = {{}}
                                    }
                                },
                                .type = {{}}
                            },
                            TextBlockParam{
                                .text = std::string{"What is in the above image?"},
                                .type = {{}}
                            }
                        }
                    },
                    .role = Role::USER
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

        REQUIRE(resp.stop_reason.value() == StopReason::TOOL_USE);
        auto content = resp.content.value();
        REQUIRE(content.size() == 2);

        auto* tool_use = std::get_if<ToolUseBlock>(&content[1]);
        REQUIRE(tool_use != nullptr);
        REQUIRE(tool_use->name.value() == "get_weather");
        REQUIRE(tool_use->id.value() == "toolu_01A09z9HS");

        auto loc_it = tool_use->input.value().find("location");
        REQUIRE(loc_it != tool_use->input.value().end());
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

        auto content = resp.content.value();
        REQUIRE(content.size() == 3);
        REQUIRE(std::holds_alternative<TextBlock>(content[0]));
        REQUIRE(std::holds_alternative<ToolUseBlock>(content[1]));
        REQUIRE(std::holds_alternative<ToolUseBlock>(content[2]));

        auto& tool1 = std::get<ToolUseBlock>(content[1]);
        REQUIRE(tool1.name.value() == "get_weather");

        auto& tool2 = std::get<ToolUseBlock>(content[2]);
        REQUIRE(tool2.name.value() == "get_calendar");

        REQUIRE(resp.usage.value().input_tokens.value() == 150);
        REQUIRE(resp.usage.value().cache_read_input_tokens.value() == 50);
        REQUIRE(resp.usage.value().service_tier.value() == UsageServiceTier::PRIORITY);

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

    auto content = resp.content.value();
    REQUIRE(content.size() == 3);

    // ThinkingBlock
    REQUIRE(std::holds_alternative<ThinkingBlock>(content[0]));
    auto& thinking = std::get<ThinkingBlock>(content[0]);
    REQUIRE(thinking.signature.value() == "sig_abc123");
    REQUIRE(thinking.thinking.value() == "Let me reason through this step by step...");

    // RedactedThinkingBlock
    REQUIRE(std::holds_alternative<RedactedThinkingBlock>(content[1]));
    auto& redacted = std::get<RedactedThinkingBlock>(content[1]);
    REQUIRE(redacted.data.value() == "ZW5jcnlwdGVkX2RhdGE=");

    // TextBlock (still present after thinking)
    REQUIRE(std::holds_alternative<TextBlock>(content[2]));
    auto& text = std::get<TextBlock>(content[2]);
    REQUIRE(text.text.value() == "Based on my analysis, the answer is 42.");

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

    REQUIRE(resp.id.value() == "msg_end_001");
    REQUIRE(resp.stop_reason.value() == StopReason::END_TURN);
    // Response role is a Kind struct with value "assistant"
    static_assert(RespMsg::RoleKind::value == "assistant");

    auto content = resp.content.value();
    REQUIRE(content.size() == 1);
    auto* text = std::get_if<TextBlock>(&content[0]);
    REQUIRE(text != nullptr);
    REQUIRE(text->text.value() == "Hello! How can I help you today?");

    REQUIRE(resp.usage.value().input_tokens.value() == 8);
    REQUIRE(resp.usage.value().output_tokens.value() == 12);

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

    auto content = resp.content.value();
    REQUIRE(content.size() == 2);

    // ServerToolUseBlock
    REQUIRE(std::holds_alternative<ServerToolUseBlock>(content[0]));
    auto& server_tool = std::get<ServerToolUseBlock>(content[0]);
    REQUIRE(server_tool.id.value() == "srvtoolu_01ABC");
    REQUIRE(server_tool.name.value() == ServerToolUseBlock::Name::WEB_SEARCH);
    auto query_it = server_tool.input.value().find("query");
    REQUIRE(query_it != server_tool.input.value().end());
    REQUIRE(std::get<std::string>(query_it->second.data) == "latest Claude model release date");

    // TextBlock follows
    REQUIRE(std::holds_alternative<TextBlock>(content[1]));

    // Verify server_tool_use usage
    REQUIRE(resp.usage.value().server_tool_use.value().web_search_requests.value() == 1);

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
                            "type": "web_search_result",
                            "encrypted_content": "enc_content_abc",
                            "page_age": "2 days ago",
                            "title": "Claude 4 Release Notes",
                            "url": "https://example.com/claude4"
                        },
                        {
                            "type": "web_search_result",
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

        auto content = resp.content.value();
        REQUIRE(content.size() == 1);

        REQUIRE(std::holds_alternative<WebSearchToolResultBlock>(content[0]));
        auto& ws_result = std::get<WebSearchToolResultBlock>(content[0]);
        REQUIRE(ws_result.tool_use_id.value() == "srvtoolu_01ABC");

        // Content should be the vector<WebSearchResultBlock> arm
        auto* results = std::get_if<std::vector<WebSearchToolResultBlock::WebSearchResultBlock>>(&ws_result.content.value());
        REQUIRE(results != nullptr);
        REQUIRE(results->size() == 2);
        REQUIRE((*results)[0].title.value() == "Claude 4 Release Notes");
        REQUIRE((*results)[0].encrypted_content.value() == "enc_content_abc");
        REQUIRE((*results)[0].page_age.value() == "2 days ago");
        REQUIRE((*results)[1].title.value() == "AI Model Comparison 2025");
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

        auto content = resp.content.value();
        REQUIRE(content.size() == 1);

        auto& ws_result = std::get<WebSearchToolResultBlock>(content[0]);
        REQUIRE(ws_result.tool_use_id.value() == "srvtoolu_02DEF");

        // Content should be the error arm
        auto* error = std::get_if<WebSearchToolResultBlock::WebSearchToolResultError>(&ws_result.content.value());
        REQUIRE(error != nullptr);
        REQUIRE(error->error_code.value() == WebSearchToolResultBlock::WebSearchToolResultError::ErrorCode::MAX_USES_EXCEEDED);
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

    auto content = resp.content.value();
    REQUIRE(content.size() == 1);

    auto& text_block = std::get<TextBlock>(content[0]);
    REQUIRE(text_block.text.value() == "According to the documents, the answer is 42.");

    auto& citations = text_block.citations.value();
    REQUIRE(citations.size() == 5);

    // 1. char_location
    REQUIRE(std::holds_alternative<CitationCharLocation>(citations[0]));
    auto& char_loc = std::get<CitationCharLocation>(citations[0]);
    REQUIRE(char_loc.cited_text.value() == "the answer is 42");
    REQUIRE(char_loc.document_index.value() == 0);
    REQUIRE(char_loc.document_title.value() == "Guide to Life");
    REQUIRE(char_loc.end_char_index.value() == 150);
    REQUIRE(char_loc.start_char_index.value() == 134);
    REQUIRE(char_loc.file_id.value() == "file_001");

    // 2. page_location
    REQUIRE(std::holds_alternative<CitationPageLocation>(citations[1]));
    auto& page_loc = std::get<CitationPageLocation>(citations[1]);
    REQUIRE(page_loc.cited_text.value() == "on page seven");
    REQUIRE(page_loc.end_page_number.value() == 8);
    REQUIRE(page_loc.start_page_number.value() == 7);
    REQUIRE(page_loc.file_id.value() == "file_002");

    // 3. content_block_location
    REQUIRE(std::holds_alternative<CitationContentBlockLocation>(citations[2]));
    auto& block_loc = std::get<CitationContentBlockLocation>(citations[2]);
    REQUIRE(block_loc.cited_text.value() == "block-level citation");
    REQUIRE(block_loc.end_block_index.value() == 5);
    REQUIRE(block_loc.start_block_index.value() == 3);
    REQUIRE(block_loc.file_id.value() == "file_003");

    // 4. web_search_result_location
    REQUIRE(std::holds_alternative<CitationsWebSearchResultLocation>(citations[3]));
    auto& web_loc = std::get<CitationsWebSearchResultLocation>(citations[3]);
    REQUIRE(web_loc.cited_text.value() == "from search results");
    REQUIRE(web_loc.encrypted_index.value() == "enc_idx_abc");
    REQUIRE(web_loc.title.value() == "Web Result Title");

    // 5. search_result_location
    REQUIRE(std::holds_alternative<CitationsSearchResultLocation>(citations[4]));
    auto& search_loc = std::get<CitationsSearchResultLocation>(citations[4]);
    REQUIRE(search_loc.cited_text.value() == "from custom search");
    REQUIRE(search_loc.search_result_index.value() == 3);
    REQUIRE(search_loc.source.value() == "knowledge_base");
    REQUIRE(search_loc.title.value() == "KB Article");

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

    REQUIRE(resp.stop_reason.value() == StopReason::MAX_TOKENS);
    // stop_sequence is std::optional<std::string>; absent when stop_reason is max_tokens
    REQUIRE(!resp.stop_sequence.has_value());
    REQUIRE(resp.usage.value().output_tokens.value() == 4096);

    auto content = resp.content.value();
    REQUIRE(content.size() == 1);
    auto& text = std::get<TextBlock>(content[0]);
    REQUIRE(text.text.value() == "This response was truncated because it hit the max tok");

    std::println("[SUCCESS] max_tokens Stop Reason Deserialization passed.");
}

void test_thinking_config_serialization() {
    std::println("Testing Anthropic ThinkingConfig Serialization...");

    // 1. ThinkingConfigEnabled
    {
        Request req{
            .max_tokens = 16384,
            .messages = std::vector<MessageParam>{
                MessageParam{
                    .content = MessageParam::Content{std::string{"Think step by step."}},
                    .role = Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = ThinkingConfig{
            ThinkingConfigEnabled{
                .budget_tokens = 10000,
                .type = {{}}
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"budget_tokens\":10000") != std::string::npos);
        REQUIRE(json_str.find("\"type\":\"enabled\"") != std::string::npos);
    }

    // 2. ThinkingConfigDisabled
    {
        Request req{
            .max_tokens = 4096,
            .messages = std::vector<MessageParam>{
                MessageParam{
                    .content = MessageParam::Content{std::string{"Hello"}},
                    .role = Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = ThinkingConfig{
            ThinkingConfigDisabled{
                .type = {{}}
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"type\":\"disabled\"") != std::string::npos);
        REQUIRE(json_str.find("\"budget_tokens\"") == std::string::npos);
    }

    // 3. ThinkingConfigAdaptive
    {
        Request req{
            .max_tokens = 4096,
            .messages = std::vector<MessageParam>{
                MessageParam{
                    .content = MessageParam::Content{std::string{"Hello"}},
                    .role = Role::USER
                }
            },
            .model = std::string{"claude-sonnet-4-20250514"}
        };
        req.thinking = ThinkingConfig{
            ThinkingConfigAdaptive{
                .type = {{}}
            }
        };

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"type\":\"adaptive\"") != std::string::npos);
        REQUIRE(json_str.find("\"budget_tokens\"") == std::string::npos);
    }

    std::println("[SUCCESS] ThinkingConfig Serialization passed.");
}


void test_tool_choice_serialization() {
    std::println("Testing Anthropic ToolChoice Serialization...");

    auto make_base_req = []() {
        return Request{
            .max_tokens = 1024,
            .messages = std::vector<MessageParam>{
                MessageParam{
                    .content = MessageParam::Content{std::string{"Hello"}},
                    .role = Role::USER
                }
            },
            .model = std::string{"claude-3-5-sonnet-20240620"}
        };
    };

    // 1. ToolChoiceAuto
    {
        auto req = make_base_req();
        req.tool_choice = ToolChoice{ToolChoiceAuto{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"auto\"}") != std::string::npos);
    }

    // 2. ToolChoiceAny
    {
        auto req = make_base_req();
        req.tool_choice = ToolChoice{ToolChoiceAny{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"any\"}") != std::string::npos);
    }

    // 3. ToolChoiceTool
    {
        auto req = make_base_req();
        req.tool_choice = ToolChoice{
            ToolChoiceTool{
                .name = std::string{"get_weather"},
                .type = {{}}
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
        req.tool_choice = ToolChoice{ToolChoiceNone{.type = {{}}}};

        auto serialized = anthropic::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

        REQUIRE(json_str.find("\"tool_choice\":{\"type\":\"none\"}") != std::string::npos);
    }

    std::println("[SUCCESS] ToolChoice Serialization passed.");
}


void test_web_search_tool_serialization() {
    std::println("Testing Anthropic WebSearchTool20250305 Serialization...");

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{std::string{"Search for recent AI news."}},
                .role = Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    // user_location is json::Object in generated code
    WebSearchTool20250305::UserLocation user_loc{
        .type = {{}},
        .city = "San Francisco",
        .country = "US",
        .region = "California",
        .timezone = "America/Los_Angeles"
    };

    WebSearchTool20250305 ws_tool{
        .name = {{}},
        .type = {{}},
        .allowed_domains = std::vector<std::string>{"example.com", "news.example.com"},
        .blocked_domains = std::vector<std::string>{"spam.example.com"},
        .max_uses = 5,
        .user_location = user_loc
    };

    req.tools = std::vector<ToolUnion>{ws_tool};

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

    Request req{
        .max_tokens = 1024,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{std::string{"Hello"}},
                .role = Role::USER
            }
        },
        .model = std::string{"claude-3-5-sonnet-20240620"}
    };

    req.system = System{
        std::vector<SystemTextBlockParam>{
            SystemTextBlockParam{
                .text = std::string{"You are a coding assistant."},
                .type = {{}},
                .cache_control = Request::TextBlockParam::CacheControlEphemeral{.type = {{}}}
            },
            SystemTextBlockParam{
                .text = std::string{"Always provide complete solutions."},
                .type = {{}}
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

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{std::string{"Run a command."}},
                .role = Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    ToolBash20250124 bash_tool{
        .name = {{}},
        .type = {{}}
    };

    ToolTextEditor20250124 text_editor_124{
        .name = {{}},
        .type = {{}}
    };

    ToolTextEditor20250429 text_editor_429{
        .name = {{}},
        .type = {{}}
    };

    ToolTextEditor20250728 text_editor_728{
        .name = {{}},
        .type = {{}},
        .max_characters = 50000
    };

    req.tools = std::vector<ToolUnion>{
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

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{
                    std::vector<ContentBlockParam>{
                        ToolResultBlockParam{
                            .tool_use_id = std::string{"toolu_abc123"},
                            .type = {{}},
                            .content = ToolResultBlockParam::Content{
                                ToolResultBlockParam::TextBlockParam{
                                    .text = std::string{"The result text here."},
                                    .type = {{}}
                                }
                            },
                            .is_error = false
                        },
                        ToolResultBlockParam{
                            .tool_use_id = std::string{"toolu_def456"},
                            .type = {{}},
                            .content = ToolResultBlockParam::Content{
                                std::string{"Error: command not found"}
                            },
                            .is_error = true
                        }
                    }
                },
                .role = Role::USER
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    auto serialized = anthropic::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    // First tool result has content
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

    Request req{
        .max_tokens = 1024,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{
                    std::vector<ContentBlockParam>{
                        ImageBlockParam{
                            .source = ImageSource{
                                URLImageSource{
                                    .type = {{}},
                                    .url = "https://example.com/image.png"
                                }
                            },
                            .type = {{}}
                        },
                        TextBlockParam{
                            .text = std::string{"Describe this image."},
                            .type = {{}}
                        }
                    }
                },
                .role = Role::USER
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

    Request req{
        .max_tokens = 4096,
        .messages = std::vector<MessageParam>{
            MessageParam{
                .content = MessageParam::Content{
                    std::vector<ContentBlockParam>{
                        TextBlockParam{
                            .text = std::string{"Important cached text."},
                            .type = {{}},
                            .cache_control = CacheControlEphemeral{
                                .type = {{}},
                                .ttl = CacheControlTTL::V_5M
                            }
                        }
                    }
                },
                .role = Role::USER
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

    REQUIRE(resp.stop_reason.value() == StopReason::STOP_SEQUENCE);
    REQUIRE(resp.stop_sequence.value() == "---");

    auto content = resp.content.value();
    REQUIRE(content.size() == 1);
    auto& text = std::get<TextBlock>(content[0]);
    REQUIRE(text.text.value().find("---") != std::string::npos);

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

    Tool custom_tool{
        .input_schema = json::Object{},
        .name = std::string{"get_weather"},
        .description = std::string{"Get weather for a location"}
    };
    custom_tool.input_schema.value()["type"] = "object";
    custom_tool.input_schema.value()["properties"] = properties;
    custom_tool.input_schema.value()["required"] = json::Array{json::Value{std::string{"location"}}};

    // Build output_config with JSON schema
    json::Object schema;
    schema["type"] = "object";
    json::Object schema_props;
    json::Object answer_prop;
    answer_prop["type"] = "string";
    schema_props["answer"] = answer_prop;
    schema["properties"] = schema_props;

    Request req{
        .max_tokens = 8192,
        .messages = std::vector<MessageParam>{
            // User message with multi-type content blocks
            MessageParam{
                .content = MessageParam::Content{
                    std::vector<ContentBlockParam>{
                        TextBlockParam{
                            .text = std::string{"Analyze this image and run a tool."},
                            .type = {{}},
                            .cache_control = CacheControlEphemeral{
                                .type = {{}},
                                .ttl = CacheControlTTL::V_1H
                            }
                        },
                        ImageBlockParam{
                            .source = ImageSource{
                                URLImageSource{
                                    .type = {{}},
                                    .url = "https://example.com/photo.jpg"
                                }
                            },
                            .type = {{}}
                        }
                    }
                },
                .role = Role::USER
            },
            // Assistant message with string content
            MessageParam{
                .content = MessageParam::Content{std::string{"I'll analyze that for you."}},
                .role = Role::ASSISTANT
            }
        },
        .model = std::string{"claude-sonnet-4-20250514"}
    };

    // Set ALL optional fields
    req.metadata = Metadata{.user_id = std::string{"user_42"}};
    req.output_config = OutputConfig{};
    req.output_config.value().format = OutputConfig::JSONOutputFormat{
        .schema = schema,
        .type = {{}}
    };
    req.service_tier = RequestServiceTier::AUTO;
    req.stop_sequences = std::vector<std::string>{"---", "END"};
    req.stream = false;
    req.system = System{
        std::vector<SystemTextBlockParam>{
            SystemTextBlockParam{
                .text = std::string{"You are a helpful assistant."},
                .type = {{}},
                .cache_control = Request::TextBlockParam::CacheControlEphemeral{.type = {{}}}
            }
        }
    };
    req.temperature = 0.8;
    req.thinking = ThinkingConfig{
        ThinkingConfigEnabled{
            .budget_tokens = 5000,
            .type = {{}}
        }
    };
    req.tool_choice = ToolChoice{
        ToolChoiceTool{
            .name = std::string{"get_weather"},
            .type = {{}}
        }
    };
    req.tools = std::vector<ToolUnion>{custom_tool};
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
    REQUIRE(resp.id.value() == "msg_full_001");
    REQUIRE(resp.model.value() == "claude-sonnet-4-20250514");
    REQUIRE(resp.stop_reason.value() == StopReason::STOP_SEQUENCE);
    REQUIRE(resp.stop_sequence.value() == "---");

    // Content: 3 blocks (thinking, text, tool_use)
    auto content = resp.content.value();
    REQUIRE(content.size() == 3);

    // Block 0: ThinkingBlock
    auto* thinking = std::get_if<ThinkingBlock>(&content[0]);
    REQUIRE(thinking != nullptr);
    REQUIRE(thinking->signature.value() == "EqoB...");
    REQUIRE(thinking->thinking.value().find("step by step") != std::string::npos);

    // Block 1: TextBlock with 2 citations
    auto* text = std::get_if<TextBlock>(&content[1]);
    REQUIRE(text != nullptr);
    REQUIRE(text->text.value().find("analysis") != std::string::npos);
    REQUIRE(text->citations.value().size() == 2);

    auto* char_cite = std::get_if<CitationCharLocation>(&text->citations.value()[0]);
    REQUIRE(char_cite != nullptr);
    REQUIRE(char_cite->cited_text.value() == "important finding");
    REQUIRE(char_cite->document_title.value() == "Research Paper");
    REQUIRE(char_cite->start_char_index.value() == 100);
    REQUIRE(char_cite->end_char_index.value() == 150);
    REQUIRE(char_cite->file_id.value() == "file_abc123");

    auto* web_cite = std::get_if<CitationsWebSearchResultLocation>(&text->citations.value()[1]);
    REQUIRE(web_cite != nullptr);
    REQUIRE(web_cite->title.value() == "Web Article");

    // Block 2: ToolUseBlock
    auto* tool_use = std::get_if<ToolUseBlock>(&content[2]);
    REQUIRE(tool_use != nullptr);
    REQUIRE(tool_use->id.value() == "toolu_full_001");
    REQUIRE(tool_use->name.value() == "get_weather");

    // Usage — all sub-fields
    auto& usage = resp.usage.value();
    REQUIRE(usage.input_tokens.value() == 1500);
    REQUIRE(usage.output_tokens.value() == 350);
    REQUIRE(usage.cache_creation_input_tokens.value() == 300);
    REQUIRE(usage.cache_read_input_tokens.value() == 500);
    REQUIRE(usage.cache_creation.value().ephemeral_1h_input_tokens.value() == 200);
    REQUIRE(usage.cache_creation.value().ephemeral_5m_input_tokens.value() == 100);
    REQUIRE(usage.server_tool_use.value().web_search_requests.value() == 3);
    REQUIRE(usage.service_tier.value() == UsageServiceTier::PRIORITY);

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
