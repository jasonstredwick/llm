#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"


namespace jai::llm::openai_4 {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view conversions defined below.
 */
enum class ContentPartType { IMAGE_URL, TEXT };
enum class FinishReason { CONTENT_FILTER, FINISH_REASON_UNSPECIFIED, LENGTH, STOP, TOOL_CALLS };
enum class ImageDetail { AUTO, HIGH, LOW };
enum class ObjectType { CHAT_COMPLETION };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, DEVELOPER, SYSTEM, TOOL, USER };
enum class ToolType { FUNCTION };


/***
 * Request
 */
struct Image {
    struct ImageUrl {
        std::string url;
        std::optional<ImageDetail> detail{};
    };

    ContentPartType type{ContentPartType::IMAGE_URL};
    ImageUrl image_url;
};


struct Text {
    ContentPartType type{ContentPartType::TEXT};
    std::string text;
};


struct ToolCall {
    struct Function {
        std::string name;
        std::string arguments{}; // JSON string
    };

    ToolType type{ToolType::FUNCTION};
    std::string id;
    Function function;
};


struct Message {
    using Part = std::variant<Text, Image>;

    /**
     * OpenAI Constraint: 'developer' and 'system' roles MUST be text-only (std::string).
     * 'user' and 'assistant' roles can be multimodal (vector of parts).
     */
    Role role{Role::USER};
    std::variant<std::string, std::vector<Part>> content;
    std::optional<std::string> name{};
    std::optional<std::string> tool_call_id{};
    std::vector<ToolCall> tool_calls{};
};


struct ResponseFormat {
    struct JsonSchema {
        std::string name;
        std::optional<std::string> description{};
        std::string schema; // JSON Schema string
        std::optional<bool> strict{};
    };

    ResponseFormatType type{ResponseFormatType::TEXT};
    std::optional<JsonSchema> json_schema{};
};


struct Tool {
    struct Function {
        std::string name{};
        std::optional<std::string> description{};
        std::string parameters{}; // JSON Schema string
        std::optional<bool> strict{};
    };

    ToolType type{ToolType::FUNCTION};
    Function function{};
};


struct Request {
    std::string model;
    std::vector<Message> messages;
    std::optional<uint32_t> max_tokens{};
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<uint32_t> n{};
    std::vector<std::string> stop{};
    std::optional<double> presence_penalty{};
    std::optional<double> frequency_penalty{};
    std::optional<std::string> user{};
    std::optional<uint64_t> seed{};
    std::optional<ResponseFormat> response_format{};
    std::vector<Tool> tools{};
};


/***
 * Response
 */
struct ResponseTelemetry {
    uint32_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};

struct UsageMetadata {
    struct PromptTokensDetails {
        uint32_t cached_tokens{0};
    };

    uint32_t prompt_tokens{0};
    uint32_t completion_tokens{0};
    uint32_t total_tokens{0};
    PromptTokensDetails prompt_tokens_details{};
};

struct Response {
    struct Choice {
        struct Message {
            Role role{Role::ASSISTANT};
            std::optional<std::string> content{};
            std::vector<ToolCall> tool_calls{};
        };

        uint32_t index{0};
        FinishReason finish_reason{FinishReason::FINISH_REASON_UNSPECIFIED};
        Message message;
    };

    std::string id;
    ObjectType object{ObjectType::CHAT_COMPLETION};
    uint64_t created{0};
    std::string model;
    std::optional<std::string> system_fingerprint{};
    std::vector<Choice> choices{};
    UsageMetadata usage{};
    std::optional<ResponseTelemetry> telemetry{};
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

    ResponseAsync GenerateTextAsync(const Request& r) const;
    Response      GenerateTextSync(const Request& r) const;
};


} // namespace jai::llm::openai_4


namespace jai::llm {


constexpr std::string_view to_string_view(openai_4::Role val) {
    switch (val) {
        case openai_4::Role::USER: return "user";
        case openai_4::Role::SYSTEM: return "system";
        case openai_4::Role::ASSISTANT: return "assistant";
        case openai_4::Role::TOOL: return "tool";
        case openai_4::Role::DEVELOPER: return "developer";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4::ContentPartType val) {
    switch (val) {
        case openai_4::ContentPartType::TEXT: return "text";
        case openai_4::ContentPartType::IMAGE_URL: return "image_url";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4::ImageDetail val) {
    switch (val) {
        case openai_4::ImageDetail::AUTO: return "auto";
        case openai_4::ImageDetail::LOW: return "low";
        case openai_4::ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}


constexpr std::string_view to_string_view(openai_4::FinishReason val) {
    switch (val) {
        case openai_4::FinishReason::STOP: return "stop";
        case openai_4::FinishReason::LENGTH: return "length";
        case openai_4::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_4::FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4::ToolType val) {
    switch (val) {
        case openai_4::ToolType::FUNCTION: return "function";
        default: return "function";
    }
}


constexpr std::string_view to_string_view(openai_4::ResponseFormatType val) {
    switch (val) {
        case openai_4::ResponseFormatType::TEXT: return "text";
        case openai_4::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_4::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}


constexpr std::string_view to_string_view(openai_4::ObjectType val) {
    switch (val) {
        case openai_4::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}


} // namespace jai::llm
