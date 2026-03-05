// proj::text Generate/Extract specializations for Gemini GenerateContent endpoint.

#include <charconv>
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


// ----- ParseModelInfo -----
// Gemini model strings: "gemini-{version}-{variant}[-suffix]"
// e.g. "gemini-2.5-pro-preview-05-06", "gemini-3-flash"

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
} // anonymous namespace

template <>
ModelInfo ParseModelInfo<gemini::GenerateContent>(std::string_view model) {
    // Extract version: parse number after "gemini-"
    auto version = 0.0;
    constexpr auto prefix = std::string_view{"gemini-"};
    if (model.starts_with(prefix)) {
        version = ParseVersion(model.substr(prefix.size()));
    }

    // Family: strip suffixes (-preview, -exp, trailing date/point-release)
    // to get something like "gemini-2.5-pro", "gemini-3-flash".
    auto family = std::string{model};
    if (auto pos = family.find("-preview"); pos != std::string::npos) {
        family.erase(pos);
    } else if (auto pos2 = family.find("-exp"); pos2 != std::string::npos) {
        family.erase(pos2);
    }

    return ModelInfo{
        .model=std::string{model},
        .family=std::move(family),
        .version=version
    };
}


// ----- Generate -----

template <>
gemini::GenerateContent::Request_t
Generate<gemini::GenerateContent>(const ModelInfo& model_info,
                                  std::optional<Prompt> system_prompt,
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
    if (model_info.version >= 3.0) {
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
        .model=model_info.model,
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
