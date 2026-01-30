#include "../../../interface/providers/anthropic.hpp"
#include "../../../interface/providers/strings/anthropic.hpp" // must include before base.hpp
#include "base.hpp"
#include "../../curl.hpp"

#include <map>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>


/***
 * Local defined MACROs for source file only.
 */
#define FIELD(src, member) Extract<#member, T, &T::member>((src))
#define BEGIN_PARSE(Type)                             \
template <>                                           \
Type Parse<Type>(const simdjson::dom::element& src) { \
    using T = Type;                                   \
    return T{
#define  END_PARSE \
    };             \
}


namespace jai::llm {


/***
 * Basic and Container Specializations
 */
template <>
json::Object Parse<json::Object>(const simdjson::dom::element& src) {
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        return std::pair{std::string{key}, Parse<json::Value>(value)};
    }) | std::ranges::to<json::Object>();
}

template <>
std::map<std::string, std::string> Parse<std::map<std::string, std::string>>(const simdjson::dom::element& src) {
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        return std::pair{std::string{key}, Parse<std::string>(value)};
    }) | std::ranges::to<std::map<std::string, std::string>>();
}


/***
 * Forward Declarations for complex variants to fix ordering issues.
 */

template <>
anthropic::Citation Parse<anthropic::Citation>(const simdjson::dom::element& src);

template <>
anthropic::ImageSource Parse<anthropic::ImageSource>(const simdjson::dom::element& src);

template <>
anthropic::DocumentSource Parse<anthropic::DocumentSource>(const simdjson::dom::element& src);

template <>
anthropic::ContentBlock Parse<anthropic::ContentBlock>(const simdjson::dom::element& src);

template <>
anthropic::ToolUnion Parse<anthropic::ToolUnion>(const simdjson::dom::element& src);

template <>
anthropic::ToolChoice Parse<anthropic::ToolChoice>(const simdjson::dom::element& src);

template <>
anthropic::ThinkingConfig Parse<anthropic::ThinkingConfig>(const simdjson::dom::element& src);

template <>
anthropic::ContentBlockSource::Content Parse<anthropic::ContentBlockSource::Content>(const simdjson::dom::element& src);

template <>
anthropic::ToolResultBlock::Content Parse<anthropic::ToolResultBlock::Content>(const simdjson::dom::element& src);

template <>
anthropic::MessageParam::Content Parse<anthropic::MessageParam::Content>(const simdjson::dom::element& src);

template <>
std::variant<anthropic::TextBlock, anthropic::ImageBlock>
    Parse<std::variant<anthropic::TextBlock, anthropic::ImageBlock>>(const simdjson::dom::element& src);

template <>
std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>
    Parse<std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>>(const simdjson::dom::element& src);

template <>
anthropic::WebSearchToolResultBlock::Content Parse<anthropic::WebSearchToolResultBlock::Content>(const simdjson::dom::element& src);


/***
 * Shared Substructures (Block 3)
 */

BEGIN_PARSE(anthropic::CacheControlEphemeral)
    FIELD(src, type),
    FIELD(src, ttl)
END_PARSE

BEGIN_PARSE(anthropic::CitationsConfig)
    FIELD(src, enabled)
END_PARSE


/***
 * Citations (Block 4)
 */

BEGIN_PARSE(anthropic::CitationCharLocation)
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_char_index),
    FIELD(src, start_char_index),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::CitationPageLocation)
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_page_number),
    FIELD(src, start_page_number),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::CitationContentBlockLocation)
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_block_index),
    FIELD(src, start_block_index),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::CitationWebSearchResultLocation)
    FIELD(src, cited_text),
    FIELD(src, encrypted_index),
    FIELD(src, title),
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(anthropic::CitationSearchResultLocation)
    FIELD(src, cited_text),
    FIELD(src, end_block_index),
    FIELD(src, search_result_index),
    FIELD(src, source),
    FIELD(src, start_block_index),
    FIELD(src, title),
    FIELD(src, type)
END_PARSE

template <>
anthropic::Citation Parse<anthropic::Citation>(const simdjson::dom::element& src) {
    using T = anthropic::Citation;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "char_location")           return T{Parse<anthropic::CitationCharLocation>(src)};
    if (type_sv == "page_location")           return T{Parse<anthropic::CitationPageLocation>(src)};
    if (type_sv == "content_block_location")  return T{Parse<anthropic::CitationContentBlockLocation>(src)};
    if (type_sv == "web_search_result_location") return T{Parse<anthropic::CitationWebSearchResultLocation>(src)};
    if (type_sv == "search_result_location")  return T{Parse<anthropic::CitationSearchResultLocation>(src)};
    throw std::logic_error{"Citation variant unsatisfied"};
}


