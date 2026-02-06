#pragma once

#include <charconv>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

#include <simdjson.h>

#include "../../../interface/core/error.hpp"
#include "../../../interface/core/types.hpp"
#include "../../../interface/protocols/anthropic/strings.hpp"
#include "../../../interface/protocols/gemini/strings.hpp"
#include "../../../interface/protocols/openai/strings.hpp"


/***
 * MACROs for source file only.
 */
#define FIELD(src, member) Extract<#member, T, &T::member>((src))
#define BEGIN_PARSE(Type)                             \
template <>                                           \
Type Parse<Type>(const simdjson::dom::element& src) { \
    using T = Type;                                   \
    return T{
#define  END_PARSE \
    };             \
}


namespace jai::llm {


template <typename T>
std::vector<T> ParseArrayOf(const simdjson::dom::element& src) {
    return src.get_array() |
           std::views::transform([](auto const& in) { return Parse<T>(in); }) |
           std::ranges::to<std::vector<T>>();
}


template <typename K, typename V>
std::map<K, V> ParseMapOf(const simdjson::dom::element& src) {
    return  src.get_object() |
            std::views::transform([](auto const& kv) {
                auto const& [key, value] = kv;
                return std::pair{K{key}, Parse<V>(value)};
            }) |
            std::ranges::to<std::map<K, V>>();
}


/***
 * Parse
 */
template <typename T>
T Parse(const simdjson::dom::element&) = delete;


template <typename T>
requires (Kind_c<T> && !std::same_as<T, std::byte>)
T Parse(const simdjson::dom::element& src) {
    auto sv = src.get_string().value();
    auto result = from_string_view<T>(sv);
    if (!result) { throw AnnotatedException{ std::string{"Invalid Kind: "} + std::string{sv}}; }
    return *result;
}


template <typename T>
requires (std::is_enum_v<std::remove_cvref_t<T>> && !std::same_as<T, std::byte>)
T Parse(const simdjson::dom::element& src) {
    auto sv = src.get_string().value();
    auto result = from_string_view<T>(sv);
    if (!result) { throw AnnotatedException{ std::string{"Invalid enum: "} + std::string{sv}}; }
    return *result;
}


template <typename T>
requires Like_c<bool, T>
bool Parse(const simdjson::dom::element& src) {
    return src.get_bool().value();
}


template <typename T>
requires Like_c<std::byte, T>
std::byte Parse(const simdjson::dom::element& src) {
    uint64_t v = src.get_uint64().value();
    if (v > 0xFF) { throw AnnotatedException{"byte value out of range"}; }
    return static_cast<std::byte>(v);
}


template <typename T>
requires Like_c<double, T>
double Parse(const simdjson::dom::element& src) {
    return src.get_double().value();
}


template <typename T>
requires Like_c<EncodedUrl, T>
EncodedUrl Parse(const simdjson::dom::element& src) {
    return EncodedUrl{src.get_string().value()};
}


template <typename T>
requires Like_c<int64_t, T>
int64_t Parse(const simdjson::dom::element& src) {
    return src.get_int64().value();
}


template <typename T>
requires Like_c<Int64Str, T>
Int64Str Parse(const simdjson::dom::element& src) {
    const std::string_view str = src.get_string().value();
    const char* it_begin = str.data();
    const char* it_end = it_begin + str.size();
    int64_t value = 0;
    auto [ptr, ec] = std::from_chars(it_begin, it_end, value);
    if (ec != std::errc{}) {
        throw AnnotatedException{"Failed to parse string; expected number if string form."};
    }
    return Int64Str{value};
}


template <typename T>
requires Like_c<Int64, T>
Int64 Parse(const simdjson::dom::element& src) {
    return Int64{src.get_int64().value()};
}


template <int64_t N1, int64_t N2>
IntN<N1, N2> Parse(const simdjson::dom::element& src) {
    return IntN<N1, N2>{src.get_int64().value()};
}


template <typename K, typename V>
std::map<K, V> Parse(const simdjson::dom::element& src) {
    return ParseMapOf<K, V>(src);
}


template <typename T>
requires Like_c<Name64, T>
Name64 Parse(const simdjson::dom::element& src) {
    return Name64{src.get_string().value()};
}


template <typename T>
requires Like_c<NameLen<64>, T>
NameLen<64> Parse(const simdjson::dom::element& src) {
    return NameLen<64>{src.get_string().value()};
}


template <typename T>
requires Like_c<NameLen<512>, T>
NameLen<512> Parse(const simdjson::dom::element& src) {
    return NameLen<512>{src.get_string().value()};
}


template <typename T>
requires Like_c<RFC3339Timestamp, T>
RFC3339Timestamp Parse(const simdjson::dom::element& src) {
    return RFC3339Timestamp::Parse(src.get_string().value());
}


template <typename T>
requires Like_c<std::string, T>
std::string Parse(const simdjson::dom::element& src) {
    return std::string{src.get_string().value()};
}


template <typename T>
requires Like_c<std::string_view, T>
std::string_view Parse(const simdjson::dom::element& src) {
    return src.get_string().value();
}


template <typename T>
requires Like_c<json::Value, T>
json::Value Parse(const simdjson::dom::element& src) {
    switch (src.type()) {
    case simdjson::dom::element_type::NULL_VALUE:
        return {nullptr};
    case simdjson::dom::element_type::BOOL:
        return {src.get_bool().value()};
    case simdjson::dom::element_type::UINT64:
    {
        uint64_t x = src.get_uint64().value();
        return {
            x > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) ?
                static_cast<double>(x) :
                static_cast<int64_t>(x)
        };
    }
    case simdjson::dom::element_type::INT64:
        return {src.get_int64().value()};
    case simdjson::dom::element_type::DOUBLE:
        return {src.get_double().value()};
    case simdjson::dom::element_type::STRING:
        return {std::string(src.get_string().value())};
    case simdjson::dom::element_type::ARRAY:
        return {
            src.get_array() |
            std::views::transform([](auto const& v){ return Parse<json::Value>(v); }) |
            std::ranges::to<json::Array>()
        };
    case simdjson::dom::element_type::OBJECT:
        return {
            src.get_object() |
            std::views::transform([](auto&& kv) {
                auto const& [key, value] = kv;
                return std::pair{std::string{key}, Parse<json::Value>(value)};
            }) |
            std::ranges::to<json::Object>()
        };
    }
    std::unreachable();
}


