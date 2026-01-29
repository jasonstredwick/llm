/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <map>

#include "../types.hpp"


namespace jai::llm::openai {


/***
 * Kinds
 */
enum class AnnotationType { FILE_CITATION, URL_CITATION, CONTAINER_FILE_CITATION, FILE_PATH };
enum class ApplyPatchOperationType { CREATE_FILE, DELETE_FILE, UPDATE_FILE };
enum class CodeInterpreterOutputType { LOGS, IMAGE };
enum class ComputerActionType { CLICK, DOUBLE_CLICK, DRAG, KEYPRESS, MOVE, SCREENSHOT, SCROLL, TYPE, WAIT };
enum class CustomToolFormatType { TEXT, GRAMMAR };
enum class InputItemType {
    APPLY_PATCH_CALL, APPLY_PATCH_CALL_OUTPUT, CODE_INTERPRETER_CALL, COMPACTION, COMPUTER_CALL, COMPUTER_CALL_OUTPUT,
    CUSTOM_TOOL_CALL, CUSTOM_TOOL_CALL_OUTPUT, FILE_SEARCH_CALL, FUNCTION_CALL, FUNCTION_CALL_OUTPUT, INPUT_FILE,
    INPUT_IMAGE, INPUT_TEXT, IMAGE_GENERATION_CALL, ITEM_REFERENCE, LOCAL_SHELL_CALL, LOCAL_SHELL_CALL_OUTPUT,
    MCP_APPROVAL_REQUEST, MCP_APPROVAL_RESPONSE, MCP_CALL, MCP_LIST_TOOLS, MESSAGE, REASONING, SHELL_CALL,
    SHELL_CALL_OUTPUT, WEB_SEARCH_CALL
};
enum class OutputMessageContentType { OUTPUT_TEXT, REFUSAL };
enum class ShellCallOutcomeType { EXIT, TIMEOUT };
enum class ToolChoiceType { ALLOWED_TOOLS, FUNCTION, MCP, CUSTOM, APPLY_PATCH, SHELL };
enum class ToolType {
    APPLY_PATCH, CODE_INTERPRETER, COMPUTER_USE_PREVIEW, CUSTOM, FILE_SEARCH, FUNCTION, IMAGE_GENERATION,
    LOCAL_SHELL, MCP, SHELL, WEB_SEARCH, WEB_SEARCH_PREVIEW
};
enum class WebSearchActionType { SEARCH, OPEN_PAGE, FIND };


/***
 * Kinds (Individual)
 */
enum class AllowedToolsChoiceKind { ALLOWED_TOOLS };
enum class ApplyPatchCallKind { APPLY_PATCH_CALL };
enum class ApplyPatchCallOutputKind { APPLY_PATCH_CALL_OUTPUT };
enum class ApplyPatchToolKind { APPLY_PATCH };
enum class ClickActionKind { CLICK };
enum class CodeInterpreterCallKind { CODE_INTERPRETER_CALL };
enum class CodeInterpreterImageKind { IMAGE };
enum class CodeInterpreterLogKind { LOGS };
enum class CodeInterpreterToolKind { CODE_INTERPRETER };
enum class CompactionItemKind { COMPACTION };
enum class ComputerCallKind { COMPUTER_CALL };
enum class ComputerCallOutputKind { COMPUTER_CALL_OUTPUT };
enum class ComputerScreenshotKind { COMPUTER_SCREENSHOT };
enum class ComputerUseToolKind { COMPUTER_USE_PREVIEW };
enum class ContainerFileCitationKind { CONTAINER_FILE_CITATION };
enum class CreateFileOperationKind { CREATE_FILE };
enum class CustomToolCallKind { CUSTOM_TOOL_CALL };
enum class CustomToolCallOutputKind { CUSTOM_TOOL_CALL_OUTPUT };
enum class CustomToolChoiceKind { CUSTOM };
enum class CustomToolGrammarFormatKind { GRAMMAR };
enum class CustomToolKind { CUSTOM };
enum class CustomToolTextFormatKind { TEXT };
enum class DeleteFileOperationKind { DELETE_FILE };
enum class DoubleClickActionKind { DOUBLE_CLICK };
enum class DragActionKind { DRAG };
enum class FileCitationKind { FILE_CITATION };
enum class FilePathKind { FILE_PATH };
enum class FileSearchCallKind { FILE_SEARCH_CALL };
enum class FileSearchToolKind { FILE_SEARCH };
enum class FindActionKind { FIND };
enum class FunctionCallKind { FUNCTION_CALL };
enum class FunctionCallOutputKind { FUNCTION_CALL_OUTPUT };
enum class FunctionToolChoiceKind { FUNCTION };
enum class FunctionToolKind { FUNCTION };
enum class ImageGenerationCallKind { IMAGE_GENERATION_CALL };
enum class ImageGenerationToolKind { IMAGE_GENERATION };
enum class InputFileKind { INPUT_FILE };
enum class InputImageKind { INPUT_IMAGE };
enum class InputMessageKind { MESSAGE };
enum class InputTextKind { INPUT_TEXT };
enum class ItemReferenceKind { ITEM_REFERENCE };
enum class KeyPressActionKind { KEYPRESS };
enum class LocalShellActionKind { EXEC };
enum class LocalShellCallKind { LOCAL_SHELL_CALL };
enum class LocalShellCallOutputKind { LOCAL_SHELL_CALL_OUTPUT };
enum class LocalShellToolKind { LOCAL_SHELL };
enum class McpApprovalRequestKind { MCP_APPROVAL_REQUEST };
enum class McpApprovalResponseKind { MCP_APPROVAL_RESPONSE };
enum class McpCallKind { MCP_CALL };
enum class McpListToolsKind { MCP_LIST_TOOLS };
enum class McpToolChoiceKind { MCP };
enum class McpToolKind { MCP };
enum class MoveActionKind { MOVE };
enum class OpenPageActionKind { OPEN_PAGE };
enum class OutputMessageKind { MESSAGE };
enum class OutputTextKind { OUTPUT_TEXT };
enum class ReasoningItemKind { REASONING };
enum class ReasoningSummaryTextKind { SUMMARY_TEXT };
enum class ReasoningTextKind { REASONING_TEXT };
enum class RefusalKind { REFUSAL };
enum class ScreenshotActionKind { SCREENSHOT };
enum class ScrollActionKind { SCROLL };
enum class SearchActionKind { SEARCH };
enum class ShellCallKind { SHELL_CALL };
enum class ShellCallOutputKind { SHELL_CALL_OUTPUT };
enum class ShellExitOutcomeKind { EXIT };
enum class ShellTimeoutOutcomeKind { TIMEOUT };
enum class ResponseKind { RESPONSE };
enum class ShellToolKind { SHELL };
enum class SpecificApplyPatchToolChoiceKind { APPLY_PATCH };
enum class SpecificShellToolChoiceKind { SHELL };
enum class TypeActionKind { TYPE };
enum class UpdateFileOperationKind { UPDATE_FILE };
enum class UrlCitationKind { URL_CITATION };
enum class WaitActionKind { WAIT };
enum class ResponseFormatTextKind { TEXT };
enum class ResponseFormatJsonSchemaKind { JSON_SCHEMA };
enum class WebSearchCallKind { WEB_SEARCH_CALL };
enum class WebSearchPreviewToolKind { WEB_SEARCH_PREVIEW, WEB_SEARCH_PREVIEW_2025_03_11 };
enum class WebSearchToolKind { WEB_SEARCH, WEB_SEARCH_2025_08_26 };


/***
 * Vocabulary
 */
enum class ApplyPatchCallOutputStatus { COMPLETED, FAILED };
enum class ApplyPatchCallStatus { IN_PROGRESS, COMPLETED };
enum class CallStatus { IN_PROGRESS, COMPLETED, INCOMPLETE, CALLING, FAILED };
enum class CodeInterpreterStatus { IN_PROGRESS, COMPLETED, INCOMPLETE, INTERPRETING, FAILED };
enum class ComputerCallOutputType { COMPUTER_SCREENSHOT };
enum class Detail { HIGH, LOW, AUTO };
enum class FileSearchStatus { IN_PROGRESS, SEARCHING, INCOMPLETE, FAILED };
enum class FilterCompoundType { AND, OR };
enum class FilterOperator { EQ, NE, GT, GTE, LT, LTE, IN, NIN };
enum class FunctionCallStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class GrammarSyntax { LARK, REGEX };
enum class HostedToolMode { FILE_SEARCH, WEB_SEARCH_PREVIEW, COMPUTER_USE_PREVIEW, CODE_INTERPRETER, IMAGE_GENERATION };
enum class ImageGenerationBackground { TRANSPARENT, OPAQUE, AUTO };
enum class ImageGenerationFidelity { HIGH, LOW };
enum class ImageGenerationFormat { PNG, WEBP, JPEG };
enum class ImageGenerationQuality { LOW, MEDIUM, HIGH, AUTO };
enum class ImageGenerationSize { SIZE_1024_1024, SIZE_1024_1536, SIZE_1536_1024, AUTO };
enum class IncompleteReason { MAX_OUTPUT_TOKENS, CONTENT_FILTER };
enum class ItemStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class LocalShellActionType { EXEC };
enum class LocationType { APPROXIMATE };
enum class McpApprovalSetting { ALWAYS, NEVER };
enum class MouseButton { LEFT, RIGHT, WHEEL, BACK, FORWARD };
enum class PendingSafetyCheckStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class PromptCacheRetention { HOURS_24 };
enum class ReasoningEffort { NONE, MINIMAL, LOW, MEDIUM, HIGH, XHIGH };
enum class ReasoningItemContentType { SUMMARY_TEXT, REASONING_TEXT };
enum class ReasoningStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class ReasoningSummary { AUTO, CONCISE, DETAILED };
enum class ResponseFormatType { TEXT, JSON_SCHEMA };
enum class ResponseStatus { COMPLETED, FAILED, IN_PROGRESS, CANCELLED, QUEUED, INCOMPLETE };
enum class Role { USER, ASSISTANT, SYSTEM, DEVELOPER };
enum class SearchContextSize { LOW, MEDIUM, HIGH };
enum class ServiceTier { AUTO, DEFAULT, FLEX, SCALE, PRIORITY };
enum class ToolChoiceMode { NONE, AUTO, REQUIRED };
enum class TruncationStrategy { AUTO, DISABLED };
enum class Verbosity { LOW, MEDIUM, HIGH };
enum class WebSearchPreviewToolType { WEB_SEARCH_PREVIEW, WEB_SEARCH_PREVIEW_2025_03_11 };
enum class WebSearchStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class WebSearchToolType { WEB_SEARCH, WEB_SEARCH_2025_08_26 };


/***
 * Shared Substructures
 */

struct ResponseFormatText {
    ResponseFormatTextKind type = ResponseFormatTextKind::TEXT;
};

struct ResponseFormatJsonSchema {
    std::string name;
    jai::llm::json::Object schema;
    ResponseFormatJsonSchemaKind type = ResponseFormatJsonSchemaKind::JSON_SCHEMA;
    std::optional<std::string> description{};
    std::optional<bool> strict{};
};

using ResponseFormat = std::variant<ResponseFormatText, ResponseFormatJsonSchema>;

struct TextConfig {
    std::optional<ResponseFormat> format{};
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
    InputTextKind type = InputTextKind::INPUT_TEXT;
};

struct InputImage {
    Detail detail; // Required defaults to auto, but marked required in doc
    InputImageKind type = InputImageKind::INPUT_IMAGE;
    std::optional<std::string> file_id{};
    std::optional<std::string> image_url{};
};

struct InputFile {
    InputFileKind type = InputFileKind::INPUT_FILE;
    std::optional<std::string> file_data{};
    std::optional<std::string> file_id{};
    std::optional<std::string> file_url{};
    std::optional<std::string> filename{};
};

struct ItemReference {
    std::string id;
    ItemReferenceKind type = ItemReferenceKind::ITEM_REFERENCE;
};

struct ConversationRef {
    std::string id;
};

struct PromptRef {
    using VariableValue = std::variant<std::string, InputImage, InputFile, ItemReference>;
    std::string id;
    std::optional<std::map<std::string, VariableValue>> variables{};
    std::optional<std::string> version{};
};


/***
 * Tool Call Actions and Operations (Block 2)
 */

// Computer Tool Call Actions
struct ClickAction {
    MouseButton button;
    ClickActionKind type = ClickActionKind::CLICK;
    int64_t x;
    int64_t y;
};

struct DoubleClickAction {
    DoubleClickActionKind type = DoubleClickActionKind::DOUBLE_CLICK;
    int64_t x;
    int64_t y;
};

struct DragAction {
    struct Coordinate {
        int64_t x;
        int64_t y;
    };
    std::vector<Coordinate> path;
    DragActionKind type = DragActionKind::DRAG;
};

struct KeyPressAction {
    std::vector<std::string> keys;
    KeyPressActionKind type = KeyPressActionKind::KEYPRESS;
};

struct MoveAction {
    MoveActionKind type = MoveActionKind::MOVE;
    int64_t x;
    int64_t y;
};

struct ScreenshotAction {
    ScreenshotActionKind type = ScreenshotActionKind::SCREENSHOT;
};

struct ScrollAction {
    int64_t scroll_x;
    int64_t scroll_y;
    ScrollActionKind type = ScrollActionKind::SCROLL;
    int64_t x;
    int64_t y;
};

struct TypeAction {
    std::string text;
    TypeActionKind type = TypeActionKind::TYPE;
};

struct WaitAction {
    WaitActionKind type = WaitActionKind::WAIT;
};

using ComputerAction = std::variant<
    ClickAction, DoubleClickAction, DragAction, KeyPressAction,
    MoveAction, ScreenshotAction, ScrollAction, TypeAction, WaitAction
>;

// Web Search Tool Call Actions
struct SearchAction {
    SearchActionKind type = SearchActionKind::SEARCH;
    std::optional<std::vector<std::string>> queries{};
    struct Source {
        std::string type = "url";
        std::string url;
    };
    std::optional<std::vector<Source>> sources{};
};

struct OpenPageAction {
    OpenPageActionKind type = OpenPageActionKind::OPEN_PAGE;
    std::string url;
};

struct FindAction {
    std::string pattern;
    FindActionKind type = FindActionKind::FIND;
    std::string url;
};

using WebSearchAction = std::variant<SearchAction, OpenPageAction, FindAction>;

// Apply Patch Operations
struct CreateFileOperation {
    std::string diff;
    std::string path;
    CreateFileOperationKind type = CreateFileOperationKind::CREATE_FILE;
};

struct DeleteFileOperation {
    std::string path;
    DeleteFileOperationKind type = DeleteFileOperationKind::DELETE_FILE;
};

struct UpdateFileOperation {
    std::string diff;
    std::string path;
    UpdateFileOperationKind type = UpdateFileOperationKind::UPDATE_FILE;
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
    FileSearchCallKind type = FileSearchCallKind::FILE_SEARCH_CALL;
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
    ComputerCallKind type = ComputerCallKind::COMPUTER_CALL;
};

struct ComputerScreenshot {
    ComputerScreenshotKind type = ComputerScreenshotKind::COMPUTER_SCREENSHOT;
    std::optional<std::string> file_id{};
    std::optional<std::string> image_url{};
};

struct ComputerCallOutput {
    std::string call_id;
    ComputerScreenshot output;
    ComputerCallOutputKind type = ComputerCallOutputKind::COMPUTER_CALL_OUTPUT;
    std::optional<std::vector<std::string>> acknowledged_safety_checks{};
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct WebSearchCall {
    WebSearchAction action;
    std::string id;
    ItemStatus status; // Doc says "status" but doesn't list enum. Assuming ItemStatus.
    WebSearchCallKind type = WebSearchCallKind::WEB_SEARCH_CALL;
};

struct FunctionCall {
    std::string arguments;
    std::string call_id;
    std::string name;
    FunctionCallKind type = FunctionCallKind::FUNCTION_CALL;
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct FunctionCallOutput {
    std::string call_id;
    std::variant<std::string, std::vector<std::variant<InputText, InputImage, InputFile>>> output;
    FunctionCallOutputKind type = FunctionCallOutputKind::FUNCTION_CALL_OUTPUT;
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct ReasoningItem {
    struct Summary {
        std::string text;
        ReasoningSummaryTextKind type = ReasoningSummaryTextKind::SUMMARY_TEXT;
    };
    struct Content {
        std::string text;
        ReasoningTextKind type = ReasoningTextKind::REASONING_TEXT;
    };
    std::string id;
    std::optional<std::vector<Summary>> summary{};
    ReasoningItemKind type = ReasoningItemKind::REASONING;
    std::optional<std::vector<Content>> content{};
    std::optional<std::string> encrypted_content{};
    std::optional<ItemStatus> status{};
};

struct CompactionItem {
    std::string encrypted_content;
    CompactionItemKind type = CompactionItemKind::COMPACTION;
    std::optional<std::string> created_by{};
    std::optional<std::string> id{};
};

struct ImageGenerationCall {
    std::string id;
    std::string result;
    std::string status;
    ImageGenerationCallKind type = ImageGenerationCallKind::IMAGE_GENERATION_CALL;
};

struct CodeInterpreterCall {
    struct OutputLog {
        std::string logs;
        CodeInterpreterOutputType type = CodeInterpreterOutputType::LOGS;
    };
    struct OutputImage {
        CodeInterpreterOutputType type = CodeInterpreterOutputType::IMAGE;
        std::string url;
    };
    using Output = std::variant<OutputLog, OutputImage>;

    std::optional<std::string> code{}; // Required if available
    std::string container_id;
    std::string id;
    std::optional<std::vector<Output>> outputs{}; // Required if available
    CodeInterpreterStatus status;
    CodeInterpreterCallKind type = CodeInterpreterCallKind::CODE_INTERPRETER_CALL;
};

struct LocalShellCall {
    struct Action {
        std::vector<std::string> command;
        std::optional<std::map<std::string, std::string>> env{};
        LocalShellActionKind type = LocalShellActionKind::EXEC;
        std::optional<int64_t> timeout_ms{};
        std::optional<std::string> user{};
        std::optional<std::string> working_directory{};
    };
    Action action;
    std::string call_id;
    std::string id;
    ItemStatus status;
    LocalShellCallKind type = LocalShellCallKind::LOCAL_SHELL_CALL;
};

struct LocalShellCallOutput {
    struct ActionOutcome {
        int64_t exit_code;
        LocalShellActionKind type = LocalShellActionKind::EXEC;
        std::optional<std::string> std_err{};
        std::optional<std::string> std_out{};
    };
    ActionOutcome action;
    std::string call_id;
    int64_t max_output_length;
    LocalShellCallOutputKind type = LocalShellCallOutputKind::LOCAL_SHELL_CALL_OUTPUT;
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct ShellCall {
    struct Action {
        std::vector<std::string> commands;
        std::optional<int64_t> max_output_length{};
        std::optional<int64_t> timeout_ms{};
    };
    Action action;
    std::string call_id;
    std::string id;
    ItemStatus status;
    ShellCallKind type = ShellCallKind::SHELL_CALL;
    std::optional<std::string> created_by{};
};

struct ShellCallOutput {
    struct ShellExitOutcome {
        int64_t exit_code;
        ShellExitOutcomeKind type = ShellExitOutcomeKind::EXIT;
        std::optional<std::string> std_err{};
        std::optional<std::string> std_out{};
    };
    struct ShellTimeoutOutcome {
        ShellTimeoutOutcomeKind type = ShellTimeoutOutcomeKind::TIMEOUT;
    };
    using Outcome = std::variant<ShellExitOutcome, ShellTimeoutOutcome>;
    struct Content {
        Outcome outcome;
    };
    std::string call_id;
    int64_t max_output_length;
    std::vector<Content> output;
    ShellCallOutputKind type = ShellCallOutputKind::SHELL_CALL_OUTPUT;
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
    std::optional<std::string> created_by{};
};

struct ApplyPatchCall {
    ApplyPatchOperation operation;
    ApplyPatchCallStatus status;
    ApplyPatchCallKind type = ApplyPatchCallKind::APPLY_PATCH_CALL;
    std::optional<std::string> call_id{};
    std::optional<std::string> created_by{};
    std::optional<std::string> id{};
};

struct ApplyPatchCallOutput {
    ApplyPatchCallOutputStatus status;
    ApplyPatchCallOutputKind type = ApplyPatchCallOutputKind::APPLY_PATCH_CALL_OUTPUT;
    std::optional<std::string> call_id{};
    std::optional<std::string> created_by{};
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
    McpListToolsKind type = McpListToolsKind::MCP_LIST_TOOLS;
    std::optional<std::string> error{};
};

struct McpApprovalRequest {
    std::string arguments;
    std::string id;
    std::string name;
    std::string server_label;
    McpApprovalRequestKind type = McpApprovalRequestKind::MCP_APPROVAL_REQUEST;
};

struct McpApprovalResponse {
    std::string approval_request_id;
    bool approve;
    McpApprovalResponseKind type = McpApprovalResponseKind::MCP_APPROVAL_RESPONSE;
    std::optional<std::string> id{};
    std::optional<std::string> reason{};
};

struct McpCall {
    std::string arguments;
    std::string id;
    std::string name;
    std::string server_label;
    McpCallKind type = McpCallKind::MCP_CALL;
    std::optional<std::string> approval_request_id{};
    std::optional<std::string> error{};
    std::optional<std::string> output{};
    std::optional<CallStatus> status{};
};

struct CustomToolCall {
    std::string input;
    std::string name;
    CustomToolCallKind type = CustomToolCallKind::CUSTOM_TOOL_CALL;
    std::optional<std::string> call_id{};
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct CustomToolCallOutput {
    std::string output;
    CustomToolCallOutputKind type = CustomToolCallOutputKind::CUSTOM_TOOL_CALL_OUTPUT;
    std::optional<std::string> call_id{};
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};


/***
 * Message and InputItem variants (Block 4)
 */

struct InputMessage {
    std::vector<std::variant<InputText, InputImage, InputFile, ItemReference>> content;
    Role role;
    InputMessageKind type = InputMessageKind::MESSAGE;
    std::optional<std::string> id{};
    std::optional<ItemStatus> status{};
};

struct OutputMessage {
    struct OutputText {
        struct CitationContainer {
            std::string container_id;
            int64_t end_index;
            std::string file_id;
            std::string filename;
            int64_t start_index;
            ContainerFileCitationKind type = ContainerFileCitationKind::CONTAINER_FILE_CITATION;
        };
        struct CitationFile {
            std::string file_id;
            std::string filename;
            int64_t index;
            FileCitationKind type = FileCitationKind::FILE_CITATION;
        };
        struct CitationUrl {
            int64_t end_index;
            int64_t start_index;
            std::string title;
            UrlCitationKind type = UrlCitationKind::URL_CITATION;
            std::string url;
        };
        struct FilePath {
            std::string file_id;
            int64_t index;
            FilePathKind type = FilePathKind::FILE_PATH;
        };
        using Annotation = std::variant<CitationFile, CitationUrl, CitationContainer, FilePath>;

        struct Logprob {
            struct TopLogprob {
                std::vector<std::byte> bytes;
                double logprob;
                std::string token;
            };
            std::vector<std::byte> bytes;
            double logprob;
            std::string token;
            std::vector<TopLogprob> top_logprobs;
        };

        std::vector<Annotation> annotations;
        std::string value;
        OutputTextKind type = OutputTextKind::OUTPUT_TEXT;
        std::optional<std::vector<Logprob>> logprobs{};
    };

    struct Refusal {
        std::string refusal;
        RefusalKind type = RefusalKind::REFUSAL;
    };

    using Content = std::variant<OutputText, Refusal>;

    std::vector<Content> content;
    std::string id;
    Role role = Role::ASSISTANT;
    ItemStatus status;
    OutputMessageKind type = OutputMessageKind::MESSAGE;
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
    struct Parameters {
        jai::llm::json::Object parameters;
        bool strict = true;
    };
    std::string description;
    std::string name;
    Parameters parameters;
    FunctionToolKind type = FunctionToolKind::FUNCTION;
};

struct FileSearchTool {
    struct RankingOptions {
        struct HybridSearch {
            double embedding_weight;
            double text_weight;
        };
        std::optional<HybridSearch> hybrid_search{};
        std::optional<std::string> ranker{};
        std::optional<double> score_threshold{};
    };

    FileSearchToolKind type = FileSearchToolKind::FILE_SEARCH;
    std::vector<std::string> vector_store_ids;
    std::optional<int64_t> max_num_results{};
    std::optional<RankingOptions> ranking_options{};
};

struct ComputerUseTool {
    int64_t display_height;
    int64_t display_width;
    std::string environment;
    ComputerUseToolKind type = ComputerUseToolKind::COMPUTER_USE_PREVIEW;
};

struct WebSearchTool {
    WebSearchToolKind type = WebSearchToolKind::WEB_SEARCH;
    struct Filters {
        std::optional<std::vector<std::string>> allowed_domains{};
        std::optional<SearchContextSize> search_context_size{};
        struct Location {
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
            LocationType type = LocationType::APPROXIMATE;
        };
        std::optional<Location> user_location{};
    };
    std::optional<Filters> filters{};
};

struct McpTool {
    struct Filter {
        bool read_only;
        std::vector<std::string> tool_names;
    };
    using AllowedTools = std::variant<std::vector<std::string>, Filter>;
    using ApprovalFilter = std::variant<std::string, Filter>; // "always", "never" or filter

    std::string server_label;
    McpToolKind type = McpToolKind::MCP;
    std::optional<AllowedTools> allowed_tools{};
    std::optional<std::string> authorization{};
    std::optional<std::string> connector_id{};
    std::optional<std::map<std::string, std::string>> headers{};
    std::optional<ApprovalFilter> require_approval{};
    std::optional<std::string> server_description{};
    std::optional<std::string> server_url{};
};

struct CodeInterpreterTool {
    struct ContainerConfig {
        std::vector<std::string> file_ids;
        std::string type = "auto";
        std::optional<std::string> memory_limit{};
    };
    using Container = std::variant<std::string, ContainerConfig>;

    Container container;
    CodeInterpreterToolKind type = CodeInterpreterToolKind::CODE_INTERPRETER;
};

struct ImageGenerationTool {
    ImageGenerationToolKind type = ImageGenerationToolKind::IMAGE_GENERATION;
    std::optional<ImageGenerationBackground> background{};
    std::optional<ImageGenerationFidelity> input_fidelity{};
    struct Mask {
        std::string file_id;
        std::string image_url;
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
    LocalShellToolKind type = LocalShellToolKind::LOCAL_SHELL;
};

struct ShellTool {
    ShellToolKind type = ShellToolKind::SHELL;
};

struct CustomTool {
    struct GrammarFormat {
        GrammarSyntax syntax;
        CustomToolGrammarFormatKind type = CustomToolGrammarFormatKind::GRAMMAR;
        std::string value;
    };
    struct TextFormat {
        CustomToolTextFormatKind type = CustomToolTextFormatKind::TEXT;
    };
    using Format = std::variant<TextFormat, GrammarFormat>;
    std::string description;
    Format format;
    std::string name;
    CustomToolKind type = CustomToolKind::CUSTOM;
};

struct WebSearchPreviewTool {
    WebSearchPreviewToolKind type = WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW;
    std::optional<SearchContextSize> search_context_size{};
    struct Location {
        LocationType type = LocationType::APPROXIMATE;
        std::optional<std::string> city{};
        std::optional<std::string> country{};
        std::optional<std::string> region{};
        std::optional<std::string> timezone{};
    };
    std::optional<Location> user_location{};
};

struct ApplyPatchTool {
    ApplyPatchToolKind type = ApplyPatchToolKind::APPLY_PATCH;
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
    struct RestrictedTool {
        std::string type;
        std::optional<std::string> name{};
        std::optional<std::string> server_label{};
    };
    ToolChoiceMode mode;
    std::vector<RestrictedTool> tools;
    AllowedToolsChoiceKind type = AllowedToolsChoiceKind::ALLOWED_TOOLS;
};

struct HostedToolChoice {
    HostedToolMode type;
};

struct FunctionToolChoice {
    std::string name;
    FunctionToolChoiceKind type = FunctionToolChoiceKind::FUNCTION;
};

struct McpToolChoice {
    std::string name;
    std::string server_label;
    McpToolChoiceKind type = McpToolChoiceKind::MCP;
};

struct CustomToolChoice {
    std::string name;
    CustomToolChoiceKind type = CustomToolChoiceKind::CUSTOM;
};

struct SpecificApplyPatchToolChoice {
    SpecificApplyPatchToolChoiceKind type = SpecificApplyPatchToolChoiceKind::APPLY_PATCH;
};

struct SpecificShellToolChoice {
    SpecificShellToolChoiceKind type = SpecificShellToolChoiceKind::SHELL;
};

using ToolChoice = std::variant<
    ToolChoiceMode,
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
    std::optional<std::map<std::string, std::string>> metadata{};
    std::optional<std::string> model{};
    std::optional<bool> parallel_tool_calls{};
    std::optional<std::string> previous_response_id{};
    std::optional<PromptRef> prompt{};
    std::optional<ReasoningConfig> reasoning{};
    std::optional<ServiceTier> service_tier{};
    std::optional<bool> store{};
    std::optional<double> temperature{};
    std::optional<TextConfig> text{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<std::vector<Tool>> tools{};
    std::optional<double> top_p{};
};


/***
 * Response Substructures
 */

struct ResponseError {
    std::string code;
    std::string message;
};

struct IncompleteDetails {
    IncompleteReason reason;
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
    ResponseKind object = ResponseKind::RESPONSE;
    std::optional<bool> background{};
    std::optional<int64_t> completed_at{};
    std::optional<ConversationRef> conversation{};
    std::optional<int64_t> created_at{};
    std::optional<ResponseError> error{};
    std::optional<IncompleteDetails> incomplete_details{};
    std::optional<std::variant<std::string, std::vector<InputItem>>> instructions{}; // echo; for tracing only
    std::optional<int64_t> max_tokens{};
    std::optional<std::map<std::string, std::string>> metadata{};
    std::optional<std::string> model{};
    std::optional<std::string> previous_response_id{};
    std::optional<ReasoningConfig> reasoning{};
    std::optional<ServiceTier> service_tier{};
    std::optional<ItemStatus> status{};
    std::optional<double> temperature{};
    std::optional<TextConfig> text{};
    std::optional<ToolChoice> tool_choice{};
    std::optional<std::vector<Tool>> tools{};
    std::optional<double> top_p{};
    std::optional<ResponseUsage> usage{};
};


}
