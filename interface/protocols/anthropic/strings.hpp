#pragma once

#include "messages.hpp"

#include <optional>
#include <stdexcept>
#include <string_view>


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


/***
 * from_string_view Specializations
 */

template <>
constexpr std::optional<anthropic::Base64SourceKind> from_string_view<anthropic::Base64SourceKind>(std::string_view sv) {
    if (sv == "base64") return anthropic::Base64SourceKind::BASE64;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::CacheControlEphemeralKind> from_string_view<anthropic::CacheControlEphemeralKind>(std::string_view sv) {
    if (sv == "ephemeral") return anthropic::CacheControlEphemeralKind::EPHEMERAL;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::CacheControlTTL> from_string_view<anthropic::CacheControlTTL>(std::string_view sv) {
    if (sv == "5m") return anthropic::CacheControlTTL::TTL_5M;
    if (sv == "1h") return anthropic::CacheControlTTL::TTL_1H;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::CharLocationKind> from_string_view<anthropic::CharLocationKind>(std::string_view sv) {
    if (sv == "char_location") return anthropic::CharLocationKind::CHAR_LOCATION;
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
constexpr std::optional<anthropic::ContentBlockLocationKind> from_string_view<anthropic::ContentBlockLocationKind>(std::string_view sv) {
    if (sv == "content_block_location") return anthropic::ContentBlockLocationKind::CONTENT_BLOCK_LOCATION;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ContentSourceKind> from_string_view<anthropic::ContentSourceKind>(std::string_view sv) {
    if (sv == "content") return anthropic::ContentSourceKind::CONTENT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ContentUnitKind> from_string_view<anthropic::ContentUnitKind>(std::string_view sv) {
    if (sv == "text") return anthropic::ContentUnitKind::TEXT;
    if (sv == "image") return anthropic::ContentUnitKind::IMAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::CustomToolKind> from_string_view<anthropic::CustomToolKind>(std::string_view sv) {
    if (sv == "custom") return anthropic::CustomToolKind::CUSTOM;
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
constexpr std::optional<anthropic::DocumentBlockKind> from_string_view<anthropic::DocumentBlockKind>(std::string_view sv) {
    if (sv == "document") return anthropic::DocumentBlockKind::DOCUMENT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ImageBlockKind> from_string_view<anthropic::ImageBlockKind>(std::string_view sv) {
    if (sv == "image") return anthropic::ImageBlockKind::IMAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ImageMediaType> from_string_view<anthropic::ImageMediaType>(std::string_view sv) {
    if (sv == "image/jpeg") return anthropic::ImageMediaType::IMAGE_JPEG;
    if (sv == "image/png") return anthropic::ImageMediaType::IMAGE_PNG;
    if (sv == "image/gif") return anthropic::ImageMediaType::IMAGE_GIF;
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
constexpr std::optional<anthropic::JsonSchemaTypeKind> from_string_view<anthropic::JsonSchemaTypeKind>(std::string_view sv) {
    if (sv == "object") return anthropic::JsonSchemaTypeKind::OBJECT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::MessageType> from_string_view<anthropic::MessageType>(std::string_view sv) {
    if (sv == "message") return anthropic::MessageType::MESSAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::PageLocationKind> from_string_view<anthropic::PageLocationKind>(std::string_view sv) {
    if (sv == "page_location") return anthropic::PageLocationKind::PAGE_LOCATION;
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
constexpr std::optional<anthropic::PlainTextSourceKind> from_string_view<anthropic::PlainTextSourceKind>(std::string_view sv) {
    if (sv == "text") return anthropic::PlainTextSourceKind::TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::RedactedThinkingBlockKind> from_string_view<anthropic::RedactedThinkingBlockKind>(std::string_view sv) {
    if (sv == "redacted_thinking") return anthropic::RedactedThinkingBlockKind::REDACTED_THINKING;
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
    if (sv == "text") return anthropic::ResponseContentBlockKinds::TEXT;
    if (sv == "thinking") return anthropic::ResponseContentBlockKinds::THINKING;
    if (sv == "redacted_thinking") return anthropic::ResponseContentBlockKinds::REDACTED_THINKING;
    if (sv == "tool_use") return anthropic::ResponseContentBlockKinds::TOOL_USE;
    if (sv == "server_tool_use") return anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE;
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
    if (sv == "user") return anthropic::Role::USER;
    if (sv == "assistant") return anthropic::Role::ASSISTANT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::SearchResultBlockKind> from_string_view<anthropic::SearchResultBlockKind>(std::string_view sv) {
    if (sv == "search_result") return anthropic::SearchResultBlockKind::SEARCH_RESULT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::SearchResultLocationKind> from_string_view<anthropic::SearchResultLocationKind>(std::string_view sv) {
    if (sv == "search_result_location") return anthropic::SearchResultLocationKind::SEARCH_RESULT_LOCATION;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ServerToolUseBlockKind> from_string_view<anthropic::ServerToolUseBlockKind>(std::string_view sv) {
    if (sv == "server_tool_use") return anthropic::ServerToolUseBlockKind::SERVER_TOOL_USE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::StopReason> from_string_view<anthropic::StopReason>(std::string_view sv) {
    if (sv == "end_turn") return anthropic::StopReason::END_TURN;
    if (sv == "max_tokens") return anthropic::StopReason::MAX_TOKENS;
    if (sv == "stop_sequence") return anthropic::StopReason::STOP_SEQUENCE;
    if (sv == "tool_use") return anthropic::StopReason::TOOL_USE;
    if (sv == "pause_turn") return anthropic::StopReason::PAUSE_TURN;
    if (sv == "refusal") return anthropic::StopReason::REFUSAL;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::StructuredOutputFormatKind> from_string_view<anthropic::StructuredOutputFormatKind>(std::string_view sv) {
    if (sv == "json_schema") return anthropic::StructuredOutputFormatKind::JSON_SCHEMA;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::TextBlockKind> from_string_view<anthropic::TextBlockKind>(std::string_view sv) {
    if (sv == "text") return anthropic::TextBlockKind::TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ThinkingBlockKind> from_string_view<anthropic::ThinkingBlockKind>(std::string_view sv) {
    if (sv == "thinking") return anthropic::ThinkingBlockKind::THINKING;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ThinkingConfigType> from_string_view<anthropic::ThinkingConfigType>(std::string_view sv) {
    if (sv == "enabled") return anthropic::ThinkingConfigType::ENABLED;
    if (sv == "disabled") return anthropic::ThinkingConfigType::DISABLED;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolBash20250124Name> from_string_view<anthropic::ToolBash20250124Name>(std::string_view sv) {
    if (sv == "bash") return anthropic::ToolBash20250124Name::BASH;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolBash20250124Type> from_string_view<anthropic::ToolBash20250124Type>(std::string_view sv) {
    if (sv == "bash_20250124") return anthropic::ToolBash20250124Type::BASH_20250124;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolChoiceAnyKind> from_string_view<anthropic::ToolChoiceAnyKind>(std::string_view sv) {
    if (sv == "any") return anthropic::ToolChoiceAnyKind::ANY;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolChoiceAutoKind> from_string_view<anthropic::ToolChoiceAutoKind>(std::string_view sv) {
    if (sv == "auto") return anthropic::ToolChoiceAutoKind::AUTO;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolChoiceNoneKind> from_string_view<anthropic::ToolChoiceNoneKind>(std::string_view sv) {
    if (sv == "none") return anthropic::ToolChoiceNoneKind::NONE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolChoiceToolKind> from_string_view<anthropic::ToolChoiceToolKind>(std::string_view sv) {
    if (sv == "tool") return anthropic::ToolChoiceToolKind::TOOL;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolResultBlockKind> from_string_view<anthropic::ToolResultBlockKind>(std::string_view sv) {
    if (sv == "tool_result") return anthropic::ToolResultBlockKind::TOOL_RESULT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolTextEditor20250124Name> from_string_view<anthropic::ToolTextEditor20250124Name>(std::string_view sv) {
    if (sv == "text_editor_20250124") return anthropic::ToolTextEditor20250124Name::TEXT_EDITOR_20250124;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolTextEditor20250429Name> from_string_view<anthropic::ToolTextEditor20250429Name>(std::string_view sv) {
    if (sv == "text_editor_20250429") return anthropic::ToolTextEditor20250429Name::TEXT_EDITOR_20250429;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolTextEditor20250728Name> from_string_view<anthropic::ToolTextEditor20250728Name>(std::string_view sv) {
    if (sv == "text_editor_20250728") return anthropic::ToolTextEditor20250728Name::TEXT_EDITOR_20250728;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::ToolUseBlockKind> from_string_view<anthropic::ToolUseBlockKind>(std::string_view sv) {
    if (sv == "tool_use") return anthropic::ToolUseBlockKind::TOOL_USE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::UrlSourceKind> from_string_view<anthropic::UrlSourceKind>(std::string_view sv) {
    if (sv == "url") return anthropic::UrlSourceKind::URL;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::UsageServiceTier> from_string_view<anthropic::UsageServiceTier>(std::string_view sv) {
    if (sv == "standard") return anthropic::UsageServiceTier::STANDARD;
    if (sv == "priority") return anthropic::UsageServiceTier::PRIORITY;
    if (sv == "batch") return anthropic::UsageServiceTier::BATCH;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::UserLocationTypeKind> from_string_view<anthropic::UserLocationTypeKind>(std::string_view sv) {
    if (sv == "approximate") return anthropic::UserLocationTypeKind::APPROXIMATE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchName> from_string_view<anthropic::WebSearchName>(std::string_view sv) {
    if (sv == "web_search") return anthropic::WebSearchName::WEB_SEARCH;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchResultLocationKind> from_string_view<anthropic::WebSearchResultLocationKind>(std::string_view sv) {
    if (sv == "web_search_result_location") return anthropic::WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchTool20250305Kind> from_string_view<anthropic::WebSearchTool20250305Kind>(std::string_view sv) {
    if (sv == "web_search_20250305") return anthropic::WebSearchTool20250305Kind::WEB_SEARCH_20250305;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchToolResultBlockKind> from_string_view<anthropic::WebSearchToolResultBlockKind>(std::string_view sv) {
    if (sv == "web_search_tool_result") return anthropic::WebSearchToolResultBlockKind::WEB_SEARCH_TOOL_RESULT;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchToolResultErrorCode> from_string_view<anthropic::WebSearchToolResultErrorCode>(std::string_view sv) {
    if (sv == "invalid_tool_input") return anthropic::WebSearchToolResultErrorCode::INVALID_TOOL_INPUT;
    if (sv == "unavailable") return anthropic::WebSearchToolResultErrorCode::UNAVAILABLE;
    if (sv == "max_uses_exceeded") return anthropic::WebSearchToolResultErrorCode::MAX_USES_EXCEEDED;
    if (sv == "too_many_requests") return anthropic::WebSearchToolResultErrorCode::TOO_MANY_REQUESTS;
    if (sv == "query_too_long") return anthropic::WebSearchToolResultErrorCode::QUERY_TOO_LONG;
    if (sv == "request_too_large") return anthropic::WebSearchToolResultErrorCode::REQUEST_TOO_LARGE;
    return std::nullopt;
}

template <>
constexpr std::optional<anthropic::WebSearchToolResultErrorType> from_string_view<anthropic::WebSearchToolResultErrorType>(std::string_view sv) {
    if (sv == "web_search_tool_result_error") return anthropic::WebSearchToolResultErrorType::WEB_SEARCH_TOOL_RESULT_ERROR;
    return std::nullopt;
}


/***
 * to_string_view Overloads
 */

constexpr std::string_view to_string_view(anthropic::Base64SourceKind val) {
    switch (val) {
        case anthropic::Base64SourceKind::BASE64: return "base64";
        default: throw std::logic_error("invalid anthropic::Base64SourceKind");
    }
}

constexpr std::string_view to_string_view(anthropic::CacheControlEphemeralKind val) {
    switch (val) {
        case anthropic::CacheControlEphemeralKind::EPHEMERAL: return "ephemeral";
        default: throw std::logic_error("invalid anthropic::CacheControlEphemeralKind");
    }
}

constexpr std::string_view to_string_view(anthropic::CacheControlTTL val) {
    switch (val) {
        case anthropic::CacheControlTTL::TTL_5M: return "5m";
        case anthropic::CacheControlTTL::TTL_1H: return "1h";
        default: throw std::logic_error("invalid anthropic::CacheControlTTL");
    }
}

constexpr std::string_view to_string_view(anthropic::CharLocationKind val) {
    switch (val) {
        case anthropic::CharLocationKind::CHAR_LOCATION: return "char_location";
        default: throw std::logic_error("invalid anthropic::CharLocationKind");
    }
}

constexpr std::string_view to_string_view(anthropic::CitationKinds val) {
    switch (val) {
        case anthropic::CitationKinds::CHAR_LOCATION: return "char_location";
        case anthropic::CitationKinds::CONTENT_BLOCK_LOCATION: return "content_block_location";
        case anthropic::CitationKinds::PAGE_LOCATION: return "page_location";
        case anthropic::CitationKinds::SEARCH_RESULT_LOCATION: return "search_result_location";
        case anthropic::CitationKinds::WEB_SEARCH_RESULT_LOCATION: return "web_search_result_location";
        default: throw std::logic_error("invalid anthropic::CitationKinds");
    }
}

constexpr std::string_view to_string_view(anthropic::ContentBlockLocationKind val) {
    switch (val) {
        case anthropic::ContentBlockLocationKind::CONTENT_BLOCK_LOCATION: return "content_block_location";
        default: throw std::logic_error("invalid anthropic::ContentBlockLocationKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ContentSourceKind val) {
    switch (val) {
        case anthropic::ContentSourceKind::CONTENT: return "content";
        default: throw std::logic_error("invalid anthropic::ContentSourceKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ContentUnitKind val) {
    switch (val) {
        case anthropic::ContentUnitKind::TEXT: return "text";
        case anthropic::ContentUnitKind::IMAGE: return "image";
        default: throw std::logic_error("invalid anthropic::ContentUnitKind");
    }
}

constexpr std::string_view to_string_view(anthropic::CustomToolKind val) {
    switch (val) {
        case anthropic::CustomToolKind::CUSTOM: return "custom";
        default: throw std::logic_error("invalid anthropic::CustomToolKind");
    }
}

constexpr std::string_view to_string_view(anthropic::DocSrcKind val) {
    switch (val) {
        case anthropic::DocSrcKind::BLOCK: return "content";
        case anthropic::DocSrcKind::PDF: return "base64";
        case anthropic::DocSrcKind::PLAIN_TEXT: return "text";
        case anthropic::DocSrcKind::URL_PDF: return "url";
        default: throw std::logic_error("invalid anthropic::DocSrcKind");
    }
}

constexpr std::string_view to_string_view(anthropic::DocumentBlockKind val) {
    switch (val) {
        case anthropic::DocumentBlockKind::DOCUMENT: return "document";
        default: throw std::logic_error("invalid anthropic::DocumentBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ImageBlockKind val) {
    switch (val) {
        case anthropic::ImageBlockKind::IMAGE: return "image";
        default: throw std::logic_error("invalid anthropic::ImageBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ImageMediaType val) {
    switch (val) {
        case anthropic::ImageMediaType::IMAGE_JPEG: return "image/jpeg";
        case anthropic::ImageMediaType::IMAGE_PNG: return "image/png";
        case anthropic::ImageMediaType::IMAGE_GIF: return "image/gif";
        case anthropic::ImageMediaType::IMAGE_WEBP: return "image/webp";
        default: throw std::logic_error("invalid anthropic::ImageMediaType");
    }
}

constexpr std::string_view to_string_view(anthropic::ImageSourceKinds val) {
    switch (val) {
        case anthropic::ImageSourceKinds::BASE64: return "base64";
        case anthropic::ImageSourceKinds::URL: return "url";
        default: throw std::logic_error("invalid anthropic::ImageSourceKinds");
    }
}

constexpr std::string_view to_string_view(anthropic::JsonSchemaTypeKind val) {
    switch (val) {
        case anthropic::JsonSchemaTypeKind::OBJECT: return "object";
        default: throw std::logic_error("invalid anthropic::JsonSchemaTypeKind");
    }
}

constexpr std::string_view to_string_view(anthropic::MessageType val) {
    switch (val) {
        case anthropic::MessageType::MESSAGE: return "message";
        default: throw std::logic_error("invalid anthropic::MessageType");
    }
}

constexpr std::string_view to_string_view(anthropic::PageLocationKind val) {
    switch (val) {
        case anthropic::PageLocationKind::PAGE_LOCATION: return "page_location";
        default: throw std::logic_error("invalid anthropic::PageLocationKind");
    }
}

constexpr std::string_view to_string_view(anthropic::PDFMediaType val) {
    switch (val) {
        case anthropic::PDFMediaType::APPLICATION_PDF: return "application/pdf";
        default: throw std::logic_error("invalid anthropic::PDFMediaType");
    }
}

constexpr std::string_view to_string_view(anthropic::PlainTextMediaType val) {
    switch (val) {
        case anthropic::PlainTextMediaType::TEXT_PLAIN: return "text/plain";
        default: throw std::logic_error("invalid anthropic::PlainTextMediaType");
    }
}

constexpr std::string_view to_string_view(anthropic::PlainTextSourceKind val) {
    switch (val) {
        case anthropic::PlainTextSourceKind::TEXT: return "text";
        default: throw std::logic_error("invalid anthropic::PlainTextSourceKind");
    }
}

constexpr std::string_view to_string_view(anthropic::RedactedThinkingBlockKind val) {
    switch (val) {
        case anthropic::RedactedThinkingBlockKind::REDACTED_THINKING: return "redacted_thinking";
        default: throw std::logic_error("invalid anthropic::RedactedThinkingBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ReplaceBasedEditor val) {
    switch (val) {
        case anthropic::ReplaceBasedEditor::STRING: return "str_replace_based_edit_tool";
        default: throw std::logic_error("invalid anthropic::ReplaceBasedEditor");
    }
}

constexpr std::string_view to_string_view(anthropic::ReplaceEditor val) {
    switch (val) {
        case anthropic::ReplaceEditor::STRING: return "str_replace_editor";
        default: throw std::logic_error("invalid anthropic::ReplaceEditor");
    }
}

constexpr std::string_view to_string_view(anthropic::RequestServiceTier val) {
    switch (val) {
        case anthropic::RequestServiceTier::AUTO: return "auto";
        case anthropic::RequestServiceTier::STANDARD_ONLY: return "standard_only";
        default: throw std::logic_error("invalid anthropic::RequestServiceTier");
    }
}

constexpr std::string_view to_string_view(anthropic::ResponseContentBlockKinds val) {
    switch (val) {
        case anthropic::ResponseContentBlockKinds::TEXT: return "text";
        case anthropic::ResponseContentBlockKinds::THINKING: return "thinking";
        case anthropic::ResponseContentBlockKinds::REDACTED_THINKING: return "redacted_thinking";
        case anthropic::ResponseContentBlockKinds::TOOL_USE: return "tool_use";
        case anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE: return "server_tool_use";
        case anthropic::ResponseContentBlockKinds::WEB_SEARCH_TOOL_RESULT: return "web_search_tool_result";
        default: throw std::logic_error("invalid anthropic::ResponseContentBlockKinds");
    }
}

constexpr std::string_view to_string_view(anthropic::ResponseRole val) {
    switch (val) {
        case anthropic::ResponseRole::ASSISTANT: return "assistant";
        default: throw std::logic_error("invalid anthropic::ResponseRole");
    }
}

constexpr std::string_view to_string_view(anthropic::Role val) {
    switch (val) {
        case anthropic::Role::USER: return "user";
        case anthropic::Role::ASSISTANT: return "assistant";
        default: throw std::logic_error("invalid anthropic::Role");
    }
}

constexpr std::string_view to_string_view(anthropic::SearchResultBlockKind val) {
    switch (val) {
        case anthropic::SearchResultBlockKind::SEARCH_RESULT: return "search_result";
        default: throw std::logic_error("invalid anthropic::SearchResultBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::SearchResultLocationKind val) {
    switch (val) {
        case anthropic::SearchResultLocationKind::SEARCH_RESULT_LOCATION: return "search_result_location";
        default: throw std::logic_error("invalid anthropic::SearchResultLocationKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ServerToolUseBlockKind val) {
    switch (val) {
        case anthropic::ServerToolUseBlockKind::SERVER_TOOL_USE: return "server_tool_use";
        default: throw std::logic_error("invalid anthropic::ServerToolUseBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::StopReason val) {
    switch (val) {
        case anthropic::StopReason::END_TURN: return "end_turn";
        case anthropic::StopReason::MAX_TOKENS: return "max_tokens";
        case anthropic::StopReason::STOP_SEQUENCE: return "stop_sequence";
        case anthropic::StopReason::TOOL_USE: return "tool_use";
        case anthropic::StopReason::PAUSE_TURN: return "pause_turn";
        case anthropic::StopReason::REFUSAL: return "refusal";
        default: throw std::logic_error("invalid anthropic::StopReason");
    }
}

constexpr std::string_view to_string_view(anthropic::StructuredOutputFormatKind val) {
    switch (val) {
        case anthropic::StructuredOutputFormatKind::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid anthropic::StructuredOutputFormatKind");
    }
}

constexpr std::string_view to_string_view(anthropic::TextBlockKind val) {
    switch (val) {
        case anthropic::TextBlockKind::TEXT: return "text";
        default: throw std::logic_error("invalid anthropic::TextBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ThinkingBlockKind val) {
    switch (val) {
        case anthropic::ThinkingBlockKind::THINKING: return "thinking";
        default: throw std::logic_error("invalid anthropic::ThinkingBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ThinkingConfigType val) {
    switch (val) {
        case anthropic::ThinkingConfigType::ENABLED: return "enabled";
        case anthropic::ThinkingConfigType::DISABLED: return "disabled";
        default: throw std::logic_error("invalid anthropic::ThinkingConfigType");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolBash20250124Name val) {
    switch (val) {
        case anthropic::ToolBash20250124Name::BASH: return "bash";
        default: throw std::logic_error("invalid anthropic::ToolBash20250124Name");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolBash20250124Type val) {
    switch (val) {
        case anthropic::ToolBash20250124Type::BASH_20250124: return "bash_20250124";
        default: throw std::logic_error("invalid anthropic::ToolBash20250124Type");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolChoiceAnyKind val) {
    switch (val) {
        case anthropic::ToolChoiceAnyKind::ANY: return "any";
        default: throw std::logic_error("invalid anthropic::ToolChoiceAnyKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolChoiceAutoKind val) {
    switch (val) {
        case anthropic::ToolChoiceAutoKind::AUTO: return "auto";
        default: throw std::logic_error("invalid anthropic::ToolChoiceAutoKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolChoiceNoneKind val) {
    switch (val) {
        case anthropic::ToolChoiceNoneKind::NONE: return "none";
        default: throw std::logic_error("invalid anthropic::ToolChoiceNoneKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolChoiceToolKind val) {
    switch (val) {
        case anthropic::ToolChoiceToolKind::TOOL: return "tool";
        default: throw std::logic_error("invalid anthropic::ToolChoiceToolKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolResultBlockKind val) {
    switch (val) {
        case anthropic::ToolResultBlockKind::TOOL_RESULT: return "tool_result";
        default: throw std::logic_error("invalid anthropic::ToolResultBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolTextEditor20250124Name val) {
    switch (val) {
        case anthropic::ToolTextEditor20250124Name::TEXT_EDITOR_20250124: return "text_editor_20250124";
        default: throw std::logic_error("invalid anthropic::ToolTextEditor20250124Name");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolTextEditor20250429Name val) {
    switch (val) {
        case anthropic::ToolTextEditor20250429Name::TEXT_EDITOR_20250429: return "text_editor_20250429";
        default: throw std::logic_error("invalid anthropic::ToolTextEditor20250429Name");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolTextEditor20250728Name val) {
    switch (val) {
        case anthropic::ToolTextEditor20250728Name::TEXT_EDITOR_20250728: return "text_editor_20250728";
        default: throw std::logic_error("invalid anthropic::ToolTextEditor20250728Name");
    }
}

constexpr std::string_view to_string_view(anthropic::ToolUseBlockKind val) {
    switch (val) {
        case anthropic::ToolUseBlockKind::TOOL_USE: return "tool_use";
        default: throw std::logic_error("invalid anthropic::ToolUseBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::UrlSourceKind val) {
    switch (val) {
        case anthropic::UrlSourceKind::URL: return "url";
        default: throw std::logic_error("invalid anthropic::UrlSourceKind");
    }
}

constexpr std::string_view to_string_view(anthropic::UsageServiceTier val) {
    switch (val) {
        case anthropic::UsageServiceTier::STANDARD: return "standard";
        case anthropic::UsageServiceTier::PRIORITY: return "priority";
        case anthropic::UsageServiceTier::BATCH: return "batch";
        default: throw std::logic_error("invalid anthropic::UsageServiceTier");
    }
}

constexpr std::string_view to_string_view(anthropic::UserLocationTypeKind val) {
    switch (val) {
        case anthropic::UserLocationTypeKind::APPROXIMATE: return "approximate";
        default: throw std::logic_error("invalid anthropic::UserLocationTypeKind");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchName val) {
    switch (val) {
        case anthropic::WebSearchName::WEB_SEARCH: return "web_search";
        default: throw std::logic_error("invalid anthropic::WebSearchName");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchResultLocationKind val) {
    switch (val) {
        case anthropic::WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION: return "web_search_result_location";
        default: throw std::logic_error("invalid anthropic::WebSearchResultLocationKind");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchTool20250305Kind val) {
    switch (val) {
        case anthropic::WebSearchTool20250305Kind::WEB_SEARCH_20250305: return "web_search_20250305";
        default: throw std::logic_error("invalid anthropic::WebSearchTool20250305Kind");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchToolResultBlockKind val) {
    switch (val) {
        case anthropic::WebSearchToolResultBlockKind::WEB_SEARCH_TOOL_RESULT: return "web_search_tool_result";
        default: throw std::logic_error("invalid anthropic::WebSearchToolResultBlockKind");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchToolResultErrorCode val) {
    switch (val) {
        case anthropic::WebSearchToolResultErrorCode::INVALID_TOOL_INPUT: return "invalid_tool_input";
        case anthropic::WebSearchToolResultErrorCode::UNAVAILABLE: return "unavailable";
        case anthropic::WebSearchToolResultErrorCode::MAX_USES_EXCEEDED: return "max_uses_exceeded";
        case anthropic::WebSearchToolResultErrorCode::TOO_MANY_REQUESTS: return "too_many_requests";
        case anthropic::WebSearchToolResultErrorCode::QUERY_TOO_LONG: return "query_too_long";
        case anthropic::WebSearchToolResultErrorCode::REQUEST_TOO_LARGE: return "request_too_large";
        default: throw std::logic_error("invalid anthropic::WebSearchToolResultErrorCode");
    }
}

constexpr std::string_view to_string_view(anthropic::WebSearchToolResultErrorType val) {
    switch (val) {
        case anthropic::WebSearchToolResultErrorType::WEB_SEARCH_TOOL_RESULT_ERROR: return "web_search_tool_result_error";
        default: throw std::logic_error("invalid anthropic::WebSearchToolResultErrorType");
    }
}


}
