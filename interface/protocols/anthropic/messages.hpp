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
#include "../../core/url.hpp"


namespace jai::llm::anthropic {


/***
 * Kinds
 */
enum class Base64SourceKind { BASE64 };
enum class CitationKinds {
    CHAR_LOCATION,
    CONTENT_BLOCK_LOCATION,
    PAGE_LOCATION,
    SEARCH_RESULT_LOCATION,
    WEB_SEARCH_RESULT_LOCATION
};
enum class ResponseContentBlockKinds {
    TEXT,
    THINKING,
    REDACTED_THINKING,
    TOOL_USE,
    SERVER_TOOL_USE,
    WEB_SEARCH_TOOL_RESULT
};
enum class ToolBash20250124Type { BASH_20250124 };
enum class ToolBash20250124Name { BASH };
enum class CacheControlEphemeralKind { EPHEMERAL };
enum class CharLocationKind { CHAR_LOCATION };
enum class ContentBlockLocationKind { CONTENT_BLOCK_LOCATION };
enum class ContentSourceKind { CONTENT };
enum class ContentUnitKind { TEXT, IMAGE };
enum class CustomToolKind { CUSTOM };
enum class DocumentBlockKind { DOCUMENT };
enum class DocSrcKind { BLOCK, PDF, PLAIN_TEXT, URL_PDF};
enum class ImageBlockKind { IMAGE };
enum class ImageSourceKinds { BASE64, URL };
enum class JsonSchemaTypeKind { OBJECT };
enum class MessageType { MESSAGE };
enum class PageLocationKind { PAGE_LOCATION };
enum class PlainTextSourceKind { TEXT };
enum class RedactedThinkingBlockKind { REDACTED_THINKING };
enum class ReplaceEditor { STRING };
enum class ReplaceBasedEditor { STRING };
enum class ResponseRole { ASSISTANT };
enum class SearchResultBlockKind { SEARCH_RESULT };
enum class SearchResultLocationKind { SEARCH_RESULT_LOCATION };
enum class ServerToolUseBlockKind { SERVER_TOOL_USE };
enum class StructuredOutputFormatKind { JSON_SCHEMA };
enum class TextBlockKind { TEXT };
enum class ToolTextEditor20250124Name { TEXT_EDITOR_20250124 };
enum class ToolTextEditor20250429Name { TEXT_EDITOR_20250429 };
enum class ToolTextEditor20250728Name { TEXT_EDITOR_20250728 };
enum class ThinkingBlockKind { THINKING };
enum class ToolChoiceAnyKind { ANY };
enum class ToolChoiceAutoKind { AUTO };
enum class ToolChoiceNoneKind { NONE };
enum class ToolChoiceToolKind { TOOL };
enum class ToolResultBlockKind { TOOL_RESULT };
enum class WebSearchTool20250305Kind { WEB_SEARCH_20250305 };
enum class ToolUseBlockKind { TOOL_USE };
enum class UrlSourceKind { URL };
enum class UserLocationTypeKind { APPROXIMATE };
enum class WebSearchName { WEB_SEARCH };
enum class WebSearchResultLocationKind { WEB_SEARCH_RESULT_LOCATION };
enum class WebSearchToolResultBlockKind { WEB_SEARCH_TOOL_RESULT };


/***
 * Vocabulary
 */
enum class CacheControlTTL { TTL_5M, TTL_1H };
enum class ImageMediaType { IMAGE_JPEG, IMAGE_PNG, IMAGE_GIF, IMAGE_WEBP };
enum class PDFMediaType { APPLICATION_PDF };
enum class PlainTextMediaType { TEXT_PLAIN };
enum class RequestServiceTier { AUTO, STANDARD_ONLY };
enum class Role { USER, ASSISTANT };
enum class StopReason { END_TURN, MAX_TOKENS, STOP_SEQUENCE, TOOL_USE, PAUSE_TURN, REFUSAL };
enum class ThinkingConfigType { ENABLED, DISABLED };
enum class UsageServiceTier { STANDARD, PRIORITY, BATCH };
enum class WebSearchToolResultErrorCode {
    INVALID_TOOL_INPUT, UNAVAILABLE, MAX_USES_EXCEEDED,
    TOO_MANY_REQUESTS, QUERY_TOO_LONG, REQUEST_TOO_LARGE
};
enum class WebSearchToolResultErrorType { WEB_SEARCH_TOOL_RESULT_ERROR };


/***
 * Request Shared Substructures
 */

struct CacheControlEphemeral {
    CacheControlEphemeralKind type = CacheControlEphemeralKind::EPHEMERAL;
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
    CharLocationKind type = CharLocationKind::CHAR_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_char_index;
    int64_t start_char_index;
};

struct CitationPageLocationParam {
    PageLocationKind type = PageLocationKind::PAGE_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_page_number;
    int64_t start_page_number;
};

struct CitationContentBlockLocationParam {
    ContentBlockLocationKind type = ContentBlockLocationKind::CONTENT_BLOCK_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_block_index;
    int64_t start_block_index;
};

struct CitationWebSearchResultLocationParam {
    WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
    std::string cited_text;
    std::string encrypted_index;
    std::string title;
    EncodedUrl url;
};

struct CitationSearchResultLocationParam {
    SearchResultLocationKind type = SearchResultLocationKind::SEARCH_RESULT_LOCATION;
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
    Base64SourceKind type = Base64SourceKind::BASE64;
    std::string data;
    ImageMediaType media_type;
};

struct URLImageSource {
    UrlSourceKind type = UrlSourceKind::URL;
    EncodedUrl url;
};

using ImageSource = std::variant<Base64ImageSource, URLImageSource>;

struct TextBlockParam {
    TextBlockKind type = TextBlockKind::TEXT;
    std::string text;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::vector<TextCitationParam>> citations{};
};

struct ImageBlockParam {
    ImageBlockKind type = ImageBlockKind::IMAGE;
    ImageSource source;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct Base64PDFSource {
    Base64SourceKind type = Base64SourceKind::BASE64;
    std::string data;
    PDFMediaType media_type;
};

struct PlainTextSource {
    PlainTextSourceKind type = PlainTextSourceKind::TEXT;
    std::string data;
    PlainTextMediaType media_type;
};

struct URLPDFSource {
    UrlSourceKind type = UrlSourceKind::URL;
    EncodedUrl url;
};

struct ContentBlockSource {
    using ContentBlockSourceContent = std::variant<TextBlockParam, ImageBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentBlockSourceContent>>;

