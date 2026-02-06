#pragma once

#include "../../core/strings_base.hpp"
#include "../../core/error.hpp"
#include "responses_enums.hpp"


namespace jai::llm {


/***
 * from_string_view Specializations
 */
// Kinds
template <>
constexpr std::optional<openai::KindAllowedToolsChoice> from_string_view<openai::KindAllowedToolsChoice>(std::string_view sv) {
    if (sv == openai::KindAllowedToolsChoice::value) return openai::KindAllowedToolsChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindApplyPatchCall> from_string_view<openai::KindApplyPatchCall>(std::string_view sv) {
    if (sv == openai::KindApplyPatchCall::value) return openai::KindApplyPatchCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindApplyPatchCallOutput> from_string_view<openai::KindApplyPatchCallOutput>(std::string_view sv) {
    if (sv == openai::KindApplyPatchCallOutput::value) return openai::KindApplyPatchCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindApplyPatchTool> from_string_view<openai::KindApplyPatchTool>(std::string_view sv) {
    if (sv == openai::KindApplyPatchTool::value) return openai::KindApplyPatchTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindClickAction> from_string_view<openai::KindClickAction>(std::string_view sv) {
    if (sv == openai::KindClickAction::value) return openai::KindClickAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCodeInterpreterCall> from_string_view<openai::KindCodeInterpreterCall>(std::string_view sv) {
    if (sv == openai::KindCodeInterpreterCall::value) return openai::KindCodeInterpreterCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCodeInterpreterTool> from_string_view<openai::KindCodeInterpreterTool>(std::string_view sv) {
    if (sv == openai::KindCodeInterpreterTool::value) return openai::KindCodeInterpreterTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCompactionItem> from_string_view<openai::KindCompactionItem>(std::string_view sv) {
    if (sv == openai::KindCompactionItem::value) return openai::KindCompactionItem{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindComputerCall> from_string_view<openai::KindComputerCall>(std::string_view sv) {
    if (sv == openai::KindComputerCall::value) return openai::KindComputerCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindComputerCallOutput> from_string_view<openai::KindComputerCallOutput>(std::string_view sv) {
    if (sv == openai::KindComputerCallOutput::value) return openai::KindComputerCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindComputerScreenshot> from_string_view<openai::KindComputerScreenshot>(std::string_view sv) {
    if (sv == openai::KindComputerScreenshot::value) return openai::KindComputerScreenshot{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindComputerUseTool> from_string_view<openai::KindComputerUseTool>(std::string_view sv) {
    if (sv == openai::KindComputerUseTool::value) return openai::KindComputerUseTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindContainerFileCitation> from_string_view<openai::KindContainerFileCitation>(std::string_view sv) {
    if (sv == openai::KindContainerFileCitation::value) return openai::KindContainerFileCitation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindContainerConfig> from_string_view<openai::KindContainerConfig>(std::string_view sv) {
    if (sv == openai::KindContainerConfig::value) return openai::KindContainerConfig{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCreateFileOperation> from_string_view<openai::KindCreateFileOperation>(std::string_view sv) {
    if (sv == openai::KindCreateFileOperation::value) return openai::KindCreateFileOperation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomToolCall> from_string_view<openai::KindCustomToolCall>(std::string_view sv) {
    if (sv == openai::KindCustomToolCall::value) return openai::KindCustomToolCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomToolCallOutput> from_string_view<openai::KindCustomToolCallOutput>(std::string_view sv) {
    if (sv == openai::KindCustomToolCallOutput::value) return openai::KindCustomToolCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomToolChoice> from_string_view<openai::KindCustomToolChoice>(std::string_view sv) {
    if (sv == openai::KindCustomToolChoice::value) return openai::KindCustomToolChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomToolGrammarFormat> from_string_view<openai::KindCustomToolGrammarFormat>(std::string_view sv) {
    if (sv == openai::KindCustomToolGrammarFormat::value) return openai::KindCustomToolGrammarFormat{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomTool> from_string_view<openai::KindCustomTool>(std::string_view sv) {
    if (sv == openai::KindCustomTool::value) return openai::KindCustomTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindCustomToolTextFormat> from_string_view<openai::KindCustomToolTextFormat>(std::string_view sv) {
    if (sv == openai::KindCustomToolTextFormat::value) return openai::KindCustomToolTextFormat{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindDeleteFileOperation> from_string_view<openai::KindDeleteFileOperation>(std::string_view sv) {
    if (sv == openai::KindDeleteFileOperation::value) return openai::KindDeleteFileOperation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindDoubleClickAction> from_string_view<openai::KindDoubleClickAction>(std::string_view sv) {
    if (sv == openai::KindDoubleClickAction::value) return openai::KindDoubleClickAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindDragAction> from_string_view<openai::KindDragAction>(std::string_view sv) {
    if (sv == openai::KindDragAction::value) return openai::KindDragAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFileCitation> from_string_view<openai::KindFileCitation>(std::string_view sv) {
    if (sv == openai::KindFileCitation::value) return openai::KindFileCitation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFilePath> from_string_view<openai::KindFilePath>(std::string_view sv) {
    if (sv == openai::KindFilePath::value) return openai::KindFilePath{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFileSearchToolCall> from_string_view<openai::KindFileSearchToolCall>(std::string_view sv) {
    if (sv == openai::KindFileSearchToolCall::value) return openai::KindFileSearchToolCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFileSearchTool> from_string_view<openai::KindFileSearchTool>(std::string_view sv) {
    if (sv == openai::KindFileSearchTool::value) return openai::KindFileSearchTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFindAction> from_string_view<openai::KindFindAction>(std::string_view sv) {
    if (sv == openai::KindFindAction::value) return openai::KindFindAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFormatText> from_string_view<openai::KindFormatText>(std::string_view sv) {
    if (sv == openai::KindFormatText::value) return openai::KindFormatText{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFormatJsonSchema> from_string_view<openai::KindFormatJsonSchema>(std::string_view sv) {
    if (sv == openai::KindFormatJsonSchema::value) return openai::KindFormatJsonSchema{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFunctionCall> from_string_view<openai::KindFunctionCall>(std::string_view sv) {
    if (sv == openai::KindFunctionCall::value) return openai::KindFunctionCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFunctionCallOutput> from_string_view<openai::KindFunctionCallOutput>(std::string_view sv) {
    if (sv == openai::KindFunctionCallOutput::value) return openai::KindFunctionCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFunctionToolChoice> from_string_view<openai::KindFunctionToolChoice>(std::string_view sv) {
    if (sv == openai::KindFunctionToolChoice::value) return openai::KindFunctionToolChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindFunctionTool> from_string_view<openai::KindFunctionTool>(std::string_view sv) {
    if (sv == openai::KindFunctionTool::value) return openai::KindFunctionTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindImageGenerationCall> from_string_view<openai::KindImageGenerationCall>(std::string_view sv) {
    if (sv == openai::KindImageGenerationCall::value) return openai::KindImageGenerationCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindImageGenerationTool> from_string_view<openai::KindImageGenerationTool>(std::string_view sv) {
    if (sv == openai::KindImageGenerationTool::value) return openai::KindImageGenerationTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindInputFile> from_string_view<openai::KindInputFile>(std::string_view sv) {
    if (sv == openai::KindInputFile::value) return openai::KindInputFile{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindInputImage> from_string_view<openai::KindInputImage>(std::string_view sv) {
    if (sv == openai::KindInputImage::value) return openai::KindInputImage{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindInputMessage> from_string_view<openai::KindInputMessage>(std::string_view sv) {
    if (sv == openai::KindInputMessage::value) return openai::KindInputMessage{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindInputText> from_string_view<openai::KindInputText>(std::string_view sv) {
    if (sv == openai::KindInputText::value) return openai::KindInputText{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindItemReference> from_string_view<openai::KindItemReference>(std::string_view sv) {
    if (sv == openai::KindItemReference::value) return openai::KindItemReference{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindKeyPressAction> from_string_view<openai::KindKeyPressAction>(std::string_view sv) {
    if (sv == openai::KindKeyPressAction::value) return openai::KindKeyPressAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindLocalShellAction> from_string_view<openai::KindLocalShellAction>(std::string_view sv) {
    if (sv == openai::KindLocalShellAction::value) return openai::KindLocalShellAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindLocalShellCall> from_string_view<openai::KindLocalShellCall>(std::string_view sv) {
    if (sv == openai::KindLocalShellCall::value) return openai::KindLocalShellCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindLocalShellCallOutput> from_string_view<openai::KindLocalShellCallOutput>(std::string_view sv) {
    if (sv == openai::KindLocalShellCallOutput::value) return openai::KindLocalShellCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindLocalShellTool> from_string_view<openai::KindLocalShellTool>(std::string_view sv) {
    if (sv == openai::KindLocalShellTool::value) return openai::KindLocalShellTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPApprovalRequest> from_string_view<openai::KindMCPApprovalRequest>(std::string_view sv) {
    if (sv == openai::KindMCPApprovalRequest::value) return openai::KindMCPApprovalRequest{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPApprovalResponse> from_string_view<openai::KindMCPApprovalResponse>(std::string_view sv) {
    if (sv == openai::KindMCPApprovalResponse::value) return openai::KindMCPApprovalResponse{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPCall> from_string_view<openai::KindMCPCall>(std::string_view sv) {
    if (sv == openai::KindMCPCall::value) return openai::KindMCPCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPListTools> from_string_view<openai::KindMCPListTools>(std::string_view sv) {
    if (sv == openai::KindMCPListTools::value) return openai::KindMCPListTools{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPToolChoice> from_string_view<openai::KindMCPToolChoice>(std::string_view sv) {
    if (sv == openai::KindMCPToolChoice::value) return openai::KindMCPToolChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMCPTool> from_string_view<openai::KindMCPTool>(std::string_view sv) {
    if (sv == openai::KindMCPTool::value) return openai::KindMCPTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindMoveAction> from_string_view<openai::KindMoveAction>(std::string_view sv) {
    if (sv == openai::KindMoveAction::value) return openai::KindMoveAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindOpenPageAction> from_string_view<openai::KindOpenPageAction>(std::string_view sv) {
    if (sv == openai::KindOpenPageAction::value) return openai::KindOpenPageAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindOutputMessage> from_string_view<openai::KindOutputMessage>(std::string_view sv) {
    if (sv == openai::KindOutputMessage::value) return openai::KindOutputMessage{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindOutputText> from_string_view<openai::KindOutputText>(std::string_view sv) {
    if (sv == openai::KindOutputText::value) return openai::KindOutputText{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindReasoningItem> from_string_view<openai::KindReasoningItem>(std::string_view sv) {
    if (sv == openai::KindReasoningItem::value) return openai::KindReasoningItem{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindReasoningSummaryText> from_string_view<openai::KindReasoningSummaryText>(std::string_view sv) {
    if (sv == openai::KindReasoningSummaryText::value) return openai::KindReasoningSummaryText{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindReasoningText> from_string_view<openai::KindReasoningText>(std::string_view sv) {
    if (sv == openai::KindReasoningText::value) return openai::KindReasoningText{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindRefusal> from_string_view<openai::KindRefusal>(std::string_view sv) {
    if (sv == openai::KindRefusal::value) return openai::KindRefusal{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindResponse> from_string_view<openai::KindResponse>(std::string_view sv) {
    if (sv == openai::KindResponse::value) return openai::KindResponse{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindScreenshotAction> from_string_view<openai::KindScreenshotAction>(std::string_view sv) {
    if (sv == openai::KindScreenshotAction::value) return openai::KindScreenshotAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindScrollAction> from_string_view<openai::KindScrollAction>(std::string_view sv) {
    if (sv == openai::KindScrollAction::value) return openai::KindScrollAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindSearchAction> from_string_view<openai::KindSearchAction>(std::string_view sv) {
    if (sv == openai::KindSearchAction::value) return openai::KindSearchAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindSearchActionSource> from_string_view<openai::KindSearchActionSource>(std::string_view sv) {
    if (sv == openai::KindSearchActionSource::value) return openai::KindSearchActionSource{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindShellCall> from_string_view<openai::KindShellCall>(std::string_view sv) {
    if (sv == openai::KindShellCall::value) return openai::KindShellCall{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindShellCallOutput> from_string_view<openai::KindShellCallOutput>(std::string_view sv) {
    if (sv == openai::KindShellCallOutput::value) return openai::KindShellCallOutput{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindShellExitOutcome> from_string_view<openai::KindShellExitOutcome>(std::string_view sv) {
    if (sv == openai::KindShellExitOutcome::value) return openai::KindShellExitOutcome{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindShellTimeoutOutcome> from_string_view<openai::KindShellTimeoutOutcome>(std::string_view sv) {
    if (sv == openai::KindShellTimeoutOutcome::value) return openai::KindShellTimeoutOutcome{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindShellTool> from_string_view<openai::KindShellTool>(std::string_view sv) {
    if (sv == openai::KindShellTool::value) return openai::KindShellTool{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindSpecificApplyPatchToolChoice> from_string_view<openai::KindSpecificApplyPatchToolChoice>(std::string_view sv) {
    if (sv == openai::KindSpecificApplyPatchToolChoice::value) return openai::KindSpecificApplyPatchToolChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindSpecificShellToolChoice> from_string_view<openai::KindSpecificShellToolChoice>(std::string_view sv) {
    if (sv == openai::KindSpecificShellToolChoice::value) return openai::KindSpecificShellToolChoice{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindTypeAction> from_string_view<openai::KindTypeAction>(std::string_view sv) {
    if (sv == openai::KindTypeAction::value) return openai::KindTypeAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindUpdateFileOperation> from_string_view<openai::KindUpdateFileOperation>(std::string_view sv) {
    if (sv == openai::KindUpdateFileOperation::value) return openai::KindUpdateFileOperation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindUrlCitation> from_string_view<openai::KindUrlCitation>(std::string_view sv) {
    if (sv == openai::KindUrlCitation::value) return openai::KindUrlCitation{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindWaitAction> from_string_view<openai::KindWaitAction>(std::string_view sv) {
    if (sv == openai::KindWaitAction::value) return openai::KindWaitAction{};
    return std::nullopt;
}

template <>
constexpr std::optional<openai::KindWebSearchCall> from_string_view<openai::KindWebSearchCall>(std::string_view sv) {
    if (sv == openai::KindWebSearchCall::value) return openai::KindWebSearchCall{};
    return std::nullopt;
}


// Enums
template <>
constexpr std::optional<openai::AnnotationType> from_string_view<openai::AnnotationType>(std::string_view sv) {
    if (sv == "file_citation") return openai::AnnotationType::FILE_CITATION;
    if (sv == "url_citation") return openai::AnnotationType::URL_CITATION;
    if (sv == "container_file_citation") return openai::AnnotationType::CONTAINER_FILE_CITATION;
    if (sv == "file_path") return openai::AnnotationType::FILE_PATH;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ContentType> from_string_view<openai::ContentType>(std::string_view sv) {
    if (sv == "input_text") return openai::ContentType::INPUT_TEXT;
    if (sv == "input_image") return openai::ContentType::INPUT_IMAGE;
    if (sv == "input_file") return openai::ContentType::INPUT_FILE;
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
constexpr std::optional<openai::CallStatus> from_string_view<openai::CallStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::CallStatus::IN_PROGRESS;
    if (sv == "completed") return openai::CallStatus::COMPLETED;
    if (sv == "incomplete") return openai::CallStatus::INCOMPLETE;
    if (sv == "calling") return openai::CallStatus::CALLING;
    if (sv == "failed") return openai::CallStatus::FAILED;
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
constexpr std::optional<openai::ComputerCallOutputType> from_string_view<openai::ComputerCallOutputType>(std::string_view sv) {
    if (sv == "computer_screenshot") return openai::ComputerCallOutputType::COMPUTER_SCREENSHOT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ConnectId> from_string_view<openai::ConnectId>(std::string_view sv) {
    if (sv == "connector_dropbox") return openai::ConnectId::DROPBOX;
    if (sv == "connector_gmail") return openai::ConnectId::GMAIL;
    if (sv == "connector_googlecalendar") return openai::ConnectId::GOOGLE_CALENDAR;
    if (sv == "connector_googledrive") return openai::ConnectId::GOOGLE_DRIVE;
    if (sv == "connector_microsoftteams") return openai::ConnectId::MICROSOFT_TEAMS;
    if (sv == "connector_outlookcalendar") return openai::ConnectId::OUTLOOK_CALENDAR;
    if (sv == "connector_outlookemail") return openai::ConnectId::OUTLOOK_EMAIL;
    if (sv == "connector_sharepoint") return openai::ConnectId::SHAREPOINT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CustomToolFormatType> from_string_view<openai::CustomToolFormatType>(std::string_view sv) {
    if (sv == "text") return openai::CustomToolFormatType::TEXT;
    if (sv == "grammar") return openai::CustomToolFormatType::GRAMMAR;
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
constexpr std::optional<openai::FileSearchStatus> from_string_view<openai::FileSearchStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::FileSearchStatus::IN_PROGRESS;
    if (sv == "searching") return openai::FileSearchStatus::SEARCHING;
    if (sv == "incomplete") return openai::FileSearchStatus::INCOMPLETE;
    if (sv == "failed") return openai::FileSearchStatus::FAILED;
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
constexpr std::optional<openai::FunctionCallStatus> from_string_view<openai::FunctionCallStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::FunctionCallStatus::IN_PROGRESS;
    if (sv == "completed") return openai::FunctionCallStatus::COMPLETED;
    if (sv == "incomplete") return openai::FunctionCallStatus::INCOMPLETE;
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
constexpr std::optional<openai::IncludeOutputData> from_string_view<openai::IncludeOutputData>(std::string_view sv) {
    if (sv == "code_interpreter_call.outputs") return openai::IncludeOutputData::CODE_INTERPRETER_CALL_OUTPUTS;
    if (sv == "computer_call_output.output.image_url") return openai::IncludeOutputData::COMPUTER_CALL_OUTPUT_OUTPUT_IMAGE_URL;
    if (sv == "file_search_call.results") return openai::IncludeOutputData::FILE_SEARCH_CALL_RESULTS;
    if (sv == "message.input_image.image_url") return openai::IncludeOutputData::MESSAGE_INPUT_IMAGE_IMAGE_URL;
    if (sv == "message.output_text.logprobs") return openai::IncludeOutputData::MESSAGE_OUTPUT_TEXT_LOGPROBS;
    if (sv == "reasoning.encrypted_content") return openai::IncludeOutputData::REASONING_ENCRYPTED_CONTENT;
    if (sv == "web_search_call.action.sources") return openai::IncludeOutputData::WEB_SEARCH_CALL_ACTION_SOURCES;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::IncompleteReason> from_string_view<openai::IncompleteReason>(std::string_view sv) {
    if (sv == "max_output_tokens") return openai::IncompleteReason::MAX_OUTPUT_TOKENS;
    if (sv == "content_filter") return openai::IncompleteReason::CONTENT_FILTER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::InputItemType> from_string_view<openai::InputItemType>(std::string_view sv) {
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
constexpr std::optional<openai::ItemStatus> from_string_view<openai::ItemStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::ItemStatus::IN_PROGRESS;
    if (sv == "completed") return openai::ItemStatus::COMPLETED;
    if (sv == "incomplete") return openai::ItemStatus::INCOMPLETE;
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
constexpr std::optional<openai::MCPApprovalSetting> from_string_view<openai::MCPApprovalSetting>(std::string_view sv) {
    if (sv == "always") return openai::MCPApprovalSetting::ALWAYS;
    if (sv == "never") return openai::MCPApprovalSetting::NEVER;
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
constexpr std::optional<openai::OutputItemType> from_string_view<openai::OutputItemType>(std::string_view sv) {
    if (sv == "message") return openai::OutputItemType::MESSAGE;
    if (sv == "reasoning") return openai::OutputItemType::REASONING;
    if (sv == "compaction") return openai::OutputItemType::COMPACTION;
    if (sv == "file_search_call") return openai::OutputItemType::FILE_SEARCH_CALL;
    if (sv == "computer_call") return openai::OutputItemType::COMPUTER_CALL;
    if (sv == "web_search_call") return openai::OutputItemType::WEB_SEARCH_CALL;
    if (sv == "function_call") return openai::OutputItemType::FUNCTION_CALL;
    if (sv == "image_generation_call") return openai::OutputItemType::IMAGE_GENERATION_CALL;
    if (sv == "code_interpreter_call") return openai::OutputItemType::CODE_INTERPRETER_CALL;
    if (sv == "local_shell_call") return openai::OutputItemType::LOCAL_SHELL_CALL;
    if (sv == "shell_call") return openai::OutputItemType::SHELL_CALL;
    if (sv == "shell_call_output") return openai::OutputItemType::SHELL_CALL_OUTPUT;
    if (sv == "apply_patch_call") return openai::OutputItemType::APPLY_PATCH_CALL;
    if (sv == "apply_patch_call_output") return openai::OutputItemType::APPLY_PATCH_CALL_OUTPUT;
    if (sv == "mcp_list_tools") return openai::OutputItemType::MCP_LIST_TOOLS;
    if (sv == "mcp_approval_request") return openai::OutputItemType::MCP_APPROVAL_REQUEST;
    if (sv == "mcp_call") return openai::OutputItemType::MCP_CALL;
    if (sv == "custom_tool_call") return openai::OutputItemType::CUSTOM_TOOL_CALL;
    if (sv == "item_reference") return openai::OutputItemType::ITEM_REFERENCE;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::OutputMessageContentType> from_string_view<openai::OutputMessageContentType>(std::string_view sv) {
    if (sv == "output_text") return openai::OutputMessageContentType::OUTPUT_TEXT;
    if (sv == "refusal") return openai::OutputMessageContentType::REFUSAL;
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
constexpr std::optional<openai::ResponseFormatType> from_string_view<openai::ResponseFormatType>(std::string_view sv) {
    if (sv == "text") return openai::ResponseFormatType::TEXT;
    if (sv == "json_schema") return openai::ResponseFormatType::JSON_SCHEMA;
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
constexpr std::optional<openai::RoleAssistant> from_string_view<openai::RoleAssistant>(std::string_view sv) {
    if (sv == "assistant") return openai::RoleAssistant::ASSISTANT;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::RoleInputMessage> from_string_view<openai::RoleInputMessage>(std::string_view sv) {
    if (sv == "user") return openai::RoleInputMessage::USER;
    if (sv == "assistant") return openai::RoleInputMessage::ASSISTANT;
    if (sv == "system") return openai::RoleInputMessage::SYSTEM;
    if (sv == "developer") return openai::RoleInputMessage::DEVELOPER;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::RoleUser> from_string_view<openai::RoleUser>(std::string_view sv) {
    if (sv == "user") return openai::RoleUser::USER;
    if (sv == "system") return openai::RoleUser::SYSTEM;
    if (sv == "developer") return openai::RoleUser::DEVELOPER;
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
    if (sv == "priority") return openai::ServiceTier::PRIORITY;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ShellCallOutcomeType> from_string_view<openai::ShellCallOutcomeType>(std::string_view sv) {
    if (sv == "exit") return openai::ShellCallOutcomeType::EXIT;
    if (sv == "timeout") return openai::ShellCallOutcomeType::TIMEOUT;
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
constexpr std::optional<openai::ToolChoiceModeNotNone> from_string_view<openai::ToolChoiceModeNotNone>(std::string_view sv) {
    if (sv == "auto") return openai::ToolChoiceModeNotNone::AUTO;
    if (sv == "required") return openai::ToolChoiceModeNotNone::REQUIRED;
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
constexpr std::optional<openai::Verbosity> from_string_view<openai::Verbosity>(std::string_view sv) {
    if (sv == "low") return openai::Verbosity::LOW;
    if (sv == "medium") return openai::Verbosity::MEDIUM;
    if (sv == "high") return openai::Verbosity::HIGH;
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
constexpr std::string_view to_string_view(openai::AnnotationType val) {
    switch (val) {
        case openai::AnnotationType::FILE_CITATION: return "file_citation";
        case openai::AnnotationType::URL_CITATION: return "url_citation";
        case openai::AnnotationType::CONTAINER_FILE_CITATION: return "container_file_citation";
        case openai::AnnotationType::FILE_PATH: return "file_path";
        default: throw AnnotatedException{"invalid openai::AnnotationType"};
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallOutputStatus val) {
    switch (val) {
        case openai::ApplyPatchCallOutputStatus::COMPLETED: return "completed";
        case openai::ApplyPatchCallOutputStatus::FAILED: return "failed";
        default: throw AnnotatedException{"invalid openai::ApplyPatchCallOutputStatus"};
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchCallStatus val) {
    switch (val) {
        case openai::ApplyPatchCallStatus::IN_PROGRESS: return "in_progress";
        case openai::ApplyPatchCallStatus::COMPLETED: return "completed";
        default: throw AnnotatedException{"invalid openai::ApplyPatchCallStatus"};
    }
}

constexpr std::string_view to_string_view(openai::ApplyPatchOperationType val) {
    switch (val) {
        case openai::ApplyPatchOperationType::CREATE_FILE: return "create_file";
        case openai::ApplyPatchOperationType::DELETE_FILE: return "delete_file";
        case openai::ApplyPatchOperationType::UPDATE_FILE: return "update_file";
        default: throw AnnotatedException{"invalid openai::ApplyPatchOperationType"};
    }
}

constexpr std::string_view to_string_view(openai::CallStatus val) {
    switch (val) {
        case openai::CallStatus::IN_PROGRESS: return "in_progress";
        case openai::CallStatus::COMPLETED: return "completed";
        case openai::CallStatus::INCOMPLETE: return "incomplete";
        case openai::CallStatus::CALLING: return "calling";
        case openai::CallStatus::FAILED: return "failed";
        default: throw AnnotatedException{"invalid openai::CallStatus"};
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterOutputType val) {
    switch (val) {
        case openai::CodeInterpreterOutputType::LOGS: return "logs";
        case openai::CodeInterpreterOutputType::IMAGE: return "image";
        default: throw AnnotatedException{"invalid openai::CodeInterpreterOutputType"};
    }
}

constexpr std::string_view to_string_view(openai::CodeInterpreterStatus val) {
    switch (val) {
        case openai::CodeInterpreterStatus::IN_PROGRESS: return "in_progress";
        case openai::CodeInterpreterStatus::COMPLETED: return "completed";
        case openai::CodeInterpreterStatus::INCOMPLETE: return "incomplete";
        case openai::CodeInterpreterStatus::INTERPRETING: return "interpreting";
        case openai::CodeInterpreterStatus::FAILED: return "failed";
        default: throw AnnotatedException{"invalid openai::CodeInterpreterStatus"};
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
        default: throw AnnotatedException{"invalid openai::ComputerActionType"};
    }
}

constexpr std::string_view to_string_view(openai::ComputerCallOutputType val) {
    switch (val) {
        case openai::ComputerCallOutputType::COMPUTER_SCREENSHOT: return "computer_screenshot";
        default: throw AnnotatedException{"invalid openai::ComputerCallOutputType"};
    }
}

constexpr std::string_view to_string_view(openai::ConnectId val) {
    switch (val) {
        case openai::ConnectId::DROPBOX: return "dropbox";
        case openai::ConnectId::GMAIL: return "gmail";
        case openai::ConnectId::GOOGLE_CALENDAR: return "google_calendar";
        case openai::ConnectId::GOOGLE_DRIVE: return "google_drive";
        case openai::ConnectId::MICROSOFT_TEAMS: return "microsoft_teams";
        case openai::ConnectId::OUTLOOK_CALENDAR: return "outlook_calendar";
        case openai::ConnectId::OUTLOOK_EMAIL: return "outlook_email";
        case openai::ConnectId::SHAREPOINT: return "sharepoint";
        default: throw AnnotatedException{"invalid openai::ConnectId"};
    }
}

constexpr std::string_view to_string_view(openai::CustomToolFormatType val) {
    switch (val) {
        case openai::CustomToolFormatType::TEXT: return "text";
        case openai::CustomToolFormatType::GRAMMAR: return "grammar";
        default: throw AnnotatedException{"invalid openai::CustomToolFormatType"};
    }
}

constexpr std::string_view to_string_view(openai::Detail val) {
    switch (val) {
        case openai::Detail::HIGH: return "high";
        case openai::Detail::LOW: return "low";
        case openai::Detail::AUTO: return "auto";
        default: throw AnnotatedException{"invalid openai::Detail"};
    }
}

constexpr std::string_view to_string_view(openai::FileSearchStatus val) {
    switch (val) {
        case openai::FileSearchStatus::IN_PROGRESS: return "in_progress";
        case openai::FileSearchStatus::SEARCHING: return "searching";
        case openai::FileSearchStatus::INCOMPLETE: return "incomplete";
        case openai::FileSearchStatus::FAILED: return "failed";
        default: throw AnnotatedException{"invalid openai::FileSearchStatus"};
    }
}

constexpr std::string_view to_string_view(openai::FilterCompoundType val) {
    switch (val) {
        case openai::FilterCompoundType::AND: return "and";
        case openai::FilterCompoundType::OR: return "or";
        default: throw AnnotatedException{"invalid openai::FilterCompoundType"};
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
        default: throw AnnotatedException{"invalid openai::FilterOperator"};
    }
}

constexpr std::string_view to_string_view(openai::FunctionCallStatus val) {
    switch (val) {
        case openai::FunctionCallStatus::IN_PROGRESS: return "in_progress";
        case openai::FunctionCallStatus::COMPLETED: return "completed";
        case openai::FunctionCallStatus::INCOMPLETE: return "incomplete";
        default: throw AnnotatedException{"invalid openai::FunctionCallStatus"};
    }
}

constexpr std::string_view to_string_view(openai::GrammarSyntax val) {
    switch (val) {
        case openai::GrammarSyntax::LARK: return "lark";
        case openai::GrammarSyntax::REGEX: return "regex";
        default: throw AnnotatedException{"invalid openai::GrammarSyntax"};
    }
}

constexpr std::string_view to_string_view(openai::HostedToolMode val) {
    switch (val) {
        case openai::HostedToolMode::FILE_SEARCH: return "file_search";
        case openai::HostedToolMode::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::HostedToolMode::COMPUTER_USE_PREVIEW: return "computer_use_preview";
        case openai::HostedToolMode::CODE_INTERPRETER: return "code_interpreter";
        case openai::HostedToolMode::IMAGE_GENERATION: return "image_generation";
        default: throw AnnotatedException{"invalid openai::HostedToolMode"};
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationBackground val) {
    switch (val) {
        case openai::ImageGenerationBackground::TRANSPARENT: return "transparent";
        case openai::ImageGenerationBackground::OPAQUE: return "opaque";
        case openai::ImageGenerationBackground::AUTO: return "auto";
        default: throw AnnotatedException{"invalid openai::ImageGenerationBackground"};
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationFidelity val) {
    switch (val) {
        case openai::ImageGenerationFidelity::HIGH: return "high";
        case openai::ImageGenerationFidelity::LOW: return "low";
        default: throw AnnotatedException{"invalid openai::ImageGenerationFidelity"};
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationFormat val) {
    switch (val) {
        case openai::ImageGenerationFormat::PNG: return "png";
        case openai::ImageGenerationFormat::WEBP: return "webp";
        case openai::ImageGenerationFormat::JPEG: return "jpeg";
        default: throw AnnotatedException{"invalid openai::ImageGenerationFormat"};
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationQuality val) {
    switch (val) {
        case openai::ImageGenerationQuality::LOW: return "low";
        case openai::ImageGenerationQuality::MEDIUM: return "medium";
        case openai::ImageGenerationQuality::HIGH: return "high";
        case openai::ImageGenerationQuality::AUTO: return "auto";
        default: throw AnnotatedException{"invalid openai::ImageGenerationQuality"};
    }
}

constexpr std::string_view to_string_view(openai::ImageGenerationSize val) {
    switch (val) {
        case openai::ImageGenerationSize::SIZE_1024_1024: return "1024x1024";
        case openai::ImageGenerationSize::SIZE_1024_1536: return "1024x1536";
        case openai::ImageGenerationSize::SIZE_1536_1024: return "1536x1024";
        case openai::ImageGenerationSize::AUTO: return "auto";
        default: throw AnnotatedException{"invalid openai::ImageGenerationSize"};
    }
}

constexpr std::string_view to_string_view(openai::IncludeOutputData val) {
    switch (val) {
        case openai::IncludeOutputData::CODE_INTERPRETER_CALL_OUTPUTS: return "code_interpreter_call.outputs";
        case openai::IncludeOutputData::COMPUTER_CALL_OUTPUT_OUTPUT_IMAGE_URL: return "computer_call_output.output.image_url";
        case openai::IncludeOutputData::FILE_SEARCH_CALL_RESULTS: return "file_search_call.results";
        case openai::IncludeOutputData::MESSAGE_INPUT_IMAGE_IMAGE_URL: return "message.input_image.image_url";
        case openai::IncludeOutputData::MESSAGE_OUTPUT_TEXT_LOGPROBS: return "message.output_text.logprobs";
        case openai::IncludeOutputData::REASONING_ENCRYPTED_CONTENT: return "reasoning.encrypted_content";
        case openai::IncludeOutputData::WEB_SEARCH_CALL_ACTION_SOURCES: return "web_search_call.action.sources";
        default: throw AnnotatedException{"invalid openai::IncludeOutputData"};
    }
}

constexpr std::string_view to_string_view(openai::IncompleteReason val) {
    switch (val) {
        case openai::IncompleteReason::MAX_OUTPUT_TOKENS: return "max_output_tokens";
        case openai::IncompleteReason::CONTENT_FILTER: return "content_filter";
        default: throw AnnotatedException{"invalid openai::IncompleteReason"};
    }
}

constexpr std::string_view to_string_view(openai::ContentType val) {
    switch (val) {
        case openai::ContentType::INPUT_TEXT: return "input_text";
        case openai::ContentType::INPUT_IMAGE: return "input_image";
        case openai::ContentType::INPUT_FILE: return "input_file";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::InputItemType val) {
    switch (val) {
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
        default: throw AnnotatedException{"invalid openai::InputItemType"};
    }
}

constexpr std::string_view to_string_view(openai::ItemStatus val) {
    switch (val) {
        case openai::ItemStatus::IN_PROGRESS: return "in_progress";
        case openai::ItemStatus::COMPLETED: return "completed";
        case openai::ItemStatus::INCOMPLETE: return "incomplete";
        default: throw AnnotatedException{"invalid openai::ItemStatus"};
    }
}

constexpr std::string_view to_string_view(openai::LocalShellActionType val) {
    switch (val) {
        case openai::LocalShellActionType::EXEC: return "exec";
        default: throw AnnotatedException{"invalid openai::LocalShellActionType"};
    }
}

constexpr std::string_view to_string_view(openai::LocationType val) {
    switch (val) {
        case openai::LocationType::APPROXIMATE: return "approximate";
        default: throw AnnotatedException{"invalid openai::LocationType"};
    }
}

constexpr std::string_view to_string_view(openai::MCPApprovalSetting val) {
    switch (val) {
        case openai::MCPApprovalSetting::ALWAYS: return "always";
        case openai::MCPApprovalSetting::NEVER: return "never";
        default: throw AnnotatedException{"invalid openai::MCPApprovalSetting"};
    }
}

constexpr std::string_view to_string_view(openai::MouseButton val) {
    switch (val) {
        case openai::MouseButton::LEFT: return "left";
        case openai::MouseButton::RIGHT: return "right";
        case openai::MouseButton::WHEEL: return "wheel";
        case openai::MouseButton::BACK: return "back";
        case openai::MouseButton::FORWARD: return "forward";
        default: throw AnnotatedException{"invalid openai::MouseButton"};
    }
}

constexpr std::string_view to_string_view(openai::OutputItemType val) {
    switch (val) {
        case openai::OutputItemType::MESSAGE: return "message";
        case openai::OutputItemType::REASONING: return "reasoning";
        case openai::OutputItemType::COMPACTION: return "compaction";
        case openai::OutputItemType::FILE_SEARCH_CALL: return "file_search_call";
        case openai::OutputItemType::COMPUTER_CALL: return "computer_call";
        case openai::OutputItemType::WEB_SEARCH_CALL: return "web_search_call";
        case openai::OutputItemType::FUNCTION_CALL: return "function_call";
        case openai::OutputItemType::IMAGE_GENERATION_CALL: return "image_generation_call";
        case openai::OutputItemType::CODE_INTERPRETER_CALL: return "code_interpreter_call";
        case openai::OutputItemType::LOCAL_SHELL_CALL: return "local_shell_call";
        case openai::OutputItemType::SHELL_CALL: return "shell_call";
        case openai::OutputItemType::SHELL_CALL_OUTPUT: return "shell_call_output";
        case openai::OutputItemType::APPLY_PATCH_CALL: return "apply_patch_call";
        case openai::OutputItemType::APPLY_PATCH_CALL_OUTPUT: return "apply_patch_call_output";
        case openai::OutputItemType::MCP_LIST_TOOLS: return "mcp_list_tools";
        case openai::OutputItemType::MCP_APPROVAL_REQUEST: return "mcp_approval_request";
        case openai::OutputItemType::MCP_CALL: return "mcp_call";
        case openai::OutputItemType::CUSTOM_TOOL_CALL: return "custom_tool_call";
        case openai::OutputItemType::ITEM_REFERENCE: return "item_reference";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::OutputMessageContentType val) {
    switch (val) {
        case openai::OutputMessageContentType::OUTPUT_TEXT: return "output_text";
        case openai::OutputMessageContentType::REFUSAL: return "refusal";
        default: throw AnnotatedException{"invalid openai::OutputMessageContentType"};
    }
}

constexpr std::string_view to_string_view(openai::PendingSafetyCheckStatus val) {
    switch (val) {
        case openai::PendingSafetyCheckStatus::IN_PROGRESS: return "in_progress";
        case openai::PendingSafetyCheckStatus::COMPLETED: return "completed";
        case openai::PendingSafetyCheckStatus::INCOMPLETE: return "incomplete";
        default: throw AnnotatedException{"invalid openai::PendingSafetyCheckStatus"};
    }
}

constexpr std::string_view to_string_view(openai::PromptCacheRetention val) {
    switch (val) {
        case openai::PromptCacheRetention::HOURS_24: return "24h";
        default: throw AnnotatedException{"invalid openai::PromptCacheRetention"};
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
        default: throw AnnotatedException{"invalid openai::ReasoningEffort"};
    }
}

constexpr std::string_view to_string_view(openai::ReasoningItemContentType val) {
    switch (val) {
        case openai::ReasoningItemContentType::SUMMARY_TEXT: return "summary_text";
        case openai::ReasoningItemContentType::REASONING_TEXT: return "reasoning_text";
        default: throw AnnotatedException{"invalid openai::ReasoningItemContentType"};
    }
}

constexpr std::string_view to_string_view(openai::ReasoningStatus val) {
    switch (val) {
        case openai::ReasoningStatus::IN_PROGRESS: return "in_progress";
        case openai::ReasoningStatus::COMPLETED: return "completed";
        case openai::ReasoningStatus::INCOMPLETE: return "incomplete";
        default: throw AnnotatedException{"invalid openai::ReasoningStatus"};
    }
}

constexpr std::string_view to_string_view(openai::ReasoningSummary val) {
    switch (val) {
        case openai::ReasoningSummary::AUTO: return "auto";
        case openai::ReasoningSummary::CONCISE: return "concise";
        case openai::ReasoningSummary::DETAILED: return "detailed";
        default: throw AnnotatedException{"invalid openai::ReasoningSummary"};
    }
}

constexpr std::string_view to_string_view(openai::ResponseFormatType val) {
    switch (val) {
        case openai::ResponseFormatType::TEXT: return "text";
        case openai::ResponseFormatType::JSON_SCHEMA: return "json_schema";
        default: throw AnnotatedException{"invalid openai::ResponseFormatType"};
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
        default: throw AnnotatedException{"invalid openai::ResponseStatus"};
    }
}

constexpr std::string_view to_string_view(openai::RoleAssistant val) {
    switch (val) {
        case openai::RoleAssistant::ASSISTANT: return "assistant";
        default: throw AnnotatedException{"invalid openai::RoleAssistant"};
    }
}

constexpr std::string_view to_string_view(openai::RoleInputMessage val) {
    switch (val) {
        case openai::RoleInputMessage::USER: return "user";
        case openai::RoleInputMessage::ASSISTANT: return "assistant";
        case openai::RoleInputMessage::SYSTEM: return "system";
        case openai::RoleInputMessage::DEVELOPER: return "developer";
        default: throw AnnotatedException{"invalid openai::RoleInputMessage"};
    }
}

constexpr std::string_view to_string_view(openai::RoleUser val) {
    switch (val) {
        case openai::RoleUser::USER: return "user";
        case openai::RoleUser::SYSTEM: return "system";
        case openai::RoleUser::DEVELOPER: return "developer";
        default: throw AnnotatedException{"invalid openai::RoleUser"};
    }
}

constexpr std::string_view to_string_view(openai::SearchContextSize val) {
    switch (val) {
        case openai::SearchContextSize::LOW: return "low";
        case openai::SearchContextSize::MEDIUM: return "medium";
        case openai::SearchContextSize::HIGH: return "high";
        default: throw AnnotatedException{"invalid openai::SearchContextSize"};
    }
}

constexpr std::string_view to_string_view(openai::ServiceTier val) {
    switch (val) {
        case openai::ServiceTier::AUTO: return "auto";
        case openai::ServiceTier::DEFAULT: return "default";
        case openai::ServiceTier::FLEX: return "flex";
        case openai::ServiceTier::PRIORITY: return "priority";
        default: throw AnnotatedException{"invalid openai::ServiceTier"};
    }
}

constexpr std::string_view to_string_view(openai::ShellCallOutcomeType val) {
    switch (val) {
        case openai::ShellCallOutcomeType::EXIT: return "exit";
        case openai::ShellCallOutcomeType::TIMEOUT: return "timeout";
        default: throw AnnotatedException{"invalid openai::ShellCallOutcomeType"};
    }
}

constexpr std::string_view to_string_view(openai::ToolChoiceMode val) {
    switch (val) {
        case openai::ToolChoiceMode::NONE: return "none";
        case openai::ToolChoiceMode::AUTO: return "auto";
        case openai::ToolChoiceMode::REQUIRED: return "required";
        default: throw AnnotatedException{"invalid openai::ToolChoiceMode"};
    }
}

constexpr std::string_view to_string_view(openai::ToolChoiceModeNotNone val) {
    switch (val) {
        case openai::ToolChoiceModeNotNone::AUTO: return "auto";
        case openai::ToolChoiceModeNotNone::REQUIRED: return "required";
        default: throw AnnotatedException{"invalid openai::ToolChoiceModeNotNone"};
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
        default: throw AnnotatedException{"invalid openai::ToolChoiceType"};
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
        default: throw AnnotatedException{"invalid openai::ToolType"};
    }
}

constexpr std::string_view to_string_view(openai::TruncationStrategy val) {
    switch (val) {
        case openai::TruncationStrategy::AUTO: return "auto";
        case openai::TruncationStrategy::DISABLED: return "disabled";
        default: throw AnnotatedException{"invalid openai::TruncationStrategy"};
    }
}

constexpr std::string_view to_string_view(openai::Verbosity val) {
    switch (val) {
        case openai::Verbosity::LOW: return "low";
        case openai::Verbosity::MEDIUM: return "medium";
        case openai::Verbosity::HIGH: return "high";
        default: throw AnnotatedException{"invalid openai::Verbosity"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchActionType val) {
    switch (val) {
        case openai::WebSearchActionType::SEARCH: return "search";
        case openai::WebSearchActionType::OPEN_PAGE: return "open_page";
        case openai::WebSearchActionType::FIND: return "find";
        default: throw AnnotatedException{"invalid openai::WebSearchActionType"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchPreviewToolKind val) {
    switch (val) {
        case openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::WebSearchPreviewToolKind::WEB_SEARCH_PREVIEW_2025_03_11: return "web_search_preview_2025_03_11";
        default: throw AnnotatedException{"invalid openai::WebSearchPreviewToolKind"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchPreviewToolType val) {
    switch (val) {
        case openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW: return "web_search_preview";
        case openai::WebSearchPreviewToolType::WEB_SEARCH_PREVIEW_2025_03_11: return "web_search_preview_2025_03_11";
        default: throw AnnotatedException{"invalid openai::WebSearchPreviewToolType"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchStatus val) {
    switch (val) {
        case openai::WebSearchStatus::IN_PROGRESS: return "in_progress";
        case openai::WebSearchStatus::COMPLETED: return "completed";
        case openai::WebSearchStatus::INCOMPLETE: return "incomplete";
        default: throw AnnotatedException{"invalid openai::WebSearchStatus"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchToolKind val) {
    switch (val) {
        case openai::WebSearchToolKind::WEB_SEARCH: return "web_search";
        case openai::WebSearchToolKind::WEB_SEARCH_2025_08_26: return "web_search_2025_08_26";
        default: throw AnnotatedException{"invalid openai::WebSearchToolKind"};
    }
}

constexpr std::string_view to_string_view(openai::WebSearchToolType val) {
    switch (val) {
        case openai::WebSearchToolType::WEB_SEARCH: return "web_search";
        case openai::WebSearchToolType::WEB_SEARCH_2025_08_26: return "web_search_2025_08_26";
        default: throw AnnotatedException{"invalid openai::WebSearchToolType"};
    }
}


}
