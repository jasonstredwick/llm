#include "../../interface/providers/gemini.hpp"
#include "../../interface/providers/strings/gemini.hpp" // must include before base.hpp
#include "base.hpp"

#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>


using namespace simdjson;
using namespace builder;


namespace jai::llm {


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Blob& obj) {
    builder.start_object();
    builder.append_key_value<"mimeType">(obj.mimeType);
    builder.append_comma();
    builder.append_key_value<"data">(obj.data);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::CodeExecution&) {
    builder.start_object();
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ComputerUse& obj) {
    builder.start_object();
    builder.append_key_value<"environment">(obj.environment);
    AddOptKV<"excludedPredefinedFunctions", CommaDirection::BEFORE> (builder, obj.excludedPredefinedFunctions);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FileData& obj) {
    builder.start_object();
    AddOptKV<"mimeType", CommaDirection::AFTER>(builder, obj.mimeType);
    builder.append_key_value<"fileUri">(obj.fileUri);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FileSearch& obj) {
    builder.start_object();
    builder.append_key_value<"fileSearchStoreNames">(obj.fileSearchStoreNames);
    AddOptKV<"metadataFilter", CommaDirection::BEFORE>(builder, obj.metadataFilter);
    AddOptKV<"topK", CommaDirection::BEFORE>(builder, obj.topK);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FunctionDeclaration& obj) {
    builder.start_object();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"description">(obj.description);
    AddOptKV<"behavior",             CommaDirection::BEFORE>(builder, obj.behavior);
    AddOptKV<"parameters",           CommaDirection::BEFORE>(builder, obj.parameters);
    AddOptKV<"parametersJsonSchema", CommaDirection::BEFORE>(builder, obj.parametersJsonSchema);
    AddOptKV<"response",             CommaDirection::BEFORE>(builder, obj.response);
    AddOptKV<"responseJsonSchema",   CommaDirection::BEFORE>(builder, obj.responseJsonSchema);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::FunctionResponse& obj) {
    builder.start_object();
    AddOptKV<"id",           CommaDirection::AFTER>(builder, obj.id);
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"response">(obj.response);
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
        if constexpr (std::is_same_v<T, gemini::Blob>) { builder.append_key_value<"inlineData">(x); }
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
    AddOptKV<"startTime", CommaDirection::NONE>(builder, obj.startTime);
    if (obj.startTime && obj.endTime) { builder.append_comma(); }
    AddOptKV<"endTime", CommaDirection::NONE>(builder, obj.endTime);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearchRetrieval& obj) {
    builder.start_object();
    builder.append_key_value<"dynamicRetrievalConfig">(obj.dynamicRetrievalConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::GoogleSearchRetrieval::Config& obj) {
    builder.start_object();
    builder.append_key_value<"mode">(obj.mode);
    AddOptKV<"dynamicThreshold", CommaDirection::BEFORE>(builder, obj.dynamicThreshold);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ImageConfig& obj) {
    builder.start_object();
    AddOptKV<"aspectRatio", CommaDirection::NONE>(builder, obj.aspectRatio);
    if (obj.aspectRatio && obj.imageSize) { builder.append_comma(); }
    AddOptKV<"imageSize", CommaDirection::NONE>(builder, obj.imageSize);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Request& obj) {
    builder.start_object();
    builder.append_key_value<"contents">(obj.contents);
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
    builder.append_key_value<"parts">(obj.parts);
    AddOptKV<"role", CommaDirection::BEFORE>(builder, obj.role);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::RequestContent::RequestPart& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if      constexpr (std::is_same_v<T, gemini::Blob>)                { builder.append_key_value<"inlineData">(x); }
        else if constexpr (std::is_same_v<T, gemini::CodeExecutionResult>) { builder.append_key_value<"codeExecutionResult">(x); }
        else if constexpr (std::is_same_v<T, gemini::FileData>)            { builder.append_key_value<"fileData">(x); }
        else if constexpr (std::is_same_v<T, gemini::FunctionResponse>)    { builder.append_key_value<"functionResponse">(x); }
        else if constexpr (std::is_same_v<T, gemini::Text>)                { builder.append_key_value<"text">(x); }
        else { static_assert(always_false_v<T>, "tag_invoke: Unhandled RequestContent::Part variant alternative."); }
    }, obj.data);
    builder.append_comma();
    builder.append_key_value<"partMetadata">(obj.partMetadata);
    builder.append_comma();
    builder.append_key_value<"metadata">(obj.metadata);
    AddOptKV<"thoughtSignature", CommaDirection::BEFORE>(builder, obj.thoughtSignature);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::RequestContent::RequestPart::Metadata& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, gemini::VideoMetadata>) { builder.append_key_value<"videoMetadata">(x); }
        else {
            static_assert(always_false_v<T>,
                          "tag_invoke: Unhandled RequestContent::RequestPart::Metadata variant alternative.");
        }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SafetySetting& obj) {
    builder.start_object();
    builder.append_key_value<"category">(obj.category);
    builder.append_comma();
    builder.append_key_value<"threshold">(obj.threshold);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Schema& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"format",           CommaDirection::BEFORE>(builder, obj.format);
    AddOptKV<"title",            CommaDirection::BEFORE>(builder, obj.title);
    AddOptKV<"description",      CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"nullable",         CommaDirection::BEFORE>(builder, obj.nullable);
    AddOptKV<"enum",             CommaDirection::BEFORE>(builder, obj.enum_values);
    AddOptKV<"minItems",         CommaDirection::BEFORE>(builder, obj.minItems);
    AddOptKV<"maxItems",         CommaDirection::BEFORE>(builder, obj.maxItems);
    AddOptKV<"minProperties",    CommaDirection::BEFORE>(builder, obj.minProperties);
    AddOptKV<"maxProperties",    CommaDirection::BEFORE>(builder, obj.maxProperties);
    AddOptKV<"minLength",        CommaDirection::BEFORE>(builder, obj.minLength);
    AddOptKV<"maxLength",        CommaDirection::BEFORE>(builder, obj.maxLength);
    AddOptKV<"pattern",          CommaDirection::BEFORE>(builder, obj.pattern);
    AddOptKV<"properties",       CommaDirection::BEFORE>(builder, obj.properties);
    AddOptKV<"required",         CommaDirection::BEFORE>(builder, obj.required);
    AddOptKV<"propertyOrdering", CommaDirection::BEFORE>(builder, obj.propertyOrdering);
    AddOptKV<"items",            CommaDirection::BEFORE>(builder, obj.items);
    AddOptKV<"anyOf",            CommaDirection::BEFORE>(builder, obj.anyOf);
    AddOptKV<"example",          CommaDirection::BEFORE>(builder, obj.example);
    AddOptKV<"default",          CommaDirection::BEFORE>(builder, obj.default_value);
    AddOptKV<"minimum",          CommaDirection::BEFORE>(builder, obj.minimum);
    AddOptKV<"maximum",          CommaDirection::BEFORE>(builder, obj.maximum);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig& obj) {
    builder.start_object();
    builder.append_key_value<"voiceConfig">(obj.voiceConfig);
    AddOptKV<"multiSpeakerVoiceConfig", CommaDirection::BEFORE>(builder, obj.multiSpeakerVoiceConfig);
    AddOptKV<"languageCode", CommaDirection::BEFORE>(builder, obj.languageCode);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::MultiSpeakerVoiceConfig& obj) {
    builder.start_object();
    builder.append_key_value<"speakerVoiceConfigs">(obj.speakerVoiceConfigs);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::PrebuiltVoiceConfig& obj) {
    builder.start_object();
    builder.append_key_value<"voiceName">(obj.voiceName);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::SpeakerVoiceConfig& obj) {
    builder.start_object();
    builder.append_key_value<"speaker">(obj.speaker);
    builder.append_comma();
    builder.append_key_value<"voiceConfig">(obj.voiceConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::SpeechConfig::VoiceConfig& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, gemini::SpeechConfig::PrebuiltVoiceConfig>) {
            builder.append_key_value<"prebuiltVoiceConfig">(x);
        } else {
            static_assert(always_false_v<T>,
                          "tag_invoke: Unhandled SpeechConfig::VoiceConfig variant alternative.");
        }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Text& obj) {
    builder.start_object();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ThinkingConfig& obj) {
    builder.start_object();
    builder.append_key_value<"includeThoughts">(obj.includeThoughts);
    builder.append_comma();
    builder.append_key_value<"thinkingBudget">(obj.thinkingBudget);
    AddOptKV<"thinkingLevel", CommaDirection::BEFORE>(builder, obj.thinkingLevel);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::Tool& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if      constexpr (std::is_same_v<T, gemini::CodeExecution>)         { builder.append_key_value<"codeExecution">(x); }
        else if constexpr (std::is_same_v<T, gemini::ComputerUse>)           { builder.append_key_value<"computerUse">(x); }
        else if constexpr (std::is_same_v<T, gemini::FileSearch>)            { builder.append_key_value<"fileSearch">(x); }
        else if constexpr (std::is_same_v<T, gemini::FunctionDeclaration>)   { builder.append_key_value<"functionDeclarations">(x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleMaps>)            { builder.append_key_value<"googleMaps">(x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleSearch>)          { builder.append_key_value<"googleSearch">(x); }
        else if constexpr (std::is_same_v<T, gemini::GoogleSearchRetrieval>) { builder.append_key_value<"googleSearchRetrieval">(x); }
        else if constexpr (std::is_same_v<T, gemini::UrlContext>)            { builder.append_key_value<"urlContext">(x); }
        else { static_assert(always_false_v<T>, "tag_invoke: Unhandled Tool variant alternative."); }
    }, obj);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig& obj) {
    builder.start_object();
    AddOptKV<"functionCallingConfig", CommaDirection::NONE>(builder, obj.functionCallingConfig);
    if (obj.functionCallingConfig && obj.retrievalConfig) { builder.append_comma(); }
    AddOptKV<"retrievalConfig", CommaDirection::NONE>(builder, obj.retrievalConfig);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::FunctionCallingConfig& obj) {
    builder.start_object();
    AddOptKV<"mode", CommaDirection::NONE>(builder, obj.mode);
    if (obj.mode && !obj.allowedFunctionNames.empty()) { builder.append_comma(); }
    AddOptKV<"allowedFunctionNames", CommaDirection::NONE>(builder, obj.allowedFunctionNames);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::RetrievalConfig& obj) {
    builder.start_object();
    AddOptKV<"latLng", CommaDirection::NONE>(builder, obj.latLng);
    if (obj.latLng && obj.languageCode) { builder.append_comma(); }
    AddOptKV<"languageCode", CommaDirection::NONE>(builder, obj.languageCode);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::ToolConfig::RetrievalConfig::LatLng& obj) {
    builder.start_object();
    builder.append_key_value<"latitude">(obj.latitude);
    builder.append_comma();
    builder.append_key_value<"longitude">(obj.longitude);
    builder.end_object();
}


void tag_invoke(serialize_tag, string_builder& builder, const gemini::UrlContext&) {
    builder.start_object();
    builder.end_object();
}


}


namespace jai::llm::gemini {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local simdjson::builder::string_builder builder{};

    jai::llm::tag_invoke(serialize_tag{}, builder, request);
    builder.validate_unicode();
    std::string_view json_str = builder.view();

    return json_str |
           std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
           std::ranges::to<std::vector<std::byte>>();
}


}
