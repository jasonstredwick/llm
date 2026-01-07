#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../../interface/llm.hpp"

namespace jai::llm::providers::anthropic_4_5_sonnet {

/**
 * Anthropic Claude 4.5 Sonnet - Messages API (2026 Snapshot)
 * Isolated "Shared-Nothing" Header
 */

constexpr std::string_view ENDPOINT = "https://api.anthropic.com/v1/messages";
constexpr std::string_view API_VERSION = "2023-06-01";

// --- Type Glossary (Enums) ---
enum class CacheType { EPHEMERAL };
enum class CitationType { CHAR_LOCATION, PAGE_LOCATION };
enum class ContentBlockType { TEXT, THINKING, TOOL_USE };
enum class MessageContentPartType { AUDIO, DOCUMENT, IMAGE, TEXT, TOOL_RESULT, TOOL_USE };
enum class MessageType { ERROR, MESSAGE };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, USER };
enum class SourceType { BASE64, URL };
enum class StopReason { END_TURN, MAX_TOKENS, STOP_SEQUENCE, TOOL_USE };
enum class ThinkingType { ENABLED };
enum class ToolChoiceType { ANY, AUTO, TOOL };

// --- Request Structures ---

struct CacheControl {
    CacheType type = CacheType::EPHEMERAL;
};

struct ResponseFormat {
    ResponseFormatType type = ResponseFormatType::TEXT;
    std::optional<std::string> json_schema; // JSON Schema string
};

struct Message {
    struct ContentPart {
        struct Audio {
            struct Source {
                SourceType type = SourceType::BASE64;
                std::string media_type;
                std::string data;
            };
            MessageContentPartType type = MessageContentPartType::AUDIO;
            Source source;
            std::optional<CacheControl> cache_control;
        };
        struct Document {
            struct Source {
                SourceType type = SourceType::BASE64;
                std::string media_type;
                std::string data;
            };
            MessageContentPartType type = MessageContentPartType::DOCUMENT;
            Source source;
            std::optional<CacheControl> cache_control;
        };
        struct Image {
            struct Source {
                SourceType type = SourceType::BASE64;
                std::optional<std::string> media_type;
                std::optional<std::string> data;
                std::optional<std::string> url;
            };
            MessageContentPartType type = MessageContentPartType::IMAGE;
            Source source;
            std::optional<CacheControl> cache_control;
        };
        struct Text {
            MessageContentPartType type = MessageContentPartType::TEXT;
            std::string text;
            std::optional<CacheControl> cache_control;
        };
        struct ToolResult {
            MessageContentPartType type = MessageContentPartType::TOOL_RESULT;
            std::string tool_use_id;
            std::string content;
            std::optional<bool> is_error;
            std::optional<CacheControl> cache_control;
        };
        struct ToolUse {
            MessageContentPartType type = MessageContentPartType::TOOL_USE;
            std::string id;
            std::string name;
            std::string input; // JSON string
            std::optional<CacheControl> cache_control;
        };

        using Part = std::variant<Text, Image, Audio, Document, ToolUse, ToolResult>;
    };

    Role role = Role::USER;
    std::variant<std::string, std::vector<ContentPart::Part>> content;
};

struct MessageRequest {
    struct SystemPrompt {
        std::string text;
        std::optional<CacheControl> cache_control;
    };
    struct ThinkingConfig {
        ThinkingType type = ThinkingType::ENABLED;
        uint32_t budget_tokens = 0;
    };
    struct Tool {
        struct Custom {
            std::string name;
            std::string description;
            std::string input_schema; // JSON Schema string
        };
        struct Computer {
            std::string type = "computer_20241022";
            std::string name = "computer";
            uint32_t display_width_px = 0;
            uint32_t display_height_px = 0;
            std::optional<uint32_t> display_number;
        };
        struct Bash {
            std::string type = "bash_20241022";
            std::string name = "bash";
        };
        struct TextEditor {
            std::string type = "text_editor_20250124";
            std::string name = "str_replace_editor";
        };

        using Config = std::variant<Custom, Computer, Bash, TextEditor>;
        Config config;
        std::optional<CacheControl> cache_control;
    };
    struct ToolChoice {
        ToolChoiceType type = ToolChoiceType::AUTO;
        std::optional<std::string> name;
        std::optional<bool> disable_parallel_tool_use;
    };

    std::string model;
    std::vector<Message> messages;
    std::variant<std::string, std::vector<SystemPrompt>> system;
    uint32_t max_tokens = 4096;
    std::optional<ThinkingConfig> thinking; 
    std::vector<std::string> stop_sequences;
    std::optional<ResponseFormat> response_format;
    std::optional<double> temperature;
    std::optional<double> top_p;
    std::optional<uint32_t> top_k;
    std::vector<Tool> tools;
    std::optional<ToolChoice> tool_choice;
};

