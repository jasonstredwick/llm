#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace jai::llm::providers::openai_4o {

/**
 * OpenAI GPT-4o ("Omni") - Chat Completions API
 * Isolated "Shared-Nothing" Header
 */

// --- Type Glossary (Enums) ---
enum class CacheRetention { HOURS_24, IN_MEMORY };
enum class ContentPartType { AUDIO, IMAGE_URL, TEXT, VIDEO };
enum class FinishReason { CONTENT_FILTER, FINISH_REASON_UNSPECIFIED, LENGTH, STOP, TOOL_CALLS };
enum class ImageDetail { AUTO, HIGH, LOW };
enum class Modality { AUDIO, IMAGE, TEXT, VIDEO };
enum class ModerationModel { 
    OMNI_MODERATION_2024_09_26, OMNI_MODERATION_LATEST, 
    TEXT_MODERATION_LATEST, TEXT_MODERATION_STABLE 
};
enum class ObjectType { CHAT_COMPLETION };
enum class PredictionType { CONTENT };
enum class ReasoningEffort { HIGH, LOW, MEDIUM, MINIMAL, NONE, XHIGH };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, DEVELOPER, SYSTEM, TOOL, USER };
enum class ServiceTier { DEFAULT, SCALE };
enum class ToolType { CODE_INTERPRETER, FILE_SEARCH, FUNCTION };
enum class Verbosity { CONCISE, DETAILED, MEDIUM };

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
        struct Audio {
            struct AudioData {
                std::string data; // Base64
                std::string format;
            };
            ContentPartType type = ContentPartType::AUDIO;
            AudioData audio;
        };
        struct Image {
            struct ImageUrl {
                std::string url;
                std::optional<ImageDetail> detail; // "auto", "low", "high"
            };
            ContentPartType type = ContentPartType::IMAGE_URL;
            ImageUrl image_url;
        };
        struct Text {
            ContentPartType type = ContentPartType::TEXT;
            std::string text;
        };
        struct Video {
            struct VideoUrl {
                std::string url;
                std::optional<ImageDetail> detail;
            };
            ContentPartType type = ContentPartType::VIDEO;
            VideoUrl video_url;
        };

        using Part = std::variant<Text, Image, Audio, Video>;
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
    struct Metadata {
        std::string key;
        std::string value;
    };
    struct Prediction {
        PredictionType type = PredictionType::CONTENT;
        std::variant<std::string, std::vector<ChatCompletionMessage::ContentPart::Part>> content;
    };
    struct ResponseFormat {
        struct JsonSchema {
            std::string name;
            std::optional<std::string> description;
            std::string schema; // JSON Schema string
            std::optional<bool> strict;
        };
        ResponseFormatType type = ResponseFormatType::TEXT; // "text", "json_object", "json_schema"
        std::optional<JsonSchema> json_schema;
    };
    struct AudioConfig {
        std::string voice;
        std::string format;
    };
    struct Tool {
        struct CodeInterpreter {};
        struct FileSearch {
            uint32_t max_num_results = 20;
        };
        struct Function {
            std::string name;
            std::optional<std::string> description;
            std::string parameters; // JSON Schema string
            std::optional<bool> strict;
        };
        ToolType type = ToolType::FUNCTION;
        Function function;
        std::optional<CodeInterpreter> code_interpreter;
        std::optional<FileSearch> file_search;
    };
    struct ToolChoiceSpecific {
        struct Function {
            std::string name;
        };
        ToolType type = ToolType::FUNCTION;
        Function function;
    };

    std::string model;
    std::vector<ChatCompletionMessage> messages;
    std::optional<uint32_t> max_completion_tokens; // Replaces max_tokens for newer models
    std::optional<double> temperature;
    std::optional<double> top_p;
    
    std::optional<ReasoningEffort> reasoning_effort;
    std::optional<Verbosity> verbosity;
    std::optional<bool> compaction;

    // Prompt Caching Controls
    std::optional<std::string> prompt_cache_key;
    std::optional<CacheRetention> prompt_cache_retention; // "in_memory", "24h"

    std::vector<std::string> stop;
    std::optional<double> presence_penalty;
    std::optional<double> frequency_penalty;
    std::optional<std::string> user;
    std::optional<uint64_t> seed;
    std::optional<ResponseFormat> response_format;
    std::vector<Tool> tools;
    std::variant<std::monostate, std::string, ToolChoiceSpecific> tool_choice;
    std::optional<bool> parallel_tool_calls;
    std::optional<Prediction> prediction;
    std::optional<AudioConfig> audio;
    std::vector<Modality> modalities;
    std::optional<bool> store;
    std::vector<Metadata> metadata;
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
        uint32_t audio_tokens = 0;
        uint32_t image_tokens = 0;
        uint32_t video_tokens = 0;
    };
    struct CompletionTokensDetails {
        uint32_t reasoning_tokens = 0;
        uint32_t audio_tokens = 0;
        uint32_t accepted_prediction_tokens = 0;
        uint32_t rejected_prediction_tokens = 0;
    };

    uint32_t prompt_tokens = 0;
    uint32_t completion_tokens = 0;
    uint32_t total_tokens = 0;
    PromptTokensDetails prompt_tokens_details;
    CompletionTokensDetails completion_tokens_details;
};

struct ChatCompletionResponse {
    struct Choice {
        struct Logprobs {
            struct Content {
                struct TopLogprob {
                    std::string token;
                    double logprob = 0.0;
                    std::vector<uint8_t> bytes;
                };
                std::string token;
                double logprob = 0.0;
                std::vector<uint8_t> bytes;
                std::vector<TopLogprob> top_logprobs;
            };
            std::vector<Content> content;
            std::vector<Content> refusal;
        };
        struct Message {
            struct Annotation {
                struct FileCitation {
                    std::string file_id;
                    std::string quote;
                };
                struct UrlCitation {
                    std::string url;
                    std::string title;
                };
                uint32_t start_index = 0;
                uint32_t end_index = 0;
                std::string text;
                std::optional<FileCitation> file_citation;
                std::optional<UrlCitation> url_citation;
            };
            struct Audio {
                std::string id;
                uint64_t expires_at = 0;
                std::string data; // Base64
                std::string transcript;
            };
            Role role = Role::ASSISTANT;
            std::optional<std::string> content;
            std::optional<std::string> refusal;           // Explicit refusal message
            std::optional<std::string> reasoning_content; // Internal reasoning chain
            std::vector<Annotation> annotations;
            std::optional<Audio> audio;
            std::vector<ToolCall> tool_calls;
        };

        uint32_t index = 0;
        FinishReason finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
        Message message;
        std::optional<Logprobs> logprobs;
    };

    std::string id;
    uint64_t created = 0;
    std::string model;
    std::optional<std::string> system_fingerprint;
    std::vector<Choice> choices;
    UsageMetadata usage;
    std::optional<ServiceTier> service_tier; // "scale", "default"
    std::optional<ResponseTelemetry> telemetry;
};

// --- Moderation Structures ---

struct ModerationRequest {
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

    std::variant<std::string, std::vector<ContentPart::Part>> input;
    ModerationModel model = ModerationModel::OMNI_MODERATION_LATEST;
};

struct ModerationResponse {
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
        };
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
        };
        Categories categories;
        CategoryScores category_scores;
        bool flagged = false;
    };

    std::string id;
    std::string model;
    std::vector<Result> results;
};

} // namespace jai::llm::providers::openai_4o
