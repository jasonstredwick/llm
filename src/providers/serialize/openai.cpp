#include "../../interface/protocols/openai/responses.hpp"
#include "../../interface/protocols/openai/strings.hpp" // must include before base.hpp
#include "base.hpp"

#include <cstddef>
#include <ranges>
#include <vector>

#include <simdjson.h>


using namespace simdjson;
using namespace builder;


namespace jai::llm {


/***
 * Shared Substructures (Block 1)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseFormatText& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseFormatJsonSchema& obj) {
    builder.start_object();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"schema">(obj.schema);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"strict",      CommaDirection::BEFORE>(builder, obj.strict);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseFormat& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig& obj) {
    builder.start_object();
    AddOptKV<"format",    CommaDirection::NONE>  (builder, obj.format);
    AddOptKV<"verbosity", CommaDirection::BEFORE>(builder, obj.verbosity);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::StreamOptions& obj) {
    builder.start_object();
    AddOptKV<"include_obfuscation", CommaDirection::NONE>(builder, obj.include_obfuscation);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ReasoningConfig& obj) {
    builder.start_object();
    AddOptKV<"effort",  CommaDirection::NONE>  (builder, obj.effort);
    AddOptKV<"summary", CommaDirection::BEFORE>(builder, obj.summary);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::InputText& obj) {
    builder.start_object();
    builder.append_key_value<"text">(obj.text);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::InputImage& obj) {
    builder.start_object();
    builder.append_key_value<"detail">(obj.detail);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::InputFile& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_data", CommaDirection::BEFORE>(builder, obj.file_data);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"file_url",  CommaDirection::BEFORE>(builder, obj.file_url);
    AddOptKV<"filename",  CommaDirection::BEFORE>(builder, obj.filename);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ItemReference& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ConversationRef& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PromptRef::VariableValue& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PromptRef& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    AddOptKV<"variables", CommaDirection::BEFORE>(builder, obj.variables);
    AddOptKV<"version",   CommaDirection::BEFORE>(builder, obj.version);
    builder.end_object();
}


/***
 * Tool Call Actions and Operations (Block 2)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::ClickAction& obj) {
    builder.start_object();
    builder.append_key_value<"button">(obj.button);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::DoubleClickAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::DragAction::Coordinate& obj) {
    builder.start_object();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::DragAction& obj) {
    builder.start_object();
    builder.append_key_value<"path">(obj.path);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::KeyPressAction& obj) {
    builder.start_object();
    builder.append_key_value<"keys">(obj.keys);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::MoveAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ScreenshotAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ScrollAction& obj) {
    builder.start_object();
    builder.append_key_value<"scroll_x">(obj.scroll_x);
    builder.append_comma();
    builder.append_key_value<"scroll_y">(obj.scroll_y);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TypeAction& obj) {
    builder.start_object();
    builder.append_key_value<"text">(obj.text);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WaitAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerAction& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::SearchAction::Source& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::SearchAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"queries", CommaDirection::BEFORE>(builder, obj.queries);
    AddOptKV<"sources", CommaDirection::BEFORE>(builder, obj.sources);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OpenPageAction& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FindAction& obj) {
    builder.start_object();
    builder.append_key_value<"pattern">(obj.pattern);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchAction& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CreateFileOperation& obj) {
    builder.start_object();
    builder.append_key_value<"diff">(obj.diff);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::DeleteFileOperation& obj) {
    builder.start_object();
    builder.append_key_value<"path">(obj.path);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::UpdateFileOperation& obj) {
    builder.start_object();
    builder.append_key_value<"diff">(obj.diff);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ApplyPatchOperation& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Call and Output structs (Block 3)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::FileSearchCall::Result& obj) {
    builder.start_object();
    AddOptKV<"attributes", CommaDirection::NONE>  (builder, obj.attributes);
    AddOptKV<"file_id",    CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"filename",   CommaDirection::BEFORE>(builder, obj.filename);
    AddOptKV<"score",      CommaDirection::BEFORE>(builder, obj.score);
    AddOptKV<"text",       CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FileSearchCall& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"queries">(obj.queries);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"results", CommaDirection::BEFORE>(builder, obj.results);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerCall::PendingSafetyCheck& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerCall& obj) {
    builder.start_object();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"pending_safety_checks">(obj.pending_safety_checks);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerScreenshot& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"acknowledged_safety_checks", CommaDirection::BEFORE>(builder, obj.acknowledged_safety_checks);
    AddOptKV<"id",                         CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",                     CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchCall& obj) {
    builder.start_object();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FunctionCall& obj) {
    builder.start_object();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FunctionCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ReasoningItem::Summary& obj) {
    builder.start_object();
    builder.append_key_value<"text">(obj.text);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ReasoningItem::Content& obj) {
    builder.start_object();
    builder.append_key_value<"text">(obj.text);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ReasoningItem& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"summary",           CommaDirection::BEFORE>(builder, obj.summary);
    AddOptKV<"content",           CommaDirection::BEFORE>(builder, obj.content);
    AddOptKV<"encrypted_content", CommaDirection::BEFORE>(builder, obj.encrypted_content);
    AddOptKV<"status",            CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CompactionItem& obj) {
    builder.start_object();
    builder.append_key_value<"encrypted_content">(obj.encrypted_content);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"created_by", CommaDirection::BEFORE>(builder, obj.created_by);
    AddOptKV<"id",         CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ImageGenerationCall& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"result">(obj.result);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterCall::OutputLog& obj) {
    builder.start_object();
    builder.append_key_value<"logs">(obj.logs);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterCall::OutputImage& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterCall& obj) {
    builder.start_object();
    builder.append_key_value<"container_id">(obj.container_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"outputs", CommaDirection::BEFORE>(builder, obj.outputs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::LocalShellCall::Action& obj) {
    builder.start_object();
    builder.append_key_value<"command">(obj.command);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"env",               CommaDirection::BEFORE>(builder, obj.env);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    AddOptKV<"user",             CommaDirection::BEFORE>(builder, obj.user);
    AddOptKV<"working_directory", CommaDirection::BEFORE>(builder, obj.working_directory);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::LocalShellCall& obj) {
    builder.start_object();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::LocalShellCallOutput::ActionOutcome& obj) {
    builder.start_object();
    builder.append_key_value<"exit_code">(obj.exit_code);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"std_err", CommaDirection::BEFORE>(builder, obj.std_err);
    AddOptKV<"std_out", CommaDirection::BEFORE>(builder, obj.std_out);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::LocalShellCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"max_output_length">(obj.max_output_length);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCall::Action& obj) {
    builder.start_object();
    builder.append_key_value<"commands">(obj.commands);
    AddOptKV<"max_output_length", CommaDirection::BEFORE>(builder, obj.max_output_length);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCall& obj) {
    builder.start_object();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"created_by", CommaDirection::BEFORE>(builder, obj.created_by);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCallOutput::ShellExitOutcome& obj) {
    builder.start_object();
    builder.append_key_value<"exit_code">(obj.exit_code);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"std_err", CommaDirection::BEFORE>(builder, obj.std_err);
    AddOptKV<"std_out", CommaDirection::BEFORE>(builder, obj.std_out);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCallOutput::ShellTimeoutOutcome& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCallOutput::Content& obj) {
    builder.start_object();
    builder.append_key_value<"outcome">(obj.outcome);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"max_output_length">(obj.max_output_length);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",         CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",     CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"created_by", CommaDirection::BEFORE>(builder, obj.created_by);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ApplyPatchCall& obj) {
    builder.start_object();
    builder.append_key_value<"operation">(obj.operation);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"call_id",    CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"created_by", CommaDirection::BEFORE>(builder, obj.created_by);
    AddOptKV<"id",         CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ApplyPatchCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"call_id",    CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"created_by", CommaDirection::BEFORE>(builder, obj.created_by);
    AddOptKV<"id",         CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"output",     CommaDirection::BEFORE>(builder, obj.output);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpListTools::ToolDef& obj) {
    builder.start_object();
    builder.append_key_value<"input_schema">(obj.input_schema);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"annotations", CommaDirection::BEFORE>(builder, obj.annotations);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpListTools& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"tools">(obj.tools);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"error", CommaDirection::BEFORE>(builder, obj.error);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpApprovalRequest& obj) {
    builder.start_object();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpApprovalResponse& obj) {
    builder.start_object();
    builder.append_key_value<"approval_request_id">(obj.approval_request_id);
    builder.append_comma();
    builder.append_key_value<"approve">(obj.approve);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"reason", CommaDirection::BEFORE>(builder, obj.reason);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpCall& obj) {
    builder.start_object();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"approval_request_id", CommaDirection::BEFORE>(builder, obj.approval_request_id);
    AddOptKV<"error",               CommaDirection::BEFORE>(builder, obj.error);
    AddOptKV<"output",              CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"status",              CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"input">(obj.input);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"output">(obj.output);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status",  CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}


/***
 * Message and InputItem variants (Block 4)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::InputMessage& obj) {
    builder.start_object();
    builder.append_key_value<"content">(obj.content);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::CitationContainer& obj) {
    builder.start_object();
    builder.append_key_value<"container_id">(obj.container_id);
    builder.append_comma();
    builder.append_key_value<"end_index">(obj.end_index);
    builder.append_comma();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"filename">(obj.filename);
    builder.append_comma();
    builder.append_key_value<"start_index">(obj.start_index);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::CitationFile& obj) {
    builder.start_object();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"filename">(obj.filename);
    builder.append_comma();
    builder.append_key_value<"index">(obj.index);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::CitationUrl& obj) {
    builder.start_object();
    builder.append_key_value<"end_index">(obj.end_index);
    builder.append_comma();
    builder.append_key_value<"start_index">(obj.start_index);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::FilePath& obj) {
    builder.start_object();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"index">(obj.index);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::Logprob::TopLogprob& obj) {
    builder.start_object();
    builder.append_key_value<"bytes">(obj.bytes);
    builder.append_comma();
    builder.append_key_value<"logprob">(obj.logprob);
    builder.append_comma();
    builder.append_key_value<"token">(obj.token);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText::Logprob& obj) {
    builder.start_object();
    builder.append_key_value<"bytes">(obj.bytes);
    builder.append_comma();
    builder.append_key_value<"logprob">(obj.logprob);
    builder.append_comma();
    builder.append_key_value<"token">(obj.token);
    builder.append_comma();
    builder.append_key_value<"top_logprobs">(obj.top_logprobs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::OutputText& obj) {
    builder.start_object();
    builder.append_key_value<"annotations">(obj.annotations);
    builder.append_comma();
    builder.append_key_value<"value">(obj.value);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"logprobs", CommaDirection::BEFORE>(builder, obj.logprobs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage::Refusal& obj) {
    builder.start_object();
    builder.append_key_value<"refusal">(obj.refusal);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::OutputMessage& obj) {
    builder.start_object();
    builder.append_key_value<"content">(obj.content);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::InputItem& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Model Tools (Block 5)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::FunctionTool::Parameters& obj) {
    builder.start_object();
    builder.append_key_value<"parameters">(obj.parameters);
    builder.append_comma();
    builder.append_key_value<"strict">(obj.strict);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FunctionTool& obj) {
    builder.start_object();
    builder.append_key_value<"description">(obj.description);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"parameters">(obj.parameters);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FileSearchTool::RankingOptions::HybridSearch& obj) {
    builder.start_object();
    builder.append_key_value<"embedding_weight">(obj.embedding_weight);
    builder.append_comma();
    builder.append_key_value<"text_weight">(obj.text_weight);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FileSearchTool::RankingOptions& obj) {
    builder.start_object();
    AddOptKV<"hybrid_search",    CommaDirection::NONE>  (builder, obj.hybrid_search);
    AddOptKV<"ranker",           CommaDirection::BEFORE>(builder, obj.ranker);
    AddOptKV<"score_threshold",  CommaDirection::BEFORE>(builder, obj.score_threshold);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FileSearchTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"vector_store_ids">(obj.vector_store_ids);
    AddOptKV<"max_num_results", CommaDirection::BEFORE>(builder, obj.max_num_results);
    AddOptKV<"ranking_options", CommaDirection::BEFORE>(builder, obj.ranking_options);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerUseTool& obj) {
    builder.start_object();
    builder.append_key_value<"display_height">(obj.display_height);
    builder.append_comma();
    builder.append_key_value<"display_width">(obj.display_width);
    builder.append_comma();
    builder.append_key_value<"environment">(obj.environment);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchTool::Filters::Location& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchTool::Filters& obj) {
    builder.start_object();
    AddOptKV<"allowed_domains",      CommaDirection::NONE>  (builder, obj.allowed_domains);
    AddOptKV<"search_context_size",  CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",        CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"filters", CommaDirection::BEFORE>(builder, obj.filters);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpTool::Filter& obj) {
    builder.start_object();
    builder.append_key_value<"read_only">(obj.read_only);
    builder.append_comma();
    builder.append_key_value<"tool_names">(obj.tool_names);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpTool::AllowedTools& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpTool::ApprovalFilter& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpTool& obj) {
    builder.start_object();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"allowed_tools",      CommaDirection::BEFORE>(builder, obj.allowed_tools);
    AddOptKV<"authorization",       CommaDirection::BEFORE>(builder, obj.authorization);
    AddOptKV<"connector_id",        CommaDirection::BEFORE>(builder, obj.connector_id);
    AddOptKV<"headers",             CommaDirection::BEFORE>(builder, obj.headers);
    AddOptKV<"require_approval",    CommaDirection::BEFORE>(builder, obj.require_approval);
    AddOptKV<"server_description",  CommaDirection::BEFORE>(builder, obj.server_description);
    AddOptKV<"server_url",          CommaDirection::BEFORE>(builder, obj.server_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterTool::ContainerConfig& obj) {
    builder.start_object();
    builder.append_key_value<"file_ids">(obj.file_ids);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"memory_limit", CommaDirection::BEFORE>(builder, obj.memory_limit);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterTool::Container& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CodeInterpreterTool& obj) {
    builder.start_object();
    builder.append_key_value<"container">(obj.container);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ImageGenerationTool::Mask& obj) {
    builder.start_object();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"image_url">(obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ImageGenerationTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"background",         CommaDirection::BEFORE>(builder, obj.background);
    AddOptKV<"input_fidelity",     CommaDirection::BEFORE>(builder, obj.input_fidelity);
    AddOptKV<"input_image_mask",   CommaDirection::BEFORE>(builder, obj.input_image_mask);
    AddOptKV<"model",               CommaDirection::BEFORE>(builder, obj.model);
    AddOptKV<"moderation",          CommaDirection::BEFORE>(builder, obj.moderation);
    AddOptKV<"output_compression",  CommaDirection::BEFORE>(builder, obj.output_compression);
    AddOptKV<"output_format",       CommaDirection::BEFORE>(builder, obj.output_format);
    AddOptKV<"partial_images",      CommaDirection::BEFORE>(builder, obj.partial_images);
    AddOptKV<"quality",             CommaDirection::BEFORE>(builder, obj.quality);
    AddOptKV<"size",                CommaDirection::BEFORE>(builder, obj.size);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::LocalShellTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomTool::GrammarFormat& obj) {
    builder.start_object();
    builder.append_key_value<"syntax">(obj.syntax);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"value">(obj.value);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomTool::TextFormat& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomTool& obj) {
    builder.start_object();
    builder.append_key_value<"description">(obj.description);
    builder.append_comma();
    builder.append_key_value<"format">(obj.format);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchPreviewTool::Location& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::WebSearchPreviewTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"search_context_size", CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",       CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ApplyPatchTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ShellCallOutput::Outcome& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::Tool& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Tool Choice models (Block 6)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::AllowedToolsChoice::RestrictedTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"name",         CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::AllowedToolsChoice& obj) {
    builder.start_object();
    builder.append_key_value<"mode">(obj.mode);
    builder.append_comma();
    builder.append_key_value<"tools">(obj.tools);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::HostedToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::FunctionToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::McpToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::CustomToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::SpecificApplyPatchToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::SpecificShellToolChoice& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ToolChoice& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Request (Block 7)
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::Request& obj) {
    builder.start_object();
    AddOptKV<"background",           CommaDirection::NONE>  (builder, obj.background);
    AddOptKV<"conversation",         CommaDirection::BEFORE>(builder, obj.conversation);
    AddOptKV<"include",              CommaDirection::BEFORE>(builder, obj.include);
    AddOptKV<"input",                CommaDirection::BEFORE>(builder, obj.input);
    AddOptKV<"instructions",         CommaDirection::BEFORE>(builder, obj.instructions);
    AddOptKV<"max_output_tokens",    CommaDirection::BEFORE>(builder, obj.max_output_tokens);
    AddOptKV<"metadata",             CommaDirection::BEFORE>(builder, obj.metadata);
    AddOptKV<"model",                CommaDirection::BEFORE>(builder, obj.model);
    AddOptKV<"parallel_tool_calls", CommaDirection::BEFORE>(builder, obj.parallel_tool_calls);
    AddOptKV<"previous_response_id", CommaDirection::BEFORE>(builder, obj.previous_response_id);
    AddOptKV<"prompt",               CommaDirection::BEFORE>(builder, obj.prompt);
    AddOptKV<"reasoning",            CommaDirection::BEFORE>(builder, obj.reasoning);
    AddOptKV<"service_tier",         CommaDirection::BEFORE>(builder, obj.service_tier);
    AddOptKV<"store",                CommaDirection::BEFORE>(builder, obj.store);
    AddOptKV<"temperature",          CommaDirection::BEFORE>(builder, obj.temperature);
    AddOptKV<"text",                 CommaDirection::BEFORE>(builder, obj.text);
    AddOptKV<"tool_choice",          CommaDirection::BEFORE>(builder, obj.tool_choice);
    AddOptKV<"tools",                CommaDirection::BEFORE>(builder, obj.tools);
    AddOptKV<"top_p",                CommaDirection::BEFORE>(builder, obj.top_p);
    builder.end_object();
}


namespace openai {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    builder.clear();
    jai::llm::tag_invoke(serialize_tag{}, builder, request);
    builder.validate_unicode();
    std::string_view json_str = builder.view();

    return json_str |
           std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
           std::ranges::to<std::vector<std::byte>>();
}


}


}
