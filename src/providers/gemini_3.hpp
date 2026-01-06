#pragma once

#include "base.hpp"
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace jai::llm::providers::gemini {

using namespace jai::llm::providers;

enum class ThinkingLevel {
    THINKING_LEVEL_UNSPECIFIED,
    MINIMAL,
    LOW,
    MEDIUM,
    HIGH
};

enum class MediaResolution {
    MEDIA_RESOLUTION_UNSPECIFIED,
    LOW,
    MEDIUM,
    HIGH
};

struct ThinkingConfig {
    ThinkingLevel thinking_level = ThinkingLevel::MINIMAL;
    std::optional<uint32_t> include_thoughts; // Boolean would be simpler but API sometimes uses flags
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
    // Placeholder for actual tool definitions (e.g. Google Search, Function declarations)
    struct GoogleSearchRetrieval {};
    std::optional<GoogleSearchRetrieval> google_search_retrieval;
};

struct ToolConfig {
    // Placeholder for function calling config
};

struct GenerateContentRequest {
    std::optional<std::string> system_instruction;
    std::vector<Content> contents;

    std::optional<GenerationConfig> generation_config;
    std::vector<SafetySetting> safety_settings;
    std::vector<Tool> tools;
    std::optional<ToolConfig> tool_config;
};

struct UsageMetadata {
    uint32_t prompt_token_count = 0;
    uint32_t candidates_token_count = 0;
    uint32_t total_token_count = 0;
    uint32_t reasoning_token_count = 0;
};

struct Candidate {
    Content content;
    FinishReason finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
    
    // Gemini 3.0 Reasoning
    std::optional<std::string> thought; // Raw reasoning chain
    
    // Usage per candidate if available
    std::optional<UsageMetadata> usage_metadata;
};

struct GenerateContentResponse {
    std::vector<Candidate> candidates;
    std::optional<UsageMetadata> usage_metadata;
    std::optional<std::string> model_version;
};

} // namespace jai::llm::providers::gemini
