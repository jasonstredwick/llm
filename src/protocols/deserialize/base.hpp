#pragma once

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

#include <simdjson.h>

#include "debug.hpp"
#include "../../../interface/core/error.hpp"
#include "../../../interface/core/strings_base.hpp"
#include "../../../interface/core/types.hpp"


/***
 * MACROs for source file only.
 */
#define FIELD(src, member) Extract<#member, T, &T::member>((src))
#define FIELD_PLACEHOLDER(src, member) ExtractPlaceholder<T, &T::member>((src))
#define BEGIN_DESERIALIZE(Type)\
template <>\
Type DeserializeTo<Type>(const simdjson::dom::element& src) {\
    using T = Type;\
    return T{
#define END_DESERIALIZE\
    };\
}

#define EXTRACT_KIND(Type, src, field_name) ExtractKind<Type>((src), field_name, #Type)
#define FIELD_KIND(src, kind, target, Type) if (kind == target) { return T{DeserializeTo<Type>((src))}; }
#define FIELD_VARIANT(obj, name, Type) if (auto r = ExtractInPlace<name>((obj)); r.has_value()) { return T{DeserializeTo<Type>(*r)}; }
#define FIELD_VARIANT_KV(obj, name, Type) if (auto r = ExtractInPlaceKV<name>((obj)); r.has_value()) { return T{DeserializeTo<Type>(*r)}; }
#define BEGIN_DESERIALIZE_VARIANT(Type)\
template <>\
Type DeserializeTo<Type>(const simdjson::dom::element& src) {\
    using T = Type;
#define END_DESERIALIZE_VARIANT(Type)\
    throw AnnotatedException{#Type " variant unsatisfied."};\
}


namespace jai::llm {


/***
 * DeserializeTo
 */
// Forward declarations
template <typename T>
T DeserializeTo(const simdjson::dom::element&) = delete;


template <typename T>
requires StdMap_c<T>
T DeserializeTo(const simdjson::dom::element&);

template <typename T>
requires Optional_c<T>
T DeserializeTo(const simdjson::dom::element&);

template <typename T>
requires Required_c<T>
T DeserializeTo(const simdjson::dom::element&);

template <typename T>
requires Like_c<json::Value, T>
json::Value DeserializeTo(const simdjson::dom::element&);

template <typename T>
requires ValueBox_c<T>
T DeserializeTo(const simdjson::dom::element&);

template <typename T>
requires StdVector_c<T>
T DeserializeTo(const simdjson::dom::element&);


// Kinds
template <typename T>
requires (Kind_c<T> && !std::same_as<T, std::byte>)
T DeserializeTo(const simdjson::dom::element& obj) {
    auto sv = obj.get_string().value();
    auto result = from_string_view<T>(sv);
    if (!result) { throw AnnotatedException{ std::string{"Invalid Kind: "} + std::string{sv}}; }
    return *result;
}


// Enums
template <typename T>
requires (std::is_enum_v<std::remove_cvref_t<T>> && !std::same_as<T, std::byte>)
T DeserializeTo(const simdjson::dom::element& obj) {
    auto sv = obj.get_string().value();
    auto result = from_string_view<T>(sv);
    if (!result) { throw AnnotatedException{ std::string{"Invalid enum: "} + std::string{sv}}; }
    return *result;
}


// Base types
template <typename T>
requires Like_c<bool, T>
bool DeserializeTo(const simdjson::dom::element& obj) {
    return obj.get_bool().value();
}

template <typename T>
requires Like_c<std::byte, T>
std::byte DeserializeTo(const simdjson::dom::element& obj) {
    uint64_t v = obj.get_uint64().value();
    if (v > 0xFF) { throw AnnotatedException{"byte value out of range"}; }
    return static_cast<std::byte>(v);
}

template <typename T>
requires Like_c<double, T>
double DeserializeTo(const simdjson::dom::element& obj) {
    return obj.get_double().value();
}

template <typename T>
requires Like_c<EncodedUrl, T>
EncodedUrl DeserializeTo(const simdjson::dom::element& obj) {
    return EncodedUrl{obj.get_string().value()};
}

template <typename T>
requires Like_c<int64_t, T>
int64_t DeserializeTo(const simdjson::dom::element& obj) {
    return obj.get_int64().value();
}

template <typename T>
requires Like_c<Int64, T>
Int64 DeserializeTo(const simdjson::dom::element& obj) {
    return Int64{obj.get_int64().value()};
}

template <typename T>
requires Int64Bounded_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    return T{obj.get_int64().value()};
}

template <typename T>
requires Like_c<Int64Str, T>
Int64Str DeserializeTo(const simdjson::dom::element& obj) {
    const std::string_view str = obj.get_string().value();
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
requires Like_c<Name64, T>
Name64 DeserializeTo(const simdjson::dom::element& obj) {
    return Name64{obj.get_string().value()};
}

template <typename T>
requires Like_c<NameLen<64>, T>
NameLen<64> DeserializeTo(const simdjson::dom::element& obj) {
    return NameLen<64>{obj.get_string().value()};
}

template <typename T>
requires Like_c<NameLen<512>, T>
NameLen<512> DeserializeTo(const simdjson::dom::element& obj) {
    return NameLen<512>{obj.get_string().value()};
}

template <typename T>
requires Like_c<RFC3339Timestamp, T>
RFC3339Timestamp DeserializeTo(const simdjson::dom::element& obj) {
    return RFC3339Timestamp::Parse(obj.get_string().value());
}

template <typename T>
requires Like_c<std::string, T>
std::string DeserializeTo(const simdjson::dom::element& obj) {
    return std::string{obj.get_string().value()};
}

template <typename T>
requires Like_c<std::string_view, T>
std::string_view DeserializeTo(const simdjson::dom::element& obj) {
    return obj.get_string().value();
}


// Containers
template <typename T>
requires StdMap_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    using K = typename T::key_type;
    using V = typename T::mapped_type;

    return  obj.get_object() |
            std::views::transform([](auto const& kv) {
                auto const& [key, value] = kv;
                return std::pair{K{key}, DeserializeTo<V>(value)};
            }) |
            std::ranges::to<T>();
}

template <typename T>
requires Optional_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    return {DeserializeTo<typename T::value_type>(obj)};
}

