/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once


#include "../../core/types.hpp"


namespace jai::llm::openai {


/***
 * Kinds
 */
struct KindAllowedToolsChoice           : Kind { static constexpr std::string_view value = "allowed_tools"; };
struct KindApplyPatchCall               : Kind { static constexpr std::string_view value = "apply_patch_call"; };
struct KindApplyPatchCallOutput         : Kind { static constexpr std::string_view value = "apply_patch_call_output"; };
struct KindApplyPatchTool               : Kind { static constexpr std::string_view value = "apply_patch"; };
struct KindClickAction                  : Kind { static constexpr std::string_view value = "click"; };
struct KindCodeInterpreterCall          : Kind { static constexpr std::string_view value = "code_interpreter_call"; };
struct KindCodeInterpreterTool          : Kind { static constexpr std::string_view value = "code_interpreter"; };
struct KindCompactionItem               : Kind { static constexpr std::string_view value = "compaction"; };
struct KindComputerCall                 : Kind { static constexpr std::string_view value = "computer_call"; };
struct KindComputerCallOutput           : Kind { static constexpr std::string_view value = "computer_call_output"; };
struct KindComputerScreenshot           : Kind { static constexpr std::string_view value = "computer_screenshot"; };
struct KindComputerUseTool              : Kind { static constexpr std::string_view value = "computer_use_preview"; };
struct KindContainerFileCitation        : Kind { static constexpr std::string_view value = "container_file_citation"; };
struct KindContainerConfig              : Kind { static constexpr std::string_view value = "auto"; };
struct KindCreateFileOperation          : Kind { static constexpr std::string_view value = "create_file"; };
struct KindCustomToolCall               : Kind { static constexpr std::string_view value = "custom_tool_call"; };
struct KindCustomToolCallOutput         : Kind { static constexpr std::string_view value = "custom_tool_call_output"; };
struct KindCustomToolChoice             : Kind { static constexpr std::string_view value = "custom"; };
struct KindCustomToolGrammarFormat      : Kind { static constexpr std::string_view value = "grammar"; };
struct KindCustomTool                   : Kind { static constexpr std::string_view value = "custom"; };
struct KindCustomToolTextFormat         : Kind { static constexpr std::string_view value = "text"; };
struct KindDeleteFileOperation          : Kind { static constexpr std::string_view value = "delete_file"; };
struct KindDoubleClickAction            : Kind { static constexpr std::string_view value = "double_click"; };
struct KindDragAction                   : Kind { static constexpr std::string_view value = "drag"; };
struct KindFileCitation                 : Kind { static constexpr std::string_view value = "file_citation"; };
struct KindFilePath                     : Kind { static constexpr std::string_view value = "file_path"; };
struct KindFileSearchToolCall           : Kind { static constexpr std::string_view value = "file_search_call"; };
struct KindFileSearchTool               : Kind { static constexpr std::string_view value = "file_search"; };
struct KindFindAction                   : Kind { static constexpr std::string_view value = "find"; };
struct KindFormatText                   : Kind { static constexpr std::string_view value = "text"; };
struct KindFormatJsonSchema             : Kind { static constexpr std::string_view value = "json_schema"; };
struct KindFunctionCall                 : Kind { static constexpr std::string_view value = "function_call"; };
struct KindFunctionCallOutput           : Kind { static constexpr std::string_view value = "function_call_output"; };
struct KindFunctionToolChoice           : Kind { static constexpr std::string_view value = "function"; };
struct KindFunctionTool                 : Kind { static constexpr std::string_view value = "function"; };
struct KindImageGenerationCall          : Kind { static constexpr std::string_view value = "image_generation_call"; };
struct KindImageGenerationTool          : Kind { static constexpr std::string_view value = "image_generation"; };
struct KindInputFile                    : Kind { static constexpr std::string_view value = "input_file"; };
struct KindInputImage                   : Kind { static constexpr std::string_view value = "input_image"; };
struct KindInputMessage                 : Kind { static constexpr std::string_view value = "message"; };
struct KindInputText                    : Kind { static constexpr std::string_view value = "input_text"; };
struct KindItemReference                : Kind { static constexpr std::string_view value = "item_reference"; };
struct KindKeyPressAction               : Kind { static constexpr std::string_view value = "keypress"; };
struct KindLocalShellAction             : Kind { static constexpr std::string_view value = "exec"; };
struct KindLocalShellCall               : Kind { static constexpr std::string_view value = "local_shell_call"; };
struct KindLocalShellCallOutput         : Kind { static constexpr std::string_view value = "local_shell_call_output"; };
struct KindLocalShellTool               : Kind { static constexpr std::string_view value = "local_shell"; };
struct KindMCPApprovalRequest           : Kind { static constexpr std::string_view value = "mcp_approval_request"; };
struct KindMCPApprovalResponse          : Kind { static constexpr std::string_view value = "mcp_approval_response"; };
struct KindMCPCall                      : Kind { static constexpr std::string_view value = "mcp_call"; };
struct KindMCPListTools                 : Kind { static constexpr std::string_view value = "mcp_list_tools"; };
struct KindMCPToolChoice                : Kind { static constexpr std::string_view value = "mcp"; };
struct KindMCPTool                      : Kind { static constexpr std::string_view value = "mcp"; };
struct KindMoveAction                   : Kind { static constexpr std::string_view value = "move"; };
struct KindOpenPageAction               : Kind { static constexpr std::string_view value = "open_page"; };
struct KindOutputMessage                : Kind { static constexpr std::string_view value = "message"; };
struct KindOutputText                   : Kind { static constexpr std::string_view value = "output_text"; };
struct KindReasoningItem                : Kind { static constexpr std::string_view value = "reasoning"; };
struct KindReasoningSummaryText         : Kind { static constexpr std::string_view value = "summary_text"; };
struct KindReasoningText                : Kind { static constexpr std::string_view value = "reasoning_text"; };
struct KindRefusal                      : Kind { static constexpr std::string_view value = "refusal"; };
struct KindResponse                     : Kind { static constexpr std::string_view value = "response"; };
struct KindScreenshotAction             : Kind { static constexpr std::string_view value = "screenshot"; };
struct KindScrollAction                 : Kind { static constexpr std::string_view value = "scroll"; };
struct KindSearchAction                 : Kind { static constexpr std::string_view value = "search"; };
struct KindSearchActionSource           : Kind { static constexpr std::string_view value = "url"; };
struct KindShellCall                    : Kind { static constexpr std::string_view value = "shell_call"; };
struct KindShellCallOutput              : Kind { static constexpr std::string_view value = "shell_call_output"; };
struct KindShellExitOutcome             : Kind { static constexpr std::string_view value = "exit"; };
struct KindShellTimeoutOutcome          : Kind { static constexpr std::string_view value = "timeout"; };
struct KindShellTool                    : Kind { static constexpr std::string_view value = "shell"; };
struct KindSpecificApplyPatchToolChoice : Kind { static constexpr std::string_view value = "apply_patch"; };
struct KindSpecificShellToolChoice      : Kind { static constexpr std::string_view value = "shell"; };
struct KindTypeAction                   : Kind { static constexpr std::string_view value = "type"; };
struct KindUpdateFileOperation          : Kind { static constexpr std::string_view value = "update_file"; };
struct KindUrlCitation                  : Kind { static constexpr std::string_view value = "url_citation"; };
struct KindWaitAction                   : Kind { static constexpr std::string_view value = "wait"; };
struct KindWebSearchCall                : Kind { static constexpr std::string_view value = "web_search_call"; };


/***
 * Vocabulary
 */
enum class AnnotationType { FILE_CITATION, URL_CITATION, CONTAINER_FILE_CITATION, FILE_PATH };
enum class ApplyPatchCallOutputStatus { COMPLETED, FAILED };
enum class ApplyPatchCallStatus { IN_PROGRESS, COMPLETED };
enum class ApplyPatchOperationType { CREATE_FILE, DELETE_FILE, UPDATE_FILE };
enum class CallStatus { IN_PROGRESS, COMPLETED, INCOMPLETE, CALLING, FAILED };
enum class CodeInterpreterOutputType { LOGS, IMAGE };
enum class CodeInterpreterStatus { IN_PROGRESS, COMPLETED, INCOMPLETE, INTERPRETING, FAILED };
enum class ComputerActionType { CLICK, DOUBLE_CLICK, DRAG, KEYPRESS, MOVE, SCREENSHOT, SCROLL, TYPE, WAIT };
enum class ComputerCallOutputType { COMPUTER_SCREENSHOT };
enum class ConnectId {
    DROPBOX, GMAIL, GOOGLE_CALENDAR, GOOGLE_DRIVE, MICROSOFT_TEAMS, OUTLOOK_CALENDAR, OUTLOOK_EMAIL, SHAREPOINT
};
enum class ContentType { INPUT_FILE, INPUT_IMAGE, INPUT_TEXT };
enum class CustomToolFormatType { TEXT, GRAMMAR };
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
enum class IncludeOutputData {
    WEB_SEARCH_CALL_ACTION_SOURCES, CODE_INTERPRETER_CALL_OUTPUTS, COMPUTER_CALL_OUTPUT_OUTPUT_IMAGE_URL,
    FILE_SEARCH_CALL_RESULTS, MESSAGE_INPUT_IMAGE_IMAGE_URL, MESSAGE_OUTPUT_TEXT_LOGPROBS, REASONING_ENCRYPTED_CONTENT
};
enum class IncompleteReason { MAX_OUTPUT_TOKENS, CONTENT_FILTER };
enum class InputItemType {
    APPLY_PATCH_CALL, APPLY_PATCH_CALL_OUTPUT, CODE_INTERPRETER_CALL, COMPACTION, COMPUTER_CALL, COMPUTER_CALL_OUTPUT,
    CUSTOM_TOOL_CALL, CUSTOM_TOOL_CALL_OUTPUT, FILE_SEARCH_CALL, FUNCTION_CALL, FUNCTION_CALL_OUTPUT,
    IMAGE_GENERATION_CALL, ITEM_REFERENCE, LOCAL_SHELL_CALL, LOCAL_SHELL_CALL_OUTPUT,
    MCP_APPROVAL_REQUEST, MCP_APPROVAL_RESPONSE, MCP_CALL, MCP_LIST_TOOLS, MESSAGE, REASONING, SHELL_CALL,
    SHELL_CALL_OUTPUT, WEB_SEARCH_CALL
};
enum class ItemStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class LocalShellActionType { EXEC };
enum class LocationType { APPROXIMATE };
enum class MCPApprovalSetting { ALWAYS, NEVER };
enum class MouseButton { LEFT, RIGHT, WHEEL, BACK, FORWARD };
enum class OutputItemType {
    APPLY_PATCH_CALL, APPLY_PATCH_CALL_OUTPUT, CODE_INTERPRETER_CALL, COMPACTION, COMPUTER_CALL, CUSTOM_TOOL_CALL,
    FILE_SEARCH_CALL, FUNCTION_CALL, IMAGE_GENERATION_CALL, ITEM_REFERENCE, LOCAL_SHELL_CALL, MCP_APPROVAL_REQUEST,
    MCP_CALL, MCP_LIST_TOOLS, MESSAGE, REASONING, SHELL_CALL, SHELL_CALL_OUTPUT, WEB_SEARCH_CALL
};
enum class OutputMessageContentType { OUTPUT_TEXT, REFUSAL };
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
enum class ShellCallOutcomeType { EXIT, TIMEOUT };
enum class ToolChoiceMode { NONE, AUTO, REQUIRED };
enum class ToolChoiceModeNotNone { AUTO, REQUIRED };
enum class ToolChoiceType { ALLOWED_TOOLS, FUNCTION, MCP, CUSTOM, APPLY_PATCH, SHELL };
enum class ToolType {
    APPLY_PATCH, CODE_INTERPRETER, COMPUTER_USE_PREVIEW, CUSTOM, FILE_SEARCH, FUNCTION, IMAGE_GENERATION,
    LOCAL_SHELL, MCP, SHELL, WEB_SEARCH, WEB_SEARCH_PREVIEW
};
enum class TruncationStrategy { AUTO, DISABLED };
enum class Verbosity { LOW, MEDIUM, HIGH };
enum class WebSearchPreviewToolKind { WEB_SEARCH_PREVIEW, WEB_SEARCH_PREVIEW_2025_03_11 };
enum class WebSearchPreviewToolType { WEB_SEARCH_PREVIEW, WEB_SEARCH_PREVIEW_2025_03_11 };
enum class WebSearchActionType { SEARCH, OPEN_PAGE, FIND };
enum class WebSearchStatus { IN_PROGRESS, COMPLETED, INCOMPLETE };
enum class WebSearchToolKind { WEB_SEARCH, WEB_SEARCH_2025_08_26 };
enum class WebSearchToolType { WEB_SEARCH, WEB_SEARCH_2025_08_26 };


}