    ContentSourceKind type = ContentSourceKind::CONTENT;
    Content content;
};

using DocumentSource = std::variant<
    Base64PDFSource,
    PlainTextSource,
    ContentBlockSource,
    URLPDFSource
>;

struct DocumentBlockParam {
    DocumentBlockKind type = DocumentBlockKind::DOCUMENT;
    DocumentSource source;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
    std::optional<std::string> context{};
    std::optional<std::string> title{};
};

struct SearchResultBlockParam {
    SearchResultBlockKind type = SearchResultBlockKind::SEARCH_RESULT;
    std::vector<TextBlockParam> content;
    std::string source;
    std::string title;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfigParam> citations{};
};

struct ThinkingBlockParam {
    ThinkingBlockKind type = ThinkingBlockKind::THINKING;
    std::string signature;
    std::string thinking;
};

struct RedactedThinkingBlockParam {
    RedactedThinkingBlockKind type = RedactedThinkingBlockKind::REDACTED_THINKING;
    std::string data;
};

struct ToolUseBlockParam {
    ToolUseBlockKind type = ToolUseBlockKind::TOOL_USE;
    std::string id;
    jai::llm::json::Object input;
    std::string name;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct ToolResultBlockParam {
    using ContentUnit = std::variant<TextBlockParam, ImageBlockParam, SearchResultBlockParam, DocumentBlockParam>;
    using Content = std::variant<std::string, std::vector<ContentUnit>>;

    ToolResultBlockKind type = ToolResultBlockKind::TOOL_RESULT;
    std::string tool_use_id;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<Content> content{};
    std::optional<bool> is_error{};
};

struct ServerToolUseBlockParam {
    ServerToolUseBlockKind type = ServerToolUseBlockKind::SERVER_TOOL_USE;
    WebSearchName name = WebSearchName::WEB_SEARCH;
    std::string id;
    jai::llm::json::Object input;
    std::optional<CacheControlEphemeral> cache_control{};
};

struct WebSearchToolResultBlockParam {
    struct WebSearchResultBlockParamItem {
        WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
        std::string encrypted_content;
        std::string title;
        EncodedUrl url;
        std::optional<std::string> page_age{};
    };

    struct WebSearchToolRequestError {
        WebSearchToolResultErrorType type = WebSearchToolResultErrorType::WEB_SEARCH_TOOL_RESULT_ERROR;
        WebSearchToolResultErrorCode error_code;
    };

    using Content = std::variant<std::vector<WebSearchResultBlockParamItem>, WebSearchToolRequestError>;

