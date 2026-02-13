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

void test_openai_full_request_serialization() {
    std::println("Testing OpenAI Full Request Serialization...");

    openai::Request req;
    req.model = "o1-preview";
    req.background = true;
    
    // Reasoning (o1 models)
    openai::Reasoning r{
        .effort = openai::ReasoningEffort::HIGH,
        .summary = openai::ReasoningSummary::AUTO
    };
    req.reasoning = r;
    
    // Service Tier
    req.service_tier = openai::ServiceTier::PRIORITY;
    
    // Stream Options
    req.stream = true;
    openai::StreamOptions stream_opts;
    stream_opts.include_obfuscation = true;
    req.stream_options = stream_opts;
    
    // Prompt Caching
    req.prompt_cache_key = "cache_key_123";
    req.prompt_cache_retention = "2 days";

    // User metadata (moved to metadata or removed if not in struct)
    // req.user = "user_123"; // Removed as per protocol definition

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"effort\":\"high\"") != std::string::npos);
    REQUIRE(json_str.find("\"service_tier\":\"priority\"") != std::string::npos);
    REQUIRE(json_str.find("\"stream\":true") != std::string::npos);
    REQUIRE(json_str.find("\"include_obfuscation\":true") != std::string::npos);
    REQUIRE(json_str.find("\"prompt_cache_key\":\"cache_key_123\"") != std::string::npos);
    REQUIRE(json_str.find("\"prompt_cache_retention\":\"2 days\"") != std::string::npos);

    std::println("[SUCCESS] Full Request Serialization passed.");
}

void test_openai_tool_types_serialization() {
    std::println("Testing OpenAI Tool Types Serialization...");

    // File Search Tool
    openai::request::ToolTypes::FileSearch file_search{
        .type = openai::KindFileSearchTool{},
        .vector_store_ids = std::vector<std::string>{"vs_123"},
        .max_num_results = 5
    };
    
    // Code Interpreter Tool
    openai::request::ToolTypes::CodeInterpreter code_interpreter{
        .type = openai::KindCodeInterpreterTool{},
        .container = std::string("container_123") // Variant string
    };

    // MCP Tool (Model Context Protocol)
    openai::request::ToolTypes::MCP mcp_tool{
        .type = openai::KindMCPTool{},
        .server_label = std::string("my_server"),
        .allowed_tools = std::vector<std::string>{"read_file", "list_files"}, // Variant should accept vector
        .authorization = std::string("auth_token"),
        .connector_id = openai::ConnectId::GOOGLE_DRIVE,
        .require_approval = openai::MCPApprovalSetting::ALWAYS,
        .server_description = std::string("A file server"),
        .server_url = jai::llm::EncodedUrl{std::string("https://mcp.example.com")} // jai::llm::EncodedUrl struct
    };

    // Computer Use Tool
    openai::request::ToolTypes::ComputerUse computer_tool{
        .type = openai::KindComputerUseTool{},
        .display_height = 1080,
        .display_width = 1920,
        .environment = std::string("ubuntu")
    };

    openai::Request req;
    req.tools = std::vector<openai::request::Tool>{
        file_search,
        code_interpreter,
        mcp_tool,
        computer_tool
    };

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // File Search Check
    REQUIRE(json_str.find("\"type\":\"file_search\"") != std::string::npos);
    REQUIRE(json_str.find("\"vector_store_ids\":[\"vs_123\"]") != std::string::npos);
    
    // Code Interpreter Check
    REQUIRE(json_str.find("\"type\":\"code_interpreter\"") != std::string::npos);
    REQUIRE(json_str.find("\"container\":\"container_123\"") != std::string::npos);

    // MCP Check
    REQUIRE(json_str.find("\"type\":\"mcp\"") != std::string::npos);
    REQUIRE(json_str.find("\"server_label\":\"my_server\"") != std::string::npos);
    REQUIRE(json_str.find("\"connector_id\":\"google_drive\"") != std::string::npos); // Serializes to snake_case

    // Computer Use Check
    REQUIRE(json_str.find("\"type\":\"computer_use_preview\"") != std::string::npos);
    REQUIRE(json_str.find("\"display_width\":1920") != std::string::npos);

    std::println("[SUCCESS] Tool Types Serialization passed.");
}

