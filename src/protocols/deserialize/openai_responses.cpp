#include <map>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../../../interface/protocols/openai/responses.hpp"
#include "../../../interface//core/error.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


/***
 * ComputerToolActions
 */
BEGIN_PARSE(openai::ComputerToolActions::Click)
    FIELD(src, type),
    FIELD(src, button),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::DoubleClick)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Drag::Coordinate)
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Drag)
    FIELD(src, type),
    FIELD(src, path)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::KeyPress)
    FIELD(src, type),
    FIELD(src, keys)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Move)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Screenshot)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Scroll)
    FIELD(src, type),
    FIELD(src, scroll_x),
    FIELD(src, scroll_y),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Type)
    FIELD(src, type),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(openai::ComputerToolActions::Wait)
    FIELD(src, type)
END_PARSE

template <>
openai::ComputerToolActions::All Parse<openai::ComputerToolActions::All>(const simdjson::dom::element& src) {
    using T = openai::ComputerToolActions::All;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ComputerActionType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::ComputerToolActions type: "} + std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ComputerActionType::CLICK:        return T{Parse<openai::ComputerToolActions::Click>(src)};
    case openai::ComputerActionType::DOUBLE_CLICK: return T{Parse<openai::ComputerToolActions::DoubleClick>(src)};
    case openai::ComputerActionType::DRAG:         return T{Parse<openai::ComputerToolActions::Drag>(src)};
    case openai::ComputerActionType::KEYPRESS:     return T{Parse<openai::ComputerToolActions::KeyPress>(src)};
    case openai::ComputerActionType::MOVE:         return T{Parse<openai::ComputerToolActions::Move>(src)};
    case openai::ComputerActionType::SCREENSHOT:   return T{Parse<openai::ComputerToolActions::Screenshot>(src)};
    case openai::ComputerActionType::SCROLL:       return T{Parse<openai::ComputerToolActions::Scroll>(src)};
    case openai::ComputerActionType::TYPE:         return T{Parse<openai::ComputerToolActions::Type>(src)};
    case openai::ComputerActionType::WAIT:         return T{Parse<openai::ComputerToolActions::Wait>(src)};
    default: throw AnnotatedException{"openai::ComputerToolActions variant unsatisfied"};
    }
}

/***
 * PatchFileOperations
 */
BEGIN_PARSE(openai::PatchFileOperations::Create)
    FIELD(src, type),
    FIELD(src, diff),
    FIELD(src, path)
END_PARSE

BEGIN_PARSE(openai::PatchFileOperations::Delete)
    FIELD(src, type),
    FIELD(src, path)
END_PARSE

BEGIN_PARSE(openai::PatchFileOperations::Update)
    FIELD(src, type),
    FIELD(src, diff),
    FIELD(src, path)
END_PARSE


/***
 * Other Common Structures
 */
BEGIN_PARSE(openai::ConversationRef)
    FIELD(src, id)
END_PARSE

BEGIN_PARSE(openai::IncompleteDetails)
    FIELD(src, reason)
END_PARSE

BEGIN_PARSE(openai::Reasoning)
    FIELD(src, effort),
    FIELD(src, summary)
END_PARSE

BEGIN_PARSE(openai::ResponseError)
    FIELD(src, code),
    FIELD(src, message)
END_PARSE

BEGIN_PARSE(openai::ResponseUsage::InputTokenDetails)
    FIELD(src, cached_tokens)
END_PARSE

BEGIN_PARSE(openai::ResponseUsage::OutputTokenDetails)
    FIELD(src, reasoning_tokens)
END_PARSE

BEGIN_PARSE(openai::ResponseUsage)
    FIELD(src, input_tokens),
    FIELD(src, input_tokens_details),
    FIELD(src, output_tokens),
    FIELD(src, output_tokens_details),
    FIELD(src, total_tokens)
END_PARSE

BEGIN_PARSE(openai::StreamOptions)
    FIELD(src, include_obfuscation)
END_PARSE

/***
 * TextConfig
 */
BEGIN_PARSE(openai::TextConfig::FormatText)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::TextConfig::FormatJsonSchema)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, schema),
    FIELD(src, description),
    FIELD(src, strict)
END_PARSE

