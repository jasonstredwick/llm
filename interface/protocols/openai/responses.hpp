/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <map>

#include "responses_enums.hpp"
#include "responses_common.hpp"
#include "responses_req.hpp"
#include "responses_res.hpp"
#include "../../core/types.hpp"


namespace jai::llm::openai {


struct Request {
    std::optional<bool> background{};
    std::optional<std::variant<std::string, ConversationRef>> conversation{};
    std::optional<std::vector<IncludeOutputData>> include{};
    std::optional<request::Input> input{};
    std::optional<std::string> instructions{};
    std::optional<int64_t> max_output_tokens{};
    std::optional<int64_t> max_tool_calls{};
    std::optional<std::map<NameLen<64>, NameLen<512>>> metadata{};
    std::optional<std::string> model{};
    std::optional<bool> parallel_tool_calls{};
    std::optional<std::string> previous_response_id{};
    std::optional<request::Prompt> prompt{};
    std::optional<std::string> prompt_cache_key{};
    std::optional<std::string> prompt_cache_retention{};
    std::optional<Reasoning> reasoning{};
    std::optional<std::string> safety_identifier{};
    std::optional<ServiceTier> service_tier{};
    std::optional<bool> store{};
    std::optional<bool> stream{};
    std::optional<StreamOptions> stream_options{};
    std::optional<double> temperature{};
    std::optional<TextConfig> text{};
    std::optional<request::ToolChoice> tool_choice{};
    std::optional<std::vector<request::Tool>> tools{};
    std::optional<int64_t> top_logprobs{};
    std::optional<double> top_p{};
    std::optional<TruncationStrategy> truncation{};
};


struct Response {
    ResponseKind object = ResponseKind::RESPONSE;
    bool background;
    double completed_at;
    ConversationRef conversation;
    double created_at;
    ResponseError error;
    std::string id;
    IncompleteDetails incomplete_details;
    response::Output instructions;
    int64_t max_output_tokens;
    int64_t max_tool_calls;
    std::map<NameLen<64>, NameLen<512>> metadata;
    std::string model;
    std::vector<response::Item> output;
    bool parallel_tool_calls;
    std::string previous_response_id;
    response::Prompt prompt;
    std::string prompt_cache_key;
    std::string prompt_cache_retention;
    Reasoning reasoning;
    std::string safety_identifier;
    ServiceTier service_tier;
    ResponseStatus status;
    double temperature;
    TextConfig text;
    response::ToolChoice tool_choice;
    std::vector<response::Tool> tools;
    int64_t top_logprobs;
    double top_p;
    TruncationStrategy truncation;
    ResponseUsage usage;
};


}
