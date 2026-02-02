/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once


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
    CUSTOM_TOOL_CALL, CUSTOM_TOOL_CALL_OUTPUT, FILE_SEARCH_CALL, FUNCTION_CALL, FUNCTION_CALL_OUTPUT,
    IMAGE_GENERATION_CALL, ITEM_REFERENCE, LOCAL_SHELL_CALL, LOCAL_SHELL_CALL_OUTPUT,
    MCP_APPROVAL_REQUEST, MCP_APPROVAL_RESPONSE, MCP_CALL, MCP_LIST_TOOLS, MESSAGE, REASONING, SHELL_CALL,
    SHELL_CALL_OUTPUT, WEB_SEARCH_CALL
};
enum class OutputItemType {
    APPLY_PATCH_CALL, APPLY_PATCH_CALL_OUTPUT, CODE_INTERPRETER_CALL, COMPACTION, COMPUTER_CALL, CUSTOM_TOOL_CALL,
    FILE_SEARCH_CALL, FUNCTION_CALL, IMAGE_GENERATION_CALL, ITEM_REFERENCE, LOCAL_SHELL_CALL, MCP_APPROVAL_REQUEST,
    MCP_CALL, MCP_LIST_TOOLS, MESSAGE, REASONING, SHELL_CALL, SHELL_CALL_OUTPUT, WEB_SEARCH_CALL
};
enum class ContentType { INPUT_FILE, INPUT_IMAGE, INPUT_TEXT };
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
enum class ContainerConfigKind { AUTO };
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
enum class FileSearchToolCallKind { FILE_SEARCH_CALL };
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
enum class MCPApprovalRequestKind { MCP_APPROVAL_REQUEST };
enum class MCPApprovalResponseKind { MCP_APPROVAL_RESPONSE };
enum class MCPCallKind { MCP_CALL };
enum class MCPListToolsKind { MCP_LIST_TOOLS };
enum class MCPToolChoiceKind { MCP };
enum class MCPToolKind { MCP };
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
enum class SearchActionSourceKind { URL };
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
enum class FormatTextKind { TEXT };
enum class FormatJsonSchemaKind { JSON_SCHEMA };
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
enum class ConnectId {
    DROPBOX, GMAIL, GOOGLE_CALENDAR, GOOGLE_DRIVE, MICROSOFT_TEAMS, OUTLOOK_CALENDAR, OUTLOOK_EMAIL, SHAREPOINT
};
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
enum class IncludeOutputData {
    WEB_SEARCH_CALL_ACTION_SOURCES,
    CODE_INTERPRETER_CALL_OUTPUTS,
    COMPUTER_CALL_OUTPUT_OUTPUT_IMAGE_URL,
    FILE_SEARCH_CALL_RESULTS,
    MESSAGE_INPUT_IMAGE_IMAGE_URL,
    MESSAGE_OUTPUT_TEXT_LOGPROBS,
    REASONING_ENCRYPTED_CONTENT
};
enum class LocalShellActionType { EXEC };
enum class LocationType { APPROXIMATE };
enum class MCPApprovalSetting { ALWAYS, NEVER };
enum class MouseButton { LEFT, RIGHT, WHEEL, BACK, FORWARD };
enum class PendingSafetyCheckStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class PromptCacheRetention { HOURS_24 };
enum class ReasoningEffort { NONE, MINIMAL, LOW, MEDIUM, HIGH, XHIGH };
enum class ReasoningItemContentType { SUMMARY_TEXT, REASONING_TEXT };
enum class ReasoningStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class ReasoningSummary { AUTO, CONCISE, DETAILED };
enum class ResponseFormatType { TEXT, JSON_SCHEMA };
enum class ResponseStatus { COMPLETED, FAILED, IN_PROGRESS, CANCELLED, QUEUED, INCOMPLETE };
enum class RoleAssistant { ASSISTANT };
enum class RoleInputMessage { USER, ASSISTANT, SYSTEM, DEVELOPER };
enum class RoleUser { USER, SYSTEM, DEVELOPER };
enum class SearchContextSize { LOW, MEDIUM, HIGH };
enum class ServiceTier { AUTO, DEFAULT, FLEX, PRIORITY };
enum class ToolChoiceMode { NONE, AUTO, REQUIRED };
enum class ToolChoiceModeNotNone { AUTO, REQUIRED };
enum class TruncationStrategy { AUTO, DISABLED };
enum class Verbosity { LOW, MEDIUM, HIGH };
enum class WebSearchPreviewToolType { WEB_SEARCH_PREVIEW, WEB_SEARCH_PREVIEW_2025_03_11 };
enum class WebSearchStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class WebSearchToolType { WEB_SEARCH, WEB_SEARCH_2025_08_26 };


}
