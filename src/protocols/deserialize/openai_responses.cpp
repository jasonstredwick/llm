#include "../../../interface/protocols/openai/responses.hpp"
#include "../../../interface/protocols/openai/strings.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


/***
 * ComputerToolActions
 */
BEGIN_DESERIALIZE(openai::ComputerToolActions::Click)
    FIELD(src, type),
    FIELD(src, button),
    FIELD(src, x),
    FIELD(src, y)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::DoubleClick)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Drag::Coordinate)
    FIELD(src, x),
    FIELD(src, y)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Drag)
    FIELD(src, type),
    FIELD(src, path)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::KeyPress)
    FIELD(src, type),
    FIELD(src, keys)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Move)
    FIELD(src, type),
    FIELD(src, x),
    FIELD(src, y)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Screenshot)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Scroll)
    FIELD(src, type),
    FIELD(src, scroll_x),
    FIELD(src, scroll_y),
    FIELD(src, x),
    FIELD(src, y)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Type)
    FIELD(src, type),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ComputerToolActions::Wait)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::ComputerToolActions::All)
    auto kind = EXTRACT_KIND(openai::ComputerActionType, src, "type");
    FIELD_KIND(src, kind, openai::ComputerActionType::CLICK,        openai::ComputerToolActions::Click)
    FIELD_KIND(src, kind, openai::ComputerActionType::DOUBLE_CLICK, openai::ComputerToolActions::DoubleClick)
    FIELD_KIND(src, kind, openai::ComputerActionType::DRAG,         openai::ComputerToolActions::Drag)
    FIELD_KIND(src, kind, openai::ComputerActionType::KEYPRESS,     openai::ComputerToolActions::KeyPress)
    FIELD_KIND(src, kind, openai::ComputerActionType::MOVE,         openai::ComputerToolActions::Move)
    FIELD_KIND(src, kind, openai::ComputerActionType::SCREENSHOT,   openai::ComputerToolActions::Screenshot)
    FIELD_KIND(src, kind, openai::ComputerActionType::SCROLL,       openai::ComputerToolActions::Scroll)
    FIELD_KIND(src, kind, openai::ComputerActionType::TYPE,         openai::ComputerToolActions::Type)
    FIELD_KIND(src, kind, openai::ComputerActionType::WAIT,         openai::ComputerToolActions::Wait)
END_DESERIALIZE_VARIANT(openai::ComputerToolActions::All)


/***
 * PatchFileOperations
 */
BEGIN_DESERIALIZE(openai::PatchFileOperations::Create)
    FIELD(src, type),
    FIELD(src, diff),
    FIELD(src, path)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::PatchFileOperations::Delete)
    FIELD(src, type),
    FIELD(src, path)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::PatchFileOperations::Update)
    FIELD(src, type),
    FIELD(src, diff),
    FIELD(src, path)
END_DESERIALIZE


/***
 * Other Common Structures
 */
BEGIN_DESERIALIZE(openai::ConversationRef)
    FIELD(src, id)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::IncompleteDetails)
    FIELD(src, reason)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::Reasoning)
    FIELD(src, effort),
    FIELD(src, summary)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ResponseError)
    FIELD(src, code),
    FIELD(src, message)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ResponseUsage::InputTokenDetails)
    FIELD(src, cached_tokens)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ResponseUsage::OutputTokenDetails)
    FIELD(src, reasoning_tokens)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::ResponseUsage)
    FIELD(src, input_tokens),
    FIELD(src, input_tokens_details),
    FIELD(src, output_tokens),
    FIELD(src, output_tokens_details),
    FIELD(src, total_tokens)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::StreamOptions)
    FIELD(src, include_obfuscation)
END_DESERIALIZE

/***
 * TextConfig
 */