/***
 * Message Blocks (Block 5)
 */

BEGIN_PARSE(anthropic::TextBlock)
    FIELD(src, text),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, citations)
END_PARSE

BEGIN_PARSE(anthropic::Base64ImageSource)
    FIELD(src, data),
    FIELD(src, media_type),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::UrlImageSource)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

template <>
anthropic::ImageSource Parse<anthropic::ImageSource>(const simdjson::dom::element& src) {
    using T = anthropic::ImageSource;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "base64") return T{Parse<anthropic::Base64ImageSource>(src)};
    if (type_sv == "url")    return T{Parse<anthropic::UrlImageSource>(src)};
    throw std::logic_error{"ImageSource variant unsatisfied"};
}

BEGIN_PARSE(anthropic::ImageBlock)
    FIELD(src, source),
    FIELD(src, type),
    FIELD(src, cache_control)
END_PARSE


/***
 * Recursive Sources & Documents (Block 6)
 */

template <>
std::variant<anthropic::TextBlock, anthropic::ImageBlock> Parse<std::variant<anthropic::TextBlock, anthropic::ImageBlock>>(const simdjson::dom::element& src) {
    using T = std::variant<anthropic::TextBlock, anthropic::ImageBlock>;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "text")  return T{Parse<anthropic::TextBlock>(src)};
    if (type_sv == "image") return T{Parse<anthropic::ImageBlock>(src)};
    throw std::logic_error{"TextBlock/ImageBlock variant unsatisfied"};
}

template <>
anthropic::ContentBlockSource::Content Parse<anthropic::ContentBlockSource::Content>(const simdjson::dom::element& src) {
    using T = anthropic::ContentBlockSource::Content;
    if (src.is_string()) return T{std::string{src.get_string().value()}};
    using V = std::vector<std::variant<anthropic::TextBlock, anthropic::ImageBlock>>;
    return T{Parse<V>(src)};
}

BEGIN_PARSE(anthropic::ContentBlockSource)
    FIELD(src, content),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::Base64PDFSource)
    FIELD(src, data),
    FIELD(src, media_type),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::PlainTextSource)
    FIELD(src, data),
    FIELD(src, media_type),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::URLPDFSource)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

template <>
anthropic::DocumentSource Parse<anthropic::DocumentSource>(const simdjson::dom::element& src) {
    using T = anthropic::DocumentSource;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "base64")  return T{Parse<anthropic::Base64PDFSource>(src)};
    if (type_sv == "text")    return T{Parse<anthropic::PlainTextSource>(src)};
    if (type_sv == "url")     return T{Parse<anthropic::URLPDFSource>(src)};
    if (type_sv == "content") return T{Parse<anthropic::ContentBlockSource>(src)};
    throw std::logic_error{"DocumentSource variant unsatisfied"};
}

BEGIN_PARSE(anthropic::DocumentBlock)
    FIELD(src, source),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, citations),
    FIELD(src, context),
    FIELD(src, title)
END_PARSE

BEGIN_PARSE(anthropic::SearchResultBlock)
    FIELD(src, content),
    FIELD(src, source),
    FIELD(src, title),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, citations)
END_PARSE


/***
 * Thinking Blocks (Block 7)
 */

BEGIN_PARSE(anthropic::ThinkingBlock)
    FIELD(src, signature),
    FIELD(src, thinking),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::RedactedThinkingBlock)
    FIELD(src, data),
    FIELD(src, type)
END_PARSE


/***
 * Tool Call Blocks (Block 8)
 */

BEGIN_PARSE(anthropic::ToolUseBlock)
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

template <>
std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>
    Parse<std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>>(const simdjson::dom::element& src)
{
    using T = std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "text")          return T{Parse<anthropic::TextBlock>(src)};
    if (type_sv == "image")         return T{Parse<anthropic::ImageBlock>(src)};
    if (type_sv == "search_result") return T{Parse<anthropic::SearchResultBlock>(src)};
    if (type_sv == "document")      return T{Parse<anthropic::DocumentBlock>(src)};
    throw std::logic_error{"ToolResult content element variant unsatisfied"};
}

template <>
anthropic::ToolResultBlock::Content Parse<anthropic::ToolResultBlock::Content>(const simdjson::dom::element& src) {
    using T = anthropic::ToolResultBlock::Content;
    if (src.is_string()) return T{std::string{src.get_string().value()}};
    using E = std::variant<anthropic::TextBlock, anthropic::ImageBlock, anthropic::SearchResultBlock, anthropic::DocumentBlock>;
    return T{Parse<std::vector<E>>(src)};
}

