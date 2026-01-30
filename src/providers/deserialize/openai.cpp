#include <map>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "../../../interface/protocols/openai/responses.hpp"
#include "../../../interface/protocols/openai/strings.hpp" // must include before base.hpp
#include "base.hpp"
#include "../../curl.hpp"


/***
 * Local defined MACROs for source file only.
 */
#define FIELD(src, member) Extract<#member, T, &T::member>((src))
#define BEGIN_PARSE(Type)                             \
template <>                                           \
Type Parse<Type>(const simdjson::dom::element& src) { \
    using T = Type;                                   \
    return T{
#define  END_PARSE \
    };             \
}


namespace jai::llm {


/***
 * Basic and Container Specializations
 */
template <>
std::byte Parse<std::byte>(const simdjson::dom::element& src) {
    return static_cast<std::byte>(src.get_uint64().value());
}

template <>
std::map<std::string, std::string> Parse<std::map<std::string, std::string>>(const simdjson::dom::element& src) {
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        return std::pair{std::string{key}, Parse<std::string>(value)};
    }) | std::ranges::to<std::map<std::string, std::string>>();
}

template <>
std::map<std::string, std::variant<std::string, bool, double>> Parse<std::map<std::string, std::variant<std::string, bool, double>>>(const simdjson::dom::element& src) {
    using V = std::variant<std::string, bool, double>;
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        V variant_val;
        switch (value.type()) {
            case simdjson::dom::element_type::STRING: variant_val = std::string{value.get_string().value()}; break;
            case simdjson::dom::element_type::BOOL:   variant_val = value.get_bool().value(); break;
            case simdjson::dom::element_type::DOUBLE: variant_val = value.get_double().value(); break;
            case simdjson::dom::element_type::INT64:  variant_val = static_cast<double>(value.get_int64().value()); break;
            case simdjson::dom::element_type::UINT64: variant_val = static_cast<double>(value.get_uint64().value()); break;
            default: throw std::runtime_error{"Unexpected type in variant map"};
        }
        return std::pair{std::string{key}, variant_val};
    }) | std::ranges::to<std::map<std::string, V>>();
}


/***
 * Forward Declarations for complex variants to fix ordering issues in vectors
 */

template <>
openai::ResponseFormat Parse<openai::ResponseFormat>(const simdjson::dom::element& src);

template <>
openai::ComputerAction Parse<openai::ComputerAction>(const simdjson::dom::element& src);

template <>
openai::WebSearchAction Parse<openai::WebSearchAction>(const simdjson::dom::element& src);

template <>
openai::ApplyPatchOperation Parse<openai::ApplyPatchOperation>(const simdjson::dom::element& src);

template <>
openai::PromptRef::VariableValue Parse<openai::PromptRef::VariableValue>(const simdjson::dom::element& src);

template <>
std::variant<openai::InputText, openai::InputImage, openai::InputFile>
    Parse<std::variant<openai::InputText, openai::InputImage, openai::InputFile>>(const simdjson::dom::element& src);

template <>
std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>
    Parse<std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>>(const simdjson::dom::element& src);

template <>
openai::OutputMessage::OutputText::Annotation Parse<openai::OutputMessage::OutputText::Annotation>(const simdjson::dom::element& src);

template <>
openai::OutputMessage::Content Parse<openai::OutputMessage::Content>(const simdjson::dom::element& src);

template <>
openai::InputItem Parse<openai::InputItem>(const simdjson::dom::element& src);

template <>
openai::Tool Parse<openai::Tool>(const simdjson::dom::element& src);

template <>
openai::ToolChoice Parse<openai::ToolChoice>(const simdjson::dom::element& src);

template <>
openai::CodeInterpreterCall::Output Parse<openai::CodeInterpreterCall::Output>(const simdjson::dom::element& src);

template <>
openai::ShellCallOutput::Outcome Parse<openai::ShellCallOutput::Outcome>(const simdjson::dom::element& src);

template <>
openai::CustomTool::Format Parse<openai::CustomTool::Format>(const simdjson::dom::element& src);