template <typename T>
requires Required_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    return {DeserializeTo<typename T::value_type>(obj)};
}

template <typename T>
requires Like_c<json::Value, T>
json::Value DeserializeTo(const simdjson::dom::element& obj) {
    switch (obj.type()) {
    case simdjson::dom::element_type::NULL_VALUE:
        return {nullptr};
    case simdjson::dom::element_type::BOOL:
        return {obj.get_bool().value()};
    case simdjson::dom::element_type::UINT64:
    {
        uint64_t x = obj.get_uint64().value();
        return {
            x > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) ?
                static_cast<double>(x) :
                static_cast<int64_t>(x)
        };
    }
    case simdjson::dom::element_type::INT64:
        return {obj.get_int64().value()};
    case simdjson::dom::element_type::DOUBLE:
        return {obj.get_double().value()};
    case simdjson::dom::element_type::STRING:
        return {std::string(obj.get_string().value())};
    case simdjson::dom::element_type::ARRAY:
        return {
            obj.get_array() |
            std::views::transform([](auto const& v){ return DeserializeTo<json::Value>(v); }) |
            std::ranges::to<json::Array>()
        };
    case simdjson::dom::element_type::OBJECT:
        return {
            obj.get_object() |
            std::views::transform([](auto&& kv) {
                auto const& [key, value] = kv;
                return std::pair{std::string{key}, DeserializeTo<json::Value>(value)};
            }) |
            std::ranges::to<json::Object>()
        };
    }
    std::unreachable();
}

template <typename T>
requires ValueBox_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    return {DeserializeTo<typename T::element_type>(obj)};
}

