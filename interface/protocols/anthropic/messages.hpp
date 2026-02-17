/***
 * This file defines the Anthropic Messages Endpoint; /v1/messages API.
 * Semantic protocol, not transport contract.
 *
 * See https://platform.claude.com/docs/en/api/messages/create
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../core/types.hpp"


namespace jai::llm::anthropic {


/***
 * Kinds
 */
struct KindBase64Source               : Kind { static constexpr std::string_view value = "base64"; };
struct KindCharLocation               : Kind { static constexpr std::string_view value = "char_location"; };
struct KindContentBlockLocation       : Kind { static constexpr std::string_view value = "content_block_location"; };
struct KindContentSource              : Kind { static constexpr std::string_view value = "content"; };
struct KindCustomTool                 : Kind { static constexpr std::string_view value = "custom"; };
struct KindDocumentBlock              : Kind { static constexpr std::string_view value = "document"; };
struct KindEphemeral                  : Kind { static constexpr std::string_view value = "ephemeral"; };
struct KindImageBlock                 : Kind { static constexpr std::string_view value = "image"; };
struct KindJsonSchemaType             : Kind { static constexpr std::string_view value = "object"; };
struct KindMessage                    : Kind { static constexpr std::string_view value = "message"; };
struct KindPageLocation               : Kind { static constexpr std::string_view value = "page_location"; };
struct KindPlainTextSource            : Kind { static constexpr std::string_view value = "text"; };
struct KindRedactedThinkingBlock      : Kind { static constexpr std::string_view value = "redacted_thinking"; };
struct KindSearchResultBlock          : Kind { static constexpr std::string_view value = "search_result"; };
struct KindSearchResultLocation       : Kind { static constexpr std::string_view value = "search_result_location"; };
struct KindServerToolUseBlock         : Kind { static constexpr std::string_view value = "server_tool_use"; };
struct KindStructuredOutputFormat     : Kind { static constexpr std::string_view value = "json_schema"; };
struct KindTextBlock                  : Kind { static constexpr std::string_view value = "text"; };
struct KindThinkingBlock              : Kind { static constexpr std::string_view value = "thinking"; };
struct KindToolBash20250124Type       : Kind { static constexpr std::string_view value = "bash_20250124"; };
struct KindToolChoiceAny              : Kind { static constexpr std::string_view value = "any"; };
struct KindToolChoiceAuto             : Kind { static constexpr std::string_view value = "auto"; };
struct KindToolChoiceNone             : Kind { static constexpr std::string_view value = "none"; };
struct KindToolChoiceTool             : Kind { static constexpr std::string_view value = "tool"; };
struct KindToolResultBlock            : Kind { static constexpr std::string_view value = "tool_result"; };
struct KindToolTextEditor20250124Name : Kind { static constexpr std::string_view value = "text_editor_20250124"; };
struct KindToolTextEditor20250429Name : Kind { static constexpr std::string_view value = "text_editor_20250429"; };
struct KindToolTextEditor20250728Name : Kind { static constexpr std::string_view value = "text_editor_20250728"; };
struct KindToolUseBlock               : Kind { static constexpr std::string_view value = "tool_use"; };
struct KindUrlSource                  : Kind { static constexpr std::string_view value = "url"; };
struct KindWebSearchResultLocation    : Kind { static constexpr std::string_view value = "web_search_result_location"; };
struct KindWebSearchTool20250305      : Kind { static constexpr std::string_view value = "web_search_20250305"; };
struct KindWebSearchToolResultBlock   : Kind { static constexpr std::string_view value = "web_search_tool_result"; };
struct KindWebSearchToolResultError   : Kind { static constexpr std::string_view value = "web_search_tool_result_error"; };


/***
 * Vocabulary
 */