template <>
openai::CodeInterpreterTool::Container Parse<openai::CodeInterpreterTool::Container>(const simdjson::dom::element& src);

using FunctionCallOutput_output_t = std::variant<std::string, std::vector<std::variant<openai::InputText, openai::InputImage, openai::InputFile>>>;
template <>
FunctionCallOutput_output_t Parse<FunctionCallOutput_output_t>(const simdjson::dom::element& src);


/***
 * Block 8: Response Substructures (simplest)
 */

BEGIN_PARSE(openai::ResponseError)
    FIELD(src, code),
    FIELD(src, message)
END_PARSE

BEGIN_PARSE(openai::IncompleteDetails)
    FIELD(src, reason)
END_PARSE

BEGIN_PARSE(openai::ResponseUsage::OutputTokenDetails)
    FIELD(src, accepted_prediction_tokens),
    FIELD(src, audio_tokens),
    FIELD(src, reasoning_tokens),
    FIELD(src, rejected_prediction_tokens)
END_PARSE

BEGIN_PARSE(openai::ResponseUsage)
    FIELD(src, input_tokens),
    FIELD(src, output_tokens),
    FIELD(src, total_tokens),
    FIELD(src, output_token_details)
END_PARSE


/***
 * Block 1: Shared Substructures
 */

BEGIN_PARSE(openai::ResponseFormatText)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ResponseFormatJsonSchema)
    FIELD(src, name),
    FIELD(src, schema),
    FIELD(src, type),
    FIELD(src, description),
    FIELD(src, strict)
END_PARSE

template <>
openai::ResponseFormat Parse<openai::ResponseFormat>(const simdjson::dom::element& src) {
    using T = openai::ResponseFormat;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "json_schema") {
        return T{Parse<openai::ResponseFormatJsonSchema>(src["json_schema"])};
    } else {
        return T{Parse<openai::ResponseFormatText>(src)};
    }
}

BEGIN_PARSE(openai::TextConfig)
    FIELD(src, format),
    FIELD(src, verbosity)
END_PARSE

BEGIN_PARSE(openai::StreamOptions)
    FIELD(src, include_obfuscation)
END_PARSE

BEGIN_PARSE(openai::ReasoningConfig)
    FIELD(src, effort),
    FIELD(src, summary)
END_PARSE

BEGIN_PARSE(openai::InputText)
    FIELD(src, text),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::InputImage)
    FIELD(src, detail),
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::InputFile)
    FIELD(src, type),
    FIELD(src, file_data),
    FIELD(src, file_id),
    FIELD(src, file_url),
    FIELD(src, filename)
END_PARSE

BEGIN_PARSE(openai::ItemReference)
    FIELD(src, id),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ConversationRef)
    FIELD(src, id)
END_PARSE

template <>
openai::PromptRef::VariableValue Parse<openai::PromptRef::VariableValue>(const simdjson::dom::element& src) {
    using T = openai::PromptRef::VariableValue;
    if (src.is_string()) {
        return T{std::string{src.get_string().value()}};
    }
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "input_image") return T{Parse<openai::InputImage>(src)};
    if (type_sv == "input_file" ) return T{Parse<openai::InputFile>(src)};
    if (type_sv == "item_reference") return T{Parse<openai::ItemReference>(src)};
    throw std::logic_error{"PromptRef::VariableValue variant unsatisfied"};
}

BEGIN_PARSE(openai::PromptRef)
    FIELD(src, id),
    FIELD(src, variables),
    FIELD(src, version)
END_PARSE


/***
 * Block 2: Tool Call Actions and Operations
 */

BEGIN_PARSE(openai::ClickAction)
    FIELD(src, button),
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::DoubleClickAction)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::DragAction::Coordinate)
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::DragAction)
    FIELD(src, path),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::KeyPressAction)
    FIELD(src, keys),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::MoveAction)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::ScreenshotAction)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ScrollAction)
    FIELD(src, scroll_x),
    FIELD(src, scroll_y),
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_PARSE

