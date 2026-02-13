#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>

#include "../../core/types.hpp"


namespace jai::llm::gemini {


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
    Required<MediaType> mimeType;
    Required<std::string> data; // base64
};


struct CodeExecutionResult {
    Required<ExecutionOutcome> outcome;
    std::optional<std::string> output{};
};


struct ExecutableCode {
    Required<CodeLanguage> language;
    Required<std::string> code;
};


struct FileData {
    std::optional<MediaType> mimeType{};
    Required<std::string> fileUri;
};


struct FunctionCall {
    std::optional<std::string> id{};
    Required<Name64> name;
    Required<json::Object> args;
};


struct FunctionResponse {
    using Part = std::variant<Blob>;

    Required<Name64> name;
    Required<json::Object> response;
    std::optional<std::string> id{};
    Required<std::vector<Part>> parts{{}};
    std::optional<bool> willContinue{};
    std::optional<Scheduling> scheduling{};
};


struct Text {
    Required<std::string> text;
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

        Required<Data> data;
        Required<json::Object> partMetadata;
        Required<Metadata> metadata;
        std::optional<std::string> thoughtSignature{};
    };

    Required<std::vector<RequestPart>> parts;
    std::optional<Role> role{};
};


struct ResponseContent {
    struct ResponsePart {
        using ResponsePartData = std::variant<Blob, ExecutableCode, FileData, FunctionCall, Text>;

        Required<ResponsePartData> data;
        Required<json::Object> partMetadata;
        std::optional<std::string> thoughtSignature{};
        std::optional<bool> thought{};
    };

    Required<std::vector<ResponsePart>> parts{{}};
    std::optional<Role> role{};
};


/***
 * Tool data structures
 */
struct Schema {
    Required<SchemaType> type;
    std::optional<std::string> format{};
    std::optional<std::string> title{};
    std::optional<std::string> description{};
    std::optional<bool> nullable{};
    std::optional<std::vector<std::string>> enum_values{};
    std::optional<Int64Str>maxItems{};
    std::optional<Int64Str>minItems{};
    std::optional<std::map<std::string, Schema>> properties{};
    std::optional<std::vector<std::string>> required{};
    std::optional<Int64Str> minProperties{};
    std::optional<Int64Str> maxProperties{};
    std::optional<Int64Str> minLength{};
    std::optional<Int64Str> maxLength{};
    std::optional<std::string> pattern{};
    std::optional<json::Value> example{};
    std::optional<std::vector<Schema>> anyOf{};
    std::optional<std::vector<std::string>> propertyOrdering{};
    std::optional<json::Value> default_value{};
    std::optional<ValueBox<Schema>> items{};
    std::optional<double> minimum{};
    std::optional<double> maximum{};
};


struct CodeExecution {};


struct ComputerUse {
    Required<Environment> environment;
    Required<std::vector<std::string>> excludedPredefinedFunctions{{}};
};


struct FileSearch {
    Required<std::vector<std::string>> fileSearchStoreNames;
    std::optional<std::string> metadataFilter{};
    std::optional<int64_t> topK{};
};


struct FunctionDeclaration {
    Required<Name64> name;
    Required<std::string> description;
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
        Required<DynamicRetrievalMode> mode;
        std::optional<double> dynamicThreshold{};
    };

    Required<Config> dynamicRetrievalConfig;
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
        Required<std::string> voiceName;
    };

    struct VoiceConfig {
        Required<std::variant<PrebuiltVoiceConfig>> voice_config;
    };

    struct SpeakerVoiceConfig {
        Required<std::string> speaker;
        Required<VoiceConfig> voiceConfig;
    };

    struct MultiSpeakerVoiceConfig {
        Required<std::vector<SpeakerVoiceConfig>> speakerVoiceConfigs;
    };

    Required<VoiceConfig> voiceConfig;
    std::optional<MultiSpeakerVoiceConfig> multiSpeakerVoiceConfig{};
    std::optional<std::string> languageCode{};
    // TODO: Add enum?
    // Valid values are: de-DE, en-AU, en-GB, en-IN, en-US, es-US, fr-FR, hi-IN, pt-BR, ar-XA, es-ES, fr-CA, id-ID,
    //                   it-IT, ja-JP, tr-TR, vi-VN, bn-IN, gu-IN, kn-IN, ml-IN, mr-IN, ta-IN, te-IN, nl-NL, ko-KR,
    //                   cmn-CN, pl-PL, ru-RU, and th-TH.
};


struct ThinkingConfig {
    Required<int64_t> thinkingBudget;
    std::optional<ThinkingLevel> thinkingLevel{};
    Required<bool> includeThoughts;
};


struct GenerationConfig {
    Required<std::vector<std::string>> stopSequences{{}};
    std::optional<ResponseMimeType> responseMimeType{};
    std::optional<Schema> responseSchema{};
    std::optional<json::Value> _responseJsonSchema{};
    std::optional<json::Value> responseJsonSchema{};
    Required<std::vector<Modality>> responseModalities{{}};
    std::optional<int64_t> candidateCount{};
    std::optional<int64_t> maxOutputTokens{};
    std::optional<double> temperature{};
    std::optional<double> topP{};
    std::optional<int64_t> topK{};
    std::optional<int64_t> seed{};
    std::optional<double> presencePenalty{};
    std::optional<bool> responseLogprobs{};
    std::optional<Int64Bounded<0, 20>> logprobs{};
    std::optional<bool> enableEnhancedCivicAnswers{};
    std::optional<SpeechConfig> speechConfig{};
    std::optional<ThinkingConfig> thinkingConfig{};
    std::optional<ImageConfig> imageConfig{};
    std::optional<MediaResolution> mediaResolution{};
};


