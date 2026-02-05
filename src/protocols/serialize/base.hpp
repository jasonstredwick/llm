#pragma once

#include <algorithm>
#include <cstdint>
#include <format>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <vector>

#include <simdjson.h>

#include "../../interface/core/types.hpp"
#include "../../../interface/protocols/anthropic/strings.hpp"
#include "../../../interface/protocols/gemini/strings.hpp"
#include "../../../interface/protocols/openai/strings.hpp"


#define TAG_ENUM(T) \
    inline void tag_invoke(simdjson::serialize_tag, string_builder& builder, T v) { \
        builder.escape_and_append_with_quotes(to_string_view(v)); \
    }

#define TAG_KIND(T) \
    inline void tag_invoke(simdjson::serialize_tag, string_builder& builder, T v) { \
        builder.escape_and_append_with_quotes(to_string_view(v)); \
    }


namespace jai::llm {


/***
 * Helper functions for designating whether the key/field is required or optionally present:
 *
 * AddReqKV: Always emits the field. Empty containers serialize as [] and disengaged optionals serialize as null.
 * AddOptKV: Omits the field entirely when empty or disengaged.
 */
enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };


template <typename T>
inline void AppendNumber(simdjson::builder::string_builder& builder, T v) {
    builder.append(v);
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddKV_base(simdjson::builder::string_builder& builder, const T& v) {
    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    if constexpr (std::is_arithmetic_v<T> &&
                  !std::is_same_v<std::remove_cv_t<T>, char> &&
                  !std::is_same_v<std::remove_cv_t<T>, signed char> &&
                  !std::is_same_v<std::remove_cv_t<T>, unsigned char>)
    {
        builder.escape_and_append_with_quotes(key);
        builder.append_colon();
        builder.append<T>(v);
    } else {
        builder.append_key_value<key>(v);
    }
    if constexpr (Dir == CommaDirection::AFTER || Dir == CommaDirection::BOTH) { builder.append_comma(); }
}


// Required fields; adds field no matter what.
template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddReqKV(simdjson::builder::string_builder& builder, const std::vector<T>& v) {
    AddKV_base<key, Dir>(builder, v);
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
requires (std::is_arithmetic_v<T> &&
          !std::same_as<T, char> && !std::same_as<T, signed char> && !std::same_as<T, unsigned char>)
inline void AddReqKV(simdjson::builder::string_builder& builder, const std::vector<T>& v) {
    auto Append = [&builder](T const& x) {
        if constexpr (std::same_as<T, bool>) {
            builder.append<bool>(x);
        } else {
            builder.append<T>(x);
        }
    };

    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    builder.escape_and_append_with_quotes(key);
    builder.append_colon();
    builder.start_array();
    if (!v.empty()) {
        Append(v[0]);
        for (auto const& x : v | std::views::drop(1)) {
            builder.append_comma();
            Append(x);
        }
    }
    builder.end_array();
    if constexpr (Dir == CommaDirection::AFTER || Dir == CommaDirection::BOTH) { builder.append_comma(); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddReqKV(simdjson::builder::string_builder& builder, const std::optional<T>& v) {
    AddKV_base<key, Dir>(builder, v);
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddReqKV(simdjson::builder::string_builder& builder, const T& v) {
    AddKV_base<key, Dir>(builder, v);
}


// Optional fields; does not add field if optional is nullopt.
template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const std::vector<T>& v) {
    if (!v.empty()) { AddReqKV<key, Dir>(builder, v); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const std::optional<T>& v) {
    if (v) { AddKV_base<key, Dir>(builder, *v); }
}


template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddOptKV(simdjson::builder::string_builder& builder, const T& v) {
    AddKV_base<key, Dir>(builder, v);
}


}


namespace simdjson {


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder, std::byte value) {
    builder.append(static_cast<uint64_t>(value));
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::EncodedUrl& value)
{
    builder.escape_and_append_with_quotes(value.Get());
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::Int64& value)
{
    jai::llm::AppendNumber(builder, value.Get());
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::Int64Str& value)
{
    builder.escape_and_append_with_quotes(value.Get());
}


template <int64_t L, int64_t U>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::IntN<L, U>& value)
{
    jai::llm::AppendNumber(builder, value.Get());
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
                       const jai::llm::json::Value& value);


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::json::Array& value)
{
    builder.start_array();
    if (!value.empty()) {
        tag_invoke(simdjson::serialize_tag{}, builder, value[0]);
        for (auto const& x : value | std::views::drop(1)) {
            builder.append_comma();
            tag_invoke(simdjson::serialize_tag{}, builder, x);
        }
    }
    builder.end_array();
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::json::Object& value)
{
    builder.start_object();
    if (!value.empty()) {
        auto it = value.begin();
        builder.escape_and_append_with_quotes(it->first);
        builder.append_colon();
        tag_invoke(simdjson::serialize_tag{}, builder, it->second);
        
        for (++it; it != value.end(); ++it) {
            builder.append_comma();
            builder.escape_and_append_with_quotes(it->first);
            builder.append_colon();
            tag_invoke(simdjson::serialize_tag{}, builder, it->second);
        }
    }
    builder.end_object();
}


inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::json::Value& value)
{
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            builder.append_null();
        } else if constexpr (std::is_same_v<T, bool>) {
            builder.append<bool>(x);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            builder.append<int64_t>(x);
        } else if constexpr (std::is_same_v<T, double>) {
            builder.append<double>(x);
        } else if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else {
            tag_invoke(simdjson::serialize_tag{}, builder, x);
        }
    }, value.data);
}


template <typename T>
inline void tag_invoke(simdjson::serialize_tag, simdjson::builder::string_builder& builder,
                       const jai::llm::ValueBox<T>& value)
{
    tag_invoke(simdjson::serialize_tag{}, builder, value.Get());
}


}
