#include "../../interface/protocols/openai/responses.hpp"
#include "base.hpp"


using namespace simdjson::builder;


namespace jai::llm::openai {


/***
 * ComputerToolActions
 */
BEGIN_SERIALIZE(ComputerToolActions::Click)
    FIELD(obj, type,   CommaDirection::NONE)
    FIELD(obj, button, CommaDirection::BEFORE)
    FIELD(obj, x,      CommaDirection::BEFORE)
    FIELD(obj, y,      CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::DoubleClick)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, x,    CommaDirection::BEFORE)
    FIELD(obj, y,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Drag::Coordinate)
    FIELD(obj, x, CommaDirection::NONE)
    FIELD(obj, y, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Drag)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, path, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::KeyPress)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, keys, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Move)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, x,    CommaDirection::BEFORE)
    FIELD(obj, y,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Screenshot)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Scroll)
    FIELD(obj, type,     CommaDirection::NONE)
    FIELD(obj, scroll_x, CommaDirection::BEFORE)
    FIELD(obj, scroll_y, CommaDirection::BEFORE)
    FIELD(obj, x,        CommaDirection::BEFORE)
    FIELD(obj, y,        CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Type)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, text, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ComputerToolActions::Wait)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE


/***
 * Shared Substructures
 */
BEGIN_SERIALIZE(ConversationRef)
    FIELD(obj, id, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(PatchFileOperations::Create)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, diff, CommaDirection::BEFORE)
    FIELD(obj, path, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(PatchFileOperations::Delete)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, path, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(PatchFileOperations::Update)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, diff, CommaDirection::BEFORE)
    FIELD(obj, path, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(IncompleteDetails)
    FIELD(obj, reason, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(Reasoning)
    FIELD(obj, effort,  CommaDirection::NONE)
    FIELD(obj, summary, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ResponseError)
    FIELD(obj, code,    CommaDirection::NONE)
    FIELD(obj, message, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ResponseUsage::InputTokenDetails)
    FIELD(obj, cached_tokens, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ResponseUsage::OutputTokenDetails)
    FIELD(obj, reasoning_tokens, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ResponseUsage)
    FIELD(obj, input_tokens,          CommaDirection::NONE)
    FIELD(obj, input_tokens_details,  CommaDirection::BEFORE)
    FIELD(obj, output_tokens,         CommaDirection::BEFORE)
    FIELD(obj, output_tokens_details, CommaDirection::BEFORE)
    FIELD(obj, total_tokens,          CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(StreamOptions)
    FIELD(obj, include_obfuscation, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(TextConfig::FormatText)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(TextConfig::FormatJsonSchema)
    FIELD(obj, type,        CommaDirection::NONE)
    FIELD(obj, name,        CommaDirection::BEFORE)
    FIELD(obj, schema,      CommaDirection::BEFORE)
    FIELD(obj, description, CommaDirection::BEFORE)
    FIELD(obj, strict,      CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(TextConfig)
    FIELD(obj, format,    CommaDirection::NONE)
    FIELD(obj, verbosity, CommaDirection::BEFORE)
END_SERIALIZE


namespace request {


/***
 * ContentTypes
 */
BEGIN_SERIALIZE(ContentTypes::File)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, file_data, CommaDirection::BEFORE)
    FIELD(obj, file_id,   CommaDirection::BEFORE)
    FIELD(obj, file_url,  CommaDirection::BEFORE)
    FIELD(obj, filename,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::Image)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, detail,    CommaDirection::BEFORE)
    FIELD(obj, file_id,   CommaDirection::BEFORE)
    FIELD(obj, image_url, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::ContainerFileCitation)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, container_id, CommaDirection::BEFORE)
    FIELD(obj, end_index,    CommaDirection::BEFORE)
    FIELD(obj, file_id,      CommaDirection::BEFORE)
    FIELD(obj, filename,     CommaDirection::BEFORE)
    FIELD(obj, start_index,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::FileCitation)
    FIELD(obj, type,     CommaDirection::NONE)
    FIELD(obj, file_id,  CommaDirection::BEFORE)
    FIELD(obj, filename, CommaDirection::BEFORE)
    FIELD(obj, index,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::UrlCitation)
    FIELD(obj, type,        CommaDirection::NONE)
    FIELD(obj, end_index,   CommaDirection::BEFORE)
    FIELD(obj, start_index, CommaDirection::BEFORE)
    FIELD(obj, title,       CommaDirection::BEFORE)
    FIELD(obj, url,         CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::FilePath)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, file_id, CommaDirection::BEFORE)
    FIELD(obj, index,   CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::LogProb::TopLogprob)
    FIELD(obj, bytes,   CommaDirection::NONE)
    FIELD(obj, logprob, CommaDirection::BEFORE)
    FIELD(obj, token,   CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText::LogProb)
    FIELD(obj, bytes,         CommaDirection::NONE)
    FIELD(obj, logprob,       CommaDirection::BEFORE)
    FIELD(obj, token,         CommaDirection::BEFORE)
    FIELD(obj, top_logprobs, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::OutputText)
    FIELD(obj, type,        CommaDirection::NONE)
    FIELD(obj, annotations, CommaDirection::BEFORE)
    FIELD(obj, logprobs,    CommaDirection::BEFORE)
    FIELD(obj, text,        CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::Refusal)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, refusal, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ContentTypes::Text)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, text, CommaDirection::BEFORE)
