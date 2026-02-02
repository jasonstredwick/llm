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
openai::ResponseInputItem Parse<openai::ResponseInputItem>(const simdjson::dom::element& src);

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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"json_schema") {
        return T{Parse<openai::ResponseFormatJsonSchema>(src["json_schema"])};
    } else {
        return T{Parse<openai::ResponseFormatText>(*r)}; }
    }
}

BEGIN_PARSE(openai::TextConfig)
    FIELD(src, format),
    FIELD(src, verbosity)
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"click">(obj); r.has_value()) { return T{Parse<openai::ClickAction>(*r)}; }
    else if (auto r = ExtractForVariant<"double_click">(obj); r.has_value()) { return T{Parse<openai::DoubleClickAction>(*r)}; }
    else if (auto r = ExtractForVariant<"drag">(obj); r.has_value()) { return T{Parse<openai::DragAction>(*r)}; }
    else if (auto r = ExtractForVariant<"keypress">(obj); r.has_value()) { return T{Parse<openai::KeyPressAction>(*r)}; }
    else if (auto r = ExtractForVariant<"move">(obj); r.has_value()) { return T{Parse<openai::MoveAction>(*r)}; }
    else if (auto r = ExtractForVariant<"screenshot">(obj); r.has_value()) { return T{Parse<openai::ScreenshotAction>(*r)}; }
    else if (auto r = ExtractForVariant<"scroll">(obj); r.has_value()) { return T{Parse<openai::ScrollAction>(*r)}; }
    else if (auto r = ExtractForVariant<"type">(obj); r.has_value()) { return T{Parse<openai::TypeAction>(*r)}; }
    else if (auto r = ExtractForVariant<"wait">(obj); r.has_value()) { return T{Parse<openai::WaitAction>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"search">(obj); r.has_value()) { return T{Parse<openai::SearchAction>(*r)}; }
    else if (auto r = ExtractForVariant<"open_page">(obj); r.has_value()) { return T{Parse<openai::OpenPageAction>(*r)}; }
    else if (auto r = ExtractForVariant<"find">(obj); r.has_value()) { return T{Parse<openai::FindAction>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"create_file">(obj); r.has_value()) { return T{Parse<openai::CreateFileOperation>(*r)}; }
    else if (auto r = ExtractForVariant<"delete_file">(obj); r.has_value()) { return T{Parse<openai::DeleteFileOperation>(*r)}; }
    else if (auto r = ExtractForVariant<"update_file">(obj); r.has_value()) { return T{Parse<openai::UpdateFileOperation>(*r)}; }
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
    return FunctionCallOutput_output_t{Parse<std::vector<std::variant<openai::InputText, openai::InputImage, openai::InputFile>>>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"logs">(obj); r.has_value()) { return T{Parse<openai::CodeInterpreterCall::OutputLog>(*r)}; }
    else if (auto r = ExtractForVariant<"image">(obj); r.has_value()) { return T{Parse<openai::CodeInterpreterCall::OutputImage>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"exit">(obj); r.has_value()) { return T{Parse<openai::ShellCallOutput::ShellExitOutcome>(*r)}; }
    else if (auto r = ExtractForVariant<"timeout">(obj); r.has_value()) { return T{Parse<openai::ShellCallOutput::ShellTimeoutOutcome>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"input_text">(obj); r.has_value()) { return T{Parse<openai::InputText>(*r)}; }
    else if (auto r = ExtractForVariant<"input_image">(obj); r.has_value()) { return T{Parse<openai::InputImage>(*r)}; }
    else if (auto r = ExtractForVariant<"input_file">(obj); r.has_value()) { return T{Parse<openai::InputFile>(*r)}; }
    throw std::logic_error{"Variant unsatisfied"};
}

template <>
std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>
    Parse<std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>>(const simdjson::dom::element& src)
{
    using T = std::variant<openai::InputText, openai::InputImage, openai::InputFile, openai::ItemReference>;
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"input_text">(obj); r.has_value()) { return T{Parse<openai::InputText>(*r)}; }
    else if (auto r = ExtractForVariant<"input_image">(obj); r.has_value()) { return T{Parse<openai::InputImage>(*r)}; }
    else if (auto r = ExtractForVariant<"input_file">(obj); r.has_value()) { return T{Parse<openai::InputFile>(*r)}; }
    else if (auto r = ExtractForVariant<"item_reference">(obj); r.has_value()) { return T{Parse<openai::ItemReference>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"file_citation">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::OutputText::CitationFile>(*r)}; }
    else if (auto r = ExtractForVariant<"url_citation">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::OutputText::CitationUrl>(*r)}; }
    else if (auto r = ExtractForVariant<"container_file_citation">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::OutputText::CitationContainer>(*r)}; }
    else if (auto r = ExtractForVariant<"file_path">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::OutputText::FilePath>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"output_text">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::OutputText>(*r)}; }
    else if (auto r = ExtractForVariant<"refusal">(obj); r.has_value()) { return T{Parse<openai::OutputMessage::Refusal>(*r)}; }
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
openai::ResponseInputItem Parse<openai::ResponseInputItem>(const simdjson::dom::element& src) {
    using T = openai::ResponseInputItem;
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"message") {
        auto role_sv = src["role"].get_string().value();
        if (role_sv == "assistant">(obj); r.has_value()) { return T{Parse<openai::OutputMessage>(*r)}; }
        return T{Parse<openai::InputMessage>(*r)}; }
    }
    else if (auto r = ExtractForVariant<"file_search_call">(obj); r.has_value()) { return T{Parse<openai::FileSearchCall>(*r)}; }
    else if (auto r = ExtractForVariant<"computer_call">(obj); r.has_value()) { return T{Parse<openai::ComputerCall>(*r)}; }
    else if (auto r = ExtractForVariant<"computer_call_output">(obj); r.has_value()) { return T{Parse<openai::ComputerCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"web_search_call">(obj); r.has_value()) { return T{Parse<openai::WebSearchCall>(*r)}; }
    else if (auto r = ExtractForVariant<"function_call">(obj); r.has_value()) { return T{Parse<openai::FunctionCall>(*r)}; }
    else if (auto r = ExtractForVariant<"function_call_output">(obj); r.has_value()) { return T{Parse<openai::FunctionCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"reasoning">(obj); r.has_value()) { return T{Parse<openai::ReasoningItem>(*r)}; }
    else if (auto r = ExtractForVariant<"compaction">(obj); r.has_value()) { return T{Parse<openai::CompactionItem>(*r)}; }
    else if (auto r = ExtractForVariant<"image_generation_call">(obj); r.has_value()) { return T{Parse<openai::ImageGenerationCall>(*r)}; }
    else if (auto r = ExtractForVariant<"code_interpreter_call">(obj); r.has_value()) { return T{Parse<openai::CodeInterpreterCall>(*r)}; }
    else if (auto r = ExtractForVariant<"local_shell_call">(obj); r.has_value()) { return T{Parse<openai::LocalShellCall>(*r)}; }
    else if (auto r = ExtractForVariant<"local_shell_call_output">(obj); r.has_value()) { return T{Parse<openai::LocalShellCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"shell_call">(obj); r.has_value()) { return T{Parse<openai::ShellCall>(*r)}; }
    else if (auto r = ExtractForVariant<"shell_call_output">(obj); r.has_value()) { return T{Parse<openai::ShellCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"apply_patch_call">(obj); r.has_value()) { return T{Parse<openai::ApplyPatchCall>(*r)}; }
    else if (auto r = ExtractForVariant<"apply_patch_call_output">(obj); r.has_value()) { return T{Parse<openai::ApplyPatchCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp_list_tools">(obj); r.has_value()) { return T{Parse<openai::McpListTools>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp_approval_request">(obj); r.has_value()) { return T{Parse<openai::McpApprovalRequest>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp_approval_response">(obj); r.has_value()) { return T{Parse<openai::McpApprovalResponse>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp_call">(obj); r.has_value()) { return T{Parse<openai::McpCall>(*r)}; }
    else if (auto r = ExtractForVariant<"custom_tool_call">(obj); r.has_value()) { return T{Parse<openai::CustomToolCall>(*r)}; }
    else if (auto r = ExtractForVariant<"custom_tool_call_output">(obj); r.has_value()) { return T{Parse<openai::CustomToolCallOutput>(*r)}; }
    else if (auto r = ExtractForVariant<"item_reference">(obj); r.has_value()) { return T{Parse<openai::ItemReference>(*r)}; }
    throw std::logic_error{"ResponseInputItem variant unsatisfied"};
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
    return T{Parse<openai::CodeInterpreterTool::ContainerConfig>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"grammar">(obj); r.has_value()) { return T{Parse<openai::CustomTool::GrammarFormat>(*r)}; }
    else if (auto r = ExtractForVariant<"text">(obj); r.has_value()) { return T{Parse<openai::CustomTool::TextFormat>(*r)}; }
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
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"function">(obj); r.has_value()) { return T{Parse<openai::FunctionTool>(*r)}; }
    else if (auto r = ExtractForVariant<"file_search">(obj); r.has_value()) { return T{Parse<openai::FileSearchTool>(*r)}; }
    else if (auto r = ExtractForVariant<"computer_20241022">(obj); r.has_value()) { return T{Parse<openai::ComputerUseTool>(*r)}; }
    else if (auto r = ExtractForVariant<"web_search_20250124">(obj); r.has_value()) { return T{Parse<openai::WebSearchTool>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp">(obj); r.has_value()) { return T{Parse<openai::McpTool>(*r)}; }
    else if (auto r = ExtractForVariant<"code_interpreter">(obj); r.has_value()) { return T{Parse<openai::CodeInterpreterTool>(*r)}; }
    else if (auto r = ExtractForVariant<"image_generation">(obj); r.has_value()) { return T{Parse<openai::ImageGenerationTool>(*r)}; }
    else if (auto r = ExtractForVariant<"local_shell">(obj); r.has_value()) { return T{Parse<openai::LocalShellTool>(*r)}; }
    else if (auto r = ExtractForVariant<"shell">(obj); r.has_value()) { return T{Parse<openai::ShellTool>(*r)}; }
    else if (auto r = ExtractForVariant<"custom">(obj); r.has_value()) { return T{Parse<openai::CustomTool>(*r)}; }
    else if (auto r = ExtractForVariant<"web_search_preview">(obj); r.has_value()) { return T{Parse<openai::WebSearchPreviewTool>(*r)}; }
    else if (auto r = ExtractForVariant<"apply_patch">(obj); r.has_value()) { return T{Parse<openai::ApplyPatchTool>(*r)}; }
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
    if (src.is_string()) return T{Parse<openai::ToolChoiceMode>(*r)}; }
    auto obj = src.get_object();
    else if (auto r = ExtractForVariant<"allowed_tools">(obj); r.has_value()) { return T{Parse<openai::AllowedToolsChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"hosted">(obj); r.has_value()) { return T{Parse<openai::HostedToolChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"function">(obj); r.has_value()) { return T{Parse<openai::FunctionToolChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"mcp">(obj); r.has_value()) { return T{Parse<openai::McpToolChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"custom">(obj); r.has_value()) { return T{Parse<openai::CustomToolChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"apply_patch">(obj); r.has_value()) { return T{Parse<openai::SpecificApplyPatchToolChoice>(*r)}; }
    else if (auto r = ExtractForVariant<"shell">(obj); r.has_value()) { return T{Parse<openai::SpecificShellToolChoice>(*r)}; }
    throw std::logic_error{"ToolChoice variant unsatisfied"};
}


/***
 * Block 7: Request
 */



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
