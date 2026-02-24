#pragma once

#include <optional>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../core/error.hpp"
#include "../core/types.hpp"
#include "../protocols/anthropic/messages.hpp"
#include "../protocols/anthropic/messages_strings.hpp"
#include "../protocols/gemini/generate_content.hpp"
#include "../protocols/gemini/generate_content_strings.hpp"
#include "../protocols/openai/responses.hpp"
#include "../protocols/openai/responses_strings.hpp"


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


inline void Call(std::optional<Prompt> system_prompt, const std::vector<Block>& content, Options options = default_options) {
}




template <typename T> constexpr T Generate(std::optional<Prompt> system_prompt,
                                           const std::vector<Block>& content,
                                           Options options = default_options);


template <>
constexpr anthropic::Request Generate<anthropic::Request>(std::optional<Prompt> system_prompt,
                                                          const std::vector<Block>& content,
                                                          Options options) {
    auto TransformBlock_f = [](Block const& block) {
        return anthropic::ContentBlockParam
        {
            std::visit(overloaded{
                [](projection::text::Image const& image) -> anthropic::ContentBlockParam {
                    auto media_type = jai::llm::from_string_view<anthropic::ImageMediaType>(image.media_type);
                    if (!media_type) {
                        throw AnnotatedException{
                            std::string{"Unknown anthropic::ImageMediaType provided: "} + image.media_type,
                            "while generating anthropic::Base64ImageSource for projection::text"
                        };
                    }
                    return anthropic::ImageBlockParam{
                               .source=anthropic::Base64ImageSource{
                                   .data=image.base64_content,
                                   .media_type=*media_type
                               }
                           };
                },
                [](projection::text::Prompt const& prompt) -> anthropic::ContentBlockParam {
                    return anthropic::TextBlockParam{.text=prompt.text};
                }
            }, block)
        };
    };

    std::optional<anthropic::OutputConfig> output_config{};
    std::optional<anthropic::ThinkingConfig> thinking_config{};
    if (true) { // model verssion >= 4.6
        if (options.thinking_effort) {
            switch (*options.thinking_effort) {
                case projection::text::ThinkingEffort::NONE:
                    thinking_config = anthropic::ThinkingConfigDisabled{};
                    break;
                case projection::text::ThinkingEffort::MINIMAL:
                case projection::text::ThinkingEffort::LOW:
                    thinking_config = anthropic::ThinkingConfigAdaptive{};
                    output_config = anthropic::OutputConfig{.effort=anthropic::ThinkingEffort::LOW};
                    break;
                case projection::text::ThinkingEffort::MEDIUM:
                    thinking_config = anthropic::ThinkingConfigAdaptive{};
                    output_config = anthropic::OutputConfig{.effort=anthropic::ThinkingEffort::MEDIUM};
                    break;
                case projection::text::ThinkingEffort::HIGH:
                    thinking_config = anthropic::ThinkingConfigAdaptive{};
                    output_config = anthropic::OutputConfig{.effort=anthropic::ThinkingEffort::HIGH};
                    break;
                case projection::text::ThinkingEffort::MAX:
                    thinking_config = anthropic::ThinkingConfigAdaptive{};
                    if (true) { // opus 4.6
                        output_config = anthropic::OutputConfig{.effort=anthropic::ThinkingEffort::MAX};
                    } else {
                        output_config = anthropic::OutputConfig{.effort=anthropic::ThinkingEffort::HIGH};
                    }
                    break;
                default:
                    throw AnnotatedException{
                        "Invalid projection::text::ThinkingEffort value provided."
                    };
            }
        }
    } else {
        thinking_config = anthropic::ThinkingConfigDisabled{};
    }

    std::optional<anthropic::System> system{};
    if (system_prompt) { system = system_prompt->text; }

    return anthropic::Request{
        .max_tokens=options.max_output_tokens,
        .messages=std::vector<anthropic::MessageParam>{{
            .content=content | std::views::transform(TransformBlock_f) | std::ranges::to<std::vector>(),
            .role=anthropic::Role::USER
        }},
        .model="",
        .output_config=output_config,
        .system=std::move(system),
        .temperature=options.temperature,
        .thinking=thinking_config,
        .top_k=options.top_k,
        .top_p=options.top_p
    };
}


