#pragma once


#include <string_view>


namespace jai::llm {


template<typename T>
constexpr std::string_view to_string_view(T val);


} // namespace jai::llm
