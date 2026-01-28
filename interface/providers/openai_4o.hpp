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


namespace jai::llm::openai_4o {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view/from_string_view conversions defined below.
 */
enum class CacheRetention { HOURS_24, IN_MEMORY };
enum class ContentPartType { AUDIO, IMAGE_URL, TEXT, VIDEO };
enum class FinishReason { CONTENT_FILTER, FINISH_REASON_UNSPECIFIED, LENGTH, STOP, TOOL_CALLS };
enum class ImageDetail { AUTO, HIGH, LOW };
enum class Modality { AUDIO, IMAGE, TEXT, VIDEO };
enum class ObjectType { CHAT_COMPLETION };
enum class PredictionType { CONTENT };
enum class ReasoningEffort { HIGH, LOW, MEDIUM, MINIMAL, NONE, XHIGH };
enum class ResponseFormatType { JSON_OBJECT, JSON_SCHEMA, TEXT };
enum class Role { ASSISTANT, DEVELOPER, SYSTEM, TOOL, USER };
enum class ServiceTier { DEFAULT, SCALE };
enum class ToolType { CODE_INTERPRETER, FILE_SEARCH, FUNCTION };
enum class Verbosity { CONCISE, DETAILED, MEDIUM };


/***
 * Request
 */
struct Audio {
    struct AudioData {
        std::string data; // Base64
        std::string format;
    };

    AudioData audio;
};


struct AudioConfig {
    std::string voice;
    std::string format;
};


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


struct Video {
    struct VideoUrl {
        EncodedUrl url;
        std::optional<ImageDetail> detail{};
    };

    VideoUrl video_url;
};


struct Message {
    using Part = std::variant<Text, Image, Audio, Video>;

    /**
     * OpenAI Constraint: 'developer' and 'system' roles MUST be text-only (std::string).
     * 'user' and 'assistant' roles can be multimodal (vector of parts).
     */
    Role role{Role::USER};
    std::variant<std::string, std::vector<Part>> content;
    std::optional<std::string> name{};
    std::optional<std::string> tool_call_id{};
    std::vector<ToolCall> tool_calls;
};


struct Metadata {
    std::string key;
    std::string value;
};


struct Prediction {
    std::variant<std::string, std::vector<Message::Part>> content;
};


struct ResponseFormat {
    struct JsonObject {};
    struct JsonSchema {
        std::string name;
        std::optional<std::string> description{};
        std::string schema{}; // JSON Schema string
        std::optional<bool> strict{};
    };
    struct Text {};

    std::variant<Text, JsonObject, JsonSchema> detail{Text{}};
};


struct Tool {
    struct CodeInterpreter {};

    struct FileSearch {
        uint32_t max_num_results{20};
    };

    struct Function {
        std::string name{};
        std::optional<std::string> description{};
        std::string parameters{}; // JSON Schema string
        std::optional<bool> strict{};
    };

    std::variant<Function, CodeInterpreter, FileSearch> detail{Function{}};
};


struct ToolChoiceSpecific {
    struct Function { std::string name; };
    Function function;
};


struct Request {
    std::string model;
    std::vector<Message> messages;
    std::optional<uint64_t> max_completion_tokens{}; // Replaces max_tokens for newer models
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<ReasoningEffort> reasoning_effort{};
    std::optional<Verbosity> verbosity{};
    std::optional<bool> compaction{};
    std::optional<std::string> prompt_cache_key{};
    std::optional<CacheRetention> prompt_cache_retention{}; // "in_memory", "24h"
    std::vector<std::string> stop{};
    std::optional<double> presence_penalty{};
    std::optional<double> frequency_penalty{};
    std::optional<std::string> user{};
    std::optional<uint64_t> seed{};
    std::optional<ResponseFormat> response_format{};
    std::vector<Tool> tools{};
    std::variant<std::monostate, std::string, ToolChoiceSpecific> tool_choice{};
    std::optional<bool> parallel_tool_calls{};
    std::optional<Prediction> prediction{};
    std::optional<AudioConfig> audio{};
    std::vector<Modality> modalities{};
    std::optional<bool> store{};
    std::vector<Metadata> metadata{};
};


/***
 * Response
 */
struct Logprobs {
    struct Content {
        struct TopLogprob {
            std::string token;
            double logprob{0.0};
            std::vector<uint8_t> bytes;
        };