template <>
openai::TextConfig::Format Parse<openai::TextConfig::Format>(const simdjson::dom::element& src) {
    using T = openai::TextConfig::Format;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ResponseFormatType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::TextConfig::Format type: "} + std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ResponseFormatType::TEXT:        return T{Parse<openai::TextConfig::FormatText>(src)};
    case openai::ResponseFormatType::JSON_SCHEMA: return T{Parse<openai::TextConfig::FormatJsonSchema>(src)};
    default: throw AnnotatedException{"openai::TextConfig::Format variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::TextConfig)
    FIELD(src, format),
    FIELD(src, verbosity)
END_PARSE


/***
 * ContentTypes
 */
BEGIN_PARSE(openai::response::ContentTypes::File)
    FIELD(src, type),
    FIELD(src, file_data),
    FIELD(src, file_id),
    FIELD(src, file_url),
    FIELD(src, filename)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::Image)
    FIELD(src, type),
    FIELD(src, detail),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::ContainerFileCitation)
    FIELD(src, type),
    FIELD(src, container_id),
    FIELD(src, end_index),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, start_index)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::FileCitation)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, index)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::UrlCitation)
    FIELD(src, type),
    FIELD(src, end_index),
    FIELD(src, start_index),
    FIELD(src, title),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::FilePath)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, index)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::LogProb::TopLogprob)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::OutputText::LogProb)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token),
    FIELD(src, top_logprobs)
END_PARSE

template <>
openai::response::ContentTypes::OutputText::Annotation
    Parse<openai::response::ContentTypes::OutputText::Annotation>(const simdjson::dom::element& src)
{
    using BaseT = openai::response::ContentTypes::OutputText;
    using T = BaseT::Annotation;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::AnnotationType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::ContentTypes::OutputText::Annotation type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::AnnotationType::FILE_CITATION:           return T{Parse<BaseT::FileCitation>(src)};
    case openai::AnnotationType::URL_CITATION:            return T{Parse<BaseT::UrlCitation>(src)};
    case openai::AnnotationType::CONTAINER_FILE_CITATION: return T{Parse<BaseT::ContainerFileCitation>(src)};
    case openai::AnnotationType::FILE_PATH:               return T{Parse<BaseT::FilePath>(src)};
    default: throw AnnotatedException{"openai::response::ContentTypes::OutputText::Annotation variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::ContentTypes::OutputText)
    FIELD(src, type),
    FIELD(src, annotations),
    FIELD(src, logprobs),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::Refusal)
    FIELD(src, type),
    FIELD(src, refusal)
END_PARSE

BEGIN_PARSE(openai::response::ContentTypes::Text)
    FIELD(src, type),
    FIELD(src, text)
END_PARSE


/***
 * WebSearchToolActions
 */
BEGIN_PARSE(openai::response::WebSearchToolActions::Find)
    FIELD(src, type),
    FIELD(src, pattern),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::response::WebSearchToolActions::OpenPage)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::response::WebSearchToolActions::Search::Source)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::response::WebSearchToolActions::Search)
    FIELD(src, type),
    FIELD(src, queries),
    FIELD(src, sources)
END_PARSE

template <>
openai::response::WebSearchToolActions::All Parse<openai::response::WebSearchToolActions::All>(const simdjson::dom::element& src) {
    using T = openai::response::WebSearchToolActions::All;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::WebSearchActionType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::WebSearchToolActions type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::WebSearchActionType::FIND:      return T{Parse<openai::response::WebSearchToolActions::Find>(src)};
    case openai::WebSearchActionType::OPEN_PAGE: return T{Parse<openai::response::WebSearchToolActions::OpenPage>(src)};
    case openai::WebSearchActionType::SEARCH:    return T{Parse<openai::response::WebSearchToolActions::Search>(src)};
    default: throw AnnotatedException{"openai::response::WebSearchToolActions variant unsatisfied"};
    }
}


/***
 * InputTypes
 */
template <>
openai::response::InputTypes::Message::Content
    Parse<openai::response::InputTypes::Message::Content>(const simdjson::dom::element& src)
{
    using T = openai::response::InputTypes::Message::Content;
    if (src.is_string()) { return T{Parse<std::string>(src)}; }
    else if (src.is_array()) { return T{ParseArrayOf<openai::response::InputTypes::MessageContentUnit>(src)}; }
    throw AnnotatedException{"Invalid InputTypes::Message::Content"};
}

