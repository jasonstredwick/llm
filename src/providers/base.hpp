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
    DEVELOPER
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

using ContentPart = std::variant<
    TextPart, 
    InlineImagePart, 
    FileData, 
    FunctionCall, 
    FunctionResponse, 
    ExecutableCode, 
    CodeExecutionResult
>;

struct Content {
    Role role = Role::USER;
    std::vector<ContentPart> parts;
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
    MALFORMED_FUNCTION_CALL
};

} // namespace jai::llm::providers
