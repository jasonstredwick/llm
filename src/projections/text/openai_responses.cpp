// proj::text Generate/Extract specializations for OpenAI Responses endpoint.

#include <optional>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../../../interface/core/error.hpp"
#include "../../../interface/core/types.hpp"
#include "../../../interface/endpoints/openai_responses.hpp"
#include "../../../interface/projections/text.hpp"


namespace jai::llm::proj::text {


template <>
openai::Responses::Request_t
Generate<openai::Responses>(std::optional<Prompt> system_prompt,
                            const std::vector<Block>& content,
                            Options options)
{
    using Request = openai::Responses::Request_t;
    using EasyInputMessage = Request::EasyInputMessage;

    auto TransformContent_f = [](Block const& block) -> EasyInputMessage::ContentItem {
        return std::visit(overloaded{
            [](proj::text::Image const& image) -> EasyInputMessage::ContentItem {
                return EasyInputMessage::ResponseInputImage{
                    .detail=EasyInputMessage::ResponseInputImage::Detail::AUTO,
                    .image_url="data:" + image.media_type + ";base64," + image.base64_content
                };
            },
            [](proj::text::Prompt const& prompt) -> EasyInputMessage::ContentItem {
                return EasyInputMessage::ResponseInputText{.text=prompt.text};
            }
        }, block);
    };

    auto input_items = std::vector<Request::InputItem>{};

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

    auto reasoning = std::optional<Request::Reasoning>{};
    if (true) { // model supports reasoning (o-series, gpt-5+)
        if (options.thinking_effort) {
            using Effort = Request::Reasoning::Effort;
            auto effort = [&]() -> Effort {
                switch (*options.thinking_effort) {
                    case proj::text::ThinkingEffort::NONE:
                        if (true) { return Effort::NONE; }    // gpt-5.1+
                        else      { return Effort::LOW; }
                    case proj::text::ThinkingEffort::MINIMAL: return Effort::MINIMAL;
                    case proj::text::ThinkingEffort::LOW:     return Effort::LOW;
                    case proj::text::ThinkingEffort::MEDIUM:  return Effort::MEDIUM;
                    case proj::text::ThinkingEffort::HIGH:    return Effort::HIGH;
                    case proj::text::ThinkingEffort::MAX:
                        if (true) { return Effort::XHIGH; }   // gpt-5.1-codex-max+
                        else      { return Effort::HIGH; }
                    default:
                        throw AnnotatedException{
                            "Invalid proj::text::ThinkingEffort value provided."
                        };
                }
            }();
            reasoning = Request::Reasoning{
                .effort=effort,
                .summary=Request::Reasoning::Summary::AUTO
            };
        }
    }

    return Request{
        .input=std::move(input_items),
        .max_output_tokens=options.max_output_tokens ?
            std::optional{static_cast<double>(*options.max_output_tokens)} : std::nullopt,
        .reasoning=reasoning,
        .temperature=options.temperature,
        .top_p=options.top_p
    };
}


template <>
Result Extract<openai::Responses>(openai::Responses::Response_t const& response) {
    using Response = openai::Responses::Response_t;
    using OutputMessage = Response::ResponseOutputMessage;

    auto text = std::string{};

    // Walk output items: extract text from ResponseOutputMessage blocks,
    // ignore all other output item types (tool calls, reasoning, refusal, etc.).
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
                        }
                    }
                }
            }
        }
    }

    return Result{.text=std::move(text)};
}


}