END_SERIALIZE


/***
 * request::WebSearchToolActions
 */
BEGIN_SERIALIZE(WebSearchToolActions::Find)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, pattern, CommaDirection::BEFORE)
    FIELD(obj, url,     CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolActions::OpenPage)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, url,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolActions::Search::Source)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, url,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(WebSearchToolActions::Search)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, queries, CommaDirection::BEFORE)
    FIELD(obj, sources, CommaDirection::BEFORE)
END_SERIALIZE


/***
 * request::InputTypes
 */
BEGIN_SERIALIZE(InputTypes::Message)
    FIELD(obj, content, CommaDirection::NONE)
    FIELD(obj, role,    CommaDirection::BEFORE)
    FIELD(obj, type,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::InputMessage)
    FIELD(obj, content, CommaDirection::NONE)
    FIELD(obj, role,    CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
    FIELD(obj, type,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::OutputMessage)
    FIELD(obj, content, CommaDirection::NONE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, role,    CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
    FIELD(obj, type,    CommaDirection::BEFORE)
END_SERIALIZE


/***
 * request::ItemReference
 */
BEGIN_SERIALIZE(ItemReference)
    FIELD(obj, type, CommaDirection::NONE)
    if (obj.type) { builder.append_comma(); }
    FIELD(obj, id, CommaDirection::NONE)
END_SERIALIZE


/***
 * request::ToolCallItems
 */
BEGIN_SERIALIZE(InputTypes::Item::FileSearchToolCall::Result)
    FIELD(obj, attributes, CommaDirection::NONE)
    FIELD(obj, file_id,    CommaDirection::BEFORE)
    FIELD(obj, filename,   CommaDirection::BEFORE)
    FIELD(obj, score,      CommaDirection::BEFORE)
    FIELD(obj, text,       CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::FileSearchToolCall)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, queries, CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
    FIELD(obj, results, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ComputerToolCall::PendingSafetyCheck)
    FIELD(obj, id,      CommaDirection::NONE)
    FIELD(obj, code,    CommaDirection::BEFORE)
    FIELD(obj, message, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ComputerToolCall)
    FIELD(obj, type,                  CommaDirection::NONE)
    FIELD(obj, action,                CommaDirection::BEFORE)
    FIELD(obj, call_id,               CommaDirection::BEFORE)
    FIELD(obj, id,                    CommaDirection::BEFORE)
    FIELD(obj, pending_safety_checks, CommaDirection::BEFORE)
    FIELD(obj, status,                CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ComputerToolCallOutput::ComputerScreenshot)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, file_id,   CommaDirection::BEFORE)
    FIELD(obj, image_url, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ComputerToolCallOutput::AcknowledgedSafetyCheck)
    FIELD(obj, id,      CommaDirection::NONE)
    FIELD(obj, code,    CommaDirection::BEFORE)
    FIELD(obj, message, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ComputerToolCallOutput)
    FIELD(obj, type,                       CommaDirection::NONE)
    FIELD(obj, call_id,                    CommaDirection::BEFORE)
    FIELD(obj, output,                     CommaDirection::BEFORE)
    FIELD(obj, acknowledged_safety_checks, CommaDirection::BEFORE)
    FIELD(obj, id,                         CommaDirection::BEFORE)
    FIELD(obj, status,                     CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::WebSearchToolCall)
    FIELD(obj, type,   CommaDirection::NONE)
    FIELD(obj, action, CommaDirection::BEFORE)
    FIELD(obj, id,     CommaDirection::BEFORE)
    FIELD(obj, status, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::FunctionToolCall)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, arguments, CommaDirection::BEFORE)
    FIELD(obj, call_id,   CommaDirection::BEFORE)
    FIELD(obj, name,      CommaDirection::BEFORE)
    FIELD(obj, id,        CommaDirection::BEFORE)
    FIELD(obj, status,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::FunctionToolCallOutput)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, output,  CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::Reasoning::Summary)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, text, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::Reasoning::Content)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, text, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::Reasoning)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, id,                CommaDirection::BEFORE)
    FIELD(obj, summary,           CommaDirection::BEFORE)
    FIELD(obj, content,           CommaDirection::BEFORE)
    FIELD(obj, encrypted_content, CommaDirection::BEFORE)
    FIELD(obj, status,            CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CompactionItem)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, encrypted_content, CommaDirection::BEFORE)
    FIELD(obj, id,                CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ImageGenerationCall)
    FIELD(obj, type,   CommaDirection::NONE)
    FIELD(obj, id,     CommaDirection::BEFORE)
    FIELD(obj, result, CommaDirection::BEFORE)
    FIELD(obj, status, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputLog)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, logs, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CodeInterpreterToolCall::CodeInterpreterOutputImage)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, url,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CodeInterpreterToolCall)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, code,         CommaDirection::BEFORE)
    FIELD(obj, container_id, CommaDirection::BEFORE)
    FIELD(obj, id,           CommaDirection::BEFORE)
    FIELD(obj, outputs,      CommaDirection::BEFORE)
    FIELD(obj, status,       CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::LocalShellCall::Action)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, command,           CommaDirection::BEFORE)
    FIELD(obj, env,               CommaDirection::BEFORE)
    FIELD(obj, timeout_ms,        CommaDirection::BEFORE)
    FIELD(obj, user,              CommaDirection::BEFORE)
    FIELD(obj, working_directory, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::LocalShellCall)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, action,  CommaDirection::BEFORE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::LocalShellCallOutput)
    FIELD(obj, type,   CommaDirection::NONE)
    FIELD(obj, id,     CommaDirection::BEFORE)
    FIELD(obj, output, CommaDirection::BEFORE)
    FIELD(obj, status, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCall::Action)
    FIELD(obj, commands,          CommaDirection::NONE)
    FIELD(obj, max_output_length, CommaDirection::BEFORE)
    FIELD(obj, timeout_ms,        CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCall)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, action,  CommaDirection::BEFORE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, status,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCallOutput::ShellCallExitOutcome)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, exit_code, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCallOutput::ShellCallTimeoutOutcome)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCallOutput::Output)
    FIELD(obj, outcome, CommaDirection::NONE)
    FIELD(obj, std_err, CommaDirection::BEFORE)
    FIELD(obj, std_out, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ShellToolCallOutput)
    FIELD(obj, type,              CommaDirection::NONE)
    FIELD(obj, call_id,           CommaDirection::BEFORE)
    FIELD(obj, output,            CommaDirection::BEFORE)
    FIELD(obj, id,                CommaDirection::BEFORE)
    FIELD(obj, max_output_length, CommaDirection::BEFORE)
    FIELD(obj, status,            CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ApplyPatchToolCall)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, call_id,   CommaDirection::BEFORE)
    FIELD(obj, operation, CommaDirection::BEFORE)
    FIELD(obj, status,    CommaDirection::BEFORE)
    FIELD(obj, id,        CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::ApplyPatchToolCallOutput)
    FIELD(obj, type,   CommaDirection::NONE)
    FIELD(obj, status, CommaDirection::BEFORE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
    FIELD(obj, output,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::MCPListTools::ToolDef)
    FIELD(obj, input_schema, CommaDirection::NONE)
    FIELD(obj, name,         CommaDirection::BEFORE)
    FIELD(obj, annotations,  CommaDirection::BEFORE)
    FIELD(obj, description,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::MCPListTools)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, id,           CommaDirection::BEFORE)
    FIELD(obj, server_label, CommaDirection::BEFORE)
    FIELD(obj, tools,        CommaDirection::BEFORE)
    FIELD(obj, error,        CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::MCPApprovalRequest)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, arguments,    CommaDirection::BEFORE)
    FIELD(obj, id,           CommaDirection::BEFORE)
    FIELD(obj, name,         CommaDirection::BEFORE)
    FIELD(obj, server_label, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::MCPApprovalResponse)
    FIELD(obj, type,                CommaDirection::NONE)
    FIELD(obj, approval_request_id, CommaDirection::BEFORE)
    FIELD(obj, approve,             CommaDirection::BEFORE)
    FIELD(obj, id,                  CommaDirection::BEFORE)
    FIELD(obj, reason,              CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::MCPToolCall)
    FIELD(obj, type,                CommaDirection::NONE)
    FIELD(obj, arguments,           CommaDirection::BEFORE)
    FIELD(obj, id,                  CommaDirection::BEFORE)
    FIELD(obj, name,                CommaDirection::BEFORE)
    FIELD(obj, server_label,        CommaDirection::BEFORE)
    FIELD(obj, approval_request_id, CommaDirection::BEFORE)
    FIELD(obj, error,               CommaDirection::BEFORE)
    FIELD(obj, output,              CommaDirection::BEFORE)
    FIELD(obj, status,              CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CustomToolCallOutput)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, output,  CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(InputTypes::Item::CustomToolCall)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, call_id, CommaDirection::BEFORE)
    FIELD(obj, input,   CommaDirection::BEFORE)
    FIELD(obj, name,    CommaDirection::BEFORE)
    FIELD(obj, id,      CommaDirection::BEFORE)