template <>
openai::response::InputTypes::MessageContentUnit
    Parse<openai::response::InputTypes::MessageContentUnit>(const simdjson::dom::element& src)
{
    using T = openai::response::InputTypes::MessageContentUnit;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ContentType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::InputTypes::MessageContentUnit type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ContentType::INPUT_TEXT:  return T{Parse<openai::response::ContentTypes::Text>(src)};
    case openai::ContentType::INPUT_IMAGE: return T{Parse<openai::response::ContentTypes::Image>(src)};
    case openai::ContentType::INPUT_FILE:  return T{Parse<openai::response::ContentTypes::File>(src)};
    default: throw AnnotatedException{"openai::response::InputTypes::MessageContentUnit variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::InputTypes::Message)
    FIELD(src, content),
    FIELD(src, role),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::InputMessage)
    FIELD(src, content),
    FIELD(src, role),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

template <>
openai::response::InputTypes::Item::OutputMessage::Content
    Parse<openai::response::InputTypes::Item::OutputMessage::Content>(const simdjson::dom::element& src)
{
    using T = openai::response::InputTypes::Item::OutputMessage::Content;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::OutputMessageContentType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::InputTypes::Item::OutputMessage::Content type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::OutputMessageContentType::OUTPUT_TEXT: return T{Parse<openai::response::ContentTypes::OutputText>(src)};
    case openai::OutputMessageContentType::REFUSAL:     return T{Parse<openai::response::ContentTypes::Refusal>(src)};
    default: throw AnnotatedException{"openai::response::InputTypes::Item::OutputMessage::Content variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::InputTypes::Item::OutputMessage)
    FIELD(src, content),
    FIELD(src, id),
    FIELD(src, role),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::FileSearchToolCall::Result)
    FIELD(src, attributes),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, score),
    FIELD(src, text)
END_PARSE

template <>
std::variant<NameLen<512>, bool, double> Parse<std::variant<NameLen<512>, bool, double>>(const simdjson::dom::element& src) {
    using T = std::variant<NameLen<512>, bool, double>;
    switch (src.type()) {
    case simdjson::dom::element_type::BOOL:   return T{Parse<bool>(src)};
    case simdjson::dom::element_type::UINT64:
    case simdjson::dom::element_type::INT64:
    case simdjson::dom::element_type::DOUBLE: return T{Parse<double>(src)};
    case simdjson::dom::element_type::STRING: return T{Parse<NameLen<512>>(src)};
    default: throw AnnotatedException{"Unexpected type in FileSearch result attributes."};
    }
}

template <>
std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>> 
Parse<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>>(const simdjson::dom::element& src) {
    return src.get_object() |
           std::views::transform([](auto&& kv) {
               auto const& [key, value] = kv;
               return std::pair{NameLen<64>{std::string{key}}, Parse<std::variant<NameLen<512>, bool, double>>(value)};
           }) |
           std::ranges::to<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>>();
}

BEGIN_PARSE(openai::response::InputTypes::Item::FileSearchToolCall)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, queries),
    FIELD(src, status),
    FIELD(src, results)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ComputerToolCall::PendingSafetyCheck)
    FIELD(src, id),
    FIELD(src, code),
    FIELD(src, message)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ComputerToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, pending_safety_checks),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ComputerToolCallOutput::ComputerScreenshot)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ComputerToolCallOutput::AcknowledgedSafetyCheck)
    FIELD(src, id),
    FIELD(src, code),
    FIELD(src, message)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ComputerToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, acknowledged_safety_checks),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::WebSearchToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::FunctionToolCall)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, call_id),
    FIELD(src, name),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::FunctionToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::Reasoning::Summary)
    FIELD(src, type),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::Reasoning::Content)
    FIELD(src, type),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::Reasoning)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, summary),
    FIELD(src, content),
    FIELD(src, encrypted_content),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::CompactionItem)
    FIELD(src, type),
    FIELD(src, encrypted_content),
    FIELD(src, id),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ImageGenerationCall)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, result),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputLog)
    FIELD(src, type),
    FIELD(src, logs)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputImage)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

