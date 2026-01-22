#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"
#include "../url.hpp"


namespace jai::llm::openai_4 {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view/from_string_view conversions defined below.
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
        EncodedUrl url;
        std::optional<ImageDetail> detail{};
    };

    ImageUrl image_url;
};


struct Text {
    std::string text;
};


struct ToolCall {
    struct Function {
        std::string name;
        std::string arguments{}; // JSON string
    };

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
    struct JsonObject {};
    struct JsonSchema {
        std::string name;
        std::optional<std::string> description{};
        std::string schema; // JSON Schema string
        std::optional<bool> strict{};
    };
    struct Text {};

    std::variant<Text, JsonObject, JsonSchema> detail{Text{}};
};


struct Tool {
    struct Function {
        std::string name{};
        std::optional<std::string> description{};
        std::string parameters{}; // JSON Schema string
        std::optional<bool> strict{};
    };

    Function function{};
};


struct Request {
    std::string model;
    std::vector<Message> messages;
    std::optional<uint64_t> max_tokens{};
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
    uint64_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};

struct UsageMetadata {
    struct PromptTokensDetails {
        uint64_t cached_tokens{0};
    };

    uint64_t prompt_tokens{0};
    uint64_t completion_tokens{0};
    uint64_t total_tokens{0};
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

    AsyncTask<Response> GenerateTextAsync(const Request& r) const;
    Response GenerateTextSync(const Request& r) const;
};


}


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv);


template <>
constexpr std::optional<openai_4::ContentPartType> from_string_view<openai_4::ContentPartType>(std::string_view sv) {
    if (sv == "text") return openai_4::ContentPartType::TEXT;
    if (sv == "image_url") return openai_4::ContentPartType::IMAGE_URL;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::FinishReason> from_string_view<openai_4::FinishReason>(std::string_view sv) {
    if (sv == "stop") return openai_4::FinishReason::STOP;
    if (sv == "length") return openai_4::FinishReason::LENGTH;
    if (sv == "content_filter") return openai_4::FinishReason::CONTENT_FILTER;
    if (sv == "tool_calls") return openai_4::FinishReason::TOOL_CALLS;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::ImageDetail> from_string_view<openai_4::ImageDetail>(std::string_view sv) {
    if (sv == "auto") return openai_4::ImageDetail::AUTO;
    if (sv == "low") return openai_4::ImageDetail::LOW;
    if (sv == "high") return openai_4::ImageDetail::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::ObjectType> from_string_view<openai_4::ObjectType>(std::string_view sv) {
    if (sv == "chat.completion") return openai_4::ObjectType::CHAT_COMPLETION;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::ResponseFormatType> from_string_view<openai_4::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return openai_4::ResponseFormatType::TEXT;
    if (sv == "json_object") return openai_4::ResponseFormatType::JSON_OBJECT;
    if (sv == "json_schema") return openai_4::ResponseFormatType::JSON_SCHEMA;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::Role> from_string_view<openai_4::Role>(std::string_view sv) {
    if (sv == "user") return openai_4::Role::USER;
    if (sv == "system") return openai_4::Role::SYSTEM;
    if (sv == "assistant") return openai_4::Role::ASSISTANT;
    if (sv == "tool") return openai_4::Role::TOOL;
    if (sv == "developer") return openai_4::Role::DEVELOPER;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4::ToolType> from_string_view<openai_4::ToolType>(std::string_view sv) {
    if (sv == "function") return openai_4::ToolType::FUNCTION;
    return std::nullopt;
}


constexpr std::string_view to_string_view(openai_4::ContentPartType val) {
    switch (val) {
        case openai_4::ContentPartType::TEXT: return "text";
        case openai_4::ContentPartType::IMAGE_URL: return "image_url";
        default: throw std::logic_error("invalid openai_4::ContentPartType");
    }
}


constexpr std::string_view to_string_view(openai_4::FinishReason val) {
    switch (val) {
        case openai_4::FinishReason::STOP: return "stop";
        case openai_4::FinishReason::LENGTH: return "length";
        case openai_4::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_4::FinishReason::TOOL_CALLS: return "tool_calls";
        default: throw std::logic_error("invalid openai_4::FinishReason");
    }
}


constexpr std::string_view to_string_view(openai_4::ImageDetail val) {
    switch (val) {
        case openai_4::ImageDetail::AUTO: return "auto";
        case openai_4::ImageDetail::LOW: return "low";
        case openai_4::ImageDetail::HIGH: return "high";
        default: throw std::logic_error("invalid openai_4::ImageDetail");
    }
}


constexpr std::string_view to_string_view(openai_4::ObjectType val) {
    switch (val) {
        case openai_4::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: throw std::logic_error("invalid openai_4::ObjectType");
    }
}


constexpr std::string_view to_string_view(openai_4::ResponseFormatType val) {
    switch (val) {
        case openai_4::ResponseFormatType::TEXT: return "text";
        case openai_4::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_4::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid openai_4::ResponseFormatType");
    }
}


constexpr std::string_view to_string_view(openai_4::Role val) {
    switch (val) {
        case openai_4::Role::USER: return "user";
        case openai_4::Role::SYSTEM: return "system";
        case openai_4::Role::ASSISTANT: return "assistant";
        case openai_4::Role::TOOL: return "tool";
        case openai_4::Role::DEVELOPER: return "developer";
        default: throw std::logic_error("invalid openai_4::Role");
    }
}


constexpr std::string_view to_string_view(openai_4::ToolType val) {
    switch (val) {
        case openai_4::ToolType::FUNCTION: return "function";
        default: throw std::logic_error("invalid openai_4::ToolType");
    }
}


}
