#include <algorithm>
#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>

#include "base.hpp"
#include "../../interface/protocols/openai/responses.hpp"


using namespace simdjson::builder;
using namespace jai::llm;


namespace simdjson {


TAG_KIND(openai::KindAllowedToolsChoice)
TAG_KIND(openai::KindApplyPatchCall)
TAG_KIND(openai::KindApplyPatchCallOutput)
TAG_KIND(openai::KindApplyPatchTool)
TAG_KIND(openai::KindClickAction)
TAG_KIND(openai::KindCodeInterpreterCall)
TAG_KIND(openai::KindCodeInterpreterTool)
TAG_KIND(openai::KindCompactionItem)
TAG_KIND(openai::KindComputerCall)
TAG_KIND(openai::KindComputerCallOutput)
TAG_KIND(openai::KindComputerScreenshot)
TAG_KIND(openai::KindComputerUseTool)
TAG_KIND(openai::KindContainerConfig)
TAG_KIND(openai::KindContainerFileCitation)
TAG_KIND(openai::KindCreateFileOperation)
TAG_KIND(openai::KindCustomTool)
TAG_KIND(openai::KindCustomToolCall)
TAG_KIND(openai::KindCustomToolCallOutput)
TAG_KIND(openai::KindCustomToolChoice)
TAG_KIND(openai::KindCustomToolGrammarFormat)
TAG_KIND(openai::KindCustomToolTextFormat)
TAG_KIND(openai::KindDeleteFileOperation)
TAG_KIND(openai::KindDoubleClickAction)
TAG_KIND(openai::KindDragAction)
TAG_KIND(openai::KindFileCitation)
TAG_KIND(openai::KindFilePath)
TAG_KIND(openai::KindFileSearchTool)
TAG_KIND(openai::KindFileSearchToolCall)
TAG_KIND(openai::KindFindAction)
TAG_KIND(openai::KindFormatJsonSchema)
TAG_KIND(openai::KindFormatText)
TAG_KIND(openai::KindFunctionCall)
TAG_KIND(openai::KindFunctionCallOutput)
TAG_KIND(openai::KindFunctionTool)
TAG_KIND(openai::KindFunctionToolChoice)
TAG_KIND(openai::KindImageGenerationCall)
TAG_KIND(openai::KindImageGenerationTool)
TAG_KIND(openai::KindInputFile)
TAG_KIND(openai::KindInputImage)
TAG_KIND(openai::KindInputMessage)
TAG_KIND(openai::KindInputText)
TAG_KIND(openai::KindItemReference)
TAG_KIND(openai::KindKeyPressAction)
TAG_KIND(openai::KindLocalShellAction)
TAG_KIND(openai::KindLocalShellCall)
TAG_KIND(openai::KindLocalShellCallOutput)
TAG_KIND(openai::KindLocalShellTool)
TAG_KIND(openai::KindMCPApprovalRequest)
TAG_KIND(openai::KindMCPApprovalResponse)
TAG_KIND(openai::KindMCPCall)
TAG_KIND(openai::KindMCPListTools)
TAG_KIND(openai::KindMCPTool)
TAG_KIND(openai::KindMCPToolChoice)
TAG_KIND(openai::KindMoveAction)
TAG_KIND(openai::KindOpenPageAction)
TAG_KIND(openai::KindOutputMessage)
TAG_KIND(openai::KindOutputText)
TAG_KIND(openai::KindReasoningItem)
TAG_KIND(openai::KindReasoningSummaryText)
TAG_KIND(openai::KindReasoningText)
TAG_KIND(openai::KindRefusal)
TAG_KIND(openai::KindResponse)
TAG_KIND(openai::KindScreenshotAction)
TAG_KIND(openai::KindScrollAction)
TAG_KIND(openai::KindSearchAction)
TAG_KIND(openai::KindSearchActionSource)
TAG_KIND(openai::KindShellCall)
TAG_KIND(openai::KindShellCallOutput)
TAG_KIND(openai::KindShellExitOutcome)
TAG_KIND(openai::KindShellTimeoutOutcome)
TAG_KIND(openai::KindShellTool)
TAG_KIND(openai::KindSpecificApplyPatchToolChoice)
TAG_KIND(openai::KindSpecificShellToolChoice)
TAG_KIND(openai::KindTypeAction)
TAG_KIND(openai::KindUpdateFileOperation)
TAG_KIND(openai::KindUrlCitation)
TAG_KIND(openai::KindWaitAction)
TAG_KIND(openai::KindWebSearchCall)

TAG_ENUM(openai::AnnotationType)
TAG_ENUM(openai::ApplyPatchCallOutputStatus)
TAG_ENUM(openai::ApplyPatchCallStatus)
TAG_ENUM(openai::ApplyPatchOperationType)
TAG_ENUM(openai::CallStatus)
TAG_ENUM(openai::CodeInterpreterOutputType)
TAG_ENUM(openai::CodeInterpreterStatus)
TAG_ENUM(openai::ComputerActionType)
TAG_ENUM(openai::ComputerCallOutputType)
TAG_ENUM(openai::ConnectId)
TAG_ENUM(openai::ContentType)
TAG_ENUM(openai::CustomToolFormatType)
TAG_ENUM(openai::Detail)
TAG_ENUM(openai::FileSearchStatus)
TAG_ENUM(openai::FilterCompoundType)
TAG_ENUM(openai::FilterOperator)
TAG_ENUM(openai::FunctionCallStatus)
TAG_ENUM(openai::GrammarSyntax)
TAG_ENUM(openai::HostedToolMode)
TAG_ENUM(openai::ImageGenerationBackground)
TAG_ENUM(openai::ImageGenerationFidelity)
TAG_ENUM(openai::ImageGenerationFormat)
TAG_ENUM(openai::ImageGenerationQuality)
TAG_ENUM(openai::ImageGenerationSize)
TAG_ENUM(openai::IncludeOutputData)
TAG_ENUM(openai::IncompleteReason)
TAG_ENUM(openai::InputItemType)
TAG_ENUM(openai::ItemStatus)
TAG_ENUM(openai::LocalShellActionType)
TAG_ENUM(openai::LocationType)
TAG_ENUM(openai::MCPApprovalSetting)
TAG_ENUM(openai::MouseButton)
TAG_ENUM(openai::OutputItemType)
TAG_ENUM(openai::OutputMessageContentType)
TAG_ENUM(openai::PendingSafetyCheckStatus)
TAG_ENUM(openai::PromptCacheRetention)
TAG_ENUM(openai::ReasoningEffort)
TAG_ENUM(openai::ReasoningItemContentType)
TAG_ENUM(openai::ReasoningStatus)
TAG_ENUM(openai::ReasoningSummary)
TAG_ENUM(openai::ResponseFormatType)
TAG_ENUM(openai::ResponseStatus)
TAG_ENUM(openai::RoleAssistant)
TAG_ENUM(openai::RoleInputMessage)
TAG_ENUM(openai::RoleUser)
TAG_ENUM(openai::SearchContextSize)
TAG_ENUM(openai::ServiceTier)
TAG_ENUM(openai::ShellCallOutcomeType)
TAG_ENUM(openai::ToolChoiceMode)
TAG_ENUM(openai::ToolChoiceModeNotNone)
TAG_ENUM(openai::ToolChoiceType)
TAG_ENUM(openai::ToolType)
TAG_ENUM(openai::TruncationStrategy)
TAG_ENUM(openai::Verbosity)
TAG_ENUM(openai::WebSearchActionType)
TAG_ENUM(openai::WebSearchPreviewToolKind)
TAG_ENUM(openai::WebSearchPreviewToolType)
TAG_ENUM(openai::WebSearchStatus)
TAG_ENUM(openai::WebSearchToolKind)
TAG_ENUM(openai::WebSearchToolType)


/***
 * ComputerToolActions
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Click& obj) {
    builder.start_object();
    AddReqKV<"type",   CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"button", CommaDirection::BEFORE>(builder, obj.button);
    AddReqKV<"x",      CommaDirection::BEFORE>(builder, obj.x);
    AddReqKV<"y",      CommaDirection::BEFORE>(builder, obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::DoubleClick& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"x",    CommaDirection::BEFORE>(builder, obj.x);
    AddReqKV<"y",    CommaDirection::BEFORE>(builder, obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Drag::Coordinate& obj) {
    builder.start_object();
    AddReqKV<"x", CommaDirection::NONE>  (builder, obj.x);
    AddReqKV<"y", CommaDirection::BEFORE>(builder, obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Drag& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"path", CommaDirection::BEFORE>(builder, obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::KeyPress& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"keys", CommaDirection::BEFORE>(builder, obj.keys);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Move& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"x",    CommaDirection::BEFORE>(builder, obj.x);
    AddReqKV<"y",    CommaDirection::BEFORE>(builder, obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Screenshot& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Scroll& obj) {
    builder.start_object();
    AddReqKV<"type",     CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"scroll_x", CommaDirection::BEFORE>(builder, obj.scroll_x);
    AddReqKV<"scroll_y", CommaDirection::BEFORE>(builder, obj.scroll_y);
    AddReqKV<"x",        CommaDirection::BEFORE>(builder, obj.x);
    AddReqKV<"y",        CommaDirection::BEFORE>(builder, obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Type& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"text", CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Wait& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::All& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Shared Substructures
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::ConversationRef& obj) {
    builder.start_object();
    AddReqKV<"id", CommaDirection::NONE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Create& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"diff", CommaDirection::BEFORE>(builder, obj.diff);
    AddReqKV<"path", CommaDirection::BEFORE>(builder, obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Delete& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"path", CommaDirection::BEFORE>(builder, obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Update& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"diff", CommaDirection::BEFORE>(builder, obj.diff);
    AddReqKV<"path", CommaDirection::BEFORE>(builder, obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::IncompleteDetails& obj) {
    builder.start_object();
    AddReqKV<"reason", CommaDirection::NONE>(builder, obj.reason);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::Reasoning& obj) {
    builder.start_object();
    AddReqKV<"effort",  CommaDirection::NONE>  (builder, obj.effort);
    AddReqKV<"summary", CommaDirection::BEFORE>(builder, obj.summary);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseError& obj) {
    builder.start_object();
    AddReqKV<"code",    CommaDirection::NONE>  (builder, obj.code);
    AddReqKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage::InputTokenDetails& obj) {
    builder.start_object();
    AddReqKV<"cached_tokens", CommaDirection::NONE>(builder, obj.cached_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage::OutputTokenDetails& obj) {
    builder.start_object();
    AddReqKV<"reasoning_tokens", CommaDirection::NONE>(builder, obj.reasoning_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage& obj) {
    builder.start_object();
    AddReqKV<"input_tokens",          CommaDirection::NONE>  (builder, obj.input_tokens);
    AddReqKV<"input_tokens_details",  CommaDirection::BEFORE>(builder, obj.input_tokens_details);
    AddReqKV<"output_tokens",         CommaDirection::BEFORE>(builder, obj.output_tokens);
    AddReqKV<"output_tokens_details", CommaDirection::BEFORE>(builder, obj.output_tokens_details);
    AddReqKV<"total_tokens",          CommaDirection::BEFORE>(builder, obj.total_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::StreamOptions& obj) {
    builder.start_object();
    AddReqKV<"include_obfuscation", CommaDirection::NONE>(builder, obj.include_obfuscation);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig::FormatText& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig::FormatJsonSchema& obj) {
    builder.start_object();
    AddReqKV<"type",        CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",        CommaDirection::BEFORE>(builder, obj.name);
    AddReqKV<"schema",      CommaDirection::BEFORE>(builder, obj.schema);
    AddReqKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    AddReqKV<"strict",      CommaDirection::BEFORE>(builder, obj.strict);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { AddReqKV<"format", CommaDirection::NONE>(builder, x); }, obj.format);
    AddReqKV<"verbosity", CommaDirection::BEFORE>(builder, obj.verbosity);
    builder.end_object();
}


/***
 * request::ContentTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::File& obj) {
    builder.start_object();
    AddReqKV<"type",      CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"file_data", CommaDirection::BEFORE>(builder, obj.file_data);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"file_url",  CommaDirection::BEFORE>(builder, obj.file_url);
    AddOptKV<"filename",  CommaDirection::BEFORE>(builder, obj.filename);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Image& obj) {
    builder.start_object();
    AddReqKV<"type",      CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"detail",    CommaDirection::BEFORE>(builder, obj.detail);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::ContainerFileCitation& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"container_id", CommaDirection::BEFORE>(builder, obj.container_id);
    AddReqKV<"end_index",    CommaDirection::BEFORE>(builder, obj.end_index);
    AddReqKV<"file_id",      CommaDirection::BEFORE>(builder, obj.file_id);
    AddReqKV<"filename",     CommaDirection::BEFORE>(builder, obj.filename);
    AddReqKV<"start_index",  CommaDirection::BEFORE>(builder, obj.start_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::FileCitation& obj) {
    builder.start_object();
    AddReqKV<"type",     CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"file_id",  CommaDirection::BEFORE>(builder, obj.file_id);
    AddReqKV<"filename", CommaDirection::BEFORE>(builder, obj.filename);
    AddReqKV<"index",    CommaDirection::BEFORE>(builder, obj.index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::UrlCitation& obj) {
    builder.start_object();
    AddReqKV<"type",        CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"end_index",   CommaDirection::BEFORE>(builder, obj.end_index);
    AddReqKV<"start_index", CommaDirection::BEFORE>(builder, obj.start_index);
    AddReqKV<"title",       CommaDirection::BEFORE>(builder, obj.title);
    AddReqKV<"url",         CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::FilePath& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"file_id", CommaDirection::BEFORE>(builder, obj.file_id);
    AddReqKV<"index",   CommaDirection::BEFORE>(builder, obj.index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::LogProb::TopLogprob& obj) {
    builder.start_object();
    AddReqKV<"bytes",   CommaDirection::NONE>  (builder, obj.bytes);
    AddReqKV<"logprob", CommaDirection::BEFORE>(builder, obj.logprob);
    AddReqKV<"token",   CommaDirection::BEFORE>(builder, obj.token);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::LogProb& obj) {
    builder.start_object();
    AddReqKV<"bytes",         CommaDirection::NONE>  (builder, obj.bytes);
    AddReqKV<"logprob",       CommaDirection::BEFORE>(builder, obj.logprob);
    AddReqKV<"token",         CommaDirection::BEFORE>(builder, obj.token);
    AddReqKV<"top_logprobs", CommaDirection::BEFORE>(builder, obj.top_logprobs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder,
                const openai::request::ContentTypes::OutputText::Annotation& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText& obj) {
    builder.start_object();
    AddReqKV<"type",        CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"annotations", CommaDirection::BEFORE>(builder, obj.annotations);
    AddReqKV<"logprobs",    CommaDirection::BEFORE>(builder, obj.logprobs);
    AddReqKV<"text",        CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Refusal& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"refusal", CommaDirection::BEFORE>(builder, obj.refusal);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Text& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"text", CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}


/***
 * request::WebSearchToolActions
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Find& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"pattern", CommaDirection::BEFORE>(builder, obj.pattern);
    AddReqKV<"url",     CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::OpenPage& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"url",  CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Search::Source& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"url",  CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Search& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"queries", CommaDirection::BEFORE>(builder, obj.queries);
    AddOptKV<"sources", CommaDirection::BEFORE>(builder, obj.sources);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::All& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * request::InputTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::MessageContentUnit& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Message::Content& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Message& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { AddReqKV<"content", CommaDirection::NONE>(builder, x); }, obj.content);
    AddReqKV<"role",    CommaDirection::BEFORE>(builder, obj.role);
    AddOptKV<"type",    CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::InputMessage& obj) {
    builder.start_object();
    AddReqKV<"content", CommaDirection::NONE>  (builder, obj.content);
    AddReqKV<"role",    CommaDirection::BEFORE>(builder, obj.role);
    AddOptKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"type",    CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder,
                const openai::request::InputTypes::Item::OutputMessage::Content& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::OutputMessage& obj) {
    builder.start_object();
    AddReqKV<"content", CommaDirection::NONE>  (builder, obj.content);
    AddReqKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"role",    CommaDirection::BEFORE>(builder, obj.role);
    AddOptKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"type",    CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}


/***
 * request::ItemReference
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ItemReference& obj) {
    builder.start_object();
    AddOptKV<"type", CommaDirection::NONE>(builder, obj.type);
    if (obj.type) { builder.append_comma(); }
    AddReqKV<"id", CommaDirection::NONE>(builder, obj.id);
    builder.end_object();
}


/***
 * request::ToolCallItems
 */
void tag_invoke(serialize_tag, string_builder& builder, const std::variant<NameLen<512>, bool, double>& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, double>) {
            jai::llm::AppendNumber(builder, x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder,
                const std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>& obj) {
    auto F = [&builder](auto const& kv_pair) {
        auto const& [key, value] = kv_pair;
        simdjson::tag_invoke(serialize_tag{}, builder, key);
        builder.append_colon();
        simdjson::tag_invoke(serialize_tag{}, builder, value);
    };

    builder.start_object();
    if (!obj.empty()) {
        F(*obj.begin());
        std::ranges::for_each(obj | std::views::drop(1), [&builder, &F](auto const& kv_pair) {
            builder.append_comma();
            F(kv_pair);
        });
    }
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FileSearchToolCall::Result& obj) {
    builder.start_object();
    AddOptKV<"attributes", CommaDirection::NONE>  (builder, obj.attributes);
    AddOptKV<"file_id",    CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"filename",   CommaDirection::BEFORE>(builder, obj.filename);
    AddOptKV<"score",      CommaDirection::BEFORE>(builder, obj.score);
    AddOptKV<"text",       CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FileSearchToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"queries", CommaDirection::BEFORE>(builder, obj.queries);
    AddReqKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"results", CommaDirection::BEFORE>(builder, obj.results);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCall::PendingSafetyCheck& obj) {
    builder.start_object();
    AddReqKV<"id",      CommaDirection::NONE>  (builder, obj.id);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCall& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"action", CommaDirection::NONE>(builder, x); }, obj.action);
    AddReqKV<"call_id",               CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"id",                    CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"pending_safety_checks", CommaDirection::BEFORE>(builder, obj.pending_safety_checks);
    AddReqKV<"status",                CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput::ComputerScreenshot& obj) {
    builder.start_object();
    AddReqKV<"type",      CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput::AcknowledgedSafetyCheck& obj) {
    builder.start_object();
    AddReqKV<"id",      CommaDirection::NONE>  (builder, obj.id);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type",                       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"call_id",                    CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"output",                     CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"acknowledged_safety_checks", CommaDirection::BEFORE>(builder, obj.acknowledged_safety_checks);
    AddOptKV<"id",                         CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",                     CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::WebSearchToolCall& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"action", CommaDirection::NONE>(builder, x); }, obj.action);
    AddReqKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FunctionToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",      CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"arguments", CommaDirection::BEFORE>(builder, obj.arguments);
    AddReqKV<"call_id",   CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"name",      CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"id",        CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",    CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FunctionToolCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"output", CommaDirection::NONE>(builder, x); }, obj.output);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning::Summary& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"text", CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning::Content& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"text", CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",                CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"summary",           CommaDirection::BEFORE>(builder, obj.summary);
    AddOptKV<"content",           CommaDirection::BEFORE>(builder, obj.content);
    AddOptKV<"encrypted_content", CommaDirection::BEFORE>(builder, obj.encrypted_content);
    AddOptKV<"status",            CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CompactionItem& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"encrypted_content", CommaDirection::BEFORE>(builder, obj.encrypted_content);
    AddOptKV<"id", CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ImageGenerationCall& obj) {
    builder.start_object();
    AddReqKV<"type",   CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"result", CommaDirection::BEFORE>(builder, obj.result);
    AddReqKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputLog& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"logs", CommaDirection::BEFORE>(builder, obj.logs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputImage& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"url",  CommaDirection::BEFORE>(builder, obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder,
                const openai::request::InputTypes::Item::CodeInterpreterToolCall::Output& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"code",         CommaDirection::BEFORE>(builder, obj.code);
    AddReqKV<"container_id", CommaDirection::BEFORE>(builder, obj.container_id);
    AddReqKV<"id",           CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"outputs",      CommaDirection::BEFORE>(builder, obj.outputs);
    AddReqKV<"status",       CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCall::Action& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"command",           CommaDirection::BEFORE>(builder, obj.command);
    AddReqKV<"env",               CommaDirection::BEFORE>(builder, obj.env);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    AddOptKV<"user",              CommaDirection::BEFORE>(builder, obj.user);
    AddOptKV<"working_directory", CommaDirection::BEFORE>(builder, obj.working_directory);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCall& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"action",  CommaDirection::BEFORE>(builder, obj.action);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type",   CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"output", CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCall::Action& obj) {
    builder.start_object();
    AddReqKV<"commands", CommaDirection::NONE>(builder, obj.commands);
    AddOptKV<"max_output_length", CommaDirection::BEFORE>(builder, obj.max_output_length);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"action",  CommaDirection::BEFORE>(builder, obj.action);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"exit_code", CommaDirection::BEFORE>(builder, obj.exit_code);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::Output& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { AddReqKV<"outcome", CommaDirection::NONE>(builder, x); }, obj.outcome);
    AddReqKV<"std_err", CommaDirection::BEFORE>(builder, obj.std_err);
    AddReqKV<"std_out", CommaDirection::BEFORE>(builder, obj.std_out);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type",              CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"call_id",           CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"output",            CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"id",                CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"max_output_length", CommaDirection::BEFORE>(builder, obj.max_output_length);
    AddOptKV<"status",            CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ApplyPatchToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"operation", CommaDirection::NONE>(builder, x); }, obj.operation);
    AddReqKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ApplyPatchToolCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type",   CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"output",  CommaDirection::BEFORE>(builder, obj.output);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPListTools::ToolDef& obj) {
    builder.start_object();
    AddReqKV<"input_schema", CommaDirection::NONE>(builder, obj.input_schema);
    AddReqKV<"name", CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"annotations", CommaDirection::BEFORE>(builder, obj.annotations);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPListTools& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"id",           CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    AddReqKV<"tools",        CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"error",        CommaDirection::BEFORE>(builder, obj.error);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPApprovalRequest& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"arguments",    CommaDirection::BEFORE>(builder, obj.arguments);
    AddReqKV<"id",           CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"name",         CommaDirection::BEFORE>(builder, obj.name);
    AddReqKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPApprovalResponse& obj) {
    builder.start_object();
    AddReqKV<"type",                CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"approval_request_id", CommaDirection::BEFORE>(builder, obj.approval_request_id);
    AddReqKV<"approve",             CommaDirection::BEFORE>(builder, obj.approve);
    AddOptKV<"id",                  CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"reason",              CommaDirection::BEFORE>(builder, obj.reason);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",                CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"arguments",           CommaDirection::BEFORE>(builder, obj.arguments);
    AddReqKV<"id",                  CommaDirection::BEFORE>(builder, obj.id);
    AddReqKV<"name",                CommaDirection::BEFORE>(builder, obj.name);
    AddReqKV<"server_label",        CommaDirection::BEFORE>(builder, obj.server_label);
    AddOptKV<"approval_request_id", CommaDirection::BEFORE>(builder, obj.approval_request_id);
    AddOptKV<"error",               CommaDirection::BEFORE>(builder, obj.error);
    AddOptKV<"output",              CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"status",              CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CustomToolCallOutput& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"output", CommaDirection::NONE>(builder, x); }, obj.output);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CustomToolCall& obj) {
    builder.start_object();
    AddReqKV<"type",    CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddReqKV<"input",   CommaDirection::BEFORE>(builder, obj.input);
    AddReqKV<"name",    CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}