template <>
openai::response::InputTypes::Item::CodeInterpreterToolCall::Output
    Parse<openai::response::InputTypes::Item::CodeInterpreterToolCall::Output>(const simdjson::dom::element& src)
{
    using BaseT = openai::response::InputTypes::Item::CodeInterpreterToolCall;
    using T = BaseT::Output;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::CodeInterpreterOutputType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::InputTypes::Item::CodeInterpreterToolCall::Output type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::CodeInterpreterOutputType::LOGS:  return T{Parse<BaseT::CodeInterpreterOutputLog>(src)};
    case openai::CodeInterpreterOutputType::IMAGE: return T{Parse<BaseT::CodeInterpreterOutputImage>(src)};
    default: throw AnnotatedException{"openai::response::InputTypes::Item::CodeInterpreterToolCall::Output variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::InputTypes::Item::CodeInterpreterToolCall)
    FIELD(src, type),
    FIELD(src, code),
    FIELD(src, container_id),
    FIELD(src, id),
    FIELD(src, outputs),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::LocalShellCall::Action)
    FIELD(src, type),
    FIELD(src, command),
    FIELD(src, env),
    FIELD(src, timeout_ms),
    FIELD(src, user),
    FIELD(src, working_directory)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::LocalShellCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::LocalShellCallOutput)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, output),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCall::Action)
    FIELD(src, commands),
    FIELD(src, max_output_length),
    FIELD(src, timeout_ms)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome)
    FIELD(src, type),
    FIELD(src, exit_code)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome)
    FIELD(src, type)
END_PARSE

template <>
openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome
    Parse<openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome>(const simdjson::dom::element& src)
{
    using T = openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ShellCallOutcomeType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ShellCallOutcomeType::EXIT:
        return T{Parse<openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome>(src)};
    case openai::ShellCallOutcomeType::TIMEOUT:
        return T{Parse<openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome>(src)};
    default:
        throw AnnotatedException{"openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCallOutput::Output)
    FIELD(src, outcome),
    Extract<"stderr",
            openai::response::InputTypes::Item::ShellToolCallOutput::Output,
            &openai::response::InputTypes::Item::ShellToolCallOutput::Output::std_err>(src),
    Extract<"stdout",
            openai::response::InputTypes::Item::ShellToolCallOutput::Output,
            &openai::response::InputTypes::Item::ShellToolCallOutput::Output::std_out>(src),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ShellToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id),
    FIELD(src, max_output_length),
    FIELD(src, status),
    FIELD(src, created_by)
END_PARSE

template <>
openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation 
    Parse<openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation>(const simdjson::dom::element& src)
{
    using T = openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ApplyPatchOperationType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ApplyPatchOperationType::CREATE_FILE: return T{Parse<openai::PatchFileOperations::Create>(src)};
    case openai::ApplyPatchOperationType::DELETE_FILE: return T{Parse<openai::PatchFileOperations::Delete>(src)};
    case openai::ApplyPatchOperationType::UPDATE_FILE: return T{Parse<openai::PatchFileOperations::Update>(src)};
    default: throw AnnotatedException{"openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::InputTypes::Item::ApplyPatchToolCall)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, operation),
    FIELD(src, status),
    FIELD(src, id),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::ApplyPatchToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, status),
    FIELD(src, id),
    FIELD(src, output),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::MCPListTools::ToolDef)
    FIELD(src, input_schema),
    FIELD(src, name),
    FIELD(src, annotations),
    FIELD(src, description)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::MCPListTools)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, server_label),
    FIELD(src, tools),
    FIELD(src, error)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::MCPApprovalRequest)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::MCPApprovalResponse)
    FIELD(src, type),
    FIELD(src, approval_request_id),
    FIELD(src, approve),
    FIELD(src, id),
    FIELD(src, reason)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::MCPToolCall)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label),
    FIELD(src, approval_request_id),
    FIELD(src, error),
    FIELD(src, output),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::CustomToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id)
END_PARSE

BEGIN_PARSE(openai::response::InputTypes::Item::CustomToolCall)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, input),
    FIELD(src, name),
    FIELD(src, id)
END_PARSE


/***
 * ItemReference & Prompt
 */
BEGIN_PARSE(openai::response::ItemReference)
    FIELD(src, type),
    FIELD(src, id)
END_PARSE

