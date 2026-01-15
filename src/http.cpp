#include "http.hpp"

#include <algorithm>
#include <ranges>


namespace jai::llm::http {


Headers::Headers(const std::vector<std::string>& headers_) {
    std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
}


Headers::Headers(const std::vector<std::string_view>& headers_) {
    std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
}


std::vector<std::string> Headers::ExtractKeys(const std::vector<std::string>& headers_) const {
    return  headers_ |
            std::views::transform([](const auto& header) {
                auto pos = header.find(':');
                auto key = header.substr(0, pos);

                auto first = key.find_first_not_of(WHITESPACE);
                if (first == std::string_view::npos) { return std::string{}; }
                auto last = key.find_last_not_of(WHITESPACE);
                auto len = last - first + 1;
                auto trimmed_key = key.substr(first, len);

                return trimmed_key |
                       std::views::transform([](char c) {
                           unsigned char uc = static_cast<unsigned char>(c);
                           if (uc >= 'A' && uc <= 'Z') {
                               return static_cast<char>(uc + ('a' - 'A'));
                           }
                           return c;
                       }) |
                       std::ranges::to<std::string>();
            }) |
            std::ranges::to<std::vector<std::string>>();
}


std::optional<DroppedHeader::Reason> Headers::IsNotValidHeader(std::string_view header) const {
    auto pos = header.find_first_of(":\n\r"sv);
    if (pos == std::string_view::npos || header[pos] != ':') {
        return DroppedHeader::Reason::MissingColon;
    }
    if (header.find_first_of("\n\r"sv, pos + 1) != std::string_view::npos) {
        return DroppedHeader::Reason::ContainsNewline;
    }
    return std::nullopt;
}


void Headers::ProcessDefaultHeaders(const std::vector<std::string>& default_headers_) {
    auto IsValidHeaderFunc = [this](std::string_view header) { return !IsNotValidHeader(header); };
    auto valid_default_headers = default_headers_ |
                                 std::views::filter(IsValidHeaderFunc) |
                                 std::ranges::to<std::vector<std::string>>();
    if (valid_default_headers.empty()) { return; }

    auto keys = ExtractKeys(headers);
    auto default_keys = ExtractKeys(valid_default_headers);
    for (auto const [index, key] : std::views::enumerate(default_keys)) {
        if (std::ranges::find(keys, key) == keys.end()) { headers.push_back(valid_default_headers[index]); }
    }
}


} // namespace jai::llm::http
