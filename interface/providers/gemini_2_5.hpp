#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"


namespace jai::llm::gemini_2_5 {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view conversions defined below.
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
        int32_t thinking_budget{-1};
        bool include_thoughts{false};
    };

    std::optional<double> frequency_penalty{};
    std::optional<uint32_t> logprobs{}; // Number of top candidates
    std::optional<uint32_t> max_output_tokens{};
    std::optional<MediaResolution> media_resolution{};
    std::optional<double> presence_penalty{};
    std::optional<bool> response_logprobs{};
    std::optional<ResponseMimeType> response_mime_type{};
    std::optional<std::string> response_schema{}; // OpenAPI schema as JSON string
    std::optional<uint32_t> seed{};
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
            std::string file_uri;
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

        struct ThoughtSignature {
            std::string signature;
        };

        using Part = std::variant<
            TextPart, InlineImagePart, FileData, FunctionCall,
            FunctionResponse, ExecutableCode, CodeExecutionResult, ThoughtSignature
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
        std::string uri{};
        std::string license{};
    };

    std::vector<CitationSource> citation_sources{};
};


struct GroundingMetadata {
    struct GroundingChunk {
        struct Web {
            std::string uri{};
            std::string title{};
        };

        Web web{};
    };

    struct GroundingSupport {
        std::vector<uint32_t> grounding_chunk_indices{};
        uint32_t segment_start_index{0};
        uint32_t segment_end_index{0};
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
        uint32_t text_token_count{0};
        uint32_t image_token_count{0};
        uint32_t video_token_count{0};
        uint32_t audio_token_count{0};
    };

    uint32_t prompt_token_count{0};
    uint32_t candidates_token_count{0};
    uint32_t total_token_count{0};
    uint32_t reasoning_token_count{0};
    uint32_t cached_content_token_count{0};
    std::vector<TokenCountDetails> prompt_token_count_details{};
    std::vector<TokenCountDetails> candidates_token_count_details{};
};


struct Candidate {
    Content content;
    FinishReason finish_reason{FinishReason::FINISH_REASON_UNSPECIFIED};
    std::optional<std::string> thinking_process{};
    std::optional<CitationMetadata> citation_metadata{};
    std::optional<GroundingMetadata> grounding_metadata{};
    std::vector<SafetyRating> safety_ratings{};
    std::optional<UsageMetadata> usage_metadata{};
    std::optional<LogprobsResult> logprobs_result{};
    double avg_logprobs{0.0};
};


struct PromptFeedback {
    struct SafetyRatingDetail {
        HarmCategory category{HarmCategory::UNSPECIFIED};
        HarmProbability probability{HarmProbability::UNSPECIFIED};
    };

    std::vector<SafetyRatingDetail> safety_ratings{};
    std::optional<std::string> block_reason{};
};


struct Telemetry {
    uint32_t processing_ms{0};
    std::string request_id;
    std::optional<std::string> organization{};
    std::optional<std::string> version_header{};
};


struct Response {
    std::vector<Candidate> candidates;
    std::optional<PromptFeedback> prompt_feedback{};
    std::optional<UsageMetadata> usage_metadata{};
    std::optional<std::string> model_version{};
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

    ResponseAsync GenerateTextAsync(const Request& r) const;
    Response      GenerateTextSync(const Request& r) const;
};


} // namespace jai::llm::gemini_2_5


/***
 * Vocabulary to string conversions.
 */
namespace jai::llm {


constexpr std::string_view to_string_view(gemini_2_5::CodeLanguage val) {
    switch (val) {
        case gemini_2_5::CodeLanguage::PYTHON: return "python";
        default: return "python";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::ExecutionOutcome val) {
    switch (val) {
        case gemini_2_5::ExecutionOutcome::OK: return "OUTCOME_OK";
        case gemini_2_5::ExecutionOutcome::FAILED: return "OUTCOME_FAILED";
        case gemini_2_5::ExecutionOutcome::DEADLINE_EXCEEDED: return "OUTCOME_DEADLINE_EXCEEDED";
        default: return "OUTCOME_UNSPECIFIED";
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
        default: return "FINISH_REASON_UNSPECIFIED";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::FunctionCallingMode val) {
    switch (val) {
        case gemini_2_5::FunctionCallingMode::AUTO: return "AUTO";
        case gemini_2_5::FunctionCallingMode::ANY: return "ANY";
        case gemini_2_5::FunctionCallingMode::NONE: return "NONE";
        default: return "MODE_UNSPECIFIED";
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
        default: return "";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::HarmProbability val) {
    switch (val) {
        case gemini_2_5::HarmProbability::NEGLIGIBLE: return "NEGLIGIBLE";
        case gemini_2_5::HarmProbability::LOW: return "LOW";
        case gemini_2_5::HarmProbability::MEDIUM: return "MEDIUM";
        case gemini_2_5::HarmProbability::HIGH: return "HIGH";
        default: return "UNSPECIFIED";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::MediaResolution val) {
    switch (val) {
        case gemini_2_5::MediaResolution::LOW: return "LOW";
        case gemini_2_5::MediaResolution::MEDIUM: return "MEDIUM";
        case gemini_2_5::MediaResolution::HIGH: return "HIGH";
        default: return "MEDIA_RESOLUTION_UNSPECIFIED";
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
        default: return "";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::Modality val) {
    switch (val) {
        case gemini_2_5::Modality::TEXT: return "text";
        case gemini_2_5::Modality::IMAGE: return "image";
        case gemini_2_5::Modality::VIDEO: return "video";
        case gemini_2_5::Modality::AUDIO: return "audio";
        default: return "";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::ResponseMimeType val) {
    switch (val) {
        case gemini_2_5::ResponseMimeType::APPLICATION_JSON: return "application/json";
        case gemini_2_5::ResponseMimeType::TEXT_X_ENUM: return "text/x.enum";
        case gemini_2_5::ResponseMimeType::TEXT_PLAIN:
        default: return "text/plain";
    }
}


constexpr std::string_view to_string_view(gemini_2_5::Role val) {
    switch (val) {
        case gemini_2_5::Role::USER: return "user";
        case gemini_2_5::Role::MODEL: return "model";
        case gemini_2_5::Role::SYSTEM: return "system";
        default: return "";
    }
}


} // namespace jai::llm
