#pragma once

#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <simdjson.h>

#include "../../../interface/core/types.hpp"
#include "../../../interface/core/url.hpp"


namespace jai::llm {


// Forward declare
template <typename T>
constexpr std::optional<T> from_string_view(std::string_view sv);


/***
 * Parse
 */
template <typename T> T Parse(const simdjson::dom::element&) = delete;


template <typename T>
requires Like_c<bool, T>
bool Parse(const simdjson::dom::element& src) {
    return src.get_bool().value();
}


template <typename T>
requires (std::is_enum_v<std::remove_cvref_t<T>>)
T Parse(const simdjson::dom::element& src) {
    auto sv = src.get_string().value();
    auto result = from_string_view<T>(sv);
    if (!result) { throw std::runtime_error{ std::string{"Invalid enum value: "} + std::string{sv}}; }
    return *result;
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
requires Like_c<std::map<std::string, std::string>, T>
std::map<std::string, std::string> Parse(const simdjson::dom::element& src) {
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        return std::pair{std::string{key}, Parse<std::string>(value)};
    }) | std::ranges::to<std::map<std::string, std::string>>();
}


template <typename T>
requires Like_c<json::Object, T>
json::Object Parse(const simdjson::dom::element& src) {
    return src.get_object() | std::views::transform([](auto&& kv) {
        auto const& [key, value] = kv;
        return std::pair{std::string{key}, Parse<json::Value>(value)};
    }) | std::ranges::to<json::Object>();
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
requires Like_c<NameLen<64>, T>
NameLen<64> Parse(const simdjson::dom::element& src) {
    return NameLen<64>{Parse<std::string>(src)};
}

template <typename T>
requires Like_c<NameLen<512>, T>
NameLen<512> Parse(const simdjson::dom::element& src) {
    return NameLen<512>{Parse<std::string>(src)};
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
            src.get_object() | std::views::transform([](auto&& kv) {
                auto const& [key, value] = kv;
                return std::pair{std::string{key}, Parse<json::Value>(value)};
            }) |
            std::ranges::to<json::Object>()
        };
    }
    std::unreachable();
}


template <typename T, typename Allocator>
std::vector<T, Allocator> Parse(const simdjson::dom::element& src) {
    return src.get_array() | std::views::transform([](auto const& in) { return Parse<T>(in); }) | std::ranges::to();
}


template <typename T>
std::vector<T> ParseArrayOf(const simdjson::dom::element& src) {
    return src.get_array() | std::views::transform([](auto const& in) { return Parse<T>(in); }) | std::ranges::to();
}


/***
 * Extract
 */
template <simdjson::constevalutil::fixed_string key, typename T, auto Member>
requires (Optional_c<member_type_t<T, Member>>)
member_type_t<T, Member> Extract(const simdjson::dom::element& src) {
    using ValueType = member_type_t<T, Member>::value_type;
    auto obj = src.get_object();
    auto r = obj.at_key(key);
    if (r.error() == simdjson::NO_SUCH_FIELD) {
        return std::nullopt;
    } else {
        const simdjson::dom::element& elem = r.value();
        return Parse<ValueType>(elem);
    }
}


template <simdjson::constevalutil::fixed_string key, typename T, auto Member>
member_type_t<T, Member> Extract(const simdjson::dom::element& src) {
    using U = member_type_t<T, Member>;
    return Parse<U>(src[key]);
}


template <simdjson::constevalutil::fixed_string key>
std::optional<simdjson::dom::element> ExtractForVariant(const simdjson::simdjson_result<simdjson::dom::object>& obj) {
    simdjson::simdjson_result<simdjson::dom::object> result = obj.at_key(key);
    if (result.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }
    return simdjson::dom::element(result.value());
}


}
