#include "../../interface/protocols/anthropic/messages.hpp"
#include "base.hpp"


using namespace simdjson::builder;


namespace jai::llm::anthropic {


/***
 * Request Shared Substructures
 */
BEGIN_SERIALIZE(CacheControlEphemeral)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, ttl,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(Metadata)
    FIELD(obj, user_id, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(CitationsConfigParam)
    FIELD(obj, enabled, CommaDirection::NONE)
END_SERIALIZE


/***
 * Request Citations
 */
BEGIN_SERIALIZE(CitationCharLocationParam)
    FIELD(obj, type,             CommaDirection::NONE)
    FIELD(obj, cited_text,       CommaDirection::BEFORE)
    FIELD(obj, document_index,   CommaDirection::BEFORE)
    FIELD(obj, document_title,   CommaDirection::BEFORE)
    FIELD(obj, end_char_index,   CommaDirection::BEFORE)
    FIELD(obj, start_char_index, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(CitationPageLocationParam)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, cited_text,        CommaDirection::BEFORE)
    FIELD(obj, document_index,    CommaDirection::BEFORE)
    FIELD(obj, document_title,    CommaDirection::BEFORE)
    FIELD(obj, end_page_number,   CommaDirection::BEFORE)
    FIELD(obj, start_page_number, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(CitationContentBlockLocationParam)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, cited_text,        CommaDirection::BEFORE)
    FIELD(obj, document_index,    CommaDirection::BEFORE)
    FIELD(obj, document_title,    CommaDirection::BEFORE)
    FIELD(obj, end_block_index,   CommaDirection::BEFORE)
    FIELD(obj, start_block_index, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(CitationWebSearchResultLocationParam)
    FIELD(obj, type,            CommaDirection::NONE)
    FIELD(obj, cited_text,      CommaDirection::BEFORE)
    FIELD(obj, encrypted_index, CommaDirection::BEFORE)
    FIELD(obj, title,           CommaDirection::BEFORE)
    FIELD(obj, url,             CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(CitationSearchResultLocationParam)
    FIELD(obj, type,                CommaDirection::NONE)
    FIELD(obj, cited_text,          CommaDirection::BEFORE)
    FIELD(obj, end_block_index,     CommaDirection::BEFORE)
    FIELD(obj, search_result_index, CommaDirection::BEFORE)
    FIELD(obj, source,              CommaDirection::BEFORE)
    FIELD(obj, start_block_index,   CommaDirection::BEFORE)
    FIELD(obj, title,               CommaDirection::BEFORE)
END_SERIALIZE


/***
 * Request Content Blocks
 */
BEGIN_SERIALIZE(Base64ImageSource)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, data,       CommaDirection::BEFORE)
    FIELD(obj, media_type, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(URLImageSource)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, url,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(TextBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, text,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    FIELD(obj, citations,     CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ImageBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, source,        CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(Base64PDFSource)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, data,       CommaDirection::BEFORE)
    FIELD(obj, media_type, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(PlainTextSource)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, data,       CommaDirection::BEFORE)
    FIELD(obj, media_type, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(URLPDFSource)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, url,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentBlockSource)
    FIELD(obj, type, CommaDirection::NONE)
    builder.append_comma();
    FIELD(obj, content, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(DocumentBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, source,        CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    FIELD(obj, citations,     CommaDirection::BEFORE)
    FIELD(obj, context,       CommaDirection::BEFORE)
    FIELD(obj, title,         CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(SearchResultBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, content,       CommaDirection::BEFORE)
    FIELD(obj, source,        CommaDirection::BEFORE)
    FIELD(obj, title,         CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    FIELD(obj, citations,     CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ThinkingBlockParam)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, signature, CommaDirection::BEFORE)
    FIELD(obj, thinking,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(RedactedThinkingBlockParam)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, data, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolUseBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, id,            CommaDirection::BEFORE)
    FIELD(obj, input,         CommaDirection::BEFORE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolResultBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, tool_use_id,   CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    if (obj.content) {
        builder.append_comma();
        FIELD(obj, content, CommaDirection::NONE)
    }
    FIELD(obj, is_error, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ServerToolUseBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, id,            CommaDirection::BEFORE)
    FIELD(obj, input,         CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolResultBlockParam::WebSearchResultBlockParamItem)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, encrypted_content, CommaDirection::BEFORE)
    FIELD(obj, title,             CommaDirection::BEFORE)
    FIELD(obj, url,               CommaDirection::BEFORE)
    FIELD(obj, page_age,          CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolResultBlockParam::WebSearchToolRequestError)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, error_code, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolResultBlockParam)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, content,       CommaDirection::BEFORE)
    FIELD(obj, tool_use_id,   CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE


/***
 * Tool Definitions
 */
BEGIN_SERIALIZE(Tool::InputSchema)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, properties, CommaDirection::BEFORE)
    FIELD(obj, required,   CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(Tool)
    FIELD(obj, input_schema,  CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    FIELD(obj, description,   CommaDirection::BEFORE)
    FIELD(obj, strict,        CommaDirection::BEFORE)
    FIELD(obj, type,          CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolBash20250124)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTextEditor20250124)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTextEditor20250429)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTextEditor20250728)
    FIELD(obj, type,          CommaDirection::NONE)
    FIELD(obj, name,          CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchTool20250305::UserLocation)
    FIELD(obj, type,     CommaDirection::NONE)
    FIELD(obj, city,     CommaDirection::BEFORE)
    FIELD(obj, country,  CommaDirection::BEFORE)
    FIELD(obj, region,   CommaDirection::BEFORE)
    FIELD(obj, timezone, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchTool20250305)
    FIELD(obj, type,            CommaDirection::NONE)
    FIELD(obj, name,            CommaDirection::BEFORE)
    FIELD(obj, allowed_domains, CommaDirection::BEFORE)
    FIELD(obj, blocked_domains, CommaDirection::BEFORE)
    FIELD(obj, cache_control,   CommaDirection::BEFORE)
    FIELD(obj, max_uses,        CommaDirection::BEFORE)
    FIELD(obj, strict,          CommaDirection::BEFORE)
    FIELD(obj, user_location,   CommaDirection::BEFORE)
END_SERIALIZE


/***
 * Tool Choice
 */
BEGIN_SERIALIZE(ToolChoiceAuto)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolChoiceAny)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolChoiceTool)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, name, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolChoiceNone)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE


/***
 * Request Infrastructure
 */
BEGIN_SERIALIZE(OutputConfig::Format)
    FIELD(obj, schema, CommaDirection::NONE)
    FIELD(obj, type,   CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(OutputConfig)
    FIELD(obj, format, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ThinkingConfigEnabled)
    FIELD(obj, budget_tokens, CommaDirection::NONE)
    FIELD(obj, type,          CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ThinkingConfigDisabled)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE


/***
 * Message Request
 */
BEGIN_SERIALIZE(MessageParam)
    FIELD(obj, content, CommaDirection::NONE)
    FIELD(obj, role,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(Request)
    FIELD(obj, max_tokens,     CommaDirection::NONE)
    FIELD(obj, messages,       CommaDirection::BEFORE)
    FIELD(obj, model,          CommaDirection::BEFORE)
    FIELD(obj, metadata,       CommaDirection::BEFORE)
    FIELD(obj, output_config,  CommaDirection::BEFORE)
    FIELD(obj, service_tier,   CommaDirection::BEFORE)
    FIELD(obj, stop_sequences, CommaDirection::BEFORE)
    FIELD(obj, stream,         CommaDirection::BEFORE)
    if (obj.system) {
        FIELD(obj, system, CommaDirection::BEFORE)
    }
    FIELD(obj, temperature,    CommaDirection::BEFORE)
    FIELD(obj, thinking,       CommaDirection::BEFORE)
    FIELD(obj, tool_choice,    CommaDirection::BEFORE)
    FIELD(obj, tools,          CommaDirection::BEFORE)
    FIELD(obj, top_k,          CommaDirection::BEFORE)
    FIELD(obj, top_p,          CommaDirection::BEFORE)
END_SERIALIZE


/***
 * Top-level Serialize
 */
std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    try {
        builder.clear();
        SerializeFrom(builder, request);

        if (!builder.validate_unicode()) {
            throw AnnotatedException{
                "anthropic::Serialize Failed",
                "string_builder generated invalid unicode data."
            };
        }

        auto result = builder.view();
        if (result.error() != simdjson::SUCCESS) {
            throw AnnotatedException{"anthropic::Serialize Failed", simdjson::error_message(result.error())};
        }
        std::string_view json_str = result.value();

        return json_str |
               std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
               std::ranges::to<std::vector<std::byte>>();
    } catch (AnnotatedException const&) {
        throw;
    } catch (std::exception const& e) {
        AnnotatedException ex{
            "anthropic::Serialize Failed",
            "string_builder failed to serialize anthropic::Response."
        };
        ex.AddContext(e.what());
        throw ex;
    }
}


}
