#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>
#include <map>
#include <memory>

#include <version> // Standard header for feature-test macros
#if defined(__cpp_lib_spanstream) && defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
#include <spanstream>
#else
#include <charconv> // Used for the zero-copy fallback parser
#endif

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "error.hpp"


namespace jai::llm {


struct Kind {};
template <typename T> class Required;


/***
 * Type traits and concepts
 */
template <typename>
inline constexpr bool always_false_v = false;


// Class/Struct member type extraction
template <typename T, auto Member>
struct member_type;

template <typename T, typename M, M T::* Member>
struct member_type<T, Member> {
    using type = M;
};

template <typename T, auto Member>
using member_type_t = typename member_type<T, Member>::type;


// Given a "Required<T>" member, extract the element type.
template <typename T, auto Member>
using field_element_t = typename std::remove_cvref_t<member_type_t<T, Member>>::value_type;


// Variant
template <typename T>
struct is_variant : std::false_type {};

template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variant_v = is_variant<T>::value;

template <typename>
struct variant_types;

template <typename... Ts>
struct variant_types<std::variant<Ts...>> {
    using type = std::tuple<Ts...>;
};

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};


// Concepts
template <typename T>
concept AssocContainer_c = requires {
    typename T::key_type;
    typename T::mapped_type;
    typename T::value_type;
};

template <typename T>
concept Int64Bounded_c =
    requires(int64_t x) {
        { T::LowerBound } -> std::convertible_to<int64_t>;
        { T::UpperBound } -> std::convertible_to<int64_t>;
        { T{x} };
        { T::IsValid(x) } -> std::same_as<bool>;
        { std::declval<const T&>().Value() } -> std::same_as<int64_t>;
    };

template <typename T>
concept Kind_c = std::is_base_of_v<Kind, T>;

template <typename TARGET, typename T>
concept Like_c = std::same_as<TARGET, std::remove_cvref_t<T>>;

template <typename T>
concept Optional_c = requires { typename T::value_type; } && std::same_as<T, std::optional<typename T::value_type>>;

template <typename T>
concept Required_c = requires { typename T::value_type; } && std::same_as<T, Required<typename T::value_type>>;

template <typename T>
concept StdMap_c = AssocContainer_c<T> &&
    std::same_as<
        std::remove_cvref_t<T>,
        std::map<typename T::key_type,
                 typename T::mapped_type,
                 typename T::key_compare,
                 typename T::allocator_type>>;

template <typename T>
concept StdVector_c =
    std::same_as<
        std::remove_cvref_t<T>,
        std::vector<typename std::remove_cvref_t<T>::value_type,
                    typename std::remove_cvref_t<T>::allocator_type>>;

template <typename T>
concept Variant_c = is_variant_v<std::remove_cvref_t<T>>;

template <typename T>
concept ValueBox_c =
    requires(T v) {
        typename T::element_type;
        { v.Get() }   -> std::same_as<const typename T::element_type&>;
        { v.Value() } -> std::same_as<const typename T::element_type&>;
        { v.operator->() } -> std::same_as<const typename T::element_type*>;
    };


/***
 * JSON schema
 */
namespace json {
    struct Value;
    using Array  = std::vector<Value>;
    using Object = std::map<std::string, Value>;

    using ValueVariant = std::variant<
        nullptr_t,   // null
        bool,        // true / false
        int64_t,     // integer numbers
        double,      // floating-point numbers
        std::string, // strings
        Array,       // arrays
        Object       // objects
    >;

    struct Value {
        ValueVariant data;

        Value()               : data(nullptr) {}
        Value(std::nullptr_t) : data(nullptr) {}
        Value(bool b)         : data(b) {}
        Value(int64_t i)      : data(i) {}
        Value(double d)       : data(d) {}
        Value(const char* s)  : data(std::string{s}) {}
        Value(std::string s)  : data(std::move(s)) {}
        Value(Array a)        : data(std::move(a)) {}
        Value(Object o)       : data(std::move(o)) {}
    };
}


/***
 * General types need to represent structures across Provider protocols where a raw value is not acceptible.
 */

 // @brief Represents a URL that is expected to be ASCII-only (already percent-encoded).
class EncodedUrl {
private:
    std::string value{};

public:
    explicit EncodedUrl(std::string v) : value{std::move(v)} { EncodedUrl::VerifyWireSafe(value); }
    explicit EncodedUrl(std::string_view v) : value{v} { EncodedUrl::VerifyWireSafe(value); }

    EncodedUrl(const EncodedUrl&) = default;
    EncodedUrl(EncodedUrl&&) noexcept = default;
    ~EncodedUrl() noexcept = default;
    EncodedUrl& operator=(const EncodedUrl&) = default;
    EncodedUrl& operator=(EncodedUrl&&) noexcept = default;

    friend auto operator<=>(const EncodedUrl&, const EncodedUrl&) = default;

