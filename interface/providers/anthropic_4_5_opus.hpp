#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"
#include "../url.hpp"


namespace jai::llm::anthropic_4_5_opus {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view/from_string_view conversions defined below.
 */
enum class CacheType { EPHEMERAL };
enum class CitationType { CHAR_LOCATION, PAGE_LOCATION };
enum class ContentBlockType { TEXT, THINKING, TOOL_USE };
enum class EffortLevel { HIGH, LOW, MEDIUM };
enum class MessageContentPartType { AUDIO, DOCUMENT, IMAGE, TEXT, TOOL_RESULT, TOOL_USE };
enum class MessageType { ERROR, MESSAGE };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, USER };
enum class SourceType { BASE64, URL };
enum class StopReason { END_TURN, MAX_TOKENS, STOP_SEQUENCE, TOOL_USE };
enum class ThinkingType { ENABLED };
enum class ToolChoiceType { ANY, AUTO, TOOL };


/***
 * Request
 */
struct CacheControl {
    CacheType type{CacheType::EPHEMERAL};
};


struct Audio {
    struct Source {
        SourceType type{SourceType::BASE64};
        std::string media_type;
        std::string data;
    };

    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Document {
    struct Source {
        SourceType type{SourceType::BASE64};
        std::string media_type;
        std::string data;
    };

    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Image {
    struct Source {
        struct Base64 {
            std::string media_type;
            std::string data;
        };

        struct URL {
            EncodedUrl url;
        };

        std::variant<Base64, URL> detail;
    };

    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Text {
    std::string text;
    std::optional<CacheControl> cache_control{};
};


struct ToolResult {
    std::string tool_use_id;
    std::string content;
    std::optional<bool> is_error{};
    std::optional<CacheControl> cache_control{};
};


struct ToolUse {
    std::string id;
    std::string name;
    std::string input; // JSON string
    std::optional<CacheControl> cache_control{};
};


struct Message {
    using Part = std::variant<Text, Image, Audio, Document, ToolUse, ToolResult>;

    Role role{Role::USER};
    std::variant<std::string, std::vector<Part>> content;
};


struct ResponseFormat {
    struct JsonObject {};
    struct JsonSchema { std::string schema; };
    struct Text {};

    std::variant<Text, JsonObject, JsonSchema> detail{Text{}};
};


struct SystemPrompt {
    std::string text;
    std::optional<CacheControl> cache_control{};
};


struct ThinkingConfig {
    ThinkingType type{ThinkingType::ENABLED};
    uint64_t budget_tokens{0};
};


struct Tool {
    struct Bash {
        std::string type{"bash_20241022"};
        std::string name{"bash"};
    };

    struct Computer {
        std::string type{"computer_20241022"};
        std::string name{"computer"};
        uint32_t display_width_px{0};
        uint32_t display_height_px{0};
        std::optional<uint32_t> display_number{};
    };

    struct Custom {
        std::string name;
        std::string description;
        std::string input_schema; // JSON Schema string
    };

    struct TextEditor {
        std::string type{"text_editor_20250124"};
        std::string name{"str_replace_editor"};
    };

    using Config = std::variant<Custom, Computer, Bash, TextEditor>;

    Config config;
    std::optional<CacheControl> cache_control{};
};


struct ToolChoice {
    ToolChoiceType type{ToolChoiceType::AUTO};
    std::optional<std::string> name{};
    std::optional<bool> disable_parallel_tool_use{};
};


struct Request {
    std::string model;
    std::vector<Message> messages;
    std::variant<std::string, std::vector<SystemPrompt>> system;
    uint64_t max_tokens{4096};
    std::optional<ThinkingConfig> thinking{};
    std::optional<EffortLevel> effort{};
    std::vector<std::string> stop_sequences{};
    std::optional<ResponseFormat> response_format{};
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<uint64_t> top_k{};
    std::vector<Tool> tools{};
    std::optional<ToolChoice> tool_choice{};
};


/***
 * Response
 */
struct Citation {
    CitationType type{CitationType::CHAR_LOCATION};
    std::optional<uint64_t> document_index{};
    std::optional<std::string> document_title{};
    std::string cite{};
    uint64_t start_index{0};
    uint64_t end_index{0};
    std::optional<uint64_t> start_page_number{};
    std::optional<uint64_t> end_page_number{};
};


struct ContentBlock {
    struct Text { std::string text{}; };
    struct Thinking { std::string thinking{}; std::string signature{}; };
    struct ToolUse { std::string id{}; std::string name{}; std::string input{}; };

    std::variant<Text, Thinking, ToolUse> detail;
    std::vector<Citation> citations{};
};


struct Stop {
    std::optional<StopReason> reason{};
    std::optional<std::string> sequence{};
};


struct Telemetry {
    uint64_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};


struct UsageMetadata {
    uint64_t input_tokens{0};
    uint64_t output_tokens{0};
    uint64_t thinking_tokens{0};
    uint64_t cache_read_input_tokens{0};
    uint64_t cache_creation_input_tokens{0};
};


struct Response {
    std::string id;
    Role role{Role::ASSISTANT};
    std::string model;
    std::vector<ContentBlock> content{};
    std::optional<Stop> stop{};
    std::optional<Telemetry> telemetry{};
    UsageMetadata usage{};
};


/***
 * Client
 */
class Client {
private:
    ClientPolicy policy;

public:
    Client(const ClientPolicy& client_policy) : policy{client_policy} {}
    Client(ClientPolicy&& client_policy) : policy{std::move(client_policy)} {}
    Client(const Client&) = default;
    Client(Client&&) noexcept = default;
    ~Client() noexcept = default;
    Client& operator=(const Client&) = default;
    Client& operator=(Client&&) noexcept = default;

    AsyncTask<Response> GenerateTextAsync(const Request& r) const;
    Response GenerateTextSync(const Request& r) const;
};


}


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


template <>
constexpr std::optional<anthropic_4_5_opus::CacheType> from_string_view<anthropic_4_5_opus::CacheType>(std::string_view sv) {
    if (sv == "ephemeral") return anthropic_4_5_opus::CacheType::EPHEMERAL;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::CitationType> from_string_view<anthropic_4_5_opus::CitationType>(std::string_view sv) {
    if (sv == "char_location") return anthropic_4_5_opus::CitationType::CHAR_LOCATION;
    if (sv == "page_location") return anthropic_4_5_opus::CitationType::PAGE_LOCATION;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::ContentBlockType> from_string_view<anthropic_4_5_opus::ContentBlockType>(std::string_view sv) {
    if (sv == "text") return anthropic_4_5_opus::ContentBlockType::TEXT;
    if (sv == "thinking") return anthropic_4_5_opus::ContentBlockType::THINKING;
    if (sv == "tool_use") return anthropic_4_5_opus::ContentBlockType::TOOL_USE;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::EffortLevel> from_string_view<anthropic_4_5_opus::EffortLevel>(std::string_view sv) {
    if (sv == "low") return anthropic_4_5_opus::EffortLevel::LOW;
    if (sv == "medium") return anthropic_4_5_opus::EffortLevel::MEDIUM;
    if (sv == "high") return anthropic_4_5_opus::EffortLevel::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::MessageContentPartType> from_string_view<anthropic_4_5_opus::MessageContentPartType>(std::string_view sv) {
    if (sv == "text") return anthropic_4_5_opus::MessageContentPartType::TEXT;
    if (sv == "image") return anthropic_4_5_opus::MessageContentPartType::IMAGE;
    if (sv == "audio") return anthropic_4_5_opus::MessageContentPartType::AUDIO;
    if (sv == "document") return anthropic_4_5_opus::MessageContentPartType::DOCUMENT;
    if (sv == "tool_use") return anthropic_4_5_opus::MessageContentPartType::TOOL_USE;
    if (sv == "tool_result") return anthropic_4_5_opus::MessageContentPartType::TOOL_RESULT;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::MessageType> from_string_view<anthropic_4_5_opus::MessageType>(std::string_view sv) {
    if (sv == "message") return anthropic_4_5_opus::MessageType::MESSAGE;
    if (sv == "error") return anthropic_4_5_opus::MessageType::ERROR;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::Role> from_string_view<anthropic_4_5_opus::Role>(std::string_view sv) {
    if (sv == "user") return anthropic_4_5_opus::Role::USER;
    if (sv == "assistant") return anthropic_4_5_opus::Role::ASSISTANT;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::ResponseFormatType> from_string_view<anthropic_4_5_opus::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return anthropic_4_5_opus::ResponseFormatType::TEXT;
    if (sv == "json_object") return anthropic_4_5_opus::ResponseFormatType::JSON_OBJECT;
    if (sv == "json_schema") return anthropic_4_5_opus::ResponseFormatType::JSON_SCHEMA;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::SourceType> from_string_view<anthropic_4_5_opus::SourceType>(std::string_view sv) {
    if (sv == "base64") return anthropic_4_5_opus::SourceType::BASE64;
    if (sv == "url") return anthropic_4_5_opus::SourceType::URL;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::StopReason> from_string_view<anthropic_4_5_opus::StopReason>(std::string_view sv) {
    if (sv == "end_turn") return anthropic_4_5_opus::StopReason::END_TURN;
    if (sv == "max_tokens") return anthropic_4_5_opus::StopReason::MAX_TOKENS;
    if (sv == "stop_sequence") return anthropic_4_5_opus::StopReason::STOP_SEQUENCE;
    if (sv == "tool_use") return anthropic_4_5_opus::StopReason::TOOL_USE;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::ThinkingType> from_string_view<anthropic_4_5_opus::ThinkingType>(std::string_view sv) {
    if (sv == "enabled") return anthropic_4_5_opus::ThinkingType::ENABLED;
    return std::nullopt;
}


template <>
constexpr std::optional<anthropic_4_5_opus::ToolChoiceType> from_string_view<anthropic_4_5_opus::ToolChoiceType>(std::string_view sv) {
    if (sv == "auto") return anthropic_4_5_opus::ToolChoiceType::AUTO;
    if (sv == "any") return anthropic_4_5_opus::ToolChoiceType::ANY;
    if (sv == "tool") return anthropic_4_5_opus::ToolChoiceType::TOOL;
    return std::nullopt;
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::CacheType val) {
    switch (val) {
        case anthropic_4_5_opus::CacheType::EPHEMERAL: return "ephemeral";
        default: throw std::logic_error("invalid anthropic_4_5_opus::CacheType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::CitationType val) {
    switch (val) {
        case anthropic_4_5_opus::CitationType::CHAR_LOCATION: return "char_location";
        case anthropic_4_5_opus::CitationType::PAGE_LOCATION: return "page_location";
        default: throw std::logic_error("invalid anthropic_4_5_opus::CitationType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ContentBlockType val) {
    switch (val) {
        case anthropic_4_5_opus::ContentBlockType::TEXT: return "text";
        case anthropic_4_5_opus::ContentBlockType::THINKING: return "thinking";
        case anthropic_4_5_opus::ContentBlockType::TOOL_USE: return "tool_use";
        default: throw std::logic_error("invalid anthropic_4_5_opus::ContentBlockType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::EffortLevel val) {
    switch (val) {
        case anthropic_4_5_opus::EffortLevel::LOW: return "low";
        case anthropic_4_5_opus::EffortLevel::MEDIUM: return "medium";
        case anthropic_4_5_opus::EffortLevel::HIGH: return "high";
        default: throw std::logic_error("invalid anthropic_4_5_opus::EffortLevel");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::MessageContentPartType val) {
    switch (val) {
        case anthropic_4_5_opus::MessageContentPartType::TEXT: return "text";
        case anthropic_4_5_opus::MessageContentPartType::IMAGE: return "image";
        case anthropic_4_5_opus::MessageContentPartType::AUDIO: return "audio";
        case anthropic_4_5_opus::MessageContentPartType::DOCUMENT: return "document";
        case anthropic_4_5_opus::MessageContentPartType::TOOL_USE: return "tool_use";
        case anthropic_4_5_opus::MessageContentPartType::TOOL_RESULT: return "tool_result";
        default: throw std::logic_error("invalid anthropic_4_5_opus::MessageContentPartType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::MessageType val) {
    switch (val) {
        case anthropic_4_5_opus::MessageType::MESSAGE: return "message";
        case anthropic_4_5_opus::MessageType::ERROR: return "error";
        default: throw std::logic_error("invalid anthropic_4_5_opus::MessageType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::Role val) {
    switch (val) {
        case anthropic_4_5_opus::Role::USER: return "user";
        case anthropic_4_5_opus::Role::ASSISTANT: return "assistant";
        default: throw std::logic_error("invalid anthropic_4_5_opus::Role");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ResponseFormatType val) {
    switch (val) {
        case anthropic_4_5_opus::ResponseFormatType::TEXT: return "text";
        case anthropic_4_5_opus::ResponseFormatType::JSON_OBJECT: return "json_object";
        case anthropic_4_5_opus::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid anthropic_4_5_opus::ResponseFormatType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::SourceType val) {
    switch (val) {
        case anthropic_4_5_opus::SourceType::BASE64: return "base64";
        case anthropic_4_5_opus::SourceType::URL: return "url";
        default: throw std::logic_error("invalid anthropic_4_5_opus::SourceType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::StopReason val) {
    switch (val) {
        case anthropic_4_5_opus::StopReason::END_TURN: return "end_turn";
        case anthropic_4_5_opus::StopReason::MAX_TOKENS: return "max_tokens";
        case anthropic_4_5_opus::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case anthropic_4_5_opus::StopReason::TOOL_USE: return "tool_use";
        default: throw std::logic_error("invalid anthropic_4_5_opus::StopReason");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ThinkingType val) {
    switch (val) {
        case anthropic_4_5_opus::ThinkingType::ENABLED: return "enabled";
        default: throw std::logic_error("invalid anthropic_4_5_opus::ThinkingType");
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ToolChoiceType val) {
    switch (val) {
        case anthropic_4_5_opus::ToolChoiceType::AUTO: return "auto";
        case anthropic_4_5_opus::ToolChoiceType::ANY: return "any";
        case anthropic_4_5_opus::ToolChoiceType::TOOL: return "tool";
        default: throw std::logic_error("invalid anthropic_4_5_opus::ToolChoiceType");
    }
}


}
