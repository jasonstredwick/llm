#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

namespace jai::llm::providers::gemini_2_5 {

/**
 * Gemini 2.5 API (2026 Snapshot)
 * Isolated "Shared-Nothing" Header
 */

constexpr std::string_view ENDPOINT_BASE = "https://generativelanguage.googleapis.com/v1beta/models/{MODEL_ID}:generateContent";
constexpr std::string_view ENDPOINT_VERTEX_BASE = "https://{REGION}-aiplatform.googleapis.com/v1/projects/{PROJECT_ID}/locations/{REGION}/publishers/google/models/{MODEL_ID}:generateContent";

// --- Type Glossary (Enums) ---
enum class ExecutionOutcome { DEADLINE_EXCEEDED, FAILED, OK, UNSPECIFIED };
enum class FinishReason { 
    BLOCKLIST, FINISH_REASON_UNSPECIFIED, MALFORMED_FUNCTION_CALL, MAX_TOKENS, 
    OTHER, PROHIBITED_CONTENT, RECITATION, SAFETY, SPII, STOP 
};
enum class FunctionCallingMode { ANY, AUTO, MODE_UNSPECIFIED, NONE };
enum class HarmCategory { 
    CIVIC_INTEGRITY, CIVIC_INTEGRITY_ELECTIONS, DANGEROUS_CONTENT, DEROGATORY, 
    HARASSMENT, HARASSMENT_CONTENT, HATE_CONTENT, HATE_SPEECH, MEDICAL, 
    MEDICAL_ADVICE, PUBLIC_SAFETY, SEXUAL, SEXUAL_CONTENT, SEXUALLY_EXPLICIT, 
    TOXICITY, UNSPECIFIED, VIOLENT_CONTENT 
};
enum class HarmProbability { HIGH, LOW, MEDIUM, NEGLIGIBLE, UNSPECIFIED };
enum class MediaResolution { HIGH, LOW, MEDIA_RESOLUTION_UNSPECIFIED, MEDIUM };
enum class Modality { AUDIO, IMAGE, TEXT, UNSPECIFIED, VIDEO };
enum class ResponseMimeType { APPLICATION_JSON, TEXT_PLAIN, TEXT_X_ENUM };
enum class Role { MODEL, SYSTEM, USER };
enum class SafetyThreshold { 
    BLOCK_LOW_AND_ABOVE, BLOCK_MEDIUM_AND_ABOVE, BLOCK_NONE, 
    BLOCK_ONLY_HIGH, OFF 
};

// --- Request Structures ---

struct Content {
    struct ContentPart {
        struct CodeExecutionResult {
            ExecutionOutcome outcome = ExecutionOutcome::UNSPECIFIED;
            std::string output;
        };
        struct ExecutableCode {
            std::string language;
            std::string code;
        };
        struct FileData {
            std::string mime_type;
            std::string file_uri;
        };
        struct FunctionCall {
            std::string name;
            std::string args; // JSON string
        };
        struct FunctionResponse {
            std::string name;
            std::string response; // JSON string
        };
        struct InlineImagePart {
            std::string mime_type;
            std::string base64_data;
        };
        struct TextPart {
            std::string text;
        };
        struct ThoughtSignature {
            std::string signature;
        };

        using Part = std::variant<
            TextPart, InlineImagePart, FileData, FunctionCall, 
            FunctionResponse, ExecutableCode, CodeExecutionResult, ThoughtSignature
        >;
    };

    Role role = Role::USER;
    std::vector<ContentPart::Part> parts;
};

struct GenerationConfig {
    struct ThinkingConfig {
        int32_t thinking_budget = -1;
        bool include_thoughts = false;
    };
    std::optional<uint32_t> max_output_tokens;
    std::optional<double> temperature;
    std::optional<uint32_t> top_k;
    std::optional<double> top_p;
    std::optional<double> presence_penalty;
    std::optional<double> frequency_penalty;
    std::optional<uint32_t> seed;
    std::optional<ThinkingConfig> thinking_config;
    std::optional<MediaResolution> media_resolution;
    std::optional<ResponseMimeType> response_mime_type;
    std::optional<std::string> response_schema; // OpenAPI schema as JSON string
    std::optional<bool> response_logprobs;
    std::optional<uint32_t> logprobs; // Number of top candidates
    std::vector<std::string> stop_sequences;
};

struct SafetySetting {
    HarmCategory category = HarmCategory::UNSPECIFIED;
    SafetyThreshold threshold = SafetyThreshold::BLOCK_MEDIUM_AND_ABOVE;
};

struct ToolConfig {
    struct FunctionCallingConfig {
        FunctionCallingMode mode = FunctionCallingMode::AUTO;
        std::vector<std::string> allowed_function_names;
    };
    std::optional<FunctionCallingConfig> function_calling_config;
};

