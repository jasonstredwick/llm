#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"


namespace jai::llm::anthropic_4_5_opus {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view conversions defined below.
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

    MessageContentPartType type{MessageContentPartType::AUDIO};
    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Document {
    struct Source {
        SourceType type{SourceType::BASE64};
        std::string media_type;
        std::string data;
    };

    MessageContentPartType type{MessageContentPartType::DOCUMENT};
    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Image {
    struct Source {
        SourceType type{SourceType::BASE64};
        /**
         * For BASE64: media_type and data are required.
         * For URL: url is required.
         */

        std::optional<std::string> media_type{};
        std::optional<std::string> data{};
        std::optional<std::string> url{};
    };

    MessageContentPartType type{MessageContentPartType::IMAGE};
    Source source;
    std::optional<CacheControl> cache_control{};
};


struct Text {
    MessageContentPartType type{MessageContentPartType::TEXT};
    std::string text;
    std::optional<CacheControl> cache_control{};
};


struct ToolResult {
    MessageContentPartType type{MessageContentPartType::TOOL_RESULT};
    std::string tool_use_id;
    std::string content;
    std::optional<bool> is_error{};
    std::optional<CacheControl> cache_control{};
};


struct ToolUse {
    MessageContentPartType type{MessageContentPartType::TOOL_USE};
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
    ResponseFormatType type{ResponseFormatType::TEXT};
    std::optional<std::string> json_schema{};
};


struct SystemPrompt {
    std::string text;
    std::optional<CacheControl> cache_control{};
};


struct ThinkingConfig {
    ThinkingType type{ThinkingType::ENABLED};
    uint32_t budget_tokens{0};
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
    uint32_t max_tokens{4096};
    std::optional<ThinkingConfig> thinking{};
    std::optional<EffortLevel> effort{};
    std::vector<std::string> stop_sequences{};
    std::optional<ResponseFormat> response_format{};
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<uint32_t> top_k{};
    std::vector<Tool> tools{};
    std::optional<ToolChoice> tool_choice{};
};


/***
 * Response
 */
struct Citation {
    struct Source {
        SourceType type{SourceType::BASE64};
        std::optional<std::string> media_type{};
        std::optional<std::string> data{};
        std::optional<std::string> file_id{};
    };

    CitationType type{CitationType::CHAR_LOCATION};
    Source source{};
    std::string cite{};
    uint32_t start_index{0};
    uint32_t end_index{0};
    std::optional<uint32_t> start_page_number{};
    std::optional<uint32_t> end_page_number{};
};


struct ContentBlock {
    ContentBlockType type{ContentBlockType::TEXT};
    std::optional<std::string> text{};
    std::optional<std::string> thinking{};
    std::optional<std::string> id{};
    std::optional<std::string> name{};
    std::optional<std::string> input{};
    std::vector<Citation> citations{};
};


struct Telemetry {
    uint32_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};


struct UsageMetadata {
    uint32_t input_tokens{0};
    uint32_t output_tokens{0};
    uint32_t thinking_tokens{0};
    uint32_t cache_read_input_tokens{0};
    uint32_t cache_creation_input_tokens{0};
};


struct Response {
    std::string id;
    MessageType type{MessageType::MESSAGE};
    Role role{Role::ASSISTANT};
    std::string model;
    std::vector<ContentBlock> content{};
    std::optional<StopReason> stop_reason{};
    std::optional<std::string> stop_sequence{};
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


constexpr std::string_view to_string_view(anthropic_4_5_opus::CacheType val) {
    switch (val) {
        case anthropic_4_5_opus::CacheType::EPHEMERAL: return "ephemeral";
        default: return "ephemeral";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::CitationType val) {
    switch (val) {
        case anthropic_4_5_opus::CitationType::CHAR_LOCATION: return "char_location";
        case anthropic_4_5_opus::CitationType::PAGE_LOCATION: return "page_location";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ContentBlockType val) {
    switch (val) {
        case anthropic_4_5_opus::ContentBlockType::TEXT: return "text";
        case anthropic_4_5_opus::ContentBlockType::THINKING: return "thinking";
        case anthropic_4_5_opus::ContentBlockType::TOOL_USE: return "tool_use";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::EffortLevel val) {
    switch (val) {
        case anthropic_4_5_opus::EffortLevel::LOW: return "low";
        case anthropic_4_5_opus::EffortLevel::MEDIUM: return "medium";
        case anthropic_4_5_opus::EffortLevel::HIGH: return "high";
        default: return "medium";
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
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::MessageType val) {
    switch (val) {
        case anthropic_4_5_opus::MessageType::MESSAGE: return "message";
        case anthropic_4_5_opus::MessageType::ERROR: return "error";
        default: return "message";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::Role val) {
    switch (val) {
        case anthropic_4_5_opus::Role::USER: return "user";
        case anthropic_4_5_opus::Role::ASSISTANT: return "assistant";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ResponseFormatType val) {
    switch (val) {
        case anthropic_4_5_opus::ResponseFormatType::TEXT: return "text";
        case anthropic_4_5_opus::ResponseFormatType::JSON_OBJECT: return "json_object";
        case anthropic_4_5_opus::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::SourceType val) {
    switch (val) {
        case anthropic_4_5_opus::SourceType::BASE64: return "base64";
        case anthropic_4_5_opus::SourceType::URL: return "url";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::StopReason val) {
    switch (val) {
        case anthropic_4_5_opus::StopReason::END_TURN: return "end_turn";
        case anthropic_4_5_opus::StopReason::MAX_TOKENS: return "max_tokens";
        case anthropic_4_5_opus::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case anthropic_4_5_opus::StopReason::TOOL_USE: return "tool_use";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ThinkingType val) {
    switch (val) {
        case anthropic_4_5_opus::ThinkingType::ENABLED: return "enabled";
        default: return "";
    }
}


constexpr std::string_view to_string_view(anthropic_4_5_opus::ToolChoiceType val) {
    switch (val) {
        case anthropic_4_5_opus::ToolChoiceType::AUTO: return "auto";
        case anthropic_4_5_opus::ToolChoiceType::ANY: return "any";
        case anthropic_4_5_opus::ToolChoiceType::TOOL: return "tool";
        default: return "auto";
    }
}


}
