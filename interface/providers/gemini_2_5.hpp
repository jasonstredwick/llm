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


namespace jai::llm::gemini_2_5 {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view/from_string_view conversions defined below.
 */
enum class CodeLanguage { PYTHON, UNSPECIFIED };
enum class ExecutionOutcome { DEADLINE_EXCEEDED, FAILED, OK, UNSPECIFIED };
enum class FinishReason {
    BLOCKLIST, FINISH_REASON_UNSPECIFIED, MALFORMED_FUNCTION_CALL, MAX_TOKENS,
    OTHER, PROHIBITED_CONTENT, RECITATION, SAFETY, SPII, STOP
};
enum class FunctionCallingMode { ANY, AUTO, MODE_UNSPECIFIED, NONE };
enum class HarmCategory {
    CIVIC_INTEGRITY, CIVIC_INTEGRITY_ELECTIONS, DANGEROUS_CONTENT, DEROGATORY,
    HARASSMENT, HARASSMENT_CONTENT, HATE_CONTENT, HATE_SPEECH, MEDICAL,
    MEDICAL_ADVICE, PUBLIC_SAFETY, SEXUAL, SEXUAL_CONTENT, SEXUALLY_EXPLICIT,
    TOXICITY, UNSPECIFIED, VIOLENT_CONTENT
};
enum class HarmProbability { HIGH, LOW, MEDIUM, NEGLIGIBLE, UNSPECIFIED };
enum class MediaResolution { HIGH, LOW, MEDIA_RESOLUTION_UNSPECIFIED, MEDIUM };
enum class MediaType {
    APPLICATION_PDF, AUDIO_AAC, AUDIO_FLAC, AUDIO_MP3, AUDIO_MP4, AUDIO_MPEG, AUDIO_OGG, AUDIO_WAV,
    IMAGE_BMP, IMAGE_GIF, IMAGE_JPEG, IMAGE_PNG, IMAGE_WEBP,
    VIDEO_MOV, VIDEO_MPEG, VIDEO_MP4, VIDEO_MPG, VIDEO_OGG, VIDEO_QT, VIDEO_WEBM,
    UNSPECIFIED
};
enum class Modality { AUDIO, IMAGE, TEXT, UNSPECIFIED, VIDEO };
enum class ResponseMimeType { APPLICATION_JSON, TEXT_PLAIN, TEXT_X_ENUM };
enum class Role { MODEL, SYSTEM, USER };
enum class SafetyThreshold {
    BLOCK_LOW_AND_ABOVE, BLOCK_MEDIUM_AND_ABOVE, BLOCK_NONE,
    BLOCK_ONLY_HIGH, OFF
};


/***
 * Request
 */
struct Config {
    struct ThinkingConfig {
        int64_t thinking_budget{-1};
        bool include_thoughts{false};
    };

    std::optional<double> frequency_penalty{};
    std::optional<uint32_t> logprobs{}; // Number of top candidates
    std::optional<uint64_t> max_output_tokens{};
    std::optional<MediaResolution> media_resolution{};
    std::optional<double> presence_penalty{};
    std::optional<bool> response_logprobs{};
    std::optional<ResponseMimeType> response_mime_type{};
    std::optional<std::string> response_schema{}; // OpenAPI schema as JSON string
    std::optional<uint64_t> seed{};
    std::vector<std::string> stop_sequences{};
    std::optional<double> temperature{};
    std::optional<ThinkingConfig> thinking_config{};
    std::optional<uint32_t> top_k{};
    std::optional<double> top_p{};
};


struct Content {
    struct ContentPart {
        struct CodeExecutionResult {
            ExecutionOutcome outcome{ExecutionOutcome::UNSPECIFIED};
            std::string output;
        };

        struct ExecutableCode {
            CodeLanguage language{CodeLanguage::PYTHON};
            std::string code;
        };

        struct FileData {
            MediaType mime_type{MediaType::UNSPECIFIED};
            EncodedUrl file_uri;
        };

        struct FunctionCall {
            std::string name;
            std::string args; // JSON string
        };