BEGIN_PARSE(openai::TypeAction)
    FIELD(src, text),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::WaitAction)
    FIELD(src, type)
END_PARSE

template <>
openai::ComputerAction Parse<openai::ComputerAction>(const simdjson::dom::element& src) {
    using T = openai::ComputerAction;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "click") return T{Parse<openai::ClickAction>(src)};
    if (type_sv == "double_click") return T{Parse<openai::DoubleClickAction>(src)};
    if (type_sv == "drag") return T{Parse<openai::DragAction>(src)};
    if (type_sv == "keypress") return T{Parse<openai::KeyPressAction>(src)};
    if (type_sv == "move") return T{Parse<openai::MoveAction>(src)};
    if (type_sv == "screenshot") return T{Parse<openai::ScreenshotAction>(src)};
    if (type_sv == "scroll") return T{Parse<openai::ScrollAction>(src)};
    if (type_sv == "type") return T{Parse<openai::TypeAction>(src)};
    if (type_sv == "wait") return T{Parse<openai::WaitAction>(src)};
    throw std::logic_error{"ComputerAction variant unsatisfied"};
}

BEGIN_PARSE(openai::SearchAction::Source)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::SearchAction)
    FIELD(src, type),
    FIELD(src, queries),
    FIELD(src, sources)
END_PARSE

BEGIN_PARSE(openai::OpenPageAction)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::FindAction)
    FIELD(src, pattern),
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

template <>
openai::WebSearchAction Parse<openai::WebSearchAction>(const simdjson::dom::element& src) {
    using T = openai::WebSearchAction;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "search") return T{Parse<openai::SearchAction>(src)};
    if (type_sv == "open_page") return T{Parse<openai::OpenPageAction>(src)};
    if (type_sv == "find") return T{Parse<openai::FindAction>(src)};
    throw std::logic_error{"WebSearchAction variant unsatisfied"};
}

BEGIN_PARSE(openai::CreateFileOperation)
    FIELD(src, diff),
    FIELD(src, path),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::DeleteFileOperation)
    FIELD(src, path),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::UpdateFileOperation)
    FIELD(src, diff),
    FIELD(src, path),
    FIELD(src, type)
END_PARSE

template <>
openai::ApplyPatchOperation Parse<openai::ApplyPatchOperation>(const simdjson::dom::element& src) {
    using T = openai::ApplyPatchOperation;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "create_file") return T{Parse<openai::CreateFileOperation>(src)};
    if (type_sv == "delete_file") return T{Parse<openai::DeleteFileOperation>(src)};
    if (type_sv == "update_file") return T{Parse<openai::UpdateFileOperation>(src)};
    throw std::logic_error{"ApplyPatchOperation variant unsatisfied"};
}


/***
 * Block 3: Tool Call and Output structs
 */

BEGIN_PARSE(openai::FileSearchCall::Result)
    FIELD(src, attributes),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, score),
    FIELD(src, text)
END_PARSE

BEGIN_PARSE(openai::FileSearchCall)
    FIELD(src, id),
    FIELD(src, queries),
    FIELD(src, status),
    FIELD(src, type),
    FIELD(src, results)
END_PARSE

BEGIN_PARSE(openai::ComputerCall::PendingSafetyCheck)
    FIELD(src, id),
    FIELD(src, code),
    FIELD(src, message)
END_PARSE

BEGIN_PARSE(openai::ComputerCall)
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, pending_safety_checks),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ComputerScreenshot)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::ComputerCallOutput)
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, type),
    FIELD(src, acknowledged_safety_checks),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::WebSearchCall)
    FIELD(src, action),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::FunctionCall)
    FIELD(src, arguments),
    FIELD(src, call_id),
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

template <>
FunctionCallOutput_output_t Parse<FunctionCallOutput_output_t>(const simdjson::dom::element& src) {
    if (src.is_string()) return FunctionCallOutput_output_t{std::string{src.get_string().value()}};
    return FunctionCallOutput_output_t{Parse<std::vector<std::variant<openai::InputText, openai::InputImage, openai::InputFile>>>(src)};
}

