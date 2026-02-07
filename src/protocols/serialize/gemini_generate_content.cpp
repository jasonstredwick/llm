#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>

#include "../../interface/protocols/gemini/generate_content.hpp"
#include "base.hpp"


using namespace simdjson::builder;
using namespace jai::llm;


namespace simdjson {


TAG_ENUM(gemini::AspectRatio)
TAG_ENUM(gemini::Behavior)
TAG_ENUM(gemini::BlockReason)
TAG_ENUM(gemini::CodeLanguage)
TAG_ENUM(gemini::DynamicRetrievalMode)
TAG_ENUM(gemini::Environment)
TAG_ENUM(gemini::ExecutionOutcome)
TAG_ENUM(gemini::FinishReason)
TAG_ENUM(gemini::HarmBlockThreshold)
TAG_ENUM(gemini::HarmCategory)
TAG_ENUM(gemini::HarmProbability)
TAG_ENUM(gemini::ImageDim)
TAG_ENUM(gemini::MediaResolution)
TAG_ENUM(gemini::MediaType)
TAG_ENUM(gemini::Modality)
TAG_ENUM(gemini::ModelStage)
TAG_ENUM(gemini::ResponseMimeType)
TAG_ENUM(gemini::Role)
TAG_ENUM(gemini::Scheduling)
TAG_ENUM(gemini::SchemaType)
TAG_ENUM(gemini::ThinkingLevel)
TAG_ENUM(gemini::ToolMode)
TAG_ENUM(gemini::UrlRetrievalStatus)


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Blob& obj) {
    builder.start_object();
    AddReqKV<"mimeType", CommaDirection::NONE>  (builder, obj.mimeType);
    AddReqKV<"data",     CommaDirection::BEFORE>(builder, obj.data);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::CodeExecution&) {
    builder.start_object();
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::CodeExecutionResult& obj) {
    builder.start_object();
    AddReqKV<"outcome", CommaDirection::NONE>  (builder, obj.outcome);
    AddOptKV<"output",  CommaDirection::BEFORE>(builder, obj.output);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ComputerUse& obj) {
    builder.start_object();
    AddReqKV<"environment",                 CommaDirection::NONE>  (builder, obj.environment);
    AddOptKV<"excludedPredefinedFunctions", CommaDirection::BEFORE>(builder, obj.excludedPredefinedFunctions);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FileData& obj) {
    builder.start_object();
    AddOptKV<"mimeType", CommaDirection::AFTER>(builder, obj.mimeType);
    AddReqKV<"fileUri",  CommaDirection::NONE> (builder, obj.fileUri);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FileSearch& obj) {
    builder.start_object();
    AddReqKV<"fileSearchStoreNames", CommaDirection::NONE>  (builder, obj.fileSearchStoreNames);
    AddOptKV<"metadataFilter",       CommaDirection::BEFORE>(builder, obj.metadataFilter);
    AddOptKV<"topK",                 CommaDirection::BEFORE>(builder, obj.topK);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FunctionDeclaration& obj) {
    builder.start_object();
    AddReqKV<"name",                 CommaDirection::NONE>  (builder, obj.name);
    AddReqKV<"description",          CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"behavior",             CommaDirection::BEFORE>(builder, obj.behavior);
    AddOptKV<"parameters",           CommaDirection::BEFORE>(builder, obj.parameters);
    AddOptKV<"parametersJsonSchema", CommaDirection::BEFORE>(builder, obj.parametersJsonSchema);
    AddOptKV<"response",             CommaDirection::BEFORE>(builder, obj.response);
    AddOptKV<"responseJsonSchema",   CommaDirection::BEFORE>(builder, obj.responseJsonSchema);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FunctionResponse& obj) {
    builder.start_object();
    AddOptKV<"id",           CommaDirection::AFTER> (builder, obj.id);
    AddReqKV<"name",         CommaDirection::NONE>  (builder, obj.name);
    AddReqKV<"response",     CommaDirection::BEFORE>(builder, obj.response);
    AddOptKV<"parts",        CommaDirection::BEFORE>(builder, obj.parts);
    AddOptKV<"willContinue", CommaDirection::BEFORE>(builder, obj.willContinue);
    AddOptKV<"scheduling",   CommaDirection::BEFORE>(builder, obj.scheduling);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FunctionResponse::Part& obj)
{
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, gemini::Blob>) { AddReqKV<"inlineData", CommaDirection::NONE>(builder, x); }
        else { static_assert(always_false_v<T>, "tag_invoke: Unhandled FunctionResponse::Part variant alternative."); }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GenerationConfig& obj) {
    // Technically all the fields are optional, but to easy the writing of this function, I chose to use the
    // default provided by the API doc for responseMimeType to ensure at least on field is present.
    builder.start_object();
    std::optional<gemini::ResponseMimeType> mime_type =
        obj.responseMimeType.value_or(gemini::ResponseMimeType::TEXT_PLAIN);
    AddOptKV<"responseMimeType",           CommaDirection::NONE>  (builder, mime_type);
    AddOptKV<"stopSequences",              CommaDirection::BEFORE>(builder, obj.stopSequences);
    AddOptKV<"responseSchema",             CommaDirection::BEFORE>(builder, obj.responseSchema);
    AddOptKV<"_responseJsonSchema",        CommaDirection::BEFORE>(builder, obj._responseJsonSchema);
    AddOptKV<"responseJsonSchema",         CommaDirection::BEFORE>(builder, obj.responseJsonSchema);
    AddOptKV<"responseModalities",         CommaDirection::BEFORE>(builder, obj.responseModalities);
    AddOptKV<"candidateCount",             CommaDirection::BEFORE>(builder, obj.candidateCount);
    AddOptKV<"maxOutputTokens",            CommaDirection::BEFORE>(builder, obj.maxOutputTokens);
    AddOptKV<"temperature",                CommaDirection::BEFORE>(builder, obj.temperature);
    AddOptKV<"topP",                       CommaDirection::BEFORE>(builder, obj.topP);
    AddOptKV<"topK",                       CommaDirection::BEFORE>(builder, obj.topK);
    AddOptKV<"seed",                       CommaDirection::BEFORE>(builder, obj.seed);
    AddOptKV<"presencePenalty",            CommaDirection::BEFORE>(builder, obj.presencePenalty);
    AddOptKV<"responseLogprobs",           CommaDirection::BEFORE>(builder, obj.responseLogprobs);
    AddOptKV<"logprobs",                   CommaDirection::BEFORE>(builder, obj.logprobs);
    AddOptKV<"enableEnhancedCivicAnswers", CommaDirection::BEFORE>(builder, obj.enableEnhancedCivicAnswers);
    AddOptKV<"speechConfig",               CommaDirection::BEFORE>(builder, obj.speechConfig);
    AddOptKV<"thinkingConfig",             CommaDirection::BEFORE>(builder, obj.thinkingConfig);
    AddOptKV<"imageConfig",                CommaDirection::BEFORE>(builder, obj.imageConfig);
    AddOptKV<"mediaResolution",            CommaDirection::BEFORE>(builder, obj.mediaResolution);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleMaps& obj) {
    builder.start_object();
    AddOptKV<"enableWidget", CommaDirection::NONE>(builder, obj.enableWidget);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearch& obj) {
    builder.start_object();
    AddOptKV<"timeRangeFilter", CommaDirection::NONE>(builder, obj.timeRangeFilter);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearch::Interval& obj) {
    builder.start_object();
    AddOptKV<"startTime", CommaDirection::NONE>  (builder, obj.startTime);
    if (obj.startTime && obj.endTime) { builder.append_comma(); }
    AddOptKV<"endTime",   CommaDirection::NONE>  (builder, obj.endTime);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearchRetrieval& obj) {
    builder.start_object();
    AddReqKV<"dynamicRetrievalConfig", CommaDirection::NONE>(builder, obj.dynamicRetrievalConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearchRetrieval::Config& obj) {
    builder.start_object();
    AddReqKV<"mode",             CommaDirection::NONE>  (builder, obj.mode);
    AddOptKV<"dynamicThreshold", CommaDirection::BEFORE>(builder, obj.dynamicThreshold);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ImageConfig& obj) {
    builder.start_object();
    AddOptKV<"aspectRatio", CommaDirection::NONE>  (builder, obj.aspectRatio);
    if (obj.aspectRatio && obj.imageSize) { builder.append_comma(); }
    AddOptKV<"imageSize",   CommaDirection::NONE>  (builder, obj.imageSize);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Request& obj) {
    builder.start_object();
    AddReqKV<"contents",          CommaDirection::NONE>  (builder, obj.contents);
    AddOptKV<"tools",             CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"toolConfig",        CommaDirection::BEFORE>(builder, obj.toolConfig);
    AddOptKV<"safetySettings",    CommaDirection::BEFORE>(builder, obj.safetySettings);
    AddOptKV<"systemInstruction", CommaDirection::BEFORE>(builder, obj.systemInstruction);
    AddOptKV<"generationConfig",  CommaDirection::BEFORE>(builder, obj.generationConfig);
    AddOptKV<"cachedContent",     CommaDirection::BEFORE>(builder, obj.cachedContent);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::RequestContent& obj) {
    builder.start_object();
    AddReqKV<"parts", CommaDirection::NONE>  (builder, obj.parts);
    AddOptKV<"role",  CommaDirection::BEFORE>(builder, obj.role);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::RequestContent::RequestPart& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if      constexpr (std::is_same_v<T, gemini::Blob>)                { AddReqKV<"inlineData",          CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::CodeExecutionResult>) { AddReqKV<"codeExecutionResult", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::FileData>)            { AddReqKV<"fileData",            CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::FunctionResponse>)    { AddReqKV<"functionResponse",    CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::Text>)                { AddReqKV<"text",                CommaDirection::NONE>(builder, x); }
        else { static_assert(always_false_v<T>, "tag_invoke: Unhandled RequestContent::Part variant alternative."); }
    }, obj.data);
    AddReqKV<"partMetadata",     CommaDirection::BEFORE>(builder, obj.partMetadata);
    AddReqKV<"metadata",         CommaDirection::BEFORE>(builder, obj.metadata);
    AddOptKV<"thoughtSignature", CommaDirection::BEFORE>(builder, obj.thoughtSignature);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::VideoMetadata& obj) {
    builder.start_object();
    AddOptKV<"startOffset", CommaDirection::NONE>  (builder, obj.startOffset);
    if (obj.startOffset && obj.endOffset) { builder.append_comma(); }
    AddOptKV<"endOffset",   CommaDirection::NONE>  (builder, obj.endOffset);
    if ((obj.startOffset || obj.endOffset) && obj.fps) { builder.append_comma(); }
    AddOptKV<"fps",         CommaDirection::NONE>  (builder, obj.fps);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::RequestContent::RequestPart::Metadata& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, gemini::VideoMetadata>) { AddReqKV<"videoMetadata", CommaDirection::NONE>(builder, x); }
        else {
            static_assert(always_false_v<T>,
                          "tag_invoke: Unhandled RequestContent::RequestPart::Metadata variant alternative.");
        }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SafetySetting& obj) {
    builder.start_object();
    AddReqKV<"category",  CommaDirection::NONE>  (builder, obj.category);
    AddReqKV<"threshold", CommaDirection::BEFORE>(builder, obj.threshold);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Schema& obj) {
    builder.start_object();
    AddReqKV<"type",             CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"format",           CommaDirection::BEFORE>(builder, obj.format);
    AddOptKV<"title",            CommaDirection::BEFORE>(builder, obj.title);
    AddOptKV<"description",      CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"nullable",         CommaDirection::BEFORE>(builder, obj.nullable);
    AddOptKV<"enum",             CommaDirection::BEFORE>(builder, obj.enum_values);
    AddOptKV<"maxItems",         CommaDirection::BEFORE>(builder, obj.maxItems);
    AddOptKV<"minItems",         CommaDirection::BEFORE>(builder, obj.minItems);
    AddOptKV<"properties",       CommaDirection::BEFORE>(builder, obj.properties);
    AddOptKV<"required",         CommaDirection::BEFORE>(builder, obj.required);
    AddOptKV<"minProperties",    CommaDirection::BEFORE>(builder, obj.minProperties);
    AddOptKV<"maxProperties",    CommaDirection::BEFORE>(builder, obj.maxProperties);
    AddOptKV<"minLength",        CommaDirection::BEFORE>(builder, obj.minLength);
    AddOptKV<"maxLength",        CommaDirection::BEFORE>(builder, obj.maxLength);
    AddOptKV<"pattern",          CommaDirection::BEFORE>(builder, obj.pattern);
    AddOptKV<"example",          CommaDirection::BEFORE>(builder, obj.example);
    AddOptKV<"anyOf",            CommaDirection::BEFORE>(builder, obj.anyOf);
    AddOptKV<"propertyOrdering", CommaDirection::BEFORE>(builder, obj.propertyOrdering);
    AddOptKV<"default",          CommaDirection::BEFORE>(builder, obj.default_value);
    AddOptKV<"items",            CommaDirection::BEFORE>(builder, obj.items);
    AddOptKV<"minimum",          CommaDirection::BEFORE>(builder, obj.minimum);
    AddOptKV<"maximum",          CommaDirection::BEFORE>(builder, obj.maximum);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig& obj) {
    builder.start_object();
    AddReqKV<"voiceConfig",             CommaDirection::NONE>  (builder, obj.voiceConfig);
    AddOptKV<"multiSpeakerVoiceConfig", CommaDirection::BEFORE>(builder, obj.multiSpeakerVoiceConfig);
    AddOptKV<"languageCode",            CommaDirection::BEFORE>(builder, obj.languageCode);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::MultiSpeakerVoiceConfig& obj) {
    builder.start_object();
    AddReqKV<"speakerVoiceConfigs", CommaDirection::NONE>(builder, obj.speakerVoiceConfigs);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::PrebuiltVoiceConfig& obj) {
    builder.start_object();
    AddReqKV<"voiceName", CommaDirection::NONE>(builder, obj.voiceName);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::SpeakerVoiceConfig& obj) {
    builder.start_object();
    AddReqKV<"speaker",     CommaDirection::NONE>  (builder, obj.speaker);
    AddReqKV<"voiceConfig", CommaDirection::BEFORE>(builder, obj.voiceConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::VoiceConfig& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, gemini::SpeechConfig::PrebuiltVoiceConfig>) {
            AddReqKV<"prebuiltVoiceConfig", CommaDirection::NONE>(builder, x);
        } else {
            static_assert(always_false_v<T>,
                          "tag_invoke: Unhandled SpeechConfig::VoiceConfig variant alternative.");
        }
    }, obj.voice_config);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Text& obj) {
    builder.start_object();
    AddReqKV<"text", CommaDirection::NONE>(builder, obj.text);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ThinkingConfig& obj) {
    builder.start_object();
    AddReqKV<"includeThoughts", CommaDirection::NONE>  (builder, obj.includeThoughts);
    AddReqKV<"thinkingBudget",  CommaDirection::BEFORE>(builder, obj.thinkingBudget);
    AddOptKV<"thinkingLevel",   CommaDirection::BEFORE>(builder, obj.thinkingLevel);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Tool& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if      constexpr (std::is_same_v<T, gemini::CodeExecution>)         { AddReqKV<"codeExecution", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::ComputerUse>)           { AddReqKV<"computerUse", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::FileSearch>)            { AddReqKV<"fileSearch", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::FunctionDeclaration>)   { AddReqKV<"functionDeclarations", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleMaps>)            { AddReqKV<"googleMaps", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleSearch>)          { AddReqKV<"googleSearch", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleSearchRetrieval>) { AddReqKV<"googleSearchRetrieval", CommaDirection::NONE>(builder, x); }
        else if constexpr (std::is_same_v<T, gemini::UrlContext>)            { AddReqKV<"urlContext", CommaDirection::NONE>(builder, x); }
        else { static_assert(always_false_v<T>, "tag_invoke: Unhandled Tool variant alternative."); }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig& obj) {
    builder.start_object();
    AddOptKV<"functionCallingConfig", CommaDirection::NONE>(builder, obj.functionCallingConfig);
    if (obj.functionCallingConfig && obj.retrievalConfig) { builder.append_comma(); }
    AddOptKV<"retrievalConfig",       CommaDirection::NONE>(builder, obj.retrievalConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::FunctionCallingConfig& obj) {
    builder.start_object();
    AddOptKV<"mode",                  CommaDirection::NONE>(builder, obj.mode);
    if (obj.mode && !obj.allowedFunctionNames.empty()) { builder.append_comma(); }
    AddOptKV<"allowedFunctionNames", CommaDirection::NONE>(builder, obj.allowedFunctionNames);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::RetrievalConfig& obj) {
    builder.start_object();
    AddOptKV<"latLng",       CommaDirection::NONE>(builder, obj.latLng);
    if (obj.latLng && obj.languageCode) { builder.append_comma(); }
    AddOptKV<"languageCode", CommaDirection::NONE>(builder, obj.languageCode);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::RetrievalConfig::LatLng& obj) {
    builder.start_object();
    AddReqKV<"latitude",  CommaDirection::NONE>  (builder, obj.latitude);
    AddReqKV<"longitude", CommaDirection::BEFORE>(builder, obj.longitude);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::UrlContext&) {
    builder.start_object();
    builder.end_object();
}


} // namespace jai::llm


#undef TAG_ENUM
#undef KIND_ENUM


/***
 * Top-level Serialize
 */
namespace jai::llm::gemini {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    try {
        builder.clear();
        simdjson::tag_invoke(simdjson::serialize_tag{}, builder, request);

        if (!builder.validate_unicode()) {
            throw AnnotatedException{"gemini::Serialize Failed", "string_builder generated invalid unicode data."};
        }

        auto result = builder.view();
        if (result.error() != simdjson::SUCCESS) {
            throw AnnotatedException{"gemini::Serialize Failed", simdjson::error_message(result.error())};
        }
        std::string_view json_str = result.value();

        return json_str |
               std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
               std::ranges::to<std::vector<std::byte>>();
    } catch (AnnotatedException const&) {
        throw;
    } catch (std::exception const& e) {
        AnnotatedException ex{"gemini::Serialize Failed", "string_builder failed to serialize gemini::Response."};
        ex.AddContext(e.what());
        throw ex;
    }
}


}
