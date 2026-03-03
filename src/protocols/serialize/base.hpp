#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>

#include <simdjson.h>

#include "../../../interface/core/strings_base.hpp"
#include "../../../interface/core/types.hpp"


/***
 * MACROs for source file only.
 */
#define FIELD(obj, member, comma) AddKV<#member, comma>(builder, (obj.member));
#define FIELD_ALT(obj, member, name, comma) AddKV<name, comma>(builder, (obj.member));
#define BEGIN_SERIALIZE(Type)\
void SerializeFrom(simdjson::builder::string_builder& builder, const Type& obj) {\
    builder.start_object();
#define BEGIN_SERIALIZE_EMPTY(Type)\
void SerializeFrom(simdjson::builder::string_builder& builder, const Type&) {\
    builder.start_object();
#define END_SERIALIZE\
    builder.end_object();\
}


namespace jai::llm {


/***
 * Forward declarations
 */
template <typename T>
void SerializeFrom(simdjson::builder::string_builder&, const T&) = delete;

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder&, const std::optional<T>&);

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder&, const std::vector<T>&);

template <typename... Ts>
inline void SerializeFrom(simdjson::builder::string_builder&, const std::variant<Ts...>&);

template <typename K, typename V>
requires std::convertible_to<K, std::string_view>
inline void SerializeFrom(simdjson::builder::string_builder&, const std::map<K, V>&);

inline void SerializeFrom(simdjson::builder::string_builder&, const std::string&);
inline void SerializeFrom(simdjson::builder::string_builder&, const std::string_view&);

inline void SerializeFrom(simdjson::builder::string_builder&, const json::Value&);


/***
 * Serialize
 */
// Kinds + enums
template <typename T>
requires ((std::is_enum_v<std::remove_cvref_t<T>> || Kind_c<T>) && !std::same_as<T, std::byte>)
inline void SerializeFrom(simdjson::builder::string_builder& builder, const T& v) {
    std::string_view name = to_string_view(v);
    builder.escape_and_append_with_quotes(name);
}


// Base types
inline void SerializeFrom(simdjson::builder::string_builder& builder, const bool& value) {
    builder.append(value);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::byte& value) {
    builder.append(static_cast<uint64_t>(value));
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const char& value) {
    builder.append(value);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const double& value) {
    builder.append(value);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const EncodedUrl& obj) {
    builder.escape_and_append_with_quotes(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const int64_t& value) {
    builder.append(value);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const Int64& obj) {
    builder.append(obj.Get());
}

template <int64_t N1, int64_t N2>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const Int64Bounded<N1, N2>& obj) {
    builder.append(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const Int64Str& obj) {
    builder.escape_and_append_with_quotes(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const Name64& obj) {
    builder.escape_and_append_with_quotes(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const NameLen<64>& obj) {
    builder.escape_and_append_with_quotes(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const NameLen<512>& obj) {
    builder.escape_and_append_with_quotes(obj.Get());
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const RFC3339Timestamp& obj) {
    char buf[64];
    auto it = std::format_to(buf, "{:%FT%TZ}", obj.Get());
    builder.escape_and_append_with_quotes(std::string_view(buf, it - buf));
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::string& obj) {
    builder.escape_and_append_with_quotes(obj);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::string_view& obj) {
    builder.escape_and_append_with_quotes(obj);
}


// Containers
template <typename K, typename V>
requires std::convertible_to<K, std::string_view>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::map<K, V>& obj) {
    builder.start_object();
    if (!obj.empty()) {
        bool add_comma = false;
        for (auto const& [key, value] : obj) {
            if (add_comma) { builder.append_comma(); }
            else           { add_comma = true; }
            builder.escape_and_append_with_quotes(key);
            builder.append_colon();
            SerializeFrom(builder, value);
        }
    }
    builder.end_object();
}

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::optional<T>& obj) {
    if (obj.has_value()) { SerializeFrom(builder, *obj); }
    else                 { builder.append_null(); }
}

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const Required<T>& obj) {
    SerializeFrom(builder, static_cast<T>(obj));
}

template <typename... Ts>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::variant<Ts...>& obj) {
    std::visit([&](auto const& x) { SerializeFrom(builder, x); }, obj);
}

inline void SerializeFrom(simdjson::builder::string_builder& builder, const json::Value& obj) {
    std::visit([&](auto const& x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            builder.append_null();
        } else if constexpr (std::is_same_v<T, bool>) {
            builder.append(x);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            builder.append(x);
        } else if constexpr (std::is_same_v<T, double>) {
            builder.append(x);
        } else if constexpr (std::is_same_v<T, std::string>) {
            builder.escape_and_append_with_quotes(x);
        } else if constexpr (std::is_same_v<T, json::Array>) {
            SerializeFrom(builder, x);
        } else if constexpr (std::is_same_v<T, json::Object>) {
            SerializeFrom(builder, x);
        } else {
            static_assert(always_false_v<T>, "json::Value contains a non-serializable alternative");
        }
    }, obj.data);
}

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const ValueBox<T>& obj) {
    SerializeFrom(builder, obj.Get());
}

template <typename T>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::vector<T>& obj) {
    builder.start_array();
    if (!obj.empty()) {
        bool add_comma = false;
        for (auto const& value : obj) {
            if (add_comma) { builder.append_comma(); }
            else           { add_comma = true; }
            SerializeFrom(builder, value);
        }
    }
    builder.end_array();
}

template <>
inline void SerializeFrom(simdjson::builder::string_builder& builder, const std::vector<bool>& obj) {
    builder.start_array();
    if (!obj.empty()) {
        bool add_comma = false;
        for (bool value : obj) {
            if (add_comma) { builder.append_comma(); }
            else           { add_comma = true; }
            builder.append(value);
        }
    }
    builder.end_array();
}


/***
 * Helper functions for designating whether the key/field is required or optionally present:
 *
 * AddReqKV: Always emits the field. Empty containers serialize as [] and disengaged optionals serialize as null.
 * AddOptKV: Omits the field entirely when empty or disengaged.
 */
enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };


template <simdjson::constevalutil::fixed_string key_, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddKV_base(simdjson::builder::string_builder& builder, const T& v) {
    if constexpr (Dir == CommaDirection::BEFORE || Dir == CommaDirection::BOTH) { builder.append_comma(); }
    std::string_view key{key_};
    builder.escape_and_append_with_quotes(key);
    builder.append_colon();
    SerializeFrom(builder, v);
    if constexpr (Dir == CommaDirection::AFTER || Dir == CommaDirection::BOTH) { builder.append_comma(); }
}


// Required fields; adds field no matter what.
template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddKV(simdjson::builder::string_builder& builder, const Required<T>& v) {
    AddKV_base<key, Dir>(builder, v);
}

template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddKV(simdjson::builder::string_builder& builder, const std::optional<T>& v) {
    if (v) { AddKV_base<key, Dir>(builder, *v); }
}

template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename T>
inline void AddKV(simdjson::builder::string_builder& builder, const std::vector<T>& v) {
    if (!v.empty()) { AddKV_base<key, Dir>(builder, v); }
}

template <simdjson::constevalutil::fixed_string key, CommaDirection Dir = CommaDirection::NONE, typename K, typename V>
inline void AddKV(simdjson::builder::string_builder& builder, const std::map<K, V>& v) {
    if (!v.empty()) { AddKV_base<key, Dir>(builder, v); }
}


}