template <>
openai::response::Prompt::VariableTypes Parse<openai::response::Prompt::VariableTypes>(const simdjson::dom::element& src) {
    using T = openai::response::Prompt::VariableTypes;
    if (src.is_string()) { return T{Parse<std::string>(src)}; }
    
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ContentType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::Prompt::VariableTypes type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ContentType::INPUT_TEXT:  return T{Parse<openai::response::ContentTypes::Text>(src)};
    case openai::ContentType::INPUT_IMAGE: return T{Parse<openai::response::ContentTypes::Image>(src)};
    case openai::ContentType::INPUT_FILE:  return T{Parse<openai::response::ContentTypes::File>(src)};
    default: throw AnnotatedException{"openai::response::Prompt::VariableTypes variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::Prompt)
    FIELD(src, id),
    FIELD(src, variables),
    FIELD(src, version)
END_PARSE


/***
 * ToolTypes
 */
BEGIN_PARSE(openai::response::ToolTypes::Function)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, parameters),
    FIELD(src, strict),
    FIELD(src, description)
END_PARSE

template <>
openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType 
    Parse<openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType>(const simdjson::dom::element& src)
{
    using T = openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType;
    switch (src.type()) {
    case simdjson::dom::element_type::STRING: return T{Parse<std::string>(src)};
    case simdjson::dom::element_type::DOUBLE:
    case simdjson::dom::element_type::INT64:
    case simdjson::dom::element_type::UINT64: return T{Parse<double>(src)};
    case simdjson::dom::element_type::BOOL:   return T{Parse<bool>(src)};
    case simdjson::dom::element_type::ARRAY: {
        auto arr = src.get_array();
        if (arr.size() == 0) return T{std::vector<std::string>{}};
        auto first = arr.at(0);
        if      (first.is_string()) return T{ParseArrayOf<std::string>(src)};
        else if (first.is_number()) return T{ParseArrayOf<double>(src)};
        else if (first.is_bool())   return T{ParseArrayOf<bool>(src)};
    }
    default: throw AnnotatedException{"openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::ToolTypes::FileSearch::ComparisonFilter)
    FIELD(src, key),
    FIELD(src, type),
    FIELD(src, value)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::FileSearch::CompoundFilter)
    FIELD(src, type),
    FIELD(src, filters)
END_PARSE

template <>
std::variant<openai::response::ToolTypes::FileSearch::ComparisonFilter,
             openai::response::ToolTypes::FileSearch::CompoundFilter> 
    Parse<std::variant<openai::response::ToolTypes::FileSearch::ComparisonFilter,
                       openai::response::ToolTypes::FileSearch::CompoundFilter>>(const simdjson::dom::element& src)
{
    using T = std::variant<openai::response::ToolTypes::FileSearch::ComparisonFilter,
                           openai::response::ToolTypes::FileSearch::CompoundFilter>;
    auto obj = src.get_object();
    if (obj["filters"].error() == simdjson::SUCCESS) {
        return T{Parse<openai::response::ToolTypes::FileSearch::CompoundFilter>(src)};
    } else {
        return T{Parse<openai::response::ToolTypes::FileSearch::ComparisonFilter>(src)};
    }
}

BEGIN_PARSE(openai::response::ToolTypes::FileSearch::RankingOptions::HybridSearch)
    FIELD(src, embedding_weight),
    FIELD(src, text_weight)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::FileSearch::RankingOptions)
    FIELD(src, hybrid_search),
    FIELD(src, ranker),
    FIELD(src, score_threshold)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::FileSearch)
    FIELD(src, type),
    FIELD(src, vector_store_ids),
    FIELD(src, filters),
    FIELD(src, max_num_results),
    FIELD(src, ranking_options)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::ComputerUse)
    FIELD(src, type),
    FIELD(src, display_height),
    FIELD(src, display_width),
    FIELD(src, environment)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::WebSearch::Filters)
    FIELD(src, allowed_domains)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::WebSearch::Location)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::WebSearch)
    FIELD(src, type),
    FIELD(src, filters),
    FIELD(src, search_context_size),
    FIELD(src, user_location)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::MCP::Filter)
    FIELD(src, read_only),
    FIELD(src, tool_names)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::MCP::ApprovalFilter)
    FIELD(src, always),
    FIELD(src, never)
END_PARSE

template <>
openai::response::ToolTypes::MCP::AllowedTools
    Parse<openai::response::ToolTypes::MCP::AllowedTools>(const simdjson::dom::element& src)
{
    using T = openai::response::ToolTypes::MCP::AllowedTools;
    if (src.is_array()) {
        return T{ParseArrayOf<std::string>(src)};
    }
    return T{Parse<openai::response::ToolTypes::MCP::Filter>(src)};
}

template <>
openai::response::ToolTypes::MCP::RequiredApproval
    Parse<openai::response::ToolTypes::MCP::RequiredApproval>(const simdjson::dom::element& src)
{
    using T = openai::response::ToolTypes::MCP::RequiredApproval;
    if (src.is_string()) {
        auto sv = src.get_string().value();
        auto opt = from_string_view<openai::MCPApprovalSetting>(sv);
        if (opt) return T{*opt};
        throw AnnotatedException{std::string{"openai::MCPApprovalSetting missing type: "} + std::string{sv}};
    }
    return T{Parse<openai::response::ToolTypes::MCP::ApprovalFilter>(src)};
}

