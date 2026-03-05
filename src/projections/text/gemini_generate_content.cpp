// proj::text Generate/Extract specializations for Gemini GenerateContent endpoint.

#include <optional>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

#include "../../../interface/core/error.hpp"
#include "../../../interface/core/types.hpp"
#include "../../../interface/endpoints/gemini_generate_content.hpp"
#include "../../../interface/projections/text.hpp"


namespace jai::llm::proj::text {


template <>
gemini::GenerateContent::Request_t
Generate<gemini::GenerateContent>(std::optional<Prompt> system_prompt,
                                  const std::vector<Block>& content,
                                  Options options)
{
    using Request = gemini::GenerateContent::Request_t;

    auto TransformPart_f = [](Block const& block) -> gemini::Part {
        return std::visit(overloaded{
            [](proj::text::Image const& image) -> gemini::Part {
                return gemini::Part{
                    .inlineData=gemini::Blob{
                        .mimeType=image.media_type,
                        .data=image.base64_content
                    }
                };
            },
            [](proj::text::Prompt const& prompt) -> gemini::Part {
                return gemini::Part{.text=prompt.text};
            }
        }, block);
    };

    auto thinking_config = std::optional<gemini::ThinkingConfig>{};
    if (true) { // model version >= 3
        if (options.thinking_effort) {
            auto level = [&]() -> gemini::ThinkingLevel {
                switch (*options.thinking_effort) {
                    case proj::text::ThinkingEffort::NONE:
                    case proj::text::ThinkingEffort::MINIMAL: return gemini::ThinkingLevel::MINIMAL;
                    case proj::text::ThinkingEffort::LOW:     return gemini::ThinkingLevel::LOW;
                    case proj::text::ThinkingEffort::MEDIUM:  return gemini::ThinkingLevel::MEDIUM;
                    case proj::text::ThinkingEffort::HIGH:
                    case proj::text::ThinkingEffort::MAX:     return gemini::ThinkingLevel::HIGH;
                    default:
                        throw AnnotatedException{
                            "Invalid proj::text::ThinkingEffort value provided."
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

    return Request{
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
Result Extract<gemini::GenerateContent>(gemini::GenerateContent::Response_t const& response) {
    auto text = std::string{};

    if (response.candidates && !response.candidates->empty()) {
        auto const& candidate = response.candidates->front();

        if (candidate.content && candidate.content->parts) {
            for (auto const& part : *candidate.content->parts) {
                if (part.text) {
                    if (!text.empty()) { text += '\n'; }
                    text += *part.text;
                }
                // Non-text parts (inlineData, functionCall, etc.) are silently ignored.
            }
        }
    }

    return Result{.text=std::move(text)};
}


}
