#include "http.hpp"

#include <algorithm>
#include <ranges>
#include <stdexcept>


namespace jai::llm::http {


std::vector<std::string> RequestHeaders::FromKVRange(HeaderKVRange_c auto const& in) {
    return
        in |
        std::views::transform([](auto const& kv) {
            auto const& [k, v] = kv;
            std::string str{};
            std::string_view key{k};
            std::string value = RequestHeaders::ToValueStr(v);

            str.reserve(key.size() + 2 + value.size());
            str += key;
            str += ": ";
            str += value;

            RequestHeaders::SecurityCheck(str);

            return str;
        }) |
        std::ranges::to<std::vector<std::string>>();
}


std::vector<std::string> RequestHeaders::FromSeq(MergedHeaderRange_c auto const& in) {
    return
        in |
        std::views::transform([](std::string_view sv) {
            std::string str{sv};
            RequestHeaders::SecurityCheck(str);
            return str;
        }) |
        std::ranges::to<std::vector<std::string>>();
}


void RequestHeaders::SecurityCheck(std::string_view sv) {
    if (sv.find('\0') != std::string_view::npos) {
        throw std::invalid_argument(std::string{"Header contains embedded NUL (\\0) character."});
    }
}


std::string RequestHeaders::ToValueStr(auto const& v) {
    if constexpr (std::convertible_to<decltype(v), std::string_view>) {
        return std::string{std::string_view{v}};
    } else {
        std::string_view sv{v};
        return std::string{reinterpret_cast<const char*>(sv.data()), sv.size()};
    }
}


ResponseHeaders::ResponseHeaders(const std::vector<std::string>& headers_) {
    std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
}


ResponseHeaders::ResponseHeaders(const std::vector<std::string_view>& headers_) {
    std::ranges::for_each(headers_, [this](const auto& header) { AddHeader(header); });
}


std::vector<std::string> ResponseHeaders::ExtractKeys(const std::vector<std::string>& headers_) const {
    return 
        headers_ |
        std::views::transform([](const auto& header) {
            auto pos = header.find(':');
            auto key = header.substr(0, pos);

            auto first = key.find_first_not_of(WHITESPACE);
            if (first == std::string_view::npos) { return std::string{}; }
            auto last = key.find_last_not_of(WHITESPACE);
            auto len = last - first + 1;
            auto trimmed_key = key.substr(first, len);

            return
                trimmed_key |
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


std::optional<DroppedHeader::Reason> ResponseHeaders::IsNotValidHeader(std::string_view header) const {
    auto pos = header.find_first_of(":\n\r"sv);
    if (pos == std::string_view::npos || header[pos] != ':') {
        return DroppedHeader::Reason::MissingColon;
    }
    if (header.find_first_of("\n\r"sv, pos + 1) != std::string_view::npos) {
        return DroppedHeader::Reason::ContainsNewline;
    }
    return std::nullopt;
}


void ResponseHeaders::ProcessDefaultHeaders(const std::vector<std::string>& default_headers_) {
    auto IsValidHeaderFunc = [this](std::string_view header) { return !IsNotValidHeader(header); };
    auto valid_default_headers = default_headers_ |
                                 std::views::filter(IsValidHeaderFunc) |
                                 std::ranges::to<std::vector<std::string>>();
    if (valid_default_headers.empty()) { return; }

    auto keys = ExtractKeys(headers);
    auto default_keys = ExtractKeys(valid_default_headers);
#if defined(__cpp_lib_ranges_enumerate) && __cpp_lib_ranges_enumerate >= 202302L
    for (auto const [index, key] : std::views::enumerate(default_keys)) {
        if (std::ranges::find(keys, key) == keys.end()) { headers.push_back(valid_default_headers[index]); }
    }
#else
    for (size_t i = 0; i < default_keys.size(); ++i) {
        if (std::ranges::find(keys, default_keys[i]) == keys.end()) {
            headers.push_back(valid_default_headers[i]);
        }
    }
#endif
}


} // namespace jai::llm::http