BEGIN_PARSE(openai::response::ToolTypes::MCP)
    FIELD(src, type),
    FIELD(src, server_label),
    FIELD(src, allowed_tools),
    FIELD(src, authorization),
    FIELD(src, connector_id),
    FIELD(src, headers),
    FIELD(src, require_approval),
    FIELD(src, server_description),
    FIELD(src, server_url)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::CodeInterpreter::ContainerConfig)
    FIELD(src, type),
    FIELD(src, file_ids),
    FIELD(src, memory_limit)
END_PARSE

template <>
openai::response::ToolTypes::CodeInterpreter::Container
    Parse<openai::response::ToolTypes::CodeInterpreter::Container>(const simdjson::dom::element& src)
{
    using T = openai::response::ToolTypes::CodeInterpreter::Container;
    if (src.is_string()) {
        return T{Parse<std::string>(src)};
    }
    return T{Parse<openai::response::ToolTypes::CodeInterpreter::ContainerConfig>(src)};
}

BEGIN_PARSE(openai::response::ToolTypes::CodeInterpreter)
    FIELD(src, type),
    FIELD(src, container)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::ImageGeneration::Mask)
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::ImageGeneration)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, background),
    FIELD(src, input_fidelity),
    FIELD(src, input_image_mask),
    FIELD(src, model),
    FIELD(src, moderation),
    FIELD(src, output_compression),
    FIELD(src, output_format),
    FIELD(src, partial_images),
    FIELD(src, quality),
    FIELD(src, size)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::LocalShell)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::Shell)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::Custom::GrammarFormat)
    FIELD(src, type),
    FIELD(src, definition),
    FIELD(src, syntax)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::Custom::TextFormat)
    FIELD(src, type)
END_PARSE

template <>
openai::response::ToolTypes::Custom::Format
    Parse<openai::response::ToolTypes::Custom::Format>(const simdjson::dom::element& src)
{
    using T = openai::response::ToolTypes::Custom::Format;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::CustomToolFormatType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::ToolTypes::Custom::Format type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::CustomToolFormatType::GRAMMAR: return T{Parse<openai::response::ToolTypes::Custom::GrammarFormat>(src)};
    case openai::CustomToolFormatType::TEXT:    return T{Parse<openai::response::ToolTypes::Custom::TextFormat>(src)};
    default: throw AnnotatedException{"openai::response::ToolTypes::Custom::Format variant unsatisfied"};
    }
}

BEGIN_PARSE(openai::response::ToolTypes::Custom)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, description),
    FIELD(src, format)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::WebSearchPreview::Location)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::WebSearchPreview)
    FIELD(src, type),
    FIELD(src, search_context_size),
    FIELD(src, user_location)
END_PARSE

BEGIN_PARSE(openai::response::ToolTypes::ApplyPatch)
    FIELD(src, type)
END_PARSE


/***
 * ToolsChoiceTypes
 */
BEGIN_PARSE(openai::response::ToolsChoiceTypes::Allowed::RestrictedTool)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, server_label)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::Allowed)
    FIELD(src, type),
    FIELD(src, mode),
    FIELD(src, tools)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::Custom)
    FIELD(src, type),
    FIELD(src, name)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::Function)
    FIELD(src, type),
    FIELD(src, name)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::Hosted)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::MCP)
    FIELD(src, type),
    FIELD(src, server_label),
    FIELD(src, name)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::SpecificApplyPatch)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::response::ToolsChoiceTypes::SpecificShell)
    FIELD(src, type)
END_PARSE


/***
 * Top-level Variants
 */