BEGIN_PARSE(openai::FunctionCallOutput)
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::ReasoningItem::Summary)
    FIELD(src, text),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ReasoningItem::Content)
    FIELD(src, text),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ReasoningItem)
    FIELD(src, id),
    FIELD(src, summary),
    FIELD(src, type),
    FIELD(src, content),
    FIELD(src, encrypted_content),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::CompactionItem)
    FIELD(src, encrypted_content),
    FIELD(src, type),
    FIELD(src, created_by),
    FIELD(src, id)
END_PARSE

BEGIN_PARSE(openai::ImageGenerationCall)
    FIELD(src, id),
    FIELD(src, result),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::CodeInterpreterCall::OutputLog)
    FIELD(src, logs),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::CodeInterpreterCall::OutputImage)
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

template <>
openai::CodeInterpreterCall::Output Parse<openai::CodeInterpreterCall::Output>(const simdjson::dom::element& src) {
    using T = openai::CodeInterpreterCall::Output;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "logs")  return T{Parse<openai::CodeInterpreterCall::OutputLog>(src)};
    if (type_sv == "image") return T{Parse<openai::CodeInterpreterCall::OutputImage>(src)};
    throw std::logic_error{"CodeInterpreterCall::Output variant unsatisfied"};
}

BEGIN_PARSE(openai::CodeInterpreterCall)
    FIELD(src, code),
    FIELD(src, container_id),
    FIELD(src, id),
    FIELD(src, outputs),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::LocalShellCall::Action)
    FIELD(src, command),
    FIELD(src, env),
    FIELD(src, type),
    FIELD(src, timeout_ms),
    FIELD(src, user),
    FIELD(src, working_directory)
END_PARSE

BEGIN_PARSE(openai::LocalShellCall)
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::LocalShellCallOutput::ActionOutcome)
    FIELD(src, exit_code),
    FIELD(src, type),
    FIELD(src, std_err),
    FIELD(src, std_out)
END_PARSE

BEGIN_PARSE(openai::LocalShellCallOutput)
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, max_output_length),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::ShellCall::Action)
    FIELD(src, commands),
    FIELD(src, max_output_length),
    FIELD(src, timeout_ms)
END_PARSE

BEGIN_PARSE(openai::ShellCall)
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, type),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::ShellCallOutput::ShellExitOutcome)
    FIELD(src, exit_code),
    FIELD(src, type),
    FIELD(src, std_err),
    FIELD(src, std_out)
END_PARSE

BEGIN_PARSE(openai::ShellCallOutput::ShellTimeoutOutcome)
    FIELD(src, type)
END_PARSE

template <>
openai::ShellCallOutput::Outcome Parse<openai::ShellCallOutput::Outcome>(const simdjson::dom::element& src) {
    using T = openai::ShellCallOutput::Outcome;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "exit") return T{Parse<openai::ShellCallOutput::ShellExitOutcome>(src)};
    if (type_sv == "timeout") return T{Parse<openai::ShellCallOutput::ShellTimeoutOutcome>(src)};
    throw std::logic_error{"ShellCallOutput::Outcome variant unsatisfied"};
}

BEGIN_PARSE(openai::ShellCallOutput::Content)
    FIELD(src, outcome)
END_PARSE

BEGIN_PARSE(openai::ShellCallOutput)
    FIELD(src, call_id),
    FIELD(src, max_output_length),
    FIELD(src, output),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, created_by)
END_PARSE

BEGIN_PARSE(openai::ApplyPatchCall)
    FIELD(src, operation),
    FIELD(src, status),
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, created_by),
    FIELD(src, id)
END_PARSE

BEGIN_PARSE(openai::ApplyPatchCallOutput)
    FIELD(src, status),
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, created_by),
    FIELD(src, id),
    FIELD(src, output)
END_PARSE

BEGIN_PARSE(openai::McpListTools::ToolDef)
    FIELD(src, input_schema),
    FIELD(src, name),
    FIELD(src, annotations),
    FIELD(src, description)
