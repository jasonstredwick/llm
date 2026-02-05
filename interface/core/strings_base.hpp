#pragma once

#include <optional>
#include <string_view>

#include "types.hpp"


namespace jai::llm {


template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


template <Kind_c T>
constexpr std::string_view to_string_view(const T&) { return T::value; }


}
