#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>

#include "../../interface/protocols/anthropic/messages.hpp"
#include "base.hpp"


using namespace simdjson::builder;
using namespace jai::llm;


namespace simdjson {


TAG_KIND(anthropic::KindBase64Source)
TAG_KIND(anthropic::KindCharLocation)
TAG_KIND(anthropic::KindContentBlockLocation)
TAG_KIND(anthropic::KindContentSource)
TAG_KIND(anthropic::KindCustomTool)
TAG_KIND(anthropic::KindDocumentBlock)
TAG_KIND(anthropic::KindEphemeral)
TAG_KIND(anthropic::KindImageBlock)
TAG_KIND(anthropic::KindJsonSchemaType)
TAG_KIND(anthropic::KindMessage)
TAG_KIND(anthropic::KindPageLocation)
TAG_KIND(anthropic::KindPlainTextSource)
TAG_KIND(anthropic::KindRedactedThinkingBlock)
TAG_KIND(anthropic::KindSearchResultBlock)
TAG_KIND(anthropic::KindSearchResultLocation)
TAG_KIND(anthropic::KindServerToolUseBlock)
TAG_KIND(anthropic::KindStructuredOutputFormat)
TAG_KIND(anthropic::KindTextBlock)
TAG_KIND(anthropic::KindThinkingBlock)
TAG_KIND(anthropic::KindToolBash20250124Type)
TAG_KIND(anthropic::KindToolChoiceAny)
TAG_KIND(anthropic::KindToolChoiceAuto)
TAG_KIND(anthropic::KindToolChoiceNone)
TAG_KIND(anthropic::KindToolChoiceTool)
TAG_KIND(anthropic::KindToolResultBlock)
TAG_KIND(anthropic::KindToolTextEditor20250124Name)
TAG_KIND(anthropic::KindToolTextEditor20250429Name)
TAG_KIND(anthropic::KindToolTextEditor20250728Name)
TAG_KIND(anthropic::KindToolUseBlock)
TAG_KIND(anthropic::KindUrlSource)
TAG_KIND(anthropic::KindWebSearchResultLocation)
TAG_KIND(anthropic::KindWebSearchTool20250305)
TAG_KIND(anthropic::KindWebSearchToolResultBlock)
TAG_KIND(anthropic::KindWebSearchToolResultError)

TAG_ENUM(anthropic::CacheControlTTL)
TAG_ENUM(anthropic::CitationKinds)
TAG_ENUM(anthropic::DocSrcKind)
TAG_ENUM(anthropic::ImageMediaType)
TAG_ENUM(anthropic::ImageSourceKinds)
TAG_ENUM(anthropic::PDFMediaType)
TAG_ENUM(anthropic::PlainTextMediaType)
TAG_ENUM(anthropic::ReplaceBasedEditor)
TAG_ENUM(anthropic::ReplaceEditor)
TAG_ENUM(anthropic::RequestServiceTier)
TAG_ENUM(anthropic::ResponseContentBlockKinds)
TAG_ENUM(anthropic::ResponseRole)
TAG_ENUM(anthropic::Role)
TAG_ENUM(anthropic::StopReason)
TAG_ENUM(anthropic::ThinkingConfigType)
TAG_ENUM(anthropic::ToolBash20250124Name)
TAG_ENUM(anthropic::UsageServiceTier)
TAG_ENUM(anthropic::UserLocationType)
TAG_ENUM(anthropic::WebSearchName)
TAG_ENUM(anthropic::WebSearchToolResultErrorCode)
TAG_ENUM(anthropic::WebSearchToolResultErrorType)


/***
 * Request Shared Substructures
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CacheControlEphemeral& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"ttl",  CommaDirection::BEFORE>(builder, obj.ttl);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Metadata& obj) {
    builder.start_object();
    AddOptKV<"user_id", CommaDirection::NONE>(builder, obj.user_id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationsConfigParam& obj) {
    builder.start_object();
    AddOptKV<"enabled", CommaDirection::NONE>(builder, obj.enabled);
    builder.end_object();
}


/***
 * Request Citations
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationCharLocationParam& obj) {
    builder.start_object();
    AddReqKV<"type",             CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"cited_text",       CommaDirection::BEFORE>(builder, obj.cited_text);
    AddReqKV<"document_index",   CommaDirection::BEFORE>(builder, obj.document_index);
    AddReqKV<"document_title",   CommaDirection::BEFORE>(builder, obj.document_title);
    AddReqKV<"end_char_index",   CommaDirection::BEFORE>(builder, obj.end_char_index);
    AddReqKV<"start_char_index", CommaDirection::BEFORE>(builder, obj.start_char_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationPageLocationParam& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"cited_text",        CommaDirection::BEFORE>(builder, obj.cited_text);
    AddReqKV<"document_index",    CommaDirection::BEFORE>(builder, obj.document_index);
    AddReqKV<"document_title",    CommaDirection::BEFORE>(builder, obj.document_title);
    AddReqKV<"end_page_number",   CommaDirection::BEFORE>(builder, obj.end_page_number);
    AddReqKV<"start_page_number", CommaDirection::BEFORE>(builder, obj.start_page_number);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationContentBlockLocationParam& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"cited_text",        CommaDirection::BEFORE>(builder, obj.cited_text);
    AddReqKV<"document_index",    CommaDirection::BEFORE>(builder, obj.document_index);
    AddReqKV<"document_title",    CommaDirection::BEFORE>(builder, obj.document_title);
    AddReqKV<"end_block_index",   CommaDirection::BEFORE>(builder, obj.end_block_index);
    AddReqKV<"start_block_index", CommaDirection::BEFORE>(builder, obj.start_block_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationWebSearchResultLocationParam& obj) {
    builder.start_object();
    AddReqKV<"type",            CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"cited_text",      CommaDirection::BEFORE>(builder, obj.cited_text);
    AddReqKV<"encrypted_index", CommaDirection::BEFORE>(builder, obj.encrypted_index);
    AddReqKV<"title",           CommaDirection::BEFORE>(builder, obj.title);
    AddReqKV<"url",             CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationSearchResultLocationParam& obj) {
    builder.start_object();
    AddReqKV<"type",                CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"cited_text",          CommaDirection::BEFORE>(builder, obj.cited_text);
    AddReqKV<"end_block_index",     CommaDirection::BEFORE>(builder, obj.end_block_index);
    AddReqKV<"search_result_index", CommaDirection::BEFORE>(builder, obj.search_result_index);
    AddReqKV<"source",              CommaDirection::BEFORE>(builder, obj.source);
    AddReqKV<"start_block_index",   CommaDirection::BEFORE>(builder, obj.start_block_index);
    AddReqKV<"title",               CommaDirection::BEFORE>(builder, obj.title);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::TextCitationParam& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}


/***
 * Request Content Blocks
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Base64ImageSource& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"data",       CommaDirection::BEFORE>(builder, obj.data);
    AddReqKV<"media_type", CommaDirection::BEFORE>(builder, obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::URLImageSource& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"url",  CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ImageSource& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::TextBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"text",          CommaDirection::BEFORE>(builder, obj.text);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ImageBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"source",        CommaDirection::BEFORE>(builder, obj.source);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Base64PDFSource& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"data",       CommaDirection::BEFORE>(builder, obj.data);
    AddReqKV<"media_type", CommaDirection::BEFORE>(builder, obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::PlainTextSource& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"data",       CommaDirection::BEFORE>(builder, obj.data);
    AddReqKV<"media_type", CommaDirection::BEFORE>(builder, obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::URLPDFSource& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"url",  CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ContentBlockSource& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"content", CommaDirection::NONE>(builder, x); }, obj.content);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder,
                const anthropic::ContentBlockSource::ContentBlockSourceContent& obj)
{
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::DocumentSource& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::DocumentBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"source",        CommaDirection::BEFORE>(builder, obj.source);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    AddOptKV<"context",       CommaDirection::BEFORE>(builder, obj.context);
    AddOptKV<"title",         CommaDirection::BEFORE>(builder, obj.title);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::SearchResultBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"content",       CommaDirection::BEFORE>(builder, obj.content);
    AddReqKV<"source",        CommaDirection::BEFORE>(builder, obj.source);
    AddReqKV<"title",         CommaDirection::BEFORE>(builder, obj.title);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",      CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"signature", CommaDirection::BEFORE>(builder, obj.signature);
    AddReqKV<"thinking",  CommaDirection::BEFORE>(builder, obj.thinking);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::RedactedThinkingBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"data", CommaDirection::BEFORE>(builder, obj.data);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolUseBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",            CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"input",         CommaDirection::BEFORE>(builder, obj.input);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolResultBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"tool_use_id",   CommaDirection::BEFORE>(builder, obj.tool_use_id);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    if (obj.content) {
        builder.append_comma();
        std::visit([&](auto const& x) { AddReqKV<"content", CommaDirection::NONE>(builder, x); }, *obj.content);
    }
    AddOptKV<"is_error", CommaDirection::BEFORE>(builder, obj.is_error);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolResultBlockParam::ContentUnit& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ServerToolUseBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddReqKV<"id",            CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"input",         CommaDirection::BEFORE>(builder, obj.input);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam::WebSearchResultBlockParamItem& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"encrypted_content", CommaDirection::BEFORE>(builder, obj.encrypted_content);
    AddReqKV<"title",             CommaDirection::BEFORE>(builder, obj.title);
    AddReqKV<"url",               CommaDirection::BEFORE>(builder, obj.url);
    AddOptKV<"page_age",          CommaDirection::BEFORE>(builder, obj.page_age);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam::WebSearchToolRequestError& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"error_code", CommaDirection::BEFORE>(builder, obj.error_code);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    std::visit([&](auto const& x) { AddReqKV<"content", CommaDirection::BEFORE>(builder, x); }, obj.content);
    AddReqKV<"tool_use_id",   CommaDirection::BEFORE>(builder, obj.tool_use_id);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ContentBlockParam& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Definitions
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Tool::InputSchema& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"properties", CommaDirection::BEFORE>(builder, obj.properties);
    AddOptKV<"required",   CommaDirection::BEFORE>(builder, obj.required);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Tool& obj) {
    builder.start_object();
    AddReqKV<"input_schema",  CommaDirection::NONE>  (builder, obj.input_schema);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"description",   CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"strict",        CommaDirection::BEFORE>(builder, obj.strict);
    AddOptKV<"type",          CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolBash20250124& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250124& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250429& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250728& obj) {
    builder.start_object();
    AddReqKV<"type",          CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",          CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchTool20250305::UserLocation& obj) {
    builder.start_object();
    AddReqKV<"type",     CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchTool20250305& obj) {
    builder.start_object();
    AddReqKV<"type",            CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",            CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"allowed_domains", CommaDirection::BEFORE>(builder, obj.allowed_domains);
    AddOptKV<"blocked_domains", CommaDirection::BEFORE>(builder, obj.blocked_domains);
    AddOptKV<"cache_control",   CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"max_uses",        CommaDirection::BEFORE>(builder, obj.max_uses);
    AddOptKV<"strict",          CommaDirection::BEFORE>(builder, obj.strict);
    AddOptKV<"user_location",   CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolUnion& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Choice
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceAuto& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceAny& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceTool& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name", CommaDirection::BEFORE>(builder, obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceNone& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoice& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}


/***
 * Request Infrastructure
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::OutputConfig::Format& obj) {
    builder.start_object();
    AddReqKV<"schema", CommaDirection::NONE>  (builder, obj.schema);
    AddReqKV<"type",   CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::OutputConfig& obj) {
    builder.start_object();
    AddOptKV<"format", CommaDirection::NONE>(builder, obj.format);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfigEnabled& obj) {
    builder.start_object();
    AddReqKV<"budget_tokens", CommaDirection::NONE>  (builder, obj.budget_tokens);
    AddReqKV<"type",          CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfigDisabled& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfig& obj) {
    std::visit([&](auto const& x) { simdjson::tag_invoke(simdjson::serialize_tag{}, builder, x); }, obj);
}


/***
 * Message Request
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::MessageParam& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { AddReqKV<"content", CommaDirection::NONE>(builder, x); }, obj.content);
    AddReqKV<"role", CommaDirection::BEFORE>(builder, obj.role);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Request& obj) {
    builder.start_object();
    AddReqKV<"max_tokens",     CommaDirection::NONE>  (builder, obj.max_tokens);
    AddReqKV<"messages",       CommaDirection::BEFORE>(builder, obj.messages);
    AddReqKV<"model",          CommaDirection::BEFORE>(builder, obj.model);
    AddOptKV<"metadata",       CommaDirection::BEFORE>(builder, obj.metadata);
    AddOptKV<"output_config",  CommaDirection::BEFORE>(builder, obj.output_config);
    AddOptKV<"service_tier",   CommaDirection::BEFORE>(builder, obj.service_tier);
    AddOptKV<"stop_sequences", CommaDirection::BEFORE>(builder, obj.stop_sequences);
    AddOptKV<"stream",         CommaDirection::BEFORE>(builder, obj.stream);
    if (obj.system) {
        std::visit([&](auto const& x) { AddReqKV<"system", CommaDirection::BEFORE>(builder, x); }, *obj.system);
    }
    AddOptKV<"temperature",    CommaDirection::BEFORE>(builder, obj.temperature);
    AddOptKV<"thinking",       CommaDirection::BEFORE>(builder, obj.thinking);
    AddOptKV<"tool_choice",    CommaDirection::BEFORE>(builder, obj.tool_choice);
    AddOptKV<"tools",          CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"top_k",          CommaDirection::BEFORE>(builder, obj.top_k);
    AddOptKV<"top_p",          CommaDirection::BEFORE>(builder, obj.top_p);
    builder.end_object();
}


} // namespace jai::llm


#undef TAG_ENUM
#undef TAG_KIND


/***
 * Top-level Serialize
 */
namespace jai::llm::anthropic {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    builder.clear();
    simdjson::tag_invoke(simdjson::serialize_tag{}, builder, request);
    builder.validate_unicode();
    std::string_view json_str = builder.view();

    return json_str |
           std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
           std::ranges::to<std::vector<std::byte>>();
}


}