BEGIN_DESERIALIZE(openai::TextConfig::FormatText)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::TextConfig::FormatJsonSchema)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, schema),
    FIELD(src, description),
    FIELD(src, strict)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::TextConfig::Format)
    auto kind = EXTRACT_KIND(openai::ResponseFormatType, src, "type");
    FIELD_KIND(src, kind, openai::ResponseFormatType::TEXT,        openai::TextConfig::FormatText)
    FIELD_KIND(src, kind, openai::ResponseFormatType::JSON_SCHEMA, openai::TextConfig::FormatJsonSchema)
END_DESERIALIZE_VARIANT(openai::TextConfig::Format)

BEGIN_DESERIALIZE(openai::TextConfig)
    FIELD(src, format),
    FIELD(src, verbosity)
END_DESERIALIZE


/***
 * ContentTypes
 */
BEGIN_DESERIALIZE(openai::response::ContentTypes::File)
    FIELD(src, type),
    FIELD(src, file_data),
    FIELD(src, file_id),
    FIELD(src, file_url),
    FIELD(src, filename)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::Image)
    FIELD(src, type),
    FIELD(src, detail),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::ContainerFileCitation)
    FIELD(src, type),
    FIELD(src, container_id),
    FIELD(src, end_index),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, start_index)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::FileCitation)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, index)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::UrlCitation)
    FIELD(src, type),
    FIELD(src, end_index),
    FIELD(src, start_index),
    FIELD(src, title),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::FilePath)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, index)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::LogProb::TopLogprob)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText::LogProb)
    FIELD(src, bytes),
    FIELD(src, logprob),
    FIELD(src, token),
    FIELD(src, top_logprobs)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ContentTypes::OutputText::Annotation)
    using BaseT = openai::response::ContentTypes::OutputText;
    auto kind = EXTRACT_KIND(openai::AnnotationType, src, "type");
    FIELD_KIND(src, kind, openai::AnnotationType::FILE_CITATION,           BaseT::FileCitation)
    FIELD_KIND(src, kind, openai::AnnotationType::URL_CITATION,            BaseT::UrlCitation)
    FIELD_KIND(src, kind, openai::AnnotationType::CONTAINER_FILE_CITATION, BaseT::ContainerFileCitation)
    FIELD_KIND(src, kind, openai::AnnotationType::FILE_PATH,               BaseT::FilePath)
END_DESERIALIZE_VARIANT(openai::response::ContentTypes::OutputText::Annotation)

BEGIN_DESERIALIZE(openai::response::ContentTypes::OutputText)
    FIELD(src, type),
    FIELD(src, annotations),
    FIELD(src, logprobs),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::Refusal)
    FIELD(src, type),
    FIELD(src, refusal)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ContentTypes::Text)
    FIELD(src, type),
    FIELD(src, text)
END_DESERIALIZE


/***
 * WebSearchToolActions
 */
BEGIN_DESERIALIZE(openai::response::WebSearchToolActions::Find)
    FIELD(src, type),
    FIELD(src, pattern),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::WebSearchToolActions::OpenPage)
    FIELD(src, type),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::WebSearchToolActions::Search::Source)
    FIELD(src, type),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::WebSearchToolActions::Search)
    FIELD(src, type),
    FIELD(src, queries),
    FIELD(src, sources)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::WebSearchToolActions::All)
    auto kind = EXTRACT_KIND(openai::WebSearchActionType, src, "type");
    FIELD_KIND(src, kind, openai::WebSearchActionType::FIND,      openai::response::WebSearchToolActions::Find)
    FIELD_KIND(src, kind, openai::WebSearchActionType::OPEN_PAGE, openai::response::WebSearchToolActions::OpenPage)
    FIELD_KIND(src, kind, openai::WebSearchActionType::SEARCH,    openai::response::WebSearchToolActions::Search)
END_DESERIALIZE_VARIANT(openai::response::WebSearchToolActions::All)


/***
 * InputTypes
 */
BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Message::Content)
    if (src.is_string()) { return T{DeserializeTo<std::string>(src)}; }
    else if (src.is_array()) {
        return T{DeserializeTo<std::vector<openai::response::InputTypes::MessageContentUnit>>(src)};
    }
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Message::Content)

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::MessageContentUnit)
    auto kind = EXTRACT_KIND(openai::ContentType, src, "type");
    FIELD_KIND(src, kind, openai::ContentType::INPUT_TEXT,  openai::response::ContentTypes::Text)
    FIELD_KIND(src, kind, openai::ContentType::INPUT_IMAGE, openai::response::ContentTypes::Image)
    FIELD_KIND(src, kind, openai::ContentType::INPUT_FILE,  openai::response::ContentTypes::File)
END_DESERIALIZE_VARIANT(openai::response::InputTypes::MessageContentUnit)

BEGIN_DESERIALIZE(openai::response::InputTypes::Message)
    FIELD(src, content),
    FIELD(src, role),
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::InputMessage)
    FIELD(src, content),
    FIELD(src, role),
    FIELD(src, status),
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::OutputMessage::Content)
    auto kind = EXTRACT_KIND(openai::OutputMessageContentType, src, "type");
    FIELD_KIND(src, kind, openai::OutputMessageContentType::OUTPUT_TEXT, openai::response::ContentTypes::OutputText)
    FIELD_KIND(src, kind, openai::OutputMessageContentType::REFUSAL,     openai::response::ContentTypes::Refusal)
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::OutputMessage::Content)

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::OutputMessage)
    FIELD(src, content),
    FIELD(src, id),
    FIELD(src, role),
    FIELD(src, status),
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::FileSearchToolCall::Result)
    FIELD(src, attributes),
    FIELD(src, file_id),
    FIELD(src, filename),
    FIELD(src, score),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::FileSearchToolCall::Result::AttributeValue)
    switch (src.type()) {
    case simdjson::dom::element_type::BOOL:   return T{DeserializeTo<bool>(src)};
    case simdjson::dom::element_type::UINT64:
    case simdjson::dom::element_type::INT64:
    case simdjson::dom::element_type::DOUBLE: return T{DeserializeTo<double>(src)};
    case simdjson::dom::element_type::STRING: return T{DeserializeTo<NameLen<512>>(src)};
    default: break;
    }
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::FileSearchToolCall::Result::AttributeValue)

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::FileSearchToolCall)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, queries),
    FIELD(src, status),
    FIELD(src, results)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ComputerToolCall::PendingSafetyCheck)
    FIELD(src, id),
    FIELD(src, code),
    FIELD(src, message)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ComputerToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, pending_safety_checks),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ComputerToolCallOutput::ComputerScreenshot)
    FIELD(src, type),
    FIELD(src, file_id),
    FIELD(src, image_url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ComputerToolCallOutput::AcknowledgedSafetyCheck)
    FIELD(src, id),
    FIELD(src, code),
    FIELD(src, message)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ComputerToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, acknowledged_safety_checks),
    FIELD(src, id),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::WebSearchToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, id),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::FunctionToolCall)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, call_id),
    FIELD(src, name),
    FIELD(src, id),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::FunctionToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::Reasoning::Summary)
    FIELD(src, type),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::Reasoning::Content)
    FIELD(src, type),
    FIELD(src, text)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::Reasoning)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, summary),
    FIELD(src, content),
    FIELD(src, encrypted_content),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CompactionItem)
    FIELD(src, type),
    FIELD(src, encrypted_content),
    FIELD(src, id),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ImageGenerationCall)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, result),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputLog)
    FIELD(src, type),
    FIELD(src, logs)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputImage)
    FIELD(src, type),
    FIELD(src, url)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::CodeInterpreterToolCall::Output)
    using BaseT = openai::response::InputTypes::Item::CodeInterpreterToolCall;
    auto kind = EXTRACT_KIND(openai::CodeInterpreterOutputType, src, "type");
    FIELD_KIND(src, kind, openai::CodeInterpreterOutputType::LOGS,  BaseT::CodeInterpreterOutputLog)
    FIELD_KIND(src, kind, openai::CodeInterpreterOutputType::IMAGE, BaseT::CodeInterpreterOutputImage)
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::CodeInterpreterToolCall::Output)

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CodeInterpreterToolCall)
    FIELD(src, type),
    FIELD(src, code),
    FIELD(src, container_id),
    FIELD(src, id),
    FIELD(src, outputs),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::LocalShellCall::Action)
    FIELD(src, type),
    FIELD(src, command),
    FIELD(src, env),
    FIELD(src, timeout_ms),
    FIELD(src, user),
    FIELD(src, working_directory)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::LocalShellCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::LocalShellCallOutput)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, output),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCall::Action)
    FIELD(src, commands),
    FIELD(src, max_output_length),
    FIELD(src, timeout_ms)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCall)
    FIELD(src, type),
    FIELD(src, action),
    FIELD(src, call_id),
    FIELD(src, id),
    FIELD(src, status),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome)
    FIELD(src, type),
    FIELD(src, exit_code)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome)
    using BaseT = openai::response::InputTypes::Item::ShellToolCallOutput;
    auto kind = EXTRACT_KIND(openai::ShellCallOutcomeType, src, "type");
    FIELD_KIND(src, kind, openai::ShellCallOutcomeType::EXIT,    BaseT::ShellCallExitOutcome)
    FIELD_KIND(src, kind, openai::ShellCallOutcomeType::TIMEOUT, BaseT::ShellCallTimeoutOutcome)
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::ShellToolCallOutput::Output::Outcome)

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCallOutput::Output)
    FIELD(src, outcome),
    Extract<"stderr",
            openai::response::InputTypes::Item::ShellToolCallOutput::Output,
            &openai::response::InputTypes::Item::ShellToolCallOutput::Output::std_err>(src),
    Extract<"stdout",
            openai::response::InputTypes::Item::ShellToolCallOutput::Output,
            &openai::response::InputTypes::Item::ShellToolCallOutput::Output::std_out>(src),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ShellToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id),
    FIELD(src, max_output_length),
    FIELD(src, status),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation)
    auto kind = EXTRACT_KIND(openai::ApplyPatchOperationType, src, "type");
    FIELD_KIND(src, kind, openai::ApplyPatchOperationType::CREATE_FILE, openai::PatchFileOperations::Create)
    FIELD_KIND(src, kind, openai::ApplyPatchOperationType::DELETE_FILE, openai::PatchFileOperations::Delete)
    FIELD_KIND(src, kind, openai::ApplyPatchOperationType::UPDATE_FILE, openai::PatchFileOperations::Update)
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::ApplyPatchToolCall::ApplyPatchOperation)

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ApplyPatchToolCall)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, operation),
    FIELD(src, status),
    FIELD(src, id),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::ApplyPatchToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, status),
    FIELD(src, id),
    FIELD(src, output),
    FIELD(src, created_by)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::MCPListTools::ToolDef)
    FIELD(src, input_schema),
    FIELD(src, name),
    FIELD(src, annotations),
    FIELD(src, description)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::MCPListTools)
    FIELD(src, type),
    FIELD(src, id),
    FIELD(src, server_label),
    FIELD(src, tools),
    FIELD(src, error)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::MCPApprovalRequest)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::MCPApprovalResponse)
    FIELD(src, type),
    FIELD(src, approval_request_id),
    FIELD(src, approve),
    FIELD(src, id),
    FIELD(src, reason)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::MCPToolCall)
    FIELD(src, type),
    FIELD(src, arguments),
    FIELD(src, id),
    FIELD(src, name),
    FIELD(src, server_label),
    FIELD(src, approval_request_id),
    FIELD(src, error),
    FIELD(src, output),
    FIELD(src, status)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CustomToolCallOutput)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, output),
    FIELD(src, id)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::InputTypes::Item::CustomToolCall)
    FIELD(src, type),
    FIELD(src, call_id),
    FIELD(src, input),
    FIELD(src, name),
    FIELD(src, id)