    struct Tool {
        struct CodeExecution {};
        struct FunctionDeclaration {
            std::string name;
            std::optional<std::string> description;
            std::optional<std::string> parameters; 
        };
        struct GoogleSearch {
            struct DynamicRetrievalConfig {
                enum class Mode { MODE_UNSPECIFIED, DYNAMIC };
                Mode mode = Mode::MODE_UNSPECIFIED;
                std::optional<double> dynamic_threshold;
            };
            std::optional<DynamicRetrievalConfig> dynamic_retrieval_config;
        };
        std::vector<FunctionDeclaration> function_declarations;
        std::optional<GoogleSearch> google_search;
        std::optional<CodeExecution> code_execution;
    };

    struct GenerateContentRequest {
        std::string model;
        std::vector<Content> contents;
        std::optional<std::string> system_instruction;
        std::optional<GenerationConfig> generation_config;
        std::vector<SafetySetting> safety_settings;
        std::vector<Tool> tools;
        std::optional<ToolConfig> tool_config;
        std::optional<std::string> cached_content; // Resource name for explicit caching
    };

    // --- Response Structures ---

    struct ResponseTelemetry {
        uint32_t processing_ms = 0;
        std::string request_id;
        std::optional<std::string> organization;
        std::optional<std::string> version_header;
    };

    struct UsageMetadata {
        struct TokenCountDetails {
            uint32_t text_token_count = 0;
            uint32_t image_token_count = 0;
            uint32_t video_token_count = 0;
            uint32_t audio_token_count = 0;
        };
        uint32_t prompt_token_count = 0;
        uint32_t candidates_token_count = 0;
        uint32_t total_token_count = 0;
        uint32_t reasoning_token_count = 0;
        uint32_t cached_content_token_count = 0;
        std::vector<TokenCountDetails> prompt_token_count_details;
        std::vector<TokenCountDetails> candidates_token_count_details;
    };

struct SafetyRating {
    HarmCategory category = HarmCategory::UNSPECIFIED;
    HarmProbability probability = HarmProbability::UNSPECIFIED;
    bool blocked = false;
};

struct PromptFeedback {
    struct SafetyRatingDetail {
        HarmCategory category = HarmCategory::UNSPECIFIED;
        HarmProbability probability = HarmProbability::UNSPECIFIED;
    };
    std::vector<SafetyRatingDetail> safety_ratings;
    std::optional<std::string> block_reason;
};

struct CitationMetadata {
    struct CitationSource {
        uint32_t start_index = 0;
        uint32_t end_index = 0;
        std::string uri;
        std::string license;
    };
    std::vector<CitationSource> citation_sources;
};

struct GroundingMetadata {
    struct GroundingChunk {
        struct Web {
            std::string uri;
            std::string title;
        };
        Web web;
    };
    struct GroundingSupport {
        std::vector<uint32_t> grounding_chunk_indices;
        uint32_t segment_start_index = 0;
        uint32_t segment_end_index = 0;
        double confidence_score = 0.0;
    };
    struct SearchEntryPoint {
        std::string rendered_content; // HTML/CSS for Search UI
    };
    std::vector<GroundingChunk> grounding_chunks;
    std::vector<std::string> web_search_queries;
    std::vector<GroundingSupport> grounding_supports;
    std::optional<SearchEntryPoint> search_entry_point;
};

struct LogprobsResult {
    struct LogprobCandidate {
        std::string token;
        double log_probability = 0.0;
        std::vector<uint8_t> bytes;
    };
    std::vector<LogprobCandidate> chosen_candidates;
    std::vector<std::vector<LogprobCandidate>> top_candidates;
};

struct Candidate {
    Content content;
    FinishReason finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
    std::optional<std::string> thinking_process; 
    std::optional<CitationMetadata> citation_metadata;
    std::optional<GroundingMetadata> grounding_metadata;
    std::vector<SafetyRating> safety_ratings;
    std::optional<UsageMetadata> usage_metadata;
    std::optional<LogprobsResult> logprobs_result;
    double avg_logprobs = 0.0;
};

struct GenerateContentResponse {
    std::vector<Candidate> candidates;
    std::optional<PromptFeedback> prompt_feedback;
    std::optional<UsageMetadata> usage_metadata;
    std::optional<std::string> model_version;
    std::optional<ResponseTelemetry> telemetry;
};

} // namespace jai::llm::providers::gemini_2_5
