#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <map>

#include "../types.hpp"


namespace jai::llm::openai {


/***
 * Vocabulary
 */
enum class Role { user, assistant, system, developer };
enum class ItemStatus { in_progress, completed, incomplete };
enum class Detail { high, low, auto_detail }; // auto is a keyword
enum class MouseButton { left, right, wheel, back, forward };
enum class FileSearchStatus { in_progress, searching, incomplete, failed };
enum class WebSearchActionType { search, open_page, find };
enum class WebSearchStatus { in_progress, completed, incomplete }; // Deduced from context
enum class FunctionCallStatus { in_progress, completed, incomplete };
enum class ReasoningStatus { in_progress, completed, incomplete };
enum class CodeInterpreterStatus { in_progress, completed, incomplete, interpreting, failed };
enum class ServiceTier { auto_tier, default_tier, flex, scale, priority }; // auto/default are keywords
enum class TruncationStrategy { auto_truncation, disabled }; // auto is a keyword
enum class ReasoningEffort { none, minimal, low, medium, high, xhigh };
enum class ReasoningSummary { auto_summary, concise, detailed }; // auto is a keyword
enum class Verbosity { low, medium, high };
enum class FilterOperator { eq, ne, gt, gte, lt, lte, in, nin };
enum class SearchContextSize { low, medium, high };
enum class ImageGenerationBackground { transparent, opaque, auto_background };
enum class ImageGenerationFidelity { high, low };
enum class ImageGenerationFormat { png, webp, jpeg };
enum class ImageGenerationQuality { low, medium, high, auto_quality };
enum class ImageGenerationSize { size_1024_1024, size_1024_1536, size_1536_1024, auto_size };
enum class GrammarSyntax { lark, regex };
enum class PromptCacheRetention { hours_24 };
enum class PendingSafetyCheckStatus { in_progress, completed, incomplete };
enum class ComputerActionType { click, double_click, drag, keypress, move, screenshot, scroll, type, wait };
enum class ResponseStatus { completed, failed, in_progress, cancelled, queued, incomplete };
enum class IncompleteReason { max_output_tokens, content_filter };


/***
 * Shared Substructures
 */

struct ConversationRef {
    std::string id;
};

struct PromptRef {
    std::string id;
    std::optional<std::map<std::string, std::variant<std::string, jai::llm::json::Object>>> variables{};
    std::optional<std::string> version{};
};

struct TextConfig {
    std::optional<jai::llm::json::Object> format{};
    std::optional<Verbosity> verbosity{};
};

struct StreamOptions {
    std::optional<bool> include_obfuscation{};
};

struct ReasoningConfig {
    std::optional<ReasoningEffort> effort{};
    std::optional<ReasoningSummary> summary{};
};

struct InputText {
    std::string text;
    std::string type = "input_text";
};

struct InputImage {
    Detail detail; // Required defaults to auto, but marked required in doc
    std::string type = "input_image";
    std::optional<std::string> file_id{};
    std::optional<std::string> image_url{};
};

struct InputFile {
    std::string type = "input_file";
    std::optional<std::string> file_data{};
    std::optional<std::string> file_id{};
    std::optional<std::string> file_url{};
    std::optional<std::string> filename{};
};

struct ItemReference {
    std::string id;
    std::string type{"item_reference"}; // always "item_reference".  May not be present in JSON response.
};


/***
 * Tool Call Actions and Operations (Block 2)
 */

// Computer Tool Call Actions
struct ClickAction {
    MouseButton button;
    std::string type = "click";
    int64_t x;
    int64_t y;
};

struct DoubleClickAction {
    std::string type = "double_click";
    int64_t x;
    int64_t y;
};

struct DragAction {
    struct Coordinate {
        int64_t x;
        int64_t y;
    };
    std::vector<Coordinate> path;
    std::string type = "drag";
};

struct KeyPressAction {
    std::vector<std::string> keys;
    std::string type = "keypress";
};

struct MoveAction {
    std::string type = "move";
    int64_t x;
    int64_t y;
};

struct ScreenshotAction {
    std::string type = "screenshot";
};

struct ScrollAction {
    int64_t scroll_x;
    int64_t scroll_y;
    std::string type = "scroll";
    int64_t x;
    int64_t y;
};

struct TypeAction {
    std::string text;
    std::string type = "type";
};

struct WaitAction {
    std::string type = "wait";
};

using ComputerAction = std::variant<
    ClickAction, DoubleClickAction, DragAction, KeyPressAction,
    MoveAction, ScreenshotAction, ScrollAction, TypeAction, WaitAction
>;

// Web Search Tool Call Actions
struct SearchAction {
    std::string type; // Always "search"
    std::optional<std::vector<std::string>> queries{};
    struct Source {
        std::string type = "url";
        std::string url;
    };
    std::optional<std::vector<Source>> sources{};
};

struct OpenPageAction {
    std::string type; // Always "open_page"
    std::string url;
};

struct FindAction {
    std::string pattern;
    std::string type; // Always "find"
    std::string url;
};

using WebSearchAction = std::variant<SearchAction, OpenPageAction, FindAction>;

// Apply Patch Operations
struct CreateFileOperation {
    std::string diff;
    std::string path;
    std::string type = "create_file";
};

struct DeleteFileOperation {
    std::string path;
    std::string type = "delete_file";
};

struct UpdateFileOperation {
    std::string diff;
    std::string path;
    std::string type = "update_file";
};

using ApplyPatchOperation = std::variant<CreateFileOperation, DeleteFileOperation, UpdateFileOperation>;


/***
 * Tool Call and Output structs (Block 3)
 */

struct FileSearchCall {
    struct Result {
        std::optional<std::map<std::string, std::variant<std::string, bool, double>>> attributes{};
        std::optional<std::string> file_id{};
        std::optional<std::string> filename{};
        std::optional<double> score{};
        std::optional<std::string> text{};
    };
    std::string id;
    std::vector<std::string> queries;
    FileSearchStatus status;
    std::string type = "file_search_call";
    std::optional<std::vector<Result>> results{};
};

struct ComputerCall {
    struct PendingSafetyCheck {
        std::string id;
        std::optional<std::string> code{};
        std::optional<std::string> message{};
    };
    ComputerAction action;
    std::string call_id;
    std::string id;
    std::vector<PendingSafetyCheck> pending_safety_checks;
    ItemStatus status;
    std::string type = "computer_call";
};

struct ComputerCallOutput {
    std::string call_id;
    jai::llm::json::Object output;
    std::string type = "computer_call_output";
    std::optional<std::vector<std::string>> acknowledged_safety_checks{};
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct WebSearchCall {
    WebSearchAction action;
    std::string id;
    ItemStatus status; // Doc says "status" but doesn't list enum. Assuming ItemStatus.
    std::string type = "web_search_call";
};

struct FunctionCall {
    std::string arguments;
    std::string call_id;
    std::string name;
    std::string type = "function_call";
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct FunctionCallOutput {
    std::string call_id;
    std::variant<std::string, std::vector<std::variant<InputText, InputImage, InputFile>>> output;
    std::string type = "function_call_output";
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct ReasoningItem {
    struct Summary {
        std::string text;
        std::string type = "summary_text";
    };
    struct Content {
        std::string text;
        std::string type = "reasoning_text";
    };
    std::string id;
    std::optional<std::vector<Summary>> summary{};
    std::string type = "reasoning";
    std::optional<std::vector<Content>> content{};
    std::optional<std::string> encrypted_content{};
    std::optional<ItemStatus> status{};
};

struct CompactionItem {
    std::string encrypted_content;
    std::string type = "compaction";
    std::optional<std::string> id{};
};

struct ImageGenerationCall {
    std::string id;
    std::string result;
    std::string status;
    std::string type = "image_generation_call";
};

struct CodeInterpreterCall {
    struct OutputLog {
        std::string logs;
        std::string type = "logs";
    };
    struct OutputImage {
        std::string type = "image";
        std::string url;
    };
    using Output = std::variant<OutputLog, OutputImage>;

    std::optional<std::string> code{}; // Required if available
    std::string container_id;
    std::string id;
    std::optional<std::vector<Output>> outputs{}; // Required if available
    CodeInterpreterStatus status;
    std::string type = "code_interpreter_call";
};

struct LocalShellCall { // client-side execution
    struct Action {
        std::vector<std::string> command;
        std::map<std::string, std::string> env;
        std::string type = "exec";
        std::optional<int64_t> timeout_ms{};
        std::optional<std::string> user{};
        std::optional<std::string> working_directory{};
    };
    Action action;
    std::string call_id;
    std::string id;
    std::string status; // Not specified values
    std::string type = "local_shell_call";
};

struct LocalShellCallOutput {
    std::string id;
    std::string output;
    std::string type = "local_shell_call_output";
    std::optional<std::string> status{};
};

struct ShellCall { // hosted / sandboxed execution (if supported)
    struct Action {
        std::vector<std::string> commands;
        std::optional<int64_t> max_output_length{};
        std::optional<int64_t> timeout_ms{};
    };
    Action action;
    std::string call_id;
    std::string type = "shell_call";
    std::optional<std::string> id{};
    std::optional<std::string> status{};
};

struct ShellCallOutput {
    struct OutcomeExit {
        int64_t exit_code;
        std::string type = "exit";
    };
    struct OutcomeTimeout {
        std::string type = "timeout";
    };
    struct OutputItem {
        std::variant<OutcomeExit, OutcomeTimeout> outcome;
        std::string stderr_text; // Doc says stderr
        std::string stdout_text; // Doc says stdout
    };
    std::string call_id;
    std::vector<OutputItem> output;
    std::string type = "shell_call_output";
    std::optional<std::string> id{};
    std::optional<int64_t> max_output_length{};
    std::optional<std::string> status{};
};

struct ApplyPatchCall {
    std::string call_id;
    ApplyPatchOperation operation;
    std::string status;
    std::string type = "apply_patch_call";
    std::optional<std::string> id{};
};

struct ApplyPatchCallOutput {
    std::string call_id;
    std::string status;
    std::string type = "apply_patch_call_output";
    std::optional<std::string> id{};
    std::optional<std::string> output{};
};

struct McpListTools {
    struct ToolDef {
        jai::llm::json::Object input_schema;
        std::string name;
        std::optional<jai::llm::json::Object> annotations{};
        std::optional<std::string> description{};
    };
    std::string id;
    std::string server_label;
    std::vector<ToolDef> tools;
    std::string type = "mcp_list_tools";
    std::optional<std::string> error{};
};

struct McpApprovalRequest {
    std::string arguments;
    std::string id;
    std::string name;
    std::string server_label;
    std::string type = "mcp_approval_request";
};

struct McpApprovalResponse {
    std::string approval_request_id;
    bool approve;
    std::string type = "mcp_approval_response";
    std::optional<std::string> id{};
    std::optional<std::string> reason{};
};

struct McpCall {
    std::string arguments;
    std::string id;
    std::string name;
    std::string server_label;
    std::string type = "mcp_call";
    std::optional<std::string> approval_request_id{};
    std::optional<std::string> error{};
    std::optional<std::string> output{};
    std::optional<std::string> status{};
};

struct CustomToolCall {
    std::string call_id;
    std::string input;
    std::string name;
    std::string type = "custom_tool_call";
    std::optional<std::string> id{};
};

struct CustomToolCallOutput {
    std::string call_id;
    std::variant<std::string, std::vector<std::variant<InputText, InputImage, InputFile>>> output;
    std::string type = "custom_tool_call_output";
    std::optional<std::string> id{};
};


/***
 * Message and InputItem variants (Block 4)
 */

struct InputMessage {
    std::variant<std::string, std::vector<std::variant<InputText, InputImage, InputFile>>> content;
    Role role;
    std::optional<ItemStatus> status{};
    std::optional<std::string> type{"message"};
};

struct OutputMessage {
    struct OutputText {
        struct CitationFile {
            std::string file_id;
            std::string filename;
            int64_t index;
            std::string type = "file_citation";
        };
        struct CitationUrl {
            int64_t end_index;
            int64_t start_index;
            std::string title;
            std::string type = "url_citation";
            std::string url;
        };
        struct CitationContainer {
            std::string container_id;
            int64_t end_index;
            std::string file_id;
            std::string filename;
            int64_t start_index;
            std::string type = "container_file_citation";
        };
        struct FilePath {
            std::string file_id;
            int64_t index;
            std::string type = "file_path";
        };
        using Annotation = std::variant<CitationFile, CitationUrl, CitationContainer, FilePath>;

        struct Logprob {
            std::vector<uint8_t> bytes;
            double logprob;
            std::string token;
            struct TopLogprob {
                std::vector<uint8_t> bytes;
                double logprob;
                std::string token;
            };
            std::vector<TopLogprob> top_logprobs;
        };

        std::vector<Annotation> annotations;
        std::string text;
        std::string type = "output_text";
        std::optional<std::vector<Logprob>> logprobs{};
    };

    struct Refusal {
        std::string refusal;
        std::string type = "refusal";
    };

    using ContentItem = std::variant<OutputText, Refusal>;

    std::vector<ContentItem> content;
    std::string id;
    Role role = Role::assistant;
    ItemStatus status;
    std::string type = "message";
};

using InputItem = std::variant<
    InputMessage,
    OutputMessage,
    FileSearchCall,
    ComputerCall,
    ComputerCallOutput,
    WebSearchCall,
    FunctionCall,
    FunctionCallOutput,
    ReasoningItem,
    CompactionItem,
    ImageGenerationCall,
    CodeInterpreterCall,
    LocalShellCall,
    LocalShellCallOutput,
    ShellCall,
    ShellCallOutput,
    ApplyPatchCall,
    ApplyPatchCallOutput,
    McpListTools,
    McpApprovalRequest,
    McpApprovalResponse,
    McpCall,
    CustomToolCall,
    CustomToolCallOutput,
    ItemReference
>;
/***
 * Model Tools (Block 5)
 */

struct FunctionTool {
    std::string name;
    jai::llm::json::Object parameters;
    bool strict = true;
    std::string type = "function";
    std::optional<std::string> description{};
};

struct FileSearchTool {
    struct FilterComparison {
        std::string key;
        FilterOperator type;
        std::variant<std::string, double, bool, std::vector<std::variant<std::string, double, bool>>> value;
    };
    struct FilterCompound {
        std::vector<std::variant<FilterComparison, FilterCompound>> filters;
        std::string type; // "and" or "or"
    };
    using Filter = std::variant<FilterComparison, FilterCompound>;

    struct RankingOptions {
        struct HybridSearch {
            double embedding_weight;
            double text_weight;
        };
        std::optional<HybridSearch> hybrid_search{};
        std::optional<std::string> ranker{};
        std::optional<double> score_threshold{};
    };

    std::string type = "file_search";
    std::vector<std::string> vector_store_ids;
    std::optional<Filter> filters{};
    std::optional<int64_t> max_num_results{};
    std::optional<RankingOptions> ranking_options{};
};

struct ComputerUseTool {
    int64_t display_height;
    int64_t display_width;
    std::string environment;
    std::string type = "computer_use_preview";
};

struct WebSearchTool {
    std::string type; // web_search or web_search_2025_08_26
    struct Filters {
        std::optional<std::vector<std::string>> allowed_domains{};
        std::optional<SearchContextSize> search_context_size{};
        struct Location {
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
            std::optional<std::string> type{"approximate"};
        };
        std::optional<Location> user_location{};
    };
    std::optional<Filters> filters{};
};

struct McpTool {
    std::string server_label;
    std::string type = "mcp";
    struct ToolFilter {
        // Doc says "List of allowed tool names or a filter object"
        // Show possible types: MCP allowed tools (array), MCP tool filter (object)
        std::optional<std::variant<std::vector<std::string>, jai::llm::json::Object>> filter{};
    };
    std::optional<std::variant<std::vector<std::string>, jai::llm::json::Object>> allowed_tools{};
    std::optional<std::string> authorization{};
    std::optional<std::string> connector_id{};
    std::optional<jai::llm::json::Object> headers{};
    struct ApprovalFilter {
        std::optional<std::variant<std::string, jai::llm::json::Object>> setting{};
    };
    std::optional<std::variant<std::string, jai::llm::json::Object>> require_approval{};
    std::optional<std::string> server_description{};
    std::optional<std::string> server_url{};
};

struct CodeInterpreterTool {
    std::string type = "code_interpreter";
    struct Container {
        // Can be a container ID or an object
        std::variant<std::string, jai::llm::json::Object> config;
    };
    Container container;
};

struct ImageGenerationTool {
    std::string type = "image_generation";
    std::optional<ImageGenerationBackground> background{};
    std::optional<ImageGenerationFidelity> input_fidelity{};
    struct Mask {
        std::optional<std::string> file_id{};
        std::optional<std::string> image_url{};
    };
    std::optional<Mask> input_image_mask{};
    std::optional<std::string> model{};
    std::optional<std::string> moderation{};
    std::optional<int64_t> output_compression{};
    std::optional<ImageGenerationFormat> output_format{};
    std::optional<int64_t> partial_images{};
    std::optional<ImageGenerationQuality> quality{};
    std::optional<ImageGenerationSize> size{};
};

struct LocalShellTool {
    std::string type = "local_shell";
};

struct ShellTool {
    std::string type = "shell";
};

struct CustomTool {
    std::string name;
    std::string type = "custom";
    std::optional<std::string> description{};
    struct TextFormat {
        std::string type = "text";
    };
    struct GrammarFormat {
        std::string definition;
        GrammarSyntax syntax;
        std::string type = "grammar";
    };
    std::optional<std::variant<TextFormat, GrammarFormat>> format{};
};

struct WebSearchPreviewTool {
    std::string type; // web_search_preview or web_search_preview_2025_03_11
    std::optional<SearchContextSize> search_context_size{};
    struct Location {
        std::string type = "approximate";
        std::optional<std::string> city{};
        std::optional<std::string> country{};
        std::optional<std::string> region{};
        std::optional<std::string> timezone{};
    };
    std::optional<Location> user_location{};
};

struct ApplyPatchTool {
    std::string type = "apply_patch";
};

using Tool = std::variant<
    FunctionTool, FileSearchTool, ComputerUseTool, WebSearchTool, 
    McpTool, CodeInterpreterTool, ImageGenerationTool, LocalShellTool, 
    ShellTool, CustomTool, WebSearchPreviewTool, ApplyPatchTool
>;
/***
 * Tool Choice models (Block 6)
 */

struct AllowedToolsChoice {
    std::string mode;
    std::vector<jai::llm::json::Object> tools;
    std::string type = "allowed_tools";
};

struct HostedToolChoice {
    std::string type;
};

struct FunctionToolChoice {
    std::string name;
    std::string type = "function";
};

struct McpToolChoice {
    std::string server_label;
    std::string type = "mcp";
    std::optional<std::string> name{};
};

struct CustomToolChoice {
    std::string name;
    std::string type = "custom";
};

struct SpecificApplyPatchToolChoice {
    std::string type = "apply_patch";
};

struct SpecificShellToolChoice {
    std::string type = "shell";
};

using ToolChoice = std::variant<
    std::string, // none, auto, required
    AllowedToolsChoice,
    HostedToolChoice,
    FunctionToolChoice,
    McpToolChoice,
    CustomToolChoice,
    SpecificApplyPatchToolChoice,
    SpecificShellToolChoice
>;


/***
 * ResponseRequest (Block 7)
 */

struct ResponseRequest {
    std::optional<bool> background{};
    std::optional<std::variant<std::string, ConversationRef>> conversation{};
    std::optional<std::vector<std::string>> include{};
    std::optional<std::variant<std::string, std::vector<InputItem>>> input{};
    std::optional<std::variant<std::string, std::vector<InputMessage>>> instructions{};
    std::optional<int64_t> max_output_tokens{};
    std::optional<int64_t> max_tool_calls{};
    std::optional<std::map<std::string, std::string>> metadata{};
    std::optional<std::string> model{};
    std::optional<bool> parallel_tool_calls{};
    std::optional<std::string> previous_response_id{};
    std::optional<PromptRef> prompt{};
    std::optional<std::string> prompt_cache_key{};
    std::optional<std::string> prompt_cache_retention{};
    std::optional<ReasoningConfig> reasoning{};
    std::optional<std::string> safety_identifier{};
    std::optional<std::string> service_tier{};
    std::optional<bool> store{};
    std::optional<bool> stream{};
    std::optional<StreamOptions> stream_options{};
    std::optional<double> temperature{};
    std::optional<TextConfig> text{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<std::vector<Tool>> tools{};
    std::optional<int64_t> top_logprobs{};
    std::optional<double> top_p{};
    std::optional<std::string> truncation{};
};


/***
 * Response Substructures
 */

struct ResponseError {
    std::string code;
    std::string message;
};

struct IncompleteDetails {
    std::string reason;
};

struct ResponseUsage {
    struct OutputTokenDetails {
        std::optional<int64_t> accepted_prediction_tokens{};
        std::optional<int64_t> audio_tokens{};
        std::optional<int64_t> reasoning_tokens{};
        std::optional<int64_t> rejected_prediction_tokens{};
    };
    int64_t input_tokens;
    int64_t output_tokens;
    int64_t total_tokens;
    std::optional<OutputTokenDetails> output_token_details{};
};


/***
 * Response
 */

struct Response {
    std::string id;
    std::string object = "response";
    std::optional<bool> background{};
    std::optional<int64_t> completed_at{};
    std::optional<ConversationRef> conversation{};
    std::optional<int64_t> created_at{};
    std::optional<ResponseError> error{};
    std::optional<IncompleteDetails> incomplete_details{};
    std::optional<int64_t> max_output_tokens{};
    std::optional<int64_t> max_tool_calls{};
    std::optional<std::map<std::string, std::string>> metadata{};
    std::optional<std::string> model{};
    std::optional<std::vector<InputItem>> output{};
    std::optional<ResponseStatus> status{};
    std::optional<ResponseUsage> usage{};
};


}