END_SERIALIZE


/***
 * request::Prompt
 */
BEGIN_SERIALIZE(Prompt)
    FIELD(obj, id,        CommaDirection::NONE)
    FIELD(obj, variables, CommaDirection::BEFORE)
    FIELD(obj, version,   CommaDirection::BEFORE)
END_SERIALIZE


/***
 * request::ToolTypes
 */
BEGIN_SERIALIZE(ToolTypes::Function)
    FIELD(obj, type,        CommaDirection::NONE)
    FIELD(obj, name,        CommaDirection::BEFORE)
    FIELD(obj, parameters,  CommaDirection::BEFORE)
    FIELD(obj, strict,      CommaDirection::BEFORE)
    FIELD(obj, description, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::FileSearch::ComparisonFilter)
    FIELD(obj, key,   CommaDirection::NONE)
    FIELD(obj, type,  CommaDirection::BEFORE)
    FIELD(obj, value, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::FileSearch::CompoundFilter)
    FIELD(obj, type,    CommaDirection::NONE)
    FIELD(obj, filters, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::FileSearch::RankingOptions::HybridSearch)
    FIELD(obj, embedding_weight, CommaDirection::NONE)
    FIELD(obj, text_weight,      CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::FileSearch::RankingOptions)
    FIELD(obj, hybrid_search,   CommaDirection::NONE)
    FIELD(obj, ranker,          CommaDirection::BEFORE)
    FIELD(obj, score_threshold, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::FileSearch)
    FIELD(obj, type,             CommaDirection::NONE)
    FIELD(obj, vector_store_ids, CommaDirection::BEFORE)
    FIELD(obj, filters,          CommaDirection::BEFORE)
    FIELD(obj, max_num_results,  CommaDirection::BEFORE)
    FIELD(obj, ranking_options,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::ComputerUse)
    FIELD(obj, type,           CommaDirection::NONE)
    FIELD(obj, display_height, CommaDirection::BEFORE)
    FIELD(obj, display_width,  CommaDirection::BEFORE)
    FIELD(obj, environment,    CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::WebSearch::Filters)
    FIELD(obj, allowed_domains, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::WebSearch::Location)
    FIELD(obj, type,     CommaDirection::NONE)
    FIELD(obj, city,     CommaDirection::BEFORE)
    FIELD(obj, country,  CommaDirection::BEFORE)
    FIELD(obj, region,   CommaDirection::BEFORE)
    FIELD(obj, timezone, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::WebSearch)
    FIELD(obj, type,                CommaDirection::NONE)
    FIELD(obj, filters,             CommaDirection::BEFORE)
    FIELD(obj, search_context_size, CommaDirection::BEFORE)
    FIELD(obj, user_location,       CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::MCP::Filter)
    FIELD(obj, read_only,  CommaDirection::NONE)
    FIELD(obj, tool_names, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::MCP::ApprovalFilter)
    FIELD(obj, always, CommaDirection::NONE)
    FIELD(obj, never,  CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::MCP)
    FIELD(obj, type,               CommaDirection::NONE)
    FIELD(obj, server_label,       CommaDirection::BEFORE)
    FIELD(obj, allowed_tools,      CommaDirection::BEFORE)
    FIELD(obj, authorization,      CommaDirection::BEFORE)
    FIELD(obj, connector_id,       CommaDirection::BEFORE)
    FIELD(obj, headers,            CommaDirection::BEFORE)
    FIELD(obj, require_approval,   CommaDirection::BEFORE)
    FIELD(obj, server_description, CommaDirection::BEFORE)
    FIELD(obj, server_url,         CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::CodeInterpreter::ContainerConfig)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, file_ids,     CommaDirection::BEFORE)
    FIELD(obj, memory_limit, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::CodeInterpreter)
    FIELD(obj, type,      CommaDirection::NONE)
    FIELD(obj, container, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::ImageGeneration::Mask)
    FIELD(obj, file_id,   CommaDirection::NONE)
    FIELD(obj, image_url, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::ImageGeneration)
    FIELD(obj, type,               CommaDirection::NONE)
    FIELD(obj, action,             CommaDirection::BEFORE)
    FIELD(obj, background,         CommaDirection::BEFORE)
    FIELD(obj, input_fidelity,     CommaDirection::BEFORE)
    FIELD(obj, input_image_mask,   CommaDirection::BEFORE)
    FIELD(obj, model,              CommaDirection::BEFORE)
    FIELD(obj, moderation,         CommaDirection::BEFORE)
    FIELD(obj, output_compression, CommaDirection::BEFORE)
    FIELD(obj, output_format,      CommaDirection::BEFORE)
    FIELD(obj, partial_images,     CommaDirection::BEFORE)
    FIELD(obj, quality,            CommaDirection::BEFORE)
    FIELD(obj, size,               CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::LocalShell)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::Shell)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::Custom::GrammarFormat)
    FIELD(obj, type,       CommaDirection::NONE)
    FIELD(obj, definition, CommaDirection::BEFORE)
    FIELD(obj, syntax,     CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::Custom::TextFormat)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::Custom)
    FIELD(obj, type,        CommaDirection::NONE)
    FIELD(obj, name,        CommaDirection::BEFORE)
    FIELD(obj, description, CommaDirection::BEFORE)
    FIELD(obj, format,      CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::WebSearchPreview::Location)
    FIELD(obj, type,     CommaDirection::NONE)
    FIELD(obj, city,     CommaDirection::BEFORE)
    FIELD(obj, country,  CommaDirection::BEFORE)
    FIELD(obj, region,   CommaDirection::BEFORE)
    FIELD(obj, timezone, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::WebSearchPreview)
    FIELD(obj, type,                CommaDirection::NONE)
    FIELD(obj, search_context_size, CommaDirection::BEFORE)
    FIELD(obj, user_location,       CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolTypes::ApplyPatch)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE


