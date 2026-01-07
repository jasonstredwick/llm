#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <cstdint>

namespace jai::llm::providers {

enum class Role {
    USER,
    MODEL,
    SYSTEM,
    ASSISTANT,
    DEVELOPER,
    TOOL,
    FUNCTION
};

struct TextPart {
    std::string text;
};

struct InlineImagePart {
    std::string mime_type;
    std::string base64_data;
};

struct FileData {
    std::string mime_type;
    std::string file_uri;
};

struct FunctionCall {
    std::string name;
    std::string args; // JSON string for now
};

struct FunctionResponse {
    std::string name;
    std::string response; // JSON string for now
};

struct ExecutableCode {
    std::string language; // e.g. "python"
    std::string code;
};

struct CodeExecutionResult {
    std::string outcome; // e.g. "OUTCOME_OK"
    std::string output;
};

struct ThoughtSignature {
    std::string signature; // Encrypted reasoning state for multi-turn verification
};

using ContentPart = std::variant<
    TextPart, 
    InlineImagePart, 
    FileData, 
    FunctionCall, 
    FunctionResponse, 
    ExecutableCode, 
    CodeExecutionResult,
    ThoughtSignature
>;

struct Content {
    Role role = Role::USER;
    std::vector<ContentPart> parts;
};

enum class SafetyThreshold {
    BLOCK_NONE,
    BLOCK_ONLY_HIGH,
    BLOCK_MEDIUM_AND_ABOVE,
    BLOCK_LOW_AND_ABOVE,
    OFF
};

struct SafetyRating {
    std::string category;
    std::string probability;
    bool blocked = false;
};

enum class FinishReason {
    FINISH_REASON_UNSPECIFIED,
    STOP,
    MAX_TOKENS,
    SAFETY,
    RECITATION,
    OTHER,
    BLOCKLIST,
    PROHIBITED_CONTENT,
    SPII,
    MALFORMED_FUNCTION_CALL,
    TOOL_CALLS,
    REFUSAL,
    CONTENT_FILTER,
    LENGTH
};

struct ResponseTelemetry {
    uint32_t processing_ms = 0;
    std::string request_id;
    std::optional<std::string> organization;
    std::optional<std::string> version_header;
};

struct PromptFeedback {
    struct SafetyRating {
        std::string category;
        std::string probability;
    };
    std::vector<SafetyRating> safety_ratings;
    std::optional<std::string> block_reason;
};

} // namespace jai::llm::providers
