#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>

namespace jai::llm::providers::openai_5 {

using namespace jai::llm::providers;

/**
 * OpenAI GPT-5.2 Chat Completions API Structures
 * Includes Reasoning Effort, Verbosity, and Compaction.
 */

struct ImageUrl {
    std::string url;
    std::optional<std::string> detail;
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
    Role role = Role::USER;
    std::variant<std::string, std::vector<MessageContentPart>> content;
    std::optional<std::string> name;
};

struct ChatCompletionRequest {
    std::string model;
    std::vector<ChatCompletionMessage> messages;

    std::optional<uint32_t> max_completion_tokens;
    std::optional<double> temperature;
    std::optional<double> top_p;
    
    std::optional<uint64_t> seed;

    // GPT-5.2 Specific Reasoning Controls
    std::optional<std::string> reasoning_effort; // "low", "medium", "high", "xhigh"
    std::optional<std::string> verbosity;        // "concise", "medium", "detailed"
    std::optional<bool> compaction;             // Context compaction feature

    struct ResponseFormat {
        std::string type = "text"; // "text", "json_object", "json_schema"
    };
    std::optional<ResponseFormat> response_format;
};

struct UsageMetadata {
    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;

    struct CompletionTokensDetails {
        uint32_t reasoning_tokens = 0;
        uint32_t accepted_prediction_tokens = 0;
        uint32_t rejected_prediction_tokens = 0;
    } completion_tokens_details;
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
            
            // GPT-5.2 Reasoning Summaries
            std::optional<std::string> reasoning_summary;
        } message;
        std::string finish_reason;
    };

    std::vector<Choice> choices;
    UsageMetadata usage;
};

} // namespace jai::llm::providers::openai_5