/***
 * request::ToolsChoiceTypes
 */
BEGIN_SERIALIZE(ToolsChoiceTypes::Allowed::RestrictedTool)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, name,         CommaDirection::BEFORE)
    FIELD(obj, server_label, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::Allowed)
    FIELD(obj, type,  CommaDirection::NONE)
    FIELD(obj, mode,  CommaDirection::BEFORE)
    FIELD(obj, tools, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::Custom)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, name, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::Function)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, name, CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::Hosted)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::MCP)
    FIELD(obj, type,         CommaDirection::NONE)
    FIELD(obj, server_label, CommaDirection::BEFORE)
    FIELD(obj, name,         CommaDirection::BEFORE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::SpecificApplyPatch)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE

BEGIN_SERIALIZE(ToolsChoiceTypes::SpecificShell)
    FIELD(obj, type, CommaDirection::NONE)
END_SERIALIZE


} // namespace request


/***
 * Top-level Request
 */

BEGIN_SERIALIZE(Request)
    FIELD(obj, background,             CommaDirection::NONE)
    FIELD(obj, conversation,           CommaDirection::BEFORE)
    FIELD(obj, include,                CommaDirection::BEFORE)
    FIELD(obj, input,                  CommaDirection::BEFORE)
    FIELD(obj, instructions,           CommaDirection::BEFORE)
    FIELD(obj, max_output_tokens,      CommaDirection::BEFORE)
    FIELD(obj, max_tool_calls,         CommaDirection::BEFORE)
    FIELD(obj, metadata,               CommaDirection::BEFORE)
    FIELD(obj, model,                  CommaDirection::BEFORE)
    FIELD(obj, parallel_tool_calls,    CommaDirection::BEFORE)
    FIELD(obj, previous_response_id,   CommaDirection::BEFORE)
    FIELD(obj, prompt,                 CommaDirection::BEFORE)
    FIELD(obj, prompt_cache_key,       CommaDirection::BEFORE)
    FIELD(obj, prompt_cache_retention, CommaDirection::BEFORE)
    FIELD(obj, reasoning,              CommaDirection::BEFORE)
    FIELD(obj, safety_identifier,      CommaDirection::BEFORE)
    FIELD(obj, service_tier,           CommaDirection::BEFORE)
    FIELD(obj, store,                  CommaDirection::BEFORE)
    FIELD(obj, stream,                 CommaDirection::BEFORE)
    FIELD(obj, stream_options,         CommaDirection::BEFORE)
    FIELD(obj, temperature,            CommaDirection::BEFORE)
    FIELD(obj, text,                   CommaDirection::BEFORE)
    FIELD(obj, tool_choice,            CommaDirection::BEFORE)
    FIELD(obj, tools,                  CommaDirection::BEFORE)
    FIELD(obj, top_logprobs,           CommaDirection::BEFORE)
    FIELD(obj, top_p,                  CommaDirection::BEFORE)
    FIELD(obj, truncation,             CommaDirection::BEFORE)
END_SERIALIZE


/***
 * Top-level Serialize
 */
std::vector<std::byte> Serialize(const Request& request) {
    static thread_local string_builder builder{};

    try {
        builder.clear();
        SerializeFrom(builder, request);

        if (!builder.validate_unicode()) {
            throw AnnotatedException{"Serialize Failed", "string_builder generated invalid unicode data."};
        }

        auto result = builder.view();
        if (result.error() != simdjson::SUCCESS) {
            throw AnnotatedException{"Serialize Failed", simdjson::error_message(result.error())};
        }
        std::string_view json_str = result.value();

        return json_str |
               std::views::transform([](auto const& c) { return static_cast<std::byte>(c); }) |
               std::ranges::to<std::vector<std::byte>>();
    } catch (AnnotatedException const&) {
        throw;
    } catch (std::exception const& e) {
        AnnotatedException ex{"Serialize Failed", "string_builder failed to serialize Response."};
        ex.AddContext(e.what());
        throw ex;
    }
}


}