enum class CacheControlTTL { TTL_5M, TTL_1H };
enum class CitationKinds {
    CHAR_LOCATION,
    CONTENT_BLOCK_LOCATION,
    PAGE_LOCATION,
    SEARCH_RESULT_LOCATION,
    WEB_SEARCH_RESULT_LOCATION
};
enum class DocSrcKind { BLOCK, PDF, PLAIN_TEXT, URL_PDF};
enum class ImageMediaType { IMAGE_JPEG, IMAGE_PNG, IMAGE_GIF, IMAGE_WEBP };
enum class ImageSourceKinds { BASE64, URL };
enum class PDFMediaType { APPLICATION_PDF };
enum class PlainTextMediaType { TEXT_PLAIN };
enum class ReplaceBasedEditor { STRING };
enum class ReplaceEditor { STRING };
enum class RequestServiceTier { AUTO, STANDARD_ONLY };
enum class ResponseContentBlockKinds {
    TEXT,
    THINKING,
    REDACTED_THINKING,
    TOOL_USE,
    SERVER_TOOL_USE,
    WEB_SEARCH_TOOL_RESULT
};
enum class ResponseRole { ASSISTANT };
enum class Role { USER, ASSISTANT };
enum class StopReason { END_TURN, MAX_TOKENS, STOP_SEQUENCE, TOOL_USE, PAUSE_TURN, REFUSAL };
enum class ThinkingConfigType { ENABLED, DISABLED, ADAPTIVE };
enum class ThinkingEffort { LOW, MEDIUM, HIGH, MAX };
enum class ToolBash20250124Name { BASH };
enum class UsageServiceTier { STANDARD, PRIORITY, BATCH };
enum class UserLocationType { APPROXIMATE };
enum class WebSearchName { WEB_SEARCH };
enum class WebSearchToolResultErrorCode {
    INVALID_TOOL_INPUT, UNAVAILABLE, MAX_USES_EXCEEDED, TOO_MANY_REQUESTS, QUERY_TOO_LONG, REQUEST_TOO_LARGE
};
enum class WebSearchToolResultErrorType { TOOL_RESULT, TOOL_RESULT_ERROR };


/***
 * Request Shared Substructures
 */
struct CacheControlEphemeral {
    Required<KindEphemeral> type{{}};
    std::optional<CacheControlTTL> ttl{};
};

struct Metadata {
    std::optional<std::string> user_id{};
};

struct CitationsConfigParam {
    std::optional<bool> enabled{};
};


/***
 * Request Citations
 */
struct CitationCharLocationParam {
    Required<KindCharLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_char_index;
    Required<int64_t> start_char_index;
};

struct CitationPageLocationParam {
    Required<KindPageLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_page_number;
    Required<int64_t> start_page_number;
};

struct CitationContentBlockLocationParam {
    Required<KindContentBlockLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_block_index;
    Required<int64_t> start_block_index;
};

struct CitationWebSearchResultLocationParam {
    Required<KindWebSearchResultLocation> type{{}};
    Required<std::string> cited_text;
    Required<std::string> encrypted_index;
    Required<std::string> title;
    Required<EncodedUrl> url;
};

struct CitationSearchResultLocationParam {
    Required<KindSearchResultLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> end_block_index;
    Required<int64_t> search_result_index;
    Required<std::string> source;
    Required<int64_t> start_block_index;
    Required<std::string> title;
};

using TextCitationParam = std::variant<
    CitationCharLocationParam,
    CitationPageLocationParam,
    CitationContentBlockLocationParam,
    CitationWebSearchResultLocationParam,
    CitationSearchResultLocationParam
>;


/***
 * Request Content Blocks
 */

struct Base64ImageSource {
    Required<KindBase64Source> type{{}};
    Required<std::string> data;
    Required<ImageMediaType> media_type;
};

struct URLImageSource {
    Required<KindUrlSource> type{{}};
    Required<EncodedUrl> url;
};

using ImageSource = std::variant<Base64ImageSource, URLImageSource>;

