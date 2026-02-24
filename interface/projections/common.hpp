#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>


namespace jai::llm::projection {


enum class StopReason {
    COMPLETE,         // normal end of generation; response text is valid
    MAX_TOKENS,       // hit output token limit; response text is valid but truncated
    CONTENT_FILTERED, // provider refused or safety-blocked; response text may be empty
    UNEXPECTED        // stop reason that should not occur for this projection (e.g., tool_use)
};


struct Diagnostics {
    StopReason stop_reason{};
    std::optional<std::string> refusal{};    // provider-supplied refusal text, when available
    std::vector<std::string> warnings{};     // non-fatal observations (unexpected blocks, safety flags, etc.)
};


struct Usage {
    int64_t input_tokens{};
    int64_t output_tokens{};
    std::optional<int64_t> thinking_tokens{};
    std::optional<int64_t> cache_read_tokens{};
    std::optional<int64_t> cache_creation_tokens{};
};


}