        std::string token;
        double logprob{0.0};
        std::vector<uint8_t> bytes;
        std::vector<TopLogprob> top_logprobs;
    };

    std::vector<Content> content;
    std::vector<Content> refusal;
};


struct ResponseMessage {
    struct Annotation {
        struct FileCitation {
            std::string file_id;
            std::string quote;
        };

        struct UrlCitation {
            EncodedUrl url;
            std::string title;
        };

        uint32_t start_index{0};
        uint32_t end_index{0};
        std::string text{};
        std::optional<FileCitation> file_citation{};
        std::optional<UrlCitation> url_citation{};
    };

    struct Audio {
        std::string id;
        uint64_t expires_at{0};
        std::string data; // Base64
        std::string transcript;
    };

    Role role{Role::ASSISTANT};
    std::optional<std::string> content{};
    std::optional<std::string> refusal{};           // Explicit refusal message
    std::optional<std::string> reasoning_content{}; // Internal reasoning chain
    std::vector<Annotation> annotations{};
    std::optional<Audio> audio{};
    std::vector<ToolCall> tool_calls{};
};

struct Choice {
    uint32_t index{0};
    FinishReason finish_reason{FinishReason::FINISH_REASON_UNSPECIFIED};
    ResponseMessage message{};
    std::optional<Logprobs> logprobs{};
};


struct Telemetry {
    uint64_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};


struct UsageMetadata {
    struct PromptTokensDetails {
        uint64_t cached_tokens{0};
        uint64_t audio_tokens{0};
        uint64_t image_tokens{0};
        uint64_t video_tokens{0};
    };

    struct CompletionTokensDetails {
        uint64_t reasoning_tokens{0};
        uint64_t audio_tokens{0};
        uint64_t accepted_prediction_tokens{0};
        uint64_t rejected_prediction_tokens{0};
    };

