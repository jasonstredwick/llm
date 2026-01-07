#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../../interface/llm.hpp"

namespace jai::llm::providers::openai_4 {

/**
 * OpenAI GPT-4 - Chat Completions API
 * Isolated "Shared-Nothing" Header
 */

constexpr std::string_view ENDPOINT = "https://api.openai.com/v1/chat/completions";

// --- Type Glossary (Enums) ---
enum class ContentPartType { IMAGE_URL, TEXT };
enum class FinishReason { CONTENT_FILTER, FINISH_REASON_UNSPECIFIED, LENGTH, STOP, TOOL_CALLS };
enum class ImageDetail { AUTO, HIGH, LOW };
enum class ObjectType { CHAT_COMPLETION };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, DEVELOPER, SYSTEM, TOOL, USER };
enum class ToolType { FUNCTION };

// --- Request Structures ---

struct ToolCall {
    struct Function {
        std::string name;
        std::string arguments; // JSON string
    };
    std::string id;
    ToolType type = ToolType::FUNCTION;
    Function function;
};

struct ChatCompletionMessage {
    struct ContentPart {
        struct Image {
            struct ImageUrl {
                std::string url;
                std::optional<ImageDetail> detail;
            };
            ContentPartType type = ContentPartType::IMAGE_URL;
            ImageUrl image_url;
        };
        struct Text {
            ContentPartType type = ContentPartType::TEXT;
            std::string text;
        };

        using Part = std::variant<Text, Image>;
    };

    Role role = Role::USER;
    /**
     * OpenAI Constraint: 'developer' and 'system' roles MUST be text-only (std::string).
     * 'user' and 'assistant' roles can be multimodal (vector of parts).
     */
    std::variant<std::string, std::vector<ContentPart::Part>> content;
    std::optional<std::string> name;
    std::optional<std::string> tool_call_id;
    std::vector<ToolCall> tool_calls;
};

struct ChatCompletionRequest {
    struct ResponseFormat {
        struct JsonSchema {
            std::string name;
            std::optional<std::string> description;
            std::string schema; // JSON Schema string
            std::optional<bool> strict;
        };
        ResponseFormatType type = ResponseFormatType::TEXT;
        std::optional<JsonSchema> json_schema;
    };
    struct Tool {
        struct Function {
            std::string name;
            std::optional<std::string> description;
            std::string parameters; // JSON Schema string
            std::optional<bool> strict;
        };
        ToolType type = ToolType::FUNCTION;
        Function function;
    };

    std::string model;
    std::vector<ChatCompletionMessage> messages;
    std::optional<uint32_t> max_tokens;
    std::optional<double> temperature;
    std::optional<double> top_p;
    std::optional<uint32_t> n;
    std::vector<std::string> stop;
    std::optional<double> presence_penalty;
    std::optional<double> frequency_penalty;
    std::optional<std::string> user;
    std::optional<uint64_t> seed;
    std::optional<ResponseFormat> response_format;
    std::vector<Tool> tools;
};

// --- Response Structures ---

struct ResponseTelemetry {
    uint32_t processing_ms = 0;
    std::string request_id;
    std::optional<std::string> organization;
    std::optional<std::string> version_header;
};

struct UsageMetadata {
    struct PromptTokensDetails {
        uint32_t cached_tokens = 0;
    };
    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;
    PromptTokensDetails prompt_tokens_details;
};

struct ChatCompletionResponse {
    struct Choice {
        struct Message {
            Role role = Role::ASSISTANT;
            std::optional<std::string> content;
            std::vector<ToolCall> tool_calls;
        };
        uint32_t index = 0;
        FinishReason finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
        Message message;
    };

    std::string id;
    ObjectType object = ObjectType::CHAT_COMPLETION;
    uint64_t created = 0;
    std::string model;
    std::optional<std::string> system_fingerprint;
    std::vector<Choice> choices;
    UsageMetadata usage;
    std::optional<ResponseTelemetry> telemetry;
};

} // namespace jai::llm::providers::openai_4

namespace jai::llm {

template<>
constexpr std::string_view to_string_view(providers::openai_4::Role val) {
    switch (val) {
        case providers::openai_4::Role::USER: return "user";
        case providers::openai_4::Role::SYSTEM: return "system";
        case providers::openai_4::Role::ASSISTANT: return "assistant";
        case providers::openai_4::Role::TOOL: return "tool";
        case providers::openai_4::Role::DEVELOPER: return "developer";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::ContentPartType val) {
    switch (val) {
        case providers::openai_4::ContentPartType::TEXT: return "text";
        case providers::openai_4::ContentPartType::IMAGE_URL: return "image_url";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::ImageDetail val) {
    switch (val) {
        case providers::openai_4::ImageDetail::AUTO: return "auto";
        case providers::openai_4::ImageDetail::LOW: return "low";
        case providers::openai_4::ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::FinishReason val) {
    switch (val) {
        case providers::openai_4::FinishReason::STOP: return "stop";
        case providers::openai_4::FinishReason::LENGTH: return "length";
        case providers::openai_4::FinishReason::CONTENT_FILTER: return "content_filter";
        case providers::openai_4::FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::ToolType val) {
    switch (val) {
        case providers::openai_4::ToolType::FUNCTION: return "function";
        default: return "function";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::ResponseFormatType val) {
    switch (val) {
        case providers::openai_4::ResponseFormatType::TEXT: return "text";
        case providers::openai_4::ResponseFormatType::JSON_OBJECT: return "json_object";
        case providers::openai_4::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}

template<>
constexpr std::string_view to_string_view(providers::openai_4::ObjectType val) {
    switch (val) {
        case providers::openai_4::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}

} // namespace jai::llm
