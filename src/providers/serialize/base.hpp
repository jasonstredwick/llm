#pragma once

#include <cstdint>
#include <format>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

#include <simdjson.h>

#include "../../interface/core/types.hpp"
#include "../../interface/core/url.hpp"


namespace jai::llm {


enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV_base(simdjson::builder::string_builder& builder, const T& v) {
    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    builder.append_key_value<key>(v);
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


}


namespace simdjson {


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::EncodedUrl& value)
{
    builder.escape_and_append_with_quotes(value.View());
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::Int64& value)
{
    char buf[64];
    auto it = std::format_to(buf, "{}", value.Get());
    builder.escape_and_append_with_quotes(std::string_view(buf, it - buf));
}


template <int64_t L, int64_t U>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::IntN<L, U>& value)
{
    char buf[64];
    auto it = std::format_to(buf, "{}", value.Get());
    builder.escape_and_append_with_quotes(std::string_view(buf, it - buf));
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::Name64& value)
{
    builder.escape_and_append_with_quotes(value.Get());
}


template <size_t N>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::NameLen<N>& value)
{
    builder.escape_and_append_with_quotes(value.Get());
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::RFC3339Timestamp& value)
{
    char buf[64];
    auto it = std::format_to(buf, "{:%FT%TZ}", value.Get());
    builder.escape_and_append_with_quotes(std::string_view(buf, it - buf));
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::json::Value& value)
{
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


template <typename T>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::ValueBox<T>& value)
{
    tag_invoke(simdjson::serialize_tag{}, builder, value.get());
}


}
