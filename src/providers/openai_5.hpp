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
    
    /**
     * OpenAI Constraint: 'developer' and 'system' roles MUST be text-only (std::string).
     * 'user' and 'assistant' roles can be multimodal (vector of parts).
     */
    std::variant<std::string, std::vector<MessageContentPart>> content;
    std::optional<std::string> name;

    struct ToolCall {
        std::string id;
        std::string type = "function";
        struct Function {
            std::string name;
            std::string arguments; // JSON string
        } function;
    };
    std::vector<ToolCall> tool_calls;
    std::optional<std::string> tool_call_id; // For 'tool' role messages
};

struct ToolChoiceSpecific {
    std::string type = "function";
    struct Function {
        std::string name;
    } function;
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
    std::optional<bool> compaction;              // Context compaction feature

    // Prompt Caching Controls (GPT-5.2)
    std::optional<std::string> prompt_cache_key; 
    std::optional<std::string> prompt_cache_retention; // "in_memory", "24h"

    struct ResponseFormat {
        std::string type = "text"; // "text", "json_object", "json_schema"
    };
    std::optional<ResponseFormat> response_format;

    struct Tool {
        std::string type = "function"; // "function", "code_interpreter", "file_search"
        struct Function {
            std::string name;
            std::optional<std::string> description;
            std::string parameters; // JSON Schema as string
            std::optional<bool> strict;
        } function;
        
        // Built-in tools for GPT-5.2
        struct CodeInterpreter {};
        std::optional<CodeInterpreter> code_interpreter;
        
        struct FileSearch {
            uint32_t max_num_results = 20;
        };
        std::optional<FileSearch> file_search;
    };
    std::vector<Tool> tools;
    std::optional<bool> parallel_tool_calls;

    // Speculative Decoding (2026)
    struct Prediction {
        std::string type = "content";
        std::variant<std::string, std::vector<MessageContentPart>> content;
    };
    std::optional<Prediction> prediction;

    // Response Tracking (2026)
    std::optional<bool> store;
    struct Metadata {
        std::string key;
        std::string value;
    };
    std::vector<Metadata> metadata; // Key-value pairs
};

struct UsageMetadata {
    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;

    struct PromptTokensDetails {
        uint32_t cached_tokens = 0;
        uint32_t audio_tokens = 0;
        uint32_t image_tokens = 0;
        uint32_t video_tokens = 0;
    } prompt_tokens_details;

    struct CompletionTokensDetails {
        uint32_t reasoning_tokens = 0;
        uint32_t audio_tokens = 0;
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
            
            struct Annotation {
                uint32_t start_index = 0;
                uint32_t end_index = 0;
                std::string text;
                struct FileCitation {
                    std::string file_id;
                    std::string quote;
                };
                std::optional<FileCitation> file_citation;
                struct UrlCitation {
                    std::string url;
                    std::string title;
                };
                std::optional<UrlCitation> url_citation;
            };
            std::vector<Annotation> annotations;

            struct Audio {
                std::string id;
                uint64_t expires_at = 0;
                std::string data; // Base64
                std::string transcript;
            };
            std::optional<Audio> audio;
            
            // GPT-5.2 Reasoning Details
            std::optional<std::string> reasoning_summary;
            std::optional<std::string> reasoning_content; 

            // Caching/Safety Refusals
            std::optional<std::string> refusal;

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

        struct Logprobs {
            struct Content {
                std::string token;
                double logprob = 0.0;
                std::vector<uint8_t> bytes;
                struct TopLogprob {
                    std::string token;
                    double logprob = 0.0;
                    std::vector<uint8_t> bytes;
                };
                std::vector<TopLogprob> top_logprobs;
            };
            std::vector<Content> content;
            std::vector<Content> refusal;
        };
        std::optional<Logprobs> logprobs;

        std::string finish_reason;
    };

    std::vector<Choice> choices;
    UsageMetadata usage;
    std::optional<std::string> service_tier; // "scale", "default"
    std::optional<ResponseTelemetry> telemetry;
};

struct ModerationContentPartText {
    std::string type = "text";
    std::string text;
};

struct ModerationContentPartImage {
    std::string type = "image_url";
    ImageUrl image_url;
};

using ModerationContentPart = std::variant<ModerationContentPartText, ModerationContentPartImage>;

struct ModerationRequest {
    /**
     * Input can be a single string or a list of content parts (text/image).
     * Supported for omni-moderation-latest.
     */
    std::variant<std::string, std::vector<ModerationContentPart>> input;
    std::string model = "omni-moderation-latest";
};

struct ModerationResponse {
    std::string id;
    std::string model;
    struct Result {
        struct Categories {
            bool hate = false;
            bool hate_threatening = false;
            bool harassment = false;
            bool harassment_threatening = false;
            bool self_harm = false;
            bool self_harm_instructions = false;
            bool self_harm_intent = false;
            bool sexual = false;
            bool sexual_minors = false;
            bool violence = false;
            bool violence_graphic = false;
            bool illicit = false;
            bool illicit_violent = false;
        } categories;

        struct CategoryScores {
            double hate = 0.0;
            double hate_threatening = 0.0;
            double harassment = 0.0;
            double harassment_threatening = 0.0;
            double self_harm = 0.0;
            double self_harm_instructions = 0.0;
            double self_harm_intent = 0.0;
            double sexual = 0.0;
            double sexual_minors = 0.0;
            double violence = 0.0;
            double violence_graphic = 0.0;
            double illicit = 0.0;
            double illicit_violent = 0.0;
        } category_scores;

        bool flagged = false;
    };
    std::vector<Result> results;
};

} // namespace jai::llm::providers::openai_5