// --- Response Structures ---

struct ResponseTelemetry {
    uint32_t processing_ms = 0;
    std::string request_id;
    std::optional<std::string> organization;
    std::optional<std::string> version_header;
};

struct UsageMetadata {
    uint32_t input_tokens = 0;
    uint32_t output_tokens = 0;
    uint32_t thinking_tokens = 0;
    uint32_t cache_read_input_tokens = 0;
    uint32_t cache_creation_input_tokens = 0;
};

struct MessageResponse {
    struct ContentBlock {
        struct Citation {
            struct Source {
                SourceType type = SourceType::BASE64;
                std::optional<std::string> media_type;
                std::optional<std::string> data;
                std::optional<std::string> file_id;
            };
            CitationType type = CitationType::CHAR_LOCATION;
            Source source;
            std::string cite;
            uint32_t start_index = 0;
            uint32_t end_index = 0;
            std::optional<uint32_t> start_page_number;
            std::optional<uint32_t> end_page_number;
        };

        ContentBlockType type = ContentBlockType::TEXT;
        std::optional<std::string> text;
        std::optional<std::string> thinking;
        std::optional<std::string> id;
        std::optional<std::string> name;
        std::optional<std::string> input;
        std::vector<Citation> citations;
    };

    std::string id;
    MessageType type = MessageType::MESSAGE;
    Role role = Role::ASSISTANT;
    std::string model;
    std::vector<ContentBlock> content;
    std::optional<StopReason> stop_reason;
    std::optional<std::string> stop_sequence;
    UsageMetadata usage;
    std::optional<ResponseTelemetry> telemetry;
};

} // namespace jai::llm::providers::anthropic_4_5_sonnet

namespace jai::llm {

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::Role val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::Role::USER: return "user";
        case providers::anthropic_4_5_sonnet::Role::ASSISTANT: return "assistant";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::MessageContentPartType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::MessageContentPartType::TEXT: return "text";
        case providers::anthropic_4_5_sonnet::MessageContentPartType::IMAGE: return "image";
        case providers::anthropic_4_5_sonnet::MessageContentPartType::AUDIO: return "audio";
        case providers::anthropic_4_5_sonnet::MessageContentPartType::DOCUMENT: return "document";
        case providers::anthropic_4_5_sonnet::MessageContentPartType::TOOL_USE: return "tool_use";
        case providers::anthropic_4_5_sonnet::MessageContentPartType::TOOL_RESULT: return "tool_result";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::SourceType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::SourceType::BASE64: return "base64";
        case providers::anthropic_4_5_sonnet::SourceType::URL: return "url";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::ThinkingType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::ThinkingType::ENABLED: return "enabled";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::StopReason val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::StopReason::END_TURN: return "end_turn";
        case providers::anthropic_4_5_sonnet::StopReason::MAX_TOKENS: return "max_tokens";
        case providers::anthropic_4_5_sonnet::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case providers::anthropic_4_5_sonnet::StopReason::TOOL_USE: return "tool_use";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::ContentBlockType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::ContentBlockType::TEXT: return "text";
        case providers::anthropic_4_5_sonnet::ContentBlockType::THINKING: return "thinking";
        case providers::anthropic_4_5_sonnet::ContentBlockType::TOOL_USE: return "tool_use";
        default: return "";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::MessageType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::MessageType::MESSAGE: return "message";
        case providers::anthropic_4_5_sonnet::MessageType::ERROR: return "error";
        default: return "message";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::ToolChoiceType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::ToolChoiceType::AUTO: return "auto";
        case providers::anthropic_4_5_sonnet::ToolChoiceType::ANY: return "any";
        case providers::anthropic_4_5_sonnet::ToolChoiceType::TOOL: return "tool";
        default: return "auto";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::CacheType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::CacheType::EPHEMERAL: return "ephemeral";
        default: return "ephemeral";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::ResponseFormatType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::ResponseFormatType::TEXT: return "text";
        case providers::anthropic_4_5_sonnet::ResponseFormatType::JSON_OBJECT: return "json_object";
        case providers::anthropic_4_5_sonnet::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}

template<>
constexpr std::string_view to_string_view(providers::anthropic_4_5_sonnet::CitationType val) {
    switch (val) {
        case providers::anthropic_4_5_sonnet::CitationType::CHAR_LOCATION: return "char_location";
        case providers::anthropic_4_5_sonnet::CitationType::PAGE_LOCATION: return "page_location";
        default: return "";
    }
}

} // namespace jai::llm
