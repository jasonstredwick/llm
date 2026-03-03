# Serialization & Deserialization Framework

The library uses simdjson for both JSON serialization and deserialization. The framework is
generic — the same core machinery handles all three providers, with per-provider struct mappings
defined in separate `.cpp` files.

## File Map

```
src/protocols/serialize/
    base.hpp                        # Generic serialization framework
    anthropic_messages.cpp          # Anthropic struct → JSON
    gemini_generate_content.cpp     # Gemini struct → JSON
    openai_responses.cpp            # OpenAI struct → JSON

src/protocols/deserialize/
    base.hpp                        # Generic deserialization framework
    debug.hpp                       # Type name demangling for error messages
    anthropic_messages.cpp          # JSON → Anthropic struct
    gemini_generate_content.cpp     # JSON → Gemini struct
    openai_responses.cpp            # JSON → OpenAI struct
```

## Serialization (Struct → JSON)

### Core Function

```cpp
template <typename T>
void SerializeFrom(simdjson::builder::string_builder& builder, const T& obj);
```

The primary `SerializeFrom` template is `= delete`'d by default. Overloads are provided for all
supported types. This means attempting to serialize an unsupported type is a compile error.

### Built-in Type Overloads

The framework provides `SerializeFrom` overloads for:

**Primitive types:** `bool`, `char`, `double`, `int64_t`, `std::byte`, `std::string`,
`std::string_view`.

**Domain types:** `EncodedUrl`, `Int64`, `Int64Bounded<N1,N2>`, `Int64Str`, `Name64`,
`NameLen<N>`, `RFC3339Timestamp`.

**Kind structs and enums:** Any type satisfying `Kind_c` or `std::is_enum_v` is serialized via
`to_string_view()`, producing a quoted JSON string.

**Container types:** `std::vector<T>` (as JSON array), `std::map<K,V>` (as JSON object, requires
string-convertible keys).

**Wrapper types:** `Required<T>` unwraps and serializes the inner value. `std::optional<T>`
serializes the value if engaged, `null` if disengaged. `ValueBox<T>` unwraps and serializes.
`std::variant<Ts...>` dispatches via `std::visit`.

**Freeform JSON:** `json::Value` serializes according to its held alternative.

### Per-Struct Serialization (Macros)

Each provider's `.cpp` file defines `SerializeFrom` overloads for every struct using macros:

```cpp
BEGIN_SERIALIZE(anthropic::TextBlockParam)
    FIELD(obj, type, CommaDirection::NONE)
    FIELD(obj, text, CommaDirection::BEFORE)
    FIELD(obj, cache_control, CommaDirection::BEFORE)
    FIELD(obj, citations, CommaDirection::BEFORE)
END_SERIALIZE
```

**`BEGIN_SERIALIZE(Type)`** — Opens the function, starts a JSON object.
**`FIELD(obj, member, comma)`** — Emits the member as a key-value pair. The field name in JSON
matches the C++ member name. The `comma` parameter controls comma placement.
**`FIELD_ALT(obj, member, name, comma)`** — Same as `FIELD` but uses a custom JSON key name.
**`END_SERIALIZE`** — Closes the JSON object and function.

### Field Emission Logic (AddKV)

The `AddKV` template handles the Required-vs-optional distinction:

```cpp
// Required<T>: always emitted
template <key, Dir, typename T>
void AddKV(builder, const Required<T>& v);

// std::optional<T>: emitted only if engaged
template <key, Dir, typename T>
void AddKV(builder, const std::optional<T>& v);

// std::vector<T>: emitted only if non-empty
template <key, Dir, typename T>
void AddKV(builder, const std::vector<T>& v);

// std::map<K,V>: emitted only if non-empty
template <key, Dir, typename K, typename V>
void AddKV(builder, const std::map<K,V>& v);
```

This means optional fields that are disengaged, empty vectors, and empty maps are silently
omitted from the output — the JSON key never appears. Required fields are always emitted.

### Comma Management

`CommaDirection` controls where commas are placed relative to a field:

```cpp
enum class CommaDirection : uint8_t { NONE, BEFORE, AFTER, BOTH };
```

The first field in an object typically uses `NONE`; subsequent fields use `BEFORE`. This is a
manual approach that avoids trailing comma issues.

Note: Because optional fields may be omitted, comma placement requires care. If an optional
field is the "first" field emitted (because all preceding optionals were disengaged), it must
not emit a leading comma. The current macro approach handles this through explicit ordering,
but this is an area where care is needed when adding new fields.

## Deserialization (JSON → Struct)

### Core Function

```cpp
template <typename T>
T DeserializeTo(const simdjson::dom::element& src);
```

