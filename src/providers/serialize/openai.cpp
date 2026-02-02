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
 * ComputerToolActions
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Click& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"button">(obj.button);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::DoubleClick& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Drag::Coordinate& obj) {
    builder.start_object();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Drag& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::KeyPress& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"keys">(obj.keys);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Move& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Screenshot& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Scroll& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"scroll_x">(obj.scroll_x);
    builder.append_comma();
    builder.append_key_value<"scroll_y">(obj.scroll_y);
    builder.append_comma();
    builder.append_key_value<"x">(obj.x);
    builder.append_comma();
    builder.append_key_value<"y">(obj.y);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Type& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::Wait& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ComputerToolActions::All& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * Shared Substructures
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::ConversationRef& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Create& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"diff">(obj.diff);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Delete& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::PatchFileOperations::Update& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"diff">(obj.diff);
    builder.append_comma();
    builder.append_key_value<"path">(obj.path);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::IncompleteDetails& obj) {
    builder.start_object();
    builder.append_key_value<"reason">(obj.reason);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::Reasoning& obj) {
    builder.start_object();
    builder.append_key_value<"effort">(obj.effort);
    builder.append_comma();
    builder.append_key_value<"summary">(obj.summary);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseError& obj) {
    builder.start_object();
    builder.append_key_value<"code">(obj.code);
    builder.append_comma();
    builder.append_key_value<"message">(obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage::InputTokenDetails& obj) {
    builder.start_object();
    builder.append_key_value<"cached_tokens">(obj.cached_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage::OutputTokenDetails& obj) {
    builder.start_object();
    builder.append_key_value<"reasoning_tokens">(obj.reasoning_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::ResponseUsage& obj) {
    builder.start_object();
    builder.append_key_value<"input_tokens">(obj.input_tokens);
    builder.append_comma();
    builder.append_key_value<"input_tokens_details">(obj.input_tokens_details);
    builder.append_comma();
    builder.append_key_value<"output_tokens">(obj.output_tokens);
    builder.append_comma();
    builder.append_key_value<"output_tokens_details">(obj.output_tokens_details);
    builder.append_comma();
    builder.append_key_value<"total_tokens">(obj.total_tokens);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::StreamOptions& obj) {
    builder.start_object();
    builder.append_key_value<"include_obfuscation">(obj.include_obfuscation);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig::FormatText& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig::FormatJsonSchema& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"schema">(obj.schema);
    builder.append_comma();
    builder.append_key_value<"description">(obj.description);
    builder.append_comma();
    builder.append_key_value<"strict">(obj.strict);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::TextConfig& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { builder.append_key_value<"format">(x); }, obj.format);
    builder.append_comma();
    builder.append_key_value<"verbosity">(obj.verbosity);
    builder.end_object();
}


/***
 * request::ContentTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::File& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_data", CommaDirection::BEFORE>(builder, obj.file_data);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"file_url",  CommaDirection::BEFORE>(builder, obj.file_url);
    AddOptKV<"filename",  CommaDirection::BEFORE>(builder, obj.filename);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Image& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"detail">(obj.detail);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::ContainerFileCitation& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"container_id">(obj.container_id);
    builder.append_comma();
    builder.append_key_value<"end_index">(obj.end_index);
    builder.append_comma();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"filename">(obj.filename);
    builder.append_comma();
    builder.append_key_value<"start_index">(obj.start_index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::FileCitation& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"filename">(obj.filename);
    builder.append_comma();
    builder.append_key_value<"index">(obj.index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::UrlCitation& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"end_index">(obj.end_index);
    builder.append_comma();
    builder.append_key_value<"start_index">(obj.start_index);
    builder.append_comma();
    builder.append_key_value<"title">(obj.title);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::FilePath& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"file_id">(obj.file_id);
    builder.append_comma();
    builder.append_key_value<"index">(obj.index);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::LogProb::TopLogprob& obj) {
    builder.start_object();
    builder.append_key_value<"bytes">(obj.bytes);
    builder.append_comma();
    builder.append_key_value<"logprob">(obj.logprob);
    builder.append_comma();
    builder.append_key_value<"token">(obj.token);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText::LogProb& obj) {
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

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::OutputText& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"annotations">(obj.annotations);
    builder.append_comma();
    builder.append_key_value<"logprobs">(obj.logprobs);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Refusal& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"refusal">(obj.refusal);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ContentTypes::Text& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}


/***
 * request::WebSearchToolActions
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Find& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"pattern">(obj.pattern);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::OpenPage& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Search::Source& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::Search& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"queries", CommaDirection::BEFORE>(builder, obj.queries);
    AddOptKV<"sources", CommaDirection::BEFORE>(builder, obj.sources);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::WebSearchToolActions::All& obj) {
    std::visit([&](auto const& x) { jai::llm::tag_invoke(serialize_tag{}, builder, x); }, obj);
}


/***
 * request::InputTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Message& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { builder.append_key_value<"content">(x); }, obj.content);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    AddOptKV<"type", CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::InputMessage& obj) {
    builder.start_object();
    builder.append_key_value<"content">(obj.content);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"type",   CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::OutputMessage& obj) {
    builder.start_object();
    builder.append_key_value<"content">(obj.content);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"role">(obj.role);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    AddOptKV<"type",   CommaDirection::BEFORE>(builder, obj.type);
    builder.end_object();
}


/***
 * request::ItemReference
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ItemReference& obj) {
    builder.start_object();
    AddOptKV<"type", CommaDirection::NONE>(builder, obj.type);
    if (obj.type) { builder.append_comma(); }
    builder.append_key_value<"id">(obj.id);
    builder.end_object();
}


/***
 * request::ToolCallItems
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FileSearchToolCall::Result& obj) {
    builder.start_object();
    AddOptKV<"attributes", CommaDirection::NONE>(builder, obj.attributes);
    AddOptKV<"file_id",    CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"filename",   CommaDirection::BEFORE>(builder, obj.filename);
    AddOptKV<"score",      CommaDirection::BEFORE>(builder, obj.score);
    AddOptKV<"text",       CommaDirection::BEFORE>(builder, obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FileSearchToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"queries">(obj.queries);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    AddOptKV<"results", CommaDirection::BEFORE>(builder, obj.results);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCall::PendingSafetyCheck& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"action">(x); }, obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"pending_safety_checks">(obj.pending_safety_checks);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput::ComputerScreenshot& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_id",   CommaDirection::BEFORE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput::AcknowledgedSafetyCheck& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    AddOptKV<"code",    CommaDirection::BEFORE>(builder, obj.code);
    AddOptKV<"message", CommaDirection::BEFORE>(builder, obj.message);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ComputerToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    AddOptKV<"acknowledged_safety_checks", CommaDirection::BEFORE>(builder, obj.acknowledged_safety_checks);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::WebSearchToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"action">(x); }, obj.action);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FunctionToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::FunctionToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"output">(x); }, obj.output);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning::Summary& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning::Content& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"text">(obj.text);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::Reasoning& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"summary">(obj.summary);
    AddOptKV<"content",           CommaDirection::BEFORE>(builder, obj.content);
    AddOptKV<"encrypted_content", CommaDirection::BEFORE>(builder, obj.encrypted_content);
    AddOptKV<"status",            CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CompactionItem& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"encrypted_content">(obj.encrypted_content);
    AddOptKV<"id", CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ImageGenerationCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"result">(obj.result);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputLog& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"logs">(obj.logs);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputImage& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"url">(obj.url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CodeInterpreterToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"code">(obj.code);
    builder.append_comma();
    builder.append_key_value<"container_id">(obj.container_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"outputs">(obj.outputs);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCall::Action& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"command">(obj.command);
    builder.append_comma();
    builder.append_key_value<"env">(obj.env);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    AddOptKV<"user",              CommaDirection::BEFORE>(builder, obj.user);
    AddOptKV<"working_directory", CommaDirection::BEFORE>(builder, obj.working_directory);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::LocalShellCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCall::Action& obj) {
    builder.start_object();
    builder.append_key_value<"commands">(obj.commands);
    AddOptKV<"max_output_length", CommaDirection::BEFORE>(builder, obj.max_output_length);
    AddOptKV<"timeout_ms",        CommaDirection::BEFORE>(builder, obj.timeout_ms);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"action">(obj.action);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"status", CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"exit_code">(obj.exit_code);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput::Output& obj) {
    builder.start_object();
    std::visit([&](auto const& x) { builder.append_key_value<"outcome">(x); }, obj.outcome);
    builder.append_comma();
    builder.append_key_value<"std_err">(obj.std_err);
    builder.append_comma();
    builder.append_key_value<"std_out">(obj.std_out);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ShellToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"output">(obj.output);
    AddOptKV<"id",                CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"max_output_length", CommaDirection::BEFORE>(builder, obj.max_output_length);
    AddOptKV<"status",            CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ApplyPatchToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"operation">(x); }, obj.operation);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    AddOptKV<"id", CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::ApplyPatchToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"status">(obj.status);
    AddOptKV<"call_id", CommaDirection::BEFORE>(builder, obj.call_id);
    AddOptKV<"id",      CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"output",  CommaDirection::BEFORE>(builder, obj.output);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPListTools::ToolDef& obj) {
    builder.start_object();
    builder.append_key_value<"input_schema">(obj.input_schema);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"annotations", CommaDirection::BEFORE>(builder, obj.annotations);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPListTools& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.append_comma();
    builder.append_key_value<"tools">(obj.tools);
    AddOptKV<"error", CommaDirection::BEFORE>(builder, obj.error);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPApprovalRequest& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPApprovalResponse& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"approval_request_id">(obj.approval_request_id);
    builder.append_comma();
    builder.append_key_value<"approve">(obj.approve);
    AddOptKV<"id",     CommaDirection::BEFORE>(builder, obj.id);
    AddOptKV<"reason", CommaDirection::BEFORE>(builder, obj.reason);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::MCPToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"arguments">(obj.arguments);
    builder.append_comma();
    builder.append_key_value<"id">(obj.id);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    AddOptKV<"approval_request_id", CommaDirection::BEFORE>(builder, obj.approval_request_id);
    AddOptKV<"error",               CommaDirection::BEFORE>(builder, obj.error);
    AddOptKV<"output",              CommaDirection::BEFORE>(builder, obj.output);
    AddOptKV<"status",              CommaDirection::BEFORE>(builder, obj.status);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CustomToolCallOutput& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"output">(x); }, obj.output);
    AddOptKV<"id", CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::InputTypes::Item::CustomToolCall& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"call_id">(obj.call_id);
    builder.append_comma();
    builder.append_key_value<"input">(obj.input);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"id", CommaDirection::BEFORE>(builder, obj.id);
    builder.end_object();
}


/***
 * request::Prompt
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::Prompt& obj) {
    builder.start_object();
    builder.append_key_value<"id">(obj.id);
    AddOptKV<"variables", CommaDirection::BEFORE>(builder, obj.variables);
    AddOptKV<"version",   CommaDirection::BEFORE>(builder, obj.version);
    builder.end_object();
}


/***
 * request::ToolTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Function& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.append_comma();
    builder.append_key_value<"parameters">(obj.parameters);
    builder.append_comma();
    builder.append_key_value<"strict">(obj.strict);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::ComparisonFilter& obj) {
    builder.start_object();
    builder.append_key_value<"key">(obj.key);
    builder.append_comma();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"value">(x); }, obj.value);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::CompoundFilter& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"filters">(obj.filters);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::RankingOptions::HybridSearch& obj) {
    builder.start_object();
    builder.append_key_value<"embedding_weight">(obj.embedding_weight);
    builder.append_comma();
    builder.append_key_value<"text_weight">(obj.text_weight);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch::RankingOptions& obj) {
    builder.start_object();
    AddOptKV<"hybrid_search",   CommaDirection::NONE>(builder, obj.hybrid_search);
    AddOptKV<"ranker",          CommaDirection::BEFORE>(builder, obj.ranker);
    AddOptKV<"score_threshold", CommaDirection::BEFORE>(builder, obj.score_threshold);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::FileSearch& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"vector_store_ids">(obj.vector_store_ids);
    AddOptKV<"filters",         CommaDirection::BEFORE>(builder, obj.filters);
    AddOptKV<"max_num_results", CommaDirection::BEFORE>(builder, obj.max_num_results);
    AddOptKV<"ranking_options", CommaDirection::BEFORE>(builder, obj.ranking_options);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ComputerUse& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"display_height">(obj.display_height);
    builder.append_comma();
    builder.append_key_value<"display_width">(obj.display_width);
    builder.append_comma();
    builder.append_key_value<"environment">(obj.environment);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch::Filters& obj) {
    builder.start_object();
    AddOptKV<"allowed_domains", CommaDirection::NONE>(builder, obj.allowed_domains);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch::Location& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearch& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"filters",             CommaDirection::BEFORE>(builder, obj.filters);
    AddOptKV<"search_context_size", CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",       CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
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

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::MCP& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
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
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"file_ids",     CommaDirection::BEFORE>(builder, obj.file_ids);
    AddOptKV<"memory_limit", CommaDirection::BEFORE>(builder, obj.memory_limit);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::CodeInterpreter& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    std::visit([&](auto const& x) { builder.append_key_value<"container">(x); }, obj.container);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ImageGeneration::Mask& obj) {
    builder.start_object();
    AddOptKV<"file_id",   CommaDirection::NONE>(builder, obj.file_id);
    AddOptKV<"image_url", CommaDirection::BEFORE>(builder, obj.image_url);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ImageGeneration& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
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
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Shell& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom::GrammarFormat& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"definition">(obj.definition);
    builder.append_comma();
    builder.append_key_value<"syntax">(obj.syntax);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom::TextFormat& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::Custom& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    AddOptKV<"description", CommaDirection::BEFORE>(builder, obj.description);
    AddOptKV<"format",      CommaDirection::BEFORE>(builder, obj.format);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearchPreview::Location& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"city",     CommaDirection::BEFORE>(builder, obj.city);
    AddOptKV<"country",  CommaDirection::BEFORE>(builder, obj.country);
    AddOptKV<"region",   CommaDirection::BEFORE>(builder, obj.region);
    AddOptKV<"timezone", CommaDirection::BEFORE>(builder, obj.timezone);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::WebSearchPreview& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"search_context_size", CommaDirection::BEFORE>(builder, obj.search_context_size);
    AddOptKV<"user_location",       CommaDirection::BEFORE>(builder, obj.user_location);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolTypes::ApplyPatch& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}


/***
 * request::ToolsChoiceTypes
 */
void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Allowed::RestrictedTool& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    AddOptKV<"name",         CommaDirection::BEFORE>(builder, obj.name);
    AddOptKV<"server_label", CommaDirection::BEFORE>(builder, obj.server_label);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Allowed& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"mode">(obj.mode);
    builder.append_comma();
    builder.append_key_value<"tools">(obj.tools);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Custom& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Function& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"name">(obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::Hosted& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::MCP& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.append_comma();
    builder.append_key_value<"server_label">(obj.server_label);
    AddOptKV<"name", CommaDirection::BEFORE>(builder, obj.name);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::SpecificApplyPatch& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
    builder.end_object();
}

void tag_invoke(serialize_tag, string_builder& builder, const openai::request::ToolsChoiceTypes::SpecificShell& obj) {
    builder.start_object();
    builder.append_key_value<"type">(obj.type);
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


/***
 * Top-level Request
 */

void tag_invoke(serialize_tag, string_builder& builder, const openai::Request& obj) {
    builder.start_object();
    AddOptKV<"background",             CommaDirection::NONE>(builder, obj.background);
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


} // namespace jai::llm


namespace jai::llm::openai {


std::vector<std::byte> Serialize(const Request& request) {
    static thread_local simdjson::builder::string_builder builder{};

    builder.clear();
    jai::llm::tag_invoke(serialize_tag{}, builder, request);
    builder.validate_unicode();
    std::string_view json_str = builder.view();

    return json_str |
           std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
           std::ranges::to<std::vector<std::byte>>();
}


}