BEGIN_PARSE(anthropic::ToolResultBlock)
    FIELD(src, tool_use_id),
    FIELD(src, type),
    FIELD(src, content),
    FIELD(src, is_error)
END_PARSE

BEGIN_PARSE(anthropic::ServerToolUseBlock)
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchToolResultBlock::WebSearchResultItem)
    FIELD(src, encrypted_content),
    FIELD(src, title),
    FIELD(src, type),
    FIELD(src, url),
    FIELD(src, page_age)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchToolResultBlock::Error)
    FIELD(src, error_code),
    FIELD(src, type)
END_PARSE

template <>
anthropic::WebSearchToolResultBlock::Content Parse<anthropic::WebSearchToolResultBlock::Content>(const simdjson::dom::element& src) {
    using T = anthropic::WebSearchToolResultBlock::Content;
    if (src.is_array()) return T{Parse<std::vector<anthropic::WebSearchToolResultBlock::WebSearchResultItem>>(src)};
    return T{Parse<anthropic::WebSearchToolResultBlock::Error>(src)};
}

BEGIN_PARSE(anthropic::WebSearchToolResultBlock)
    FIELD(src, content),
    FIELD(src, tool_use_id),
    FIELD(src, type)
END_PARSE


/***
 * Messaging (Polymorphic) (Block 9)
 */

template <>
anthropic::ContentBlock Parse<anthropic::ContentBlock>(const simdjson::dom::element& src) {
    using T = anthropic::ContentBlock;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "text")             return T{Parse<anthropic::TextBlock>(src)};
    if (type_sv == "image")            return T{Parse<anthropic::ImageBlock>(src)};
    if (type_sv == "document")         return T{Parse<anthropic::DocumentBlock>(src)};
    if (type_sv == "search_result")    return T{Parse<anthropic::SearchResultBlock>(src)};
    if (type_sv == "thinking")         return T{Parse<anthropic::ThinkingBlock>(src)};
    if (type_sv == "redacted_thinking") return T{Parse<anthropic::RedactedThinkingBlock>(src)};
    if (type_sv == "tool_use")         return T{Parse<anthropic::ToolUseBlock>(src)};
    if (type_sv == "tool_result")      return T{Parse<anthropic::ToolResultBlock>(src)};
    if (type_sv == "server_tool_use")  return T{Parse<anthropic::ServerToolUseBlock>(src)};
    if (type_sv == "web_search_tool_result") return T{Parse<anthropic::WebSearchToolResultBlock>(src)};
    throw std::logic_error{"ContentBlock variant unsatisfied"};
}

template <>
anthropic::MessageParam::Content Parse<anthropic::MessageParam::Content>(const simdjson::dom::element& src) {
    using T = anthropic::MessageParam::Content;
    if (src.is_string()) return T{std::string{src.get_string().value()}};
    return T{Parse<std::vector<anthropic::ContentBlock>>(src)};
}

BEGIN_PARSE(anthropic::MessageParam)
    FIELD(src, content),
    FIELD(src, role)
END_PARSE


/***
 * Tool Definitions (Block 10)
 */

BEGIN_PARSE(anthropic::Tool::InputSchema)
    FIELD(src, type),
    FIELD(src, properties),
    FIELD(src, required)
END_PARSE

BEGIN_PARSE(anthropic::Tool)
    FIELD(src, input_schema),
    FIELD(src, name),
    FIELD(src, cache_control),
    FIELD(src, description),
    FIELD(src, strict),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::ToolBash20250124)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, strict)
END_PARSE

BEGIN_PARSE(anthropic::ToolTextEditor20250124)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, strict)
END_PARSE

BEGIN_PARSE(anthropic::ToolTextEditor20250429)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, strict)
END_PARSE

BEGIN_PARSE(anthropic::ToolTextEditor20250728)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, cache_control),
    FIELD(src, max_characters),
    FIELD(src, strict)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchTool20250305::UserLocation)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchTool20250305)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, allowed_domains),
    FIELD(src, blocked_domains),
    FIELD(src, cache_control),
    FIELD(src, max_uses),
    FIELD(src, strict),
    FIELD(src, user_location)
END_PARSE

