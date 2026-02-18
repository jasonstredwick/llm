#include "../../../interface/protocols/anthropic/messages.hpp"
#include "../../../interface/protocols/anthropic/strings.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


/***
 * Response Citations
 */
BEGIN_DESERIALIZE(anthropic::CitationCharLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_char_index),
    FIELD(src, start_char_index),
    FIELD(src, file_id)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::CitationPageLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_page_number),
    FIELD(src, start_page_number),
    FIELD(src, file_id)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::CitationContentBlockLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, document_index),
    FIELD(src, document_title),
    FIELD(src, end_block_index),
    FIELD(src, start_block_index),
    FIELD(src, file_id)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::CitationsWebSearchResultLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, encrypted_index),
    FIELD(src, title),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::CitationsSearchResultLocation)
    FIELD(src, type),
    FIELD(src, cited_text),
    FIELD(src, end_block_index),
    FIELD(src, search_result_index),
    FIELD(src, source),
    FIELD(src, start_block_index),
    FIELD(src, title)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(anthropic::TextCitation)
    auto kind = EXTRACT_KIND(anthropic::CitationKinds, src, "type");
    FIELD_KIND(src, kind, anthropic::CitationKinds::CHAR_LOCATION,              anthropic::CitationCharLocation)
    FIELD_KIND(src, kind, anthropic::CitationKinds::CONTENT_BLOCK_LOCATION,     anthropic::CitationContentBlockLocation)
    FIELD_KIND(src, kind, anthropic::CitationKinds::PAGE_LOCATION,              anthropic::CitationPageLocation)
    FIELD_KIND(src, kind, anthropic::CitationKinds::SEARCH_RESULT_LOCATION,     anthropic::CitationsSearchResultLocation)
    FIELD_KIND(src, kind, anthropic::CitationKinds::WEB_SEARCH_RESULT_LOCATION, anthropic::CitationsWebSearchResultLocation)
END_DESERIALIZE_VARIANT(anthropic::TextCitation)


/***
 * Response Content Blocks
 */
BEGIN_DESERIALIZE(anthropic::TextBlock)
    FIELD(src, type),
    FIELD(src, citations),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::ThinkingBlock)
    FIELD(src, type),
    FIELD(src, signature),
    FIELD(src, thinking)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::RedactedThinkingBlock)
    FIELD(src, type),
    FIELD(src, data)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::ToolUseBlock)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::ServerToolUseBlock)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, input),
    FIELD(src, name)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::WebSearchToolResultBlock)
    FIELD(src, type),
    FIELD(src, content),
    FIELD(src, tool_use_id)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::WebSearchToolResultBlock::WebSearchResultBlock)
    FIELD(src, type),
    FIELD(src, encrypted_content),
    FIELD(src, page_age),
    FIELD(src, title),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::WebSearchToolResultBlock::WebSearchToolResultError)
    FIELD(src, type),
    FIELD(src, error_code)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(anthropic::WebSearchToolResultBlock::Content)
    if (src.is_array()) {
        return T{DeserializeTo<std::vector<anthropic::WebSearchToolResultBlock::WebSearchResultBlock>>(src)};
    }
    auto kind = EXTRACT_KIND(anthropic::WebSearchToolResultErrorType, src, "type");
    FIELD_KIND(src, kind,
               anthropic::WebSearchToolResultErrorType::TOOL_RESULT_ERROR,
               anthropic::WebSearchToolResultBlock::WebSearchToolResultError)
END_DESERIALIZE_VARIANT(anthropic::WebSearchToolResultBlock::Content)

BEGIN_DESERIALIZE_VARIANT(anthropic::ResponseContentBlock)
    auto kind = EXTRACT_KIND(anthropic::ResponseContentBlockKinds, src, "type");
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::REDACTED_THINKING,      anthropic::RedactedThinkingBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::SERVER_TOOL_USE,        anthropic::ServerToolUseBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::TEXT,                   anthropic::TextBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::THINKING,               anthropic::ThinkingBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::TOOL_USE,               anthropic::ToolUseBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::WEB_SEARCH_TOOL_RESULT, anthropic::WebSearchToolResultBlock)
END_DESERIALIZE_VARIANT(anthropic::ResponseContentBlock)

/***
 * Usage and Substructures
 */
BEGIN_DESERIALIZE(anthropic::CacheCreation)
    FIELD(src, ephemeral_1h_input_tokens),
    FIELD(src, ephemeral_5m_input_tokens)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::ServerToolUsage)
    FIELD(src, web_search_requests)
END_DESERIALIZE

BEGIN_DESERIALIZE(anthropic::Usage)
    FIELD(src, cache_creation),
    FIELD(src, cache_creation_input_tokens),
    FIELD(src, cache_read_input_tokens),
    FIELD(src, input_tokens),
    FIELD(src, output_tokens),
    FIELD(src, server_tool_use),
    FIELD(src, service_tier)
END_DESERIALIZE


/***
 * Top-level Response Message
 */
BEGIN_DESERIALIZE(anthropic::Response)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, content),
    FIELD(src, model),
    FIELD(src, role),
    FIELD(src, stop_reason),
    FIELD(src, stop_sequence),
    FIELD(src, usage)
END_DESERIALIZE


/***
 * Top-level Deserialize
 */
namespace anthropic {
    Response Deserialize(const curl::Response& response) {
        if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
            throw AnnotatedException{"Simdjson padding check failed"};
        }

        static thread_local simdjson::dom::parser parser{};

        try {
            simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()),
                                                    response.body_len);
            return DeserializeTo<Response>(doc);
        } catch (const simdjson::simdjson_error& e) {
            throw AnnotatedException{"Deserialize Failed", e.what()};
        }
    }
}

}
