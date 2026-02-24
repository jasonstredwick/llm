# Coding Conventions

Rules and patterns for all code in the `jai::llm` library. These apply strictly to the protocol
translation layer (`interface/protocols/...`) and generally to the rest of the codebase.

## Rule Zero: No Naked Types

Every member variable in a protocol struct must be wrapped. Never use raw types (e.g., `int64_t x;`)
as they lead to uninitialized "garbage" values. The struct must not be constructible in an invalid
state.

## Field Wrapping Rules

### Request Structs (Sent to Provider APIs)

**Spec-required fields** are wrapped in `Required<T>`:
```cpp
Required<std::string> model;
Required<std::vector<MessageParam>> messages;
```
`Required<T>` is a non-default-constructible wrapper that forces explicit initialization. The
struct cannot be aggregate-initialized without providing these values.

**Spec-optional fields** are wrapped in `std::optional<T>` and value-initialized with `{}`:
```cpp
std::optional<double> temperature{};
std::optional<int64_t> max_tokens{};
```
Optional fields default to disengaged. The serializer omits disengaged optionals entirely,
letting the server apply its own defaults. We never send explicit `null` for optional request
fields — omission is the default.

### Response Structs (Received from Provider APIs)

**Spec-required fields** use `Required<T>`:
```cpp
Required<std::string> id;
Required<StopReason> stop_reason;
```
This tells the deserializer the field must be present in the JSON. A missing required field
during deserialization throws an `AnnotatedException`.

**Spec-optional fields** use `std::optional<T>` initialized with `{}`:
```cpp
std::optional<std::string> stop_sequence{};
```
Even if the spec defines a default value, we use `std::optional` to handle the "missing" state
safely during parsing. The deserializer returns `std::nullopt` for absent optional fields.

### Kind / Type Discriminator Fields

Always declared as:
```cpp
Required<KindTextBlock> type{{}};
```
The `Required` wrapper ensures the field is part of the layout. The `{{}}` value-initializes
it to the correct static value defined in the Kind struct. These are structural constants — they
identify which variant alternative a JSON object represents.

## Kind Tag Structs

Kind tags carry compile-time string constants for JSON discriminator values:

```cpp
struct KindTextBlock : Kind { static constexpr std::string_view value = "text"; };
struct KindToolUseBlock : Kind { static constexpr std::string_view value = "tool_use"; };
```

All Kind structs inherit from `Kind` (an empty base class) which enables the `Kind_c` concept
for constrained template dispatch.

The serializer emits `Kind::value` as the JSON string. The deserializer matches incoming strings
against `Kind::value` via `from_string_view<T>()`.

## Enum Conversions

Every enum class has a pair of conversion functions in the corresponding `strings.hpp`:

```cpp
// In interface/protocols/<provider>/strings.hpp
constexpr std::string_view to_string_view(Role val);
template <> constexpr std::optional<Role> from_string_view<Role>(std::string_view sv);
```

`to_string_view` converts an enum value to the wire-format string.
`from_string_view<T>` converts a wire-format string to an enum value, returning `std::nullopt`
on unrecognized input.

Both are `constexpr`. The base templates are declared in `interface/core/strings_base.hpp`.

## Domain Primitive Types

The library defines several validated wrapper types in `interface/core/types.hpp`. These enforce
invariants at construction time and provide consistent interfaces.

| Type                       | Purpose                                    | Validation                              |
|----------------------------|--------------------------------------------|-----------------------------------------|
| `EncodedUrl`               | ASCII-only, percent-encoded URL            | Non-empty, ASCII-only, no whitespace/NUL/newlines |
| `Name64`                   | Identifier (a-zA-Z_-), max 64 chars        | Non-empty, ≤64 chars, alpha + `_-` only |
| `NameLen<N>`               | Identifier with custom max length          | Non-empty, ≤N chars                     |
| `Int64`                    | Wrapped int64_t                            | None (just wrapping)                    |
| `Int64Bounded<Lo, Hi>`     | Bounded integer                            | `Lo ≤ value ≤ Hi`                       |
| `Int64Str`                 | Integer serialized as a JSON string        | None (format concern)                   |
| `Timestamp`                | `system_clock::time_point` at nanosecond resolution | None                            |
| `RFC3339Timestamp`         | Timestamp parsed from/serialized to RFC3339 | Format validation on parse             |
| `ValueBox<T>`              | Heap-allocated value semantics (for recursive types) | Non-null                       |
| `Required<T>`              | Non-optional field wrapper                 | Must be initialized                     |

All domain types expose `Get()` and `Value()` accessors with consistent semantics. Most support
`operator<=>` for comparison.

`ValueBox<T>` exists specifically to break recursive type cycles (e.g., `Schema` containing
`optional<ValueBox<Schema>>`). It provides deep-copy semantics via `unique_ptr` while presenting
value-type behavior.

