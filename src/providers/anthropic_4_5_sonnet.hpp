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
};

struct MessageContentPartImage {
    std::string type = "image";
    ImageSource source;
};

using MessageContentPart = std::variant<MessageContentPartText, MessageContentPartImage>;

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
    std::optional<std::string> system;

    uint32_t max_tokens = 4096;
    
    std::optional<ThinkingConfig> thinking; // Sonnet 4.5 also supports thinking mode
    
    std::vector<std::string> stop_sequences;
    std::optional<double> temperature;
    std::optional<double> top_p;
    std::optional<uint32_t> top_k;
};

struct UsageMetadata {
    uint32_t input_tokens = 0;
    uint32_t output_tokens = 0;
};

struct MessageResponse {
    std::string id;
    std::string model;

    struct ContentBlock {
        std::string type; // "text", "thinking"
        std::optional<std::string> text;
        std::optional<std::string> thinking;
    };
    std::vector<ContentBlock> content;

    std::optional<std::string> stop_reason;
    UsageMetadata usage;
};

} // namespace jai::llm::providers::anthropic_4_5_sonnet
