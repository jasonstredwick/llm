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
    
    /**
     * OpenAI Constraint: 'developer' and 'system' roles MUST be text-only (std::string).
     */
    std::string content;
    std::optional<std::string> name;

    struct ToolCall {
        std::string id;
        std::string type = "function";
        struct Function {
            std::string name;
            std::string arguments;
        } function;
    };
    std::vector<ToolCall> tool_calls;
    std::optional<std::string> tool_call_id;
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

    struct Tool {
        std::string type = "function";
        struct Function {
            std::string name;
            std::optional<std::string> description;
            std::string parameters; // JSON Schema string
        } function;
    };
    std::vector<Tool> tools;
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
    std::string object = "chat.completion";
    uint64_t created = 0;
    std::string model;
    std::optional<std::string> system_fingerprint;

    struct Choice {
        uint32_t index = 0;
        
        struct Message {
            Role role = Role::ASSISTANT;
            std::optional<std::string> content;

            struct ToolCall {
                std::string id;
                std::string type = "function";
                struct Function {
                    std::string name;
                    std::string arguments;
                } function;
            };
            std::vector<ToolCall> tool_calls;
        } message;

        std::string finish_reason; // "stop", "length", "content_filter"
    };

    std::vector<Choice> choices;
    UsageMetadata usage;
};

} // namespace jai::llm::providers::openai_4