    uint64_t prompt_tokens{0};
    uint64_t completion_tokens{0};
    uint64_t total_tokens{0};
    PromptTokensDetails prompt_tokens_details{};
    CompletionTokensDetails completion_tokens_details{};
};


struct Response {
    std::string id;
    uint64_t created{0};
    std::string model;
    std::optional<std::string> system_fingerprint{};
    std::vector<Choice> choices{};
    UsageMetadata usage{};
    std::optional<ServiceTier> service_tier{};
    std::optional<Telemetry> telemetry{};
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
constexpr std::optional<openai_4o::CacheRetention> from_string_view<openai_4o::CacheRetention>(std::string_view sv) {
    if (sv == "in_memory") return openai_4o::CacheRetention::IN_MEMORY;
    if (sv == "24h") return openai_4o::CacheRetention::HOURS_24;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ContentPartType> from_string_view<openai_4o::ContentPartType>(std::string_view sv) {
    if (sv == "text") return openai_4o::ContentPartType::TEXT;
    if (sv == "image_url") return openai_4o::ContentPartType::IMAGE_URL;
    if (sv == "audio") return openai_4o::ContentPartType::AUDIO;
    if (sv == "video") return openai_4o::ContentPartType::VIDEO;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::FinishReason> from_string_view<openai_4o::FinishReason>(std::string_view sv) {
    if (sv == "stop") return openai_4o::FinishReason::STOP;
    if (sv == "length") return openai_4o::FinishReason::LENGTH;
    if (sv == "content_filter") return openai_4o::FinishReason::CONTENT_FILTER;
    if (sv == "tool_calls") return openai_4o::FinishReason::TOOL_CALLS;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ImageDetail> from_string_view<openai_4o::ImageDetail>(std::string_view sv) {
    if (sv == "auto") return openai_4o::ImageDetail::AUTO;
    if (sv == "low") return openai_4o::ImageDetail::LOW;
    if (sv == "high") return openai_4o::ImageDetail::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::Modality> from_string_view<openai_4o::Modality>(std::string_view sv) {
    if (sv == "text") return openai_4o::Modality::TEXT;
    if (sv == "image") return openai_4o::Modality::IMAGE;
    if (sv == "video") return openai_4o::Modality::VIDEO;
    if (sv == "audio") return openai_4o::Modality::AUDIO;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ObjectType> from_string_view<openai_4o::ObjectType>(std::string_view sv) {
    if (sv == "chat.completion") return openai_4o::ObjectType::CHAT_COMPLETION;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::PredictionType> from_string_view<openai_4o::PredictionType>(std::string_view sv) {
    if (sv == "content") return openai_4o::PredictionType::CONTENT;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ReasoningEffort> from_string_view<openai_4o::ReasoningEffort>(std::string_view sv) {
    if (sv == "none") return openai_4o::ReasoningEffort::NONE;
    if (sv == "minimal") return openai_4o::ReasoningEffort::MINIMAL;
    if (sv == "low") return openai_4o::ReasoningEffort::LOW;
    if (sv == "medium") return openai_4o::ReasoningEffort::MEDIUM;
    if (sv == "high") return openai_4o::ReasoningEffort::HIGH;
    if (sv == "xhigh") return openai_4o::ReasoningEffort::XHIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ResponseFormatType> from_string_view<openai_4o::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return openai_4o::ResponseFormatType::TEXT;
    if (sv == "json_object") return openai_4o::ResponseFormatType::JSON_OBJECT;
    if (sv == "json_schema") return openai_4o::ResponseFormatType::JSON_SCHEMA;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::Role> from_string_view<openai_4o::Role>(std::string_view sv) {
    if (sv == "user") return openai_4o::Role::USER;
    if (sv == "system") return openai_4o::Role::SYSTEM;
    if (sv == "assistant") return openai_4o::Role::ASSISTANT;
    if (sv == "tool") return openai_4o::Role::TOOL;
    if (sv == "developer") return openai_4o::Role::DEVELOPER;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ServiceTier> from_string_view<openai_4o::ServiceTier>(std::string_view sv) {
    if (sv == "scale") return openai_4o::ServiceTier::SCALE;
    if (sv == "default") return openai_4o::ServiceTier::DEFAULT;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::ToolType> from_string_view<openai_4o::ToolType>(std::string_view sv) {
    if (sv == "function") return openai_4o::ToolType::FUNCTION;
    if (sv == "code_interpreter") return openai_4o::ToolType::CODE_INTERPRETER;
    if (sv == "file_search") return openai_4o::ToolType::FILE_SEARCH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_4o::Verbosity> from_string_view<openai_4o::Verbosity>(std::string_view sv) {
    if (sv == "concise") return openai_4o::Verbosity::CONCISE;
    if (sv == "medium") return openai_4o::Verbosity::MEDIUM;
    if (sv == "detailed") return openai_4o::Verbosity::DETAILED;
    return std::nullopt;
}


constexpr std::string_view to_string_view(openai_4o::CacheRetention val) {
    switch (val) {
        case openai_4o::CacheRetention::IN_MEMORY: return "in_memory";
        case openai_4o::CacheRetention::HOURS_24: return "24h";
        default: throw std::logic_error("invalid openai_4o::CacheRetention");
    }
}


constexpr std::string_view to_string_view(openai_4o::ContentPartType val) {
    switch (val) {
        case openai_4o::ContentPartType::TEXT: return "text";
        case openai_4o::ContentPartType::IMAGE_URL: return "image_url";
        case openai_4o::ContentPartType::AUDIO: return "audio";
        case openai_4o::ContentPartType::VIDEO: return "video";
        default: throw std::logic_error("invalid openai_4o::ContentPartType");
    }
}


constexpr std::string_view to_string_view(openai_4o::FinishReason val) {
    switch (val) {
        case openai_4o::FinishReason::STOP: return "stop";
        case openai_4o::FinishReason::LENGTH: return "length";
        case openai_4o::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_4o::FinishReason::TOOL_CALLS: return "tool_calls";
        default: throw std::logic_error("invalid openai_4o::FinishReason");
    }
}


constexpr std::string_view to_string_view(openai_4o::ImageDetail val) {
    switch (val) {
        case openai_4o::ImageDetail::AUTO: return "auto";
        case openai_4o::ImageDetail::LOW: return "low";
        case openai_4o::ImageDetail::HIGH: return "high";
        default: throw std::logic_error("invalid openai_4o::ImageDetail");
    }
}


constexpr std::string_view to_string_view(openai_4o::Modality val) {
    switch (val) {
        case openai_4o::Modality::TEXT: return "text";
        case openai_4o::Modality::IMAGE: return "image";
        case openai_4o::Modality::VIDEO: return "video";
        case openai_4o::Modality::AUDIO: return "audio";
        default: throw std::logic_error("invalid openai_4o::Modality");
    }
}


constexpr std::string_view to_string_view(openai_4o::ObjectType val) {
    switch (val) {
        case openai_4o::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: throw std::logic_error("invalid openai_4o::ObjectType");
    }
}


constexpr std::string_view to_string_view(openai_4o::PredictionType val) {
    switch (val) {
        case openai_4o::PredictionType::CONTENT: return "content";
        default: throw std::logic_error("invalid openai_4o::PredictionType");
    }
}


constexpr std::string_view to_string_view(openai_4o::ReasoningEffort val) {
    switch (val) {
        case openai_4o::ReasoningEffort::NONE: return "none";
        case openai_4o::ReasoningEffort::MINIMAL: return "minimal";
        case openai_4o::ReasoningEffort::LOW: return "low";
        case openai_4o::ReasoningEffort::MEDIUM: return "medium";
        case openai_4o::ReasoningEffort::HIGH: return "high";
        case openai_4o::ReasoningEffort::XHIGH: return "xhigh";
        default: throw std::logic_error("invalid openai_4o::ReasoningEffort");
    }
}


constexpr std::string_view to_string_view(openai_4o::ResponseFormatType val) {
    switch (val) {
        case openai_4o::ResponseFormatType::TEXT: return "text";
        case openai_4o::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_4o::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid openai_4o::ResponseFormatType");
    }
}


constexpr std::string_view to_string_view(openai_4o::Role val) {
    switch (val) {
        case openai_4o::Role::USER: return "user";
        case openai_4o::Role::SYSTEM: return "system";
        case openai_4o::Role::ASSISTANT: return "assistant";
        case openai_4o::Role::TOOL: return "tool";
        case openai_4o::Role::DEVELOPER: return "developer";
        default: throw std::logic_error("invalid openai_4o::Role");
    }
}


constexpr std::string_view to_string_view(openai_4o::ServiceTier val) {
    switch (val) {
        case openai_4o::ServiceTier::SCALE: return "scale";
        case openai_4o::ServiceTier::DEFAULT: return "default";
        default: throw std::logic_error("invalid openai_4o::ServiceTier");
    }
}


constexpr std::string_view to_string_view(openai_4o::ToolType val) {
    switch (val) {
        case openai_4o::ToolType::FUNCTION: return "function";
        case openai_4o::ToolType::CODE_INTERPRETER: return "code_interpreter";
        case openai_4o::ToolType::FILE_SEARCH: return "file_search";
        default: throw std::logic_error("invalid openai_4o::ToolType");
    }
}


constexpr std::string_view to_string_view(openai_4o::Verbosity val) {
    switch (val) {
        case openai_4o::Verbosity::CONCISE: return "concise";
        case openai_4o::Verbosity::MEDIUM: return "medium";
        case openai_4o::Verbosity::DETAILED: return "detailed";
        default: throw std::logic_error("invalid openai_4o::Verbosity");
    }
}


}