template <>
openai::response::Item Parse<openai::response::Item>(const simdjson::dom::element& src) {
    using T = openai::response::Item;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::OutputItemType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::Item type: "} + std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::OutputItemType::MESSAGE:                 return T{Parse<openai::response::InputTypes::Item::OutputMessage>(src)};
    case openai::OutputItemType::FILE_SEARCH_CALL:        return T{Parse<openai::response::InputTypes::Item::FileSearchToolCall>(src)};
    case openai::OutputItemType::COMPUTER_CALL:           return T{Parse<openai::response::InputTypes::Item::ComputerToolCall>(src)};
    case openai::OutputItemType::WEB_SEARCH_CALL:         return T{Parse<openai::response::InputTypes::Item::WebSearchToolCall>(src)};
    case openai::OutputItemType::FUNCTION_CALL:           return T{Parse<openai::response::InputTypes::Item::FunctionToolCall>(src)};
    case openai::OutputItemType::REASONING:               return T{Parse<openai::response::InputTypes::Item::Reasoning>(src)};
    case openai::OutputItemType::COMPACTION:              return T{Parse<openai::response::InputTypes::Item::CompactionItem>(src)};
    case openai::OutputItemType::IMAGE_GENERATION_CALL:   return T{Parse<openai::response::InputTypes::Item::ImageGenerationCall>(src)};
    case openai::OutputItemType::CODE_INTERPRETER_CALL:   return T{Parse<openai::response::InputTypes::Item::CodeInterpreterToolCall>(src)};
    case openai::OutputItemType::LOCAL_SHELL_CALL:        return T{Parse<openai::response::InputTypes::Item::LocalShellCall>(src)};
    case openai::OutputItemType::SHELL_CALL:              return T{Parse<openai::response::InputTypes::Item::ShellToolCall>(src)};
    case openai::OutputItemType::SHELL_CALL_OUTPUT:       return T{Parse<openai::response::InputTypes::Item::ShellToolCallOutput>(src)};
    case openai::OutputItemType::APPLY_PATCH_CALL:        return T{Parse<openai::response::InputTypes::Item::ApplyPatchToolCall>(src)};
    case openai::OutputItemType::APPLY_PATCH_CALL_OUTPUT: return T{Parse<openai::response::InputTypes::Item::ApplyPatchToolCallOutput>(src)};
    case openai::OutputItemType::MCP_LIST_TOOLS:          return T{Parse<openai::response::InputTypes::Item::MCPListTools>(src)};
    case openai::OutputItemType::MCP_APPROVAL_REQUEST:    return T{Parse<openai::response::InputTypes::Item::MCPApprovalRequest>(src)};
    case openai::OutputItemType::MCP_CALL:                return T{Parse<openai::response::InputTypes::Item::MCPToolCall>(src)};
    case openai::OutputItemType::CUSTOM_TOOL_CALL:        return T{Parse<openai::response::InputTypes::Item::CustomToolCall>(src)};
    default: throw AnnotatedException{"openai::response::Item variant unsatisfied."};
    }
}

template <>
openai::response::Tool Parse<openai::response::Tool>(const simdjson::dom::element& src) {
    using T = openai::response::Tool;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ToolType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::Tool type: "} + std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ToolType::FUNCTION:             return T{Parse<openai::response::ToolTypes::Function>(src)};
    case openai::ToolType::FILE_SEARCH:          return T{Parse<openai::response::ToolTypes::FileSearch>(src)};
    case openai::ToolType::COMPUTER_USE_PREVIEW: return T{Parse<openai::response::ToolTypes::ComputerUse>(src)};
    case openai::ToolType::WEB_SEARCH:           return T{Parse<openai::response::ToolTypes::WebSearch>(src)};
    case openai::ToolType::MCP:                  return T{Parse<openai::response::ToolTypes::MCP>(src)};
    case openai::ToolType::CODE_INTERPRETER:     return T{Parse<openai::response::ToolTypes::CodeInterpreter>(src)};
    case openai::ToolType::IMAGE_GENERATION:     return T{Parse<openai::response::ToolTypes::ImageGeneration>(src)};
    case openai::ToolType::LOCAL_SHELL:          return T{Parse<openai::response::ToolTypes::LocalShell>(src)};
    case openai::ToolType::SHELL:                return T{Parse<openai::response::ToolTypes::Shell>(src)};
    case openai::ToolType::CUSTOM:               return T{Parse<openai::response::ToolTypes::Custom>(src)};
    case openai::ToolType::WEB_SEARCH_PREVIEW:   return T{Parse<openai::response::ToolTypes::WebSearchPreview>(src)};
    case openai::ToolType::APPLY_PATCH:          return T{Parse<openai::response::ToolTypes::ApplyPatch>(src)};
    default: throw AnnotatedException{"openai::response::Tool variant unsatisfied."};
    }
}

template <>
openai::response::OutputItemList Parse<openai::response::OutputItemList>(const simdjson::dom::element& src) {
    using T = openai::response::OutputItemList;
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::OutputItemType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::response::OutputItemList type: "} +
                                 std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::OutputItemType::MESSAGE:        return T{Parse<openai::response::InputTypes::Message>(src)};
    case openai::OutputItemType::ITEM_REFERENCE: return T{Parse<openai::response::ItemReference>(src)};
    default:                                     return T{Parse<openai::response::Item>(src)};
    }
}

