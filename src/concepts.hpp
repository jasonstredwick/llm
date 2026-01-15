#pragma once

#include <concepts>
#include <type_traits>


namespace jai::llm {


template<typename T, typename... Ts>
concept TypeInPack_c = (std::same_as<T, Ts> || ...);


} // namespace jai::llm
