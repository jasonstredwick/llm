// proj::text Generate/Extract specializations for OpenAI Responses endpoint.

#include <charconv>
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


// ----- ParseModelInfo -----
// OpenAI model strings: "gpt-{version}[-variant][-date]" or "o{version}[-date]"
// e.g. "gpt-5-2025-05-14", "o3-2025-04-16", "gpt-5.1-codex-max-2025-06-01"

namespace {
double ParseVersion(std::string_view sv) {
    int64_t major = 0;
    auto [p1, ec1] = std::from_chars(sv.data(), sv.data() + sv.size(), major);
    if (ec1 != std::errc{}) return 0.0;
    if (p1 < sv.data() + sv.size() && *p1 == '.') {
        int64_t minor = 0;
        auto [p2, ec2] = std::from_chars(p1 + 1, sv.data() + sv.size(), minor);
        if (ec2 == std::errc{}) {
            int digits = static_cast<int>(p2 - (p1 + 1));
            double divisor = 1.0;
            for (int i = 0; i < digits; ++i) divisor *= 10.0;
            return static_cast<double>(major) + static_cast<double>(minor) / divisor;
        }
    }
    return static_cast<double>(major);
}

// Strip trailing -YYYY-MM-DD date suffix from a family string.
void StripDateSuffix(std::string& family) {
    if (family.size() > 11) {
        auto tail = std::string_view{family}.substr(family.size() - 11);
        if (tail[0] == '-' && tail[5] == '-' && tail[8] == '-') {
            bool all_digits = true;
            for (size_t i : {1, 2, 3, 4, 6, 7, 9, 10}) {
                if (tail[i] < '0' || tail[i] > '9') { all_digits = false; break; }
            }
            if (all_digits) {
                family.erase(family.size() - 11);
            }
        }
    }
}
} // anonymous namespace

template <>
ModelInfo ParseModelInfo<openai::Responses>(std::string_view model) {
    auto version = 0.0;
    auto family = std::string{};

    if (model.starts_with("gpt-")) {
        version = ParseVersion(model.substr(4));
        family = std::string{model};
        StripDateSuffix(family);
    } else if (model.starts_with("o")) {
        version = ParseVersion(model.substr(1));
        family = std::string{model};
        StripDateSuffix(family);
    }

    return ModelInfo{
        .model=std::string{model},
        .family=std::move(family),
        .version=version
    };
}


// ----- Generate -----

template <>
openai::Responses::Request_t
Generate<openai::Responses>(const ModelInfo& model_info,
                            std::optional<Prompt> system_prompt,
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

    // Reasoning: o-series models always support it; gpt-5+ supports it.
    auto supports_reasoning = model_info.family.starts_with("o") || model_info.version >= 5.0;

    auto reasoning = std::optional<Request::Reasoning>{};
    if (supports_reasoning) {
        if (options.thinking_effort) {
            using Effort = Request::Reasoning::Effort;
            auto effort = [&]() -> Effort {
                switch (*options.thinking_effort) {
                    case proj::text::ThinkingEffort::NONE:
                        if (model_info.version >= 5.1) { return Effort::NONE; }
                        else                           { return Effort::LOW; }
                    case proj::text::ThinkingEffort::MINIMAL: return Effort::MINIMAL;
                    case proj::text::ThinkingEffort::LOW:     return Effort::LOW;
                    case proj::text::ThinkingEffort::MEDIUM:  return Effort::MEDIUM;
                    case proj::text::ThinkingEffort::HIGH:    return Effort::HIGH;
                    case proj::text::ThinkingEffort::MAX:
                        if (model_info.family.find("codex-max") != std::string::npos) { return Effort::XHIGH; }
                        else                                                          { return Effort::HIGH; }
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
        .model=model_info.model,
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
