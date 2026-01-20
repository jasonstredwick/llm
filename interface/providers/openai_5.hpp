#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"


namespace jai::llm::openai_5 {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view conversions defined below.
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
        std::string url;
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
        std::string url;
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
    std::optional<uint32_t> max_completion_tokens{};
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
                std::string url;
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
    uint32_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};


struct UsageMetadata {
    struct PromptTokensDetails {
        uint32_t cached_tokens{0};
        uint32_t audio_tokens{0};
        uint32_t image_tokens{0};
        uint32_t video_tokens{0};
    };

    struct CompletionTokensDetails {
        uint32_t reasoning_tokens{0};
        uint32_t audio_tokens{0};
        uint32_t accepted_prediction_tokens{0};
        uint32_t rejected_prediction_tokens{0};
    };

    uint32_t prompt_tokens{0};
    uint32_t completion_tokens{0};
    uint32_t total_tokens{0};
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


constexpr std::string_view to_string_view(openai_5::Role val) {
    switch (val) {
        case openai_5::Role::USER: return "user";
        case openai_5::Role::SYSTEM: return "system";
        case openai_5::Role::ASSISTANT: return "assistant";
        case openai_5::Role::TOOL: return "tool";
        case openai_5::Role::DEVELOPER: return "developer";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_5::Modality val) {
    switch (val) {
        case openai_5::Modality::TEXT: return "text";
        case openai_5::Modality::IMAGE: return "image";
        case openai_5::Modality::VIDEO: return "video";
        case openai_5::Modality::AUDIO: return "audio";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_5::FinishReason val) {
    switch (val) {
        case openai_5::FinishReason::STOP: return "stop";
        case openai_5::FinishReason::LENGTH: return "length";
        case openai_5::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_5::FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_5::ImageDetail val) {
    switch (val) {
        case openai_5::ImageDetail::AUTO: return "auto";
        case openai_5::ImageDetail::LOW: return "low";
        case openai_5::ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}


constexpr std::string_view to_string_view(openai_5::ContentPartType val) {
    switch (val) {
        case openai_5::ContentPartType::TEXT: return "text";
        case openai_5::ContentPartType::IMAGE_URL: return "image_url";
        case openai_5::ContentPartType::AUDIO: return "audio";
        case openai_5::ContentPartType::VIDEO: return "video";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_5::ToolType val) {
    switch (val) {
        case openai_5::ToolType::FUNCTION: return "function";
        case openai_5::ToolType::CODE_INTERPRETER: return "code_interpreter";
        case openai_5::ToolType::FILE_SEARCH: return "file_search";
        default: return "function";
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
        default: return "medium";
    }
}


constexpr std::string_view to_string_view(openai_5::Verbosity val) {
    switch (val) {
        case openai_5::Verbosity::CONCISE: return "concise";
        case openai_5::Verbosity::MEDIUM: return "medium";
        case openai_5::Verbosity::DETAILED: return "detailed";
        default: return "medium";
    }
}


constexpr std::string_view to_string_view(openai_5::CacheRetention val) {
    switch (val) {
        case openai_5::CacheRetention::IN_MEMORY: return "in_memory";
        case openai_5::CacheRetention::HOURS_24: return "24h";
        default: return "in_memory";
    }
}


constexpr std::string_view to_string_view(openai_5::ResponseFormatType val) {
    switch (val) {
        case openai_5::ResponseFormatType::TEXT: return "text";
        case openai_5::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_5::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}


constexpr std::string_view to_string_view(openai_5::PredictionType val) {
    switch (val) {
        case openai_5::PredictionType::CONTENT: return "content";
        default: return "content";
    }
}


constexpr std::string_view to_string_view(openai_5::ServiceTier val) {
    switch (val) {
        case openai_5::ServiceTier::SCALE: return "scale";
        case openai_5::ServiceTier::DEFAULT: return "default";
        default: return "default";
    }
}


constexpr std::string_view to_string_view(openai_5::ObjectType val) {
    switch (val) {
        case openai_5::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}


}