template <typename T>
requires Like_c<json::Object, T>
json::Object Parse(const simdjson::dom::element& src) {
    return  src.get_object() |
            std::views::transform([](auto&& kv) {
                auto const& [key, value] = kv;
                return std::pair{std::string{key}, Parse<json::Value>(value)};
            }) |
            std::ranges::to<json::Object>();
}


template <typename T, typename Allocator>
std::vector<T, Allocator> Parse(const simdjson::dom::element& src) {
    return ParseArrayOf<T>(src);
}


/***
 * Extract
 */
template <simdjson::constevalutil::fixed_string key, typename T, auto Member>
requires (Optional_c<member_type_t<T, Member>>)
member_type_t<T, Member> Extract(const simdjson::dom::element& src) {
    using U = member_type_t<T, Member>::value_type;

    auto obj = src.get_object();
    auto r = obj.at_key(key);
    if (r.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }

    const simdjson::dom::element& elem = r.value();
    if constexpr (StdVector_c<U>) {
        return ParseArrayOf<typename U::value_type>(elem);
    } else if constexpr (StdMap_c<U>) {
        return ParseMapOf<typename U::key_type,
                          typename U::mapped_type>(src[key]);
    } else {
        return Parse<typename member_type_t<T, Member>::value_type>(elem);
    }
}


template <simdjson::constevalutil::fixed_string key, typename T, auto Member>
member_type_t<T, Member> Extract(const simdjson::dom::element& src) {
    using U = member_type_t<T, Member>;

    if constexpr (StdVector_c<U>) {
        return ParseArrayOf<typename member_type_t<T, Member>::value_type>(src[key]);
    } else if constexpr (StdMap_c<U>) {
        return ParseMapOf<typename member_type_t<T, Member>::key_type,
                          typename member_type_t<T, Member>::mapped_type>(src[key]);
    } else {
        return Parse<member_type_t<T, Member>>(src[key]);
    }
}


template <simdjson::constevalutil::fixed_string key>
std::optional<simdjson::dom::element> ExtractForVariant(const simdjson::simdjson_result<simdjson::dom::object>& obj) {
    simdjson::simdjson_result<simdjson::dom::element> result = obj.at_key(key);
    if (result.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }
    return simdjson::dom::element(result.value());
}


}