## JSON Schema Types

`interface/core/types.hpp` defines a minimal JSON value model used for freeform JSON fields
(e.g., tool `input`, schema `properties`):

```cpp
namespace json {
    using Array  = std::vector<Value>;
    using Object = std::map<std::string, Value>;
    using ValueVariant = std::variant<nullptr_t, bool, int64_t, double, std::string, Array, Object>;
    struct Value { ValueVariant data; /* implicit constructors */ };
}
```

This is used wherever an API field accepts arbitrary JSON.

## Variant Usage

Variants model tagged unions in the API specs (e.g., "this field is one of TextBlock, ImageBlock,
or ToolUseBlock"). The variant alternatives are discriminated by the Kind/type field in the JSON.

Variant types are aliased with `using`:
```cpp
using ContentBlockParam = std::variant<TextBlockParam, ImageBlockParam, ...>;
```

The `overloaded` helper enables clean visitation:
```cpp
std::visit(overloaded{
    [](TextBlockParam const& t) { ... },
    [](ImageBlockParam const& i) { ... }
}, block);
```

## File Organization

### Protocol Files

Each provider's protocol is organized as:

```
interface/protocols/<provider>/
    <endpoint>.hpp       # Data structures (request + response)
    strings.hpp          # to_string_view / from_string_view specializations

src/protocols/serialize/
    <provider>_<endpoint>.cpp    # SerializeFrom overloads

src/protocols/deserialize/
    <provider>_<endpoint>.cpp    # DeserializeTo specializations
```

For OpenAI, the structures are further split:
```
responses_enums.hpp     # Enums and Kind structs
responses_common.hpp    # Shared substructures
responses_req.hpp       # Request-specific types
responses_res.hpp       # Response-specific types
responses.hpp           # Top-level Request and Response aggregation
```

### Header vs Implementation

Public types and their declarations live in `interface/`. Implementation details (serialization,
transport, memory management) live in `src/`. The `interface/` directory is the library's API
surface. Users include from `interface/`; they never include from `src/`.

## Naming Conventions

**Namespaces:** lowercase, period-separated where nested: `jai::llm`, `jai::llm::anthropic`,
`jai::llm::curl`, `jai::llm::http`, `jai::llm::projection::text`.

**Types:** PascalCase for structs, classes, and enum classes: `ContentBlockParam`, `AttemptPolicy`,
`StopReason`.

**Enum values:** UPPER_SNAKE_CASE: `END_TURN`, `MAX_TOKENS`, `IMAGE_JPEG`.

**Member variables:** snake_case, matching the JSON field name where applicable: `stop_reason`,
`max_tokens`, `tool_use_id`. Gemini uses camelCase to match its API: `mimeType`, `thinkingBudget`.

**Methods:** PascalCase: `Get()`, `Value()`, `IsValid()`, `ExecOnce()`.

**Free functions:** PascalCase for public APIs, snake_case for conversion functions:
`to_string_view()`, `from_string_view<T>()`.

**Template parameters:** PascalCase for types, UPPER_SNAKE_CASE for non-type parameters:
`typename T`, `int64_t N_lower`.

**Private detail namespaces:** `_detail`.

## Error Handling

The library uses `AnnotatedException` (in `interface/core/error.hpp`), which extends
`std::exception` with a stack of `Context` entries. Each context entry captures a message and a
`std::source_location`.

Pattern for adding context during error propagation:
```cpp
try {
    // operation
} catch (AnnotatedException& e) {
    e.AddContext("additional context about what was happening");
    throw;
}
```

The `to_string()` overloads format exception chains with file/line/function information, using
path normalization to show only project-relative paths.

## Initialization

Every variable must be initialized at the point of declaration. No exceptions.

For types with a natural default, always initialize explicitly with `{}` or `= value`:
```cpp
std::optional<double> temperature{};
std::vector<Part> parts{};
int64_t count = 0;
```

This applies even to types where the compiler would zero-initialize or call a default
constructor. Write `std::vector<T> v{};` not `std::vector<T> v;`. The `{}` makes the
intent unambiguous and prevents accidental omission on types where it matters.

For types that do not support default construction, or where the value depends on branching
logic, use one of these patterns:

**Ternary** — when the choice is binary and expressions are short:
```cpp
auto effort = condition ? Effort::HIGH : Effort::LOW;
```

**Immediately invoked lambda** — when the logic involves switches, multiple branches, or
differentiation guards:
```cpp
auto effort = [&]() -> Effort {
    switch (input) {
        case A: return Effort::LOW;
        case B: return Effort::HIGH;
        default: throw AnnotatedException{"..."};
    }
}();
```

The IIFE pattern is preferred over declare-then-assign because it eliminates the window where
a variable exists in an indeterminate state, and it pairs naturally with `auto` and `const`.

## Language Standard

Target C++26 where compiler support exists, C++23 as the primary fallback, C++20 as the floor.
When a feature is not available on a target compiler, use feature-test macros (e.g.,
`__cpp_lib_ranges`, `__cpp_lib_spanstream`) to select between implementations. Never
`#ifdef` on compiler identity — test for the feature, not the vendor.

## Loops and Algorithms

Prefer `std::ranges` and `std::views` over raw loops. A `for` loop is acceptable only when
ranges cannot express the operation (e.g., coroutine yields, complex stateful iteration with
early exit and side effects).

```cpp
// Preferred
auto parts = blocks | std::views::transform(f) | std::ranges::to<std::vector>();

// Acceptable only when ranges cannot express the operation
for (auto const& item : collection) { ... }
```

`std::ranges::to<Container>()` (C++23) is the standard way to materialize a view into a
container.

## Designated Initialization

Prefer designated initializers when constructing protocol structs and any aggregate with more
than two fields. Fields with default member initializers (e.g., `Required<TypeKind> type{{}}`,
`std::optional<T> field{}`) may be omitted — the compiler applies the default. Designators
must appear in declaration order.

```cpp
return Request{
    .max_tokens=static_cast<double>(*tokens),
    .messages=std::vector<MessageParam>{ msg },
    .model=""
    // temperature, top_k, top_p omitted — default to nullopt
};
```

For `std::optional` fields, omission means "let the server decide." Never send an explicit
default when omission achieves the same result.

## Type Conversions

Use `static_cast` when the projection's types differ from the protocol spec's literal types
(e.g., projection uses `int64_t` for token counts, but a spec defines the field as `double`).
Always cast at the point of construction, not earlier:

```cpp
.max_tokens=static_cast<double>(*options.max_output_tokens)
```

For `std::optional` fields that need a cast, use the ternary pattern:
```cpp
.max_output_tokens=options.max_output_tokens ?
    std::optional{static_cast<double>(*options.max_output_tokens)} : std::nullopt
```

## Scoped `using` Aliases

Prefer fully qualified type paths over `using` aliases. Aliases are permitted only when the
fully qualified path is genuinely long (3+ nesting levels) **and** the alias name is specific
enough to be unambiguous in context.

At broad scope (function level, class level), aliases should use the original type name:
```cpp
// Good — long path, specific name, at function scope
using EasyInputMessage = openai::Request::EasyInputMessage;

// Bad — too generic at function scope, hides what the type actually is
using Msg = openai::Request::EasyInputMessage;
```

At narrow scope (deep inside nested blocks, close to point of use), shorter or abbreviated
names are acceptable because the limited visibility makes them unambiguous:
```cpp
if (options.thinking_effort) {
    using Effort = openai::Request::Reasoning::Effort;
    // Effort is fine here — small scope, immediately adjacent to use
}
```

A `using` alias declared inside a scope is visible only within that scope and its descendants.
Scope aliases as tightly as practical — if an alias is only needed inside a single block,
declare it there.

## Braces

All control flow bodies must use curly braces, even single-statement bodies. No naked
statements after `if`, `else`, `for`, `while`, `do`, `switch`, or `case`:

```cpp
// Correct
if (condition) {
    doSomething();
}

// Wrong — naked body
if (condition)
    doSomething();
```

The only exception is a `case` label followed by a single `return` on the same line inside
an IIFE switch, where the pattern is visually unambiguous:
```cpp
case ThinkingEffort::LOW: return Effort::LOW;
```

## Concepts

The library defines several concepts in `interface/core/types.hpp` and `src/concepts.hpp`:

| Concept              | Purpose                                             |
|----------------------|-----------------------------------------------------|
| `Kind_c<T>`          | Type inherits from `Kind`                           |
| `Required_c<T>`      | Type is a `Required<U>` specialization              |
| `Optional_c<T>`      | Type is a `std::optional<U>` specialization         |
| `Variant_c<T>`       | Type is a `std::variant<...>` specialization        |
| `StdVector_c<T>`     | Type is a `std::vector<U>` specialization           |
| `StdMap_c<T>`        | Type is a `std::map<K,V>` specialization            |
| `Int64Bounded_c<T>`  | Type has `LowerBound`, `UpperBound`, `IsValid`, `Value` |
| `ValueBox_c<T>`      | Type has `element_type`, `Get`, `Value`, `operator->` |
| `Like_c<TARGET, T>`  | `remove_cvref_t<T>` is same as `TARGET`             |
| `AssocContainer_c<T>` | Type has `key_type`, `mapped_type`, `value_type`    |

These concepts are used extensively in the serialization/deserialization framework for constrained
template dispatch.