END_PARSE

BEGIN_PARSE(openai::McpListTools)
    FIELD(src, id),
    FIELD(src, server_label),
    FIELD(src, tools),
    FIELD(src, type),
    FIELD(src, error)
END_PARSE

BEGIN_PARSE(openai::McpApprovalRequest)
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::McpApprovalResponse)
    FIELD(src, approval_request_id),
    FIELD(src, approve),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, reason)
END_PARSE

BEGIN_PARSE(openai::McpCall)
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label),
    FIELD(src, type),
    FIELD(src, approval_request_id),
    FIELD(src, error),
    FIELD(src, output),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::CustomToolCall)
    FIELD(src, input),
    FIELD(src, name),
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::CustomToolCallOutput)
    FIELD(src, output),
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE


/***
 * Block 4: Message and InputItem variants
 */

template <>
std::variant<openai::InputText, openai::InputImage, openai::InputFile>
    Parse<std::variant<openai::InputText, openai::InputImage, openai::InputFile>>(const simdjson::dom::element& src)
{
    using T = std::variant<openai::InputText, openai::InputImage, openai::InputFile>;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "input_text")  return T{Parse<openai::InputText>(src)};
    if (type_sv == "input_image") return T{Parse<openai::InputImage>(src)};
    if (type_sv == "input_file")  return T{Parse<openai::InputFile>(src)};
    throw std::logic_error{"Variant unsatisfied"};
}

template <>
std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>
    Parse<std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>>(const simdjson::dom::element& src)
{
    using T = std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "input_text")     return T{Parse<openai::InputText>(src)};
    if (type_sv == "input_image")    return T{Parse<openai::InputImage>(src)};
    if (type_sv == "input_file")     return T{Parse<openai::InputFile>(src)};
    if (type_sv == "item_reference") return T{Parse<openai::ItemReference>(src)};
    throw std::logic_error{"InputMessage content variant unsatisfied"};
}

BEGIN_PARSE(openai::InputMessage)
    FIELD(src, content),
    FIELD(src, role),
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, status)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText::CitationContainer)
    FIELD(src, container_id),
    FIELD(src, end_index),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, start_index),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText::CitationFile)
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, index),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText::CitationUrl)
    FIELD(src, end_index),
    FIELD(src, start_index),
    FIELD(src, title),
    FIELD(src, type),
    FIELD(src, url)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText::FilePath)
    FIELD(src, file_id),
    FIELD(src, index),
    FIELD(src, type)
END_PARSE

template <>
openai::OutputMessage::OutputText::Annotation Parse<openai::OutputMessage::OutputText::Annotation>(const simdjson::dom::element& src) {
    using T = openai::OutputMessage::OutputText::Annotation;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "file_citation")           return T{Parse<openai::OutputMessage::OutputText::CitationFile>(src)};
    if (type_sv == "url_citation")            return T{Parse<openai::OutputMessage::OutputText::CitationUrl>(src)};
    if (type_sv == "container_file_citation") return T{Parse<openai::OutputMessage::OutputText::CitationContainer>(src)};
    if (type_sv == "file_path")               return T{Parse<openai::OutputMessage::OutputText::FilePath>(src)};
    throw std::logic_error{"OutputMessage::OutputText::Annotation variant unsatisfied"};
}

BEGIN_PARSE(openai::OutputMessage::OutputText::Logprob::TopLogprob)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText::Logprob)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token),
    FIELD(src, top_logprobs)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::OutputText)
    FIELD(src, annotations),
    FIELD(src, value),
    FIELD(src, type),
    FIELD(src, logprobs)
END_PARSE

BEGIN_PARSE(openai::OutputMessage::Refusal)
    FIELD(src, refusal),
    FIELD(src, type)
END_PARSE