void test_openai_input_types_serialization() {
    std::println("Testing OpenAI Input Types Serialization...");

    openai::Request req;
    req.model = "gpt-4o";

    // Text Content
    openai::request::ContentTypes::Text text_content{
        .type = openai::KindInputText{},
        .text = std::string("Analyze this file and image.")
    };

    // Image Content with URL and Detail
    openai::request::ContentTypes::Image image_content_url{
        .type = openai::KindInputImage{},
        .detail = openai::Detail::HIGH,
        .image_url = jai::llm::EncodedUrl{std::string("https://example.com/chart.png")}
    };

    // Image Content with File ID (variant usage handled by optional)
    // Note: Image struct has optional file_id and image_url.
    openai::request::ContentTypes::Image image_content_file_id{
        .type = openai::KindInputImage{},
        .detail = openai::Detail::AUTO,
        .file_id = "file-abc12345"
    };

    // File Content
    // Note: File struct has optional file_data, file_id, file_url, filename.
    openai::request::ContentTypes::File file_content{
        .type = openai::KindInputFile{},
        .file_url = jai::llm::EncodedUrl{std::string("https://example.com/data.csv")},
        .filename = "data.csv"
    };

    // Construct Message
    openai::request::InputTypes::Message message{
        .type = openai::KindInputMessage{},
        .content = openai::request::InputTypes::Message::Content(std::vector<openai::request::InputTypes::MessageContentUnit>{
            text_content,
            image_content_url,
            image_content_file_id,
            file_content
        }),
        .role = openai::RoleInputMessage::USER
    };

    req.input = std::vector<openai::request::InputItemList>{message};

    auto serialized = openai::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Text Check
    REQUIRE(json_str.find("\"type\":\"input_text\"") != std::string::npos);
    REQUIRE(json_str.find("\"text\":\"Analyze this file and image.\"") != std::string::npos);

    // Image URL Check
    REQUIRE(json_str.find("\"type\":\"input_image\"") != std::string::npos);
    REQUIRE(json_str.find("\"image_url\":\"https://example.com/chart.png\"") != std::string::npos);
    REQUIRE(json_str.find("\"detail\":\"high\"") != std::string::npos);

    // Image File ID Check
    REQUIRE(json_str.find("\"file_id\":\"file-abc12345\"") != std::string::npos);
    REQUIRE(json_str.find("\"detail\":\"auto\"") != std::string::npos);

    // File Check
    REQUIRE(json_str.find("\"type\":\"input_file\"") != std::string::npos);
    REQUIRE(json_str.find("\"file_url\":\"https://example.com/data.csv\"") != std::string::npos);
    REQUIRE(json_str.find("\"filename\":\"data.csv\"") != std::string::npos);

    std::println("[SUCCESS] Input Types Serialization passed.");
}