END_DESERIALIZE


/***
 * ItemReference & Prompt
 */
BEGIN_DESERIALIZE(openai::response::ItemReference)
    FIELD(src, type),
    FIELD(src, id)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::Prompt::VariableTypes)
    if (src.is_string()) { return T{DeserializeTo<std::string>(src)}; }

    auto kind = EXTRACT_KIND(openai::ContentType, src, "type");
    FIELD_KIND(src, kind, openai::ContentType::INPUT_TEXT, openai::response::ContentTypes::Text)
    FIELD_KIND(src, kind, openai::ContentType::INPUT_IMAGE, openai::response::ContentTypes::Image)
    FIELD_KIND(src, kind, openai::ContentType::INPUT_FILE, openai::response::ContentTypes::File)
END_DESERIALIZE_VARIANT(openai::response::Prompt::VariableTypes)

BEGIN_DESERIALIZE(openai::response::Prompt)
    FIELD(src, id),
    FIELD(src, variables),
    FIELD(src, version)
END_DESERIALIZE


/***
 * ToolTypes
 */
BEGIN_DESERIALIZE(openai::response::ToolTypes::Function)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, parameters),
    FIELD(src, strict),
    FIELD(src, description)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType)
    switch (src.type()) {
    case simdjson::dom::element_type::STRING: return T{DeserializeTo<std::string>(src)};
    case simdjson::dom::element_type::DOUBLE:
    case simdjson::dom::element_type::INT64:
    case simdjson::dom::element_type::UINT64: return T{DeserializeTo<double>(src)};
    case simdjson::dom::element_type::BOOL:   return T{DeserializeTo<bool>(src)};
    case simdjson::dom::element_type::ARRAY: {
        auto arr = src.get_array();
        if (arr.size() == 0) return T{std::vector<std::string>{}};
        auto first = arr.at(0);
        if      (first.is_string()) return T{DeserializeTo<std::vector<std::string>>(src)};
        else if (first.is_number()) return T{DeserializeTo<std::vector<double>>(src)};
        else if (first.is_bool())   return T{DeserializeTo<std::vector<bool>>(src)};
    }
    default: break;
    }
