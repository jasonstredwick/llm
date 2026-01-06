#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace jai::llm::providers::openai_4 {

using namespace jai::llm::providers;

/**
 * OpenAI GPT-4 Chat Completions API Structures
 * Focused on text-only/simple multimodal inputs and text outputs.
 */

struct ChatCompletionMessage {
    Role role = Role::USER; // system, user, assistant
    std::string content;
    std::optional<std::string> name;
};

struct ChatCompletionRequest {
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

    struct ResponseFormat {
        std::string type = "text"; // "text", "json_object"
    };
    std::optional<ResponseFormat> response_format;
};

struct UsageMetadata {
    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;
};

struct ChatCompletionResponse {
    std::string id;
    std::string object = "chat.completion";
    uint64_t created = 0;
    std::string model;
    std::optional<std::string> system_fingerprint;

    struct Choice {
        uint32_t index = 0;
        
        struct Message {
            Role role = Role::ASSISTANT;
            std::optional<std::string> content;
        } message;

        std::string finish_reason; // "stop", "length", "content_filter"
    };

    std::vector<Choice> choices;
    UsageMetadata usage;
};

} // namespace jai::llm::providers::openai_4
