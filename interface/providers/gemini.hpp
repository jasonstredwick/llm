#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>

#include "../async.hpp"
#include "../policy.hpp"
#include "../types.hpp"
#include "../url.hpp"


namespace jai::llm::gemini {


/***
 * Forward declarations
 */
class Client;
class Request;
class Response;


/***
 * Vocabulary - jai::llm::to_string_view/from_string_view conversions defined in strings/gemini.hpp.
 */
enum class AspectRatio {
    ASPECT_RATIO_1_1, ASPECT_RATIO_2_3, ASPECT_RATIO_3_2, ASPECT_RATIO_3_4, ASPECT_RATIO_4_3, ASPECT_RATIO_4_5,
    ASPECT_RATIO_5_4, ASPECT_RATIO_9_16, ASPECT_RATIO_16_9, ASPECT_RATIO_21_9
};
enum class Behavior { UNSPECIFIED, BLOCKING, NON_BLOCKING };
enum class BlockReason { BLOCK_REASON_UNSPECIFIED, SAFETY, OTHER, BLOCKLIST, PROHIBITED_CONTENT, IMAGE_SAFETY };
enum class CodeLanguage { LANGUAGE_UNSPECIFIED, PYTHON };
enum class DynamicRetrievalMode { MODE_UNSPECIFIED, MODE_DYNAMIC };
enum class Environment { ENVIRONMENT_UNSPECIFIED, ENVIRONMENT_BROWSER };
enum class ExecutionOutcome { DEADLINE_EXCEEDED, FAILED, OK, UNSPECIFIED };
enum class FinishReason {
    FINISH_REASON_UNSPECIFIED, STOP, MAX_TOKENS, SAFETY, RECITATION, LANGUAGE, OTHER, BLOCKLIST, PROHIBITED_CONTENT,
    SPII, MALFORMED_FUNCTION_CALL, IMAGE_SAFETY, IMAGE_PROHIBITED_CONTENT, IMAGE_OTHER, NO_IMAGE, IMAGE_RECITATION,
    UNEXPECTED_TOOL_CALL, TOO_MANY_TOOL_CALLS, MISSING_THOUGHT_SIGNATURE
};
enum class HarmBlockThreshold {
    HARM_BLOCK_THRESHOLD_UNSPECIFIED, BLOCK_LOW_AND_ABOVE, BLOCK_MEDIUM_AND_ABOVE, BLOCK_ONLY_HIGH,
    BLOCK_NONE, OFF
};
enum class HarmCategory {
    HARM_CATEGORY_UNSPECIFIED, HARM_CATEGORY_DEROGATORY, HARM_CATEGORY_TOXICITY, HARM_CATEGORY_VIOLENCE,
    HARM_CATEGORY_SEXUAL, HARM_CATEGORY_MEDICAL, HARM_CATEGORY_DANGEROUS, HARM_CATEGORY_HARASSMENT,
    HARM_CATEGORY_HATE_SPEECH, HARM_CATEGORY_SEXUALLY_EXPLICIT, HARM_CATEGORY_DANGEROUS_CONTENT,
    HARM_CATEGORY_CIVIC_INTEGRITY
};
enum class HarmProbability { HARM_PROBABILITY_UNSPECIFIED, NEGLIGIBLE, LOW, MEDIUM, HIGH };
enum class ImageDim { IMAGE_SIZE_1K, IMAGE_SIZE_2K, IMAGE_SIZE_4K };
enum class MediaResolution {
    MEDIA_RESOLUTION_UNSPECIFIED, MEDIA_RESOLUTION_LOW, MEDIA_RESOLUTION_MEDIUM, MEDIA_RESOLUTION_HIGH
};
enum class MediaType {
    APPLICATION_PDF, AUDIO_AAC, AUDIO_FLAC, AUDIO_MP3, AUDIO_MP4, AUDIO_MPEG, AUDIO_OGG, AUDIO_WAV,
    IMAGE_BMP, IMAGE_GIF, IMAGE_JPEG, IMAGE_PNG, IMAGE_WEBP,
    VIDEO_MOV, VIDEO_MPEG, VIDEO_MP4, VIDEO_MPG, VIDEO_OGG, VIDEO_QT, VIDEO_WEBM
};
enum class Modality { MODALITY_UNSPECIFIED, TEXT, IMAGE, VIDEO, AUDIO, DOCUMENT };
enum class ModelStage { MODEL_STAGE_UNSPECIFIED, EXPERIMENTAL, PREVIEW, STABLE, LEGACY, RETIRED };
enum class ResponseMimeType { APPLICATION_JSON, TEXT_PLAIN, TEXT_X_ENUM };
enum class Role { MODEL, SYSTEM, USER };
enum class Scheduling { SCHEDULING_UNSPECIFIED, SILENT, WHEN_IDLE, INTERRUPT };
enum class SchemaType { TYPE_UNSPECIFIED, STRING, NUMBER, INTEGER, BOOLEAN, ARRAY, OBJECT, NULL_T };
enum class ThinkingLevel { THINKING_LEVEL_UNSPECIFIED, MINIMAL, LOW, MEDIUM, HIGH };
enum class ToolMode { MODE_UNSPECIFIED, AUTO, ANY, NONE, VALIDATED };
enum class UrlRetrievalStatus {
    URL_RETRIEVAL_STATUS_UNSPECIFIED, URL_RETRIEVAL_STATUS_SUCCESS, URL_RETRIEVAL_STATUS_ERROR,
    URL_RETRIEVAL_STATUS_PAYWALL, URL_RETRIEVAL_STATUS_UNSAFE
};


/***
 * Content data structures
 */
struct Blob {
    MediaType mimeType;
    std::string data; // base64
};


struct CodeExecutionResult {
    ExecutionOutcome outcome;
    std::optional<std::string> output{};
};


struct ExecutableCode {
    CodeLanguage language;
    std::string code;
};


struct FileData {
    std::optional<MediaType> mimeType{};
    std::string fileUri;
};


struct FunctionCall {
    Name64 name;
    json::Object args;
    std::optional<std::string> id{};
};


struct FunctionResponse {
    using Part = std::variant<Blob>;