END_DESERIALIZE_VARIANT(openai::response::ToolTypes::FileSearch::ComparisonFilter::ValueType)

BEGIN_DESERIALIZE(openai::response::ToolTypes::FileSearch::ComparisonFilter)
    FIELD(src, key),
    FIELD(src, type),
    FIELD(src, value)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::FileSearch::CompoundFilter)
    FIELD(src, type),
    FIELD(src, filters)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::FileSearch::Filter)
    auto obj = src.get_object();
    if (obj["filters"].error() == simdjson::SUCCESS) {
        return T{DeserializeTo<openai::response::ToolTypes::FileSearch::CompoundFilter>(src)};
    } else {
        return T{DeserializeTo<openai::response::ToolTypes::FileSearch::ComparisonFilter>(src)};
    }
}

BEGIN_DESERIALIZE(openai::response::ToolTypes::FileSearch::RankingOptions::HybridSearch)
    FIELD(src, embedding_weight),
    FIELD(src, text_weight)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::FileSearch::RankingOptions)
    FIELD(src, hybrid_search),
    FIELD(src, ranker),
    FIELD(src, score_threshold)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::FileSearch)
    FIELD(src, type),
    FIELD(src, vector_store_ids),
    FIELD(src, filters),
    FIELD(src, max_num_results),
    FIELD(src, ranking_options)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::ComputerUse)
    FIELD(src, type),
    FIELD(src, display_height),
    FIELD(src, display_width),
    FIELD(src, environment)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::WebSearch::Filters)
    FIELD(src, allowed_domains)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::WebSearch::Location)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::WebSearch)
    FIELD(src, type),
    FIELD(src, filters),
    FIELD(src, search_context_size),
    FIELD(src, user_location)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::MCP::Filter)
    FIELD(src, read_only),
    FIELD(src, tool_names)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::MCP::ApprovalFilter)
    FIELD(src, always),
    FIELD(src, never)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::MCP::AllowedTools)
    if (src.is_array()) { return T{DeserializeTo<std::vector<std::string>>(src)}; }
    else {                return T{DeserializeTo<openai::response::ToolTypes::MCP::Filter>(src)}; }
}

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::MCP::RequiredApproval)
    if (src.is_string()) {
        auto sv = src.get_string().value();
        auto opt = from_string_view<openai::MCPApprovalSetting>(sv);
        if (opt) return T{*opt};
        throw AnnotatedException{std::string{"openai::MCPApprovalSetting missing type: "} + std::string{sv}};
    }
    return T{DeserializeTo<openai::response::ToolTypes::MCP::ApprovalFilter>(src)};
}

