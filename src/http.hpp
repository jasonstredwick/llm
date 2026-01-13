#pragma once


#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../interface/http.hpp"


using namespace std::literals::string_view_literals;


namespace jai::llm::http {


class Headers {
private:
    inline static const std::string_view WHITESPACE{" \t\f\v"sv};

    std::vector<std::string> headers{};
    std::vector<DroppedHeader> dropped_headers{};

public:
    Headers() = default;
    explicit Headers(const std::vector<std::string>& headers_) {
        std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
    }
    explicit Headers(const std::vector<std::string_view>& headers_) {
        std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
    }

    void AddDefaultHeader(const std::string& header) { ProcessDefaultHeaders(std::vector<std::string>{header}); }
    void AddDefaultHeaders(const std::vector<std::string>& headers_) { ProcessDefaultHeaders(headers_); }

    const std::vector<std::string>& GetHeaders() const { return headers; }
    const std::vector<DroppedHeader>& GetDroppedHeaders() const { return dropped_headers; }

private:
    void AddHeader(std::string_view header) {
        auto reason = IsNotValidHeader(header);
        if (reason) { dropped_headers.push_back({header, reason.value()}); }
        else        { headers.push_back(header); }
    }

    std::vector<std::string> ExtractKeys(const std::vector<std::string>& headers) const {
        return headers |
               std::views::transform([](const auto& header) {
                   auto pos = header.find(':');
                   auto key = header.substr(0, pos);

                   auto first = key.find_first_not_of(WHITESPACE);
                   if (first == std::string_view::npos) { return ""sv; }
                   auto last = key.find_last_not_of(WHITESPACE);
                   auto len = last - first + 1;
                   auto trimmed_key = key.substr(first, len);

                   return trimmed_key |
                          std::views::transform([](unsigned char c) {
                              if (c >= 'A' && c <= 'Z') { return c + ('a' - 'A'); }
                              return c;
                          }) |
                          std::ranges::to<std::string>();
               }) |
               std::ranges::to<std::vector<std::string>>();
    }

    std::optional<DroppedHeader::Reason> IsNotValidHeader(std::string_view header) const {
        auto pos = header.find_first_of(":\n\r"sv);
        if (pos == std::string_view::npos || header[pos] != ':') {
            return DroppedHeader::Reason::MissingColon;
        }
        if (header.find_first_of("\n\r"sv, pos + 1) != std::string_view::npos) {
            return DroppedHeader::Reason::ContainsNewline;
        }
        return std::nullopt;
    }

    void ProcessDefaultHeaders(const std::vector<std::string>& default_headers) {
        auto IsValidHeaderFunc = [this](std::string_view header) { return !IsNotValidHeader(header); };
        auto valid_default_headers = std::ranges::filter(default_headers, IsValidHeaderFunc) |
                                     std::ranges::to<std::vector<std::string>>();
        if (valid_default_headers.empty()) { return; }

        auto keys = ExtractKeys(headers);
        auto default_keys = ExtractKeys(valid_default_headers);
        for (auto const [index, key] : std::views::enumerate(default_keys)) {
            if (std::ranges::find(keys, key) == keys.end()) { headers.push_back(valid_default_headers[index]); }
        }
    }
};


} // namespace jai::llm::http