template <typename T>
requires StdVector_c<T>
T DeserializeTo(const simdjson::dom::element& obj) {
    using V = typename T::value_type;
    return obj.get_array() |
           std::views::transform([](auto const& in) { return DeserializeTo<V>(in); }) |
           std::ranges::to<T>();
}


/***
 * Extract
 */
template <simdjson::constevalutil::fixed_string key_, typename T, auto Member>
member_type_t<T, Member> Extract(const simdjson::dom::element& obj) {
    using Member_t = member_type_t<T, Member>;

    std::string_view key{key_};

    auto result = obj.get_object().at_key(key);
    if constexpr (Optional_c<Member_t>) {
        if (result.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }
    }

    try {
        const simdjson::dom::element& elem = result.value();
        return DeserializeTo<Member_t>(elem);
    } catch (AnnotatedException& e) {
        e.AddContext(
            std::string{"Failed to extract field "} + std::string{key} +
            std::string{"\nwhile parsing "} + std::string{debug::type_name<T>()}
        );
        throw;
    } catch (const simdjson::simdjson_error& e) {
        AnnotatedException ex{
            std::string{"Failed to extract field "} + std::string{key},
            std::string{"while parsing "} + std::string{debug::type_name<T>()}
        };
        ex.AddContext(e.what());
        throw ex;
    }
}


template <simdjson::constevalutil::fixed_string key_>
std::optional<simdjson::dom::element> ExtractInPlace(const simdjson::dom::element& src) {
    std::string_view key{key_};
    auto result = src.get_object().at_key(key);
    if (result.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }
    return src;
}


template <simdjson::constevalutil::fixed_string key_>
std::optional<simdjson::dom::element> ExtractInPlaceKV(const simdjson::dom::element& src) {
    std::string_view key{key_};
    auto result = src.get_object().at_key(key);
    if (result.error() == simdjson::NO_SUCH_FIELD) { return std::nullopt; }
    return simdjson::dom::element(result.value());
}


template <typename T, auto Member>
member_type_t<T, Member> ExtractPlaceholder(const simdjson::dom::element& obj) {
    using Member_t = member_type_t<T, Member>;
    return DeserializeTo<Member_t>(obj);
}


template <typename T>
auto ExtractKind(const simdjson::dom::element &src, std::string_view field_name, std::string_view type_name) {
    auto obj = src.get_object();
    std::string_view type_sv = obj[field_name].get_string().value();
    auto opt_kind = from_string_view<T>(type_sv);
    if (!opt_kind) {
        throw AnnotatedException{
            std::string{"Unexpected "} + std::string{type_name} + std::string{" type: "} + std::string{type_sv}};
    }
    return *opt_kind;
}


}













#if 0
namespace jai::llm {
template <typename> class VariantExtractor2;

template <typename... Ts>
class VariantExtractor2<std::variant<Ts...>> {
public:
    using Variant_t = std::variant<Ts...>;

public:
    template <simdjson::constevalutil::fixed_string key>
    static Variant_t Run(const simdjson::dom::element& src) {
        try {
            constexpr bool empty_key = key.empty();
            std::string_view value{};
            if (src.is_object() && !empty_key) {
                if (auto res = src.get_object().at_key(key); res.error() == simdjson::SUCCESS) {
                    if (auto str_res = res.get_string(); str_res.error() == simdjson::SUCCESS) {
                        value = str_res.value();
                    }
                }
            }
            return RunImpl<key, Ts...>(src, value);
        } catch (AnnotatedException& e) {
            e.AddContext(
                std::string{"Failed to extract field "} + std::string{key} +
                std::string{"\nwhile parsing "} + std::string{debug::type_name<Variant_t>()});
            throw;
        } catch (const simdjson::simdjson_error& e) {
            AnnotatedException ex{
                std::string{"Failed to extract field "} + std::string{key},
                std::string{"while parsing "} + std::string{debug::type_name<Variant_t>()}
            };
            ex.AddContext(e.what());
            throw ex;
        }
    }

private:
    template <simdjson::constevalutil::fixed_string DiscriminatorField>
    static bool TestAny(const simdjson::dom::element& src) {
        std::string_view value{};
        if (src.is_object()) {
            auto res = src.get_object().at_key(DiscriminatorField.view());
            if (res.error() == simdjson::SUCCESS) {
                auto str_res = res.get_string();
                if (str_res.error() == simdjson::SUCCESS) {
                    value = str_res.value();
                }
            }
        }
        return (Test<DiscriminatorField, Ts>(src, value) || ...);
    }

