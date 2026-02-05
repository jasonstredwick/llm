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
enum class ThinkingConfigType { ENABLED, DISABLED };
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
    KindEphemeral type{};
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
    KindCharLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_char_index;
    int64_t start_char_index;
};

struct CitationPageLocationParam {
    KindPageLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_page_number;
    int64_t start_page_number;
};

struct CitationContentBlockLocationParam {
    KindContentBlockLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_block_index;
    int64_t start_block_index;
};

struct CitationWebSearchResultLocationParam {
    KindWebSearchResultLocation type{};
    std::string cited_text;
    std::string encrypted_index;
    std::string title;
    EncodedUrl url;
};

struct CitationSearchResultLocationParam {
    KindSearchResultLocation type{};
    std::string cited_text;
    int64_t end_block_index;
    int64_t search_result_index;
    std::string source;
    int64_t start_block_index;
    std::string title;
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
    KindBase64Source type{};
    std::string data;
    ImageMediaType media_type;
};

struct URLImageSource {
    KindUrlSource type{};
    EncodedUrl url;
};

using ImageSource = std::variant<Base64ImageSource, URLImageSource>;

struct TextBlockParam {
    KindTextBlock type{};
    std::string text;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::vector<TextCitationParam>> citations{};
};

struct ImageBlockParam {
    KindImageBlock type{};
    ImageSource source;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct Base64PDFSource {
    KindBase64Source type{};
    std::string data;
    PDFMediaType media_type;
};

struct PlainTextSource {
    KindPlainTextSource type{};
    std::string data;
    PlainTextMediaType media_type;
};

struct URLPDFSource {
    KindUrlSource type{};
    EncodedUrl url;
};

struct ContentBlockSource {
    using ContentBlockSourceContent = std::variant<TextBlockParam, ImageBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentBlockSourceContent>>;

