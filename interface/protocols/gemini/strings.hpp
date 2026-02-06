#pragma once

#include "../../core/strings_base.hpp"
#include "../../core/error.hpp"
#include "generate_content.hpp"


namespace jai::llm {


template <>
constexpr std::optional<gemini::AspectRatio> from_string_view<gemini::AspectRatio>(std::string_view sv) {
    if (sv == "1:1") return gemini::AspectRatio::ASPECT_RATIO_1_1;
    if (sv == "2:3") return gemini::AspectRatio::ASPECT_RATIO_2_3;
    if (sv == "3:2") return gemini::AspectRatio::ASPECT_RATIO_3_2;
    if (sv == "3:4") return gemini::AspectRatio::ASPECT_RATIO_3_4;
    if (sv == "4:3") return gemini::AspectRatio::ASPECT_RATIO_4_3;
    if (sv == "4:5") return gemini::AspectRatio::ASPECT_RATIO_4_5;
    if (sv == "5:4") return gemini::AspectRatio::ASPECT_RATIO_5_4;
    if (sv == "9:16") return gemini::AspectRatio::ASPECT_RATIO_9_16;
    if (sv == "16:9") return gemini::AspectRatio::ASPECT_RATIO_16_9;
    if (sv == "21:9") return gemini::AspectRatio::ASPECT_RATIO_21_9;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::Behavior> from_string_view<gemini::Behavior>(std::string_view sv) {
    if (sv == "UNSPECIFIED") return gemini::Behavior::UNSPECIFIED;
    if (sv == "BLOCKING") return gemini::Behavior::BLOCKING;
    if (sv == "NON_BLOCKING") return gemini::Behavior::NON_BLOCKING;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::BlockReason> from_string_view<gemini::BlockReason>(std::string_view sv) {
    if (sv == "BLOCK_REASON_UNSPECIFIED") return gemini::BlockReason::BLOCK_REASON_UNSPECIFIED;
    if (sv == "SAFETY") return gemini::BlockReason::SAFETY;
    if (sv == "OTHER") return gemini::BlockReason::OTHER;
    if (sv == "BLOCKLIST") return gemini::BlockReason::BLOCKLIST;
    if (sv == "PROHIBITED_CONTENT") return gemini::BlockReason::PROHIBITED_CONTENT;
    if (sv == "NON_BLOCKING") return gemini::BlockReason::IMAGE_SAFETY;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::CodeLanguage> from_string_view<gemini::CodeLanguage>(std::string_view sv) {
    if (sv == "LANGUAGE_UNSPECIFIED") return gemini::CodeLanguage::LANGUAGE_UNSPECIFIED;
    if (sv == "PYTHON") return gemini::CodeLanguage::PYTHON;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::DynamicRetrievalMode> from_string_view<gemini::DynamicRetrievalMode>(std::string_view sv) {
    if (sv == "MODE_UNSPECIFIED") return gemini::DynamicRetrievalMode::MODE_UNSPECIFIED;
    if (sv == "MODE_DYNAMIC") return gemini::DynamicRetrievalMode::MODE_DYNAMIC;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::Environment> from_string_view<gemini::Environment>(std::string_view sv) {
    if (sv == "ENVIRONMENT_UNSPECIFIED") return gemini::Environment::ENVIRONMENT_UNSPECIFIED;
    if (sv == "ENVIRONMENT_BROWSER") return gemini::Environment::ENVIRONMENT_BROWSER;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ExecutionOutcome> from_string_view<gemini::ExecutionOutcome>(std::string_view sv) {
    if (sv == "OUTCOME_OK") return gemini::ExecutionOutcome::OK;
    if (sv == "OUTCOME_FAILED") return gemini::ExecutionOutcome::FAILED;
    if (sv == "OUTCOME_DEADLINE_EXCEEDED") return gemini::ExecutionOutcome::DEADLINE_EXCEEDED;
    if (sv == "OUTCOME_UNSPECIFIED") return gemini::ExecutionOutcome::UNSPECIFIED;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::FinishReason> from_string_view<gemini::FinishReason>(std::string_view sv) {
    if (sv == "FINISH_REASON_UNSPECIFIED") return gemini::FinishReason::FINISH_REASON_UNSPECIFIED;
    if (sv == "STOP") return gemini::FinishReason::STOP;
    if (sv == "MAX_TOKENS") return gemini::FinishReason::MAX_TOKENS;
    if (sv == "SAFETY") return gemini::FinishReason::SAFETY;
    if (sv == "RECITATION") return gemini::FinishReason::RECITATION;
    if (sv == "LANGUAGE") return gemini::FinishReason::LANGUAGE;
    if (sv == "OTHER") return gemini::FinishReason::OTHER;
    if (sv == "BLOCKLIST") return gemini::FinishReason::BLOCKLIST;
    if (sv == "PROHIBITED_CONTENT") return gemini::FinishReason::PROHIBITED_CONTENT;
    if (sv == "SPII") return gemini::FinishReason::SPII;
    if (sv == "MALFORMED_FUNCTION_CALL") return gemini::FinishReason::MALFORMED_FUNCTION_CALL;
    if (sv == "IMAGE_SAFETY") return gemini::FinishReason::IMAGE_SAFETY;
    if (sv == "IMAGE_PROHIBITED_CONTENT") return gemini::FinishReason::IMAGE_PROHIBITED_CONTENT;
    if (sv == "IMAGE_OTHER") return gemini::FinishReason::IMAGE_OTHER;
    if (sv == "NO_IMAGE") return gemini::FinishReason::NO_IMAGE;
    if (sv == "IMAGE_RECITATION") return gemini::FinishReason::IMAGE_RECITATION;
    if (sv == "UNEXPECTED_TOOL_CALL") return gemini::FinishReason::UNEXPECTED_TOOL_CALL;
    if (sv == "TOO_MANY_TOOL_CALLS") return gemini::FinishReason::TOO_MANY_TOOL_CALLS;
    if (sv == "MISSING_THOUGHT_SIGNATURE") return gemini::FinishReason::MISSING_THOUGHT_SIGNATURE;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::HarmCategory> from_string_view<gemini::HarmCategory>(std::string_view sv) {
    if (sv == "HARM_CATEGORY_UNSPECIFIED") return gemini::HarmCategory::HARM_CATEGORY_UNSPECIFIED;
    if (sv == "HARM_CATEGORY_DEROGATORY") return gemini::HarmCategory::HARM_CATEGORY_DEROGATORY;
    if (sv == "HARM_CATEGORY_TOXICITY") return gemini::HarmCategory::HARM_CATEGORY_TOXICITY;
    if (sv == "HARM_CATEGORY_VIOLENCE") return gemini::HarmCategory::HARM_CATEGORY_VIOLENCE;
    if (sv == "HARM_CATEGORY_SEXUAL") return gemini::HarmCategory::HARM_CATEGORY_SEXUAL;
    if (sv == "HARM_CATEGORY_MEDICAL") return gemini::HarmCategory::HARM_CATEGORY_MEDICAL;
    if (sv == "HARM_CATEGORY_DANGEROUS") return gemini::HarmCategory::HARM_CATEGORY_DANGEROUS;
    if (sv == "HARM_CATEGORY_HARASSMENT") return gemini::HarmCategory::HARM_CATEGORY_HARASSMENT;
    if (sv == "HARM_CATEGORY_HATE_SPEECH") return gemini::HarmCategory::HARM_CATEGORY_HATE_SPEECH;
    if (sv == "HARM_CATEGORY_SEXUALLY_EXPLICIT") return gemini::HarmCategory::HARM_CATEGORY_SEXUALLY_EXPLICIT;
    if (sv == "HARM_CATEGORY_DANGEROUS_CONTENT") return gemini::HarmCategory::HARM_CATEGORY_DANGEROUS_CONTENT;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::HarmProbability> from_string_view<gemini::HarmProbability>(std::string_view sv) {
    if (sv == "HARM_PROBABILITY_UNSPECIFIED") return gemini::HarmProbability::HARM_PROBABILITY_UNSPECIFIED;
    if (sv == "NEGLIGIBLE") return gemini::HarmProbability::NEGLIGIBLE;
    if (sv == "LOW") return gemini::HarmProbability::LOW;
    if (sv == "MEDIUM") return gemini::HarmProbability::MEDIUM;
    if (sv == "HIGH") return gemini::HarmProbability::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ImageDim> from_string_view<gemini::ImageDim>(std::string_view sv) {
    if (sv == "1K") return gemini::ImageDim::IMAGE_SIZE_1K;
    if (sv == "2K") return gemini::ImageDim::IMAGE_SIZE_2K;
    if (sv == "4K") return gemini::ImageDim::IMAGE_SIZE_4K;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::MediaResolution> from_string_view<gemini::MediaResolution>(std::string_view sv) {
    if (sv == "MEDIA_RESOLUTION_UNSPECIFIED") return gemini::MediaResolution::MEDIA_RESOLUTION_UNSPECIFIED;
    if (sv == "MEDIA_RESOLUTION_LOW") return gemini::MediaResolution::MEDIA_RESOLUTION_LOW;
    if (sv == "MEDIA_RESOLUTION_MEDIUM") return gemini::MediaResolution::MEDIA_RESOLUTION_MEDIUM;
    if (sv == "MEDIA_RESOLUTION_HIGH") return gemini::MediaResolution::MEDIA_RESOLUTION_HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::MediaType> from_string_view<gemini::MediaType>(std::string_view sv) {
    if (sv == "application/pdf") return gemini::MediaType::APPLICATION_PDF;
    if (sv == "audio/aac") return gemini::MediaType::AUDIO_AAC;
    if (sv == "audio/flac") return gemini::MediaType::AUDIO_FLAC;
    if (sv == "audio/mp3") return gemini::MediaType::AUDIO_MP3;
    if (sv == "audio/mp4") return gemini::MediaType::AUDIO_MP4;
    if (sv == "audio/mpeg") return gemini::MediaType::AUDIO_MPEG;
    if (sv == "audio/ogg") return gemini::MediaType::AUDIO_OGG;
    if (sv == "audio/wav") return gemini::MediaType::AUDIO_WAV;
    if (sv == "image/bmp") return gemini::MediaType::IMAGE_BMP;
    if (sv == "image/gif") return gemini::MediaType::IMAGE_GIF;
    if (sv == "image/jpeg") return gemini::MediaType::IMAGE_JPEG;
    if (sv == "image/png") return gemini::MediaType::IMAGE_PNG;
    if (sv == "image/webp") return gemini::MediaType::IMAGE_WEBP;
    if (sv == "video/mov") return gemini::MediaType::VIDEO_MOV;
    if (sv == "video/mpeg") return gemini::MediaType::VIDEO_MPEG;
    if (sv == "video/mp4") return gemini::MediaType::VIDEO_MP4;
    if (sv == "video/mpg") return gemini::MediaType::VIDEO_MPG;
    if (sv == "video/ogg") return gemini::MediaType::VIDEO_OGG;
    if (sv == "video/quicktime") return gemini::MediaType::VIDEO_QT;
    if (sv == "video/webm") return gemini::MediaType::VIDEO_WEBM;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::Modality> from_string_view<gemini::Modality>(std::string_view sv) {
    if (sv == "MODALITY_UNSPECIFIED") return gemini::Modality::MODALITY_UNSPECIFIED;
    if (sv == "TEXT") return gemini::Modality::TEXT;
    if (sv == "IMAGE") return gemini::Modality::IMAGE;
    if (sv == "VIDEO") return gemini::Modality::VIDEO;
    if (sv == "AUDIO") return gemini::Modality::AUDIO;
    if (sv == "DOCUMENT") return gemini::Modality::DOCUMENT;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ModelStage> from_string_view<gemini::ModelStage>(std::string_view sv) {
    if (sv == "MODEL_STAGE_UNSPECIFIED") return gemini::ModelStage::MODEL_STAGE_UNSPECIFIED;
    if (sv == "EXPERIMENTAL") return gemini::ModelStage::EXPERIMENTAL;
    if (sv == "PREVIEW") return gemini::ModelStage::PREVIEW;
    if (sv == "STABLE") return gemini::ModelStage::STABLE;
    if (sv == "LEGACY") return gemini::ModelStage::LEGACY;
    if (sv == "RETIRED") return gemini::ModelStage::RETIRED;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ResponseMimeType> from_string_view<gemini::ResponseMimeType>(std::string_view sv) {
    if (sv == "application/json") return gemini::ResponseMimeType::APPLICATION_JSON;
    if (sv == "text/x.enum") return gemini::ResponseMimeType::TEXT_X_ENUM;
    if (sv == "text/plain") return gemini::ResponseMimeType::TEXT_PLAIN;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::Role> from_string_view<gemini::Role>(std::string_view sv) {
    if (sv == "user") return gemini::Role::USER;
    if (sv == "model") return gemini::Role::MODEL;
    if (sv == "system") return gemini::Role::SYSTEM;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::HarmBlockThreshold> from_string_view<gemini::HarmBlockThreshold>(std::string_view sv) {
    if (sv == "HARM_BLOCK_THRESHOLD_UNSPECIFIED") return gemini::HarmBlockThreshold::HARM_BLOCK_THRESHOLD_UNSPECIFIED;
    if (sv == "BLOCK_LOW_AND_ABOVE") return gemini::HarmBlockThreshold::BLOCK_LOW_AND_ABOVE;
    if (sv == "BLOCK_MEDIUM_AND_ABOVE") return gemini::HarmBlockThreshold::BLOCK_MEDIUM_AND_ABOVE;
    if (sv == "BLOCK_ONLY_HIGH") return gemini::HarmBlockThreshold::BLOCK_ONLY_HIGH;
    if (sv == "BLOCK_NONE") return gemini::HarmBlockThreshold::BLOCK_NONE;
    if (sv == "OFF") return gemini::HarmBlockThreshold::OFF;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::Scheduling> from_string_view<gemini::Scheduling>(std::string_view sv) {
    if (sv == "SCHEDULING_UNSPECIFIED") return gemini::Scheduling::SCHEDULING_UNSPECIFIED;
    if (sv == "SILENT") return gemini::Scheduling::SILENT;
    if (sv == "WHEN_IDLE") return gemini::Scheduling::WHEN_IDLE;
    if (sv == "INTERRUPT") return gemini::Scheduling::INTERRUPT;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::SchemaType> from_string_view<gemini::SchemaType>(std::string_view sv) {
    if (sv == "TYPE_UNSPECIFIED") return gemini::SchemaType::TYPE_UNSPECIFIED;
    if (sv == "STRING") return gemini::SchemaType::STRING;
    if (sv == "INTEGER") return gemini::SchemaType::INTEGER;
    if (sv == "BOOLEAN") return gemini::SchemaType::BOOLEAN;
    if (sv == "ARRAY") return gemini::SchemaType::ARRAY;
    if (sv == "OBJECT") return gemini::SchemaType::OBJECT;
    if (sv == "NULL") return gemini::SchemaType::NULL_T;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ThinkingLevel> from_string_view<gemini::ThinkingLevel>(std::string_view sv) {
    if (sv == "THINKING_LEVEL_UNSPECIFIED") return gemini::ThinkingLevel::THINKING_LEVEL_UNSPECIFIED;
    if (sv == "MINIMAL") return gemini::ThinkingLevel::MINIMAL;
    if (sv == "LOW") return gemini::ThinkingLevel::LOW;
    if (sv == "MEDIUM") return gemini::ThinkingLevel::MEDIUM;
    if (sv == "HIGH") return gemini::ThinkingLevel::HIGH;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::ToolMode> from_string_view<gemini::ToolMode>(std::string_view sv) {
    if (sv == "MODE_UNSPECIFIED") return gemini::ToolMode::MODE_UNSPECIFIED;
    if (sv == "ANY") return gemini::ToolMode::ANY;
    if (sv == "AUTO") return gemini::ToolMode::AUTO;
    if (sv == "NONE") return gemini::ToolMode::NONE;
    if (sv == "VALIDATED") return gemini::ToolMode::VALIDATED;
    return std::nullopt;
}


template <>
constexpr std::optional<gemini::UrlRetrievalStatus> from_string_view<gemini::UrlRetrievalStatus>(std::string_view sv) {
    if (sv == "URL_RETRIEVAL_STATUS_UNSPECIFIED") return gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_UNSPECIFIED;
    if (sv == "URL_RETRIEVAL_STATUS_SUCCESS") return gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_SUCCESS;
    if (sv == "URL_RETRIEVAL_STATUS_ERROR") return gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_ERROR;
    if (sv == "URL_RETRIEVAL_STATUS_PAYWALL") return gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_PAYWALL;
    if (sv == "URL_RETRIEVAL_STATUS_UNSAFE") return gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_UNSAFE;
    return std::nullopt;
}


constexpr std::string_view to_string_view(gemini::AspectRatio val) {
    switch (val) {
        case gemini::AspectRatio::ASPECT_RATIO_1_1: return "1:1";
        case gemini::AspectRatio::ASPECT_RATIO_2_3: return "2:3";
        case gemini::AspectRatio::ASPECT_RATIO_3_2: return "3:2";
        case gemini::AspectRatio::ASPECT_RATIO_3_4: return "3:4";
        case gemini::AspectRatio::ASPECT_RATIO_4_3: return "4:3";
        case gemini::AspectRatio::ASPECT_RATIO_4_5: return "4:5";
        case gemini::AspectRatio::ASPECT_RATIO_5_4: return "5:4";
        case gemini::AspectRatio::ASPECT_RATIO_9_16: return "9:16";
        case gemini::AspectRatio::ASPECT_RATIO_16_9: return "16:9";
        case gemini::AspectRatio::ASPECT_RATIO_21_9: return "21:9";
        default: throw AnnotatedException{"invalid gemini::AspectRatio"};
    }
}


constexpr std::string_view to_string_view(gemini::Behavior val) {
    switch (val) {
        case gemini::Behavior::UNSPECIFIED: return "UNSPECIFIED";
        case gemini::Behavior::BLOCKING: return "BLOCKING";
        case gemini::Behavior::NON_BLOCKING: return "NON_BLOCKING";
        default: throw AnnotatedException{"invalid gemini::Behavior"};
    }
}


constexpr std::string_view to_string_view(gemini::BlockReason val) {
    switch (val) {
        case gemini::BlockReason::BLOCK_REASON_UNSPECIFIED: return "BLOCK_REASON_UNSPECIFIED";
        case gemini::BlockReason::SAFETY: return "SAFETY";
        case gemini::BlockReason::OTHER: return "OTHER";
        case gemini::BlockReason::BLOCKLIST: return "BLOCKLIST";
        case gemini::BlockReason::PROHIBITED_CONTENT: return "PROHIBITED_CONTENT";
        case gemini::BlockReason::IMAGE_SAFETY: return "IMAGE_SAFETY";
        default: throw AnnotatedException{"invalid gemini::BlockReason"};
    }
}


constexpr std::string_view to_string_view(gemini::CodeLanguage val) {
    switch (val) {
        case gemini::CodeLanguage::LANGUAGE_UNSPECIFIED: return "LANGUAGE_UNSPECIFIED";
        case gemini::CodeLanguage::PYTHON: return "PYTHON";
        default: throw AnnotatedException{"invalid gemini::CodeLanguage"};
    }
}


constexpr std::string_view to_string_view(gemini::DynamicRetrievalMode val) {
    switch (val) {
        case gemini::DynamicRetrievalMode::MODE_UNSPECIFIED: return "MODE_UNSPECIFIED";
        case gemini::DynamicRetrievalMode::MODE_DYNAMIC: return "MODE_DYNAMIC";
        default: throw AnnotatedException{"invalid gemini::DynamicRetrievalMode"};
    }
}


constexpr std::string_view to_string_view(gemini::Environment val) {
    switch (val) {
        case gemini::Environment::ENVIRONMENT_UNSPECIFIED: return "ENVIRONMENT_UNSPECIFIED";
        case gemini::Environment::ENVIRONMENT_BROWSER: return "ENVIRONMENT_BROWSER";
        default: throw AnnotatedException{"invalid gemini::Environment"};
    }
}


constexpr std::string_view to_string_view(gemini::ExecutionOutcome val) {
    switch (val) {
        case gemini::ExecutionOutcome::OK: return "OUTCOME_OK";
        case gemini::ExecutionOutcome::FAILED: return "OUTCOME_FAILED";
        case gemini::ExecutionOutcome::DEADLINE_EXCEEDED: return "OUTCOME_DEADLINE_EXCEEDED";
        case gemini::ExecutionOutcome::UNSPECIFIED: return "OUTCOME_UNSPECIFIED";
        default: throw AnnotatedException{"invalid gemini::ExecutionOutcome"};
    }
}
enum class FinishReason {
    FINISH_REASON_UNSPECIFIED, STOP, MAX_TOKENS, SAFETY, RECITATION, LANGUAGE, OTHER, BLOCKLIST, PROHIBITED_CONTENT,
    SPII, MALFORMED_FUNCTION_CALL, IMAGE_SAFETY, IMAGE_PROHIBITED_CONTENT, IMAGE_OTHER, NO_IMAGE, IMAGE_RECITATION,
    UNEXPECTED_TOOL_CALL, TOO_MANY_TOOL_CALLS, MISSING_THOUGHT_SIGNATURE
};


constexpr std::string_view to_string_view(gemini::FinishReason val) {
    switch (val) {
        case gemini::FinishReason::FINISH_REASON_UNSPECIFIED: return "FINISH_REASON_UNSPECIFIED";
        case gemini::FinishReason::STOP: return "STOP";
        case gemini::FinishReason::MAX_TOKENS: return "MAX_TOKENS";
        case gemini::FinishReason::SAFETY: return "SAFETY";
        case gemini::FinishReason::RECITATION: return "RECITATION";
        case gemini::FinishReason::LANGUAGE: return "LANGUAGE";
        case gemini::FinishReason::OTHER: return "OTHER";
        case gemini::FinishReason::BLOCKLIST: return "BLOCKLIST";
        case gemini::FinishReason::PROHIBITED_CONTENT: return "PROHIBITED_CONTENT";
        case gemini::FinishReason::SPII: return "SPII";
        case gemini::FinishReason::MALFORMED_FUNCTION_CALL: return "MALFORMED_FUNCTION_CALL";
        case gemini::FinishReason::IMAGE_SAFETY: return "IMAGE_SAFETY";
        case gemini::FinishReason::IMAGE_PROHIBITED_CONTENT: return "IMAGE_PROHIBITED_CONTENT";
        case gemini::FinishReason::IMAGE_OTHER: return "IMAGE_OTHER";
        case gemini::FinishReason::NO_IMAGE: return "NO_IMAGE";
        case gemini::FinishReason::IMAGE_RECITATION: return "IMAGE_RECITATION";
        case gemini::FinishReason::UNEXPECTED_TOOL_CALL: return "UNEXPECTED_TOOL_CALL";
        case gemini::FinishReason::TOO_MANY_TOOL_CALLS: return "TOO_MANY_TOOL_CALLS";
        case gemini::FinishReason::MISSING_THOUGHT_SIGNATURE: return "MISSING_THOUGHT_SIGNATURE";
        default: throw AnnotatedException{"invalid gemini::FinishReason"};
    }
}


constexpr std::string_view to_string_view(gemini::HarmCategory val) {
    switch (val) {
        case gemini::HarmCategory::HARM_CATEGORY_UNSPECIFIED: return "HARM_CATEGORY_UNSPECIFIED";
        case gemini::HarmCategory::HARM_CATEGORY_DEROGATORY: return "HARM_CATEGORY_DEROGATORY";
        case gemini::HarmCategory::HARM_CATEGORY_TOXICITY: return "HARM_CATEGORY_TOXICITY";
        case gemini::HarmCategory::HARM_CATEGORY_VIOLENCE: return "HARM_CATEGORY_VIOLENCE";
        case gemini::HarmCategory::HARM_CATEGORY_SEXUAL: return "HARM_CATEGORY_SEXUAL";
        case gemini::HarmCategory::HARM_CATEGORY_MEDICAL: return "HARM_CATEGORY_MEDICAL";
        case gemini::HarmCategory::HARM_CATEGORY_DANGEROUS: return "HARM_CATEGORY_DANGEROUS";
        case gemini::HarmCategory::HARM_CATEGORY_HARASSMENT: return "HARM_CATEGORY_HARASSMENT";
        case gemini::HarmCategory::HARM_CATEGORY_HATE_SPEECH: return "HARM_CATEGORY_HATE_SPEECH";
        case gemini::HarmCategory::HARM_CATEGORY_SEXUALLY_EXPLICIT: return "HARM_CATEGORY_SEXUALLY_EXPLICIT";
        case gemini::HarmCategory::HARM_CATEGORY_DANGEROUS_CONTENT: return "HARM_CATEGORY_DANGEROUS_CONTENT";
        default: throw AnnotatedException{"invalid gemini::HarmCategory"};
    }
}


constexpr std::string_view to_string_view(gemini::HarmProbability val) {
    switch (val) {
        case gemini::HarmProbability::HARM_PROBABILITY_UNSPECIFIED: return "HARM_PROBABILITY_UNSPECIFIED";
        case gemini::HarmProbability::NEGLIGIBLE: return "NEGLIGIBLE";
        case gemini::HarmProbability::LOW: return "LOW";
        case gemini::HarmProbability::MEDIUM: return "MEDIUM";
        case gemini::HarmProbability::HIGH: return "HIGH";
        default: throw AnnotatedException{"invalid gemini::HarmProbability"};
    }
}


constexpr std::string_view to_string_view(gemini::ImageDim val) {
    switch (val) {
        case gemini::ImageDim::IMAGE_SIZE_1K: return "1K";
        case gemini::ImageDim::IMAGE_SIZE_2K: return "2K";
        case gemini::ImageDim::IMAGE_SIZE_4K: return "4K";
        default: throw AnnotatedException{"invalid gemini::ImageDim"};
    }
}


constexpr std::string_view to_string_view(gemini::MediaResolution val) {
    switch (val) {
        case gemini::MediaResolution::MEDIA_RESOLUTION_UNSPECIFIED: return "MEDIA_RESOLUTION_UNSPECIFIED";
        case gemini::MediaResolution::MEDIA_RESOLUTION_LOW: return "MEDIA_RESOLUTION_LOW";
        case gemini::MediaResolution::MEDIA_RESOLUTION_MEDIUM: return "MEDIA_RESOLUTION_MEDIUM";
        case gemini::MediaResolution::MEDIA_RESOLUTION_HIGH: return "MEDIA_RESOLUTION_HIGH";
        default: throw AnnotatedException{"invalid gemini::MediaResolution"};
    }
}


constexpr std::string_view to_string_view(gemini::MediaType val) {
    switch (val) {
        case gemini::MediaType::APPLICATION_PDF: return "application/pdf";
        case gemini::MediaType::AUDIO_AAC: return "audio/aac";
        case gemini::MediaType::AUDIO_FLAC: return "audio/flac";
        case gemini::MediaType::AUDIO_MP3: return "audio/mp3";
        case gemini::MediaType::AUDIO_MP4: return "audio/mp4";
        case gemini::MediaType::AUDIO_MPEG: return "audio/mpeg";
        case gemini::MediaType::AUDIO_OGG: return "audio/ogg";
        case gemini::MediaType::AUDIO_WAV: return "audio/wav";
        case gemini::MediaType::IMAGE_BMP: return "image/bmp";
        case gemini::MediaType::IMAGE_GIF: return "image/gif";
        case gemini::MediaType::IMAGE_JPEG: return "image/jpeg";
        case gemini::MediaType::IMAGE_PNG: return "image/png";
        case gemini::MediaType::IMAGE_WEBP: return "image/webp";
        case gemini::MediaType::VIDEO_MOV: return "video/mov";
        case gemini::MediaType::VIDEO_MPEG: return "video/mpeg";
        case gemini::MediaType::VIDEO_MP4: return "video/mp4";
        case gemini::MediaType::VIDEO_MPG: return "video/mpg";
        case gemini::MediaType::VIDEO_OGG: return "video/ogg";
        case gemini::MediaType::VIDEO_QT: return "video/quicktime";
        case gemini::MediaType::VIDEO_WEBM: return "video/webm";
        default: throw AnnotatedException{"invalid gemini::MediaType"};
    }
}


constexpr std::string_view to_string_view(gemini::Modality val) {
    switch (val) {
        case gemini::Modality::MODALITY_UNSPECIFIED: return "MODALITY_UNSPECIFIED";
        case gemini::Modality::TEXT: return "TEXT";
        case gemini::Modality::IMAGE: return "IMAGE";
        case gemini::Modality::VIDEO: return "VIDEO";
        case gemini::Modality::AUDIO: return "AUDIO";
        case gemini::Modality::DOCUMENT: return "DOCUMENT";
        default: throw AnnotatedException{"invalid gemini::Modality"};
    }
}


constexpr std::string_view to_string_view(gemini::ModelStage val) {
    switch (val) {
        case gemini::ModelStage::MODEL_STAGE_UNSPECIFIED: return "MODEL_STAGE_UNSPECIFIED";
        case gemini::ModelStage::EXPERIMENTAL: return "EXPERIMENTAL";
        case gemini::ModelStage::PREVIEW: return "PREVIEW";
        case gemini::ModelStage::STABLE: return "STABLE";
        case gemini::ModelStage::LEGACY: return "LEGACY";
        case gemini::ModelStage::RETIRED: return "RETIRED";
        default: throw AnnotatedException{"invalid gemini::ModelStage"};
    }
}


constexpr std::string_view to_string_view(gemini::ResponseMimeType val) {
    switch (val) {
        case gemini::ResponseMimeType::APPLICATION_JSON: return "application/json";
        case gemini::ResponseMimeType::TEXT_X_ENUM: return "text/x.enum";
        case gemini::ResponseMimeType::TEXT_PLAIN: return "text/plain";
        default: throw AnnotatedException{"invalid gemini::ResponseMimeType"};
    }
}


constexpr std::string_view to_string_view(gemini::Role val) {
    switch (val) {
        case gemini::Role::USER: return "user";
        case gemini::Role::MODEL: return "model";
        case gemini::Role::SYSTEM: return "system";
        default: throw AnnotatedException{"invalid gemini::Role"};
    }
}


constexpr std::string_view to_string_view(gemini::HarmBlockThreshold val) {
    switch (val) {
        case gemini::HarmBlockThreshold::HARM_BLOCK_THRESHOLD_UNSPECIFIED: return "HARM_BLOCK_THRESHOLD_UNSPECIFIED";
        case gemini::HarmBlockThreshold::BLOCK_LOW_AND_ABOVE: return "BLOCK_LOW_AND_ABOVE";
        case gemini::HarmBlockThreshold::BLOCK_MEDIUM_AND_ABOVE: return "BLOCK_MEDIUM_AND_ABOVE";
        case gemini::HarmBlockThreshold::BLOCK_ONLY_HIGH: return "BLOCK_ONLY_HIGH";
        case gemini::HarmBlockThreshold::BLOCK_NONE: return "BLOCK_NONE";
        case gemini::HarmBlockThreshold::OFF: return "OFF";
        default: throw AnnotatedException{"invalid gemini::HarmBlockThreshold"};
    }
}


constexpr std::string_view to_string_view(gemini::Scheduling val) {
    switch (val) {
        case gemini::Scheduling::SCHEDULING_UNSPECIFIED: return "SCHEDULING_UNSPECIFIED";
        case gemini::Scheduling::SILENT: return "SILENT";
        case gemini::Scheduling::WHEN_IDLE: return "WHEN_IDLE";
        case gemini::Scheduling::INTERRUPT: return "INTERRUPT";
        default: throw AnnotatedException{"invalid gemini::Scheduling"};
    }
}


constexpr std::string_view to_string_view(gemini::SchemaType val) {
    switch (val) {
        case gemini::SchemaType::TYPE_UNSPECIFIED: return "TYPE_UNSPECIFIED";
        case gemini::SchemaType::STRING: return "STRING";
        case gemini::SchemaType::INTEGER: return "INTEGER";
        case gemini::SchemaType::BOOLEAN: return "BOOLEAN";
        case gemini::SchemaType::ARRAY: return "ARRAY";
        case gemini::SchemaType::OBJECT: return "OBJECT";
        case gemini::SchemaType::NULL_T: return "NULL";
        default: throw AnnotatedException{"invalid gemini::SchemaType"};
    }
}


enum class ThinkingLevel { THINKING_LEVEL_UNSPECIFIED, MINIMAL, LOW, MEDIUM, HIGH };
constexpr std::string_view to_string_view(gemini::ThinkingLevel val) {
    switch (val) {
        case gemini::ThinkingLevel::THINKING_LEVEL_UNSPECIFIED: return "THINKING_LEVEL_UNSPECIFIED";
        case gemini::ThinkingLevel::MINIMAL: return "MINIMAL";
        case gemini::ThinkingLevel::LOW: return "LOW";
        case gemini::ThinkingLevel::MEDIUM: return "MEDIUM";
        case gemini::ThinkingLevel::HIGH: return "HIGH";
        default: throw AnnotatedException{"invalid gemini::ThinkingLevel"};
    }
}


constexpr std::string_view to_string_view(gemini::ToolMode val) {
    switch (val) {
        case gemini::ToolMode::MODE_UNSPECIFIED: return "MODE_UNSPECIFIED";
        case gemini::ToolMode::AUTO: return "AUTO";
        case gemini::ToolMode::ANY: return "ANY";
        case gemini::ToolMode::NONE: return "NONE";
        case gemini::ToolMode::VALIDATED: return "VALIDATED";
        default: throw AnnotatedException{"invalid gemini::ToolMode"};
    }
}


constexpr std::string_view to_string_view(gemini::UrlRetrievalStatus val) {
    switch (val) {
        case gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_UNSPECIFIED: return "URL_RETRIEVAL_STATUS_UNSPECIFIED";
        case gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_SUCCESS: return "URL_RETRIEVAL_STATUS_SUCCESS";
        case gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_ERROR: return "URL_RETRIEVAL_STATUS_ERROR";
        case gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_PAYWALL: return "URL_RETRIEVAL_STATUS_PAYWALL";
        case gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_UNSAFE: return "URL_RETRIEVAL_STATUS_UNSAFE";
        default: throw AnnotatedException{"invalid gemini::UrlRetrievalStatus"};
    }
}


}
