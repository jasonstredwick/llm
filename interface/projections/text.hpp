#pragma once

#include <optional>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../core/call.hpp"
#include "../core/error.hpp"
#include "../core/types.hpp"
#include "../llm.hpp"
#include "../protocols/anthropic/messages.hpp"
#include "../protocols/anthropic/messages_strings.hpp"
#include "../protocols/gemini/generate_content.hpp"
#include "../protocols/gemini/generate_content_strings.hpp"
#include "../protocols/openai/responses.hpp"
#include "../protocols/openai/responses_strings.hpp"
#include "common.hpp"


namespace jai::llm::projection::text {


enum class ThinkingEffort { NONE, MINIMAL, LOW, MEDIUM, HIGH, MAX };


struct Image {
    std::string media_type{};
    std::string base64_content{};
};

struct Options {
    std::optional<int64_t> max_output_tokens{};
    std::optional<ThinkingEffort> thinking_effort{};
    std::optional<double> temperature{};
    std::optional<int64_t> top_k{};
    std::optional<double> top_p{};
};

struct Prompt {
    std::string text{};
};


using Block = std::variant<Image, Prompt>;


constexpr const Options default_options{
    .thinking_effort=ThinkingEffort::MINIMAL,
    .temperature=1.0
};



template <typename T> constexpr T Generate(std::optional<Prompt> system_prompt,
                                           const std::vector<Block>& content,
                                           Options options = default_options);


struct Metadata {
    std::string id{};
    std::string model{};
};

struct Result {
    std::string text{};
    projection::Diagnostics diagnostics{};
    projection::Usage usage{};
    Metadata metadata{};
};


template <typename T> constexpr Result Extract(T const& response);


template <>
constexpr anthropic::Request Generate<anthropic::Request>(std::optional<Prompt> system_prompt,
                                                          const std::vector<Block>& content,
                                                          Options options) {
    using MessageParam = anthropic::Request::MessageParam;

    auto TransformBlock_f = [](Block const& block) -> MessageParam::ContentItem {
        return std::visit(overloaded{
            [](projection::text::Image const& image) -> MessageParam::ContentItem {
                using MediaType = MessageParam::ImageBlockParam::Base64ImageSource::MediaType;
                auto media_type = jai::llm::from_string_view<MediaType>(image.media_type);
                if (!media_type) {
                    throw AnnotatedException{
                        std::string{"Unknown anthropic image MediaType provided: "} + image.media_type,
                        "while generating anthropic::Request for projection::text"
                    };
                }
                return MessageParam::ImageBlockParam{
                    .source=MessageParam::ImageBlockParam::Base64ImageSource{
                        .data=image.base64_content,
                        .media_type=*media_type
                    }
                };
            },
            [](projection::text::Prompt const& prompt) -> MessageParam::ContentItem {
                return MessageParam::TextBlockParam{.text=prompt.text};
            }
        }, block);
    };

    auto output_config = std::optional<anthropic::Request::OutputConfig>{};
    auto thinking_config = std::optional<anthropic::Request::ThinkingConfigParam>{};
    if (true) { // model version >= 4.6
        if (options.thinking_effort) {
            using Effort = anthropic::Request::OutputConfig::Effort;
            switch (*options.thinking_effort) {
                case projection::text::ThinkingEffort::NONE:
                    thinking_config = anthropic::Request::ThinkingConfigDisabled{};
                    break;
                case projection::text::ThinkingEffort::MINIMAL:
                case projection::text::ThinkingEffort::LOW:
                    thinking_config = anthropic::Request::ThinkingConfigAdaptive{};
                    output_config = anthropic::Request::OutputConfig{.effort=Effort::LOW};
                    break;
                case projection::text::ThinkingEffort::MEDIUM:
                    thinking_config = anthropic::Request::ThinkingConfigAdaptive{};
                    output_config = anthropic::Request::OutputConfig{.effort=Effort::MEDIUM};
                    break;
                case projection::text::ThinkingEffort::HIGH:
                    thinking_config = anthropic::Request::ThinkingConfigAdaptive{};
                    output_config = anthropic::Request::OutputConfig{.effort=Effort::HIGH};
                    break;
                case projection::text::ThinkingEffort::MAX:
                    thinking_config = anthropic::Request::ThinkingConfigAdaptive{};
                    if (true) { // opus 4.6
                        output_config = anthropic::Request::OutputConfig{.effort=Effort::MAX};
                    } else {
                        output_config = anthropic::Request::OutputConfig{.effort=Effort::HIGH};
                    }
                    break;
                default:
                    throw AnnotatedException{
                        "Invalid projection::text::ThinkingEffort value provided."
                    };
            }
        }
    } else {
        thinking_config = anthropic::Request::ThinkingConfigDisabled{};
    }

    auto system = std::optional<anthropic::Request::System>{};
    if (system_prompt) {
        system = system_prompt->text;
    }

    return anthropic::Request{
        .max_tokens=static_cast<double>(*options.max_output_tokens),
        .messages=std::vector<MessageParam>{MessageParam{
            .content=MessageParam::Content{content | std::views::transform(TransformBlock_f) | std::ranges::to<std::vector>()},
            .role=MessageParam::Role::USER
        }},
        .model="",
        .output_config=output_config,
        .system=std::move(system),
        .temperature=options.temperature,
        .thinking=thinking_config,
        .top_k=options.top_k ?
            std::optional{static_cast<double>(*options.top_k)} : std::nullopt,
        .top_p=options.top_p
    };
}


template <>
constexpr gemini::Request Generate<gemini::Request>(std::optional<Prompt> system_prompt,
                                                    const std::vector<Block>& content,
                                                    Options options) {
    auto TransformPart_f = [](Block const& block) -> gemini::Part {
        return std::visit(overloaded{
            [](projection::text::Image const& image) -> gemini::Part {
                return gemini::Part{
                    .inlineData=gemini::Blob{
                        .mimeType=image.media_type,
                        .data=image.base64_content
                    }
                };
            },
            [](projection::text::Prompt const& prompt) -> gemini::Part {
                return gemini::Part{.text=prompt.text};
            }
        }, block);
    };

    auto thinking_config = std::optional<gemini::ThinkingConfig>{};
    if (true) { // model version >= 3
        if (options.thinking_effort) {
            auto level = [&]() -> gemini::ThinkingLevel {
                switch (*options.thinking_effort) {
                    case projection::text::ThinkingEffort::NONE:
                    case projection::text::ThinkingEffort::MINIMAL: return gemini::ThinkingLevel::MINIMAL;
                    case projection::text::ThinkingEffort::LOW:     return gemini::ThinkingLevel::LOW;
                    case projection::text::ThinkingEffort::MEDIUM:  return gemini::ThinkingLevel::MEDIUM;
                    case projection::text::ThinkingEffort::HIGH:
                    case projection::text::ThinkingEffort::MAX:     return gemini::ThinkingLevel::HIGH;
                    default:
                        throw AnnotatedException{
                            "Invalid projection::text::ThinkingEffort value provided."
                        };
                }
            }();
            thinking_config = gemini::ThinkingConfig{
                .includeThoughts=false,
                .thinkingBudget=0, // deprecated for 2.5+, required by struct
                .thinkingLevel=level
            };
        }
    } else {
        thinking_config = gemini::ThinkingConfig{
            .includeThoughts=false,
            .thinkingBudget=0
        };
    }

    return gemini::Request{
        .model="",
        .systemInstruction=system_prompt ?
            std::optional{gemini::Content{
                .parts=std::vector<gemini::Part>{gemini::Part{.text=system_prompt->text}}
            }} : std::nullopt,
        .contents=std::vector<gemini::Content>{gemini::Content{
            .parts=content | std::views::transform(TransformPart_f) | std::ranges::to<std::vector>(),
            .role="user"
        }},
        .generationConfig=gemini::GenerationConfig{
            .candidateCount=1,
            .maxOutputTokens=options.max_output_tokens,
            .temperature=options.temperature,
            .topP=options.top_p,
            .topK=options.top_k,
            .thinkingConfig=thinking_config
        }
    };
}


template <>
constexpr openai::Request Generate<openai::Request>(std::optional<Prompt> system_prompt,
                                                    const std::vector<Block>& content,
                                                    Options options) {
    using EasyInputMessage = openai::Request::EasyInputMessage;

    auto TransformContent_f = [](Block const& block) -> EasyInputMessage::ContentItem {
        return std::visit(overloaded{
            [](projection::text::Image const& image) -> EasyInputMessage::ContentItem {
                return EasyInputMessage::ResponseInputImage{
                    .detail=EasyInputMessage::ResponseInputImage::Detail::AUTO,
                    .image_url="data:" + image.media_type + ";base64," + image.base64_content
                };
            },
            [](projection::text::Prompt const& prompt) -> EasyInputMessage::ContentItem {
                return EasyInputMessage::ResponseInputText{.text=prompt.text};
            }
        }, block);
    };

    auto input_items = std::vector<openai::Request::InputItem>{};

    if (system_prompt) {
        input_items.emplace_back(EasyInputMessage{
            .content=system_prompt->text,
            .role=EasyInputMessage::Role::SYSTEM
        });
    }

    input_items.emplace_back(EasyInputMessage{
        .content=content | std::views::transform(TransformContent_f) | std::ranges::to<std::vector>(),
        .role=EasyInputMessage::Role::USER
    });

    auto reasoning = std::optional<openai::Request::Reasoning>{};
    if (true) { // model supports reasoning (o-series, gpt-5+)
        if (options.thinking_effort) {
            using Effort = openai::Request::Reasoning::Effort;
            auto effort = [&]() -> Effort {
                switch (*options.thinking_effort) {
                    case projection::text::ThinkingEffort::NONE:
                        if (true) { return Effort::NONE; }    // gpt-5.1+
                        else      { return Effort::LOW; }
                    case projection::text::ThinkingEffort::MINIMAL: return Effort::MINIMAL;
                    case projection::text::ThinkingEffort::LOW:     return Effort::LOW;
                    case projection::text::ThinkingEffort::MEDIUM:  return Effort::MEDIUM;
                    case projection::text::ThinkingEffort::HIGH:    return Effort::HIGH;
                    case projection::text::ThinkingEffort::MAX:
                        if (true) { return Effort::XHIGH; }   // gpt-5.1-codex-max+
                        else      { return Effort::HIGH; }
                    default:
                        throw AnnotatedException{
                            "Invalid projection::text::ThinkingEffort value provided."
                        };
                }
            }();
            reasoning = openai::Request::Reasoning{
                .effort=effort,
                .summary=openai::Request::Reasoning::Summary::AUTO
            };
        }
    }

    return openai::Request{
        .input=std::move(input_items),
        .max_output_tokens=options.max_output_tokens ?
            std::optional{static_cast<double>(*options.max_output_tokens)} : std::nullopt,
        .reasoning=reasoning,
        .temperature=options.temperature,
        .top_p=options.top_p
    };
}


// --- Extract specializations ---


template <>
constexpr Result Extract<anthropic::Message>(anthropic::Message const& response) {
    auto text = std::string{};
    auto warnings = std::vector<std::string>{};

    if (response.content) {
        for (auto const& block : *response.content) {
            if (auto const* tb = std::get_if<anthropic::Message::TextBlock>(&block)) {
                if (tb->text) {
                    if (!text.empty()) { text += '\n'; }
                    text += *tb->text;
                }
            }
            // All non-text blocks (ThinkingBlock, ToolUseBlock, etc.) are silently ignored.
            // This is the text projection — we only extract text.
        }
    }

    auto stop_reason = [&]() -> projection::StopReason {
        if (!response.stop_reason) { return projection::StopReason::UNEXPECTED; }
        switch (*response.stop_reason) {
            case anthropic::Message::StopReason::END_TURN:      return projection::StopReason::COMPLETE;
            case anthropic::Message::StopReason::MAX_TOKENS:    return projection::StopReason::MAX_TOKENS;
            case anthropic::Message::StopReason::REFUSAL:       return projection::StopReason::CONTENT_FILTERED;
            case anthropic::Message::StopReason::STOP_SEQUENCE:
                warnings.emplace_back("stop_reason=STOP_SEQUENCE: unexpected for text projection");
                return projection::StopReason::UNEXPECTED;
            case anthropic::Message::StopReason::TOOL_USE:
                warnings.emplace_back("stop_reason=TOOL_USE: unexpected for text projection");
                return projection::StopReason::UNEXPECTED;
            case anthropic::Message::StopReason::PAUSE_TURN:
                warnings.emplace_back("stop_reason=PAUSE_TURN: unexpected for text projection");
                return projection::StopReason::UNEXPECTED;
            default:
                warnings.emplace_back("stop_reason: unrecognized value");
                return projection::StopReason::UNEXPECTED;
        }
    }();

    auto usage = projection::Usage{};
    if (response.usage) {
        auto const& u = *response.usage;
        usage.input_tokens = u.input_tokens ? static_cast<int64_t>(*u.input_tokens) : 0;
        usage.output_tokens = u.output_tokens ? static_cast<int64_t>(*u.output_tokens) : 0;
        if (u.cache_read_input_tokens) {
            usage.cache_read_tokens = static_cast<int64_t>(*u.cache_read_input_tokens);
        }
        if (u.cache_creation_input_tokens) {
            usage.cache_creation_tokens = static_cast<int64_t>(*u.cache_creation_input_tokens);
        }
        // Anthropic does not break out thinking tokens separately.
    }

    return Result{
        .text=std::move(text),
        .diagnostics=projection::Diagnostics{
            .stop_reason=stop_reason,
            .refusal={}, // Anthropic does not provide refusal text
            .warnings=std::move(warnings)
        },
        .usage=usage,
        .metadata=Metadata{
            .id=response.id.value_or(""),
            .model=response.model ? std::visit(overloaded{
                [](std::string const& s) -> std::string { return s; },
                [](anthropic::Message::ModelValues const& v) -> std::string {
                    return std::string{jai::llm::to_string_view(v)};
                }
            }, *response.model) : std::string{}
        }
    };
}


template <>
constexpr Result Extract<gemini::Response>(gemini::Response const& response) {
    auto text = std::string{};
    auto warnings = std::vector<std::string>{};
    auto stop_reason = projection::StopReason::UNEXPECTED;

    if (response.candidates && !response.candidates->empty()) {
        auto const& candidate = response.candidates->front();

        // Extract text from parts
        if (candidate.content && candidate.content->parts) {
            for (auto const& part : *candidate.content->parts) {
                if (part.text) {
                    if (!text.empty()) { text += '\n'; }
                    text += *part.text;
                }
                // Non-text parts (inlineData, functionCall, etc.) are silently ignored.
            }
        }

        // Map finish reason
        stop_reason = [&]() -> projection::StopReason {
            if (!candidate.finishReason) { return projection::StopReason::UNEXPECTED; }
            switch (*candidate.finishReason) {
                case gemini::FinishReason::STOP:       return projection::StopReason::COMPLETE;
                case gemini::FinishReason::MAX_TOKENS:  return projection::StopReason::MAX_TOKENS;
                case gemini::FinishReason::SAFETY:
                case gemini::FinishReason::BLOCKLIST:
                case gemini::FinishReason::PROHIBITED_CONTENT:
                case gemini::FinishReason::SPII:
                case gemini::FinishReason::IMAGE_SAFETY:
                case gemini::FinishReason::IMAGE_PROHIBITED_CONTENT:
                    return projection::StopReason::CONTENT_FILTERED;
                case gemini::FinishReason::RECITATION:
                    warnings.emplace_back("finishReason=RECITATION: copyright filter triggered");
                    return projection::StopReason::CONTENT_FILTERED;
                case gemini::FinishReason::MALFORMED_FUNCTION_CALL:
                case gemini::FinishReason::UNEXPECTED_TOOL_CALL:
                case gemini::FinishReason::TOO_MANY_TOOL_CALLS:
                    warnings.emplace_back("finishReason indicates tool-related issue: unexpected for text projection");
                    return projection::StopReason::UNEXPECTED;
                default:
                    warnings.emplace_back("finishReason: unrecognized or uncommon value");
                    return projection::StopReason::UNEXPECTED;
            }
        }();

        if (candidate.finishMessage) {
            warnings.emplace_back("finishMessage: " + *candidate.finishMessage);
        }
    } else {
        // No candidates at all — check promptFeedback for blocking
        if (response.promptFeedback && response.promptFeedback->blockReason) {
            stop_reason = projection::StopReason::CONTENT_FILTERED;
            warnings.emplace_back("prompt was blocked by provider (promptFeedback.blockReason present)");
        }
    }

    auto usage = projection::Usage{};
    if (response.usageMetadata) {
        auto const& u = *response.usageMetadata;
        usage.input_tokens = u.promptTokenCount.value_or(0);
        usage.output_tokens = u.candidatesTokenCount.value_or(0);
        if (u.thoughtsTokenCount) {
            usage.thinking_tokens = *u.thoughtsTokenCount;
        }
        if (u.cachedContentTokenCount) {
            usage.cache_read_tokens = *u.cachedContentTokenCount;
        }
        // Gemini does not report cache creation tokens.
    }

    return Result{
        .text=std::move(text),
        .diagnostics=projection::Diagnostics{
            .stop_reason=stop_reason,
            .refusal={}, // Gemini does not provide refusal text
            .warnings=std::move(warnings)
        },
        .usage=usage,
        .metadata=Metadata{
            .id=response.responseId.value_or(""),
            .model=response.modelVersion.value_or("")
        }
    };
}


template <>
constexpr Result Extract<openai::Response>(openai::Response const& response) {
    using OutputMessage = openai::Response::ResponseOutputMessage;

    auto text = std::string{};
    auto warnings = std::vector<std::string>{};
    auto refusal = std::optional<std::string>{};

    // Walk output items: extract text from ResponseOutputMessage blocks,
    // detect refusal, ignore all other output item types (tool calls, reasoning, etc.).
    // Note: Response::output_text is an SDK-synthesized convenience property that
    // aggregates text from output items; it is not present in the wire JSON.
    if (response.output) {
        for (auto const& item : *response.output) {
            if (auto const* msg = std::get_if<OutputMessage>(&item)) {
                if (msg->content) {
                    for (auto const& content_item : *msg->content) {
                        if (auto const* tb = std::get_if<OutputMessage::ResponseOutputText>(&content_item)) {
                            if (tb->text) {
                                if (!text.empty()) { text += '\n'; }
                                text += *tb->text;
                            }
                        } else if (auto const* ref = std::get_if<OutputMessage::ResponseOutputRefusal>(&content_item)) {
                            if (ref->refusal) {
                                refusal = *ref->refusal;
                            }
                        }
                    }
                }
            }
        }
    }

    // Map response-level status to stop reason
    auto stop_reason = [&]() -> projection::StopReason {
        if (!response.status) { return projection::StopReason::UNEXPECTED; }
        switch (*response.status) {
            case openai::Response::Status::COMPLETED:
                return refusal ? projection::StopReason::CONTENT_FILTERED
                               : projection::StopReason::COMPLETE;
            case openai::Response::Status::INCOMPLETE: {
                if (response.incomplete_details && response.incomplete_details->reason) {
                    using Reason = openai::Response::Response_incomplete_details::Reason;
                    switch (*response.incomplete_details->reason) {
                        case Reason::MAX_OUTPUT_TOKENS:
                            warnings.emplace_back("response status=INCOMPLETE: max_output_tokens");
                            return projection::StopReason::MAX_TOKENS;
                        case Reason::CONTENT_FILTER:
                            warnings.emplace_back("response status=INCOMPLETE: content_filter");
                            return projection::StopReason::CONTENT_FILTERED;
                        default:
                            break;
                    }
                }
                warnings.emplace_back("response status=INCOMPLETE: reason unknown");
                return projection::StopReason::MAX_TOKENS;
            }
            case openai::Response::Status::FAILED:
                if (response.error && response.error->message) {
                    warnings.emplace_back("error: " + *response.error->message);
                }
                return projection::StopReason::CONTENT_FILTERED;
            case openai::Response::Status::CANCELLED:
                warnings.emplace_back("response status=CANCELLED");
                return projection::StopReason::UNEXPECTED;
            case openai::Response::Status::QUEUED:
            case openai::Response::Status::IN_PROGRESS:
                warnings.emplace_back("response status indicates incomplete processing");
                return projection::StopReason::UNEXPECTED;
            default:
                warnings.emplace_back("response status: unrecognized value");
                return projection::StopReason::UNEXPECTED;
        }
    }();

    auto usage = projection::Usage{};
    if (response.usage) {
        auto const& u = *response.usage;
        usage.input_tokens = u.input_tokens ? static_cast<int64_t>(*u.input_tokens) : 0;
        usage.output_tokens = u.output_tokens ? static_cast<int64_t>(*u.output_tokens) : 0;
        if (u.output_tokens_details && u.output_tokens_details->reasoning_tokens) {
            usage.thinking_tokens = static_cast<int64_t>(*u.output_tokens_details->reasoning_tokens);
        }
        if (u.input_tokens_details && u.input_tokens_details->cached_tokens) {
            usage.cache_read_tokens = static_cast<int64_t>(*u.input_tokens_details->cached_tokens);
        }
    }

    return Result{
        .text=std::move(text),
        .diagnostics=projection::Diagnostics{
            .stop_reason=stop_reason,
            .refusal=std::move(refusal),
            .warnings=std::move(warnings)
        },
        .usage=usage,
        .metadata=Metadata{
            .id=response.id.value_or(""),
            .model=response.model ? std::visit(overloaded{
                [](std::string const& s) -> std::string { return s; },
                [](openai::Response::ResponsesModelValues const& v) -> std::string {
                    return std::string{jai::llm::to_string_view(v)};
                }
            }, *response.model) : std::string{}
        }
    };
}


// --- Tier 3: Projection Call functions ---
// Build a provider-specific request via Generate, then delegate to Tier 2
// with Extract as the transform. Endpoint is deduced from the ClientHandle.

template <typename Endpoint>
jai::llm::AsyncResult<Endpoint, Result> CallAsync(
    const jai::llm::Instance::ClientHandle<Endpoint>& client,
    std::optional<Prompt> system_prompt,
    const std::vector<Block>& content,
    Options options = default_options,
    const jai::llm::AttemptPolicy& policy = {}) {
    auto request = Generate<typename Endpoint::Request_t>(
        std::move(system_prompt), content, options);
    return jai::llm::CallAsync<Endpoint, Result>(
        client.Id(), request,
        &Extract<typename Endpoint::Response_t>, policy);
}

template <typename Endpoint>
jai::llm::CoroAsyncResult<Endpoint, Result> CallCoro(
    const jai::llm::Instance::ClientHandle<Endpoint>& client,
    std::optional<Prompt> system_prompt,
    const std::vector<Block>& content,
    Options options = default_options,
    const jai::llm::AttemptPolicy& policy = {}) {
    auto request = Generate<typename Endpoint::Request_t>(
        std::move(system_prompt), content, options);
    return jai::llm::CallCoro<Endpoint, Result>(
        client.Id(), request,
        &Extract<typename Endpoint::Response_t>, policy);
}

template <typename Endpoint>
jai::llm::Result<Endpoint, Result> CallSync(
    const jai::llm::Instance::ClientHandle<Endpoint>& client,
    std::optional<Prompt> system_prompt,
    const std::vector<Block>& content,
    Options options = default_options,
    const jai::llm::AttemptPolicy& policy = {}) {
    auto request = Generate<typename Endpoint::Request_t>(
        std::move(system_prompt), content, options);
    return jai::llm::CallSync<Endpoint, Result>(
        client.Id(), request,
        &Extract<typename Endpoint::Response_t>, policy);
}


}