    Name64 name;
    json::Object response;
    std::optional<std::string> id{};
    std::vector<Part> parts{};
    std::optional<bool> willContinue{};
    std::optional<Scheduling> scheduling{};
};


struct Text {
    std::string text;
};


struct VideoMetadata {
    std::optional<std::string> startOffset{};
    std::optional<std::string> endOffset{};
    std::optional<double> fps{};
};


struct RequestContent {
    struct RequestPart {
        using Data = std::variant<Blob, CodeExecutionResult, FileData, FunctionResponse, Text>;
        using Metadata = std::variant<VideoMetadata>;

        Data data;
        json::Object partMetadata;
        std::optional<Metadata> metadata{};
        std::optional<std::string> thoughtSignature{};
    };

    std::vector<RequestPart> parts;
    std::optional<Role> role{};
};


struct ResponseContent {
    struct ResponsePart {
        using ResponsePartData = std::variant<Blob, ExecutableCode, FileData, FunctionCall, Text>;

        ResponsePartData data;
        json::Object partMetadata;
        std::optional<std::string> thoughtSignature{};
        std::optional<bool> thought{};
    };

    std::vector<ResponsePart> parts{};
    std::optional<Role> role{};
};


/***
 * Tool data structures
 */
struct Schema {
    SchemaType type;
    std::optional<std::string> format{};
    std::optional<std::string> title{};
    std::optional<std::string> description{};
    std::optional<bool> nullable{};
    std::optional<std::vector<std::string>> enum_values{};
    std::optional<Int64>maxItems{};
    std::optional<Int64>minItems{};
    std::optional<std::map<std::string, Schema>> properties{};
    std::optional<std::vector<std::string>> required{};
    std::optional<Int64> minProperties{};
    std::optional<Int64> maxProperties{};
    std::optional<Int64> minLength{};
    std::optional<Int64> maxLength{};
    std::optional<std::string> pattern{};
    std::optional<json::Value> example{};
    std::optional<std::vector<Schema>> anyOf{};
    std::optional<std::vector<std::string>> propertyOrdering{};
    std::optional<json::Value> default_value{};
    std::optional<Schema> items{};
    std::optional<double> minimum{};
    std::optional<double> maximum{};
};


struct CodeExecution {};


struct ComputerUse {
    Environment environment;
    std::vector<std::string> excludedPredefinedFunctions{};
};


struct FileSearch {
    std::vector<std::string> fileSearchStoreNames;
    std::optional<std::string> metadataFilter{};
    std::optional<int64_t> topK{};
};


struct FunctionDeclaration {
    Name64 name;
    std::string description;
    std::optional<Behavior> behavior{};
    std::optional<Schema> parameters{};
    std::optional<json::Value> parametersJsonSchema{};
    std::optional<Schema> response{};
    std::optional<json::Value> responseJsonSchema{};
};


struct GoogleMaps {
    std::optional<bool> enableWidget{};
};


struct GoogleSearch {
    struct Interval {
        std::optional<RFC3339Timestamp> startTime{};
        std::optional<RFC3339Timestamp> endTime{};
    };