        struct FunctionResponse {
            std::string name;
            std::string response; // JSON string
        };
        struct InlineImagePart {
            MediaType mime_type{MediaType::UNSPECIFIED};
            std::string base64_data;
        };

        struct TextPart {
            std::string text;
        };

        struct Thought {
            std::string text;
        };
        struct ThoughtSignature {
            std::string signature;
        };

        using Part = std::variant<
            TextPart, InlineImagePart, FileData, FunctionCall,
            FunctionResponse, ExecutableCode, CodeExecutionResult, Thought, ThoughtSignature
        >;
    };

    Role role{Role::USER};
    std::vector<ContentPart::Part> parts;
};


struct SafetySetting {
    HarmCategory category{HarmCategory::UNSPECIFIED};
    SafetyThreshold threshold{SafetyThreshold::BLOCK_MEDIUM_AND_ABOVE};
};


struct Tool {
    struct CodeExecution {};

    struct FunctionDeclaration {
        std::string name;
        std::optional<std::string> description{};
        std::optional<std::string> parameters{};
    };

    struct GoogleSearch {
        struct DynamicRetrievalConfig {
            enum class Mode { MODE_UNSPECIFIED, DYNAMIC };
            Mode mode{Mode::MODE_UNSPECIFIED};
            std::optional<double> dynamic_threshold{};
        };

        std::optional<DynamicRetrievalConfig> dynamic_retrieval_config{};
    };

    std::optional<CodeExecution> code_execution{};
    std::vector<FunctionDeclaration> function_declarations{};
    std::optional<GoogleSearch> google_search{};
};


struct ToolConfig {
    struct FunctionCallingConfig {
        FunctionCallingMode mode{FunctionCallingMode::AUTO};
        std::vector<std::string> allowed_function_names{};
    };

    std::optional<FunctionCallingConfig> function_calling_config{};
};


struct Request {
    std::string model;
    std::vector<Content> contents{};
    std::optional<std::string> system_instruction{};
    std::optional<Config> config{};
    std::vector<SafetySetting> safety_settings{};
    std::vector<Tool> tools{};
    std::optional<ToolConfig> tool_config{};
    std::optional<std::string> cached_content{};
};


/***
 * Response
 */
struct CitationMetadata {
    struct CitationSource {
        uint32_t start_index{0};
        uint32_t end_index{0};
        EncodedUrl uri;
        std::string license{};
    };

    std::vector<CitationSource> citation_sources{};
};


struct GroundingMetadata {
    struct GroundingChunk {
        struct Web {
            EncodedUrl uri;
            std::string title{};
        };

        Web web;
    };

    struct GroundingSupport {
        std::vector<uint32_t> grounding_chunk_indices{};
        uint32_t segment_start_index{0};
        uint32_t segment_end_index{0};
        std::optional<std::string> segment_text{};
        double confidence_score{0.0};
    };

    struct SearchEntryPoint {
        std::string rendered_content{}; // HTML/CSS for Search UI
    };

    std::vector<GroundingChunk> grounding_chunks{};
    std::vector<std::string> web_search_queries{};
    std::vector<GroundingSupport> grounding_supports{};
    std::optional<SearchEntryPoint> search_entry_point{};
};


struct LogprobsResult {
    struct LogprobCandidate {
        std::string token;
        double log_probability{0.0};
        std::vector<uint8_t> bytes;
    };

    std::vector<LogprobCandidate> chosen_candidates{};
    std::vector<std::vector<LogprobCandidate>> top_candidates{};
};


struct SafetyRating {
    HarmCategory category{HarmCategory::UNSPECIFIED};
    HarmProbability probability{HarmProbability::UNSPECIFIED};
    bool blocked{false};
};


struct UsageMetadata {
    struct TokenCountDetails {
        uint64_t text_token_count{0};
        uint64_t image_token_count{0};
        uint64_t video_token_count{0};
        uint64_t audio_token_count{0};
    };