    WebSearchToolResultBlockKind type = WebSearchToolResultBlockKind::WEB_SEARCH_TOOL_RESULT;
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
        JsonSchemaTypeKind type = JsonSchemaTypeKind::OBJECT;
        std::optional<jai::llm::json::Object> properties{};
        std::optional<std::vector<std::string>> required{};
    };
    InputSchema input_schema;
    std::string name;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::string> description{};
    std::optional<bool> strict{};
    std::optional<CustomToolKind> type{};
};

struct ToolBash20250124 {
    ToolBash20250124Type type = ToolBash20250124Type::BASH_20250124;
    ToolBash20250124Name name = ToolBash20250124Name::BASH;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250124 {
    ToolTextEditor20250124Name type = ToolTextEditor20250124Name::TEXT_EDITOR_20250124;
    ReplaceEditor name = ReplaceEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250429 {
    ToolTextEditor20250429Name type = ToolTextEditor20250429Name::TEXT_EDITOR_20250429;
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250728 {
    ToolTextEditor20250728Name type = ToolTextEditor20250728Name::TEXT_EDITOR_20250728;
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<int64_t> max_characters{};
    std::optional<bool> strict{};
};

struct WebSearchTool20250305 {
    struct UserLocation {
        UserLocationTypeKind type = UserLocationTypeKind::APPROXIMATE;
        std::optional<std::string> city{};
        std::optional<std::string> country{};
        std::optional<std::string> region{};
        std::optional<std::string> timezone{};
    };
    WebSearchTool20250305Kind type = WebSearchTool20250305Kind::WEB_SEARCH_20250305;
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
    ToolChoiceAutoKind type = ToolChoiceAutoKind::AUTO;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceAny {
    ToolChoiceAnyKind type = ToolChoiceAnyKind::ANY;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceTool {
    ToolChoiceToolKind type = ToolChoiceToolKind::TOOL;
    std::string name;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceNone {
    ToolChoiceNoneKind type = ToolChoiceNoneKind::NONE;
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
        StructuredOutputFormatKind type = StructuredOutputFormatKind::JSON_SCHEMA;
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
    using Content = std::variant<
        std::string,
        std::vector<ContentBlockParam>
    >;
    Content content;
    Role role;
};

struct Request {
    int64_t max_tokens;
    std::vector<MessageParam> messages;
    std::string model;
    std::optional<Metadata> metadata{};
    std::optional<OutputConfig> output_config{};
    std::optional<RequestServiceTier> service_tier{};
    std::optional<std::vector<std::string>> stop_sequences{};
    std::optional<bool> stream{};
    using System = std::variant<
        std::string,
        std::vector<TextBlockParam>
    >;
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
    CharLocationKind type = CharLocationKind::CHAR_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_char_index;
    int64_t start_char_index;
    std::string file_id;
};

struct CitationPageLocation {
    PageLocationKind type = PageLocationKind::PAGE_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_page_number;
    int64_t start_page_number;
    std::string file_id;
};

struct CitationContentBlockLocation {
    ContentBlockLocationKind type = ContentBlockLocationKind::CONTENT_BLOCK_LOCATION;
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_block_index;
    int64_t start_block_index;
    std::string file_id;
};

struct CitationsWebSearchResultLocation {
    WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
    std::string cited_text;
    std::string encrypted_index;
    std::string title;
    EncodedUrl url;
};

struct CitationsSearchResultLocation {
    SearchResultLocationKind type = SearchResultLocationKind::SEARCH_RESULT_LOCATION;
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
    TextBlockKind type = TextBlockKind::TEXT;
    std::vector<TextCitation> citations;
    std::string text;
};

struct ThinkingBlock {
    ThinkingBlockKind type = ThinkingBlockKind::THINKING;
    std::string signature;
    std::string thinking;
};

struct RedactedThinkingBlock {
    RedactedThinkingBlockKind type = RedactedThinkingBlockKind::REDACTED_THINKING;
    std::string data;
};

struct ToolUseBlock {
    ToolUseBlockKind type = ToolUseBlockKind::TOOL_USE;
    std::string id;
    jai::llm::json::Object input;
    std::string name;
};

struct ServerToolUseBlock {
    ServerToolUseBlockKind type = ServerToolUseBlockKind::SERVER_TOOL_USE;
    std::string id;
    jai::llm::json::Object input;
    WebSearchName name = WebSearchName::WEB_SEARCH;
};

struct WebSearchToolResultBlock {
    struct WebSearchResultBlock {
        WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
        std::string encrypted_content;
        std::string page_age;
        std::string title;
        EncodedUrl url;
    };

    struct WebSearchToolResultError {
        WebSearchToolResultErrorType type = WebSearchToolResultErrorType::WEB_SEARCH_TOOL_RESULT_ERROR;
        WebSearchToolResultErrorCode error_code;
    };

    using Content = std::variant<WebSearchToolResultError, std::vector<WebSearchResultBlock>>;

    WebSearchToolResultBlockKind type = WebSearchToolResultBlockKind::WEB_SEARCH_TOOL_RESULT;
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
    MessageType type = MessageType::MESSAGE;
    std::string id;
    std::vector<ResponseContentBlock> content;
    std::string model;
    ResponseRole role = ResponseRole::ASSISTANT;
    StopReason stop_reason;
    std::optional<std::string> stop_sequence{};
    Usage usage;
};


}