BEGIN_DESERIALIZE(openai::response::ToolTypes::MCP)
    FIELD(src, type),
    FIELD(src, server_label),
    FIELD(src, allowed_tools),
    FIELD(src, authorization),
    FIELD(src, connector_id),
    FIELD(src, headers),
    FIELD(src, require_approval),
    FIELD(src, server_description),
    FIELD(src, server_url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::CodeInterpreter::ContainerConfig)
    FIELD(src, type),
    FIELD(src, file_ids),
    FIELD(src, memory_limit)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::CodeInterpreter::Container)
    using BaseT = openai::response::ToolTypes::CodeInterpreter;
    if (src.is_string()) { return T{DeserializeTo<std::string>(src)}; }
    else {                 return T{DeserializeTo<BaseT::ContainerConfig>(src)}; }
}

BEGIN_DESERIALIZE(openai::response::ToolTypes::CodeInterpreter)
    FIELD(src, type),
    FIELD(src, container)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::ImageGeneration::Mask)
    FIELD(src, file_id),
    FIELD(src, image_url)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::ImageGeneration)
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
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::LocalShell)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::Shell)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::Custom::GrammarFormat)
    FIELD(src, type),
    FIELD(src, definition),
    FIELD(src, syntax)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::Custom::TextFormat)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolTypes::Custom::Format)
    auto kind = EXTRACT_KIND(openai::CustomToolFormatType, src, "type");
    FIELD_KIND(src, kind, openai::CustomToolFormatType::GRAMMAR, openai::response::ToolTypes::Custom::GrammarFormat)
    FIELD_KIND(src, kind, openai::CustomToolFormatType::TEXT,    openai::response::ToolTypes::Custom::TextFormat)
END_DESERIALIZE_VARIANT(openai::response::ToolTypes::Custom::Format)

BEGIN_DESERIALIZE(openai::response::ToolTypes::Custom)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, description),
    FIELD(src, format)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::WebSearchPreview::Location)
    FIELD(src, type),
    FIELD(src, city),
    FIELD(src, country),
    FIELD(src, region),
    FIELD(src, timezone)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::WebSearchPreview)
    FIELD(src, type),
    FIELD(src, search_context_size),
    FIELD(src, user_location)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolTypes::ApplyPatch)
    FIELD(src, type)
END_DESERIALIZE


/***
 * ToolsChoiceTypes
 */
BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::Allowed::RestrictedTool)
    FIELD(src, type),
    FIELD(src, name),
    FIELD(src, server_label)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::Allowed)
    FIELD(src, type),
    FIELD(src, mode),
    FIELD(src, tools)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::Custom)
    FIELD(src, type),
    FIELD(src, name)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::Function)
    FIELD(src, type),
    FIELD(src, name)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::Hosted)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::MCP)
    FIELD(src, type),
    FIELD(src, server_label),
    FIELD(src, name)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::SpecificApplyPatch)
    FIELD(src, type)
END_DESERIALIZE

BEGIN_DESERIALIZE(openai::response::ToolsChoiceTypes::SpecificShell)
    FIELD(src, type)
END_DESERIALIZE


/***
 * Top-level Variants
 */