template <>
openai::response::Output Parse<openai::response::Output>(const simdjson::dom::element& src) {
    using T = openai::response::Output;
    if (src.is_string()) return T{Parse<std::string>(src)};
    return T{ParseArrayOf<openai::response::OutputItemList>(src)};
}

template <>
openai::response::ToolChoice Parse<openai::response::ToolChoice>(const simdjson::dom::element& src) {
    using T = openai::response::ToolChoice;
    if (src.is_string()) {
        auto type_sv = src.get_string().value();
        auto opt_mode = from_string_view<openai::ToolChoiceMode>(type_sv);
        if (opt_mode) return T{*opt_mode};
        throw AnnotatedException{std::string{"openai::response::ToolChoice missing type: "} + std::string{type_sv}};
    }
    
    auto obj = src.get_object();
    auto type_sv = obj["type"].get_string().value();
    auto opt_kind = from_string_view<openai::ToolChoiceType>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{std::string{"Unexpected openai::ToolChoiceType type: "} + std::string{type_sv}};
    }

    switch (*opt_kind) {
    case openai::ToolChoiceType::ALLOWED_TOOLS: return T{Parse<openai::response::ToolsChoiceTypes::Allowed>(src)};
    case openai::ToolChoiceType::CUSTOM:        return T{Parse<openai::response::ToolsChoiceTypes::Custom>(src)};
    case openai::ToolChoiceType::FUNCTION:      return T{Parse<openai::response::ToolsChoiceTypes::Function>(src)};
    case openai::ToolChoiceType::MCP:           return T{Parse<openai::response::ToolsChoiceTypes::MCP>(src)};
    case openai::ToolChoiceType::APPLY_PATCH:   return T{Parse<openai::response::ToolsChoiceTypes::SpecificApplyPatch>(src)};
    case openai::ToolChoiceType::SHELL:         return T{Parse<openai::response::ToolsChoiceTypes::SpecificShell>(src)};
    }

    auto opt_hosted = from_string_view<openai::HostedToolMode>(type_sv);
    if (opt_hosted) {
        return T{openai::response::ToolsChoiceTypes::Hosted{*opt_hosted}};
    }

    throw AnnotatedException{"openai::response::ToolChoice variant unsatisfied."};
}


/***
 * Top-level Response
 */
template <>
std::map<NameLen<64>, NameLen<512>> Parse<std::map<NameLen<64>, NameLen<512>>>(const simdjson::dom::element& src) {
    return src.get_object() |
           std::views::transform([](auto&& kv) {
               auto const& [key, value] = kv;
               return std::pair{NameLen<64>{std::string{key}}, NameLen<512>{Parse<std::string>(value)}};
           }) |
           std::ranges::to<std::map<NameLen<64>, NameLen<512>>>();
}

BEGIN_PARSE(openai::Response)
    FIELD(src, object),
    FIELD(src, background),
    FIELD(src, completed_at),
    FIELD(src, conversation),
    FIELD(src, created_at),
    FIELD(src, error),
    FIELD(src, id),
    FIELD(src, incomplete_details),
    FIELD(src, instructions),
    FIELD(src, max_output_tokens),
    FIELD(src, max_tool_calls),
    FIELD(src, metadata),
    FIELD(src, model),
    FIELD(src, output),
    FIELD(src, parallel_tool_calls),
    FIELD(src, previous_response_id),
    FIELD(src, prompt),
    FIELD(src, prompt_cache_key),
    FIELD(src, prompt_cache_retention),
    FIELD(src, reasoning),
    FIELD(src, safety_identifier),
    FIELD(src, service_tier),
    FIELD(src, status),
    FIELD(src, temperature),
    FIELD(src, text),
    FIELD(src, tool_choice),
    FIELD(src, tools),
    FIELD(src, top_logprobs),
    FIELD(src, top_p),
    FIELD(src, truncation),
    FIELD(src, usage)
END_PARSE


} // namespace jai::llm


#undef FIELD
#undef BEGIN_PARSE
#undef END_PARSE


/***
 * Top-level Deserialize
 */
namespace jai::llm::openai {


Response Deserialize(const curl::Response& response) {
    if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
        throw AnnotatedException{"Simdjson padding check failed"};
    }

    static thread_local simdjson::dom::parser parser{};
    simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()), response.body_len);

    Response out = Parse<Response>(doc);
    return out;
}


}
