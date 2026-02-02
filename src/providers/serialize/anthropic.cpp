#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>

#include "../../interface/protocols/anthropic/messages.hpp"
#include "../../interface/protocols/anthropic/strings.hpp" // must include before base.hpp
#include "base.hpp"


using namespace simdjson;
using namespace builder;


namespace jai::llm {


/***
 * Request Shared Substructures
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CacheControlEphemeral& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"ttl", CommaDirection::BEFORE>(builder, obj.ttl);
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
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"cited_text">(obj.cited_text);
    builder.append_comma();
    builder.append_key_value<"document_index">(obj.document_index);
    builder.append_comma();
    builder.append_key_value<"document_title">(obj.document_title);
    builder.append_comma();
    builder.append_key_value<"end_char_index">(obj.end_char_index);
    builder.append_comma();
    builder.append_key_value<"start_char_index">(obj.start_char_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationPageLocationParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"cited_text">(obj.cited_text);
    builder.append_comma();
    builder.append_key_value<"document_index">(obj.document_index);
    builder.append_comma();
    builder.append_key_value<"document_title">(obj.document_title);
    builder.append_comma();
    builder.append_key_value<"end_page_number">(obj.end_page_number);
    builder.append_comma();
    builder.append_key_value<"start_page_number">(obj.start_page_number);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationContentBlockLocationParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"cited_text">(obj.cited_text);
    builder.append_comma();
    builder.append_key_value<"document_index">(obj.document_index);
    builder.append_comma();
    builder.append_key_value<"document_title">(obj.document_title);
    builder.append_comma();
    builder.append_key_value<"end_block_index">(obj.end_block_index);
    builder.append_comma();
    builder.append_key_value<"start_block_index">(obj.start_block_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationWebSearchResultLocationParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"cited_text">(obj.cited_text);
    builder.append_comma();
    builder.append_key_value<"encrypted_index">(obj.encrypted_index);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::CitationSearchResultLocationParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"cited_text">(obj.cited_text);
    builder.append_comma();
    builder.append_key_value<"end_block_index">(obj.end_block_index);
    builder.append_comma();
    builder.append_key_value<"search_result_index">(obj.search_result_index);
    builder.append_comma();
    builder.append_key_value<"source">(obj.source);
    builder.append_comma();
    builder.append_key_value<"start_block_index">(obj.start_block_index);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::TextCitationParam& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Request Content Blocks
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Base64ImageSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"data">(obj.data);
    builder.append_comma();
    builder.append_key_value<"media_type">(obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::URLImageSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ImageSource& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::TextBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ImageBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"source">(obj.source);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Base64PDFSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"data">(obj.data);
    builder.append_comma();
    builder.append_key_value<"media_type">(obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::PlainTextSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"data">(obj.data);
    builder.append_comma();
    builder.append_key_value<"media_type">(obj.media_type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::URLPDFSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ContentBlockSource& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) {
        builder.append_key_value<"content">(x);
    }, obj.content);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::DocumentSource& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::DocumentBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"source">(obj.source);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    AddOptKV<"context",       CommaDirection::BEFORE>(builder, obj.context);
    AddOptKV<"title",         CommaDirection::BEFORE>(builder, obj.title);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::SearchResultBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"content">(obj.content);
    builder.append_comma();
    builder.append_key_value<"source">(obj.source);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"citations",     CommaDirection::BEFORE>(builder, obj.citations);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"signature">(obj.signature);
    builder.append_comma();
    builder.append_key_value<"thinking">(obj.thinking);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::RedactedThinkingBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"data">(obj.data);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolUseBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"input">(obj.input);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolResultBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"tool_use_id">(obj.tool_use_id);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    if (obj.content) {
        builder.append_comma();
        std::visit([&](auto const& x) {
            builder.append_key_value<"content">(x);
        }, *obj.content);
    }
    AddOptKV<"is_error", CommaDirection::BEFORE>(builder, obj.is_error);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ServerToolUseBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"input">(obj.input);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam::WebSearchResultBlockParamItem& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"encrypted_content">(obj.encrypted_content);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    AddOptKV<"page_age", CommaDirection::BEFORE>(builder, obj.page_age);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam::WebSearchToolRequestError& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"error_code">(obj.error_code);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchToolResultBlockParam& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) {
        builder.append_key_value<"content">(x);
    }, obj.content);
    builder.append_comma();
    builder.append_key_value<"tool_use_id">(obj.tool_use_id);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ContentBlockParam& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Definitions
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Tool::InputSchema& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"properties", CommaDirection::BEFORE>(builder, obj.properties);
    AddOptKV<"required",   CommaDirection::BEFORE>(builder, obj.required);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Tool& obj) {
    builder.start_object();
    builder.append_key_value<"input_schema">(obj.input_schema);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"description",   CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"strict",        CommaDirection::BEFORE>(builder, obj.strict);
    AddOptKV<"type",          CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolBash20250124& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250124& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250429& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolTextEditor20250728& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"cache_control", CommaDirection::BEFORE>(builder, obj.cache_control);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchTool20250305::UserLocation& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::WebSearchTool20250305& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"allowed_domains", CommaDirection::BEFORE>(builder, obj.allowed_domains);
    AddOptKV<"blocked_domains", CommaDirection::BEFORE>(builder, obj.blocked_domains);
    AddOptKV<"cache_control",   CommaDirection::BEFORE>(builder, obj.cache_control);
    AddOptKV<"max_uses",        CommaDirection::BEFORE>(builder, obj.max_uses);
    AddOptKV<"strict",          CommaDirection::BEFORE>(builder, obj.strict);
    AddOptKV<"user_location",   CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolUnion& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Choice
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceAuto& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceAny& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoiceNone& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ToolChoice& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Request Infrastructure
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::OutputConfig::Format& obj) {
    builder.start_object();
    builder.append_key_value<"schema">(obj.schema);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::OutputConfig& obj) {
    builder.start_object();
    AddOptKV<"format", CommaDirection::NONE>(builder, obj.format);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfigEnabled& obj) {
    builder.start_object();
    builder.append_key_value<"budget_tokens">(obj.budget_tokens);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfigDisabled& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::ThinkingConfig& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Message Request
 */
void tag_invoke(serialize_tag, string_builder& builder, const anthropic::MessageParam& obj) {
    builder.start_object();
    std::visit([&](auto const& x) {
        builder.append_key_value<"content">(x);
    }, obj.content);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const anthropic::Request& obj) {
    builder.start_object();
    builder.append_key_value<"max_tokens">(obj.max_tokens);
    builder.append_comma();
    builder.append_key_value<"messages">(obj.messages);
    builder.append_comma();
    builder.append_key_value<"model">(obj.model);
    AddOptKV<"metadata",      CommaDirection::BEFORE>(builder, obj.metadata);
    AddOptKV<"output_config", CommaDirection::BEFORE>(builder, obj.output_config);
    AddOptKV<"service_tier",  CommaDirection::BEFORE>(builder, obj.service_tier);
    AddOptKV<"stop_sequences", CommaDirection::BEFORE>(builder, obj.stop_sequences);
    AddOptKV<"stream",        CommaDirection::BEFORE>(builder, obj.stream);
    if (obj.system) {
        builder.append_comma();
        std::visit([&](auto const& x) {
            builder.append_key_value<"system">(x);
        }, *obj.system);
    }
    AddOptKV<"temperature",   CommaDirection::BEFORE>(builder, obj.temperature);
    AddOptKV<"thinking",      CommaDirection::BEFORE>(builder, obj.thinking);
    AddOptKV<"tool_choice",   CommaDirection::BEFORE>(builder, obj.tool_choice);
    AddOptKV<"tools",         CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"top_k",         CommaDirection::BEFORE>(builder, obj.top_k);
    AddOptKV<"top_p",         CommaDirection::BEFORE>(builder, obj.top_p);
    builder.end_object();
}


} // namespace jai::llm


/***
 * Top-level Serialize
 */
namespace jai::llm::anthropic {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    builder.clear();
    jai::llm::tag_invoke(serialize_tag{}, builder, request);
    builder.validate_unicode();
    std::string_view json_str = builder.view();

    return json_str |
           std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
           std::ranges::to<std::vector<std::byte>>();
}


}