Like serialization, the primary template is `= delete`'d. Overloads are provided for all
supported types.

### Built-in Type Overloads

The framework provides `DeserializeTo` overloads mirroring the serialization side:

**Primitive types:** `bool`, `std::byte`, `double`, `int64_t`, `std::string`, `std::string_view`.

**Domain types:** `EncodedUrl`, `Int64`, `Int64Bounded<Lo,Hi>`, `Int64Str`, `Name64`,
`NameLen<N>`, `RFC3339Timestamp`.

**Kind structs and enums:** Extracted as strings, then converted via `from_string_view<T>()`.
Unrecognized values throw `AnnotatedException`.

**Container types:** `std::vector<T>` (from JSON array), `std::map<K,V>` (from JSON object).

**Wrapper types:** `Required<T>`, `std::optional<T>`, `ValueBox<T>` all unwrap to inner type.

**Freeform JSON:** `json::Value` inspects the simdjson element type and constructs the
appropriate variant alternative.

### Per-Struct Deserialization (Macros)

```cpp
BEGIN_DESERIALIZE(anthropic::TextBlockParam)
    FIELD(src, type),
    FIELD(src, text),
    FIELD(src, cache_control),
    FIELD(src, citations)
END_DESERIALIZE
```

**`BEGIN_DESERIALIZE(Type)`** — Opens a template specialization returning `Type`, using aggregate
initialization.
**`FIELD(src, member)`** — Extracts the named field from the JSON object. The field name in JSON
matches the C++ member name.
**`FIELD_PLACEHOLDER(src, member)`** — Extracts a value without a named key (used for positional
data within the current element).
**`END_DESERIALIZE`** — Closes the aggregate initializer and function.

### Field Extraction (Extract)

```cpp
template <key, typename T, auto Member>
member_type_t<T, Member> Extract(const simdjson::dom::element& obj);
```

`Extract` looks up the JSON key in the object. The behavior depends on the member type:

- **`Required<T>`**: The key must exist. Missing key → exception.
- **`std::optional<T>`**: Missing key → `std::nullopt`. Present key → value.
- **Any other type**: Missing key → exception.

On extraction failure, the exception is annotated with the field name and parent type for
diagnostic clarity.

### Variant Deserialization

Variants are deserialized using a discriminator-based approach in the per-provider `.cpp` files.
The pattern uses macros:

```cpp
BEGIN_DESERIALIZE_VARIANT(anthropic::ContentBlockParam)
    auto kind = EXTRACT_KIND(anthropic::ResponseContentBlockKinds, src, "type");
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::TEXT, anthropic::TextBlock)
    FIELD_KIND(src, kind, anthropic::ResponseContentBlockKinds::TOOL_USE, anthropic::ToolUseBlock)
    // ...
END_DESERIALIZE_VARIANT(anthropic::ContentBlockParam)
```

**`EXTRACT_KIND(EnumType, src, field)`** — Reads the discriminator field and converts it to the
enum type.
**`FIELD_KIND(src, kind, expected, TargetType)`** — If the extracted kind matches `expected`,
deserialize the element as `TargetType` and return it wrapped in the variant.

For variants discriminated by field presence rather than a type tag:

**`FIELD_VARIANT(obj, field_name, TargetType)`** — If the named field exists in the JSON object,
deserialize the whole object as `TargetType`.
**`FIELD_VARIANT_KV(obj, field_name, TargetType)`** — If the named field exists, deserialize
just that field's value as `TargetType`.

### Error Reporting

Deserialization errors produce `AnnotatedException` with context chains showing the field path:

```
Exception-
Reason: Invalid enum: unknown_value

Context-
    Failed to extract field stop_reason
    while parsing anthropic::Response
    [protocols/deserialize/anthropic_messages.cpp (42, 5) - DeserializeTo]
```

## Adding a New Struct

To add serialization and deserialization for a new struct:

1. Define the struct in the appropriate `interface/protocols/<provider>/<endpoint>.hpp`.
2. Add any new enums to the same file. Add Kind structs if needed.
3. Add `to_string_view` and `from_string_view` specializations in the provider's `strings.hpp`.
4. In `src/protocols/serialize/<provider>_<endpoint>.cpp`, add a `BEGIN_SERIALIZE` / `END_SERIALIZE`
   block listing all fields with correct comma directions.
5. In `src/protocols/deserialize/<provider>_<endpoint>.cpp`, add a `BEGIN_DESERIALIZE` /
   `END_DESERIALIZE` block listing all fields in declaration order.
6. For variant types, add a `BEGIN_DESERIALIZE_VARIANT` / `END_DESERIALIZE_VARIANT` block with
   the discriminator logic.
7. Add unit tests in `tests/unit/test_<provider>.cpp`.