    KindContentSource type{};
    Content content;
};

using DocumentSource = std::variant<
    Base64PDFSource,
    PlainTextSource,
    ContentBlockSource,
    URLPDFSource
>;

struct DocumentBlockParam {
    KindDocumentBlock type{};
    DocumentSource source;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
    std::optional<std::string> context{};
    std::optional<std::string> title{};
};

struct SearchResultBlockParam {
    KindSearchResultBlock type{};
    std::vector<TextBlockParam> content;
    std::string source;
    std::string title;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
};

struct ThinkingBlockParam {
    KindThinkingBlock type{};
    std::string signature;
    std::string thinking;
};

struct RedactedThinkingBlockParam {
    KindRedactedThinkingBlock type{};
    std::string data;
};

struct ToolUseBlockParam {
    KindToolUseBlock type{};
    std::string id;
    jai::llm::json::Object input;
    std::string name;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct ToolResultBlockParam {
    using ContentUnit = std::variant<TextBlockParam, ImageBlockParam, SearchResultBlockParam, DocumentBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentUnit>>;

    KindToolResultBlock type{};
    std::string tool_use_id;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<Content> content{};
    std::optional<bool> is_error{};
};

struct ServerToolUseBlockParam {
    KindServerToolUseBlock type{};
    WebSearchName name = WebSearchName::WEB_SEARCH;
    std::string id;
    jai::llm::json::Object input;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct WebSearchToolResultBlockParam {
    struct WebSearchResultBlockParamItem {
        KindWebSearchResultLocation type{};
        std::string encrypted_content;
        std::string title;
        EncodedUrl url;
        std::optional<std::string> page_age{};
    };

    struct WebSearchToolRequestError {
        KindWebSearchToolResultError type{};
        WebSearchToolResultErrorCode error_code;
    };

    using Content = std::variant<std::vector<WebSearchResultBlockParamItem>, WebSearchToolRequestError>;

    KindWebSearchToolResultBlock type{};
    Content content;
    std::string tool_use_id;
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
        KindJsonSchemaType type{};
        std::optional<jai::llm::json::Object> properties{};
        std::optional<std::vector<std::string>> required{};
    };
    InputSchema input_schema;
    std::string name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::string> description{};
    std::optional<bool> strict{};
    std::optional<KindCustomTool> type{};
};

struct ToolBash20250124 {
    KindToolBash20250124Type type{};
    ToolBash20250124Name name = ToolBash20250124Name::BASH;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250124 {
    KindToolTextEditor20250124Name type{};
    ReplaceEditor name = ReplaceEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250429 {
    KindToolTextEditor20250429Name type{};
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250728 {
    KindToolTextEditor20250728Name type{};
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<int64_t> max_characters{};
    std::optional<bool> strict{};
};

struct WebSearchTool20250305 {
    struct UserLocation {
        UserLocationType type{UserLocationType::APPROXIMATE};
        std::optional<std::string> city{};
        std::optional<std::string> country{};
        std::optional<std::string> region{};
        std::optional<std::string> timezone{};
    };
    KindWebSearchTool20250305 type{};
    WebSearchName name = WebSearchName::WEB_SEARCH;
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
    KindToolChoiceAuto type{};
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceAny {
    KindToolChoiceAny type{};
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceTool {
    KindToolChoiceTool type{};
    std::string name;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceNone {
    KindToolChoiceNone type{};
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
        KindStructuredOutputFormat type{};
        jai::llm::json::Object schema;
    };
    std::optional<Format> format{};
};

struct ThinkingConfigEnabled {
    ThinkingConfigType type = ThinkingConfigType::ENABLED;
    int64_t budget_tokens;
};

struct ThinkingConfigDisabled {
    ThinkingConfigType type = ThinkingConfigType::DISABLED;
};

using ThinkingConfig = std::variant<
    ThinkingConfigEnabled,
    ThinkingConfigDisabled
>;


/***
 * Message Request
 */
struct MessageParam {
    using Content = std::variant<std::string, std::vector<ContentBlockParam>>;

    Content content;
    Role role;
};

using System = std::variant<std::string, std::vector<TextBlockParam>>;

struct Request {
    int64_t max_tokens;
    std::vector<MessageParam> messages;
    std::string model;
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
    KindCharLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_char_index;
    int64_t start_char_index;
    std::string file_id;
};

struct CitationPageLocation {
    KindPageLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_page_number;
    int64_t start_page_number;
    std::string file_id;
};

struct CitationContentBlockLocation {
    KindContentBlockLocation type{};
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_block_index;
    int64_t start_block_index;
    std::string file_id;
};

struct CitationsWebSearchResultLocation {
    KindWebSearchResultLocation type{};
    std::string cited_text;
    std::string encrypted_index;
    std::string title;
    EncodedUrl url;
};

struct CitationsSearchResultLocation {
    KindSearchResultLocation type{};
    std::string cited_text;
    int64_t end_block_index;
    int64_t search_result_index;
    std::string source;
    int64_t start_block_index;
    std::string title;
};

using TextCitation = std::variant<
    CitationCharLocation,
    CitationPageLocation,
    CitationContentBlockLocation,
    CitationsWebSearchResultLocation,
    CitationsSearchResultLocation
>;

struct TextBlock {
    KindTextBlock type{};
    std::vector<TextCitation> citations;
    std::string text;
};

struct ThinkingBlock {
    KindThinkingBlock type{};
    std::string signature;
    std::string thinking;
};

struct RedactedThinkingBlock {
    KindRedactedThinkingBlock type{};
    std::string data;
};

struct ToolUseBlock {
    KindToolUseBlock type{};
    std::string id;
    jai::llm::json::Object input;
    std::string name;
};

struct ServerToolUseBlock {
    KindServerToolUseBlock type{};
    std::string id;
    jai::llm::json::Object input;
    WebSearchName name = WebSearchName::WEB_SEARCH;
};

struct WebSearchToolResultBlock {
    struct WebSearchResultBlock {
        KindWebSearchResultLocation type{};
        std::string encrypted_content;
        std::string page_age;
        std::string title;
        EncodedUrl url;
    };

    struct WebSearchToolResultError {
        KindWebSearchToolResultError type{};
        WebSearchToolResultErrorCode error_code;
    };

    using Content = std::variant<WebSearchToolResultError, std::vector<WebSearchResultBlock>>;

    KindWebSearchToolResultBlock type{};
    Content content;
    std::string tool_use_id;
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
    int64_t ephemeral_1h_input_tokens;
    int64_t ephemeral_5m_input_tokens;
};

struct ServerToolUsage {
    int64_t web_search_requests;
};

struct Usage {
    CacheCreation cache_creation;
    int64_t cache_creation_input_tokens;
    int64_t cache_read_input_tokens;
    int64_t input_tokens;
    int64_t output_tokens;
    ServerToolUsage server_tool_use;
    UsageServiceTier service_tier;
};

struct Response {
    KindMessage type{};
    std::string id;
    std::vector<ResponseContentBlock> content;
    std::string model;
    ResponseRole role = ResponseRole::ASSISTANT;
    StopReason stop_reason;
    std::optional<std::string> stop_sequence{};
    Usage usage;
};


}