    std::string_view   Get()   const { return value; }
    const std::string& Value() const { return value; }

private:
    static void VerifyWireSafe(std::string_view sv) {
        if (sv.empty()) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: empty string provided."};
        } else if (!std::ranges::all_of(sv, [](auto c) { return static_cast<unsigned char>(c) <= 127; })) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: contains non-ASCII characters."};
        } else if (sv.find('\0') != std::string_view::npos) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: contains NUL (\\0) character"};
        } else if (sv.find('\n') != std::string_view::npos) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: contains newline (\\n) character"};
        } else if (sv.find('\r') != std::string_view::npos) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: contains carriage return (\\r) character"};
        } else if (sv.find(' ') != std::string_view::npos) {
            throw AnnotatedException{"EncodedUrl must already be ASCII-encoded: contains space character"};
        }
    }
};


class Int64 {
private:
    int64_t value;

public:
    Int64(int64_t in) : value{in} {}
    Int64(const Int64&) = default;
    Int64(Int64&&) noexcept = default;
    ~Int64() noexcept = default;
    Int64& operator=(const Int64&) = default;
    Int64& operator=(Int64&&) noexcept = default;

    friend auto operator<=>(const Int64&, const Int64&) = default;

    int64_t Get()   const { return value; }
    int64_t Value() const { return value; }
};


template <int64_t N_lower, int64_t N_upper>
class Int64Bounded {
public:
    static_assert(N_lower <= N_upper, "Int64Bounded provided invalid bounds.");

    static constexpr int64_t LowerBound = N_lower;
    static constexpr int64_t UpperBound = N_upper;

private:
    int64_t value;

public:
    Int64Bounded(int64_t in) : value{in} { Validate(); }
    Int64Bounded(const Int64Bounded&) = default;
    Int64Bounded(Int64Bounded&&) noexcept = default;
    ~Int64Bounded() noexcept = default;
    Int64Bounded& operator=(const Int64Bounded&) = default;
    Int64Bounded& operator=(Int64Bounded&&) noexcept = default;

    friend auto operator<=>(const Int64Bounded&, const Int64Bounded&) = default;
    static bool IsValid(int64_t x) { return N_lower <= x && x <= N_upper; }

    int64_t Get()   const { return value; }
    int64_t Value() const { return value; }

private:
    void Validate() {
        if (!IsValid(value)) { throw AnnotatedException{"Int64Bounded provided invalid value."}; }
    }
};


class Int64Str {
private:
    int64_t value;

public:
    Int64Str(int64_t in) : value{in} {}
    Int64Str(const Int64Str&) = default;
    Int64Str(Int64Str&&) noexcept = default;
    ~Int64Str() noexcept = default;
    Int64Str& operator=(const Int64Str&) = default;
    Int64Str& operator=(Int64Str&&) noexcept = default;

    friend auto operator<=>(const Int64Str&, const Int64Str&) = default;

    std::string Get() const {
        char buf[32];
        auto it = std::format_to(buf, "{}", value);
        return std::string(buf, it - buf);
    }

    int64_t Value() const { return value; }
};


class Name64 {
private:
    std::string name;

public:
    static constexpr bool IsValid(std::string_view in) {
        return !in.empty() &&
               in.size() <= 64 &&
               std::ranges::all_of(in, [](const auto c) {
                   return ('a' <= c && c <= 'z') ||
                          ('A' <= c && c <= 'Z') ||
                          c == '_' ||
                          c == '-';});
    }

public:
    explicit Name64(std::string in)      : name{std::move(in)} { Validate(); }
    explicit Name64(std::string_view in) : name{in}            { Validate(); }
    Name64(const Name64&) = default;
    Name64(Name64&&) noexcept = default;
    ~Name64() noexcept = default;
    Name64& operator=(const Name64&) = default;
    Name64& operator=(Name64&&) noexcept = default;

    friend auto operator<=>(const Name64&, const Name64&) = default;

    std::string_view   Get()   const noexcept { return name; }
    const std::string& Value() const noexcept { return name; }
    operator std::string_view() const noexcept { return Get(); }

    void Validate() {
        if (!Name64::IsValid(name)) { throw AnnotatedException{"Name not valid: a-zA-Z_-"}; }
    }
};


template <size_t N>
class NameLen {
private:
    std::string name;

public:
    static constexpr bool IsValid(std::string_view in) { return !in.empty() && in.size() <= N; }

public:
    explicit NameLen(std::string in)      : name{std::move(in)} { Validate(); }
    explicit NameLen(std::string_view in) : name{in}            { Validate(); }
    NameLen(const NameLen&) = default;
    NameLen(NameLen&&) noexcept = default;
    ~NameLen() noexcept = default;
    NameLen& operator=(const NameLen&) = default;
    NameLen& operator=(NameLen&&) noexcept = default;

    friend auto operator<=>(const NameLen&, const NameLen&) = default;

    std::string_view   Get()   const noexcept { return name; }
    const std::string& Value() const noexcept { return name; }
    operator std::string_view() const noexcept { return Get(); }