template <>
anthropic::ToolUnion Parse<anthropic::ToolUnion>(const simdjson::dom::element& src) {
    using T = anthropic::ToolUnion;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "bash_20250124")            return T{Parse<anthropic::ToolBash20250124>(src)};
    if (type_sv == "text_editor_20250124")     return T{Parse<anthropic::ToolTextEditor20250124>(src)};
    if (type_sv == "text_editor_20250429")     return T{Parse<anthropic::ToolTextEditor20250429>(src)};
    if (type_sv == "text_editor_20250728")     return T{Parse<anthropic::ToolTextEditor20250728>(src)};
    if (type_sv == "web_search_20250305")      return T{Parse<anthropic::WebSearchTool20250305>(src)};
    // Default to Custom Tool
    return T{Parse<anthropic::Tool>(src)};
}


/***
 * Tool Choice models (Block 11)
 */

BEGIN_PARSE(anthropic::ToolChoiceAuto)
    FIELD(src, type),
    FIELD(src, disable_parallel_tool_use)
END_PARSE

BEGIN_PARSE(anthropic::ToolChoiceAny)
    FIELD(src, type),
    FIELD(src, disable_parallel_tool_use)
END_PARSE

BEGIN_PARSE(anthropic::ToolChoiceTool)
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, disable_parallel_tool_use)
END_PARSE

BEGIN_PARSE(anthropic::ToolChoiceNone)
    FIELD(src, type)
END_PARSE

template <>
anthropic::ToolChoice Parse<anthropic::ToolChoice>(const simdjson::dom::element& src) {
    using T = anthropic::ToolChoice;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "auto") return T{Parse<anthropic::ToolChoiceAuto>(src)};
    if (type_sv == "any")  return T{Parse<anthropic::ToolChoiceAny>(src)};
    if (type_sv == "tool") return T{Parse<anthropic::ToolChoiceTool>(src)};
    if (type_sv == "none") return T{Parse<anthropic::ToolChoiceNone>(src)};
    throw std::logic_error{"ToolChoice variant unsatisfied"};
}


/***
 * Infrastructure Root (Block 12)
 */

BEGIN_PARSE(anthropic::Usage::CacheCreation)
    FIELD(src, ephemeral_1h_input_tokens),
    FIELD(src, ephemeral_5m_input_tokens)
END_PARSE

BEGIN_PARSE(anthropic::Usage::ServerToolUsage)
    FIELD(src, web_search_requests)
END_PARSE

BEGIN_PARSE(anthropic::Usage)
    FIELD(src, cache_creation),
    FIELD(src, cache_creation_input_tokens),
    FIELD(src, cache_read_input_tokens),
    FIELD(src, input_tokens),
    FIELD(src, output_tokens),
    FIELD(src, server_tool_use),
    FIELD(src, service_tier)
END_PARSE

BEGIN_PARSE(anthropic::Metadata)
    FIELD(src, user_id)
END_PARSE

BEGIN_PARSE(anthropic::OutputConfig::Format)
    FIELD(src, schema),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::OutputConfig)
    FIELD(src, format)
END_PARSE

BEGIN_PARSE(anthropic::ThinkingConfigEnabled)
    FIELD(src, budget_tokens),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(anthropic::ThinkingConfigDisabled)
    FIELD(src, type)
END_PARSE

template <>
anthropic::ThinkingConfig Parse<anthropic::ThinkingConfig>(const simdjson::dom::element& src) {
    using T = anthropic::ThinkingConfig;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "enabled")  return T{Parse<anthropic::ThinkingConfigEnabled>(src)};
    if (type_sv == "disabled") return T{Parse<anthropic::ThinkingConfigDisabled>(src)};
    throw std::logic_error{"ThinkingConfig variant unsatisfied"};
}

BEGIN_PARSE(anthropic::Request)
    FIELD(src, max_tokens),
    FIELD(src, messages),
    FIELD(src, model),
    FIELD(src, metadata),
    FIELD(src, output_config),
    FIELD(src, stop_sequences),
    FIELD(src, stream),
    FIELD(src, system),
    FIELD(src, temperature),
    FIELD(src, thinking),
    FIELD(src, tool_choice),
    FIELD(src, tools),
    FIELD(src, top_k),
    FIELD(src, top_p)
END_PARSE

BEGIN_PARSE(anthropic::Response)
    FIELD(src, id),
    FIELD(src, content),
    FIELD(src, model),
    FIELD(src, role),
    FIELD(src, stop_reason),
    FIELD(src, stop_sequence),
    FIELD(src, type),
    FIELD(src, usage)
END_PARSE


/***
 * Top-level Deserialize
 */

namespace anthropic {

Response Deserialize(const curl::Response& response) {
    simdjson::dom::parser parser;
    simdjson::dom::element doc = parser.parse(response.body);
    return Parse<Response>(doc);
}

}


}
