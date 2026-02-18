#pragma once

#include "../../core/strings_base.hpp"
#include "../../core/error.hpp"
#include "messages.hpp"


namespace jai::llm {


/***
 * from_string_view
 */
// Kinds
template <>
constexpr std::optional<anthropic::KindBase64Source> from_string_view<anthropic::KindBase64Source>(std::string_view sv) {
    if (sv == anthropic::KindBase64Source::value) return anthropic::KindBase64Source{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindCharLocation> from_string_view<anthropic::KindCharLocation>(std::string_view sv) {
    if (sv == anthropic::KindCharLocation::value) return anthropic::KindCharLocation{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindContentBlockLocation> from_string_view<anthropic::KindContentBlockLocation>(std::string_view sv) {
    if (sv == anthropic::KindContentBlockLocation::value) return anthropic::KindContentBlockLocation{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindContentSource> from_string_view<anthropic::KindContentSource>(std::string_view sv) {
    if (sv == anthropic::KindContentSource::value) return anthropic::KindContentSource{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindCustomTool> from_string_view<anthropic::KindCustomTool>(std::string_view sv) {
    if (sv == anthropic::KindCustomTool::value) return anthropic::KindCustomTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindDocumentBlock> from_string_view<anthropic::KindDocumentBlock>(std::string_view sv) {
    if (sv == anthropic::KindDocumentBlock::value) return anthropic::KindDocumentBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindEphemeral> from_string_view<anthropic::KindEphemeral>(std::string_view sv) {
    if (sv == anthropic::KindEphemeral::value) return anthropic::KindEphemeral{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindImageBlock> from_string_view<anthropic::KindImageBlock>(std::string_view sv) {
    if (sv == anthropic::KindImageBlock::value) return anthropic::KindImageBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindJsonSchemaType> from_string_view<anthropic::KindJsonSchemaType>(std::string_view sv) {
    if (sv == anthropic::KindJsonSchemaType::value) return anthropic::KindJsonSchemaType{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindMessage> from_string_view<anthropic::KindMessage>(std::string_view sv) {
    if (sv == anthropic::KindMessage::value) return anthropic::KindMessage{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindPageLocation> from_string_view<anthropic::KindPageLocation>(std::string_view sv) {
    if (sv == anthropic::KindPageLocation::value) return anthropic::KindPageLocation{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindPlainTextSource> from_string_view<anthropic::KindPlainTextSource>(std::string_view sv) {
    if (sv == anthropic::KindPlainTextSource::value) return anthropic::KindPlainTextSource{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindRedactedThinkingBlock> from_string_view<anthropic::KindRedactedThinkingBlock>(std::string_view sv) {
    if (sv == anthropic::KindRedactedThinkingBlock::value) return anthropic::KindRedactedThinkingBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindSearchResultBlock> from_string_view<anthropic::KindSearchResultBlock>(std::string_view sv) {
    if (sv == anthropic::KindSearchResultBlock::value) return anthropic::KindSearchResultBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindSearchResultLocation> from_string_view<anthropic::KindSearchResultLocation>(std::string_view sv) {
    if (sv == anthropic::KindSearchResultLocation::value) return anthropic::KindSearchResultLocation{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindServerToolUseBlock> from_string_view<anthropic::KindServerToolUseBlock>(std::string_view sv) {
    if (sv == anthropic::KindServerToolUseBlock::value) return anthropic::KindServerToolUseBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindStructuredOutputFormat> from_string_view<anthropic::KindStructuredOutputFormat>(std::string_view sv) {
    if (sv == anthropic::KindStructuredOutputFormat::value) return anthropic::KindStructuredOutputFormat{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindTextBlock> from_string_view<anthropic::KindTextBlock>(std::string_view sv) {
    if (sv == anthropic::KindTextBlock::value) return anthropic::KindTextBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindThinkingBlock> from_string_view<anthropic::KindThinkingBlock>(std::string_view sv) {
    if (sv == anthropic::KindThinkingBlock::value) return anthropic::KindThinkingBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolBash20250124Type> from_string_view<anthropic::KindToolBash20250124Type>(std::string_view sv) {
    if (sv == anthropic::KindToolBash20250124Type::value) return anthropic::KindToolBash20250124Type{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolChoiceAny> from_string_view<anthropic::KindToolChoiceAny>(std::string_view sv) {
    if (sv == anthropic::KindToolChoiceAny::value) return anthropic::KindToolChoiceAny{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolChoiceAuto> from_string_view<anthropic::KindToolChoiceAuto>(std::string_view sv) {
    if (sv == anthropic::KindToolChoiceAuto::value) return anthropic::KindToolChoiceAuto{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolChoiceNone> from_string_view<anthropic::KindToolChoiceNone>(std::string_view sv) {
    if (sv == anthropic::KindToolChoiceNone::value) return anthropic::KindToolChoiceNone{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolChoiceTool> from_string_view<anthropic::KindToolChoiceTool>(std::string_view sv) {
    if (sv == anthropic::KindToolChoiceTool::value) return anthropic::KindToolChoiceTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolResultBlock> from_string_view<anthropic::KindToolResultBlock>(std::string_view sv) {
    if (sv == anthropic::KindToolResultBlock::value) return anthropic::KindToolResultBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolTextEditor20250124Name> from_string_view<anthropic::KindToolTextEditor20250124Name>(std::string_view sv) {
    if (sv == anthropic::KindToolTextEditor20250124Name::value) return anthropic::KindToolTextEditor20250124Name{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolTextEditor20250429Name> from_string_view<anthropic::KindToolTextEditor20250429Name>(std::string_view sv) {
    if (sv == anthropic::KindToolTextEditor20250429Name::value) return anthropic::KindToolTextEditor20250429Name{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolTextEditor20250728Name> from_string_view<anthropic::KindToolTextEditor20250728Name>(std::string_view sv) {
    if (sv == anthropic::KindToolTextEditor20250728Name::value) return anthropic::KindToolTextEditor20250728Name{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindToolUseBlock> from_string_view<anthropic::KindToolUseBlock>(std::string_view sv) {
    if (sv == anthropic::KindToolUseBlock::value) return anthropic::KindToolUseBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindUrlSource> from_string_view<anthropic::KindUrlSource>(std::string_view sv) {
    if (sv == anthropic::KindUrlSource::value) return anthropic::KindUrlSource{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindWebSearchResultLocation> from_string_view<anthropic::KindWebSearchResultLocation>(std::string_view sv) {
    if (sv == anthropic::KindWebSearchResultLocation::value) return anthropic::KindWebSearchResultLocation{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindWebSearchTool20250305> from_string_view<anthropic::KindWebSearchTool20250305>(std::string_view sv) {
    if (sv == anthropic::KindWebSearchTool20250305::value) return anthropic::KindWebSearchTool20250305{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindWebSearchToolResultBlock> from_string_view<anthropic::KindWebSearchToolResultBlock>(std::string_view sv) {
    if (sv == anthropic::KindWebSearchToolResultBlock::value) return anthropic::KindWebSearchToolResultBlock{};
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::KindWebSearchToolResultError> from_string_view<anthropic::KindWebSearchToolResultError>(std::string_view sv) {
    if (sv == anthropic::KindWebSearchToolResultError::value) return anthropic::KindWebSearchToolResultError{};
    return std::nullopt;
}


// Enums
template <>
constexpr std::optional<anthropic::CacheControlTTL> from_string_view<anthropic::CacheControlTTL>(std::string_view sv) {
    if (sv == "1h") return anthropic::CacheControlTTL::TTL_1H;
    if (sv == "5m") return anthropic::CacheControlTTL::TTL_5M;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::CitationKinds> from_string_view<anthropic::CitationKinds>(std::string_view sv) {
    if (sv == "char_location") return anthropic::CitationKinds::CHAR_LOCATION;
    if (sv == "content_block_location") return anthropic::CitationKinds::CONTENT_BLOCK_LOCATION;
    if (sv == "page_location") return anthropic::CitationKinds::PAGE_LOCATION;
    if (sv == "search_result_location") return anthropic::CitationKinds::SEARCH_RESULT_LOCATION;
    if (sv == "web_search_result_location") return anthropic::CitationKinds::WEB_SEARCH_RESULT_LOCATION;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::DocSrcKind> from_string_view<anthropic::DocSrcKind>(std::string_view sv) {
    if (sv == "content") return anthropic::DocSrcKind::BLOCK;
    if (sv == "base64") return anthropic::DocSrcKind::PDF;
    if (sv == "text") return anthropic::DocSrcKind::PLAIN_TEXT;
    if (sv == "url") return anthropic::DocSrcKind::URL_PDF;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ImageMediaType> from_string_view<anthropic::ImageMediaType>(std::string_view sv) {
    if (sv == "image/gif") return anthropic::ImageMediaType::IMAGE_GIF;
    if (sv == "image/jpeg") return anthropic::ImageMediaType::IMAGE_JPEG;
    if (sv == "image/png") return anthropic::ImageMediaType::IMAGE_PNG;
    if (sv == "image/webp") return anthropic::ImageMediaType::IMAGE_WEBP;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ImageSourceKinds> from_string_view<anthropic::ImageSourceKinds>(std::string_view sv) {
    if (sv == "base64") return anthropic::ImageSourceKinds::BASE64;
    if (sv == "url") return anthropic::ImageSourceKinds::URL;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::PDFMediaType> from_string_view<anthropic::PDFMediaType>(std::string_view sv) {
    if (sv == "application/pdf") return anthropic::PDFMediaType::APPLICATION_PDF;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::PlainTextMediaType> from_string_view<anthropic::PlainTextMediaType>(std::string_view sv) {
    if (sv == "text/plain") return anthropic::PlainTextMediaType::TEXT_PLAIN;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ReplaceBasedEditor> from_string_view<anthropic::ReplaceBasedEditor>(std::string_view sv) {
    if (sv == "str_replace_based_edit_tool") return anthropic::ReplaceBasedEditor::STRING;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ReplaceEditor> from_string_view<anthropic::ReplaceEditor>(std::string_view sv) {
    if (sv == "str_replace_editor") return anthropic::ReplaceEditor::STRING;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::RequestServiceTier> from_string_view<anthropic::RequestServiceTier>(std::string_view sv) {
    if (sv == "auto") return anthropic::RequestServiceTier::AUTO;
    if (sv == "standard_only") return anthropic::RequestServiceTier::STANDARD_ONLY;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ResponseContentBlockKinds> from_string_view<anthropic::ResponseContentBlockKinds>(std::string_view sv) {
    if (sv == "redacted_thinking") return anthropic::ResponseContentBlockKinds::REDACTED_THINKING;
    if (sv == "server_tool_use") return anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE;
    if (sv == "text") return anthropic::ResponseContentBlockKinds::TEXT;
    if (sv == "thinking") return anthropic::ResponseContentBlockKinds::THINKING;
    if (sv == "tool_use") return anthropic::ResponseContentBlockKinds::TOOL_USE;
    if (sv == "web_search_tool_result") return anthropic::ResponseContentBlockKinds::WEB_SEARCH_TOOL_RESULT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ResponseRole> from_string_view<anthropic::ResponseRole>(std::string_view sv) {
    if (sv == "assistant") return anthropic::ResponseRole::ASSISTANT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::Role> from_string_view<anthropic::Role>(std::string_view sv) {
    if (sv == "assistant") return anthropic::Role::ASSISTANT;
    if (sv == "user") return anthropic::Role::USER;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::StopReason> from_string_view<anthropic::StopReason>(std::string_view sv) {
    if (sv == "end_turn") return anthropic::StopReason::END_TURN;
    if (sv == "max_tokens") return anthropic::StopReason::MAX_TOKENS;
    if (sv == "pause_turn") return anthropic::StopReason::PAUSE_TURN;
    if (sv == "refusal") return anthropic::StopReason::REFUSAL;
    if (sv == "stop_sequence") return anthropic::StopReason::STOP_SEQUENCE;
    if (sv == "tool_use") return anthropic::StopReason::TOOL_USE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ThinkingConfigType> from_string_view<anthropic::ThinkingConfigType>(std::string_view sv) {
    if (sv == "adaptive") return anthropic::ThinkingConfigType::ADAPTIVE;
    if (sv == "disabled") return anthropic::ThinkingConfigType::DISABLED;
    if (sv == "enabled") return anthropic::ThinkingConfigType::ENABLED;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ThinkingEffort> from_string_view<anthropic::ThinkingEffort>(std::string_view sv) {
    if (sv == "high") return anthropic::ThinkingEffort::HIGH;
    if (sv == "low") return anthropic::ThinkingEffort::LOW;
    if (sv == "max") return anthropic::ThinkingEffort::MAX;
    if (sv == "medium") return anthropic::ThinkingEffort::MEDIUM;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolBash20250124Name> from_string_view<anthropic::ToolBash20250124Name>(std::string_view sv) {
    if (sv == "bash") return anthropic::ToolBash20250124Name::BASH;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::UsageServiceTier> from_string_view<anthropic::UsageServiceTier>(std::string_view sv) {
    if (sv == "batch") return anthropic::UsageServiceTier::BATCH;
    if (sv == "priority") return anthropic::UsageServiceTier::PRIORITY;
    if (sv == "standard") return anthropic::UsageServiceTier::STANDARD;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::UserLocationType> from_string_view<anthropic::UserLocationType>(std::string_view sv) {
    if (sv == "approximate") return anthropic::UserLocationType::APPROXIMATE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchName> from_string_view<anthropic::WebSearchName>(std::string_view sv) {
    if (sv == "web_search") return anthropic::WebSearchName::WEB_SEARCH;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchToolResultErrorCode> from_string_view<anthropic::WebSearchToolResultErrorCode>(std::string_view sv) {
    if (sv == "invalid_tool_input") return anthropic::WebSearchToolResultErrorCode::INVALID_TOOL_INPUT;
    if (sv == "max_uses_exceeded") return anthropic::WebSearchToolResultErrorCode::MAX_USES_EXCEEDED;
    if (sv == "query_too_long") return anthropic::WebSearchToolResultErrorCode::QUERY_TOO_LONG;
    if (sv == "request_too_large") return anthropic::WebSearchToolResultErrorCode::REQUEST_TOO_LARGE;
    if (sv == "too_many_requests") return anthropic::WebSearchToolResultErrorCode::TOO_MANY_REQUESTS;
    if (sv == "unavailable") return anthropic::WebSearchToolResultErrorCode::UNAVAILABLE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchToolResultErrorType> from_string_view<anthropic::WebSearchToolResultErrorType>(std::string_view sv) {
    if (sv == "web_search_tool_result") return anthropic::WebSearchToolResultErrorType::TOOL_RESULT;
    if (sv == "web_search_tool_result_error") return anthropic::WebSearchToolResultErrorType::TOOL_RESULT_ERROR;
    return std::nullopt;
}


/***
 * to_string_view
 */
constexpr std::string_view to_string_view(anthropic::CacheControlTTL val) {
    switch (val) {
        case anthropic::CacheControlTTL::TTL_1H: return "1h";
        case anthropic::CacheControlTTL::TTL_5M: return "5m";
        default: throw AnnotatedException{"invalid anthropic::CacheControlTTL"};
    }
}

constexpr std::string_view to_string_view(anthropic::CitationKinds val) {
    switch (val) {
        case anthropic::CitationKinds::CHAR_LOCATION: return "char_location";
        case anthropic::CitationKinds::CONTENT_BLOCK_LOCATION: return "content_block_location";
        case anthropic::CitationKinds::PAGE_LOCATION: return "page_location";
        case anthropic::CitationKinds::SEARCH_RESULT_LOCATION: return "search_result_location";
        case anthropic::CitationKinds::WEB_SEARCH_RESULT_LOCATION: return "web_search_result_location";
        default: throw AnnotatedException{"invalid anthropic::CitationKinds"};
    }
}

constexpr std::string_view to_string_view(anthropic::DocSrcKind val) {
    switch (val) {
        case anthropic::DocSrcKind::BLOCK: return "content";
        case anthropic::DocSrcKind::PDF: return "base64";
        case anthropic::DocSrcKind::PLAIN_TEXT: return "text";
        case anthropic::DocSrcKind::URL_PDF: return "url";
        default: throw AnnotatedException{"invalid anthropic::DocSrcKind"};
    }
}

constexpr std::string_view to_string_view(anthropic::ImageMediaType val) {
    switch (val) {
        case anthropic::ImageMediaType::IMAGE_GIF: return "image/gif";
        case anthropic::ImageMediaType::IMAGE_JPEG: return "image/jpeg";
        case anthropic::ImageMediaType::IMAGE_PNG: return "image/png";
        case anthropic::ImageMediaType::IMAGE_WEBP: return "image/webp";
        default: throw AnnotatedException{"invalid anthropic::ImageMediaType"};
    }
}

constexpr std::string_view to_string_view(anthropic::ImageSourceKinds val) {
    switch (val) {
        case anthropic::ImageSourceKinds::BASE64: return "base64";
        case anthropic::ImageSourceKinds::URL: return "url";
        default: throw AnnotatedException{"invalid anthropic::ImageSourceKinds"};
    }
}

constexpr std::string_view to_string_view(anthropic::PDFMediaType val) {
    switch (val) {
        case anthropic::PDFMediaType::APPLICATION_PDF: return "application/pdf";
        default: throw AnnotatedException{"invalid anthropic::PDFMediaType"};
    }
}

constexpr std::string_view to_string_view(anthropic::PlainTextMediaType val) {
    switch (val) {
        case anthropic::PlainTextMediaType::TEXT_PLAIN: return "text/plain";
        default: throw AnnotatedException{"invalid anthropic::PlainTextMediaType"};
    }
}

constexpr std::string_view to_string_view(anthropic::ReplaceBasedEditor val) {
    switch (val) {
        case anthropic::ReplaceBasedEditor::STRING: return "str_replace_based_edit_tool";
        default: throw AnnotatedException{"invalid anthropic::ReplaceBasedEditor"};
    }
}

constexpr std::string_view to_string_view(anthropic::ReplaceEditor val) {
    switch (val) {
        case anthropic::ReplaceEditor::STRING: return "str_replace_editor";
        default: throw AnnotatedException{"invalid anthropic::ReplaceEditor"};
    }
}

constexpr std::string_view to_string_view(anthropic::RequestServiceTier val) {
    switch (val) {
        case anthropic::RequestServiceTier::AUTO: return "auto";
        case anthropic::RequestServiceTier::STANDARD_ONLY: return "standard_only";
        default: throw AnnotatedException{"invalid anthropic::RequestServiceTier"};
    }
}

constexpr std::string_view to_string_view(anthropic::ResponseContentBlockKinds val) {
    switch (val) {
        case anthropic::ResponseContentBlockKinds::REDACTED_THINKING: return "redacted_thinking";
        case anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE: return "server_tool_use";
        case anthropic::ResponseContentBlockKinds::TEXT: return "text";
        case anthropic::ResponseContentBlockKinds::THINKING: return "thinking";
        case anthropic::ResponseContentBlockKinds::TOOL_USE: return "tool_use";
        case anthropic::ResponseContentBlockKinds::WEB_SEARCH_TOOL_RESULT: return "web_search_tool_result";
        default: throw AnnotatedException{"invalid anthropic::ResponseContentBlockKinds"};
    }
}

constexpr std::string_view to_string_view(anthropic::ResponseRole val) {
    switch (val) {
        case anthropic::ResponseRole::ASSISTANT: return "assistant";
        default: throw AnnotatedException{"invalid anthropic::ResponseRole"};
    }
}

constexpr std::string_view to_string_view(anthropic::Role val) {
    switch (val) {
        case anthropic::Role::ASSISTANT: return "assistant";
        case anthropic::Role::USER: return "user";
        default: throw AnnotatedException{"invalid anthropic::Role"};
    }
}

constexpr std::string_view to_string_view(anthropic::StopReason val) {
    switch (val) {
        case anthropic::StopReason::END_TURN: return "end_turn";
        case anthropic::StopReason::MAX_TOKENS: return "max_tokens";
        case anthropic::StopReason::PAUSE_TURN: return "pause_turn";
        case anthropic::StopReason::REFUSAL: return "refusal";
        case anthropic::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case anthropic::StopReason::TOOL_USE: return "tool_use";
        default: throw AnnotatedException{"invalid anthropic::StopReason"};
    }
}

constexpr std::string_view to_string_view(anthropic::ThinkingConfigType val) {
    switch (val) {
        case anthropic::ThinkingConfigType::ADAPTIVE: return "adaptive";
        case anthropic::ThinkingConfigType::DISABLED: return "disabled";
        case anthropic::ThinkingConfigType::ENABLED: return "enabled";
        default: throw AnnotatedException{"invalid anthropic::ThinkingConfigType"};
    }
}

constexpr std::string_view to_string_view(anthropic::ThinkingEffort val) {
    switch (val) {
        case anthropic::ThinkingEffort::HIGH: return "high";
        case anthropic::ThinkingEffort::LOW: return "low";
        case anthropic::ThinkingEffort::MAX: return "max";
        case anthropic::ThinkingEffort::MEDIUM: return "medium";
        default: throw AnnotatedException{"invalid anthropic::ThinkingEffort"};
    }
}

constexpr std::string_view to_string_view(anthropic::ToolBash20250124Name val) {
    switch (val) {
        case anthropic::ToolBash20250124Name::BASH: return "bash";
        default: throw AnnotatedException{"invalid anthropic::ToolBash20250124Name"};
    }
}

constexpr std::string_view to_string_view(anthropic::UsageServiceTier val) {
    switch (val) {
        case anthropic::UsageServiceTier::BATCH: return "batch";
        case anthropic::UsageServiceTier::PRIORITY: return "priority";
        case anthropic::UsageServiceTier::STANDARD: return "standard";
        default: throw AnnotatedException{"invalid anthropic::UsageServiceTier"};
    }
}

constexpr std::string_view to_string_view(anthropic::UserLocationType val) {
    switch (val) {
        case anthropic::UserLocationType::APPROXIMATE: return "approximate";
        default: throw AnnotatedException{"invalid anthropic::UserLocationType"};
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchName val) {
    switch (val) {
        case anthropic::WebSearchName::WEB_SEARCH: return "web_search";
        default: throw AnnotatedException{"invalid anthropic::WebSearchName"};
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchToolResultErrorCode val) {
    switch (val) {
        case anthropic::WebSearchToolResultErrorCode::INVALID_TOOL_INPUT: return "invalid_tool_input";
        case anthropic::WebSearchToolResultErrorCode::MAX_USES_EXCEEDED: return "max_uses_exceeded";
        case anthropic::WebSearchToolResultErrorCode::QUERY_TOO_LONG: return "query_too_long";
        case anthropic::WebSearchToolResultErrorCode::REQUEST_TOO_LARGE: return "request_too_large";
        case anthropic::WebSearchToolResultErrorCode::TOO_MANY_REQUESTS: return "too_many_requests";
        case anthropic::WebSearchToolResultErrorCode::UNAVAILABLE: return "unavailable";
        default: throw AnnotatedException{"invalid anthropic::WebSearchToolResultErrorCode"};
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchToolResultErrorType val) {
    switch (val) {
        case anthropic::WebSearchToolResultErrorType::TOOL_RESULT: return "web_search_tool_result";
        case anthropic::WebSearchToolResultErrorType::TOOL_RESULT_ERROR: return "web_search_tool_result_error";
        default: throw AnnotatedException{"invalid anthropic::WebSearchToolResultErrorType"};
    }
}


}
