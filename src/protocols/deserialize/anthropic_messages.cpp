#include "../../../interface/protocols/anthropic/messages.hpp"
#include "../../../interface//core/error.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


/***
 * Response Citations
 */
BEGIN_PARSE(anthropic::CitationCharLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_char_index),
    FIELD(src, start_char_index),
    FIELD(src, file_id)
END_PARSE

BEGIN_PARSE(anthropic::CitationPageLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_page_number),
    FIELD(src, start_page_number),
    FIELD(src, file_id)
END_PARSE

BEGIN_PARSE(anthropic::CitationContentBlockLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_block_index),
    FIELD(src, start_block_index),
    FIELD(src, file_id)
END_PARSE

BEGIN_PARSE(anthropic::CitationsWebSearchResultLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, encrypted_index),
    FIELD(src, title),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(anthropic::CitationsSearchResultLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, end_block_index),
    FIELD(src, search_result_index),
    FIELD(src, source),
    FIELD(src, start_block_index),
    FIELD(src, title)
END_PARSE


/***
 * Response Content Blocks
 */
BEGIN_PARSE(anthropic::TextBlock)
    FIELD(src, type),
    FIELD(src, citations),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(anthropic::ThinkingBlock)
    FIELD(src, type),
    FIELD(src, signature),
    FIELD(src, thinking)
END_PARSE

BEGIN_PARSE(anthropic::RedactedThinkingBlock)
    FIELD(src, type),
    FIELD(src, data)
END_PARSE

BEGIN_PARSE(anthropic::ToolUseBlock)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name)
END_PARSE

BEGIN_PARSE(anthropic::ServerToolUseBlock)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchToolResultBlock)
    FIELD(src, type),
    FIELD(src, content),
    FIELD(src, tool_use_id)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchToolResultBlock::WebSearchResultBlock)
    FIELD(src, type),
    FIELD(src, encrypted_content),
    FIELD(src, page_age),
    FIELD(src, title),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(anthropic::WebSearchToolResultBlock::WebSearchToolResultError)
    FIELD(src, type),
    FIELD(src, error_code)
END_PARSE


/***
 * Usage and Substructures
 */
BEGIN_PARSE(anthropic::CacheCreation)
    FIELD(src, ephemeral_1h_input_tokens),
    FIELD(src, ephemeral_5m_input_tokens)
END_PARSE

BEGIN_PARSE(anthropic::ServerToolUsage)
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


/***
 * Top-level Response Message
 */
BEGIN_PARSE(anthropic::Response)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, content),
    FIELD(src, model),
    FIELD(src, role),
    FIELD(src, stop_reason),
    FIELD(src, stop_sequence),
    FIELD(src, usage)
END_PARSE


/***
 * Variants
 */
template <>
anthropic::TextCitation Parse<anthropic::TextCitation>(const simdjson::dom::element& src) {
    using T = anthropic::TextCitation;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<anthropic::CitationKinds>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected anthropic::TextCitation type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case anthropic::CitationKinds::CHAR_LOCATION:
        return T{Parse<anthropic::CitationCharLocation>(src)};
    case anthropic::CitationKinds::CONTENT_BLOCK_LOCATION:
        return T{Parse<anthropic::CitationContentBlockLocation>(src)};
    case anthropic::CitationKinds::PAGE_LOCATION:
        return T{Parse<anthropic::CitationPageLocation>(src)};
    case anthropic::CitationKinds::SEARCH_RESULT_LOCATION:
        return T{Parse<anthropic::CitationsSearchResultLocation>(src)};
    case anthropic::CitationKinds::WEB_SEARCH_RESULT_LOCATION:
        return T{Parse<anthropic::CitationsWebSearchResultLocation>(src)};
    default:
        throw AnnotatedException{"anthropic::TextCitation variant unsatisfied"};
    }
}

template <>
anthropic::ResponseContentBlock Parse<anthropic::ResponseContentBlock>(const simdjson::dom::element& src) {
    using T = anthropic::ResponseContentBlock;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = jai::llm::from_string_view<anthropic::ResponseContentBlockKinds>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected anthropic::ResponseContentBlockKinds type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case anthropic::ResponseContentBlockKinds::TEXT:
        return T{Parse<anthropic::TextBlock>(src)};
    case anthropic::ResponseContentBlockKinds::THINKING:
        return T{Parse<anthropic::ThinkingBlock>(src)};
    case anthropic::ResponseContentBlockKinds::REDACTED_THINKING:
        return T{Parse<anthropic::RedactedThinkingBlock>(src)};
    case anthropic::ResponseContentBlockKinds::TOOL_USE:
        return T{Parse<anthropic::ToolUseBlock>(src)};
    case anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE:
        return T{Parse<anthropic::ServerToolUseBlock>(src)};
    case anthropic::ResponseContentBlockKinds::WEB_SEARCH_TOOL_RESULT:
        return T{Parse<anthropic::WebSearchToolResultBlock>(src)};
    default:
        throw AnnotatedException{"anthropic::ResponseContentBlock variant unsatisfied"};
    }
}

template <>
anthropic::WebSearchToolResultBlock::Content
    Parse<anthropic::WebSearchToolResultBlock::Content>(const simdjson::dom::element& src)
{
    using T = anthropic::WebSearchToolResultBlock::Content;

    if (src.is_array()) {
        return T{ParseArrayOf<anthropic::WebSearchToolResultBlock::WebSearchResultBlock>(src)};
    }

    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = jai::llm::from_string_view<anthropic::WebSearchToolResultErrorType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected anthropic::WebSearchToolResultBlock::Content type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case anthropic::WebSearchToolResultErrorType::TOOL_RESULT_ERROR:
        return T{Parse<anthropic::WebSearchToolResultBlock::WebSearchToolResultError>(src)};
    default:
        throw AnnotatedException{"anthropic::WebSearchToolResultErrorType variant unsatisfied"};
    }
}


} // namespace jai::llm


#undef FIELD
#undef BEGIN_PARSE
#undef END_PARSE


/***
 * Top-level Deserialize
 */
namespace jai::llm::anthropic {


Response Deserialize(const curl::Response& response) {
    if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
        throw AnnotatedException{"Simdjson padding check failed"};
    }

    static thread_local simdjson::dom::parser parser{};
    simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()), response.body_len);

    Response out = Parse<Response>(doc);
    return out;
}


}