BEGIN_DESERIALIZE_VARIANT(openai::response::Item)
    using BaseT = openai::response::InputTypes::Item;
    auto kind = EXTRACT_KIND(openai::OutputItemType, src, "type");
    FIELD_KIND(src, kind, openai::OutputItemType::MESSAGE,                 BaseT::OutputMessage)
    FIELD_KIND(src, kind, openai::OutputItemType::FILE_SEARCH_CALL,        BaseT::FileSearchToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::COMPUTER_CALL,           BaseT::ComputerToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::WEB_SEARCH_CALL,         BaseT::WebSearchToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::FUNCTION_CALL,           BaseT::FunctionToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::REASONING,               BaseT::Reasoning)
    FIELD_KIND(src, kind, openai::OutputItemType::COMPACTION,              BaseT::CompactionItem)
    FIELD_KIND(src, kind, openai::OutputItemType::IMAGE_GENERATION_CALL,   BaseT::ImageGenerationCall)
    FIELD_KIND(src, kind, openai::OutputItemType::CODE_INTERPRETER_CALL,   BaseT::CodeInterpreterToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::LOCAL_SHELL_CALL,        BaseT::LocalShellCall)
    FIELD_KIND(src, kind, openai::OutputItemType::SHELL_CALL,              BaseT::ShellToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::SHELL_CALL_OUTPUT,       BaseT::ShellToolCallOutput)
    FIELD_KIND(src, kind, openai::OutputItemType::APPLY_PATCH_CALL,        BaseT::ApplyPatchToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::APPLY_PATCH_CALL_OUTPUT, BaseT::ApplyPatchToolCallOutput)
    FIELD_KIND(src, kind, openai::OutputItemType::MCP_LIST_TOOLS,          BaseT::MCPListTools)
    FIELD_KIND(src, kind, openai::OutputItemType::MCP_APPROVAL_REQUEST,    BaseT::MCPApprovalRequest)
    FIELD_KIND(src, kind, openai::OutputItemType::MCP_CALL,                BaseT::MCPToolCall)
    FIELD_KIND(src, kind, openai::OutputItemType::CUSTOM_TOOL_CALL,        BaseT::CustomToolCall)
END_DESERIALIZE_VARIANT(openai::response::Item)

BEGIN_DESERIALIZE_VARIANT(openai::response::Tool)
    auto kind = EXTRACT_KIND(openai::ToolType, src, "type");
    FIELD_KIND(src, kind, openai::ToolType::FUNCTION,             openai::response::ToolTypes::Function)
    FIELD_KIND(src, kind, openai::ToolType::FILE_SEARCH,          openai::response::ToolTypes::FileSearch)
    FIELD_KIND(src, kind, openai::ToolType::COMPUTER_USE_PREVIEW, openai::response::ToolTypes::ComputerUse)
    FIELD_KIND(src, kind, openai::ToolType::WEB_SEARCH,           openai::response::ToolTypes::WebSearch)
    FIELD_KIND(src, kind, openai::ToolType::MCP,                  openai::response::ToolTypes::MCP)
    FIELD_KIND(src, kind, openai::ToolType::CODE_INTERPRETER,     openai::response::ToolTypes::CodeInterpreter)
    FIELD_KIND(src, kind, openai::ToolType::IMAGE_GENERATION,     openai::response::ToolTypes::ImageGeneration)
    FIELD_KIND(src, kind, openai::ToolType::LOCAL_SHELL,          openai::response::ToolTypes::LocalShell)
    FIELD_KIND(src, kind, openai::ToolType::SHELL,                openai::response::ToolTypes::Shell)
    FIELD_KIND(src, kind, openai::ToolType::CUSTOM,               openai::response::ToolTypes::Custom)
    FIELD_KIND(src, kind, openai::ToolType::WEB_SEARCH_PREVIEW,   openai::response::ToolTypes::WebSearchPreview)
    FIELD_KIND(src, kind, openai::ToolType::APPLY_PATCH,          openai::response::ToolTypes::ApplyPatch)