    void Validate() {
        if (!NameLen::IsValid(name)) { throw AnnotatedException{"NameLen not valid"}; }
    }
};


template<typename T>
class Required {
public:
    using value_type = T;

private:
    value_type val;

public:
    Required() = delete;
    constexpr Required(value_type val) : val(std::move(val)) {}

    template <typename U>
    requires (!std::is_same_v<std::decay_t<U>, value_type>) && std::is_constructible_v<value_type, U>
    constexpr Required(U&& val) : val(std::forward<U>(val)) {}

    constexpr operator const value_type&() const { return val; }
    constexpr value_type& Value() { return val; }
    constexpr const value_type& Value() const { return val; }
};


class Timestamp {
public:
    using clock      = std::chrono::system_clock;
    using duration   = std::chrono::nanoseconds;
    using time_point = std::chrono::time_point<clock, duration>;

private:
    time_point tp;

public:
    explicit Timestamp(time_point in) : tp{in} {}
    Timestamp(const Timestamp&) = default;
    Timestamp(Timestamp&&) noexcept = default;
    ~Timestamp() noexcept = default;
    Timestamp& operator=(const Timestamp&) = default;
    Timestamp& operator=(Timestamp&&) noexcept = default;

    friend auto operator<=>(const Timestamp&, const Timestamp&) = default;

    time_point Get()   const { return tp; }
    time_point Value() const { return tp; }

    static Timestamp Now() {
        return Timestamp{std::chrono::time_point_cast<duration>(clock::now())};
    }
};


struct RFC3339Timestamp : public Timestamp {
    using Timestamp::Timestamp;

    static RFC3339Timestamp Parse(std::string_view sv) {
#if defined(__cpp_lib_spanstream) && defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
        std::ispanstream iss{std::span<const char>{sv.data(), sv.size()}};
        time_point tp{};
        iss >> std::chrono::parse("%FT%TZ", tp);
        if (iss.fail()) { throw AnnotatedException{"Failed to convert string to RFC3339Timestamp."}; }
        return RFC3339Timestamp{tp};
#else
        // Fallback: Zero-copy manual parser
        // Expected format: YYYY-MM-DDTHH:MM:SSZ
        if (sv.size() < 20) { throw AnnotatedException{"RFC3339Timestamp string too short."}; }

        int y, m, d, h, min, s;
        auto res = std::from_chars(sv.data(), sv.data() + 4, y);
        if (res.ec != std::errc{} || sv[4] != '-') throw AnnotatedException{"Invalid year format."};

        res = std::from_chars(sv.data() + 5, sv.data() + 7, m);
        if (res.ec != std::errc{} || sv[7] != '-') throw AnnotatedException{"Invalid month format."};

        res = std::from_chars(sv.data() + 8, sv.data() + 10, d);
        if (res.ec != std::errc{} || sv[10] != 'T') throw AnnotatedException{"Invalid day format."};

        res = std::from_chars(sv.data() + 11, sv.data() + 13, h);
        if (res.ec != std::errc{} || sv[13] != ':') throw AnnotatedException{"Invalid hour format."};

        res = std::from_chars(sv.data() + 14, sv.data() + 16, min);
        if (res.ec != std::errc{} || sv[16] != ':') throw AnnotatedException{"Invalid minute format."};

        res = std::from_chars(sv.data() + 17, sv.data() + 19, s);
        if (res.ec != std::errc{}) throw AnnotatedException{"Invalid second format."};

        // Construct using C++20 chrono (safe and zero-copy)
        auto date = std::chrono::year(y) / m / d;
        if (!date.ok()) throw AnnotatedException{"Invalid date components."};

        auto tp_fallback = std::chrono::sys_days{date} + 
                           std::chrono::hours{h} + 
                           std::chrono::minutes{min} + 
                           std::chrono::seconds{s};

        return RFC3339Timestamp{tp_fallback};
#endif
    }
};


template <typename T>
class ValueBox {
public:
    using element_type = T;

private:
    std::unique_ptr<element_type> ptr;

public:
    ValueBox() = delete;
    ValueBox(const element_type& v) : ptr(std::make_unique<element_type>(v)) {}
    ValueBox(element_type&& v)      : ptr(std::make_unique<element_type>(std::move(v))) {}
    ValueBox(const ValueBox& other) : ptr(std::make_unique<element_type>(*other.ptr)) {}
    ValueBox(ValueBox&&) noexcept = default;
    ~ValueBox() noexcept = default;

    ValueBox& operator=(const ValueBox& other) {
        if (this != &other) { *ptr = *(other.ptr); }
        return *this;
    }
    ValueBox& operator=(ValueBox&&) noexcept = default;

    friend auto operator<=>(const ValueBox& a, const ValueBox& b) { return *(a.ptr) <=> *(b.ptr); }

    const element_type& Get()   const { return *ptr; }
    const element_type& Value() const { return *ptr; }

    const element_type* operator->() const { return ptr.get(); }
};


}