struct TextBlockParam {
    Required<KindTextBlock> type{{}};
    Required<std::string> text;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::vector<TextCitationParam>> citations{};
};

struct ImageBlockParam {
    Required<KindImageBlock> type{{}};
    Required<ImageSource> source;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct Base64PDFSource {
    Required<KindBase64Source> type{{}};
    Required<std::string> data;
    Required<PDFMediaType> media_type;
};

struct PlainTextSource {
    Required<KindPlainTextSource> type{{}};
    Required<std::string> data;
    Required<PlainTextMediaType> media_type;
};

struct URLPDFSource {
    Required<KindUrlSource> type{{}};
    Required<EncodedUrl> url;
};

struct ContentBlockSource {
    using ContentBlockSourceContent = std::variant<TextBlockParam, ImageBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentBlockSourceContent>>;

    Required<KindContentSource> type{{}};
    Required<Content> content;
};

using DocumentSource = std::variant<
    Base64PDFSource,
    PlainTextSource,
    ContentBlockSource,
    URLPDFSource
>;

struct DocumentBlockParam {
    Required<KindDocumentBlock> type{{}};
    Required<DocumentSource> source;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
    std::optional<std::string> context{};
    std::optional<std::string> title{};
};

struct SearchResultBlockParam {
    Required<KindSearchResultBlock> type{{}};
    Required<std::vector<TextBlockParam>> content;
    Required<std::string> source;
    Required<std::string> title;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
};

struct ThinkingBlockParam {
    Required<KindThinkingBlock> type{{}};
    Required<std::string> signature;
    Required<std::string> thinking;
};

struct RedactedThinkingBlockParam {
    Required<KindRedactedThinkingBlock> type{{}};
    Required<std::string> data;
};

struct ToolUseBlockParam {
    Required<KindToolUseBlock> type{{}};
    Required<std::string> id;
    Required<jai::llm::json::Object> input;
    Required<std::string> name;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct ToolResultBlockParam {
    using ContentUnit = std::variant<TextBlockParam, ImageBlockParam, SearchResultBlockParam, DocumentBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentUnit>>;

    Required<KindToolResultBlock> type{{}};
    Required<std::string> tool_use_id;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<Content> content{};
    std::optional<bool> is_error{};
};

struct ServerToolUseBlockParam {
    Required<KindServerToolUseBlock> type{{}};
    Required<WebSearchName> name;
    Required<std::string> id;
    Required<jai::llm::json::Object> input;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct WebSearchToolResultBlockParam {
    struct WebSearchResultBlockParamItem {
        Required<KindWebSearchResultLocation> type{{}};
        Required<std::string> encrypted_content;
        Required<std::string> title;
        Required<EncodedUrl> url;
        std::optional<std::string> page_age{};
    };

    struct WebSearchToolRequestError {
        Required<KindWebSearchToolResultError> type{{}};
        Required<WebSearchToolResultErrorCode> error_code;
    };

    using Content = std::variant<std::vector<WebSearchResultBlockParamItem>, WebSearchToolRequestError>;

    Required<KindWebSearchToolResultBlock> type{{}};
    Required<Content> content;
    Required<std::string> tool_use_id;
    std::optional<CacheControlEphemeral> cache_control{};
};

using ContentBlockParam = std::variant<
    TextBlockParam,
    ImageBlockParam,
    DocumentBlockParam,
    SearchResultBlockParam,
    ThinkingBlockParam,
    RedactedThinkingBlockParam,
    ToolUseBlockParam,
    ToolResultBlockParam,
    ServerToolUseBlockParam,
    WebSearchToolResultBlockParam
>;


/***
 * Tool Definitions
 */

struct Tool {
    struct InputSchema {
        Required<KindJsonSchemaType> type{{}};
        std::optional<jai::llm::json::Object> properties{};
        std::optional<std::vector<std::string>> required{};
    };
    Required<InputSchema> input_schema;
    Required<std::string> name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::string> description{};
    std::optional<bool> strict{};
    std::optional<KindCustomTool> type{};
};

struct ToolBash20250124 {
    Required<KindToolBash20250124Type> type{{}};
    Required<ToolBash20250124Name> name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250124 {
    Required<KindToolTextEditor20250124Name> type{{}};
    Required<ReplaceEditor> name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250429 {
    Required<KindToolTextEditor20250429Name> type{{}};
    Required<ReplaceBasedEditor> name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250728 {
    Required<KindToolTextEditor20250728Name> type{{}};
    Required<ReplaceBasedEditor> name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<int64_t> max_characters{};
    std::optional<bool> strict{};
};

struct WebSearchTool20250305 {
    struct UserLocation {
        Required<UserLocationType> type;
        std::optional<std::string> city{};
        std::optional<std::string> country{};
        std::optional<std::string> region{};
        std::optional<std::string> timezone{};
    };
    Required<KindWebSearchTool20250305> type{{}};
    Required<WebSearchName> name;
    std::optional<std::vector<std::string>> allowed_domains{};
    std::optional<std::vector<std::string>> blocked_domains{};
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<int64_t> max_uses{};
    std::optional<bool> strict{};
    std::optional<UserLocation> user_location{};
};

using ToolUnion = std::variant<
    Tool,
    ToolBash20250124,
    ToolTextEditor20250124,
    ToolTextEditor20250429,
    ToolTextEditor20250728,
    WebSearchTool20250305
>;


/***
 * Tool Choice
 */

struct ToolChoiceAuto {
    Required<KindToolChoiceAuto> type{{}};
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceAny {
    Required<KindToolChoiceAny> type{{}};
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceTool {
    Required<KindToolChoiceTool> type{{}};
    Required<std::string> name;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceNone {
    Required<KindToolChoiceNone> type{{}};
};

using ToolChoice = std::variant<
    ToolChoiceAuto,
    ToolChoiceAny,
    ToolChoiceTool,
    ToolChoiceNone
>;


/***
 * Request Infrastructure
 */

struct OutputConfig {
    struct Format {
        Required<KindStructuredOutputFormat> type{{}};
        Required<jai::llm::json::Object> schema;
    };
    std::optional<ThinkingEffort> effort{};
    std::optional<Format> format{};
};

struct ThinkingConfigEnabled {
    Required<ThinkingConfigType> type{ThinkingConfigType::ENABLED};
    Required<int64_t> budget_tokens;
};

struct ThinkingConfigDisabled {
    Required<ThinkingConfigType> type{ThinkingConfigType::DISABLED};
};

struct ThinkingConfigAdaptive {
    Required<ThinkingConfigType> type{ThinkingConfigType::ADAPTIVE};
};

using ThinkingConfig = std::variant<
    ThinkingConfigEnabled,
    ThinkingConfigDisabled,
    ThinkingConfigAdaptive
>;


/***
 * Message Request
 */
struct MessageParam {
    using Content = std::variant<std::string, std::vector<ContentBlockParam>>;

    Required<Content> content;
    Required<Role> role;
};

using System = std::variant<std::string, std::vector<TextBlockParam>>;

struct Request {
    std::optional<int64_t> max_tokens;
    Required<std::vector<MessageParam>> messages;
    Required<std::string> model;
    std::optional<Metadata> metadata{};
    std::optional<OutputConfig> output_config{};
    std::optional<RequestServiceTier> service_tier{};
    std::optional<std::vector<std::string>> stop_sequences{};
    std::optional<bool> stream{};
    std::optional<System> system{};
    std::optional<double> temperature{};
    std::optional<ThinkingConfig> thinking{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<std::vector<ToolUnion>> tools{};
    std::optional<int64_t> top_k{};
    std::optional<double> top_p{};
};


/***
 * Response Objects
 */

struct CitationCharLocation {
    Required<KindCharLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_char_index;
    Required<int64_t> start_char_index;
    Required<std::string> file_id;
};

struct CitationPageLocation {
    Required<KindPageLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_page_number;
    Required<int64_t> start_page_number;
    Required<std::string> file_id;
};

struct CitationContentBlockLocation {
    Required<KindContentBlockLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> document_index;
    Required<std::string> document_title;
    Required<int64_t> end_block_index;
    Required<int64_t> start_block_index;
    Required<std::string> file_id;
};

struct CitationsWebSearchResultLocation {
    Required<KindWebSearchResultLocation> type{{}};
    Required<std::string> cited_text;
    Required<std::string> encrypted_index;
    Required<std::string> title;
    Required<EncodedUrl> url;
};

struct CitationsSearchResultLocation {
    Required<KindSearchResultLocation> type{{}};
    Required<std::string> cited_text;
    Required<int64_t> end_block_index;
    Required<int64_t> search_result_index;
    Required<std::string> source;
    Required<int64_t> start_block_index;
    Required<std::string> title;
};

using TextCitation = std::variant<
    CitationCharLocation,
    CitationPageLocation,
    CitationContentBlockLocation,
    CitationsWebSearchResultLocation,
    CitationsSearchResultLocation
>;

struct TextBlock {
    Required<KindTextBlock> type{{}};
    Required<std::vector<TextCitation>> citations;
    Required<std::string> text;
};

struct ThinkingBlock {
    Required<KindThinkingBlock> type{{}};
    Required<std::string> signature;
    Required<std::string> thinking;
};

struct RedactedThinkingBlock {
    Required<KindRedactedThinkingBlock> type{{}};
    Required<std::string> data;
};

struct ToolUseBlock {
    Required<KindToolUseBlock> type{{}};
    Required<std::string> id;
    Required<jai::llm::json::Object> input;
    Required<std::string> name;
};

struct ServerToolUseBlock {
    Required<KindServerToolUseBlock> type{{}};
    Required<std::string> id;
    Required<jai::llm::json::Object> input;
    Required<WebSearchName> name;
};

struct WebSearchToolResultBlock {
    struct WebSearchResultBlock {
        Required<KindWebSearchResultLocation> type{{}};
        Required<std::string> encrypted_content;
        Required<std::string> page_age;
        Required<std::string> title;
        Required<EncodedUrl> url;
    };

    struct WebSearchToolResultError {
        Required<KindWebSearchToolResultError> type{{}};
        Required<WebSearchToolResultErrorCode> error_code;
    };

    using Content = std::variant<WebSearchToolResultError, std::vector<WebSearchResultBlock>>;

    Required<KindWebSearchToolResultBlock> type{{}};
    Required<Content> content;
    Required<std::string> tool_use_id;
};

using ResponseContentBlock = std::variant<
    TextBlock,
    ThinkingBlock,
    RedactedThinkingBlock,
    ToolUseBlock,
    ServerToolUseBlock,
    WebSearchToolResultBlock
>;

struct CacheCreation {
    Required<int64_t> ephemeral_1h_input_tokens;
    Required<int64_t> ephemeral_5m_input_tokens;
};

struct ServerToolUsage {
    Required<int64_t> web_search_requests;
};

struct Usage {
    Required<CacheCreation> cache_creation;
    Required<int64_t> cache_creation_input_tokens;
    Required<int64_t> cache_read_input_tokens;
    Required<int64_t> input_tokens;
    Required<int64_t> output_tokens;
    Required<ServerToolUsage> server_tool_use;
    Required<UsageServiceTier> service_tier;
};

struct Response {
    Required<KindMessage> type{{}};
    Required<std::string> id;
    Required<std::vector<ResponseContentBlock>> content;
    Required<std::string> model;
    Required<ResponseRole> role;
    Required<StopReason> stop_reason;
    std::optional<std::string> stop_sequence{};
    Required<Usage> usage;
};


}
