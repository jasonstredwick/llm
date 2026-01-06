#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>

namespace jai::llm::providers::openai_4o {

using namespace jai::llm::providers;

/**
 * OpenAI GPT-4o ("Omni") Chat Completions API Structures
 * Supports Developer role, Multimodal inputs, and Text outputs.
 */

struct ImageUrl {
    std::string url;
    std::optional<std::string> detail; // "auto", "low", "high"
};

struct MessageContentPartText {
    std::string type = "text";
    std::string text;
};

struct MessageContentPartImage {
    std::string type = "image_url";
    ImageUrl image_url;
};

using MessageContentPart = std::variant<MessageContentPartText, MessageContentPartImage>;

struct ChatCompletionMessage {
    Role role = Role::USER; // developer, system, user, assistant
    std::variant<std::string, std::vector<MessageContentPart>> content;
    std::optional<std::string> name;
};

struct ChatCompletionRequest {
    std::string model;
    std::vector<ChatCompletionMessage> messages;

    std::optional<uint32_t> max_completion_tokens; // Replaces max_tokens for newer models
    std::optional<double> temperature;
    std::optional<double> top_p;
    
    std::vector<std::string> stop;
    
    std::optional<double> presence_penalty;
    std::optional<double> frequency_penalty;
    
    std::optional<uint64_t> seed;

    struct ResponseFormat {
        std::string type = "text"; // "text", "json_object", "json_schema"
        // json_schema structure omitted but supported in GPT-4o
    };
    std::optional<ResponseFormat> response_format;
};

struct UsageMetadata {
    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;

    struct PromptTokensDetails {
        uint32_t cached_tokens = 0;
    } prompt_tokens_details;
};

struct ChatCompletionResponse {
    std::string id;
    uint64_t created = 0;
    std::string model;
    std::optional<std::string> system_fingerprint;

    struct Choice {
        uint32_t index = 0;
        struct Message {
            Role role = Role::ASSISTANT;
            std::optional<std::string> content;
        } message;
        std::string finish_reason;
    };

    std::vector<Choice> choices;
    UsageMetadata usage;
};

} // namespace jai::llm::providers::openai_4o
