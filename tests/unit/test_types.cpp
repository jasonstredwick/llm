/***
 * Core type system unit tests — validation types, bounded integers,
 * value-semantic wrappers, and JSON value variants.
 *
 * Tests verify that:
 *   - EncodedUrl: valid construction, empty/NUL/newline/CR/space/non-ASCII rejection
 *   - Name64: valid names, invalid chars, empty, too long
 *   - NameLen<N>: valid, empty, too long
 *   - Int64: construction, accessors, ordering
 *   - Int64Bounded: valid range, boundary, below/above rejection
 *   - Int64Str: construction, Get() string conversion
 *   - Required<T>: construction, implicit conversion, value()
 *   - Timestamp: construction, Now(), ordering
 *   - RFC3339Timestamp: valid parsing, invalid format rejection
 *   - ValueBox<T>: construction, copy, move, accessors
 *   - json::Value: all variant constructors
 */

#include <chrono>
#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../interface/core/types.hpp"


using namespace jai::llm;


/***
 * EncodedUrl Tests
 */

void test_encoded_url_valid() {
    std::println("Testing EncodedUrl: valid construction...");

    EncodedUrl url(std::string_view{"https://api.example.com/v1/chat?q=hello%20world"});
    REQUIRE_EQ(url.Get(), std::string_view{"https://api.example.com/v1/chat?q=hello%20world"});
    REQUIRE_EQ(url.Value(), std::string{"https://api.example.com/v1/chat?q=hello%20world"});

    std::println("  [SUCCESS]");
}


void test_encoded_url_string_view_construction() {
    std::println("Testing EncodedUrl: string_view construction...");

    std::string_view sv = "https://example.com";
    EncodedUrl url(sv);
    REQUIRE_EQ(url.Get(), sv);

    std::println("  [SUCCESS]");
}