    std::optional<Interval> timeRangeFilter{};
};


struct GoogleSearchRetrieval {
    struct Config {
        DynamicRetrievalMode mode;
        std::optional<int64_t> dynamicThreshold{};
    };

    Config dynamicRetrievalConfig;
};


struct UrlContext {};


using Tool = std::variant<CodeExecution, ComputerUse, FileSearch, FunctionDeclaration, GoogleMaps, GoogleSearch,
                          GoogleSearchRetrieval, UrlContext>;


/***
 * Request
 */
struct ImageConfig {
    std::optional<AspectRatio> aspectRatio{};
    std::optional<ImageDim> imageSize{};
};


struct SpeechConfig {
    struct PrebuiltVoiceConfig{
        std::string voiceName;
    };

    struct VoiceConfig {
        std::variant<PrebuiltVoiceConfig> voice_config;
    };

    struct SpeakerVoiceConfig {
        std::string speaker;
        VoiceConfig voiceConfig;
    };

    struct MultiSpeakerVoiceConfig {
        std::vector<SpeakerVoiceConfig> speakerVoiceConfigs;
    };

    VoiceConfig voiceConfig;
    std::optional<MultiSpeakerVoiceConfig> multiSpeakerVoiceConfig{};
    std::optional<std::string> languageCode{};
    // TODO: Add enum?
    // Valid values are: de-DE, en-AU, en-GB, en-IN, en-US, es-US, fr-FR, hi-IN, pt-BR, ar-XA, es-ES, fr-CA, id-ID,
    //                   it-IT, ja-JP, tr-TR, vi-VN, bn-IN, gu-IN, kn-IN, ml-IN, mr-IN, ta-IN, te-IN, nl-NL, ko-KR,
    //                   cmn-CN, pl-PL, ru-RU, and th-TH.
};


struct ThinkingConfig {
    int64_t thinkingBudget;
    std::optional<ThinkingLevel> thinkingLevel{};
    bool includeThoughts;
};


struct GenerationConfig {
    std::vector<std::string> stopSequences{};
    std::optional<ResponseMimeType> responseMimeType{};
    std::optional<Schema> responseSchema{};
    std::optional<json::Value> _responseJsonSchema{};
    std::optional<json::Value> responseJsonSchema{};
    std::vector<Modality> responseModalities{};
    std::optional<int64_t> candidateCount{};
    std::optional<int64_t> maxOutputTokens{};
    std::optional<double> temperature{};
    std::optional<double> topP{};
    std::optional<int64_t> topK{};
    std::optional<int64_t> seed{};
    std::optional<double> presencePenalty{};
    std::optional<bool> responseLogprobs{};
    std::optional<IntN<0, 20>> logprobs{};
    std::optional<bool> enableEnhancedCivicAnswers{};
    std::optional<SpeechConfig> speechConfig{};
    std::optional<ThinkingConfig> thinkingConfig{};
    std::optional<ImageConfig> imageConfig{};
    std::optional<MediaResolution> mediaResolution{};
};


struct SafetySetting {
    HarmCategory category;
    HarmBlockThreshold threshold;
};


struct ToolConfig {
    struct FunctionCallingConfig {
        std::optional<ToolMode> mode{};
        std::vector<std::string> allowedFunctionNames{};
    };

    struct RetrievalConfig {
        struct LatLng {
            double latitude;
            double longitude;
        };
        std::optional<LatLng> latLng{};
        std::optional<std::string> languageCode{};
    };

    std::optional<FunctionCallingConfig> functionCallingConfig{};
    std::optional<RetrievalConfig> retrievalConfig{};
};


struct Request {
    std::vector<RequestContent> contents;
    std::vector<Tool> tools{};
    std::optional<ToolConfig> toolConfig{};
    std::vector<SafetySetting> safetySettings{};
    std::optional<RequestContent> systemInstruction{};
    std::optional<GenerationConfig> generationConfig{};
    std::optional<std::string> cachedContent{};
};


/***
 * Response
 */
struct CitationMetadata {
    struct CitationSource {
        std::optional<int64_t> startIndex{};
        std::optional<int64_t> endIndex{};
        std::optional<EncodedUrl> uri;
        std::optional<std::string> license{};
    };

