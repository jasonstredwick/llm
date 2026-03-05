// proj::text Generate/Extract specializations for Anthropic Messages endpoint.

#include <optional>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../../../interface/core/error.hpp"
#include "../../../interface/core/types.hpp"
#include "../../../interface/endpoints/anthropic_messages.hpp"
#include "../../../interface/projections/text.hpp"
#include "../../../interface/protocols/anthropic/messages_strings.hpp"


namespace jai::llm::proj::text {


template <>
anthropic::Messages::Request_t
Generate<anthropic::Messages>(std::optional<Prompt> system_prompt,
                              const std::vector<Block>& content,
                              Options options)
{
    using Request = anthropic::Messages::Request_t;
    using MessageParam = Request::MessageParam;

    auto TransformBlock_f = [](Block const& block) -> MessageParam::ContentItem {
        return std::visit(overloaded{
            [](proj::text::Image const& image) -> MessageParam::ContentItem {
                using MediaType = MessageParam::ImageBlockParam::Base64ImageSource::MediaType;
                auto media_type = jai::llm::from_string_view<MediaType>(image.media_type);
                if (!media_type) {
                    throw AnnotatedException{
                        std::string{"Unknown anthropic image MediaType provided: "} + image.media_type,
                        "while generating anthropic::Request for proj::text"
                    };
                }
                return MessageParam::ImageBlockParam{
                    .source=MessageParam::ImageBlockParam::Base64ImageSource{
                        .data=image.base64_content,
                        .media_type=*media_type
                    }
                };
            },
            [](proj::text::Prompt const& prompt) -> MessageParam::ContentItem {
                return MessageParam::TextBlockParam{.text=prompt.text};
            }
        }, block);
    };

    auto output_config = std::optional<Request::OutputConfig>{};
    auto thinking_config = std::optional<Request::ThinkingConfigParam>{};
    if (true) { // model version >= 4.6
        if (options.thinking_effort) {
            using Effort = Request::OutputConfig::Effort;
            switch (*options.thinking_effort) {
                case proj::text::ThinkingEffort::NONE:
                    thinking_config = Request::ThinkingConfigDisabled{};
                    break;
                case proj::text::ThinkingEffort::MINIMAL:
                case proj::text::ThinkingEffort::LOW:
                    thinking_config = Request::ThinkingConfigAdaptive{};
                    output_config = Request::OutputConfig{.effort=Effort::LOW};
                    break;
                case proj::text::ThinkingEffort::MEDIUM:
                    thinking_config = Request::ThinkingConfigAdaptive{};
                    output_config = Request::OutputConfig{.effort=Effort::MEDIUM};
                    break;
                case proj::text::ThinkingEffort::HIGH:
                    thinking_config = Request::ThinkingConfigAdaptive{};
                    output_config = Request::OutputConfig{.effort=Effort::HIGH};
                    break;
                case proj::text::ThinkingEffort::MAX:
                    thinking_config = Request::ThinkingConfigAdaptive{};
                    if (true) { // opus 4.6
                        output_config = Request::OutputConfig{.effort=Effort::MAX};
                    } else {
                        output_config = Request::OutputConfig{.effort=Effort::HIGH};
                    }
                    break;
                default:
                    throw AnnotatedException{
                        "Invalid proj::text::ThinkingEffort value provided."
                    };
            }
        }
    } else {
        thinking_config = Request::ThinkingConfigDisabled{};
    }

    auto system = std::optional<Request::System>{};
    if (system_prompt) {
        system = system_prompt->text;
    }

    return Request{
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
Result Extract<anthropic::Messages>(anthropic::Messages::Response_t const& response) {
    using Response = anthropic::Messages::Response_t;

    auto text = std::string{};

    if (response.content) {
        for (auto const& block : *response.content) {
            if (auto const* tb = std::get_if<Response::TextBlock>(&block)) {
                if (tb->text) {
                    if (!text.empty()) { text += '\n'; }
                    text += *tb->text;
                }
            }
            // All non-text blocks (ThinkingBlock, ToolUseBlock, etc.) are silently ignored.
            // This is the text projection — we only extract text.
        }
    }

    return Result{.text=std::move(text)};
}


}
