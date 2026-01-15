#pragma once


#include <deque>
#include <memory>
#include <variant>

#include "concepts.hpp"


namespace jai::llm {


struct EmptySlot {
    EmptySlot* next{nullptr};
    EmptySlot* prev{nullptr};
};


template <typename... Ts>
requires (!TypeInPack_c<EmptySlot, Ts...>)
using SlotVariant_t = std::variant<EmptySlot, Ts...>;


template <typename... Ts>
class SlotPool {
    using Slot_t = SlotVariant_t<Ts...>;
    using SlotList_t = std::deque<Slot_t>;
};

} // namespace jai::llm