    uint64_t prompt_token_count{0};
    uint64_t candidates_token_count{0};
    uint64_t total_token_count{0};
    uint64_t reasoning_token_count{0};
    uint64_t cached_content_token_count{0};
    std::vector<TokenCountDetails> prompt_token_count_details{};
    std::vector<TokenCountDetails> candidates_token_count_details{};
};


struct Candidate {
    Content content;
    FinishReason finish_reason{FinishReason::FINISH_REASON_UNSPECIFIED};
    std::optional<std::string> thought{};
    std::optional<CitationMetadata> citation_metadata{};
    std::optional<GroundingMetadata> grounding_metadata{};
    std::vector<SafetyRating> safety_ratings{};
    std::optional<LogprobsResult> logprobs_result{};
    double avg_logprobs{0.0};
    uint32_t index{0};
};


struct PromptFeedback {
    struct SafetyRatingDetail {
        HarmCategory category{HarmCategory::UNSPECIFIED};
        HarmProbability probability{HarmProbability::UNSPECIFIED};
        bool blocked{false};
    };

    std::vector<SafetyRatingDetail> safety_ratings{};
    std::optional<std::string> block_reason{};
};


struct Response {
    std::vector<Candidate> candidates;
    std::optional<PromptFeedback> prompt_feedback{};
    std::optional<UsageMetadata> usage_metadata{};
    std::optional<std::string> model_version{};
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


/***
 * Vocabulary to string conversions.
 */
namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv);


template <>
constexpr std::optional<gemini_2_5::CodeLanguage> from_string_view<gemini_2_5::CodeLanguage>(std::string_view sv) {
    if (sv == "python") return gemini_2_5::CodeLanguage::PYTHON;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::ExecutionOutcome> from_string_view<gemini_2_5::ExecutionOutcome>(std::string_view sv) {
    if (sv == "OUTCOME_OK") return gemini_2_5::ExecutionOutcome::OK;
    if (sv == "OUTCOME_FAILED") return gemini_2_5::ExecutionOutcome::FAILED;
    if (sv == "OUTCOME_DEADLINE_EXCEEDED") return gemini_2_5::ExecutionOutcome::DEADLINE_EXCEEDED;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::FinishReason> from_string_view<gemini_2_5::FinishReason>(std::string_view sv) {
    if (sv == "STOP") return gemini_2_5::FinishReason::STOP;
    if (sv == "MAX_TOKENS") return gemini_2_5::FinishReason::MAX_TOKENS;
    if (sv == "SAFETY") return gemini_2_5::FinishReason::SAFETY;
    if (sv == "RECITATION") return gemini_2_5::FinishReason::RECITATION;
    if (sv == "OTHER") return gemini_2_5::FinishReason::OTHER;
    if (sv == "BLOCKLIST") return gemini_2_5::FinishReason::BLOCKLIST;
    if (sv == "PROHIBITED_CONTENT") return gemini_2_5::FinishReason::PROHIBITED_CONTENT;
    if (sv == "SPII") return gemini_2_5::FinishReason::SPII;
    if (sv == "MALFORMED_FUNCTION_CALL") return gemini_2_5::FinishReason::MALFORMED_FUNCTION_CALL;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::FunctionCallingMode> from_string_view<gemini_2_5::FunctionCallingMode>(std::string_view sv) {
    if (sv == "AUTO") return gemini_2_5::FunctionCallingMode::AUTO;
    if (sv == "ANY") return gemini_2_5::FunctionCallingMode::ANY;
    if (sv == "NONE") return gemini_2_5::FunctionCallingMode::NONE;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::HarmCategory> from_string_view<gemini_2_5::HarmCategory>(std::string_view sv) {
    if (sv == "HARM_CATEGORY_HARASSMENT") return gemini_2_5::HarmCategory::HARASSMENT;
    if (sv == "HARM_CATEGORY_HATE_SPEECH") return gemini_2_5::HarmCategory::HATE_SPEECH;
    if (sv == "HARM_CATEGORY_SEXUALLY_EXPLICIT") return gemini_2_5::HarmCategory::SEXUALLY_EXPLICIT;
    if (sv == "HARM_CATEGORY_DANGEROUS_CONTENT") return gemini_2_5::HarmCategory::DANGEROUS_CONTENT;
    if (sv == "HARM_CATEGORY_CIVIC_INTEGRITY") return gemini_2_5::HarmCategory::CIVIC_INTEGRITY;
    if (sv == "HARM_CATEGORY_MEDICAL") return gemini_2_5::HarmCategory::MEDICAL;
    if (sv == "HARM_CATEGORY_SEXUAL") return gemini_2_5::HarmCategory::SEXUAL;
    if (sv == "HARM_CATEGORY_PUBLIC_SAFETY") return gemini_2_5::HarmCategory::PUBLIC_SAFETY;
    if (sv == "HARM_CATEGORY_TOXICITY") return gemini_2_5::HarmCategory::TOXICITY;
    if (sv == "HARM_CATEGORY_DEROGATORY") return gemini_2_5::HarmCategory::DEROGATORY;
    if (sv == "HARM_CATEGORY_VIOLENT_CONTENT") return gemini_2_5::HarmCategory::VIOLENT_CONTENT;
    if (sv == "HARM_CATEGORY_SEXUAL_CONTENT") return gemini_2_5::HarmCategory::SEXUAL_CONTENT;
    if (sv == "HARM_CATEGORY_MEDICAL_ADVICE") return gemini_2_5::HarmCategory::MEDICAL_ADVICE;
    if (sv == "HARM_CATEGORY_CIVIC_INTEGRITY_ELECTIONS") return gemini_2_5::HarmCategory::CIVIC_INTEGRITY_ELECTIONS;
    if (sv == "HARM_CATEGORY_HATE_CONTENT") return gemini_2_5::HarmCategory::HATE_CONTENT;
    if (sv == "HARM_CATEGORY_HARASSMENT_CONTENT") return gemini_2_5::HarmCategory::HARASSMENT_CONTENT;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::HarmProbability> from_string_view<gemini_2_5::HarmProbability>(std::string_view sv) {
    if (sv == "NEGLIGIBLE") return gemini_2_5::HarmProbability::NEGLIGIBLE;
    if (sv == "LOW") return gemini_2_5::HarmProbability::LOW;
    if (sv == "MEDIUM") return gemini_2_5::HarmProbability::MEDIUM;
    if (sv == "HIGH") return gemini_2_5::HarmProbability::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode> from_string_view<gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode>(std::string_view sv) {
    if (sv == "DYNAMIC") return gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode::DYNAMIC;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::MediaResolution> from_string_view<gemini_2_5::MediaResolution>(std::string_view sv) {
    if (sv == "LOW") return gemini_2_5::MediaResolution::LOW;
    if (sv == "MEDIUM") return gemini_2_5::MediaResolution::MEDIUM;
    if (sv == "HIGH") return gemini_2_5::MediaResolution::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::MediaType> from_string_view<gemini_2_5::MediaType>(std::string_view sv) {
    if (sv == "application/pdf") return gemini_2_5::MediaType::APPLICATION_PDF;
    if (sv == "audio/aac") return gemini_2_5::MediaType::AUDIO_AAC;
    if (sv == "audio/flac") return gemini_2_5::MediaType::AUDIO_FLAC;
    if (sv == "audio/mp3") return gemini_2_5::MediaType::AUDIO_MP3;
    if (sv == "audio/mp4") return gemini_2_5::MediaType::AUDIO_MP4;
    if (sv == "audio/mpeg") return gemini_2_5::MediaType::AUDIO_MPEG;
    if (sv == "audio/ogg") return gemini_2_5::MediaType::AUDIO_OGG;
    if (sv == "audio/wav") return gemini_2_5::MediaType::AUDIO_WAV;
    if (sv == "image/bmp") return gemini_2_5::MediaType::IMAGE_BMP;
    if (sv == "image/gif") return gemini_2_5::MediaType::IMAGE_GIF;
    if (sv == "image/jpeg") return gemini_2_5::MediaType::IMAGE_JPEG;
    if (sv == "image/png") return gemini_2_5::MediaType::IMAGE_PNG;
    if (sv == "image/webp") return gemini_2_5::MediaType::IMAGE_WEBP;
    if (sv == "video/mov") return gemini_2_5::MediaType::VIDEO_MOV;
    if (sv == "video/mpeg") return gemini_2_5::MediaType::VIDEO_MPEG;
    if (sv == "video/mp4") return gemini_2_5::MediaType::VIDEO_MP4;
    if (sv == "video/mpg") return gemini_2_5::MediaType::VIDEO_MPG;
    if (sv == "video/ogg") return gemini_2_5::MediaType::VIDEO_OGG;
    if (sv == "video/quicktime") return gemini_2_5::MediaType::VIDEO_QT;
    if (sv == "video/webm") return gemini_2_5::MediaType::VIDEO_WEBM;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::Modality> from_string_view<gemini_2_5::Modality>(std::string_view sv) {
    if (sv == "text") return gemini_2_5::Modality::TEXT;
    if (sv == "image") return gemini_2_5::Modality::IMAGE;
    if (sv == "video") return gemini_2_5::Modality::VIDEO;
    if (sv == "audio") return gemini_2_5::Modality::AUDIO;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::ResponseMimeType> from_string_view<gemini_2_5::ResponseMimeType>(std::string_view sv) {
    if (sv == "application/json") return gemini_2_5::ResponseMimeType::APPLICATION_JSON;
    if (sv == "text/x.enum") return gemini_2_5::ResponseMimeType::TEXT_X_ENUM;
    if (sv == "text/plain") return gemini_2_5::ResponseMimeType::TEXT_PLAIN;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::SafetyThreshold> from_string_view<gemini_2_5::SafetyThreshold>(std::string_view sv) {
    if (sv == "BLOCK_LOW_AND_ABOVE") return gemini_2_5::SafetyThreshold::BLOCK_LOW_AND_ABOVE;
    if (sv == "BLOCK_MEDIUM_AND_ABOVE") return gemini_2_5::SafetyThreshold::BLOCK_MEDIUM_AND_ABOVE;
    if (sv == "BLOCK_NONE") return gemini_2_5::SafetyThreshold::BLOCK_NONE;
    if (sv == "BLOCK_ONLY_HIGH") return gemini_2_5::SafetyThreshold::BLOCK_ONLY_HIGH;
    if (sv == "OFF") return gemini_2_5::SafetyThreshold::OFF;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini_2_5::Role> from_string_view<gemini_2_5::Role>(std::string_view sv) {
    if (sv == "user") return gemini_2_5::Role::USER;
    if (sv == "model") return gemini_2_5::Role::MODEL;
    if (sv == "system") return gemini_2_5::Role::SYSTEM;
    return std::nullopt;
}


constexpr std::string_view to_string_view(gemini_2_5::CodeLanguage val) {
    switch (val) {
        case gemini_2_5::CodeLanguage::PYTHON: return "python";
        default: throw std::logic_error("invalid gemini_2_5::CodeLanguage");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::ExecutionOutcome val) {
    switch (val) {
        case gemini_2_5::ExecutionOutcome::OK: return "OUTCOME_OK";
        case gemini_2_5::ExecutionOutcome::FAILED: return "OUTCOME_FAILED";
        case gemini_2_5::ExecutionOutcome::DEADLINE_EXCEEDED: return "OUTCOME_DEADLINE_EXCEEDED";
        default: throw std::logic_error("invalid gemini_2_5::ExecutionOutcome");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::FinishReason val) {
    switch (val) {
        case gemini_2_5::FinishReason::STOP: return "STOP";
        case gemini_2_5::FinishReason::MAX_TOKENS: return "MAX_TOKENS";
        case gemini_2_5::FinishReason::SAFETY: return "SAFETY";
        case gemini_2_5::FinishReason::RECITATION: return "RECITATION";
        case gemini_2_5::FinishReason::OTHER: return "OTHER";
        case gemini_2_5::FinishReason::BLOCKLIST: return "BLOCKLIST";
        case gemini_2_5::FinishReason::PROHIBITED_CONTENT: return "PROHIBITED_CONTENT";
        case gemini_2_5::FinishReason::SPII: return "SPII";
        case gemini_2_5::FinishReason::MALFORMED_FUNCTION_CALL: return "MALFORMED_FUNCTION_CALL";
        default: throw std::logic_error("invalid gemini_2_5::FinishReason");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::FunctionCallingMode val) {
    switch (val) {
        case gemini_2_5::FunctionCallingMode::AUTO: return "AUTO";
        case gemini_2_5::FunctionCallingMode::ANY: return "ANY";
        case gemini_2_5::FunctionCallingMode::NONE: return "NONE";
        default: throw std::logic_error("invalid gemini_2_5::FunctionCallingMode");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::HarmCategory val) {
    switch (val) {
        case gemini_2_5::HarmCategory::HARASSMENT: return "HARM_CATEGORY_HARASSMENT";
        case gemini_2_5::HarmCategory::HATE_SPEECH: return "HARM_CATEGORY_HATE_SPEECH";
        case gemini_2_5::HarmCategory::SEXUALLY_EXPLICIT: return "HARM_CATEGORY_SEXUALLY_EXPLICIT";
        case gemini_2_5::HarmCategory::DANGEROUS_CONTENT: return "HARM_CATEGORY_DANGEROUS_CONTENT";
        case gemini_2_5::HarmCategory::CIVIC_INTEGRITY: return "HARM_CATEGORY_CIVIC_INTEGRITY";
        case gemini_2_5::HarmCategory::MEDICAL: return "HARM_CATEGORY_MEDICAL";
        case gemini_2_5::HarmCategory::SEXUAL: return "HARM_CATEGORY_SEXUAL";
        case gemini_2_5::HarmCategory::PUBLIC_SAFETY: return "HARM_CATEGORY_PUBLIC_SAFETY";
        case gemini_2_5::HarmCategory::TOXICITY: return "HARM_CATEGORY_TOXICITY";
        case gemini_2_5::HarmCategory::DEROGATORY: return "HARM_CATEGORY_DEROGATORY";
        case gemini_2_5::HarmCategory::VIOLENT_CONTENT: return "HARM_CATEGORY_VIOLENT_CONTENT";
        case gemini_2_5::HarmCategory::SEXUAL_CONTENT: return "HARM_CATEGORY_SEXUAL_CONTENT";
        case gemini_2_5::HarmCategory::MEDICAL_ADVICE: return "HARM_CATEGORY_MEDICAL_ADVICE";
        case gemini_2_5::HarmCategory::CIVIC_INTEGRITY_ELECTIONS: return "HARM_CATEGORY_CIVIC_INTEGRITY_ELECTIONS";
        case gemini_2_5::HarmCategory::HATE_CONTENT: return "HARM_CATEGORY_HATE_CONTENT";
        case gemini_2_5::HarmCategory::HARASSMENT_CONTENT: return "HARM_CATEGORY_HARASSMENT_CONTENT";
        default: throw std::logic_error("invalid gemini_2_5::HarmCategory");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::HarmProbability val) {
    switch (val) {
        case gemini_2_5::HarmProbability::NEGLIGIBLE: return "NEGLIGIBLE";
        case gemini_2_5::HarmProbability::LOW: return "LOW";
        case gemini_2_5::HarmProbability::MEDIUM: return "MEDIUM";
        case gemini_2_5::HarmProbability::HIGH: return "HIGH";
        default: throw std::logic_error("invalid gemini_2_5::HarmProbability");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode val) {
    switch (val) {
        case gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode::DYNAMIC: return "DYNAMIC";
        default: throw std::logic_error("invalid gemini_2_5::Tool::GoogleSearch::DynamicRetrievalConfig::Mode");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::MediaResolution val) {
    switch (val) {
        case gemini_2_5::MediaResolution::LOW: return "LOW";
        case gemini_2_5::MediaResolution::MEDIUM: return "MEDIUM";
        case gemini_2_5::MediaResolution::HIGH: return "HIGH";
        default: throw std::logic_error("invalid gemini_2_5::MediaResolution");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::MediaType val) {
    switch (val) {
        case gemini_2_5::MediaType::APPLICATION_PDF: return "application/pdf";
        case gemini_2_5::MediaType::AUDIO_AAC: return "audio/aac";
        case gemini_2_5::MediaType::AUDIO_FLAC: return "audio/flac";
        case gemini_2_5::MediaType::AUDIO_MP3: return "audio/mp3";
        case gemini_2_5::MediaType::AUDIO_MP4: return "audio/mp4";
        case gemini_2_5::MediaType::AUDIO_MPEG: return "audio/mpeg";
        case gemini_2_5::MediaType::AUDIO_OGG: return "audio/ogg";
        case gemini_2_5::MediaType::AUDIO_WAV: return "audio/wav";
        case gemini_2_5::MediaType::IMAGE_BMP: return "image/bmp";
        case gemini_2_5::MediaType::IMAGE_GIF: return "image/gif";
        case gemini_2_5::MediaType::IMAGE_JPEG: return "image/jpeg";
        case gemini_2_5::MediaType::IMAGE_PNG: return "image/png";
        case gemini_2_5::MediaType::IMAGE_WEBP: return "image/webp";
        case gemini_2_5::MediaType::VIDEO_MOV: return "video/mov";
        case gemini_2_5::MediaType::VIDEO_MPEG: return "video/mpeg";
        case gemini_2_5::MediaType::VIDEO_MP4: return "video/mp4";
        case gemini_2_5::MediaType::VIDEO_MPG: return "video/mpg";
        case gemini_2_5::MediaType::VIDEO_OGG: return "video/ogg";
        case gemini_2_5::MediaType::VIDEO_QT: return "video/quicktime";
        case gemini_2_5::MediaType::VIDEO_WEBM: return "video/webm";
        default: throw std::logic_error("invalid gemini_2_5::MediaType");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::Modality val) {
    switch (val) {
        case gemini_2_5::Modality::TEXT: return "text";
        case gemini_2_5::Modality::IMAGE: return "image";
        case gemini_2_5::Modality::VIDEO: return "video";
        case gemini_2_5::Modality::AUDIO: return "audio";
        default: throw std::logic_error("invalid gemini_2_5::Modality");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::ResponseMimeType val) {
    switch (val) {
        case gemini_2_5::ResponseMimeType::APPLICATION_JSON: return "application/json";
        case gemini_2_5::ResponseMimeType::TEXT_X_ENUM: return "text/x.enum";
        case gemini_2_5::ResponseMimeType::TEXT_PLAIN: return "text/plain";
        default: throw std::logic_error("invalid gemini_2_5::ResponseMimeType");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::SafetyThreshold val) {
    switch (val) {
        case gemini_2_5::SafetyThreshold::BLOCK_LOW_AND_ABOVE: return "BLOCK_LOW_AND_ABOVE";
        case gemini_2_5::SafetyThreshold::BLOCK_MEDIUM_AND_ABOVE: return "BLOCK_MEDIUM_AND_ABOVE";
        case gemini_2_5::SafetyThreshold::BLOCK_NONE: return "BLOCK_NONE";
        case gemini_2_5::SafetyThreshold::BLOCK_ONLY_HIGH: return "BLOCK_ONLY_HIGH";
        case gemini_2_5::SafetyThreshold::OFF: return "OFF";
        default: throw std::logic_error("invalid gemini_2_5::SafetyThreshold");
    }
}


constexpr std::string_view to_string_view(gemini_2_5::Role val) {
    switch (val) {
        case gemini_2_5::Role::USER: return "user";
        case gemini_2_5::Role::MODEL: return "model";
        case gemini_2_5::Role::SYSTEM: return "system";
        default: throw std::logic_error("invalid gemini_2_5::Role");
    }
}


}
