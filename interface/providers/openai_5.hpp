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


namespace jai::llm::openai_5 {


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
    std::vector<ToolCall> tool_calls{};
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
    struct Function {
        std::string name;
    };

    Function function;
};

struct Request {
    std::string model;
    std::vector<Message> messages;
    std::optional<uint64_t> max_completion_tokens{};
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<uint64_t> seed{};
    std::optional<ReasoningEffort> reasoning_effort{};
    std::optional<Verbosity> verbosity{};
    std::optional<bool> compaction{};
    std::optional<std::string> prompt_cache_key{};
    std::optional<CacheRetention> prompt_cache_retention{};
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
struct Choice {
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

    struct Message {
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
        std::optional<std::string> reasoning_summary{};
        std::optional<std::string> reasoning_content{};
        std::optional<std::string> refusal{};
        std::vector<Annotation> annotations{};
        std::optional<Audio> audio{};
        std::vector<ToolCall> tool_calls{};
    };

    uint32_t index{0};
    FinishReason finish_reason{FinishReason::STOP};
    Message message{};
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
constexpr std::optional<openai_5::CacheRetention> from_string_view<openai_5::CacheRetention>(std::string_view sv) {
    if (sv == "in_memory") return openai_5::CacheRetention::IN_MEMORY;
    if (sv == "24h") return openai_5::CacheRetention::HOURS_24;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ContentPartType> from_string_view<openai_5::ContentPartType>(std::string_view sv) {
    if (sv == "text") return openai_5::ContentPartType::TEXT;
    if (sv == "image_url") return openai_5::ContentPartType::IMAGE_URL;
    if (sv == "audio") return openai_5::ContentPartType::AUDIO;
    if (sv == "video") return openai_5::ContentPartType::VIDEO;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::FinishReason> from_string_view<openai_5::FinishReason>(std::string_view sv) {
    if (sv == "stop") return openai_5::FinishReason::STOP;
    if (sv == "length") return openai_5::FinishReason::LENGTH;
    if (sv == "content_filter") return openai_5::FinishReason::CONTENT_FILTER;
    if (sv == "tool_calls") return openai_5::FinishReason::TOOL_CALLS;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ImageDetail> from_string_view<openai_5::ImageDetail>(std::string_view sv) {
    if (sv == "auto") return openai_5::ImageDetail::AUTO;
    if (sv == "low") return openai_5::ImageDetail::LOW;
    if (sv == "high") return openai_5::ImageDetail::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::Modality> from_string_view<openai_5::Modality>(std::string_view sv) {
    if (sv == "text") return openai_5::Modality::TEXT;
    if (sv == "image") return openai_5::Modality::IMAGE;
    if (sv == "video") return openai_5::Modality::VIDEO;
    if (sv == "audio") return openai_5::Modality::AUDIO;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ObjectType> from_string_view<openai_5::ObjectType>(std::string_view sv) {
    if (sv == "chat.completion") return openai_5::ObjectType::CHAT_COMPLETION;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::PredictionType> from_string_view<openai_5::PredictionType>(std::string_view sv) {
    if (sv == "content") return openai_5::PredictionType::CONTENT;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ReasoningEffort> from_string_view<openai_5::ReasoningEffort>(std::string_view sv) {
    if (sv == "none") return openai_5::ReasoningEffort::NONE;
    if (sv == "minimal") return openai_5::ReasoningEffort::MINIMAL;
    if (sv == "low") return openai_5::ReasoningEffort::LOW;
    if (sv == "medium") return openai_5::ReasoningEffort::MEDIUM;
    if (sv == "high") return openai_5::ReasoningEffort::HIGH;
    if (sv == "xhigh") return openai_5::ReasoningEffort::XHIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ResponseFormatType> from_string_view<openai_5::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return openai_5::ResponseFormatType::TEXT;
    if (sv == "json_object") return openai_5::ResponseFormatType::JSON_OBJECT;
    if (sv == "json_schema") return openai_5::ResponseFormatType::JSON_SCHEMA;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::Role> from_string_view<openai_5::Role>(std::string_view sv) {
    if (sv == "user") return openai_5::Role::USER;
    if (sv == "system") return openai_5::Role::SYSTEM;
    if (sv == "assistant") return openai_5::Role::ASSISTANT;
    if (sv == "tool") return openai_5::Role::TOOL;
    if (sv == "developer") return openai_5::Role::DEVELOPER;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ServiceTier> from_string_view<openai_5::ServiceTier>(std::string_view sv) {
    if (sv == "scale") return openai_5::ServiceTier::SCALE;
    if (sv == "default") return openai_5::ServiceTier::DEFAULT;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::ToolType> from_string_view<openai_5::ToolType>(std::string_view sv) {
    if (sv == "function") return openai_5::ToolType::FUNCTION;
    if (sv == "code_interpreter") return openai_5::ToolType::CODE_INTERPRETER;
    if (sv == "file_search") return openai_5::ToolType::FILE_SEARCH;
    return std::nullopt;
}


template <>
constexpr std::optional<openai_5::Verbosity> from_string_view<openai_5::Verbosity>(std::string_view sv) {
    if (sv == "concise") return openai_5::Verbosity::CONCISE;
    if (sv == "medium") return openai_5::Verbosity::MEDIUM;
    if (sv == "detailed") return openai_5::Verbosity::DETAILED;
    return std::nullopt;
}


constexpr std::string_view to_string_view(openai_5::CacheRetention val) {
    switch (val) {
        case openai_5::CacheRetention::IN_MEMORY: return "in_memory";
        case openai_5::CacheRetention::HOURS_24: return "24h";
        default: throw std::logic_error("invalid openai_5::CacheRetention");
    }
}


constexpr std::string_view to_string_view(openai_5::ContentPartType val) {
    switch (val) {
        case openai_5::ContentPartType::TEXT: return "text";
        case openai_5::ContentPartType::IMAGE_URL: return "image_url";
        case openai_5::ContentPartType::AUDIO: return "audio";
        case openai_5::ContentPartType::VIDEO: return "video";
        default: throw std::logic_error("invalid openai_5::ContentPartType");
    }
}


constexpr std::string_view to_string_view(openai_5::FinishReason val) {
    switch (val) {
        case openai_5::FinishReason::STOP: return "stop";
        case openai_5::FinishReason::LENGTH: return "length";
        case openai_5::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_5::FinishReason::TOOL_CALLS: return "tool_calls";
        default: throw std::logic_error("invalid openai_5::FinishReason");
    }
}


constexpr std::string_view to_string_view(openai_5::ImageDetail val) {
    switch (val) {
        case openai_5::ImageDetail::AUTO: return "auto";
        case openai_5::ImageDetail::LOW: return "low";
        case openai_5::ImageDetail::HIGH: return "high";
        default: throw std::logic_error("invalid openai_5::ImageDetail");
    }
}


constexpr std::string_view to_string_view(openai_5::Modality val) {
    switch (val) {
        case openai_5::Modality::TEXT: return "text";
        case openai_5::Modality::IMAGE: return "image";
        case openai_5::Modality::VIDEO: return "video";
        case openai_5::Modality::AUDIO: return "audio";
        default: throw std::logic_error("invalid openai_5::Modality");
    }
}


constexpr std::string_view to_string_view(openai_5::ObjectType val) {
    switch (val) {
        case openai_5::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: throw std::logic_error("invalid openai_5::ObjectType");
    }
}


constexpr std::string_view to_string_view(openai_5::PredictionType val) {
    switch (val) {
        case openai_5::PredictionType::CONTENT: return "content";
        default: throw std::logic_error("invalid openai_5::PredictionType");
    }
}


constexpr std::string_view to_string_view(openai_5::ReasoningEffort val) {
    switch (val) {
        case openai_5::ReasoningEffort::NONE: return "none";
        case openai_5::ReasoningEffort::MINIMAL: return "minimal";
        case openai_5::ReasoningEffort::LOW: return "low";
        case openai_5::ReasoningEffort::MEDIUM: return "medium";
        case openai_5::ReasoningEffort::HIGH: return "high";
        case openai_5::ReasoningEffort::XHIGH: return "xhigh";
        default: throw std::logic_error("invalid openai_5::ReasoningEffort");
    }
}


constexpr std::string_view to_string_view(openai_5::ResponseFormatType val) {
    switch (val) {
        case openai_5::ResponseFormatType::TEXT: return "text";
        case openai_5::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_5::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid openai_5::ResponseFormatType");
    }
}


constexpr std::string_view to_string_view(openai_5::Role val) {
    switch (val) {
        case openai_5::Role::USER: return "user";
        case openai_5::Role::SYSTEM: return "system";
        case openai_5::Role::ASSISTANT: return "assistant";
        case openai_5::Role::TOOL: return "tool";
        case openai_5::Role::DEVELOPER: return "developer";
        default: throw std::logic_error("invalid openai_5::Role");
    }
}


constexpr std::string_view to_string_view(openai_5::ServiceTier val) {
    switch (val) {
        case openai_5::ServiceTier::SCALE: return "scale";
        case openai_5::ServiceTier::DEFAULT: return "default";
        default: throw std::logic_error("invalid openai_5::ServiceTier");
    }
}


constexpr std::string_view to_string_view(openai_5::ToolType val) {
    switch (val) {
        case openai_5::ToolType::FUNCTION: return "function";
        case openai_5::ToolType::CODE_INTERPRETER: return "code_interpreter";
        case openai_5::ToolType::FILE_SEARCH: return "file_search";
        default: throw std::logic_error("invalid openai_5::ToolType");
    }
}


constexpr std::string_view to_string_view(openai_5::Verbosity val) {
    switch (val) {
        case openai_5::Verbosity::CONCISE: return "concise";
        case openai_5::Verbosity::MEDIUM: return "medium";
        case openai_5::Verbosity::DETAILED: return "detailed";
        default: throw std::logic_error("invalid openai_5::Verbosity");
    }
}


}
