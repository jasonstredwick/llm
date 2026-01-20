#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"


namespace jai::llm::openai_4o {


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

    ContentPartType type{ContentPartType::AUDIO};
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


struct Video {
    struct VideoUrl {
        std::string url;
        std::optional<ImageDetail> detail{};
    };
    ContentPartType type{ContentPartType::VIDEO};
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
    PredictionType type{PredictionType::CONTENT};
    std::variant<std::string, std::vector<Message::Part>> content;
};


struct ResponseFormat {
    struct JsonSchema {
        std::string name;
        std::optional<std::string> description{};
        std::string schema{}; // JSON Schema string
        std::optional<bool> strict{};
    };

    ResponseFormatType type{ResponseFormatType::TEXT}; // "text", "json_object", "json_schema"
    std::optional<JsonSchema> json_schema{};
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

    ToolType type{ToolType::FUNCTION};
    Function function{};
    std::optional<CodeInterpreter> code_interpreter{};
    std::optional<FileSearch> file_search{};
};


struct ToolChoiceSpecific {
    struct Function { std::string name; };

    ToolType type{ToolType::FUNCTION};
    Function function;
};


struct Request {
    std::string model;
    std::vector<Message> messages;
    std::optional<uint32_t> max_completion_tokens{}; // Replaces max_tokens for newer models
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


constexpr std::string_view to_string_view(openai_4o::Role val) {
    switch (val) {
        case openai_4o::Role::USER: return "user";
        case openai_4o::Role::SYSTEM: return "system";
        case openai_4o::Role::ASSISTANT: return "assistant";
        case openai_4o::Role::TOOL: return "tool";
        case openai_4o::Role::DEVELOPER: return "developer";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4o::Modality val) {
    switch (val) {
        case openai_4o::Modality::TEXT: return "text";
        case openai_4o::Modality::IMAGE: return "image";
        case openai_4o::Modality::VIDEO: return "video";
        case openai_4o::Modality::AUDIO: return "audio";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4o::FinishReason val) {
    switch (val) {
        case openai_4o::FinishReason::STOP: return "stop";
        case openai_4o::FinishReason::LENGTH: return "length";
        case openai_4o::FinishReason::CONTENT_FILTER: return "content_filter";
        case openai_4o::FinishReason::TOOL_CALLS: return "tool_calls";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4o::ImageDetail val) {
    switch (val) {
        case openai_4o::ImageDetail::AUTO: return "auto";
        case openai_4o::ImageDetail::LOW: return "low";
        case openai_4o::ImageDetail::HIGH: return "high";
        default: return "auto";
    }
}


constexpr std::string_view to_string_view(openai_4o::ContentPartType val) {
    switch (val) {
        case openai_4o::ContentPartType::TEXT: return "text";
        case openai_4o::ContentPartType::IMAGE_URL: return "image_url";
        case openai_4o::ContentPartType::AUDIO: return "audio";
        case openai_4o::ContentPartType::VIDEO: return "video";
        default: return "";
    }
}


constexpr std::string_view to_string_view(openai_4o::ToolType val) {
    switch (val) {
        case openai_4o::ToolType::FUNCTION: return "function";
        case openai_4o::ToolType::CODE_INTERPRETER: return "code_interpreter";
        case openai_4o::ToolType::FILE_SEARCH: return "file_search";
        default: return "function";
    }
}


constexpr std::string_view to_string_view(openai_4o::CacheRetention val) {
    switch (val) {
        case openai_4o::CacheRetention::IN_MEMORY: return "in_memory";
        case openai_4o::CacheRetention::HOURS_24: return "24h";
        default: return "in_memory";
    }
}


constexpr std::string_view to_string_view(openai_4o::ResponseFormatType val) {
    switch (val) {
        case openai_4o::ResponseFormatType::TEXT: return "text";
        case openai_4o::ResponseFormatType::JSON_OBJECT: return "json_object";
        case openai_4o::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: return "text";
    }
}


constexpr std::string_view to_string_view(openai_4o::PredictionType val) {
    switch (val) {
        case openai_4o::PredictionType::CONTENT: return "content";
        default: return "content";
    }
}


constexpr std::string_view to_string_view(openai_4o::ServiceTier val) {
    switch (val) {
        case openai_4o::ServiceTier::SCALE: return "scale";
        case openai_4o::ServiceTier::DEFAULT: return "default";
        default: return "default";
    }
}


constexpr std::string_view to_string_view(openai_4o::ObjectType val) {
    switch (val) {
        case openai_4o::ObjectType::CHAT_COMPLETION: return "chat.completion";
        default: return "chat.completion";
    }
}


}