/***
 * request::Prompt
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Prompt::VariableTypes& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Prompt& obj) {
    builder.start_object();
    AddReqKV<"id",        CommaDirection::NONE>  (builder, obj.id);
    AddOptKV<"variables", CommaDirection::BEFORE>(builder, obj.variables);
    AddOptKV<"version",   CommaDirection::BEFORE>(builder, obj.version);
    builder.end_object();
}


/***
 * request::ToolTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Function& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",       CommaDirection::BEFORE>(builder, obj.name);
    AddReqKV<"parameters", CommaDirection::BEFORE>(builder, obj.parameters);
    AddReqKV<"strict",     CommaDirection::BEFORE>(builder, obj.strict);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::ComparisonFilter& obj) {
    builder.start_object();
    AddReqKV<"key",  CommaDirection::NONE>  (builder, obj.key);
    AddReqKV<"type", CommaDirection::BEFORE>(builder, obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"value", CommaDirection::NONE>(builder, x); }, obj.value);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::CompoundFilter& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    AddReqKV<"filters", CommaDirection::BEFORE>(builder, obj.filters);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::RankingOptions::HybridSearch& obj) {
    builder.start_object();
    AddReqKV<"embedding_weight", CommaDirection::NONE>  (builder, obj.embedding_weight);
    AddReqKV<"text_weight",      CommaDirection::BEFORE>(builder, obj.text_weight);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::RankingOptions& obj) {
    builder.start_object();
    AddOptKV<"hybrid_search",   CommaDirection::NONE>  (builder, obj.hybrid_search);
    AddOptKV<"ranker",          CommaDirection::BEFORE>(builder, obj.ranker);
    AddOptKV<"score_threshold", CommaDirection::BEFORE>(builder, obj.score_threshold);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder,
                const std::variant<openai::request::ToolTypes::FileSearch::ComparisonFilter,
                                   openai::request::ToolTypes::FileSearch::CompoundFilter>& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch& obj) {
    builder.start_object();
    AddReqKV<"type",             CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"vector_store_ids", CommaDirection::BEFORE>(builder, obj.vector_store_ids);
    AddOptKV<"filters",          CommaDirection::BEFORE>(builder, obj.filters);
    AddOptKV<"max_num_results",  CommaDirection::BEFORE>(builder, obj.max_num_results);
    AddOptKV<"ranking_options",  CommaDirection::BEFORE>(builder, obj.ranking_options);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ComputerUse& obj) {
    builder.start_object();
    AddReqKV<"type",           CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"display_height", CommaDirection::BEFORE>(builder, obj.display_height);
    AddReqKV<"display_width",  CommaDirection::BEFORE>(builder, obj.display_width);
    AddReqKV<"environment",    CommaDirection::BEFORE>(builder, obj.environment);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch::Filters& obj) {
    builder.start_object();
    AddOptKV<"allowed_domains", CommaDirection::NONE>(builder, obj.allowed_domains);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch::Location& obj) {
    builder.start_object();
    AddReqKV<"type",     CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch& obj) {
    builder.start_object();
    AddReqKV<"type",                CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"filters",             CommaDirection::BEFORE>(builder, obj.filters);
    AddOptKV<"search_context_size", CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",       CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP::AllowedTools& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP::Filter& obj) {
    builder.start_object();
    AddOptKV<"read_only",  CommaDirection::NONE>(builder, obj.read_only);
    AddOptKV<"tool_names", CommaDirection::BEFORE>(builder, obj.tool_names);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP::ApprovalFilter& obj) {
    builder.start_object();
    AddOptKV<"always", CommaDirection::NONE>(builder, obj.always);
    AddOptKV<"never",  CommaDirection::BEFORE>(builder, obj.never);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP::RequiredApproval& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP& obj) {
    builder.start_object();
    AddReqKV<"type",               CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"server_label",       CommaDirection::BEFORE>(builder, obj.server_label);
    AddOptKV<"allowed_tools",      CommaDirection::BEFORE>(builder, obj.allowed_tools);
    AddOptKV<"authorization",      CommaDirection::BEFORE>(builder, obj.authorization);
    AddOptKV<"connector_id",       CommaDirection::BEFORE>(builder, obj.connector_id);
    AddOptKV<"headers",            CommaDirection::BEFORE>(builder, obj.headers);
    AddOptKV<"require_approval",   CommaDirection::BEFORE>(builder, obj.require_approval);
    AddOptKV<"server_description", CommaDirection::BEFORE>(builder, obj.server_description);
    AddOptKV<"server_url",         CommaDirection::BEFORE>(builder, obj.server_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::CodeInterpreter::ContainerConfig& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"file_ids",     CommaDirection::BEFORE>(builder, obj.file_ids);
    AddOptKV<"memory_limit", CommaDirection::BEFORE>(builder, obj.memory_limit);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::CodeInterpreter::Container& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::CodeInterpreter& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { AddReqKV<"container", CommaDirection::NONE>(builder, x); }, obj.container);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ImageGeneration::Mask& obj) {
    builder.start_object();
    AddOptKV<"file_id",   CommaDirection::NONE>  (builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ImageGeneration& obj) {
    builder.start_object();
    AddReqKV<"type",               CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"action",             CommaDirection::BEFORE>(builder, obj.action);
    AddOptKV<"background",         CommaDirection::BEFORE>(builder, obj.background);
    AddOptKV<"input_fidelity",     CommaDirection::BEFORE>(builder, obj.input_fidelity);
    AddOptKV<"input_image_mask",   CommaDirection::BEFORE>(builder, obj.input_image_mask);
    AddOptKV<"model",              CommaDirection::BEFORE>(builder, obj.model);
    AddOptKV<"moderation",         CommaDirection::BEFORE>(builder, obj.moderation);
    AddOptKV<"output_compression", CommaDirection::BEFORE>(builder, obj.output_compression);
    AddOptKV<"output_format",      CommaDirection::BEFORE>(builder, obj.output_format);
    AddOptKV<"partial_images",     CommaDirection::BEFORE>(builder, obj.partial_images);
    AddOptKV<"quality",            CommaDirection::BEFORE>(builder, obj.quality);
    AddOptKV<"size",               CommaDirection::BEFORE>(builder, obj.size);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::LocalShell& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Shell& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom::GrammarFormat& obj) {
    builder.start_object();
    AddReqKV<"type",       CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"definition", CommaDirection::BEFORE>(builder, obj.definition);
    AddReqKV<"syntax",     CommaDirection::BEFORE>(builder, obj.syntax);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom::TextFormat& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom::Format& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom& obj) {
    builder.start_object();
    AddReqKV<"type",        CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name",        CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"format",      CommaDirection::BEFORE>(builder, obj.format);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearchPreview::Location& obj) {
    builder.start_object();
    AddReqKV<"type",     CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearchPreview& obj) {
    builder.start_object();
    AddReqKV<"type",                CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"search_context_size", CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",       CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ApplyPatch& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}


/***
 * request::ToolsChoiceTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Allowed::RestrictedTool& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddOptKV<"name",         CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Allowed& obj) {
    builder.start_object();
    AddReqKV<"type",  CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"mode",  CommaDirection::BEFORE>(builder, obj.mode);
    AddReqKV<"tools", CommaDirection::BEFORE>(builder, obj.tools);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Custom& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name", CommaDirection::BEFORE>(builder, obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Function& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"name", CommaDirection::BEFORE>(builder, obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Hosted& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::MCP& obj) {
    builder.start_object();
    AddReqKV<"type",         CommaDirection::NONE>  (builder, obj.type);
    AddReqKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    AddOptKV<"name",         CommaDirection::BEFORE>(builder, obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::SpecificApplyPatch& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::SpecificShell& obj) {
    builder.start_object();
    AddReqKV<"type", CommaDirection::NONE>(builder, obj.type);
    builder.end_object();
}


/***
 * Top-level Variants (request)
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Item& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputItemList& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Tool& obj) {
    std::visit([&](auto const& x) { tag_invoke(serialize_tag{}, builder, x); }, obj);
}


void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Input& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolChoice& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, openai::ToolChoiceMode>) {
            builder.escape_and_append_with_quotes(to_string_view(x));
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder,
                const std::variant<std::string, openai::ConversationRef>& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(serialize_tag{}, builder, x);
        }
    }, obj);
}


void tag_invoke(serialize_tag, string_builder& builder,
                const std::map<NameLen<64>, NameLen<512>>& obj) {
    auto F = [&builder](auto const& kv_pair) {
        auto const& [key, value] = kv_pair;
        simdjson::tag_invoke(serialize_tag{}, builder, key);
        builder.append_colon();
        simdjson::tag_invoke(serialize_tag{}, builder, value);
    };

    builder.start_object();
    if (!obj.empty()) {
        F(*obj.begin());
        std::ranges::for_each(obj | std::views::drop(1), [&builder, &F](auto const& kv_pair) {
            builder.append_comma();
            F(kv_pair);
        });
    }
    builder.end_object();
}


/***
 * Top-level Request
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::Request& obj) {
    builder.start_object();
    AddOptKV<"background",             CommaDirection::NONE>  (builder, obj.background);
    AddOptKV<"conversation",           CommaDirection::BEFORE>(builder, obj.conversation);
    AddOptKV<"include",                CommaDirection::BEFORE>(builder, obj.include);
    AddOptKV<"input",                  CommaDirection::BEFORE>(builder, obj.input);
    AddOptKV<"instructions",           CommaDirection::BEFORE>(builder, obj.instructions);
    AddOptKV<"max_output_tokens",      CommaDirection::BEFORE>(builder, obj.max_output_tokens);
    AddOptKV<"max_tool_calls",         CommaDirection::BEFORE>(builder, obj.max_tool_calls);
    AddOptKV<"metadata",               CommaDirection::BEFORE>(builder, obj.metadata);
    AddOptKV<"model",                  CommaDirection::BEFORE>(builder, obj.model);
    AddOptKV<"parallel_tool_calls",    CommaDirection::BEFORE>(builder, obj.parallel_tool_calls);
    AddOptKV<"previous_response_id",   CommaDirection::BEFORE>(builder, obj.previous_response_id);
    AddOptKV<"prompt",                 CommaDirection::BEFORE>(builder, obj.prompt);
    AddOptKV<"prompt_cache_key",       CommaDirection::BEFORE>(builder, obj.prompt_cache_key);
    AddOptKV<"prompt_cache_retention", CommaDirection::BEFORE>(builder, obj.prompt_cache_retention);
    AddOptKV<"reasoning",              CommaDirection::BEFORE>(builder, obj.reasoning);
    AddOptKV<"safety_identifier",      CommaDirection::BEFORE>(builder, obj.safety_identifier);
    AddOptKV<"service_tier",           CommaDirection::BEFORE>(builder, obj.service_tier);
    AddOptKV<"store",                  CommaDirection::BEFORE>(builder, obj.store);
    AddOptKV<"stream",                 CommaDirection::BEFORE>(builder, obj.stream);
    AddOptKV<"stream_options",         CommaDirection::BEFORE>(builder, obj.stream_options);
    AddOptKV<"temperature",            CommaDirection::BEFORE>(builder, obj.temperature);
    AddOptKV<"text",                   CommaDirection::BEFORE>(builder, obj.text);
    AddOptKV<"tool_choice",            CommaDirection::BEFORE>(builder, obj.tool_choice);
    AddOptKV<"tools",                  CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"top_logprobs",           CommaDirection::BEFORE>(builder, obj.top_logprobs);
    AddOptKV<"top_p",                  CommaDirection::BEFORE>(builder, obj.top_p);
    AddOptKV<"truncation",             CommaDirection::BEFORE>(builder, obj.truncation);
    builder.end_object();
}


}


namespace jai::llm::openai {


/***
 * Top-level Serialize
 */
std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    try {
        builder.clear();
        simdjson::tag_invoke(simdjson::serialize_tag{}, builder, request);

        if (!builder.validate_unicode()) {
            throw AnnotatedException{"openai::Serialize Failed", "string_builder generated invalid unicode data."};
        }

        auto result = builder.view();
        if (result.error() != simdjson::SUCCESS) {
            throw AnnotatedException{"openai::Serialize Failed", simdjson::error_message(result.error())};
        }
        std::string_view json_str = result.value();

        return json_str |
               std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
               std::ranges::to<std::vector<std::byte>>();
    } catch (AnnotatedException const&) {
        throw;
    } catch (std::exception const& e) {
        AnnotatedException ex{"openai::Serialize Failed", "string_builder failed to serialize openai::Response."};
        ex.AddContext(e.what());
        throw ex;
    }
}


}
