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

#include "../types.hpp"


namespace jai::llm::anthropic {


/***
 * Kinds
 */
enum class Base64SourceKind { BASE64 };
enum class ToolBash20250124Type { BASH_20250124 };
enum class ToolBash20250124Name { BASH };
enum class CacheControlEphemeralKind { EPHEMERAL };
enum class CharLocationKind { CHAR_LOCATION };
enum class ContentBlockLocationKind { CONTENT_BLOCK_LOCATION };
enum class ContentSourceKind { CONTENT };
enum class CustomToolKind { CUSTOM };
enum class DocumentBlockKind { DOCUMENT };
enum class ImageBlockKind { IMAGE };
enum class JsonSchemaTypeKind { OBJECT };
enum class MessageType { MESSAGE };
enum class PageLocationKind { PAGE_LOCATION };
enum class PlainTextSourceKind { TEXT };
enum class RedactedThinkingBlockKind { REDACTED_THINKING };
enum class ReplaceEditor { STRING };
enum class ReplaceBasedEditor { STRING };
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
enum class Role { USER, ASSISTANT };
enum class ServiceTier { AUTO, STANDARD_ONLY, STANDARD, PRIORITY, BATCH };
enum class StopReason { END_TURN, MAX_TOKENS, STOP_SEQUENCE, TOOL_USE, PAUSE_TURN, REFUSAL };
enum class ThinkingConfigType { ENABLED, DISABLED };
enum class WebSearchToolResultErrorCode {
    INVALID_TOOL_INPUT, UNAVAILABLE, MAX_USES_EXCEEDED,
    TOO_MANY_REQUESTS, QUERY_TOO_LONG, REQUEST_TOO_LARGE
};
enum class WebSearchToolResultErrorType { WEB_SEARCH_TOOL_RESULT_ERROR };


/***
 * Shared Substructures (Primitives)
 */

struct CacheControlEphemeral {
    CacheControlEphemeralKind type = CacheControlEphemeralKind::EPHEMERAL;
    std::optional<CacheControlTTL> ttl{};
};

struct CitationsConfig {
    std::optional<bool> enabled{};
};


/***
 * Citations
 */

struct CitationCharLocation {
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_char_index;
    int64_t start_char_index;
    CharLocationKind type = CharLocationKind::CHAR_LOCATION;
};

struct CitationPageLocation {
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_page_number;
    int64_t start_page_number;
    PageLocationKind type = PageLocationKind::PAGE_LOCATION;
};

struct CitationContentBlockLocation {
    std::string cited_text;
    int64_t document_index;
    std::string document_title;
    int64_t end_block_index;
    int64_t start_block_index;
    ContentBlockLocationKind type = ContentBlockLocationKind::CONTENT_BLOCK_LOCATION;
};

struct CitationWebSearchResultLocation {
    std::string cited_text;
    std::string encrypted_index;
    std::string title;
    WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
    std::string url;
};

struct CitationSearchResultLocation {
    std::string cited_text;
    int64_t end_block_index;
    int64_t search_result_index;
    std::string source;
    int64_t start_block_index;
    std::string title;
    SearchResultLocationKind type = SearchResultLocationKind::SEARCH_RESULT_LOCATION;
};

using Citation = std::variant<
    CitationCharLocation, CitationPageLocation, CitationContentBlockLocation,
    CitationWebSearchResultLocation, CitationSearchResultLocation
>;


/***
 * Message Blocks (Base)
 */

struct TextBlock {
    std::string text;
    TextBlockKind type = TextBlockKind::TEXT;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<std::vector<Citation>> citations{};
};

struct Base64ImageSource {
    std::string data;
    ImageMediaType media_type;
    Base64SourceKind type = Base64SourceKind::BASE64;
};

struct UrlImageSource {
    UrlSourceKind type = UrlSourceKind::URL;
    std::string url;
};

using ImageSource = std::variant<Base64ImageSource, UrlImageSource>;

struct ImageBlock {
    ImageSource source;
    ImageBlockKind type = ImageBlockKind::IMAGE;
    std::optional<CacheControlEphemeral> cache_control{};
};


/***
 * Recursive Sources & Documents
 */

struct ContentBlockSource {
    using Content = std::variant<std::string, std::vector<std::variant<TextBlock, ImageBlock>>>;
    Content content;
    ContentSourceKind type = ContentSourceKind::CONTENT;
};

struct Base64PDFSource {
    std::string data;
    PDFMediaType media_type;
    Base64SourceKind type = Base64SourceKind::BASE64;
};

struct PlainTextSource {
    std::string data;
    PlainTextMediaType media_type;
    PlainTextSourceKind type = PlainTextSourceKind::TEXT;
};

struct URLPDFSource {
    UrlSourceKind type = UrlSourceKind::URL;
    std::string url;
};

using DocumentSource = std::variant<
    Base64PDFSource, PlainTextSource, URLPDFSource, ContentBlockSource
>;

struct DocumentBlock {
    DocumentSource source;
    DocumentBlockKind type = DocumentBlockKind::DOCUMENT;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfig> citations{};
    std::optional<std::string> context{};
    std::optional<std::string> title{};
};

struct SearchResultBlock {
    std::vector<TextBlock> content;
    std::string source;
    std::string title;
    SearchResultBlockKind type = SearchResultBlockKind::SEARCH_RESULT;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<CitationsConfig> citations{};
};


/***
 * Thinking Blocks
 */

struct ThinkingBlock {
    std::string signature;
    std::string thinking;
    ThinkingBlockKind type = ThinkingBlockKind::THINKING;
};

struct RedactedThinkingBlock {
    std::string data;
    RedactedThinkingBlockKind type = RedactedThinkingBlockKind::REDACTED_THINKING;
};


/***
 * Tool Call Blocks
 */

struct ToolUseBlock {
    std::string id;
    jai::llm::json::Object input;
    std::string name;
    ToolUseBlockKind type = ToolUseBlockKind::TOOL_USE;
};

struct ToolResultBlock {
    using Content = std::variant<std::string, std::vector<std::variant<TextBlock, ImageBlock, SearchResultBlock, DocumentBlock>>>;
    std::string tool_use_id;
    ToolResultBlockKind type = ToolResultBlockKind::TOOL_RESULT;
    std::optional<Content> content{};
    std::optional<bool> is_error{};
};

struct ServerToolUseBlock {
    std::string id;
    jai::llm::json::Object input;
    WebSearchName name = WebSearchName::WEB_SEARCH;
    ServerToolUseBlockKind type = ServerToolUseBlockKind::SERVER_TOOL_USE;
};

struct WebSearchToolResultBlock {
    struct WebSearchResultItem {
        std::string encrypted_content;
        std::string title;
        WebSearchResultLocationKind type = WebSearchResultLocationKind::WEB_SEARCH_RESULT_LOCATION;
        std::string url;
        std::optional<std::string> page_age{};
    };
    struct Error {
        WebSearchToolResultErrorCode error_code;
        WebSearchToolResultErrorType type = WebSearchToolResultErrorType::WEB_SEARCH_TOOL_RESULT_ERROR;
    };
    using Content = std::variant<std::vector<WebSearchResultItem>, Error>;
    Content content;
    std::string tool_use_id;
    WebSearchToolResultBlockKind type = WebSearchToolResultBlockKind::WEB_SEARCH_TOOL_RESULT;
};


/***
 * Messaging (Polymorphic)
 */

using ContentBlock = std::variant<
    TextBlock, ImageBlock, DocumentBlock, SearchResultBlock,
    ThinkingBlock, RedactedThinkingBlock, ToolUseBlock, ToolResultBlock,
    ServerToolUseBlock, WebSearchToolResultBlock
>;

struct MessageParam {
    // A string content value is semantically equivalent to a single TextBlock.
    using Content = std::variant<std::string, std::vector<ContentBlock>>;
    Content content;
    Role role;
};


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
    ToolBash20250124Name name = ToolBash20250124Name::BASH;
    ToolBash20250124Type type = ToolBash20250124Type::BASH_20250124;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250124 {
    ReplaceEditor name = ReplaceEditor::STRING;
    ToolTextEditor20250124Name type = ToolTextEditor20250124Name::TEXT_EDITOR_20250124;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250429 {
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    ToolTextEditor20250429Name type = ToolTextEditor20250429Name::TEXT_EDITOR_20250429;
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<bool> strict{};
};

struct ToolTextEditor20250728 {
    ReplaceBasedEditor name = ReplaceBasedEditor::STRING;
    ToolTextEditor20250728Name type = ToolTextEditor20250728Name::TEXT_EDITOR_20250728;
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
    WebSearchName name = WebSearchName::WEB_SEARCH;
    WebSearchTool20250305Kind type = WebSearchTool20250305Kind::WEB_SEARCH_20250305;
    std::optional<std::vector<std::string>> allowed_domains{};
    std::optional<std::vector<std::string>> blocked_domains{};
    std::optional<CacheControlEphemeral> cache_control{};
    std::optional<int64_t> max_uses{};
    std::optional<bool> strict{};
    std::optional<UserLocation> user_location{};
};

using ToolUnion = std::variant<
    Tool, ToolBash20250124, ToolTextEditor20250124, ToolTextEditor20250429, ToolTextEditor20250728,
    WebSearchTool20250305
>;

struct ToolChoiceAuto {
    ToolChoiceAutoKind type = ToolChoiceAutoKind::AUTO;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceAny {
    ToolChoiceAnyKind type = ToolChoiceAnyKind::ANY;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceTool {
    std::string name;
    ToolChoiceToolKind type = ToolChoiceToolKind::TOOL;
    std::optional<bool> disable_parallel_tool_use{};
};

struct ToolChoiceNone {
    ToolChoiceNoneKind type = ToolChoiceNoneKind::NONE;
};

using ToolChoice = std::variant<
    ToolChoiceAuto, ToolChoiceAny, ToolChoiceTool, ToolChoiceNone
>;


/***
 * Infrastructure Root (Phase 5)
 */

struct Usage {
    struct CacheCreation {
        int64_t ephemeral_1h_input_tokens;
        int64_t ephemeral_5m_input_tokens;
    };
    struct ServerToolUsage {
        int64_t web_search_requests;
    };
    std::optional<CacheCreation> cache_creation{};
    int64_t cache_creation_input_tokens;
    int64_t cache_read_input_tokens;
    int64_t input_tokens;
    int64_t output_tokens;
    std::optional<ServerToolUsage> server_tool_use{};
    std::optional<ServiceTier> service_tier{};
};

struct Metadata {
    std::optional<std::string> user_id{};
};

struct OutputConfig {
    struct Format {
        jai::llm::json::Object schema;
        StructuredOutputFormatKind type = StructuredOutputFormatKind::JSON_SCHEMA;
    };
    std::optional<Format> format{};
};

struct ThinkingConfigEnabled {
    int64_t budget_tokens;
    ThinkingConfigType type = ThinkingConfigType::ENABLED;
};

struct ThinkingConfigDisabled {
    ThinkingConfigType type = ThinkingConfigType::DISABLED;
};

using ThinkingConfig = std::variant<ThinkingConfigEnabled, ThinkingConfigDisabled>;

struct Request {
    int64_t max_tokens;
    std::vector<MessageParam> messages;
    std::string model;
    std::optional<Metadata> metadata{};
    std::optional<OutputConfig> output_config{};
    std::optional<std::vector<std::string>> stop_sequences{};
    std::optional<bool> stream{};
    std::optional<std::string> system{}; // system prompt
    std::optional<double> temperature{};
    std::optional<ThinkingConfig> thinking{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<std::vector<ToolUnion>> tools{};
    std::optional<int64_t> top_k{};
    std::optional<double> top_p{};
};

struct Response {
    std::string id;
    std::vector<ContentBlock> content;
    std::string model;
    Role role = Role::ASSISTANT;
    StopReason stop_reason;
    std::optional<std::string> stop_sequence{};
    MessageType type = MessageType::MESSAGE;
    Usage usage;
};


}