    std::vector<CitationSource> citationSources;
};


struct GroundingChunk {
    struct Maps {
        struct PlaceAnswerSources {
            struct ReviewSnippets {
                std::string reviewId;
                EncodedUrl googleMapsUri;
                std::string title;
            };

            std::vector<ReviewSnippets> reviewSnippets;
        };

        EncodedUrl uri;
        std::string title;
        std::string text;
        std::string placeId;
        PlaceAnswerSources placeAnswerSources;
    };

    struct RetrievedContext {
        std::optional<EncodedUrl> uri{};
        std::optional<std::string> title{};
        std::optional<std::string> text{};
        std::optional<std::string> fileSearchStore{};
    };

    struct Web {
        EncodedUrl uri;
        std::string title;
    };

    using ChunkType = std::variant<Maps, RetrievedContext, Web>;

    ChunkType chunk_type;
};


struct GroundingSupport {
    struct Segment {
        int64_t partIndex;
        int64_t startIndex;
        int64_t endIndex;
        std::string text;
    };

    std::optional<std::vector<int64_t>> groundingChunkIndices{};
    std::optional<std::vector<double>> confidenceScores{};
    Segment segment;
};


struct SearchEntryPoint {
    std::optional<std::string> renderedContent{};
    std::optional<std::string> sdkBlob{};
};


struct GroundingMetadata {
    struct RetrievalMetadata {
        std::optional<double> googleSearchDynamicRetrievalScore{};
    };

    std::vector<GroundingChunk> groundingChunks;
    std::vector<GroundingSupport> groundingSupports;
    std::vector<std::string> webSearchQueries;
    std::optional<SearchEntryPoint> searchEntryPoint{};
    std::optional<RetrievalMetadata> retrievalMetadata;
    std::optional<std::string> googleMapsWidgetContextToken{};
};


struct LogprobsResult {
    struct Candidate {
        std::string token;
        int64_t tokenId;
        double logProbability;
    };

    struct TopCandidate {
        std::vector<LogprobsResult::Candidate> candidates;
    };

    std::vector<TopCandidate> topCandidates;
    std::vector<LogprobsResult::Candidate> chosenCandidates;
    double logProbabilitySum;
};


struct SafetyRating {
    HarmCategory category;
    HarmProbability probability;
    bool blocked;
};


struct UrlContextMetadata {
    struct UrlMetadata {
        std::string retrievedUrl;
        UrlRetrievalStatus urlRetrievalStatus;
    };

    std::vector<UrlMetadata> urlMetadata;
};


struct Candidate {
    ResponseContent content;
    std::optional<FinishReason> finishReason{};
    std::vector<SafetyRating> safetyRatings{};
    CitationMetadata citationMetadata;
    int64_t tokenCount;
    GroundingMetadata groundingMetadata;
    std::optional<double> avgLogprobs;
    std::optional<LogprobsResult> logprobsResult;
    UrlContextMetadata urlContextMetadata;
    int64_t index;
    std::optional<std::string> finishMessage{};
};


struct ModelStatus {
    ModelStage modelStage;
    RFC3339Timestamp retirementTime;
    std::string message;
};


struct PromptFeedback {
    std::optional<BlockReason> blockReason{};
    std::vector<SafetyRating> safetyRatings;
};


struct UsageMetadata {
    struct ModalityTokenCount {
        Modality modality;
        int64_t tokenCount;
    };

    int64_t promptTokenCount;
    int64_t cachedContentTokenCount;
    int64_t candidatesTokenCount;
    int64_t toolUsePromptTokenCount;
    int64_t thoughtsTokenCount;
    int64_t totalTokenCount;
    std::vector<ModalityTokenCount> promptTokensDetails;
    std::vector<ModalityTokenCount> cacheTokensDetails;
    std::vector<ModalityTokenCount> candidatesTokensDetails;
    std::vector<ModalityTokenCount> toolUsePromptTokensDetails;
};


struct Response {
    std::vector<Candidate> candidates;
    PromptFeedback promptFeedback;
    UsageMetadata usageMetadata;
    std::string modelVersion;
    std::string responseId;
    ModelStatus modelStatus;
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