END_DESERIALIZE_VARIANT(openai::response::Tool)

BEGIN_DESERIALIZE_VARIANT(openai::response::OutputItemList)
    auto kind = EXTRACT_KIND(openai::OutputItemType, src, "type");
    FIELD_KIND(src, kind, openai::OutputItemType::MESSAGE,        openai::response::InputTypes::Message)
    FIELD_KIND(src, kind, openai::OutputItemType::ITEM_REFERENCE, openai::response::ItemReference)
    return T{DeserializeTo<openai::response::Item>(src)};
END_DESERIALIZE_VARIANT(openai::response::OutputItemList)

BEGIN_DESERIALIZE_VARIANT(openai::response::Output)
    if (src.is_string()) { return T{DeserializeTo<std::string>(src)}; }
    else {                 return T{DeserializeTo<std::vector<openai::response::OutputItemList>>(src)}; }
}

BEGIN_DESERIALIZE_VARIANT(openai::response::ToolChoice)
    if (src.is_string()) {
        auto type_sv = src.get_string().value();
        auto opt_mode = from_string_view<openai::ToolChoiceMode>(type_sv);
        if (opt_mode) return T{*opt_mode};
        throw AnnotatedException{std::string{"openai::response::ToolChoice missing type: "} + std::string{type_sv}};
    }

    auto obj = src.get_object();
    std::string_view type_sv = obj["type"].get_string().value();

    auto opt_hosted = from_string_view<openai::HostedToolMode>(type_sv);
    if (opt_hosted) { return T{openai::response::ToolsChoiceTypes::Hosted{*opt_hosted}}; }

    auto kind = EXTRACT_KIND(openai::ToolChoiceType, src, "type");
    FIELD_KIND(src, kind, openai::ToolChoiceType::ALLOWED_TOOLS, openai::response::ToolsChoiceTypes::Allowed)
    FIELD_KIND(src, kind, openai::ToolChoiceType::CUSTOM,        openai::response::ToolsChoiceTypes::Custom)
    FIELD_KIND(src, kind, openai::ToolChoiceType::FUNCTION,      openai::response::ToolsChoiceTypes::Function)
    FIELD_KIND(src, kind, openai::ToolChoiceType::MCP,           openai::response::ToolsChoiceTypes::MCP)
    FIELD_KIND(src, kind, openai::ToolChoiceType::APPLY_PATCH,   openai::response::ToolsChoiceTypes::SpecificApplyPatch)
    FIELD_KIND(src, kind, openai::ToolChoiceType::SHELL,         openai::response::ToolsChoiceTypes::SpecificShell)
END_DESERIALIZE_VARIANT(openai::response::ToolChoice)


/***
 * Top-level Response
 */
BEGIN_DESERIALIZE(openai::Response)
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
END_DESERIALIZE


/***
 * Top-level Deserialize
 */
namespace openai {
    Response Deserialize(const curl::Response& response) {
        if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
            throw AnnotatedException{"Simdjson padding check failed"};
        }

        static thread_local simdjson::dom::parser parser{};

        try {
            simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()),
                                                      response.body_len);
            return DeserializeTo<Response>(doc);
        } catch (const simdjson::simdjson_error& e) {
            throw AnnotatedException{"openai::Deserialize Failed", e.what()};
        }
    }
}


}


#if 0
BEGIN_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::FileSearchToolCall::Result::AttributeMap)
template <>
std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>> 
DeserializeTo<>(const simdjson::dom::element& src) {
    return  src.get_object() |
            std::views::transform([](auto&& kv) {
                auto const& [key, value] = kv;
                return std::pair{
                    NameLen<64>{std::string{key}},
                    DeserializeTo<std::variant<NameLen<512>, bool, double>>(value)
                };
            }) |
            std::ranges::to<T>();
        }
END_DESERIALIZE_VARIANT(openai::response::InputTypes::Item::FileSearchToolCall::Result::AttributeMap)
#endif