template <>
openai::OutputMessage::Content Parse<openai::OutputMessage::Content>(const simdjson::dom::element& src) {
    using T = openai::OutputMessage::Content;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "output_text") return T{Parse<openai::OutputMessage::OutputText>(src)};
    if (type_sv == "refusal")     return T{Parse<openai::OutputMessage::Refusal>(src)};
    throw std::logic_error{"OutputMessage::Content variant unsatisfied"};
}

BEGIN_PARSE(openai::OutputMessage)
    FIELD(src, content),
    FIELD(src, id),
    FIELD(src, role),
    FIELD(src, status),
    FIELD(src, type)
END_PARSE

template <>
openai::InputItem Parse<openai::InputItem>(const simdjson::dom::element& src) {
    using T = openai::InputItem;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "message") {
        auto role_sv = src["role"].get_string().value();
        if (role_sv == "assistant") return T{Parse<openai::OutputMessage>(src)};
        return T{Parse<openai::InputMessage>(src)};
    }
    if (type_sv == "file_search_call")        return T{Parse<openai::FileSearchCall>(src)};
    if (type_sv == "computer_call")           return T{Parse<openai::ComputerCall>(src)};
    if (type_sv == "computer_call_output")    return T{Parse<openai::ComputerCallOutput>(src)};
    if (type_sv == "web_search_call")         return T{Parse<openai::WebSearchCall>(src)};
    if (type_sv == "function_call")           return T{Parse<openai::FunctionCall>(src)};
    if (type_sv == "function_call_output")    return T{Parse<openai::FunctionCallOutput>(src)};
    if (type_sv == "reasoning")               return T{Parse<openai::ReasoningItem>(src)};
    if (type_sv == "compaction")              return T{Parse<openai::CompactionItem>(src)};
    if (type_sv == "image_generation_call")   return T{Parse<openai::ImageGenerationCall>(src)};
    if (type_sv == "code_interpreter_call")   return T{Parse<openai::CodeInterpreterCall>(src)};
    if (type_sv == "local_shell_call")        return T{Parse<openai::LocalShellCall>(src)};
    if (type_sv == "local_shell_call_output") return T{Parse<openai::LocalShellCallOutput>(src)};
    if (type_sv == "shell_call")              return T{Parse<openai::ShellCall>(src)};
    if (type_sv == "shell_call_output")       return T{Parse<openai::ShellCallOutput>(src)};
    if (type_sv == "apply_patch_call")        return T{Parse<openai::ApplyPatchCall>(src)};
    if (type_sv == "apply_patch_call_output") return T{Parse<openai::ApplyPatchCallOutput>(src)};
    if (type_sv == "mcp_list_tools")          return T{Parse<openai::McpListTools>(src)};
    if (type_sv == "mcp_approval_request")    return T{Parse<openai::McpApprovalRequest>(src)};
    if (type_sv == "mcp_approval_response")   return T{Parse<openai::McpApprovalResponse>(src)};
    if (type_sv == "mcp_call")                return T{Parse<openai::McpCall>(src)};
    if (type_sv == "custom_tool_call")        return T{Parse<openai::CustomToolCall>(src)};
    if (type_sv == "custom_tool_call_output") return T{Parse<openai::CustomToolCallOutput>(src)};
    if (type_sv == "item_reference")          return T{Parse<openai::ItemReference>(src)};
    throw std::logic_error{"InputItem variant unsatisfied"};
}


/***
 * Block 5: Model Tools
 */

BEGIN_PARSE(openai::FunctionTool::Parameters)
    FIELD(src, properties),
    FIELD(src, type),
    FIELD(src, required)
END_PARSE

BEGIN_PARSE(openai::FunctionTool)
    FIELD(src, name),
    FIELD(src, parameters),
    FIELD(src, type),
    FIELD(src, description),
    FIELD(src, strict)
END_PARSE

BEGIN_PARSE(openai::FileSearchTool::RankingOptions)
    FIELD(src, score_threshold),
    FIELD(src, ranker)
END_PARSE

BEGIN_PARSE(openai::FileSearchTool)
    FIELD(src, max_num_results),
    FIELD(src, ranking_options),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ComputerUseTool)
    FIELD(src, display_height),
    FIELD(src, display_width),
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::WebSearchTool::Filters)
    FIELD(src, domain_filter),
    FIELD(src, filter_type)
