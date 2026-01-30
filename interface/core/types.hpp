#pragma once


#include <algorithm>
#include <chrono>
#include <cstdint>
#include <map>
#include <spanstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>


namespace jai::llm {

template <typename> inline constexpr bool always_false_v = false;

template <typename T, auto Member> struct member_type;
template <typename T, typename M, M T::* Member> struct member_type<T, Member> { using type = M; };
template <typename T, auto Member>
using member_type_t = typename member_type<T, Member>::type;

template <typename T>     struct is_variant                      : std::false_type {};
template <typename... Ts> struct is_variant<std::variant<Ts...>> : std::true_type {};
template <typename T> inline constexpr bool is_variant_v = is_variant<T>::value;


template <typename T>
concept Optional_c = requires { typename T::value_type; } && std::same_as<T, std::optional<typename T::value_type>>;

template <typename T>
concept Variant_c = requires { typename std::variant_size<T>::type; };

template <typename TARGET, typename T>
concept Like_c = std::same_as<TARGET, std::remove_cvref_t<T>>;

}


namespace jai::llm {


namespace json {

class Value;
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


// Special type for use as an alternative int64_t for function argument type resolution.
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

    friend bool operator<=>(const Int64&, const Int64&) = default;

    int64_t Get() const { return value; }
};


template <int64_t N_lower, int64_t N_upper>
class IntN {
private:
    static_assert(N_lower <= N_upper, "IntN provided invalid bounds.");

    int64_t value;

public:
    IntN(int64_t in) : value{in} { Validate(); }
    IntN(const IntN&) = default;
    IntN(IntN&&) noexcept = default;
    ~IntN() noexcept = default;
    IntN& operator=(const IntN&) = default;
    IntN& operator=(IntN&&) noexcept = default;

    friend bool operator<=>(const IntN&, const IntN&) = default;
    static bool IsValid(int64_t x) { return N_lower <= x && x <= N_upper; }

    int64_t Get() const { return value; }

private:
    void Validate() {
        if (!IsValid(value)) { throw std::out_of_range{"IntN provided invalid value."}; }
    }
};


class Name64 {
private:
    std::string name;

public:
    static bool IsValid(std::string_view in) {
        return !in.empty() &&
               in.size() <= 64 &&
               std::ranges::all_of(in, [](const auto c) {
                   return ('a' <= c && c <= 'z') ||
                          ('A' <= c && c <= 'Z') ||
                          c == '_' ||
                          c == '-';});
    }

public:
    Name64(const std::string& in) : name{in} { Validate(); }
    Name64(const Name64&) = default;
    Name64(Name64&&) noexcept = default;
    ~Name64() noexcept = default;
    Name64& operator=(const Name64&) = default;
    Name64& operator=(Name64&&) noexcept = default;

    friend bool operator<=>(const Name64&, const Name64&) = default;

    std::string_view Get() const { return name; }
    const std::string& Value() const { return name; }

    void Validate() {
        if (!Name64::IsValid(name)) { throw std::runtime_error{"Name not valid: a-zA-Z0-9_-"}; }
    }
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

    friend bool operator<=>(const Timestamp&, const Timestamp&) = default;

    time_point Get() const { return tp; }

    static Timestamp Now() {
        return Timestamp{std::chrono::time_point_cast<duration>(clock::now())};
    }
};


struct RFC3339Timestamp : public Timestamp {
    using Timestamp::Timestamp;

    static RFC3339Timestamp Parse(std::string_view sv) {
        std::ispanstream iss{std::span<const char>{sv.data(), sv.size()}};
        time_point tp{};
        iss >> std::chrono::parse("%FT%TZ", tp);
        if (iss.fail()) { throw std::runtime_error{"Failed to convert string to RFC3339Timestamp."}; }
        return RFC3339Timestamp{tp};
    }
};


}
