#pragma once

#include "../openai.hpp"

#include <optional>
#include <stdexcept>
#include <string_view>


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


/***
 * from_string_view Specializations
 */

template <>
constexpr std::optional<openai::AllowedToolsChoiceKind> from_string_view<openai::AllowedToolsChoiceKind>(std::string_view sv) {
    if (sv == "allowed_tools") return openai::AllowedToolsChoiceKind::ALLOWED_TOOLS;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::AnnotationType> from_string_view<openai::AnnotationType>(std::string_view sv) {
    if (sv == "file_citation") return openai::AnnotationType::FILE_CITATION;
    if (sv == "url_citation") return openai::AnnotationType::URL_CITATION;
    if (sv == "container_file_citation") return openai::AnnotationType::CONTAINER_FILE_CITATION;
    if (sv == "file_path") return openai::AnnotationType::FILE_PATH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchCallKind> from_string_view<openai::ApplyPatchCallKind>(std::string_view sv) {
    if (sv == "apply_patch_call") return openai::ApplyPatchCallKind::APPLY_PATCH_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchCallOutputKind> from_string_view<openai::ApplyPatchCallOutputKind>(std::string_view sv) {
    if (sv == "apply_patch_call_output") return openai::ApplyPatchCallOutputKind::APPLY_PATCH_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchCallOutputStatus> from_string_view<openai::ApplyPatchCallOutputStatus>(std::string_view sv) {
    if (sv == "completed") return openai::ApplyPatchCallOutputStatus::COMPLETED;
    if (sv == "failed") return openai::ApplyPatchCallOutputStatus::FAILED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchCallStatus> from_string_view<openai::ApplyPatchCallStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::ApplyPatchCallStatus::IN_PROGRESS;
    if (sv == "completed") return openai::ApplyPatchCallStatus::COMPLETED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchOperationType> from_string_view<openai::ApplyPatchOperationType>(std::string_view sv) {
    if (sv == "create_file") return openai::ApplyPatchOperationType::CREATE_FILE;
    if (sv == "delete_file") return openai::ApplyPatchOperationType::DELETE_FILE;
    if (sv == "update_file") return openai::ApplyPatchOperationType::UPDATE_FILE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ApplyPatchToolKind> from_string_view<openai::ApplyPatchToolKind>(std::string_view sv) {
    if (sv == "apply_patch") return openai::ApplyPatchToolKind::APPLY_PATCH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CallStatus> from_string_view<openai::CallStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::CallStatus::IN_PROGRESS;
    if (sv == "completed") return openai::CallStatus::COMPLETED;
    if (sv == "incomplete") return openai::CallStatus::INCOMPLETE;
    if (sv == "calling") return openai::CallStatus::CALLING;
    if (sv == "failed") return openai::CallStatus::FAILED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ClickActionKind> from_string_view<openai::ClickActionKind>(std::string_view sv) {
    if (sv == "click") return openai::ClickActionKind::CLICK;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterCallKind> from_string_view<openai::CodeInterpreterCallKind>(std::string_view sv) {
    if (sv == "code_interpreter_call") return openai::CodeInterpreterCallKind::CODE_INTERPRETER_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterImageKind> from_string_view<openai::CodeInterpreterImageKind>(std::string_view sv) {
    if (sv == "image") return openai::CodeInterpreterImageKind::IMAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterLogKind> from_string_view<openai::CodeInterpreterLogKind>(std::string_view sv) {
    if (sv == "logs") return openai::CodeInterpreterLogKind::LOGS;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterOutputType> from_string_view<openai::CodeInterpreterOutputType>(std::string_view sv) {
    if (sv == "logs") return openai::CodeInterpreterOutputType::LOGS;
    if (sv == "image") return openai::CodeInterpreterOutputType::IMAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterStatus> from_string_view<openai::CodeInterpreterStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::CodeInterpreterStatus::IN_PROGRESS;
    if (sv == "completed") return openai::CodeInterpreterStatus::COMPLETED;
    if (sv == "incomplete") return openai::CodeInterpreterStatus::INCOMPLETE;
    if (sv == "interpreting") return openai::CodeInterpreterStatus::INTERPRETING;
    if (sv == "failed") return openai::CodeInterpreterStatus::FAILED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterToolKind> from_string_view<openai::CodeInterpreterToolKind>(std::string_view sv) {
    if (sv == "code_interpreter") return openai::CodeInterpreterToolKind::CODE_INTERPRETER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CompactionItemKind> from_string_view<openai::CompactionItemKind>(std::string_view sv) {
    if (sv == "compaction") return openai::CompactionItemKind::COMPACTION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerActionType> from_string_view<openai::ComputerActionType>(std::string_view sv) {
    if (sv == "click") return openai::ComputerActionType::CLICK;
    if (sv == "double_click") return openai::ComputerActionType::DOUBLE_CLICK;
    if (sv == "drag") return openai::ComputerActionType::DRAG;
    if (sv == "keypress") return openai::ComputerActionType::KEYPRESS;
    if (sv == "move") return openai::ComputerActionType::MOVE;
    if (sv == "screenshot") return openai::ComputerActionType::SCREENSHOT;
    if (sv == "scroll") return openai::ComputerActionType::SCROLL;
    if (sv == "type") return openai::ComputerActionType::TYPE;
    if (sv == "wait") return openai::ComputerActionType::WAIT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerCallKind> from_string_view<openai::ComputerCallKind>(std::string_view sv) {
    if (sv == "computer_call") return openai::ComputerCallKind::COMPUTER_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerCallOutputKind> from_string_view<openai::ComputerCallOutputKind>(std::string_view sv) {
    if (sv == "computer_call_output") return openai::ComputerCallOutputKind::COMPUTER_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerCallOutputType> from_string_view<openai::ComputerCallOutputType>(std::string_view sv) {
    if (sv == "computer_screenshot") return openai::ComputerCallOutputType::COMPUTER_SCREENSHOT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerScreenshotKind> from_string_view<openai::ComputerScreenshotKind>(std::string_view sv) {
    if (sv == "computer_screenshot") return openai::ComputerScreenshotKind::COMPUTER_SCREENSHOT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ComputerUseToolKind> from_string_view<openai::ComputerUseToolKind>(std::string_view sv) {
    if (sv == "computer_use_preview") return openai::ComputerUseToolKind::COMPUTER_USE_PREVIEW;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ContainerFileCitationKind> from_string_view<openai::ContainerFileCitationKind>(std::string_view sv) {
    if (sv == "container_file_citation") return openai::ContainerFileCitationKind::CONTAINER_FILE_CITATION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CreateFileOperationKind> from_string_view<openai::CreateFileOperationKind>(std::string_view sv) {
    if (sv == "create_file") return openai::CreateFileOperationKind::CREATE_FILE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolCallKind> from_string_view<openai::CustomToolCallKind>(std::string_view sv) {
    if (sv == "custom_tool_call") return openai::CustomToolCallKind::CUSTOM_TOOL_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolCallOutputKind> from_string_view<openai::CustomToolCallOutputKind>(std::string_view sv) {
    if (sv == "custom_tool_call_output") return openai::CustomToolCallOutputKind::CUSTOM_TOOL_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolChoiceKind> from_string_view<openai::CustomToolChoiceKind>(std::string_view sv) {
    if (sv == "custom") return openai::CustomToolChoiceKind::CUSTOM;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolFormatType> from_string_view<openai::CustomToolFormatType>(std::string_view sv) {
    if (sv == "text") return openai::CustomToolFormatType::TEXT;
    if (sv == "grammar") return openai::CustomToolFormatType::GRAMMAR;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolGrammarFormatKind> from_string_view<openai::CustomToolGrammarFormatKind>(std::string_view sv) {
    if (sv == "grammar") return openai::CustomToolGrammarFormatKind::GRAMMAR;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolKind> from_string_view<openai::CustomToolKind>(std::string_view sv) {
    if (sv == "custom") return openai::CustomToolKind::CUSTOM;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolTextFormatKind> from_string_view<openai::CustomToolTextFormatKind>(std::string_view sv) {
    if (sv == "text") return openai::CustomToolTextFormatKind::TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::DeleteFileOperationKind> from_string_view<openai::DeleteFileOperationKind>(std::string_view sv) {
    if (sv == "delete_file") return openai::DeleteFileOperationKind::DELETE_FILE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::Detail> from_string_view<openai::Detail>(std::string_view sv) {
    if (sv == "high") return openai::Detail::HIGH;
    if (sv == "low") return openai::Detail::LOW;
    if (sv == "auto") return openai::Detail::AUTO;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::DoubleClickActionKind> from_string_view<openai::DoubleClickActionKind>(std::string_view sv) {
    if (sv == "double_click") return openai::DoubleClickActionKind::DOUBLE_CLICK;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::DragActionKind> from_string_view<openai::DragActionKind>(std::string_view sv) {
    if (sv == "drag") return openai::DragActionKind::DRAG;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FileCitationKind> from_string_view<openai::FileCitationKind>(std::string_view sv) {
    if (sv == "file_citation") return openai::FileCitationKind::FILE_CITATION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FilePathKind> from_string_view<openai::FilePathKind>(std::string_view sv) {
    if (sv == "file_path") return openai::FilePathKind::FILE_PATH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FileSearchCallKind> from_string_view<openai::FileSearchCallKind>(std::string_view sv) {
    if (sv == "file_search_call") return openai::FileSearchCallKind::FILE_SEARCH_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FileSearchStatus> from_string_view<openai::FileSearchStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::FileSearchStatus::IN_PROGRESS;
    if (sv == "searching") return openai::FileSearchStatus::SEARCHING;
    if (sv == "incomplete") return openai::FileSearchStatus::INCOMPLETE;
    if (sv == "failed") return openai::FileSearchStatus::FAILED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FileSearchToolKind> from_string_view<openai::FileSearchToolKind>(std::string_view sv) {
    if (sv == "file_search") return openai::FileSearchToolKind::FILE_SEARCH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FilterCompoundType> from_string_view<openai::FilterCompoundType>(std::string_view sv) {
    if (sv == "and") return openai::FilterCompoundType::AND;
    if (sv == "or") return openai::FilterCompoundType::OR;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FilterOperator> from_string_view<openai::FilterOperator>(std::string_view sv) {
    if (sv == "eq") return openai::FilterOperator::EQ;
    if (sv == "ne") return openai::FilterOperator::NE;
    if (sv == "gt") return openai::FilterOperator::GT;
    if (sv == "gte") return openai::FilterOperator::GTE;
    if (sv == "lt") return openai::FilterOperator::LT;
    if (sv == "lte") return openai::FilterOperator::LTE;
    if (sv == "in") return openai::FilterOperator::IN;
    if (sv == "nin") return openai::FilterOperator::NIN;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FindActionKind> from_string_view<openai::FindActionKind>(std::string_view sv) {
    if (sv == "find") return openai::FindActionKind::FIND;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FunctionCallKind> from_string_view<openai::FunctionCallKind>(std::string_view sv) {
    if (sv == "function_call") return openai::FunctionCallKind::FUNCTION_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FunctionCallOutputKind> from_string_view<openai::FunctionCallOutputKind>(std::string_view sv) {
    if (sv == "function_call_output") return openai::FunctionCallOutputKind::FUNCTION_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FunctionCallStatus> from_string_view<openai::FunctionCallStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::FunctionCallStatus::IN_PROGRESS;
    if (sv == "completed") return openai::FunctionCallStatus::COMPLETED;
    if (sv == "incomplete") return openai::FunctionCallStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FunctionToolChoiceKind> from_string_view<openai::FunctionToolChoiceKind>(std::string_view sv) {
    if (sv == "function") return openai::FunctionToolChoiceKind::FUNCTION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FunctionToolKind> from_string_view<openai::FunctionToolKind>(std::string_view sv) {
    if (sv == "function") return openai::FunctionToolKind::FUNCTION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::GrammarSyntax> from_string_view<openai::GrammarSyntax>(std::string_view sv) {
    if (sv == "lark") return openai::GrammarSyntax::LARK;
    if (sv == "regex") return openai::GrammarSyntax::REGEX;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::HostedToolMode> from_string_view<openai::HostedToolMode>(std::string_view sv) {
    if (sv == "file_search") return openai::HostedToolMode::FILE_SEARCH;
    if (sv == "web_search_preview") return openai::HostedToolMode::WEB_SEARCH_PREVIEW;
    if (sv == "computer_use_preview") return openai::HostedToolMode::COMPUTER_USE_PREVIEW;
    if (sv == "code_interpreter") return openai::HostedToolMode::CODE_INTERPRETER;
    if (sv == "image_generation") return openai::HostedToolMode::IMAGE_GENERATION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationBackground> from_string_view<openai::ImageGenerationBackground>(std::string_view sv) {
    if (sv == "transparent") return openai::ImageGenerationBackground::TRANSPARENT;
    if (sv == "opaque") return openai::ImageGenerationBackground::OPAQUE;
    if (sv == "auto") return openai::ImageGenerationBackground::AUTO;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationCallKind> from_string_view<openai::ImageGenerationCallKind>(std::string_view sv) {
    if (sv == "image_generation_call") return openai::ImageGenerationCallKind::IMAGE_GENERATION_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationFidelity> from_string_view<openai::ImageGenerationFidelity>(std::string_view sv) {
    if (sv == "high") return openai::ImageGenerationFidelity::HIGH;
    if (sv == "low") return openai::ImageGenerationFidelity::LOW;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationFormat> from_string_view<openai::ImageGenerationFormat>(std::string_view sv) {
    if (sv == "png") return openai::ImageGenerationFormat::PNG;
    if (sv == "webp") return openai::ImageGenerationFormat::WEBP;
    if (sv == "jpeg") return openai::ImageGenerationFormat::JPEG;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationQuality> from_string_view<openai::ImageGenerationQuality>(std::string_view sv) {
    if (sv == "low") return openai::ImageGenerationQuality::LOW;
    if (sv == "medium") return openai::ImageGenerationQuality::MEDIUM;
    if (sv == "high") return openai::ImageGenerationQuality::HIGH;
    if (sv == "auto") return openai::ImageGenerationQuality::AUTO;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationSize> from_string_view<openai::ImageGenerationSize>(std::string_view sv) {
    if (sv == "1024x1024") return openai::ImageGenerationSize::SIZE_1024_1024;
    if (sv == "1024x1536") return openai::ImageGenerationSize::SIZE_1024_1536;
    if (sv == "1536x1024") return openai::ImageGenerationSize::SIZE_1536_1024;
    if (sv == "auto") return openai::ImageGenerationSize::AUTO;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationToolKind> from_string_view<openai::ImageGenerationToolKind>(std::string_view sv) {
    if (sv == "image_generation") return openai::ImageGenerationToolKind::IMAGE_GENERATION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::IncompleteReason> from_string_view<openai::IncompleteReason>(std::string_view sv) {
    if (sv == "max_output_tokens") return openai::IncompleteReason::MAX_OUTPUT_TOKENS;
    if (sv == "content_filter") return openai::IncompleteReason::CONTENT_FILTER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputFileKind> from_string_view<openai::InputFileKind>(std::string_view sv) {
    if (sv == "input_file") return openai::InputFileKind::INPUT_FILE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputImageKind> from_string_view<openai::InputImageKind>(std::string_view sv) {
    if (sv == "input_image") return openai::InputImageKind::INPUT_IMAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputItemType> from_string_view<openai::InputItemType>(std::string_view sv) {
    if (sv == "input_text") return openai::InputItemType::INPUT_TEXT;
    if (sv == "input_image") return openai::InputItemType::INPUT_IMAGE;
    if (sv == "input_file") return openai::InputItemType::INPUT_FILE;
    if (sv == "message") return openai::InputItemType::MESSAGE;
    if (sv == "reasoning") return openai::InputItemType::REASONING;
    if (sv == "compaction") return openai::InputItemType::COMPACTION;
    if (sv == "file_search_call") return openai::InputItemType::FILE_SEARCH_CALL;
    if (sv == "computer_call") return openai::InputItemType::COMPUTER_CALL;
    if (sv == "computer_call_output") return openai::InputItemType::COMPUTER_CALL_OUTPUT;
    if (sv == "web_search_call") return openai::InputItemType::WEB_SEARCH_CALL;
    if (sv == "function_call") return openai::InputItemType::FUNCTION_CALL;
    if (sv == "function_call_output") return openai::InputItemType::FUNCTION_CALL_OUTPUT;
    if (sv == "image_generation_call") return openai::InputItemType::IMAGE_GENERATION_CALL;
    if (sv == "code_interpreter_call") return openai::InputItemType::CODE_INTERPRETER_CALL;
    if (sv == "local_shell_call") return openai::InputItemType::LOCAL_SHELL_CALL;
    if (sv == "local_shell_call_output") return openai::InputItemType::LOCAL_SHELL_CALL_OUTPUT;
    if (sv == "shell_call") return openai::InputItemType::SHELL_CALL;
    if (sv == "shell_call_output") return openai::InputItemType::SHELL_CALL_OUTPUT;
    if (sv == "apply_patch_call") return openai::InputItemType::APPLY_PATCH_CALL;
    if (sv == "apply_patch_call_output") return openai::InputItemType::APPLY_PATCH_CALL_OUTPUT;
    if (sv == "mcp_list_tools") return openai::InputItemType::MCP_LIST_TOOLS;
    if (sv == "mcp_approval_request") return openai::InputItemType::MCP_APPROVAL_REQUEST;
    if (sv == "mcp_approval_response") return openai::InputItemType::MCP_APPROVAL_RESPONSE;
    if (sv == "mcp_call") return openai::InputItemType::MCP_CALL;
    if (sv == "custom_tool_call") return openai::InputItemType::CUSTOM_TOOL_CALL;
    if (sv == "custom_tool_call_output") return openai::InputItemType::CUSTOM_TOOL_CALL_OUTPUT;
    if (sv == "item_reference") return openai::InputItemType::ITEM_REFERENCE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputMessageKind> from_string_view<openai::InputMessageKind>(std::string_view sv) {
    if (sv == "message") return openai::InputMessageKind::MESSAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputTextKind> from_string_view<openai::InputTextKind>(std::string_view sv) {
    if (sv == "input_text") return openai::InputTextKind::INPUT_TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ItemReferenceKind> from_string_view<openai::ItemReferenceKind>(std::string_view sv) {
    if (sv == "item_reference") return openai::ItemReferenceKind::ITEM_REFERENCE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ItemStatus> from_string_view<openai::ItemStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::ItemStatus::IN_PROGRESS;
    if (sv == "completed") return openai::ItemStatus::COMPLETED;
    if (sv == "incomplete") return openai::ItemStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KeyPressActionKind> from_string_view<openai::KeyPressActionKind>(std::string_view sv) {
    if (sv == "keypress") return openai::KeyPressActionKind::KEYPRESS;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocalShellActionKind> from_string_view<openai::LocalShellActionKind>(std::string_view sv) {
    if (sv == "exec") return openai::LocalShellActionKind::EXEC;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocalShellCallKind> from_string_view<openai::LocalShellCallKind>(std::string_view sv) {
    if (sv == "local_shell_call") return openai::LocalShellCallKind::LOCAL_SHELL_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocalShellCallOutputKind> from_string_view<openai::LocalShellCallOutputKind>(std::string_view sv) {
    if (sv == "local_shell_call_output") return openai::LocalShellCallOutputKind::LOCAL_SHELL_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocalShellToolKind> from_string_view<openai::LocalShellToolKind>(std::string_view sv) {
    if (sv == "local_shell") return openai::LocalShellToolKind::LOCAL_SHELL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocalShellActionType> from_string_view<openai::LocalShellActionType>(std::string_view sv) {
    if (sv == "exec") return openai::LocalShellActionType::EXEC;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::LocationType> from_string_view<openai::LocationType>(std::string_view sv) {
    if (sv == "approximate") return openai::LocationType::APPROXIMATE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpApprovalRequestKind> from_string_view<openai::McpApprovalRequestKind>(std::string_view sv) {
    if (sv == "mcp_approval_request") return openai::McpApprovalRequestKind::MCP_APPROVAL_REQUEST;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpApprovalResponseKind> from_string_view<openai::McpApprovalResponseKind>(std::string_view sv) {
    if (sv == "mcp_approval_response") return openai::McpApprovalResponseKind::MCP_APPROVAL_RESPONSE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpApprovalSetting> from_string_view<openai::McpApprovalSetting>(std::string_view sv) {
    if (sv == "always") return openai::McpApprovalSetting::ALWAYS;
    if (sv == "never") return openai::McpApprovalSetting::NEVER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpCallKind> from_string_view<openai::McpCallKind>(std::string_view sv) {
    if (sv == "mcp_call") return openai::McpCallKind::MCP_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpListToolsKind> from_string_view<openai::McpListToolsKind>(std::string_view sv) {
    if (sv == "mcp_list_tools") return openai::McpListToolsKind::MCP_LIST_TOOLS;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpToolChoiceKind> from_string_view<openai::McpToolChoiceKind>(std::string_view sv) {
    if (sv == "mcp") return openai::McpToolChoiceKind::MCP;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::McpToolKind> from_string_view<openai::McpToolKind>(std::string_view sv) {
    if (sv == "mcp") return openai::McpToolKind::MCP;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::MouseButton> from_string_view<openai::MouseButton>(std::string_view sv) {
    if (sv == "left") return openai::MouseButton::LEFT;
    if (sv == "right") return openai::MouseButton::RIGHT;
    if (sv == "wheel") return openai::MouseButton::WHEEL;
    if (sv == "back") return openai::MouseButton::BACK;
    if (sv == "forward") return openai::MouseButton::FORWARD;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::MoveActionKind> from_string_view<openai::MoveActionKind>(std::string_view sv) {
    if (sv == "move") return openai::MoveActionKind::MOVE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::OpenPageActionKind> from_string_view<openai::OpenPageActionKind>(std::string_view sv) {
    if (sv == "open_page") return openai::OpenPageActionKind::OPEN_PAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::OutputMessageContentType> from_string_view<openai::OutputMessageContentType>(std::string_view sv) {
    if (sv == "output_text") return openai::OutputMessageContentType::OUTPUT_TEXT;
    if (sv == "refusal") return openai::OutputMessageContentType::REFUSAL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::OutputMessageKind> from_string_view<openai::OutputMessageKind>(std::string_view sv) {
    if (sv == "message") return openai::OutputMessageKind::MESSAGE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::OutputTextKind> from_string_view<openai::OutputTextKind>(std::string_view sv) {
    if (sv == "output_text") return openai::OutputTextKind::OUTPUT_TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::PendingSafetyCheckStatus> from_string_view<openai::PendingSafetyCheckStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::PendingSafetyCheckStatus::IN_PROGRESS;
    if (sv == "completed") return openai::PendingSafetyCheckStatus::COMPLETED;
    if (sv == "incomplete") return openai::PendingSafetyCheckStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::PromptCacheRetention> from_string_view<openai::PromptCacheRetention>(std::string_view sv) {
    if (sv == "24h") return openai::PromptCacheRetention::HOURS_24;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningEffort> from_string_view<openai::ReasoningEffort>(std::string_view sv) {
    if (sv == "none") return openai::ReasoningEffort::NONE;
    if (sv == "minimal") return openai::ReasoningEffort::MINIMAL;
    if (sv == "low") return openai::ReasoningEffort::LOW;
    if (sv == "medium") return openai::ReasoningEffort::MEDIUM;
    if (sv == "high") return openai::ReasoningEffort::HIGH;
    if (sv == "xhigh") return openai::ReasoningEffort::XHIGH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningItemContentType> from_string_view<openai::ReasoningItemContentType>(std::string_view sv) {
    if (sv == "summary_text") return openai::ReasoningItemContentType::SUMMARY_TEXT;
    if (sv == "reasoning_text") return openai::ReasoningItemContentType::REASONING_TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningItemKind> from_string_view<openai::ReasoningItemKind>(std::string_view sv) {
    if (sv == "reasoning") return openai::ReasoningItemKind::REASONING;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningStatus> from_string_view<openai::ReasoningStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::ReasoningStatus::IN_PROGRESS;
    if (sv == "completed") return openai::ReasoningStatus::COMPLETED;
    if (sv == "incomplete") return openai::ReasoningStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningSummary> from_string_view<openai::ReasoningSummary>(std::string_view sv) {
    if (sv == "auto") return openai::ReasoningSummary::AUTO;
    if (sv == "concise") return openai::ReasoningSummary::CONCISE;
    if (sv == "detailed") return openai::ReasoningSummary::DETAILED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningSummaryTextKind> from_string_view<openai::ReasoningSummaryTextKind>(std::string_view sv) {
    if (sv == "summary_text") return openai::ReasoningSummaryTextKind::SUMMARY_TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningTextKind> from_string_view<openai::ReasoningTextKind>(std::string_view sv) {
    if (sv == "reasoning_text") return openai::ReasoningTextKind::REASONING_TEXT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::RefusalKind> from_string_view<openai::RefusalKind>(std::string_view sv) {
    if (sv == "refusal") return openai::RefusalKind::REFUSAL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ResponseFormatType> from_string_view<openai::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return openai::ResponseFormatType::TEXT;
    if (sv == "json_schema") return openai::ResponseFormatType::JSON_SCHEMA;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ResponseKind> from_string_view<openai::ResponseKind>(std::string_view sv) {
    if (sv == "response") return openai::ResponseKind::RESPONSE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ResponseStatus> from_string_view<openai::ResponseStatus>(std::string_view sv) {
    if (sv == "completed") return openai::ResponseStatus::COMPLETED;
    if (sv == "failed") return openai::ResponseStatus::FAILED;
    if (sv == "in_progress") return openai::ResponseStatus::IN_PROGRESS;
    if (sv == "cancelled") return openai::ResponseStatus::CANCELLED;
    if (sv == "queued") return openai::ResponseStatus::QUEUED;
    if (sv == "incomplete") return openai::ResponseStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::Role> from_string_view<openai::Role>(std::string_view sv) {
    if (sv == "user") return openai::Role::USER;
    if (sv == "assistant") return openai::Role::ASSISTANT;
    if (sv == "system") return openai::Role::SYSTEM;
    if (sv == "developer") return openai::Role::DEVELOPER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ScreenshotActionKind> from_string_view<openai::ScreenshotActionKind>(std::string_view sv) {
    if (sv == "screenshot") return openai::ScreenshotActionKind::SCREENSHOT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ScrollActionKind> from_string_view<openai::ScrollActionKind>(std::string_view sv) {
    if (sv == "scroll") return openai::ScrollActionKind::SCROLL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::SearchActionKind> from_string_view<openai::SearchActionKind>(std::string_view sv) {
    if (sv == "search") return openai::SearchActionKind::SEARCH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::SearchContextSize> from_string_view<openai::SearchContextSize>(std::string_view sv) {
    if (sv == "low") return openai::SearchContextSize::LOW;
    if (sv == "medium") return openai::SearchContextSize::MEDIUM;
    if (sv == "high") return openai::SearchContextSize::HIGH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ServiceTier> from_string_view<openai::ServiceTier>(std::string_view sv) {
    if (sv == "auto") return openai::ServiceTier::AUTO;
    if (sv == "default") return openai::ServiceTier::DEFAULT;
    if (sv == "flex") return openai::ServiceTier::FLEX;
    if (sv == "scale") return openai::ServiceTier::SCALE;
    if (sv == "priority") return openai::ServiceTier::PRIORITY;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellCallKind> from_string_view<openai::ShellCallKind>(std::string_view sv) {
    if (sv == "shell_call") return openai::ShellCallKind::SHELL_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellCallOutcomeType> from_string_view<openai::ShellCallOutcomeType>(std::string_view sv) {
    if (sv == "exit") return openai::ShellCallOutcomeType::EXIT;
    if (sv == "timeout") return openai::ShellCallOutcomeType::TIMEOUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellCallOutputKind> from_string_view<openai::ShellCallOutputKind>(std::string_view sv) {
    if (sv == "shell_call_output") return openai::ShellCallOutputKind::SHELL_CALL_OUTPUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellExitOutcomeKind> from_string_view<openai::ShellExitOutcomeKind>(std::string_view sv) {
    if (sv == "exit") return openai::ShellExitOutcomeKind::EXIT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellTimeoutOutcomeKind> from_string_view<openai::ShellTimeoutOutcomeKind>(std::string_view sv) {
    if (sv == "timeout") return openai::ShellTimeoutOutcomeKind::TIMEOUT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellToolKind> from_string_view<openai::ShellToolKind>(std::string_view sv) {
    if (sv == "shell") return openai::ShellToolKind::SHELL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::SpecificApplyPatchToolChoiceKind> from_string_view<openai::SpecificApplyPatchToolChoiceKind>(std::string_view sv) {
    if (sv == "apply_patch") return openai::SpecificApplyPatchToolChoiceKind::APPLY_PATCH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::SpecificShellToolChoiceKind> from_string_view<openai::SpecificShellToolChoiceKind>(std::string_view sv) {
    if (sv == "shell") return openai::SpecificShellToolChoiceKind::SHELL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ToolChoiceMode> from_string_view<openai::ToolChoiceMode>(std::string_view sv) {
    if (sv == "none") return openai::ToolChoiceMode::NONE;
    if (sv == "auto") return openai::ToolChoiceMode::AUTO;
    if (sv == "required") return openai::ToolChoiceMode::REQUIRED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ToolChoiceType> from_string_view<openai::ToolChoiceType>(std::string_view sv) {
    if (sv == "allowed_tools") return openai::ToolChoiceType::ALLOWED_TOOLS;
    if (sv == "function") return openai::ToolChoiceType::FUNCTION;
    if (sv == "mcp") return openai::ToolChoiceType::MCP;
    if (sv == "custom") return openai::ToolChoiceType::CUSTOM;
    if (sv == "apply_patch") return openai::ToolChoiceType::APPLY_PATCH;
    if (sv == "shell") return openai::ToolChoiceType::SHELL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ToolType> from_string_view<openai::ToolType>(std::string_view sv) {
    if (sv == "function") return openai::ToolType::FUNCTION;
    if (sv == "file_search") return openai::ToolType::FILE_SEARCH;
    if (sv == "computer_use_preview") return openai::ToolType::COMPUTER_USE_PREVIEW;
    if (sv == "web_search") return openai::ToolType::WEB_SEARCH;
    if (sv == "mcp") return openai::ToolType::MCP;
    if (sv == "code_interpreter") return openai::ToolType::CODE_INTERPRETER;
    if (sv == "image_generation") return openai::ToolType::IMAGE_GENERATION;
    if (sv == "local_shell") return openai::ToolType::LOCAL_SHELL;
    if (sv == "shell") return openai::ToolType::SHELL;
    if (sv == "custom") return openai::ToolType::CUSTOM;
    if (sv == "web_search_preview") return openai::ToolType::WEB_SEARCH_PREVIEW;
    if (sv == "apply_patch") return openai::ToolType::APPLY_PATCH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::TruncationStrategy> from_string_view<openai::TruncationStrategy>(std::string_view sv) {
    if (sv == "auto") return openai::TruncationStrategy::AUTO;
    if (sv == "disabled") return openai::TruncationStrategy::DISABLED;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::TypeActionKind> from_string_view<openai::TypeActionKind>(std::string_view sv) {
    if (sv == "type") return openai::TypeActionKind::TYPE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::UpdateFileOperationKind> from_string_view<openai::UpdateFileOperationKind>(std::string_view sv) {
    if (sv == "update_file") return openai::UpdateFileOperationKind::UPDATE_FILE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::UrlCitationKind> from_string_view<openai::UrlCitationKind>(std::string_view sv) {
    if (sv == "url_citation") return openai::UrlCitationKind::URL_CITATION;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::Verbosity> from_string_view<openai::Verbosity>(std::string_view sv) {
    if (sv == "low") return openai::Verbosity::LOW;
    if (sv == "medium") return openai::Verbosity::MEDIUM;
    if (sv == "high") return openai::Verbosity::HIGH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WaitActionKind> from_string_view<openai::WaitActionKind>(std::string_view sv) {
    if (sv == "wait") return openai::WaitActionKind::WAIT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchActionType> from_string_view<openai::WebSearchActionType>(std::string_view sv) {
    if (sv == "search") return openai::WebSearchActionType::SEARCH;
    if (sv == "open_page") return openai::WebSearchActionType::OPEN_PAGE;
    if (sv == "find") return openai::WebSearchActionType::FIND;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchCallKind> from_string_view<openai::WebSearchCallKind>(std::string_view sv) {
    if (sv == "web_search_call") return openai::WebSearchCallKind::WEB_SEARCH_CALL;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchPreviewToolKind> from_string_view<openai::WebSearchPreviewToolKind>(std::string_view sv) {
    if (sv == "web_search_preview") return openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW;
    if (sv == "web_search_preview_2025_03_11") return openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW_2025_03_11;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchPreviewToolType> from_string_view<openai::WebSearchPreviewToolType>(std::string_view sv) {
    if (sv == "web_search_preview") return openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW;
    if (sv == "web_search_preview_2025_03_11") return openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW_2025_03_11;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchStatus> from_string_view<openai::WebSearchStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::WebSearchStatus::IN_PROGRESS;
    if (sv == "completed") return openai::WebSearchStatus::COMPLETED;
    if (sv == "incomplete") return openai::WebSearchStatus::INCOMPLETE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchToolKind> from_string_view<openai::WebSearchToolKind>(std::string_view sv) {
    if (sv == "web_search") return openai::WebSearchToolKind::WEB_SEARCH;
    if (sv == "web_search_2025_08_26") return openai::WebSearchToolKind::WEB_SEARCH_2025_08_26;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchToolType> from_string_view<openai::WebSearchToolType>(std::string_view sv) {
    if (sv == "web_search") return openai::WebSearchToolType::WEB_SEARCH;
    if (sv == "web_search_2025_08_26") return openai::WebSearchToolType::WEB_SEARCH_2025_08_26;
    return std::nullopt;
}


/***
 * to_string_view Overloads
 */

constexpr std::string_view to_string_view(openai::AllowedToolsChoiceKind val) {
    switch (val) {
        case openai::AllowedToolsChoiceKind::ALLOWED_TOOLS: return "allowed_tools";
        default: throw std::logic_error("invalid openai::AllowedToolsChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::AnnotationType val) {
    switch (val) {
        case openai::AnnotationType::FILE_CITATION: return "file_citation";
        case openai::AnnotationType::URL_CITATION: return "url_citation";
        case openai::AnnotationType::CONTAINER_FILE_CITATION: return "container_file_citation";
        case openai::AnnotationType::FILE_PATH: return "file_path";
        default: throw std::logic_error("invalid openai::AnnotationType");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallKind val) {
    switch (val) {
        case openai::ApplyPatchCallKind::APPLY_PATCH_CALL: return "apply_patch_call";
        default: throw std::logic_error("invalid openai::ApplyPatchCallKind");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallOutputKind val) {
    switch (val) {
        case openai::ApplyPatchCallOutputKind::APPLY_PATCH_CALL_OUTPUT: return "apply_patch_call_output";
        default: throw std::logic_error("invalid openai::ApplyPatchCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallOutputStatus val) {
    switch (val) {
        case openai::ApplyPatchCallOutputStatus::COMPLETED: return "completed";
        case openai::ApplyPatchCallOutputStatus::FAILED: return "failed";
        default: throw std::logic_error("invalid openai::ApplyPatchCallOutputStatus");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallStatus val) {
    switch (val) {
        case openai::ApplyPatchCallStatus::IN_PROGRESS: return "in_progress";
        case openai::ApplyPatchCallStatus::COMPLETED: return "completed";
        default: throw std::logic_error("invalid openai::ApplyPatchCallStatus");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchOperationType val) {
    switch (val) {
        case openai::ApplyPatchOperationType::CREATE_FILE: return "create_file";
        case openai::ApplyPatchOperationType::DELETE_FILE: return "delete_file";
        case openai::ApplyPatchOperationType::UPDATE_FILE: return "update_file";
        default: throw std::logic_error("invalid openai::ApplyPatchOperationType");
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchToolKind val) {
    switch (val) {
        case openai::ApplyPatchToolKind::APPLY_PATCH: return "apply_patch";
        default: throw std::logic_error("invalid openai::ApplyPatchToolKind");
    }
}

constexpr std::string_view to_string_view(openai::CallStatus val) {
    switch (val) {
        case openai::CallStatus::IN_PROGRESS: return "in_progress";
        case openai::CallStatus::COMPLETED: return "completed";
        case openai::CallStatus::INCOMPLETE: return "incomplete";
        case openai::CallStatus::CALLING: return "calling";
        case openai::CallStatus::FAILED: return "failed";
        default: throw std::logic_error("invalid openai::CallStatus");
    }
}

constexpr std::string_view to_string_view(openai::ClickActionKind val) {
    switch (val) {
        case openai::ClickActionKind::CLICK: return "click";
        default: throw std::logic_error("invalid openai::ClickActionKind");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterCallKind val) {
    switch (val) {
        case openai::CodeInterpreterCallKind::CODE_INTERPRETER_CALL: return "code_interpreter_call";
        default: throw std::logic_error("invalid openai::CodeInterpreterCallKind");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterImageKind val) {
    switch (val) {
        case openai::CodeInterpreterImageKind::IMAGE: return "image";
        default: throw std::logic_error("invalid openai::CodeInterpreterImageKind");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterLogKind val) {
    switch (val) {
        case openai::CodeInterpreterLogKind::LOGS: return "logs";
        default: throw std::logic_error("invalid openai::CodeInterpreterLogKind");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterOutputType val) {
    switch (val) {
        case openai::CodeInterpreterOutputType::LOGS: return "logs";
        case openai::CodeInterpreterOutputType::IMAGE: return "image";
        default: throw std::logic_error("invalid openai::CodeInterpreterOutputType");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterStatus val) {
    switch (val) {
        case openai::CodeInterpreterStatus::IN_PROGRESS: return "in_progress";
        case openai::CodeInterpreterStatus::COMPLETED: return "completed";
        case openai::CodeInterpreterStatus::INCOMPLETE: return "incomplete";
        case openai::CodeInterpreterStatus::INTERPRETING: return "interpreting";
        case openai::CodeInterpreterStatus::FAILED: return "failed";
        default: throw std::logic_error("invalid openai::CodeInterpreterStatus");
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterToolKind val) {
    switch (val) {
        case openai::CodeInterpreterToolKind::CODE_INTERPRETER: return "code_interpreter";
        default: throw std::logic_error("invalid openai::CodeInterpreterToolKind");
    }
}

constexpr std::string_view to_string_view(openai::CompactionItemKind val) {
    switch (val) {
        case openai::CompactionItemKind::COMPACTION: return "compaction";
        default: throw std::logic_error("invalid openai::CompactionItemKind");
    }
}

constexpr std::string_view to_string_view(openai::ComputerActionType val) {
    switch (val) {
        case openai::ComputerActionType::CLICK: return "click";
        case openai::ComputerActionType::DOUBLE_CLICK: return "double_click";
        case openai::ComputerActionType::DRAG: return "drag";
        case openai::ComputerActionType::KEYPRESS: return "keypress";
        case openai::ComputerActionType::MOVE: return "move";
        case openai::ComputerActionType::SCREENSHOT: return "screenshot";
        case openai::ComputerActionType::SCROLL: return "scroll";
        case openai::ComputerActionType::TYPE: return "type";
        case openai::ComputerActionType::WAIT: return "wait";
        default: throw std::logic_error("invalid openai::ComputerActionType");
    }
}

constexpr std::string_view to_string_view(openai::ComputerCallKind val) {
    switch (val) {
        case openai::ComputerCallKind::COMPUTER_CALL: return "computer_call";
        default: throw std::logic_error("invalid openai::ComputerCallKind");
    }
}

constexpr std::string_view to_string_view(openai::ComputerCallOutputKind val) {
    switch (val) {
        case openai::ComputerCallOutputKind::COMPUTER_CALL_OUTPUT: return "computer_call_output";
        default: throw std::logic_error("invalid openai::ComputerCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::ComputerCallOutputType val) {
    switch (val) {
        case openai::ComputerCallOutputType::COMPUTER_SCREENSHOT: return "computer_screenshot";
        default: throw std::logic_error("invalid openai::ComputerCallOutputType");
    }
}

constexpr std::string_view to_string_view(openai::ComputerScreenshotKind val) {
    switch (val) {
        case openai::ComputerScreenshotKind::COMPUTER_SCREENSHOT: return "computer_screenshot";
        default: throw std::logic_error("invalid openai::ComputerScreenshotKind");
    }
}

constexpr std::string_view to_string_view(openai::ComputerUseToolKind val) {
    switch (val) {
        case openai::ComputerUseToolKind::COMPUTER_USE_PREVIEW: return "computer_use_preview";
        default: throw std::logic_error("invalid openai::ComputerUseToolKind");
    }
}

constexpr std::string_view to_string_view(openai::ContainerFileCitationKind val) {
    switch (val) {
        case openai::ContainerFileCitationKind::CONTAINER_FILE_CITATION: return "container_file_citation";
        default: throw std::logic_error("invalid openai::ContainerFileCitationKind");
    }
}

constexpr std::string_view to_string_view(openai::CreateFileOperationKind val) {
    switch (val) {
        case openai::CreateFileOperationKind::CREATE_FILE: return "create_file";
        default: throw std::logic_error("invalid openai::CreateFileOperationKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolCallKind val) {
    switch (val) {
        case openai::CustomToolCallKind::CUSTOM_TOOL_CALL: return "custom_tool_call";
        default: throw std::logic_error("invalid openai::CustomToolCallKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolCallOutputKind val) {
    switch (val) {
        case openai::CustomToolCallOutputKind::CUSTOM_TOOL_CALL_OUTPUT: return "custom_tool_call_output";
        default: throw std::logic_error("invalid openai::CustomToolCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolChoiceKind val) {
    switch (val) {
        case openai::CustomToolChoiceKind::CUSTOM: return "custom";
        default: throw std::logic_error("invalid openai::CustomToolChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolFormatType val) {
    switch (val) {
        case openai::CustomToolFormatType::TEXT: return "text";
        case openai::CustomToolFormatType::GRAMMAR: return "grammar";
        default: throw std::logic_error("invalid openai::CustomToolFormatType");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolGrammarFormatKind val) {
    switch (val) {
        case openai::CustomToolGrammarFormatKind::GRAMMAR: return "grammar";
        default: throw std::logic_error("invalid openai::CustomToolGrammarFormatKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolKind val) {
    switch (val) {
        case openai::CustomToolKind::CUSTOM: return "custom";
        default: throw std::logic_error("invalid openai::CustomToolKind");
    }
}

constexpr std::string_view to_string_view(openai::CustomToolTextFormatKind val) {
    switch (val) {
        case openai::CustomToolTextFormatKind::TEXT: return "text";
        default: throw std::logic_error("invalid openai::CustomToolTextFormatKind");
    }
}

constexpr std::string_view to_string_view(openai::DeleteFileOperationKind val) {
    switch (val) {
        case openai::DeleteFileOperationKind::DELETE_FILE: return "delete_file";
        default: throw std::logic_error("invalid openai::DeleteFileOperationKind");
    }
}

constexpr std::string_view to_string_view(openai::Detail val) {
    switch (val) {
        case openai::Detail::HIGH: return "high";
        case openai::Detail::LOW: return "low";
        case openai::Detail::AUTO: return "auto";
        default: throw std::logic_error("invalid openai::Detail");
    }
}

constexpr std::string_view to_string_view(openai::DoubleClickActionKind val) {
    switch (val) {
        case openai::DoubleClickActionKind::DOUBLE_CLICK: return "double_click";
        default: throw std::logic_error("invalid openai::DoubleClickActionKind");
    }
}

constexpr std::string_view to_string_view(openai::DragActionKind val) {
    switch (val) {
        case openai::DragActionKind::DRAG: return "drag";
        default: throw std::logic_error("invalid openai::DragActionKind");
    }
}

constexpr std::string_view to_string_view(openai::FileCitationKind val) {
    switch (val) {
        case openai::FileCitationKind::FILE_CITATION: return "file_citation";
        default: throw std::logic_error("invalid openai::FileCitationKind");
    }
}

constexpr std::string_view to_string_view(openai::FilePathKind val) {
    switch (val) {
        case openai::FilePathKind::FILE_PATH: return "file_path";
        default: throw std::logic_error("invalid openai::FilePathKind");
    }
}

constexpr std::string_view to_string_view(openai::FileSearchCallKind val) {
    switch (val) {
        case openai::FileSearchCallKind::FILE_SEARCH_CALL: return "file_search_call";
        default: throw std::logic_error("invalid openai::FileSearchCallKind");
    }
}

constexpr std::string_view to_string_view(openai::FileSearchStatus val) {
    switch (val) {
        case openai::FileSearchStatus::IN_PROGRESS: return "in_progress";
        case openai::FileSearchStatus::SEARCHING: return "searching";
        case openai::FileSearchStatus::INCOMPLETE: return "incomplete";
        case openai::FileSearchStatus::FAILED: return "failed";
        default: throw std::logic_error("invalid openai::FileSearchStatus");
    }
}

constexpr std::string_view to_string_view(openai::FileSearchToolKind val) {
    switch (val) {
        case openai::FileSearchToolKind::FILE_SEARCH: return "file_search";
        default: throw std::logic_error("invalid openai::FileSearchToolKind");
    }
}

constexpr std::string_view to_string_view(openai::FilterCompoundType val) {
    switch (val) {
        case openai::FilterCompoundType::AND: return "and";
        case openai::FilterCompoundType::OR: return "or";
        default: throw std::logic_error("invalid openai::FilterCompoundType");
    }
}

constexpr std::string_view to_string_view(openai::FilterOperator val) {
    switch (val) {
        case openai::FilterOperator::EQ: return "eq";
        case openai::FilterOperator::NE: return "ne";
        case openai::FilterOperator::GT: return "gt";
        case openai::FilterOperator::GTE: return "gte";
        case openai::FilterOperator::LT: return "lt";
        case openai::FilterOperator::LTE: return "lte";
        case openai::FilterOperator::IN: return "in";
        case openai::FilterOperator::NIN: return "nin";
        default: throw std::logic_error("invalid openai::FilterOperator");
    }
}

constexpr std::string_view to_string_view(openai::FindActionKind val) {
    switch (val) {
        case openai::FindActionKind::FIND: return "find";
        default: throw std::logic_error("invalid openai::FindActionKind");
    }
}

constexpr std::string_view to_string_view(openai::FunctionCallKind val) {
    switch (val) {
        case openai::FunctionCallKind::FUNCTION_CALL: return "function_call";
        default: throw std::logic_error("invalid openai::FunctionCallKind");
    }
}

constexpr std::string_view to_string_view(openai::FunctionCallOutputKind val) {
    switch (val) {
        case openai::FunctionCallOutputKind::FUNCTION_CALL_OUTPUT: return "function_call_output";
        default: throw std::logic_error("invalid openai::FunctionCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::FunctionCallStatus val) {
    switch (val) {
        case openai::FunctionCallStatus::IN_PROGRESS: return "in_progress";
        case openai::FunctionCallStatus::COMPLETED: return "completed";
        case openai::FunctionCallStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::FunctionCallStatus");
    }
}

constexpr std::string_view to_string_view(openai::FunctionToolChoiceKind val) {
    switch (val) {
        case openai::FunctionToolChoiceKind::FUNCTION: return "function";
        default: throw std::logic_error("invalid openai::FunctionToolChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::FunctionToolKind val) {
    switch (val) {
        case openai::FunctionToolKind::FUNCTION: return "function";
        default: throw std::logic_error("invalid openai::FunctionToolKind");
    }
}

constexpr std::string_view to_string_view(openai::GrammarSyntax val) {
    switch (val) {
        case openai::GrammarSyntax::LARK: return "lark";
        case openai::GrammarSyntax::REGEX: return "regex";
        default: throw std::logic_error("invalid openai::GrammarSyntax");
    }
}

constexpr std::string_view to_string_view(openai::HostedToolMode val) {
    switch (val) {
        case openai::HostedToolMode::FILE_SEARCH: return "file_search";
        case openai::HostedToolMode::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::HostedToolMode::COMPUTER_USE_PREVIEW: return "computer_use_preview";
        case openai::HostedToolMode::CODE_INTERPRETER: return "code_interpreter";
        case openai::HostedToolMode::IMAGE_GENERATION: return "image_generation";
        default: throw std::logic_error("invalid openai::HostedToolMode");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationBackground val) {
    switch (val) {
        case openai::ImageGenerationBackground::TRANSPARENT: return "transparent";
        case openai::ImageGenerationBackground::OPAQUE: return "opaque";
        case openai::ImageGenerationBackground::AUTO: return "auto";
        default: throw std::logic_error("invalid openai::ImageGenerationBackground");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationCallKind val) {
    switch (val) {
        case openai::ImageGenerationCallKind::IMAGE_GENERATION_CALL: return "image_generation_call";
        default: throw std::logic_error("invalid openai::ImageGenerationCallKind");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationFidelity val) {
    switch (val) {
        case openai::ImageGenerationFidelity::HIGH: return "high";
        case openai::ImageGenerationFidelity::LOW: return "low";
        default: throw std::logic_error("invalid openai::ImageGenerationFidelity");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationFormat val) {
    switch (val) {
        case openai::ImageGenerationFormat::PNG: return "png";
        case openai::ImageGenerationFormat::WEBP: return "webp";
        case openai::ImageGenerationFormat::JPEG: return "jpeg";
        default: throw std::logic_error("invalid openai::ImageGenerationFormat");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationQuality val) {
    switch (val) {
        case openai::ImageGenerationQuality::LOW: return "low";
        case openai::ImageGenerationQuality::MEDIUM: return "medium";
        case openai::ImageGenerationQuality::HIGH: return "high";
        case openai::ImageGenerationQuality::AUTO: return "auto";
        default: throw std::logic_error("invalid openai::ImageGenerationQuality");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationSize val) {
    switch (val) {
        case openai::ImageGenerationSize::SIZE_1024_1024: return "1024x1024";
        case openai::ImageGenerationSize::SIZE_1024_1536: return "1024x1536";
        case openai::ImageGenerationSize::SIZE_1536_1024: return "1536x1024";
        case openai::ImageGenerationSize::AUTO: return "auto";
        default: throw std::logic_error("invalid openai::ImageGenerationSize");
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationToolKind val) {
    switch (val) {
        case openai::ImageGenerationToolKind::IMAGE_GENERATION: return "image_generation";
        default: throw std::logic_error("invalid openai::ImageGenerationToolKind");
    }
}

constexpr std::string_view to_string_view(openai::IncompleteReason val) {
    switch (val) {
        case openai::IncompleteReason::MAX_OUTPUT_TOKENS: return "max_output_tokens";
        case openai::IncompleteReason::CONTENT_FILTER: return "content_filter";
        default: throw std::logic_error("invalid openai::IncompleteReason");
    }
}

constexpr std::string_view to_string_view(openai::InputFileKind val) {
    switch (val) {
        case openai::InputFileKind::INPUT_FILE: return "input_file";
        default: throw std::logic_error("invalid openai::InputFileKind");
    }
}

constexpr std::string_view to_string_view(openai::InputImageKind val) {
    switch (val) {
        case openai::InputImageKind::INPUT_IMAGE: return "input_image";
        default: throw std::logic_error("invalid openai::InputImageKind");
    }
}

constexpr std::string_view to_string_view(openai::InputItemType val) {
    switch (val) {
        case openai::InputItemType::INPUT_TEXT: return "input_text";
        case openai::InputItemType::INPUT_IMAGE: return "input_image";
        case openai::InputItemType::INPUT_FILE: return "input_file";
        case openai::InputItemType::MESSAGE: return "message";
        case openai::InputItemType::REASONING: return "reasoning";
        case openai::InputItemType::COMPACTION: return "compaction";
        case openai::InputItemType::FILE_SEARCH_CALL: return "file_search_call";
        case openai::InputItemType::COMPUTER_CALL: return "computer_call";
        case openai::InputItemType::COMPUTER_CALL_OUTPUT: return "computer_call_output";
        case openai::InputItemType::WEB_SEARCH_CALL: return "web_search_call";
        case openai::InputItemType::FUNCTION_CALL: return "function_call";
        case openai::InputItemType::FUNCTION_CALL_OUTPUT: return "function_call_output";
        case openai::InputItemType::IMAGE_GENERATION_CALL: return "image_generation_call";
        case openai::InputItemType::CODE_INTERPRETER_CALL: return "code_interpreter_call";
        case openai::InputItemType::LOCAL_SHELL_CALL: return "local_shell_call";
        case openai::InputItemType::LOCAL_SHELL_CALL_OUTPUT: return "local_shell_call_output";
        case openai::InputItemType::SHELL_CALL: return "shell_call";
        case openai::InputItemType::SHELL_CALL_OUTPUT: return "shell_call_output";
        case openai::InputItemType::APPLY_PATCH_CALL: return "apply_patch_call";
        case openai::InputItemType::APPLY_PATCH_CALL_OUTPUT: return "apply_patch_call_output";
        case openai::InputItemType::MCP_LIST_TOOLS: return "mcp_list_tools";
        case openai::InputItemType::MCP_APPROVAL_REQUEST: return "mcp_approval_request";
        case openai::InputItemType::MCP_APPROVAL_RESPONSE: return "mcp_approval_response";
        case openai::InputItemType::MCP_CALL: return "mcp_call";
        case openai::InputItemType::CUSTOM_TOOL_CALL: return "custom_tool_call";
        case openai::InputItemType::CUSTOM_TOOL_CALL_OUTPUT: return "custom_tool_call_output";
        case openai::InputItemType::ITEM_REFERENCE: return "item_reference";
        default: throw std::logic_error("invalid openai::InputItemType");
    }
}

constexpr std::string_view to_string_view(openai::InputMessageKind val) {
    switch (val) {
        case openai::InputMessageKind::MESSAGE: return "message";
        default: throw std::logic_error("invalid openai::InputMessageKind");
    }
}

constexpr std::string_view to_string_view(openai::InputTextKind val) {
    switch (val) {
        case openai::InputTextKind::INPUT_TEXT: return "input_text";
        default: throw std::logic_error("invalid openai::InputTextKind");
    }
}

constexpr std::string_view to_string_view(openai::ItemReferenceKind val) {
    switch (val) {
        case openai::ItemReferenceKind::ITEM_REFERENCE: return "item_reference";
        default: throw std::logic_error("invalid openai::ItemReferenceKind");
    }
}

constexpr std::string_view to_string_view(openai::ItemStatus val) {
    switch (val) {
        case openai::ItemStatus::IN_PROGRESS: return "in_progress";
        case openai::ItemStatus::COMPLETED: return "completed";
        case openai::ItemStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::ItemStatus");
    }
}

constexpr std::string_view to_string_view(openai::KeyPressActionKind val) {
    switch (val) {
        case openai::KeyPressActionKind::KEYPRESS: return "keypress";
        default: throw std::logic_error("invalid openai::KeyPressActionKind");
    }
}

constexpr std::string_view to_string_view(openai::LocalShellActionKind val) {
    switch (val) {
        case openai::LocalShellActionKind::EXEC: return "exec";
        default: throw std::logic_error("invalid openai::LocalShellActionKind");
    }
}

constexpr std::string_view to_string_view(openai::LocalShellActionType val) {
    switch (val) {
        case openai::LocalShellActionType::EXEC: return "exec";
        default: throw std::logic_error("invalid openai::LocalShellActionType");
    }
}

constexpr std::string_view to_string_view(openai::LocalShellCallKind val) {
    switch (val) {
        case openai::LocalShellCallKind::LOCAL_SHELL_CALL: return "local_shell_call";
        default: throw std::logic_error("invalid openai::LocalShellCallKind");
    }
}

constexpr std::string_view to_string_view(openai::LocalShellCallOutputKind val) {
    switch (val) {
        case openai::LocalShellCallOutputKind::LOCAL_SHELL_CALL_OUTPUT: return "local_shell_call_output";
        default: throw std::logic_error("invalid openai::LocalShellCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::LocalShellToolKind val) {
    switch (val) {
        case openai::LocalShellToolKind::LOCAL_SHELL: return "local_shell";
        default: throw std::logic_error("invalid openai::LocalShellToolKind");
    }
}

constexpr std::string_view to_string_view(openai::LocationType val) {
    switch (val) {
        case openai::LocationType::APPROXIMATE: return "approximate";
        default: throw std::logic_error("invalid openai::LocationType");
    }
}

constexpr std::string_view to_string_view(openai::McpApprovalRequestKind val) {
    switch (val) {
        case openai::McpApprovalRequestKind::MCP_APPROVAL_REQUEST: return "mcp_approval_request";
        default: throw std::logic_error("invalid openai::McpApprovalRequestKind");
    }
}

constexpr std::string_view to_string_view(openai::McpApprovalResponseKind val) {
    switch (val) {
        case openai::McpApprovalResponseKind::MCP_APPROVAL_RESPONSE: return "mcp_approval_response";
        default: throw std::logic_error("invalid openai::McpApprovalResponseKind");
    }
}

constexpr std::string_view to_string_view(openai::McpApprovalSetting val) {
    switch (val) {
        case openai::McpApprovalSetting::ALWAYS: return "always";
        case openai::McpApprovalSetting::NEVER: return "never";
        default: throw std::logic_error("invalid openai::McpApprovalSetting");
    }
}

constexpr std::string_view to_string_view(openai::McpCallKind val) {
    switch (val) {
        case openai::McpCallKind::MCP_CALL: return "mcp_call";
        default: throw std::logic_error("invalid openai::McpCallKind");
    }
}

constexpr std::string_view to_string_view(openai::McpListToolsKind val) {
    switch (val) {
        case openai::McpListToolsKind::MCP_LIST_TOOLS: return "mcp_list_tools";
        default: throw std::logic_error("invalid openai::McpListToolsKind");
    }
}

constexpr std::string_view to_string_view(openai::McpToolChoiceKind val) {
    switch (val) {
        case openai::McpToolChoiceKind::MCP: return "mcp";
        default: throw std::logic_error("invalid openai::McpToolChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::McpToolKind val) {
    switch (val) {
        case openai::McpToolKind::MCP: return "mcp";
        default: throw std::logic_error("invalid openai::McpToolKind");
    }
}

constexpr std::string_view to_string_view(openai::MouseButton val) {
    switch (val) {
        case openai::MouseButton::LEFT: return "left";
        case openai::MouseButton::RIGHT: return "right";
        case openai::MouseButton::WHEEL: return "wheel";
        case openai::MouseButton::BACK: return "back";
        case openai::MouseButton::FORWARD: return "forward";
        default: throw std::logic_error("invalid openai::MouseButton");
    }
}

constexpr std::string_view to_string_view(openai::MoveActionKind val) {
    switch (val) {
        case openai::MoveActionKind::MOVE: return "move";
        default: throw std::logic_error("invalid openai::MoveActionKind");
    }
}

constexpr std::string_view to_string_view(openai::OpenPageActionKind val) {
    switch (val) {
        case openai::OpenPageActionKind::OPEN_PAGE: return "open_page";
        default: throw std::logic_error("invalid openai::OpenPageActionKind");
    }
}

constexpr std::string_view to_string_view(openai::OutputMessageContentType val) {
    switch (val) {
        case openai::OutputMessageContentType::OUTPUT_TEXT: return "output_text";
        case openai::OutputMessageContentType::REFUSAL: return "refusal";
        default: throw std::logic_error("invalid openai::OutputMessageContentType");
    }
}

constexpr std::string_view to_string_view(openai::OutputMessageKind val) {
    switch (val) {
        case openai::OutputMessageKind::MESSAGE: return "message";
        default: throw std::logic_error("invalid openai::OutputMessageKind");
    }
}

constexpr std::string_view to_string_view(openai::OutputTextKind val) {
    switch (val) {
        case openai::OutputTextKind::OUTPUT_TEXT: return "output_text";
        default: throw std::logic_error("invalid openai::OutputTextKind");
    }
}

constexpr std::string_view to_string_view(openai::PendingSafetyCheckStatus val) {
    switch (val) {
        case openai::PendingSafetyCheckStatus::IN_PROGRESS: return "in_progress";
        case openai::PendingSafetyCheckStatus::COMPLETED: return "completed";
        case openai::PendingSafetyCheckStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::PendingSafetyCheckStatus");
    }
}

constexpr std::string_view to_string_view(openai::PromptCacheRetention val) {
    switch (val) {
        case openai::PromptCacheRetention::HOURS_24: return "24h";
        default: throw std::logic_error("invalid openai::PromptCacheRetention");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningEffort val) {
    switch (val) {
        case openai::ReasoningEffort::NONE: return "none";
        case openai::ReasoningEffort::MINIMAL: return "minimal";
        case openai::ReasoningEffort::LOW: return "low";
        case openai::ReasoningEffort::MEDIUM: return "medium";
        case openai::ReasoningEffort::HIGH: return "high";
        case openai::ReasoningEffort::XHIGH: return "xhigh";
        default: throw std::logic_error("invalid openai::ReasoningEffort");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningItemContentType val) {
    switch (val) {
        case openai::ReasoningItemContentType::SUMMARY_TEXT: return "summary_text";
        case openai::ReasoningItemContentType::REASONING_TEXT: return "reasoning_text";
        default: throw std::logic_error("invalid openai::ReasoningItemContentType");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningItemKind val) {
    switch (val) {
        case openai::ReasoningItemKind::REASONING: return "reasoning";
        default: throw std::logic_error("invalid openai::ReasoningItemKind");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningStatus val) {
    switch (val) {
        case openai::ReasoningStatus::IN_PROGRESS: return "in_progress";
        case openai::ReasoningStatus::COMPLETED: return "completed";
        case openai::ReasoningStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::ReasoningStatus");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningSummary val) {
    switch (val) {
        case openai::ReasoningSummary::AUTO: return "auto";
        case openai::ReasoningSummary::CONCISE: return "concise";
        case openai::ReasoningSummary::DETAILED: return "detailed";
        default: throw std::logic_error("invalid openai::ReasoningSummary");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningSummaryTextKind val) {
    switch (val) {
        case openai::ReasoningSummaryTextKind::SUMMARY_TEXT: return "summary_text";
        default: throw std::logic_error("invalid openai::ReasoningSummaryTextKind");
    }
}

constexpr std::string_view to_string_view(openai::ReasoningTextKind val) {
    switch (val) {
        case openai::ReasoningTextKind::REASONING_TEXT: return "reasoning_text";
        default: throw std::logic_error("invalid openai::ReasoningTextKind");
    }
}

constexpr std::string_view to_string_view(openai::RefusalKind val) {
    switch (val) {
        case openai::RefusalKind::REFUSAL: return "refusal";
        default: throw std::logic_error("invalid openai::RefusalKind");
    }
}

constexpr std::string_view to_string_view(openai::ResponseFormatType val) {
    switch (val) {
        case openai::ResponseFormatType::TEXT: return "text";
        case openai::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw std::logic_error("invalid openai::ResponseFormatType");
    }
}

constexpr std::string_view to_string_view(openai::ResponseKind val) {
    switch (val) {
        case openai::ResponseKind::RESPONSE: return "response";
        default: throw std::logic_error("invalid openai::ResponseKind");
    }
}

constexpr std::string_view to_string_view(openai::ResponseStatus val) {
    switch (val) {
        case openai::ResponseStatus::COMPLETED: return "completed";
        case openai::ResponseStatus::FAILED: return "failed";
        case openai::ResponseStatus::IN_PROGRESS: return "in_progress";
        case openai::ResponseStatus::CANCELLED: return "cancelled";
        case openai::ResponseStatus::QUEUED: return "queued";
        case openai::ResponseStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::ResponseStatus");
    }
}

constexpr std::string_view to_string_view(openai::Role val) {
    switch (val) {
        case openai::Role::USER: return "user";
        case openai::Role::ASSISTANT: return "assistant";
        case openai::Role::SYSTEM: return "system";
        case openai::Role::DEVELOPER: return "developer";
        default: throw std::logic_error("invalid openai::Role");
    }
}

constexpr std::string_view to_string_view(openai::ScreenshotActionKind val) {
    switch (val) {
        case openai::ScreenshotActionKind::SCREENSHOT: return "screenshot";
        default: throw std::logic_error("invalid openai::ScreenshotActionKind");
    }
}

constexpr std::string_view to_string_view(openai::ScrollActionKind val) {
    switch (val) {
        case openai::ScrollActionKind::SCROLL: return "scroll";
        default: throw std::logic_error("invalid openai::ScrollActionKind");
    }
}

constexpr std::string_view to_string_view(openai::SearchActionKind val) {
    switch (val) {
        case openai::SearchActionKind::SEARCH: return "search";
        default: throw std::logic_error("invalid openai::SearchActionKind");
    }
}

constexpr std::string_view to_string_view(openai::SearchContextSize val) {
    switch (val) {
        case openai::SearchContextSize::LOW: return "low";
        case openai::SearchContextSize::MEDIUM: return "medium";
        case openai::SearchContextSize::HIGH: return "high";
        default: throw std::logic_error("invalid openai::SearchContextSize");
    }
}

constexpr std::string_view to_string_view(openai::ServiceTier val) {
    switch (val) {
        case openai::ServiceTier::AUTO: return "auto";
        case openai::ServiceTier::DEFAULT: return "default";
        case openai::ServiceTier::FLEX: return "flex";
        case openai::ServiceTier::SCALE: return "scale";
        case openai::ServiceTier::PRIORITY: return "priority";
        default: throw std::logic_error("invalid openai::ServiceTier");
    }
}

constexpr std::string_view to_string_view(openai::ShellCallKind val) {
    switch (val) {
        case openai::ShellCallKind::SHELL_CALL: return "shell_call";
        default: throw std::logic_error("invalid openai::ShellCallKind");
    }
}

constexpr std::string_view to_string_view(openai::ShellCallOutcomeType val) {
    switch (val) {
        case openai::ShellCallOutcomeType::EXIT: return "exit";
        case openai::ShellCallOutcomeType::TIMEOUT: return "timeout";
        default: throw std::logic_error("invalid openai::ShellCallOutcomeType");
    }
}

constexpr std::string_view to_string_view(openai::ShellCallOutputKind val) {
    switch (val) {
        case openai::ShellCallOutputKind::SHELL_CALL_OUTPUT: return "shell_call_output";
        default: throw std::logic_error("invalid openai::ShellCallOutputKind");
    }
}

constexpr std::string_view to_string_view(openai::ShellExitOutcomeKind val) {
    switch (val) {
        case openai::ShellExitOutcomeKind::EXIT: return "exit";
        default: throw std::logic_error("invalid openai::ShellExitOutcomeKind");
    }
}

constexpr std::string_view to_string_view(openai::ShellTimeoutOutcomeKind val) {
    switch (val) {
        case openai::ShellTimeoutOutcomeKind::TIMEOUT: return "timeout";
        default: throw std::logic_error("invalid openai::ShellTimeoutOutcomeKind");
    }
}

constexpr std::string_view to_string_view(openai::ShellToolKind val) {
    switch (val) {
        case openai::ShellToolKind::SHELL: return "shell";
        default: throw std::logic_error("invalid openai::ShellToolKind");
    }
}

constexpr std::string_view to_string_view(openai::SpecificApplyPatchToolChoiceKind val) {
    switch (val) {
        case openai::SpecificApplyPatchToolChoiceKind::APPLY_PATCH: return "apply_patch";
        default: throw std::logic_error("invalid openai::SpecificApplyPatchToolChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::SpecificShellToolChoiceKind val) {
    switch (val) {
        case openai::SpecificShellToolChoiceKind::SHELL: return "shell";
        default: throw std::logic_error("invalid openai::SpecificShellToolChoiceKind");
    }
}

constexpr std::string_view to_string_view(openai::ToolChoiceMode val) {
    switch (val) {
        case openai::ToolChoiceMode::NONE: return "none";
        case openai::ToolChoiceMode::AUTO: return "auto";
        case openai::ToolChoiceMode::REQUIRED: return "required";
        default: throw std::logic_error("invalid openai::ToolChoiceMode");
    }
}

constexpr std::string_view to_string_view(openai::ToolChoiceType val) {
    switch (val) {
        case openai::ToolChoiceType::ALLOWED_TOOLS: return "allowed_tools";
        case openai::ToolChoiceType::FUNCTION: return "function";
        case openai::ToolChoiceType::MCP: return "mcp";
        case openai::ToolChoiceType::CUSTOM: return "custom";
        case openai::ToolChoiceType::APPLY_PATCH: return "apply_patch";
        case openai::ToolChoiceType::SHELL: return "shell";
        default: throw std::logic_error("invalid openai::ToolChoiceType");
    }
}

constexpr std::string_view to_string_view(openai::ToolType val) {
    switch (val) {
        case openai::ToolType::FUNCTION: return "function";
        case openai::ToolType::FILE_SEARCH: return "file_search";
        case openai::ToolType::COMPUTER_USE_PREVIEW: return "computer_use_preview";
        case openai::ToolType::WEB_SEARCH: return "web_search";
        case openai::ToolType::MCP: return "mcp";
        case openai::ToolType::CODE_INTERPRETER: return "code_interpreter";
        case openai::ToolType::IMAGE_GENERATION: return "image_generation";
        case openai::ToolType::LOCAL_SHELL: return "local_shell";
        case openai::ToolType::SHELL: return "shell";
        case openai::ToolType::CUSTOM: return "custom";
        case openai::ToolType::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::ToolType::APPLY_PATCH: return "apply_patch";
        default: throw std::logic_error("invalid openai::ToolType");
    }
}

constexpr std::string_view to_string_view(openai::TruncationStrategy val) {
    switch (val) {
        case openai::TruncationStrategy::AUTO: return "auto";
        case openai::TruncationStrategy::DISABLED: return "disabled";
        default: throw std::logic_error("invalid openai::TruncationStrategy");
    }
}

constexpr std::string_view to_string_view(openai::TypeActionKind val) {
    switch (val) {
        case openai::TypeActionKind::TYPE: return "type";
        default: throw std::logic_error("invalid openai::TypeActionKind");
    }
}

constexpr std::string_view to_string_view(openai::UpdateFileOperationKind val) {
    switch (val) {
        case openai::UpdateFileOperationKind::UPDATE_FILE: return "update_file";
        default: throw std::logic_error("invalid openai::UpdateFileOperationKind");
    }
}

constexpr std::string_view to_string_view(openai::UrlCitationKind val) {
    switch (val) {
        case openai::UrlCitationKind::URL_CITATION: return "url_citation";
        default: throw std::logic_error("invalid openai::UrlCitationKind");
    }
}

constexpr std::string_view to_string_view(openai::Verbosity val) {
    switch (val) {
        case openai::Verbosity::LOW: return "low";
        case openai::Verbosity::MEDIUM: return "medium";
        case openai::Verbosity::HIGH: return "high";
        default: throw std::logic_error("invalid openai::Verbosity");
    }
}

constexpr std::string_view to_string_view(openai::WaitActionKind val) {
    switch (val) {
        case openai::WaitActionKind::WAIT: return "wait";
        default: throw std::logic_error("invalid openai::WaitActionKind");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchActionType val) {
    switch (val) {
        case openai::WebSearchActionType::SEARCH: return "search";
        case openai::WebSearchActionType::OPEN_PAGE: return "open_page";
        case openai::WebSearchActionType::FIND: return "find";
        default: throw std::logic_error("invalid openai::WebSearchActionType");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchCallKind val) {
    switch (val) {
        case openai::WebSearchCallKind::WEB_SEARCH_CALL: return "web_search_call";
        default: throw std::logic_error("invalid openai::WebSearchCallKind");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchPreviewToolKind val) {
    switch (val) {
        case openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW_2025_03_11: return "web_search_preview_2025_03_11";
        default: throw std::logic_error("invalid openai::WebSearchPreviewToolKind");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchPreviewToolType val) {
    switch (val) {
        case openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW_2025_03_11: return "web_search_preview_2025_03_11";
        default: throw std::logic_error("invalid openai::WebSearchPreviewToolType");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchStatus val) {
    switch (val) {
        case openai::WebSearchStatus::IN_PROGRESS: return "in_progress";
        case openai::WebSearchStatus::COMPLETED: return "completed";
        case openai::WebSearchStatus::INCOMPLETE: return "incomplete";
        default: throw std::logic_error("invalid openai::WebSearchStatus");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchToolKind val) {
    switch (val) {
        case openai::WebSearchToolKind::WEB_SEARCH: return "web_search";
        case openai::WebSearchToolKind::WEB_SEARCH_2025_08_26: return "web_search_2025_08_26";
        default: throw std::logic_error("invalid openai::WebSearchToolKind");
    }
}

constexpr std::string_view to_string_view(openai::WebSearchToolType val) {
    switch (val) {
        case openai::WebSearchToolType::WEB_SEARCH: return "web_search";
        case openai::WebSearchToolType::WEB_SEARCH_2025_08_26: return "web_search_2025_08_26";
        default: throw std::logic_error("invalid openai::WebSearchToolType");
    }
}


}