    template <simdjson::constevalutil::fixed_string DiscriminatorField, typename T>
    static bool Test(const simdjson::dom::element& src, std::string_view value) {
        if constexpr (Variant_c<T>) {
            return VariantExtractor<T>::template TestAny<DiscriminatorField>(src);
        }

        auto type = src.type();

        // 1. Primitive types
        if constexpr (Like_c<bool, T>) {
            return type == simdjson::dom::element_type::BOOL;
        } if constexpr (Like_c<int64_t, T> || Like_c<uint64_t, T> || Like_c<Int64, T> ||
                        Int64Bounded_c<T> || Like_c<std::byte, T>)
        {
            return type == simdjson::dom::element_type::INT64 || type == simdjson::dom::element_type::UINT64;
        } else if constexpr (Like_c<double, T>) {
            return type == simdjson::dom::element_type::DOUBLE;
        } else if constexpr (Like_c<std::string, T> || Like_c<std::string_view, T> || Kind_c<T> || std::is_enum_v<T> ||
                            Like_c<EncodedUrl, T> || Like_c<Name64, T> || Like_c<Int64Str, T>)
        {
            if (type != simdjson::dom::element_type::STRING) { return false; }
            if constexpr (Kind_c<T> || std::is_enum_v<T>) {
                return jai::llm::from_string_view<T>(src.get_string().value()).has_value();
            }
            return true;
        }

        // 2. Arrays -> std::vector<U>
        if constexpr (StdVector_c<T>) {
            if (type != simdjson::dom::element_type::ARRAY) return false;
            auto arr = src.get_array();
            if (arr.size() == 0) return true; // Ambiguity handled in RunImpl
            // Classification is structural inference, not full validation.
            return Test<DiscriminatorField, typename T::value_type>(arr.at(0), "");
        }

        // 3. Objects
        if (type == simdjson::dom::element_type::OBJECT) {
            if (!value.empty()) {
                // Discriminator exists -> must be domain type
                if constexpr (Domain_c<T>) {
                    using KindT = field_element_t<T, DiscriminatorField>;
                    return jai::llm::from_string_view<KindT>(value).has_value();
                }
                return false;
            } else {
                // No discriminator -> must be map-like
                return StdMap_c<T> || Like_c<json::Object, T>;
            }
        }

        return false;
    }

    template <simdjson::constevalutil::fixed_string DiscriminatorField, typename T, typename... Rest>
    static Variant_t RunImpl(const simdjson::dom::element& src, std::string_view value) {
        if (Test<DiscriminatorField, T>(src, value)) {
            if constexpr (StdVector_c<T>) {
                if (src.get_array().size() == 0) {
                    if constexpr (((StdVector_c<Ts> ? 1 : 0) + ...) > 1) {
                        throw jai::llm::AnnotatedException{
                            std::string{"Ambiguous empty array for variant "} +
                            std::string{debug::type_name<Variant_t>()}
                        };
                    }
                }
            }
            return DeserializeTo<T>(src);
        }

        if constexpr (sizeof...(Rest) == 0) {
            throw jai::llm::AnnotatedException{
                std::string{"Failed to extract variant "} + std::string{debug::type_name<Variant_t>()},
                std::string{"Unknown kind provided: "} + std::string{value}
            };
        } else {
            return RunImpl<DiscriminatorField, Rest...>(src, value);
        }
    }
};
}
#endif
