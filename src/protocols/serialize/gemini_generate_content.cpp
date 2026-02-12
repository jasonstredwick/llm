#include "../../interface/protocols/gemini/generate_content.hpp"
#include "base.hpp"


using namespace simdjson::builder;


namespace jai::llm::gemini {


BEGIN_SERIALIZE(Blob)
    FIELD(obj, mimeType, CommaDirection::NONE)
    FIELD(obj, data,     CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(CodeExecution)
END_SERIALIZE


BEGIN_SERIALIZE(CodeExecutionResult)
    FIELD(obj, outcome, CommaDirection::NONE)
    FIELD(obj, output,  CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(ComputerUse)
    FIELD(obj, environment,                 CommaDirection::NONE)
    FIELD(obj, excludedPredefinedFunctions, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(FileData)
    FIELD(obj, mimeType, CommaDirection::AFTER)
    FIELD(obj, fileUri,  CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(FileSearch)
    FIELD(obj, fileSearchStoreNames, CommaDirection::NONE)
    FIELD(obj, metadataFilter,       CommaDirection::BEFORE)
    FIELD(obj, topK,                 CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(FunctionDeclaration)
    FIELD(obj, name,                 CommaDirection::NONE)
    FIELD(obj, description,          CommaDirection::BEFORE)
    FIELD(obj, behavior,             CommaDirection::BEFORE)
    FIELD(obj, parameters,           CommaDirection::BEFORE)
    FIELD(obj, parametersJsonSchema, CommaDirection::BEFORE)
    FIELD(obj, response,             CommaDirection::BEFORE)
    FIELD(obj, responseJsonSchema,   CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(FunctionResponse)
    FIELD(obj, id,           CommaDirection::AFTER)
    FIELD(obj, name,         CommaDirection::NONE)
    FIELD(obj, response,     CommaDirection::BEFORE)
    FIELD(obj, parts,        CommaDirection::BEFORE)
    FIELD(obj, willContinue, CommaDirection::BEFORE)
    FIELD(obj, scheduling,   CommaDirection::BEFORE)
END_SERIALIZE


//void SerializeFrom(simdjson::builder::string_builder& builder, const FunctionResponse::Part& obj) {
BEGIN_SERIALIZE(FunctionResponse::Part)
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, Blob>) { AddKV_base<"inlineData", CommaDirection::NONE>(builder, x); }
        else {
            static_assert(always_false_v<T>,
                          "gemini::SerializeFrom: Unhandled FunctionResponse::Part variant alternative.");
        }
    }, obj);
END_SERIALIZE


// Technically all the fields are optional, but to easy the writing of this function, I chose to use the
// default provided by the API doc for responseMimeType to ensure at least on field is present.
BEGIN_SERIALIZE(GenerationConfig)
    std::optional<ResponseMimeType> mime_type =
        obj.responseMimeType.value_or(ResponseMimeType::TEXT_PLAIN);
    FIELD(obj, responseMimeType,           CommaDirection::NONE)
    FIELD(obj, stopSequences,              CommaDirection::BEFORE)
    FIELD(obj, responseSchema,             CommaDirection::BEFORE)
    FIELD(obj, _responseJsonSchema,        CommaDirection::BEFORE)
    FIELD(obj, responseJsonSchema,         CommaDirection::BEFORE)
    FIELD(obj, responseModalities,         CommaDirection::BEFORE)
    FIELD(obj, candidateCount,             CommaDirection::BEFORE)
    FIELD(obj, maxOutputTokens,            CommaDirection::BEFORE)
    FIELD(obj, temperature,                CommaDirection::BEFORE)
    FIELD(obj, topP,                       CommaDirection::BEFORE)
    FIELD(obj, topK,                       CommaDirection::BEFORE)
    FIELD(obj, seed,                       CommaDirection::BEFORE)
    FIELD(obj, presencePenalty,            CommaDirection::BEFORE)
    FIELD(obj, responseLogprobs,           CommaDirection::BEFORE)
    FIELD(obj, logprobs,                   CommaDirection::BEFORE)
    FIELD(obj, enableEnhancedCivicAnswers, CommaDirection::BEFORE)
    FIELD(obj, speechConfig,               CommaDirection::BEFORE)
    FIELD(obj, thinkingConfig,             CommaDirection::BEFORE)
    FIELD(obj, imageConfig,                CommaDirection::BEFORE)
    FIELD(obj, mediaResolution,            CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(GoogleMaps)
    FIELD(obj, enableWidget, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(GoogleSearch)
    FIELD(obj, timeRangeFilter, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(GoogleSearch::Interval)
    FIELD(obj, startTime, CommaDirection::NONE)
    if (obj.startTime && obj.endTime) { builder.append_comma(); }
    FIELD(obj, endTime,   CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(GoogleSearchRetrieval)
    FIELD(obj, dynamicRetrievalConfig, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(GoogleSearchRetrieval::Config)
    FIELD(obj, mode,             CommaDirection::NONE)
    FIELD(obj, dynamicThreshold, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(ImageConfig)
    FIELD(obj, aspectRatio, CommaDirection::NONE)
    if (obj.aspectRatio && obj.imageSize) { builder.append_comma(); }
    FIELD(obj, imageSize,   CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(Request)
    FIELD(obj, contents,          CommaDirection::NONE)
    FIELD(obj, tools,             CommaDirection::BEFORE)
    FIELD(obj, toolConfig,        CommaDirection::BEFORE)
    FIELD(obj, safetySettings,    CommaDirection::BEFORE)
    FIELD(obj, systemInstruction, CommaDirection::BEFORE)
    FIELD(obj, generationConfig,  CommaDirection::BEFORE)
    FIELD(obj, cachedContent,     CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(RequestContent)
    FIELD(obj, parts, CommaDirection::NONE)
    FIELD(obj, role,  CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(RequestContent::RequestPart::Data)
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if      constexpr (std::is_same_v<T, Blob>)                { AddKV_base<"inlineData",          CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, CodeExecutionResult>) { AddKV_base<"codeExecutionResult", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, FileData>)            { AddKV_base<"fileData",            CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, FunctionResponse>)    { AddKV_base<"functionResponse",    CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, Text>)                { AddKV_base<"text",                CommaDirection::NONE>(builder, x); }
        else { static_assert(always_false_v<T>, "gemini::SerializeFrom: Unhandled RequestContent::Part variant alternative."); }
    }, obj);
END_SERIALIZE


BEGIN_SERIALIZE(RequestContent::RequestPart)
    FIELD(obj, data,             CommaDirection::NONE)
    FIELD(obj, partMetadata,     CommaDirection::BEFORE)
    FIELD(obj, metadata,         CommaDirection::BEFORE)
    FIELD(obj, thoughtSignature, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(VideoMetadata)
    FIELD(obj, startOffset, CommaDirection::NONE)
    if (obj.startOffset && obj.endOffset) { builder.append_comma(); }
    FIELD(obj, endOffset,   CommaDirection::NONE)
    if ((obj.startOffset || obj.endOffset) && obj.fps) { builder.append_comma(); }
    FIELD(obj, fps,         CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(RequestContent::RequestPart::Metadata)
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, VideoMetadata>) {
            AddKV_base<"videoMetadata", CommaDirection::NONE>(builder, x);
        } else {
            static_assert(always_false_v<T>,
                          "gemini::SerializeFrom: Unhandled RequestContent::RequestPart::Metadata variant alternative.");
        }
    }, obj);
END_SERIALIZE


BEGIN_SERIALIZE(SafetySetting)
    FIELD(obj, category,  CommaDirection::NONE)
    FIELD(obj, threshold, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(Schema)
    FIELD(obj, type,             CommaDirection::NONE)
    FIELD(obj, format,           CommaDirection::BEFORE)
    FIELD(obj, title,            CommaDirection::BEFORE)
    FIELD(obj, description,      CommaDirection::BEFORE)
    FIELD(obj, nullable,         CommaDirection::BEFORE)
    FIELD_ALT(obj, enum_values, "enum", CommaDirection::BEFORE)
    FIELD(obj, maxItems,         CommaDirection::BEFORE)
    FIELD(obj, minItems,         CommaDirection::BEFORE)
    FIELD(obj, properties,       CommaDirection::BEFORE)
    FIELD(obj, required,         CommaDirection::BEFORE)
    FIELD(obj, minProperties,    CommaDirection::BEFORE)
    FIELD(obj, maxProperties,    CommaDirection::BEFORE)
    FIELD(obj, minLength,        CommaDirection::BEFORE)
    FIELD(obj, maxLength,        CommaDirection::BEFORE)
    FIELD(obj, pattern,          CommaDirection::BEFORE)
    FIELD(obj, example,          CommaDirection::BEFORE)
    FIELD(obj, anyOf,            CommaDirection::BEFORE)
    FIELD(obj, propertyOrdering, CommaDirection::BEFORE)
    FIELD_ALT(obj, default_value, "default", CommaDirection::BEFORE)
    FIELD(obj, items,            CommaDirection::BEFORE)
    FIELD(obj, minimum,          CommaDirection::BEFORE)
    FIELD(obj, maximum,          CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(SpeechConfig)
    FIELD(obj, voiceConfig,             CommaDirection::NONE)
    FIELD(obj, multiSpeakerVoiceConfig, CommaDirection::BEFORE)
    FIELD(obj, languageCode,            CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(SpeechConfig::MultiSpeakerVoiceConfig)
    FIELD(obj, speakerVoiceConfigs, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(SpeechConfig::PrebuiltVoiceConfig)
    FIELD(obj, voiceName, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(std::variant<SpeechConfig::PrebuiltVoiceConfig>)
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, SpeechConfig::PrebuiltVoiceConfig>) {
            AddKV_base<"prebuiltVoiceConfig", CommaDirection::NONE>(builder, x);
        } else {
            static_assert(always_false_v<T>,
                          "gemini::SerializeFrom: Unhandled SpeechConfig::VoiceConfig variant alternative.");
        }
    }, obj);
END_SERIALIZE


BEGIN_SERIALIZE(SpeechConfig::SpeakerVoiceConfig)
    FIELD(obj, speaker,     CommaDirection::NONE)
    FIELD(obj, voiceConfig, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(SpeechConfig::VoiceConfig)
    FIELD(obj, voice_config, CommaDirection::NONE);
END_SERIALIZE


BEGIN_SERIALIZE(Text)
    FIELD(obj, text, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(ThinkingConfig)
    FIELD(obj, includeThoughts, CommaDirection::NONE)
    FIELD(obj, thinkingBudget,  CommaDirection::BEFORE)
    FIELD(obj, thinkingLevel,   CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(Tool)
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if      constexpr (std::is_same_v<T, CodeExecution>)         { AddKV_base<"codeExecution",         CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, ComputerUse>)           { AddKV_base<"computerUse",           CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, FileSearch>)            { AddKV_base<"fileSearch",            CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, FunctionDeclaration>)   { AddKV_base<"functionDeclarations",  CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, GoogleMaps>)            { AddKV_base<"googleMaps",            CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, GoogleSearch>)          { AddKV_base<"googleSearch",          CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, GoogleSearchRetrieval>) { AddKV_base<"googleSearchRetrieval", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, UrlContext>)            { AddKV_base<"urlContext",            CommaDirection::NONE>(builder, x); }
        else { static_assert(always_false_v<T>, "gemini::SerializeFrom: Unhandled Tool variant alternative."); }
    }, obj);
END_SERIALIZE


BEGIN_SERIALIZE(ToolConfig)
    FIELD(obj, functionCallingConfig, CommaDirection::NONE)
    if (obj.functionCallingConfig && obj.retrievalConfig) { builder.append_comma(); }
    FIELD(obj, retrievalConfig,       CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(ToolConfig::FunctionCallingConfig)
    FIELD(obj, mode,                  CommaDirection::NONE)
    if (obj.mode && !obj.allowedFunctionNames.Value().empty()) { builder.append_comma(); }
    FIELD(obj, allowedFunctionNames, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(ToolConfig::RetrievalConfig)
    FIELD(obj, latLng,       CommaDirection::NONE)
    if (obj.latLng && obj.languageCode) { builder.append_comma(); }
    FIELD(obj, languageCode, CommaDirection::NONE)
END_SERIALIZE


BEGIN_SERIALIZE(ToolConfig::RetrievalConfig::LatLng)
    FIELD(obj, latitude,  CommaDirection::NONE)
    FIELD(obj, longitude, CommaDirection::BEFORE)
END_SERIALIZE


BEGIN_SERIALIZE(UrlContext)
END_SERIALIZE


/***
 * Top-level Serialize
 */
std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    try {
        builder.clear();
        SerializeFrom(builder, request);

        if (!builder.validate_unicode()) {
            throw AnnotatedException{"Serialize Failed", "string_builder generated invalid unicode data."};
        }

        auto result = builder.view();
        if (result.error() != simdjson::SUCCESS) {
            throw AnnotatedException{"Serialize Failed", simdjson::error_message(result.error())};
        }
        std::string_view json_str = result.value();

        return json_str |
               std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
               std::ranges::to<std::vector<std::byte>>();
    } catch (AnnotatedException const&) {
        throw;
    } catch (std::exception const& e) {
        AnnotatedException ex{"Serialize Failed", "string_builder failed to serialize Response."};
        ex.AddContext(e.what());
        throw ex;
    }
}


}
