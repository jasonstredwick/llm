#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

namespace jai::llm::providers::anthropic_4_5_opus {

/**
 * Anthropic Claude 4.5 Opus - Messages API (2026 Snapshot)
 * Isolated "Shared-Nothing" Header
 */

constexpr std::string_view ENDPOINT = "https://api.anthropic.com/v1/messages";
constexpr std::string_view API_VERSION = "2023-06-01"; // Default version for 4.5 series

// --- Type Glossary (Enums) ---
enum class CacheType { EPHEMERAL };
enum class CitationType { CHAR_LOCATION, PAGE_LOCATION };
enum class ContentBlockType { TEXT, THINKING, TOOL_USE };
enum class EffortLevel { HIGH, LOW, MEDIUM };
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
                /**
                 * For BASE64: media_type and data are required.
                 * For URL: url is required.
                 */
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
    std::optional<EffortLevel> effort;
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

} // namespace jai::llm::providers::anthropic_4_5_opus