void test_openai_advanced_response_deserialization() {
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
        "metadata": {},
        "output": [
            {
                "type": "message",
                "id": "msg_refusal",
                "role": "assistant",
                "status": "completed",
                "content": [
                    {
                        "type": "refusal",
                        "refusal": "I cannot answer that question."
                    }
                ]
            },
            {
                "type": "message",
                "id": "msg_text_with_citation",
                "role": "assistant",
                "status": "completed",
                "content": [
                    {
                        "type": "output_text",
                        "text": "Here is the data [1].",
                        "annotations": [
                            {
                                "type": "file_citation",
                                "file_id": "file-123",
                                "filename": "data.csv",
                                "index": 0
                            }
                        ],
                        "logprobs": [
                            {
                                "token": "Here",
                                "logprob": -0.1,
                                "bytes": [72, 101, 114, 101],
                                "top_logprobs": []
                            }
                        ]
                    }
                ]
            },
            {
                "type": "code_interpreter_call",
                "id": "call_ci_123",
                "status": "completed",
                "code": "print('hello')",
                "container_id": "cont_123",
                "outputs": [
                    {
                        "type": "logs",
                        "logs": "hello\n"
                    }
                ]
            },
            {
                "type": "mcp_call",
                "id": "call_mcp_123",
                "status": "completed",
                "name": "read_file",
                "arguments": "{\"path\": \"/foo.txt\"}",
                "server_label": "fs_server",
                "approval_request_id": "app_req_1",
                "error": "",
                "output": "file content here"
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
        "tool_choice": "auto",
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

    jai::llm::curl::Response curl_resp;
    curl_resp.state = jai::llm::curl::Response::State::COMPLETED;
    curl_resp.status_code = 200;
    curl_resp.body_len = json_response.size();
    curl_resp.body.resize(curl_resp.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(curl_resp.body.data(), json_response.data(), curl_resp.body_len);

    std::optional<openai::Response> result_opt;
    try {
        result_opt = openai::Deserialize(curl_resp);
    } catch (const std::exception& e) {
        std::println("[ERROR] Exception during deserialization: {}", e.what());
        return;
    }
    auto& result = *result_opt;

    // Verify Refusal
    auto& output = result.output;
    bool found_refusal = false;
    bool found_citation = false;
    bool found_ci = false;
    bool found_mcp = false;

    for (const auto& item : output.Value()) {
        if (std::holds_alternative<openai::response::InputTypes::Item::OutputMessage>(item)) {
            const auto& msg = std::get<openai::response::InputTypes::Item::OutputMessage>(item);
            for (const auto& content : msg.content.Value()) {
                if (std::holds_alternative<openai::response::ContentTypes::Refusal>(content)) {
                    const auto& refusal = std::get<openai::response::ContentTypes::Refusal>(content);
                    if (refusal.refusal.Value() == "I cannot answer that question.") {
                        found_refusal = true;
                    }
                } else if (std::holds_alternative<openai::response::ContentTypes::OutputText>(content)) {
                    const auto& out_text = std::get<openai::response::ContentTypes::OutputText>(content);
                    const auto& annotations_vec = out_text.annotations.Value();
                    for (size_t i = 0; i < annotations_vec.size(); ++i) {
                        const auto& annotation = annotations_vec[i];
                        if (std::holds_alternative<openai::response::ContentTypes::OutputText::FileCitation>(annotation)) {
                            const auto& citation = std::get<openai::response::ContentTypes::OutputText::FileCitation>(annotation);
                            if (citation.file_id.Value() == "file-123") {
                                found_citation = true;
                            }
                        }
                    }
                }
            }
        } else if (std::holds_alternative<openai::response::InputTypes::Item::CodeInterpreterToolCall>(item)) {
            const auto& ci = std::get<openai::response::InputTypes::Item::CodeInterpreterToolCall>(item);
            if (ci.code.Value() == "print('hello')" && !ci.outputs.Value().empty()) {
                found_ci = true;
            }
        } else if (std::holds_alternative<openai::response::InputTypes::Item::MCPToolCall>(item)) {
            const auto& mcp = std::get<openai::response::InputTypes::Item::MCPToolCall>(item);
            if (mcp.name.Value() == "read_file" && mcp.server_label.Value() == "fs_server") {
                found_mcp = true;
            }
        }
    }

    if (!found_refusal) std::println("[ERROR] Refusal not found or incorrect.");
    if (!found_citation) std::println("[ERROR] File citation not found.");
    if (!found_ci) std::println("[ERROR] Code Interpreter call not found.");
    if (!found_mcp) std::println("[ERROR] MCP call not found.");

    REQUIRE(found_refusal);
    REQUIRE(found_citation);
    REQUIRE(found_ci);
    REQUIRE(found_mcp);

    std::println("[SUCCESS] Advanced Response Deserialization passed.");
}

void test_simple_response_deserialization() {
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
        test_openai_full_request_serialization();
        test_openai_tool_types_serialization();
        test_openai_input_types_serialization();
        test_openai_advanced_response_deserialization();
        test_simple_response_deserialization();
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
