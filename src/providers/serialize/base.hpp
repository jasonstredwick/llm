#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

#include <simdjson.h>

#include "../../interface/core/types.hpp"


namespace jai::llm {


enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };


// Forward declare
constexpr std::string_view to_string_view(auto val) = delete;


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV_base(simdjson::builder::string_builder& builder, const T& v) {
    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    if constexpr (std::is_enum_v<T>) {
        builder.append_key_value<key>(jai::llm::to_string_view(v));
    } else {
        builder.append_key_value<key>(v);
    }
    if constexpr (Dir == CommaDirection::AFTER || Dir == CommaDirection::BOTH) { builder.append_comma(); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const std::vector<T>& v) {
    if (!v.empty()) { AddOptKV_base<key, Dir>(builder, v); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const std::optional<T>& v) {
    if (v) { AddOptKV_base<key, Dir>(builder, *v); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const T& v) {
    AddOptKV_base<key, Dir>(builder, v);
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder, const Int64& value) {
    builder.escape_and_append_with_quotes(std::format("{}", value.Get()));
}


template <int64_t L, int64_t U>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder, const IntN<L, U>& value) {
    builder.escape_and_append_with_quotes(std::format("{}", value.Get()));
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder, const Name64& value) {
    builder.escape_and_append_with_quotes(value.Get());
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const RFC3339Timestamp& value)
{
    builder.escape_and_append_with_quotes(std::format("{:%FT%TZ}", value.Get()));
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder, const json::Value& value) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            builder.append_null();
        } else if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            builder.append(x);
        }
    }, value.data);
}


}