struct SafetySetting {
    Required<HarmCategory> category;
    Required<HarmBlockThreshold> threshold;
};


struct ToolConfig {
    struct FunctionCallingConfig {
        std::optional<ToolMode> mode{};
        Required<std::vector<std::string>> allowedFunctionNames{{}};
    };

    struct RetrievalConfig {
        struct LatLng {
            Required<double> latitude;
            Required<double> longitude;
        };
        std::optional<LatLng> latLng{};
        std::optional<std::string> languageCode{};
    };

    std::optional<FunctionCallingConfig> functionCallingConfig{};
    std::optional<RetrievalConfig> retrievalConfig{};
};


struct Request {
    Required<std::vector<RequestContent>> contents;
    Required<std::vector<Tool>> tools{{}};
    std::optional<ToolConfig> toolConfig{};
    Required<std::vector<SafetySetting>> safetySettings{{}};
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

    Required<std::vector<CitationSource>> citationSources;
};


struct GroundingChunk {
    struct Maps {
        struct PlaceAnswerSources {
            struct ReviewSnippets {
                Required<std::string> reviewId;
                Required<EncodedUrl> googleMapsUri;
                Required<std::string> title;
            };

            Required<std::vector<ReviewSnippets>> reviewSnippets;
        };

        Required<EncodedUrl> uri;
        Required<std::string> title;
        Required<std::string> text;
        Required<std::string> placeId;
        Required<PlaceAnswerSources> placeAnswerSources;
    };

    struct RetrievedContext {
        std::optional<EncodedUrl> uri{};
        std::optional<std::string> title{};
        std::optional<std::string> text{};
        std::optional<std::string> fileSearchStore{};
    };

    struct Web {
        Required<EncodedUrl> uri;
        Required<std::string> title;
    };

    using ChunkType = std::variant<Maps, RetrievedContext, Web>;

    Required<ChunkType> chunk_type;
};


struct GroundingSupport {
    struct Segment {
        Required<int64_t> partIndex;
        Required<int64_t> startIndex;
        Required<int64_t> endIndex;
        Required<std::string> text;
    };

    std::optional<std::vector<int64_t>> groundingChunkIndices{};
    std::optional<std::vector<double>> confidenceScores{};
    Required<Segment> segment;
};


struct SearchEntryPoint {
    std::optional<std::string> renderedContent{};
    std::optional<std::string> sdkBlob{};
};


struct GroundingMetadata {
    struct RetrievalMetadata {
        std::optional<double> googleSearchDynamicRetrievalScore{};
    };

    Required<std::vector<GroundingChunk>> groundingChunks;
    Required<std::vector<GroundingSupport>> groundingSupports;
    Required<std::vector<std::string>> webSearchQueries;
    std::optional<SearchEntryPoint> searchEntryPoint{};
    std::optional<RetrievalMetadata> retrievalMetadata;
    std::optional<std::string> googleMapsWidgetContextToken{};
};


struct LogprobsResult {
    struct Candidate {
        Required<std::string> token;
        Required<int64_t> tokenId;
        Required<double> logProbability;
    };

    struct TopCandidate {
        Required<std::vector<LogprobsResult::Candidate>> candidates;
    };

    Required<std::vector<TopCandidate>> topCandidates;
    Required<std::vector<LogprobsResult::Candidate>> chosenCandidates;
    Required<double> logProbabilitySum;
};


struct SafetyRating {
    Required<HarmCategory> category;
    Required<HarmProbability> probability;
    Required<bool> blocked;
};


struct UrlContextMetadata {
    struct UrlMetadata {
        Required<std::string> retrievedUrl;
        Required<UrlRetrievalStatus> urlRetrievalStatus;
    };

    Required<std::vector<UrlMetadata>> urlMetadata;
};


struct Candidate {
    Required<ResponseContent> content;
    std::optional<FinishReason> finishReason{};
    Required<std::vector<SafetyRating>> safetyRatings{{}};
    Required<CitationMetadata> citationMetadata;
    Required<int64_t> tokenCount;
    Required<GroundingMetadata> groundingMetadata;
    std::optional<double> avgLogprobs;
    std::optional<LogprobsResult> logprobsResult;
    Required<UrlContextMetadata> urlContextMetadata;
    Required<int64_t> index;
    std::optional<std::string> finishMessage{};
};


struct ModelStatus {
    Required<ModelStage> modelStage;
    Required<RFC3339Timestamp> retirementTime;
    Required<std::string> message;
};


struct PromptFeedback {
    std::optional<BlockReason> blockReason{};
    Required<std::vector<SafetyRating>> safetyRatings;
};


struct UsageMetadata {
    struct ModalityTokenCount {
        Required<Modality> modality;
        Required<int64_t> tokenCount;
    };

    Required<int64_t> promptTokenCount;
    Required<int64_t> cachedContentTokenCount;
    Required<int64_t> candidatesTokenCount;
    Required<int64_t> toolUsePromptTokenCount;
    Required<int64_t> thoughtsTokenCount;
    Required<int64_t> totalTokenCount;
    Required<std::vector<ModalityTokenCount>> promptTokensDetails;
    Required<std::vector<ModalityTokenCount>> cacheTokensDetails;
    Required<std::vector<ModalityTokenCount>> candidatesTokensDetails;
    Required<std::vector<ModalityTokenCount>> toolUsePromptTokensDetails;
};


struct Response {
    Required<std::vector<Candidate>> candidates;
    Required<PromptFeedback> promptFeedback;
    Required<UsageMetadata> usageMetadata;
    Required<std::string> modelVersion;
    Required<std::string> responseId;
    Required<ModelStatus> modelStatus;
};


}
