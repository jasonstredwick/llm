#pragma once

#include <string_view>


namespace jai::llm::debug {


constexpr std::string_view strip_prefix(std::string_view s) {
    for (auto prefix : {
        std::string_view{"struct "},
        std::string_view{"class "},
        std::string_view{"enum "}
    }) {
        if (s.starts_with(prefix)) {
            return s.substr(prefix.size());
        }
    }
    return s;
}


template <typename T>
consteval std::string_view type_name() {
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view p = __PRETTY_FUNCTION__;
    constexpr std::string_view key = "T = ";
    auto start = p.find(key) + key.size();
    auto end = p.find(']', start);
    return strip_prefix(p.substr(start, end - start));
#elif defined(_MSC_VER)
    constexpr std::string_view p = __FUNCSIG__;
    constexpr std::string_view key = "type_name<";
    auto start = p.find(key) + key.size();
    auto end = p.find(">(void)", start);
    return strip_prefix(p.substr(start, end - start));
#else
    return "unknown";
#endif
}


}