END_PARSE

BEGIN_PARSE(openai::WebSearchTool::Location)
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, latitude),
    FIELD(src, longitude),
    FIELD(src, region)
END_PARSE

BEGIN_PARSE(openai::WebSearchTool)
    FIELD(src, filters),
    FIELD(src, location),
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::McpTool::Filter)
    FIELD(src, server_label),
    FIELD(src, tool_name)
END_PARSE

BEGIN_PARSE(openai::McpTool::AllowedTools)
    FIELD(src, filter),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::McpTool::ApprovalFilter)
    FIELD(src, approval_filter),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::McpTool)
    FIELD(src, allowed_tools),
    FIELD(src, approval_filters),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::CodeInterpreterTool::ContainerConfig)
    FIELD(src, file_ids),
    FIELD(src, type),
    FIELD(src, memory_limit)
END_PARSE

template <>
openai::CodeInterpreterTool::Container Parse<openai::CodeInterpreterTool::Container>(const simdjson::dom::element& src) {
    using T = openai::CodeInterpreterTool::Container;
    if (src.is_string()) return T{std::string{src.get_string().value()}};
    return T{Parse<openai::CodeInterpreterTool::ContainerConfig>(src)};
}

BEGIN_PARSE(openai::CodeInterpreterTool)
    FIELD(src, container),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ImageGenerationTool::Mask)
    FIELD(src, file_id),
    FIELD(src, image_url)
END_PARSE

BEGIN_PARSE(openai::ImageGenerationTool)
    FIELD(src, type),
    FIELD(src, background),
    FIELD(src, mask)
END_PARSE

BEGIN_PARSE(openai::LocalShellTool)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::ShellTool)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::CustomTool::GrammarFormat)
    FIELD(src, syntax),
    FIELD(src, type),
    FIELD(src, value)
END_PARSE

BEGIN_PARSE(openai::CustomTool::TextFormat)
    FIELD(src, type)
END_PARSE

template <>
openai::CustomTool::Format Parse<openai::CustomTool::Format>(const simdjson::dom::element& src) {
    using T = openai::CustomTool::Format;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "grammar") return T{Parse<openai::CustomTool::GrammarFormat>(src)};
    if (type_sv == "text")    return T{Parse<openai::CustomTool::TextFormat>(src)};
    throw std::logic_error{"CustomTool::Format variant unsatisfied"};
}

BEGIN_PARSE(openai::CustomTool)
    FIELD(src, description),
    FIELD(src, format),
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::WebSearchPreviewTool::Location)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_PARSE

BEGIN_PARSE(openai::WebSearchPreviewTool)
    FIELD(src, type),
    FIELD(src, search_context_size),
    FIELD(src, user_location)
END_PARSE

BEGIN_PARSE(openai::ApplyPatchTool)
    FIELD(src, type)
END_PARSE

template <>
openai::Tool Parse<openai::Tool>(const simdjson::dom::element& src) {
    using T = openai::Tool;
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "function")              return T{Parse<openai::FunctionTool>(src)};
    if (type_sv == "file_search")           return T{Parse<openai::FileSearchTool>(src)};
    if (type_sv == "computer_20241022")    return T{Parse<openai::ComputerUseTool>(src)};
    if (type_sv == "web_search_20250124")  return T{Parse<openai::WebSearchTool>(src)};
    if (type_sv == "mcp")                   return T{Parse<openai::McpTool>(src)};
    if (type_sv == "code_interpreter")      return T{Parse<openai::CodeInterpreterTool>(src)};
    if (type_sv == "image_generation")      return T{Parse<openai::ImageGenerationTool>(src)};
    if (type_sv == "local_shell")           return T{Parse<openai::LocalShellTool>(src)};
    if (type_sv == "shell")                  return T{Parse<openai::ShellTool>(src)};
    if (type_sv == "custom")                 return T{Parse<openai::CustomTool>(src)};
    if (type_sv == "web_search_preview")    return T{Parse<openai::WebSearchPreviewTool>(src)};
    if (type_sv == "apply_patch")           return T{Parse<openai::ApplyPatchTool>(src)};
    throw std::logic_error{"Tool variant unsatisfied"};
}


