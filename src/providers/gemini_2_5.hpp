#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace jai::llm::providers::gemini_2_5 {

using namespace jai::llm::providers;

enum class MediaResolution {
    MEDIA_RESOLUTION_UNSPECIFIED,
    LOW,
    MEDIUM,
    HIGH
};

struct ThinkingConfig {
    /**
     * Guides the model on the number of "thinking tokens" to use.
     * 0: Disables thinking.
     * -1: Dynamic thinking (model decides).
     * Positive integer: Upper limit on thinking tokens.
     */
    int32_t thinking_budget = -1;
    bool include_thoughts = false;
};

struct GenerationConfig {
    std::optional<uint32_t> max_output_tokens;
    std::optional<double> temperature;
    std::optional<uint32_t> top_k;
    std::optional<double> top_p;
    std::optional<double> presence_penalty;
    std::optional<double> frequency_penalty;
    std::optional<uint32_t> seed;

    std::optional<ThinkingConfig> thinking_config;
    std::optional<MediaResolution> media_resolution;

    std::optional<std::string> response_mime_type;
    std::optional<std::string> response_schema; // OpenAPI schema as JSON string

    std::vector<std::string> stop_sequences;
};

struct SafetySetting {
    std::string category;
    std::string threshold;
};

struct Tool {
    struct FunctionDeclaration {
        std::string name;
        std::string description;
        std::string parameters; // JSON Schema string
    };
    std::vector<FunctionDeclaration> function_declarations;
    
    struct GoogleSearch {};
    std::optional<GoogleSearch> google_search;

    struct CodeExecution {};
    std::optional<CodeExecution> code_execution;
};

struct ToolConfig {
    struct FunctionCallingConfig {
        enum class Mode {
            MODE_UNSPECIFIED,
            AUTO,
            ANY,
            NONE
        } mode = Mode::AUTO;
        std::vector<std::string> allowed_function_names;
    };
    std::optional<FunctionCallingConfig> function_calling_config;
};

struct GenerateContentRequest {
    std::optional<std::string> system_instruction;
    std::vector<Content> contents;

    std::optional<GenerationConfig> generation_config;
    std::vector<SafetySetting> safety_settings;
    std::vector<Tool> tools;
    std::optional<ToolConfig> tool_config;
    std::optional<std::string> cached_content; // Resource name for explicit caching
};

struct UsageMetadata {
    uint32_t prompt_token_count = 0;
    uint32_t candidates_token_count = 0;
    uint32_t total_token_count = 0;
    
    uint32_t reasoning_token_count = 0;
    uint32_t cached_content_token_count = 0;

    struct TokenCountDetails {
        uint32_t text_token_count = 0;
        uint32_t image_token_count = 0;
        uint32_t video_token_count = 0;
        uint32_t audio_token_count = 0;
    };
    TokenCountDetails prompt_token_count_details;
    TokenCountDetails candidates_token_count_details;
};

struct Candidate {
    Content content;
    FinishReason finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
    
    // Reasoning chain text if available
    std::optional<std::string> thinking_process; 
    
    std::optional<UsageMetadata> usage_metadata;
};

struct GenerateContentResponse {
    std::vector<Candidate> candidates;
    std::optional<UsageMetadata> usage_metadata;
    std::optional<std::string> model_version;
};

} // namespace jai::llm::providers::gemini_2_5
