#pragma once

#include <exception>
#include <format>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace jai::llm {


namespace _detail {
    inline std::string NormalizeFileName(std::string_view);
    inline std::string_view NormalizeFunctionName(std::string_view);
}


class AnnotatedException : public std::exception {
public:
    struct Context {
        std::string msg{};
        std::source_location source{};

        bool operator==(const Context& other) const noexcept {
            return msg                    == other.msg &&
                   source.file_name()     == other.source.file_name() &&
                   source.function_name() == other.source.function_name() &&
                   source.line()          == other.source.line() &&
                   source.column()        == other.source.column();
        }

        auto operator<=>(const Context& other) const noexcept {
            if (auto c = msg                    <=> other.msg;                    c != 0) { return c; }
            if (auto c = source.file_name()     <=> other.source.file_name();     c != 0) { return c; }
            if (auto c = source.function_name() <=> other.source.function_name(); c != 0) { return c; }
            if (auto c = source.line()          <=> other.source.line();          c != 0) { return c; }
            return       source.column()        <=> other.source.column();
        }
    };
    using ContextView = std::span<const Context>;

private:
    std::string error_msg{};
    std::vector<Context> context{};

public:
    AnnotatedException() = delete;
    explicit AnnotatedException(std::string msg_,
                                std::source_location source_ = std::source_location::current())
    : AnnotatedException(std::move(msg_), std::string{}, source_)
    {}
    explicit AnnotatedException(std::string msg_,
                                std::string context_msg_,
                                std::source_location source_ = std::source_location::current())
    : error_msg{std::move(msg_)}, context{{.msg=context_msg_, .source=source_}}
    {}
    AnnotatedException(const AnnotatedException&) = default;
    AnnotatedException(AnnotatedException&&) noexcept = default;
    ~AnnotatedException() noexcept = default;
    AnnotatedException& operator=(const AnnotatedException&) = default;
    AnnotatedException& operator=(AnnotatedException&&) noexcept = default;

    bool operator==(const AnnotatedException& other) const noexcept {
        return error_msg == other.error_msg && context == other.context;
    }

    auto operator<=>(const AnnotatedException& other) const noexcept {
        if (auto c = error_msg <=> other.error_msg; c != 0) { return c; }
        return context <=> other.context;
    }

    const char* what() const noexcept override { return error_msg.c_str(); }
    std::string_view ErrorMsg() const { return error_msg; }
    ContextView ErrorContext() const { return context; }

    void AddContext(std::optional<std::string> context_msg_ = {},
                    std::source_location source_ = std::source_location::current()) {
        context.push_back({.msg=context_msg_.value_or(std::string{}), .source=source_});
    }
};


inline std::string to_string(const AnnotatedException::Context& ctx) {
    std::string prefix{};
    if (!ctx.msg.empty()) {
        prefix = ctx.msg;
        prefix += "\n";
    }
    return std::format("{}    [{} ({}, {}) - {}]\n",
        prefix,
        _detail::NormalizeFileName(ctx.source.file_name()),
        ctx.source.line(),
        ctx.source.column(),
        _detail::NormalizeFunctionName(ctx.source.function_name()));
}


template <std::ranges::range R>
requires std::convertible_to<std::ranges::range_reference_t<R>, const AnnotatedException::Context&>
inline std::string to_string(R&& rg) {
    return rg | std::views::transform([](const AnnotatedException::Context& ctx) {
                    return jai::llm::to_string(ctx);
                }) |
                std::views::join_with(std::string_view{"\n"}) |
                std::ranges::to<std::string>();
}


inline std::string to_string(const AnnotatedException& e) {
    return std::format("Exception-\nReason: {}\n\nContext-\n{}", e.ErrorMsg(), jai::llm::to_string(e.ErrorContext()));
}


namespace _detail {
    inline std::string NormalizeFileName(std::string_view str_sv) {
        static constexpr std::string_view rev_path{"/mll/"};
        auto r = str_sv |
                std::views::transform([](auto ch) { return ch == '\\' ? '/' : ch; }) |
                std::views::reverse;
        auto match = std::ranges::search(r, rev_path);
        if (match.begin() == match.end()) { return {}; }
        return std::ranges::subrange{r.begin(), match.begin()} |
            std::views:: reverse |
            std::ranges::to<std::string>();
    }

    inline std::string_view NormalizeFunctionName(std::string_view str_sv) {
        // Cut templates / parameters
        if (auto cut = str_sv.find_first_of("(<"); cut != std::string_view::npos) {
            str_sv = str_sv.substr(0, cut);
        }

        // Trim trailing whitespace
        while (!str_sv.empty() && str_sv.back() == ' ') {
            str_sv.remove_suffix(1);
        }

        // Keep last whitespace-separated token
        if (auto pos = str_sv.find_last_of(' '); pos != std::string_view::npos) {
            str_sv = str_sv.substr(pos + 1);
        }

        return str_sv;
    }
}


}
