#pragma once

#include <string_view>


namespace jai::llm::debug {


template <typename T>
consteval std::string_view type_name() {
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view p = __PRETTY_FUNCTION__;
    constexpr std::string_view key = "T = ";
    auto start = p.find(key) + key.size();
    auto end = p.find(']', start);
    return p.substr(start, end - start);
#elif defined(_MSC_VER)
    constexpr std::string_view p = __FUNCSIG__;
    constexpr std::string_view key = "type_name<";
    auto start = p.find(key) + key.size();
    auto end = p.find(">(void)", start);
    return p.substr(start, end - start);
#else
    return "unknown";
#endif
}


}
