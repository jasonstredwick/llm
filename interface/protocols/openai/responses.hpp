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
    Required<KindResponse> object{{}};
    Required<bool> background;
    Required<double> completed_at;
    Required<ConversationRef> conversation;
    Required<double> created_at;
    Required<ResponseError> error;
    Required<std::string> id;
    Required<IncompleteDetails> incomplete_details;
    Required<response::Output> instructions;
    Required<int64_t> max_output_tokens;
    Required<int64_t> max_tool_calls;
    Required<std::map<NameLen<64>, NameLen<512>>> metadata;
    Required<std::string> model;
    Required<std::vector<response::Item>> output;
    Required<bool> parallel_tool_calls;
    Required<std::string> previous_response_id;
    Required<response::Prompt> prompt;
    Required<std::string> prompt_cache_key;
    Required<std::string> prompt_cache_retention;
    Required<Reasoning> reasoning;
    Required<std::string> safety_identifier;
    Required<ServiceTier> service_tier;
    Required<ResponseStatus> status;
    Required<double> temperature;
    Required<TextConfig> text;
    Required<response::ToolChoice> tool_choice;
    Required<std::vector<response::Tool>> tools;
    Required<int64_t> top_logprobs;
    Required<double> top_p;
    Required<TruncationStrategy> truncation;
    Required<ResponseUsage> usage;
};


}