/***
 * Block 6: Tool Choice models
 */

BEGIN_PARSE(openai::AllowedToolsChoice::RestrictedTool)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, server_label)
END_PARSE

BEGIN_PARSE(openai::AllowedToolsChoice)
    FIELD(src, mode),
    FIELD(src, tools),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::HostedToolChoice)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::FunctionToolChoice)
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::McpToolChoice)
    FIELD(src, name),
    FIELD(src, server_label),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::CustomToolChoice)
    FIELD(src, name),
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::SpecificApplyPatchToolChoice)
    FIELD(src, type)
END_PARSE

BEGIN_PARSE(openai::SpecificShellToolChoice)
    FIELD(src, type)
END_PARSE

template <>
openai::ToolChoice Parse<openai::ToolChoice>(const simdjson::dom::element& src) {
    using T = openai::ToolChoice;
    if (src.is_string()) return T{Parse<openai::ToolChoiceMode>(src)};
    auto type_sv = src["type"].get_string().value();
    if (type_sv == "allowed_tools") return T{Parse<openai::AllowedToolsChoice>(src)};
    if (type_sv == "hosted")        return T{Parse<openai::HostedToolChoice>(src)};
    if (type_sv == "function")      return T{Parse<openai::FunctionToolChoice>(src)};
    if (type_sv == "mcp")           return T{Parse<openai::McpToolChoice>(src)};
    if (type_sv == "custom")        return T{Parse<openai::CustomToolChoice>(src)};
    if (type_sv == "apply_patch")   return T{Parse<openai::SpecificApplyPatchToolChoice>(src)};
    if (type_sv == "shell")         return T{Parse<openai::SpecificShellToolChoice>(src)};
    throw std::logic_error{"ToolChoice variant unsatisfied"};
}


/***
 * Block 7: Request
 */

BEGIN_PARSE(openai::Request)
    FIELD(src, background),
    FIELD(src, conversation),
    FIELD(src, include),
    FIELD(src, input),
    FIELD(src, instructions),
    FIELD(src, max_output_tokens),
    FIELD(src, metadata),
    FIELD(src, model),
    FIELD(src, parallel_tool_calls),
    FIELD(src, previous_response_id),
    FIELD(src, prompt),
    FIELD(src, reasoning),
    FIELD(src, service_tier),
    FIELD(src, store),
    FIELD(src, temperature),
    FIELD(src, text),
    FIELD(src, tool_choice),
    FIELD(src, tools),
    FIELD(src, top_p)
END_PARSE


/***
 * Block 9: Response
 */

BEGIN_PARSE(openai::Stop)
    FIELD(src, reason),
    FIELD(src, sequence)
END_PARSE

BEGIN_PARSE(openai::Response)
    FIELD(src, id),
    FIELD(src, object),
    FIELD(src, background),
    FIELD(src, completed_at),
    FIELD(src, conversation),
    FIELD(src, created_at),
    FIELD(src, error),
    FIELD(src, incomplete_details),
    FIELD(src, instructions),
    FIELD(src, max_tokens),
    FIELD(src, metadata),
    FIELD(src, model),
    FIELD(src, previous_response_id),
    FIELD(src, reasoning),
    FIELD(src, service_tier),
    FIELD(src, status),
    FIELD(src, temperature),
    FIELD(src, text),
    FIELD(src, tool_choice),
    FIELD(src, tools),
    FIELD(src, top_p),
    FIELD(src, usage)
END_PARSE


/***
 * Top-level Deserialize
 */

namespace openai {

Response Deserialize(const curl::Response& response) {
    simdjson::dom::parser parser;
    simdjson::dom::element doc = parser.parse(response.body);
    return Parse<Response>(doc);
}

}


}