template <>
constexpr gemini::Request Generate<gemini::Request>(std::optional<Prompt> system_prompt,
                                                    const std::vector<Block>& content,
                                                    Options options) {
    auto TransformPart_f = [](Block const& block) {
        return gemini::RequestContent::RequestPart
        {
            .data=std::visit(overloaded{
                [](projection::text::Image const& image) -> gemini::RequestContent::RequestPart::Data {
                    auto media_type = jai::llm::from_string_view<gemini::MediaType>(image.media_type);
                    if (!media_type) {
                        throw AnnotatedException{
                            std::string{"Unknown gemini::MediaType provided: "} + image.media_type,
                            "while generating gemini::Request for projection::text"
                        };
                    }
                    return gemini::Blob{.mimeType=*media_type, .data=image.base64_content};
                },
                [](projection::text::Prompt const& prompt) -> gemini::RequestContent::RequestPart::Data {
                    return gemini::Text{.text=prompt.text};
                }
            }, block)
        };
    };

    auto TransformContent_f = [&TransformPart_f](Block const& block) {
        return gemini::RequestContent{
            .parts=std::vector{TransformPart_f(block)},
            .role=gemini::Role::USER
        };
    };

    std::optional<gemini::ThinkingConfig> thinking_config{};
    if (true) { // model verssion >= 3
        if (options.thinking_effort) {
            thinking_config = gemini::ThinkingConfig{.includeThoughts=false};
            switch (*options.thinking_effort) {
                case projection::text::ThinkingEffort::NONE:
                case projection::text::ThinkingEffort::MINIMAL:
                    thinking_config->thinkingLevel = gemini::ThinkingLevel::MINIMAL;
                    break;
                case projection::text::ThinkingEffort::LOW:
                    thinking_config->thinkingLevel = gemini::ThinkingLevel::LOW;
                    break;
                case projection::text::ThinkingEffort::MEDIUM:
                    thinking_config->thinkingLevel = gemini::ThinkingLevel::MEDIUM;
                    break;
                case projection::text::ThinkingEffort::HIGH:
                case projection::text::ThinkingEffort::MAX:
                    thinking_config->thinkingLevel = gemini::ThinkingLevel::HIGH;
                    break;
                default:
                    throw AnnotatedException{
                        "Invalid projection::text::ThinkingEffort value provided."
                    };
            }
        }
    } else {
        thinking_config = gemini::ThinkingConfig{.thinkingBudget=0, .includeThoughts=false};
    }

    return gemini::Request{
        .contents=content | std::views::transform(TransformContent_f) | std::ranges::to<std::vector>(),
        .systemInstruction=system_prompt ?
                                std::optional{
                                    gemini::RequestContent{
                                        .parts=std::vector{TransformPart_f(*system_prompt)},
                                        .role=gemini::Role::SYSTEM
                                    }
                                } : std::nullopt,
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
    auto TransformContent_f = [](Block const& block) -> openai::request::InputTypes::MessageContentUnit {
        return std::visit(overloaded{
            [](projection::text::Image const& image) -> openai::request::InputTypes::MessageContentUnit {
                return openai::request::ContentTypes::Image{
                    .detail=openai::Detail::AUTO,
                    .image_url=jai::llm::EncodedUrl{
                        "data:" + image.media_type + ";base64," + image.base64_content
                    }
                };
            },
            [](projection::text::Prompt const& prompt) -> openai::request::InputTypes::MessageContentUnit {
                return openai::request::ContentTypes::Text{
                    .text=prompt.text
                };
            }
        }, block);
    };

    std::vector<openai::request::InputItemList> input_items;

    if (system_prompt) {
        input_items.emplace_back(openai::request::InputTypes::Message{
            .content=system_prompt->text,
            .role=openai::RoleInputMessage::SYSTEM
        });
    }

    input_items.emplace_back(openai::request::InputTypes::Message{
        .content=content | std::views::transform(TransformContent_f) | std::ranges::to<std::vector>(),
        .role=openai::RoleInputMessage::USER
    });

    std::optional<openai::Reasoning> reasoning{};
    if (options.thinking_effort) {
        openai::ReasoningEffort effort;
        switch (*options.thinking_effort) {
            case projection::text::ThinkingEffort::NONE:    effort = openai::ReasoningEffort::NONE; break;
            case projection::text::ThinkingEffort::MINIMAL: effort = openai::ReasoningEffort::MINIMAL; break;
            case projection::text::ThinkingEffort::LOW:     effort = openai::ReasoningEffort::LOW; break;
            case projection::text::ThinkingEffort::MEDIUM:  effort = openai::ReasoningEffort::MEDIUM; break;
            case projection::text::ThinkingEffort::HIGH:    effort = openai::ReasoningEffort::HIGH; break;
            case projection::text::ThinkingEffort::MAX:     effort = openai::ReasoningEffort::XHIGH; break;
        }
        reasoning = openai::Reasoning{
            .effort=effort,
            .summary=openai::ReasoningSummary::AUTO
        };
    }

    return openai::Request{
        .input=std::move(input_items),
        .max_output_tokens=options.max_output_tokens,
        .model="", // Should be set by caller or via a default logic
        .reasoning=reasoning,
        .temperature=options.temperature,
        .top_p=options.top_p
    };
}






}
