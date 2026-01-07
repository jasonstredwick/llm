#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>

namespace jai::llm::providers::anthropic_4_5_sonnet {

using namespace jai::llm::providers;

/**
 * Anthropic Claude 4.5 Sonnet Messages API Structures
 * Supports Extended Thinking but lacks the effort control of Opus.
 */

struct ImageSource {
    std::string type = "base64";
    std::string media_type;
    std::string data;
};

struct MessageContentPartText {
    std::string type = "text";
    std::string text;

    struct CacheControl {
        std::string type = "ephemeral";
    };
    std::optional<CacheControl> cache_control;
};

struct MessageContentPartImage {
    std::string type = "image";
    ImageSource source;

    struct CacheControl {
        std::string type = "ephemeral";
    };
    std::optional<CacheControl> cache_control;
};

struct MessageContentPartToolUse {
    std::string type = "tool_use";
    std::string id;
    std::string name;
    std::string input; // JSON string

    struct CacheControl {
        std::string type = "ephemeral";
    };
    std::optional<CacheControl> cache_control;
};

struct MessageContentPartToolResult {
    std::string type = "tool_result";
    std::string tool_use_id;
    std::string content;
    std::optional<bool> is_error;

    struct CacheControl {
        std::string type = "ephemeral";
    };
    std::optional<CacheControl> cache_control;
};

using MessageContentPart = std::variant<
    MessageContentPartText, 
    MessageContentPartImage,
    MessageContentPartToolUse,
    MessageContentPartToolResult
>;

struct Message {
    Role role = Role::USER;
    std::variant<std::string, std::vector<MessageContentPart>> content;
};

struct ThinkingConfig {
    std::string type = "enabled";
    uint32_t budget_tokens = 0;
};

struct MessageRequest {
    std::string model;
    std::vector<Message> messages;
    
    struct SystemPrompt {
        std::string text;
        struct CacheControl {
            std::string type = "ephemeral";
        };
        std::optional<CacheControl> cache_control;
    };
    std::variant<std::string, std::vector<SystemPrompt>> system;

    uint32_t max_tokens = 4096;
    
    std::optional<ThinkingConfig> thinking; // Sonnet 4.5 also supports thinking mode
    
    std::vector<std::string> stop_sequences;
    std::optional<double> temperature;
    std::optional<double> top_p;
    std::optional<uint32_t> top_k;

    struct Tool {
        std::string name;
        std::string description;
        std::string input_schema; // JSON Schema as string

        struct CacheControl {
            std::string type = "ephemeral";
        };
        std::optional<CacheControl> cache_control;
    };
    std::vector<Tool> tools;

    struct ToolChoice {
        std::string type; // "auto", "any", "tool"
        std::optional<std::string> name;
    };
    std::optional<ToolChoice> tool_choice;
};

struct UsageMetadata {
    uint32_t input_tokens = 0;
    uint32_t output_tokens = 0;
    uint32_t thinking_tokens = 0;
    uint32_t cache_read_input_tokens = 0;
    uint32_t cache_creation_input_tokens = 0;
};

struct MessageResponse {
    std::string id;
    std::string model;

    struct ContentBlock {
        std::string type; // "text", "thinking", "tool_use"
        std::optional<std::string> text;
        std::optional<std::string> thinking;

        // tool_use specifically
        std::optional<std::string> id;
        std::optional<std::string> name;
        std::optional<std::string> input;

        struct Citation {
            std::string type; // "char_location", "page_location"
            struct Source {
                std::string type; // "base64", "url"
                std::string media_type;
                std::string data;
            } source;
            uint32_t start_index = 0;
            uint32_t end_index = 0;
            std::optional<uint32_t> start_page;
            std::optional<uint32_t> end_page;
        };
        std::vector<Citation> citations;
    };
    std::vector<ContentBlock> content;

    std::optional<std::string> stop_reason; // "end_turn", "max_tokens", "stop_sequence", "tool_use", "refusal"
    UsageMetadata usage;
    std::optional<ResponseTelemetry> telemetry;
};

} // namespace jai::llm::providers::anthropic_4_5_sonnet
