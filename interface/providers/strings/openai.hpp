#pragma once

#include <stdexcept>
#include <string_view>
#include <optional>
#include "../openai.hpp"


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


/***
 * from_string_view Specializations
 */

template <>
constexpr std::optional<openai::Role> from_string_view<openai::Role>(std::string_view sv) {
    if (sv == "user") return openai::Role::user;
    if (sv == "assistant") return openai::Role::assistant;
    if (sv == "system") return openai::Role::system;
    if (sv == "developer") return openai::Role::developer;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ItemStatus> from_string_view<openai::ItemStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::ItemStatus::in_progress;
    if (sv == "completed") return openai::ItemStatus::completed;
    if (sv == "incomplete") return openai::ItemStatus::incomplete;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::Detail> from_string_view<openai::Detail>(std::string_view sv) {
    if (sv == "high") return openai::Detail::high;
    if (sv == "low") return openai::Detail::low;
    if (sv == "auto") return openai::Detail::auto_detail;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::MouseButton> from_string_view<openai::MouseButton>(std::string_view sv) {
    if (sv == "left") return openai::MouseButton::left;
    if (sv == "right") return openai::MouseButton::right;
    if (sv == "wheel") return openai::MouseButton::wheel;
    if (sv == "back") return openai::MouseButton::back;
    if (sv == "forward") return openai::MouseButton::forward;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FileSearchStatus> from_string_view<openai::FileSearchStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::FileSearchStatus::in_progress;
    if (sv == "searching") return openai::FileSearchStatus::searching;
    if (sv == "incomplete") return openai::FileSearchStatus::incomplete;
    if (sv == "failed") return openai::FileSearchStatus::failed;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::WebSearchActionType> from_string_view<openai::WebSearchActionType>(std::string_view sv) {
    if (sv == "search") return openai::WebSearchActionType::search;
    if (sv == "open_page") return openai::WebSearchActionType::open_page;
    if (sv == "find") return openai::WebSearchActionType::find;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::CodeInterpreterStatus> from_string_view<openai::CodeInterpreterStatus>(std::string_view sv) {
    if (sv == "in_progress") return openai::CodeInterpreterStatus::in_progress;
    if (sv == "completed") return openai::CodeInterpreterStatus::completed;
    if (sv == "incomplete") return openai::CodeInterpreterStatus::incomplete;
    if (sv == "interpreting") return openai::CodeInterpreterStatus::interpreting;
    if (sv == "failed") return openai::CodeInterpreterStatus::failed;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ServiceTier> from_string_view<openai::ServiceTier>(std::string_view sv) {
    if (sv == "auto") return openai::ServiceTier::auto_tier;
    if (sv == "default") return openai::ServiceTier::default_tier;
    if (sv == "flex") return openai::ServiceTier::flex;
    if (sv == "scale") return openai::ServiceTier::scale;
    if (sv == "priority") return openai::ServiceTier::priority;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::TruncationStrategy> from_string_view<openai::TruncationStrategy>(std::string_view sv) {
    if (sv == "auto") return openai::TruncationStrategy::auto_truncation;
    if (sv == "disabled") return openai::TruncationStrategy::disabled;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningEffort> from_string_view<openai::ReasoningEffort>(std::string_view sv) {
    if (sv == "none") return openai::ReasoningEffort::none;
    if (sv == "minimal") return openai::ReasoningEffort::minimal;
    if (sv == "low") return openai::ReasoningEffort::low;
    if (sv == "medium") return openai::ReasoningEffort::medium;
    if (sv == "high") return openai::ReasoningEffort::high;
    if (sv == "xhigh") return openai::ReasoningEffort::xhigh;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ReasoningSummary> from_string_view<openai::ReasoningSummary>(std::string_view sv) {
    if (sv == "auto") return openai::ReasoningSummary::auto_summary;
    if (sv == "concise") return openai::ReasoningSummary::concise;
    if (sv == "detailed") return openai::ReasoningSummary::detailed;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::Verbosity> from_string_view<openai::Verbosity>(std::string_view sv) {
    if (sv == "low") return openai::Verbosity::low;
    if (sv == "medium") return openai::Verbosity::medium;
    if (sv == "high") return openai::Verbosity::high;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::FilterOperator> from_string_view<openai::FilterOperator>(std::string_view sv) {
    if (sv == "eq") return openai::FilterOperator::eq;
    if (sv == "ne") return openai::FilterOperator::ne;
    if (sv == "gt") return openai::FilterOperator::gt;
    if (sv == "gte") return openai::FilterOperator::gte;
    if (sv == "lt") return openai::FilterOperator::lt;
    if (sv == "lte") return openai::FilterOperator::lte;
    if (sv == "in") return openai::FilterOperator::in;
    if (sv == "nin") return openai::FilterOperator::nin;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::SearchContextSize> from_string_view<openai::SearchContextSize>(std::string_view sv) {
    if (sv == "low") return openai::SearchContextSize::low;
    if (sv == "medium") return openai::SearchContextSize::medium;
    if (sv == "high") return openai::SearchContextSize::high;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationBackground> from_string_view<openai::ImageGenerationBackground>(std::string_view sv) {
    if (sv == "transparent") return openai::ImageGenerationBackground::transparent;
    if (sv == "opaque") return openai::ImageGenerationBackground::opaque;
    if (sv == "auto") return openai::ImageGenerationBackground::auto_background;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationFidelity> from_string_view<openai::ImageGenerationFidelity>(std::string_view sv) {
    if (sv == "high") return openai::ImageGenerationFidelity::high;
    if (sv == "low") return openai::ImageGenerationFidelity::low;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationFormat> from_string_view<openai::ImageGenerationFormat>(std::string_view sv) {
    if (sv == "png") return openai::ImageGenerationFormat::png;
    if (sv == "webp") return openai::ImageGenerationFormat::webp;
    if (sv == "jpeg") return openai::ImageGenerationFormat::jpeg;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationQuality> from_string_view<openai::ImageGenerationQuality>(std::string_view sv) {
    if (sv == "low") return openai::ImageGenerationQuality::low;
    if (sv == "medium") return openai::ImageGenerationQuality::medium;
    if (sv == "high") return openai::ImageGenerationQuality::high;
    if (sv == "auto") return openai::ImageGenerationQuality::auto_quality;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ImageGenerationSize> from_string_view<openai::ImageGenerationSize>(std::string_view sv) {
    if (sv == "1024x1024") return openai::ImageGenerationSize::size_1024_1024;
    if (sv == "1024x1536") return openai::ImageGenerationSize::size_1024_1536;
    if (sv == "1536x1024") return openai::ImageGenerationSize::size_1536_1024;
    if (sv == "auto") return openai::ImageGenerationSize::auto_size;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::GrammarSyntax> from_string_view<openai::GrammarSyntax>(std::string_view sv) {
    if (sv == "lark") return openai::GrammarSyntax::lark;
    if (sv == "regex") return openai::GrammarSyntax::regex;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::ResponseStatus> from_string_view<openai::ResponseStatus>(std::string_view sv) {
    if (sv == "completed") return openai::ResponseStatus::completed;
    if (sv == "failed") return openai::ResponseStatus::failed;
    if (sv == "in_progress") return openai::ResponseStatus::in_progress;
    if (sv == "cancelled") return openai::ResponseStatus::cancelled;
    if (sv == "queued") return openai::ResponseStatus::queued;
    if (sv == "incomplete") return openai::ResponseStatus::incomplete;
    return std::nullopt;
}

template <>
constexpr std::optional<openai::IncompleteReason> from_string_view<openai::IncompleteReason>(std::string_view sv) {
    if (sv == "max_output_tokens") return openai::IncompleteReason::max_output_tokens;
    if (sv == "content_filter") return openai::IncompleteReason::content_filter;
    return std::nullopt;
}


/***
 * to_string_view Overloads
 */

constexpr std::string_view to_string_view(openai::Role val) {
    switch (val) {
        case openai::Role::user: return "user";
        case openai::Role::assistant: return "assistant";
        case openai::Role::system: return "system";
        case openai::Role::developer: return "developer";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ItemStatus val) {
    switch (val) {
        case openai::ItemStatus::in_progress: return "in_progress";
        case openai::ItemStatus::completed: return "completed";
        case openai::ItemStatus::incomplete: return "incomplete";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::Detail val) {
    switch (val) {
        case openai::Detail::high: return "high";
        case openai::Detail::low: return "low";
        case openai::Detail::auto_detail: return "auto";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::MouseButton val) {
    switch (val) {
        case openai::MouseButton::left: return "left";
        case openai::MouseButton::right: return "right";
        case openai::MouseButton::wheel: return "wheel";
        case openai::MouseButton::back: return "back";
        case openai::MouseButton::forward: return "forward";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::FileSearchStatus val) {
    switch (val) {
        case openai::FileSearchStatus::in_progress: return "in_progress";
        case openai::FileSearchStatus::searching: return "searching";
        case openai::FileSearchStatus::incomplete: return "incomplete";
        case openai::FileSearchStatus::failed: return "failed";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::WebSearchActionType val) {
    switch (val) {
        case openai::WebSearchActionType::search: return "search";
        case openai::WebSearchActionType::open_page: return "open_page";
        case openai::WebSearchActionType::find: return "find";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::CodeInterpreterStatus val) {
    switch (val) {
        case openai::CodeInterpreterStatus::in_progress: return "in_progress";
        case openai::CodeInterpreterStatus::completed: return "completed";
        case openai::CodeInterpreterStatus::incomplete: return "incomplete";
        case openai::CodeInterpreterStatus::interpreting: return "interpreting";
        case openai::CodeInterpreterStatus::failed: return "failed";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ServiceTier val) {
    switch (val) {
        case openai::ServiceTier::auto_tier: return "auto";
        case openai::ServiceTier::default_tier: return "default";
        case openai::ServiceTier::flex: return "flex";
        case openai::ServiceTier::scale: return "scale";
        case openai::ServiceTier::priority: return "priority";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::TruncationStrategy val) {
    switch (val) {
        case openai::TruncationStrategy::auto_truncation: return "auto";
        case openai::TruncationStrategy::disabled: return "disabled";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ReasoningEffort val) {
    switch (val) {
        case openai::ReasoningEffort::none: return "none";
        case openai::ReasoningEffort::minimal: return "minimal";
        case openai::ReasoningEffort::low: return "low";
        case openai::ReasoningEffort::medium: return "medium";
        case openai::ReasoningEffort::high: return "high";
        case openai::ReasoningEffort::xhigh: return "xhigh";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ReasoningSummary val) {
    switch (val) {
        case openai::ReasoningSummary::auto_summary: return "auto";
        case openai::ReasoningSummary::concise: return "concise";
        case openai::ReasoningSummary::detailed: return "detailed";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::Verbosity val) {
    switch (val) {
        case openai::Verbosity::low: return "low";
        case openai::Verbosity::medium: return "medium";
        case openai::Verbosity::high: return "high";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::FilterOperator val) {
    switch (val) {
        case openai::FilterOperator::eq: return "eq";
        case openai::FilterOperator::ne: return "ne";
        case openai::FilterOperator::gt: return "gt";
        case openai::FilterOperator::gte: return "gte";
        case openai::FilterOperator::lt: return "lt";
        case openai::FilterOperator::lte: return "lte";
        case openai::FilterOperator::in: return "in";
        case openai::FilterOperator::nin: return "nin";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::SearchContextSize val) {
    switch (val) {
        case openai::SearchContextSize::low: return "low";
        case openai::SearchContextSize::medium: return "medium";
        case openai::SearchContextSize::high: return "high";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ImageGenerationBackground val) {
    switch (val) {
        case openai::ImageGenerationBackground::transparent: return "transparent";
        case openai::ImageGenerationBackground::opaque: return "opaque";
        case openai::ImageGenerationBackground::auto_background: return "auto";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ImageGenerationFidelity val) {
    switch (val) {
        case openai::ImageGenerationFidelity::high: return "high";
        case openai::ImageGenerationFidelity::low: return "low";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ImageGenerationFormat val) {
    switch (val) {
        case openai::ImageGenerationFormat::png: return "png";
        case openai::ImageGenerationFormat::webp: return "webp";
        case openai::ImageGenerationFormat::jpeg: return "jpeg";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ImageGenerationQuality val) {
    switch (val) {
        case openai::ImageGenerationQuality::low: return "low";
        case openai::ImageGenerationQuality::medium: return "medium";
        case openai::ImageGenerationQuality::high: return "high";
        case openai::ImageGenerationQuality::auto_quality: return "auto";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ImageGenerationSize val) {
    switch (val) {
        case openai::ImageGenerationSize::size_1024_1024: return "1024x1024";
        case openai::ImageGenerationSize::size_1024_1536: return "1024x1536";
        case openai::ImageGenerationSize::size_1536_1024: return "1536x1024";
        case openai::ImageGenerationSize::auto_size: return "auto";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::GrammarSyntax val) {
    switch (val) {
        case openai::GrammarSyntax::lark: return "lark";
        case openai::GrammarSyntax::regex: return "regex";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::ResponseStatus val) {
    switch (val) {
        case openai::ResponseStatus::completed: return "completed";
        case openai::ResponseStatus::failed: return "failed";
        case openai::ResponseStatus::in_progress: return "in_progress";
        case openai::ResponseStatus::cancelled: return "cancelled";
        case openai::ResponseStatus::queued: return "queued";
        case openai::ResponseStatus::incomplete: return "incomplete";
    }
    return "unknown";
}

constexpr std::string_view to_string_view(openai::IncompleteReason val) {
    switch (val) {
        case openai::IncompleteReason::max_output_tokens: return "max_output_tokens";
        case openai::IncompleteReason::content_filter: return "content_filter";
    }
    return "unknown";
}


}