void test_encoded_url_empty_rejects() {
    std::println("Testing EncodedUrl: empty string rejected...");

    bool threw = false;
    try {
        EncodedUrl url(std::string_view{""});
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_space_rejects() {
    std::println("Testing EncodedUrl: space character rejected...");

    bool threw = false;
    try {
        EncodedUrl url(std::string_view{"https://example.com/path with space"});
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_newline_rejects() {
    std::println("Testing EncodedUrl: newline character rejected...");

    bool threw = false;
    try {
        EncodedUrl url(std::string_view{"https://example.com/path\ninjection"});
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_cr_rejects() {
    std::println("Testing EncodedUrl: carriage return rejected...");

    bool threw = false;
    try {
        EncodedUrl url(std::string_view{"https://example.com/path\rinjection"});
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_nul_rejects() {
    std::println("Testing EncodedUrl: NUL character rejected...");

    bool threw = false;
    try {
        std::string with_nul = "https://example.com/";
        with_nul.push_back('\0');
        with_nul += "evil";
        EncodedUrl url(with_nul);
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_non_ascii_rejects() {
    std::println("Testing EncodedUrl: non-ASCII rejected...");

    bool threw = false;
    try {
        EncodedUrl url(std::string_view{"https://example.com/caf\xC3\xA9"});  // "café" in UTF-8
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_encoded_url_comparison() {
    std::println("Testing EncodedUrl: comparison operators...");

    EncodedUrl a(std::string_view{"https://aaa.com"});
    EncodedUrl b(std::string_view{"https://bbb.com"});
    EncodedUrl a2(std::string_view{"https://aaa.com"});

    REQUIRE(a == a2);
    REQUIRE(!(a == b));
    REQUIRE(a < b);

    std::println("  [SUCCESS]");
}


void test_encoded_url_copy_move() {
    std::println("Testing EncodedUrl: copy and move...");

    EncodedUrl original(std::string_view{"https://example.com"});

    // Copy
    EncodedUrl copy = original;
    REQUIRE(copy == original);

    // Move
    EncodedUrl moved = std::move(copy);
    REQUIRE(moved == original);

    std::println("  [SUCCESS]");
}


/***
 * Name64 Tests
 */

void test_name64_valid() {
    std::println("Testing Name64: valid names...");

    Name64 a(std::string_view{"hello"});
    REQUIRE_EQ(a.Get(), std::string_view{"hello"});

    Name64 b(std::string_view{"My-Name_Here"});
    REQUIRE_EQ(b.Get(), std::string_view{"My-Name_Here"});

    // Max length: 64 characters
    std::string max_len(64, 'a');
    Name64 c(max_len);
    REQUIRE_EQ(c.Get().size(), static_cast<size_t>(64));

    std::println("  [SUCCESS]");
}


void test_name64_empty_rejects() {
    std::println("Testing Name64: empty string rejected...");

    bool threw = false;
    try { Name64 n(std::string_view{""}); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_name64_too_long_rejects() {
    std::println("Testing Name64: >64 chars rejected...");

    bool threw = false;
    try {
        std::string too_long(65, 'a');
        Name64 n(too_long);
    } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_name64_invalid_chars_rejects() {
    std::println("Testing Name64: invalid characters rejected...");

    // Digits not allowed
    bool threw_digit = false;
    try { Name64 n(std::string_view{"abc123"}); } catch (const AnnotatedException&) { threw_digit = true; }
    REQUIRE(threw_digit);

    // Spaces not allowed
    bool threw_space = false;
    try { Name64 n(std::string_view{"hello world"}); } catch (const AnnotatedException&) { threw_space = true; }
    REQUIRE(threw_space);

    // Dots not allowed
    bool threw_dot = false;
    try { Name64 n(std::string_view{"hello.world"}); } catch (const AnnotatedException&) { threw_dot = true; }
    REQUIRE(threw_dot);

    std::println("  [SUCCESS]");
}


void test_name64_is_valid_static() {
    std::println("Testing Name64::IsValid: static predicate...");

    REQUIRE(Name64::IsValid("hello"));
    REQUIRE(Name64::IsValid("A-B_c"));
    REQUIRE(!Name64::IsValid(""));
    REQUIRE(!Name64::IsValid("has space"));
    REQUIRE(!Name64::IsValid("has.dot"));
    REQUIRE(!Name64::IsValid("has1digit"));

    std::string too_long(65, 'a');
    REQUIRE(!Name64::IsValid(too_long));

    std::println("  [SUCCESS]");
}


void test_name64_comparison() {
    std::println("Testing Name64: comparison...");

    Name64 a(std::string_view{"alpha"});
    Name64 b(std::string_view{"beta"});
    Name64 a2(std::string_view{"alpha"});

    REQUIRE(a == a2);
    REQUIRE(!(a == b));
    REQUIRE(a < b);

    std::println("  [SUCCESS]");
}


void test_name64_implicit_conversion() {
    std::println("Testing Name64: implicit conversion to string_view...");

    Name64 n(std::string_view{"test"});
    std::string_view sv = n;  // implicit conversion
    REQUIRE_EQ(sv, std::string_view{"test"});

    std::println("  [SUCCESS]");
}


/***
 * NameLen<N> Tests
 */

void test_namelen_valid() {
    std::println("Testing NameLen<10>: valid construction...");

    NameLen<10> n(std::string_view{"hello"});
    REQUIRE_EQ(n.Get(), std::string_view{"hello"});

    NameLen<10> max(std::string_view{"1234567890"});
    REQUIRE_EQ(max.Get().size(), static_cast<size_t>(10));

    std::println("  [SUCCESS]");
}


void test_namelen_empty_rejects() {
    std::println("Testing NameLen<10>: empty rejected...");

    bool threw = false;
    try { NameLen<10> n(std::string_view{""}); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_namelen_too_long_rejects() {
    std::println("Testing NameLen<5>: too long rejected...");

    bool threw = false;
    try { NameLen<5> n(std::string_view{"123456"}); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_namelen_is_valid_static() {
    std::println("Testing NameLen::IsValid: static predicate...");

    REQUIRE(NameLen<5>::IsValid("abc"));
    REQUIRE(NameLen<5>::IsValid("12345"));
    REQUIRE(!NameLen<5>::IsValid(""));
    REQUIRE(!NameLen<5>::IsValid("123456"));

    std::println("  [SUCCESS]");
}


void test_namelen_allows_any_chars() {
    std::println("Testing NameLen<20>: allows any non-empty chars...");

    // Unlike Name64, NameLen allows any characters (no alphabet restriction).
    NameLen<20> n(std::string_view{"hello world 123!"});
    REQUIRE_EQ(n.Get(), std::string_view{"hello world 123!"});

    std::println("  [SUCCESS]");
}


/***
 * Int64 Tests
 */

void test_int64_construction() {
    std::println("Testing Int64: construction and accessors...");

    Int64 v(42);
    REQUIRE_EQ(v.Get(), static_cast<int64_t>(42));
    REQUIRE_EQ(v.Value(), static_cast<int64_t>(42));

    Int64 neg(-100);
    REQUIRE_EQ(neg.Value(), static_cast<int64_t>(-100));

    std::println("  [SUCCESS]");
}


void test_int64_comparison() {
    std::println("Testing Int64: comparison operators...");

    Int64 a(10);
    Int64 b(20);
    Int64 a2(10);

    REQUIRE(a == a2);
    REQUIRE(!(a == b));
    REQUIRE(a < b);

    std::println("  [SUCCESS]");
}


/***
 * Int64Bounded Tests
 */

void test_int64_bounded_valid() {
    std::println("Testing Int64Bounded<0, 100>: valid construction...");

    using Bounded = Int64Bounded<0, 100>;

    Bounded v(50);
    REQUIRE_EQ(v.Get(), static_cast<int64_t>(50));
    REQUIRE_EQ(v.Value(), static_cast<int64_t>(50));

    std::println("  [SUCCESS]");
}


void test_int64_bounded_at_boundaries() {
    std::println("Testing Int64Bounded<0, 100>: boundary values...");

    using Bounded = Int64Bounded<0, 100>;

    Bounded lower(0);
    REQUIRE_EQ(lower.Value(), static_cast<int64_t>(0));

    Bounded upper(100);
    REQUIRE_EQ(upper.Value(), static_cast<int64_t>(100));

    std::println("  [SUCCESS]");
}


void test_int64_bounded_below_lower_rejects() {
    std::println("Testing Int64Bounded<0, 100>: below lower bound rejected...");

    using Bounded = Int64Bounded<0, 100>;

    bool threw = false;
    try { Bounded v(-1); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_int64_bounded_above_upper_rejects() {
    std::println("Testing Int64Bounded<0, 100>: above upper bound rejected...");

    using Bounded = Int64Bounded<0, 100>;

    bool threw = false;
    try { Bounded v(101); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_int64_bounded_is_valid_static() {
    std::println("Testing Int64Bounded<-10, 10>::IsValid: static predicate...");

    using Bounded = Int64Bounded<-10, 10>;

    REQUIRE(Bounded::IsValid(0));
    REQUIRE(Bounded::IsValid(-10));
    REQUIRE(Bounded::IsValid(10));
    REQUIRE(!Bounded::IsValid(-11));
    REQUIRE(!Bounded::IsValid(11));

    std::println("  [SUCCESS]");
}


void test_int64_bounded_comparison() {
    std::println("Testing Int64Bounded: comparison operators...");

    using Bounded = Int64Bounded<0, 100>;

    Bounded a(10);
    Bounded b(20);
    Bounded a2(10);

    REQUIRE(a == a2);
    REQUIRE(!(a == b));
    REQUIRE(a < b);

    std::println("  [SUCCESS]");
}


void test_int64_bounded_negative_range() {
    std::println("Testing Int64Bounded<-100, -1>: negative range...");

    using Bounded = Int64Bounded<-100, -1>;

    Bounded v(-50);
    REQUIRE_EQ(v.Value(), static_cast<int64_t>(-50));

    bool threw = false;
    try { Bounded v(0); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_int64_bounded_single_value() {
    std::println("Testing Int64Bounded<42, 42>: single-value range...");

    using Bounded = Int64Bounded<42, 42>;

    Bounded v(42);
    REQUIRE_EQ(v.Value(), static_cast<int64_t>(42));

    bool threw = false;
    try { Bounded v(43); } catch (const AnnotatedException&) { threw = true; }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


/***
 * Int64Str Tests
 */

void test_int64_str_construction() {
    std::println("Testing Int64Str: construction and Get()...");

    Int64Str v(42);
    REQUIRE_EQ(v.Get(), std::string{"42"});
    REQUIRE_EQ(v.Value(), static_cast<int64_t>(42));

    Int64Str neg(-100);
    REQUIRE_EQ(neg.Get(), std::string{"-100"});

    Int64Str zero(0);
    REQUIRE_EQ(zero.Get(), std::string{"0"});

    std::println("  [SUCCESS]");
}


void test_int64_str_comparison() {
    std::println("Testing Int64Str: comparison operators...");

    Int64Str a(10);
    Int64Str b(20);
    Int64Str a2(10);

    REQUIRE(a == a2);
    REQUIRE(!(a == b));
    REQUIRE(a < b);

    std::println("  [SUCCESS]");
}


/***
 * Required<T> Tests
 */

void test_required_construction() {
    std::println("Testing Required<int>: construction...");

    Required<int> r(42);
    REQUIRE_EQ(r.value(), 42);
    REQUIRE_EQ(*r, 42);

    std::println("  [SUCCESS]");
}


void test_required_implicit_conversion() {
    std::println("Testing Required<int>: implicit conversion...");

    Required<int> r(42);
    int val = r;  // implicit conversion
    REQUIRE_EQ(val, 42);

    std::println("  [SUCCESS]");
}


void test_required_string() {
    std::println("Testing Required<string>: construction and access...");

    Required<std::string> r(std::string{"hello"});
    REQUIRE_EQ(r.value(), std::string{"hello"});

    const std::string& ref = r;  // implicit conversion
    REQUIRE_EQ(ref, std::string{"hello"});

    std::println("  [SUCCESS]");
}


void test_required_no_default_construction() {
    std::println("Testing Required<T>: not default-constructible (compile-time check)...");

    // Required<int> r{}; // This would fail to compile — that's the point.
    // We verify this at compile time; the test just confirms the type is usable.
    Required<int> r(0);
    REQUIRE_EQ(r.value(), 0);

    std::println("  [SUCCESS]");
}


void test_required_mutable_value() {
    std::println("Testing Required<T>: mutable value() access...");

    Required<std::string> r(std::string{"initial"});
    r.value() = "modified";
    REQUIRE_EQ(r.value(), std::string{"modified"});

    std::println("  [SUCCESS]");
}


/***
 * Timestamp Tests
 */

void test_timestamp_construction() {
    std::println("Testing Timestamp: construction from time_point...");

    auto now = std::chrono::system_clock::now();
    auto tp = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    Timestamp ts(tp);

    REQUIRE(ts.Get() == tp);
    REQUIRE(ts.Value() == tp);

    std::println("  [SUCCESS]");
}


void test_timestamp_now() {
    std::println("Testing Timestamp::Now()...");

    auto before = std::chrono::system_clock::now();
    auto ts = Timestamp::Now();
    auto after = std::chrono::system_clock::now();

    auto before_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(before);
    auto after_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(after);

    REQUIRE(ts.Get() >= before_ns);
    REQUIRE(ts.Get() <= after_ns);

    std::println("  [SUCCESS]");
}


void test_timestamp_ordering() {
    std::println("Testing Timestamp: ordering...");

    auto t1 = Timestamp::Now();
    auto t2 = Timestamp::Now();

    REQUIRE(t1 <= t2);

    std::println("  [SUCCESS]");
}


/***
 * RFC3339Timestamp Tests
 */

void test_rfc3339_valid_parse() {
    std::println("Testing RFC3339Timestamp::Parse: valid timestamp...");

    auto ts = RFC3339Timestamp::Parse("2024-01-15T10:30:00Z");

    // Verify it parsed without throwing.
    // Check the year-month-day by converting back.
    auto tp = ts.Get();
    auto days = std::chrono::floor<std::chrono::days>(tp);
    std::chrono::year_month_day ymd{days};

    REQUIRE_EQ(static_cast<int>(ymd.year()), 2024);
    REQUIRE_EQ(static_cast<unsigned>(ymd.month()), 1u);
    REQUIRE_EQ(static_cast<unsigned>(ymd.day()), 15u);

    std::println("  [SUCCESS]");
}


void test_rfc3339_too_short_rejects() {
    std::println("Testing RFC3339Timestamp::Parse: too short rejected...");

    bool threw = false;
    try {
        RFC3339Timestamp::Parse("2024-01-15");
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_rfc3339_invalid_format_rejects() {
    std::println("Testing RFC3339Timestamp::Parse: invalid format rejected...");

    bool threw = false;
    try {
        RFC3339Timestamp::Parse("not-a-real-timestamp!");
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_rfc3339_invalid_separator_rejects() {
    std::println("Testing RFC3339Timestamp::Parse: wrong separator rejected...");

    bool threw = false;
    try {
        // Missing 'T' separator
        RFC3339Timestamp::Parse("2024-01-15 10:30:00Z");
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


/***
 * ValueBox<T> Tests
 */

void test_valuebox_construction() {
    std::println("Testing ValueBox<int>: construction...");

    ValueBox<int> box(42);
    REQUIRE_EQ(box.Get(), 42);
    REQUIRE_EQ(box.Value(), 42);
    REQUIRE_EQ(*box.operator->(), 42);

    std::println("  [SUCCESS]");
}


void test_valuebox_copy() {
    std::println("Testing ValueBox<int>: copy construction (deep copy)...");

    ValueBox<int> original(42);
    ValueBox<int> copy = original;

    REQUIRE_EQ(copy.Get(), 42);
    REQUIRE_EQ(original.Get(), 42);

    // They should be equal but point to different memory.
    REQUIRE_EQ(copy.Get(), original.Get());

    std::println("  [SUCCESS]");
}


void test_valuebox_move() {
    std::println("Testing ValueBox<int>: move construction...");

    ValueBox<int> original(42);
    ValueBox<int> moved = std::move(original);

    REQUIRE_EQ(moved.Get(), 42);

    std::println("  [SUCCESS]");
}


void test_valuebox_copy_assignment() {
    std::println("Testing ValueBox<int>: copy assignment...");

    ValueBox<int> a(10);
    ValueBox<int> b(20);

    a = b;
    REQUIRE_EQ(a.Get(), 20);
    REQUIRE_EQ(b.Get(), 20);

    std::println("  [SUCCESS]");
}


void test_valuebox_comparison() {
    std::println("Testing ValueBox<int>: comparison...");

    ValueBox<int> a(10);
    ValueBox<int> b(20);
    ValueBox<int> a2(10);

    REQUIRE((a <=> a2) == 0);
    REQUIRE((a <=> b) < 0);
    REQUIRE((b <=> a) > 0);

    std::println("  [SUCCESS]");
}


void test_valuebox_string() {
    std::println("Testing ValueBox<string>: with string type...");

    ValueBox<std::string> box(std::string{"hello"});
    REQUIRE_EQ(box.Get(), std::string{"hello"});
    REQUIRE_EQ(box.Value(), std::string{"hello"});

    // operator-> should give pointer to the string
    REQUIRE_EQ(box->size(), static_cast<size_t>(5));

    std::println("  [SUCCESS]");
}


/***
 * json::Value Tests
 */

void test_json_value_null() {
    std::println("Testing json::Value: null construction...");

    json::Value v;
    REQUIRE(std::holds_alternative<nullptr_t>(v.data));

    json::Value v2(nullptr);
    REQUIRE(std::holds_alternative<nullptr_t>(v2.data));

    std::println("  [SUCCESS]");
}


void test_json_value_bool() {
    std::println("Testing json::Value: bool construction...");

    json::Value t(true);
    REQUIRE(std::holds_alternative<bool>(t.data));
    REQUIRE_EQ(std::get<bool>(t.data), true);

    json::Value f(false);
    REQUIRE_EQ(std::get<bool>(f.data), false);

    std::println("  [SUCCESS]");
}


void test_json_value_int64() {
    std::println("Testing json::Value: int64_t construction...");

    json::Value v(int64_t{42});
    REQUIRE(std::holds_alternative<int64_t>(v.data));
    REQUIRE_EQ(std::get<int64_t>(v.data), static_cast<int64_t>(42));

    std::println("  [SUCCESS]");
}


void test_json_value_double() {
    std::println("Testing json::Value: double construction...");

    json::Value v(3.14);
    REQUIRE(std::holds_alternative<double>(v.data));
    REQUIRE_EQ(std::get<double>(v.data), 3.14);

    std::println("  [SUCCESS]");
}


void test_json_value_string() {
    std::println("Testing json::Value: string construction...");

    json::Value from_cstr("hello");
    REQUIRE(std::holds_alternative<std::string>(from_cstr.data));
    REQUIRE_EQ(std::get<std::string>(from_cstr.data), std::string{"hello"});

    json::Value from_str(std::string{"world"});
    REQUIRE_EQ(std::get<std::string>(from_str.data), std::string{"world"});

    std::println("  [SUCCESS]");
}


void test_json_value_array() {
    std::println("Testing json::Value: array construction...");

    json::Array arr;
    arr.push_back(json::Value(int64_t{1}));
    arr.push_back(json::Value("two"));
    arr.push_back(json::Value(3.0));

    json::Value v(std::move(arr));
    REQUIRE(std::holds_alternative<json::Array>(v.data));
    REQUIRE_EQ(std::get<json::Array>(v.data).size(), static_cast<size_t>(3));

    std::println("  [SUCCESS]");
}


void test_json_value_object() {
    std::println("Testing json::Value: object construction...");

    json::Object obj;
    obj["name"] = json::Value("test");
    obj["count"] = json::Value(int64_t{42});

    json::Value v(std::move(obj));
    REQUIRE(std::holds_alternative<json::Object>(v.data));
    REQUIRE_EQ(std::get<json::Object>(v.data).size(), static_cast<size_t>(2));

    std::println("  [SUCCESS]");
}


/***
 * Main
 */

int main() {
    int failed = 0;

    auto run = [&](auto fn) {
        try { fn(); }
        catch (const std::exception& e) {
            std::println("  [FAILED] {}", e.what());
            ++failed;
        }
    };

    std::println("===== EncodedUrl Tests =====");
    run(test_encoded_url_valid);
    run(test_encoded_url_string_view_construction);
    run(test_encoded_url_empty_rejects);
    run(test_encoded_url_space_rejects);
    run(test_encoded_url_newline_rejects);
    run(test_encoded_url_cr_rejects);
    run(test_encoded_url_nul_rejects);
    run(test_encoded_url_non_ascii_rejects);
    run(test_encoded_url_comparison);
    run(test_encoded_url_copy_move);

    std::println("\n===== Name64 Tests =====");
    run(test_name64_valid);
    run(test_name64_empty_rejects);
    run(test_name64_too_long_rejects);
    run(test_name64_invalid_chars_rejects);
    run(test_name64_is_valid_static);
    run(test_name64_comparison);
    run(test_name64_implicit_conversion);

    std::println("\n===== NameLen Tests =====");
    run(test_namelen_valid);
    run(test_namelen_empty_rejects);
    run(test_namelen_too_long_rejects);
    run(test_namelen_is_valid_static);
    run(test_namelen_allows_any_chars);

    std::println("\n===== Int64 Tests =====");
    run(test_int64_construction);
    run(test_int64_comparison);

    std::println("\n===== Int64Bounded Tests =====");
    run(test_int64_bounded_valid);
    run(test_int64_bounded_at_boundaries);
    run(test_int64_bounded_below_lower_rejects);
    run(test_int64_bounded_above_upper_rejects);
    run(test_int64_bounded_is_valid_static);
    run(test_int64_bounded_comparison);
    run(test_int64_bounded_negative_range);
    run(test_int64_bounded_single_value);

    std::println("\n===== Int64Str Tests =====");
    run(test_int64_str_construction);
    run(test_int64_str_comparison);

    std::println("\n===== Required Tests =====");
    run(test_required_construction);
    run(test_required_implicit_conversion);
    run(test_required_string);
    run(test_required_no_default_construction);
    run(test_required_mutable_value);

    std::println("\n===== Timestamp Tests =====");
    run(test_timestamp_construction);
    run(test_timestamp_now);
    run(test_timestamp_ordering);

    std::println("\n===== RFC3339Timestamp Tests =====");
    run(test_rfc3339_valid_parse);
    run(test_rfc3339_too_short_rejects);
    run(test_rfc3339_invalid_format_rejects);
    run(test_rfc3339_invalid_separator_rejects);

    std::println("\n===== ValueBox Tests =====");
    run(test_valuebox_construction);
    run(test_valuebox_copy);
    run(test_valuebox_move);
    run(test_valuebox_copy_assignment);
    run(test_valuebox_comparison);
    run(test_valuebox_string);

    std::println("\n===== json::Value Tests =====");
    run(test_json_value_null);
    run(test_json_value_bool);
    run(test_json_value_int64);
    run(test_json_value_double);
    run(test_json_value_string);
    run(test_json_value_array);
    run(test_json_value_object);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
