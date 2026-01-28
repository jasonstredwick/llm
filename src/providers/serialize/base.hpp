#pragma once

#include "../../interface/types.hpp"

#include <cstdint>
#include <format>
#include <optional>
#include <type_traits>
#include <vector>

#include <simdjson.h>


namespace jai::llm {


enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };


// Forward declare
template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv) = delete;


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV_base(simdjson::builder::string_builder& builder, const T& v) {
    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    if constexpr (std::is_enum_v<T>) {
        builder.append_key_value<key>(to_string(v));
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
