# API Parity Integration Guide

How to take a pipeline audit report and propagate the changes into C++ code. This document
is for both human developers and LLM agents performing integration work.

## Prerequisites

Before starting integration, you need a completed audit:

```bash
python scripts/api_parity/run.py audit --provider <provider>
```

This produces a diff JSON and human-readable report in `scratch/api_parity/`. Verify the
report is non-empty and review it before proceeding.

## Step 1: Generate the Integration Checklist

The checklist is the single source of truth for what needs to be done. It is generated
mechanically from the diff JSON — not curated by hand. Every item in the diff becomes one
or more checklist entries. Nothing is deferred or skipped.

```bash
python scripts/api_parity/run.py checklist --provider <provider>
```

This produces `scratch/api_parity/checklists/<provider>_checklist.md`. The checklist groups
items by change type and lists the specific C++ files that need updating for each item.

**Rule: The agent must not begin writing C++ code until the checklist exists.** The checklist
prevents the agent from making ad-hoc decisions about what to implement and what to skip.

## Step 2: Work Through the Checklist

Process items in order. Mark each item done (change `[ ]` to `[x]`) as you complete it.
If an item requires a judgment call (e.g., choosing between `std::string` and `Name64`),
note the decision inline.

The checklist should be updated on disk after each batch of items so progress is preserved
across context boundaries.

### Recommended ordering

1. **New enums and kinds first** — these are dependencies for new structs and fields.
2. **New structs (leaf types first)** — structs that don't reference other new structs.
3. **New structs (composite types)** — structs that reference the leaf types above.
4. **New fields on existing structs** — these may reference the new structs.
5. **Changed fields** — type changes, required/optional changes.
6. **Variant updates** — adding new types to `ContentBlockParam`, `ContentBlock`, `ToolUnion`, etc.
7. **Removed items** — remove deprecated fields/structs last.
8. **Unit tests** — update existing tests with new Required fields, add new tests for new types.

## C++ Files Affected by Change Type

Every change in the diff maps to one or more C++ files. The checklist generator uses this
mapping to produce file-specific action items.

### Per-provider file locations (using Anthropic as example)

| File | Role | Path |
|------|------|------|
| **Protocol header** | Struct/enum/kind definitions | `interface/protocols/anthropic/messages.hpp` |
| **String conversions** | `from_string_view` / `to_string_view` for kinds and enums | `interface/protocols/anthropic/strings.hpp` |
| **Serializer** | Request serialization (`Serialize`) | `src/protocols/serialize/anthropic_messages.cpp` |
| **Deserializer** | Response deserialization (`Deserialize`) | `src/protocols/deserialize/anthropic_messages.cpp` |
| **Unit tests** | Serialization and deserialization tests | `tests/unit/test_anthropic.cpp` |

### What each change type requires

**New object (struct)**
- `messages.hpp` — define the struct with its fields
- `strings.hpp` — add kind conversion if the struct has a `type` kind field
- `serialize/*.cpp` — add serializer if the struct appears in requests
- `deserialize/*.cpp` — add deserializer if the struct appears in responses
- `messages.hpp` — add to relevant variant type (`ContentBlockParam`, `ContentBlock`,
  `ToolUnion`, etc.) if applicable
- `test_*.cpp` — add test coverage

**New field on existing struct**
- `messages.hpp` — add the field to the struct definition
- `serialize/*.cpp` — add `FIELD(obj, name, comma)` if in a request struct
- `deserialize/*.cpp` — add `FIELD(src, name)` if in a response struct
- `test_*.cpp` — update existing test JSON data if the field is `Required<T>`; add
  assertions for the new field

**New enum**
- `messages.hpp` — define `enum class` with values
- `strings.hpp` — add `from_string_view` and `to_string_view`
- No serializer/deserializer changes (enums are handled via string conversion)

**New kind (discriminator)**
- `messages.hpp` — define `struct KindFoo : Kind { ... }`
- `strings.hpp` — add `from_string_view` and `to_string_view`

**New enum value (on existing enum)**
- `messages.hpp` — add the value to the existing `enum class`
- `strings.hpp` — add case to `from_string_view` and `to_string_view`

**Changed field (type change)**
- `messages.hpp` — update the field type
- `serialize/*.cpp` / `deserialize/*.cpp` — update if the serialization changes
- `test_*.cpp` — update test data and assertions

**Changed field (required ↔ optional)**
- `messages.hpp` — change between `Required<T>` and `std::optional<T>`
- `test_*.cpp` — if changed to `Required<T>`, update all test JSON to include the field

**Removed field**
- `messages.hpp` — remove the field
- `serialize/*.cpp` / `deserialize/*.cpp` — remove the `FIELD` entry
- `test_*.cpp` — remove assertions; remove from test JSON if it was Required

**Removed object**
- Reverse of "New object" — remove from all files and variant types

### Type mapping conventions

When the diff says a field has a certain type, the agent maps it to C++ using these
conventions. The field description in the extracted JSON provides hints.

| Extraction type | Default C++ type | Override hints |
|----------------|-----------------|---------------|
| `string` | `std::string` | "max N characters" → `Name64` etc. |
| `number` | `int64_t` | "between 0 and 2" → `double` |
| `boolean` | `bool` | |
| `object` | `json::Object` | |
| `array<Foo>` | `std::vector<Foo>` | |
| `union` | `std::variant<...>` | |
| `kind` | `struct KindFoo : Kind` | |
| Named reference | The C++ struct name | May differ from doc name |

### Naming conventions

- C++ struct names match extraction names: `TextBlockParam` → `TextBlockParam`
- C++ enum class names match extraction names where possible
- Enum values are UPPER_SNAKE_CASE: `"end_turn"` → `END_TURN`
- Kind values use the doc string: `"text"` → `KindText::value = "text"`
- Kinds and enums are kept in single alphabetized lists (no grouping by feature)

## C++ Style Conventions

These rules apply to all C++ code in the protocol layer for every provider (Anthropic,
OpenAI, Gemini). Examples below use Anthropic names for illustration but the conventions
are provider-agnostic. Future agents must follow these exactly.

### Ordering and organization

**Kinds** are defined in a single alphabetized block at the top of the header under the
`/*** Kinds */` comment. They are alphabetized by struct name. No sub-grouping by feature
area, no separating comments like `// Caller kinds` or `// Web fetch kinds`. Just one
flat alphabetical list.

```cpp
/***
 * Kinds
 */
struct KindBase64Source               : Kind { static constexpr std::string_view value = "base64"; };
struct KindCharLocation               : Kind { static constexpr std::string_view value = "char_location"; };
struct KindDirectCaller               : Kind { static constexpr std::string_view value = "direct"; };
// ... all in one alphabetical list
```

**Enums** are defined in a single alphabetized block under `/*** Vocabulary */`. Same rule:
one flat alphabetical list by enum class name. No grouping comments.

```cpp
/***
 * Vocabulary
 */
enum class AllowedCallers { DIRECT, CODE_EXECUTION_20250825 };
enum class CacheControlTTL { TTL_5M, TTL_1H };
// ... alphabetized by enum name
```

**Enum values** within an `enum class` are also alphabetized.

**Fields within structs** follow the order: `type` kind field first (the discriminator),
then remaining `Required<>` fields (alphabetized), then `std::optional<>` fields
(alphabetized). This is alphabetical ordering, not documentation ordering. Doc ordering
is not used because alphabetized fields are easier to verify for completeness.

```cpp
struct ToolUseBlockParam {
    Required<KindToolUseBlock> type{{}};      // kind first
    Required<std::string> id;                  // required, alphabetized
    Required<json::Object> input;
    Required<std::string> name;
    std::optional<CacheControlEphemeral> cache_control{};  // optional, alphabetized
    std::optional<Caller> caller{};
};
```

### Comments

Do not add superfluous comments. Specifically:

- No grouping comments between kinds or enums (e.g., `// Caller kinds`, `// New in v2`)
- No comments restating what a field is (the name and type are self-documenting)
- Section-level comments (`/*** Kinds */`, `/*** Vocabulary */`, `/*** Request Content Blocks */`)
  are fine — they separate major sections of the header

### Struct organization in the header

Structs are organized in **dependency order** (bottom-up) within logical sections. A struct
must be declared before any struct that references it. This means the ordering cannot follow
documentation order (which is top-down). Within that constraint, peer-level structs (ones
that don't depend on each other) are alphabetized.

The existing sections are:

1. **Kinds** — all `KindFoo : Kind` structs (alphabetized, no dependencies)
2. **Vocabulary** — all `enum class` definitions (alphabetized, no dependencies)
3. **Shared Substructures** — types used in both request and response (e.g., `Caller`)
4. **Request Shared Substructures** — request-only shared types (e.g., `CacheControlEphemeral`)
5. **Request Citations** — citation param types
6. **Request Content Blocks** — content block param types, ending with `ContentBlockParam` variant
7. **Tool Definitions** — tool types, ending with `ToolUnion` variant
8. **Request** — the top-level `Request` struct
9. **Response Content Blocks** — response content block types, ending with `ContentBlock` variant
10. **Response** — the top-level `Response` struct and its sub-types (`Usage`, `Container`, etc.)

New structs go into the appropriate section. If a new struct doesn't fit an existing section,
add it to the closest match rather than creating a new section.

### Nesting vs. top-level

Small helper types that are only used by one parent struct can be nested inside it:

```cpp
struct WebSearchToolResultBlockParam {
    struct WebSearchResultBlockParamItem { ... };  // only used here
    struct WebSearchToolRequestError { ... };       // only used here
    using Content = std::variant<std::vector<WebSearchResultBlockParamItem>, WebSearchToolRequestError>;
    ...
};
```

Types referenced by multiple structs must be top-level (not nested).

### Required vs optional

- `Required<T>` for fields the API always returns (response) or always requires (request).
  Required response fields default-initialize, and deserialization will populate them.
  `Required<KindFoo> type{{}};` uses double-brace init for the Kind default.
- `std::optional<T>` for fields the API may omit. Always initialize with `{}`.

### Variant types

Variant type aliases (`using Foo = std::variant<...>`) list their members alphabetized.
When adding a new type to an existing variant, insert it in alphabetical order.

### Serialization / deserialization macros

Serializers use `BEGIN_SERIALIZE` / `END_SERIALIZE` with `FIELD(obj, name, Comma)`:

```cpp
BEGIN_SERIALIZE(anthropic::SomeStruct)
    FIELD(obj, first_field, HasNext)
    FIELD(obj, second_field, Last)
END_SERIALIZE(anthropic::SomeStruct)
```

- `HasNext` means a comma follows; `Last` means no comma (last field in the JSON object).
- Fields are serialized in alphabetical order.

Deserializers use `BEGIN_DESERIALIZE` / `END_DESERIALIZE` with `FIELD(src, name)`:

```cpp
BEGIN_DESERIALIZE(anthropic::SomeStruct)
    FIELD(src, first_field)
    FIELD(src, second_field)
END_DESERIALIZE(anthropic::SomeStruct)
```

- Fields are deserialized in alphabetical order.
- Variant deserializers use `BEGIN_DESERIALIZE_VARIANT` / `END_DESERIALIZE_VARIANT` with
  kind-based discrimination (check the `type` field's string value to pick the variant arm).

### strings.hpp pattern

Each Kind gets a `from_string_view` specialization and a `to_string_view` function.
Each enum gets the same. Entries are alphabetized within their respective sections
(Kinds section, Enums section).

```cpp
// In the Kinds section (alphabetized):
template<> inline KindDirectCaller from_string_view<KindDirectCaller>(std::string_view sv) { ... }
inline std::string_view to_string_view(KindDirectCaller) { return KindDirectCaller::value; }

// In the Enums section (alphabetized):
template<> inline AllowedCallers from_string_view<AllowedCallers>(std::string_view sv) { ... }
inline std::string_view to_string_view(AllowedCallers v) { ... }
```

### Unit test conventions

- Response deserialization tests use raw JSON string literals (`R"({...})"`)
- Every `Required<T>` field on a response struct must be present in all test JSON data
- When a new Required field is added, **all** existing response tests must be updated
- New struct types get their own test functions
- Test function names follow `test_<feature>_<serialization|deserialization>` pattern
- Each test prints a `[SUCCESS]` message on pass; exceptions propagate to `main()`

## Step 3: Verify

After all checklist items are done:

1. **Build**: `cmake --build build --config Release` — must compile cleanly.
2. **Tests**: Run unit tests — must pass.
3. **Re-audit**: Run the pipeline again to verify the diff is now empty (or reduced to
   only description changes). This confirms nothing was missed.

```bash
# Re-run audit to verify
python scripts/api_parity/run.py audit --provider <provider>
# The report should show 0 structural changes
```

## Step 4: Promote

Once verified, promote the current extraction as the new baseline:

```bash
python scripts/api_parity/run.py promote --provider <provider>
```

This copies both the extracted JSON and fetched markdown from scratch to docs/specs as
the new paired baseline snapshot.

## Checklist File Format

The generated checklist lives at `scratch/api_parity/checklists/<provider>_checklist.md`.
It is a markdown file with checkboxes. Example:

```markdown
# Integration Checklist: Anthropic Messages
Generated from: anthropic_messages_diff.json

## Request: New Objects (5)
- [ ] `WebFetchToolResultBlockParam` — messages.hpp, serialize/anthropic_messages.cpp, test_anthropic.cpp
- [ ] `WebFetchBlockParam` — messages.hpp, serialize/anthropic_messages.cpp, test_anthropic.cpp
  ...

## Request: New Fields (3)
- [ ] `Request.container` (string, optional) — messages.hpp, serialize/anthropic_messages.cpp
- [ ] `Request.speed` (string enum [standard, fast], optional) — messages.hpp, strings.hpp, serialize/anthropic_messages.cpp
  ...

## Request: New Enum Values (2)
- [ ] `WebSearchToolResultErrorCode` += `execution_time_exceeded` — messages.hpp, strings.hpp
  ...

## Response: New Objects (4)
- [ ] `WebFetchToolResultBlock` — messages.hpp, deserialize/anthropic_messages.cpp, test_anthropic.cpp
  ...

## Variant Updates
- [ ] Add `WebFetchToolResultBlockParam` to `ContentBlockParam` — messages.hpp
- [ ] Add `WebFetchToolResultBlock` to `ContentBlock` — messages.hpp
  ...

## Unit Test Updates
- [ ] Update response JSON in existing tests with new Required fields
- [ ] Add new test functions for new types
```

The agent checks off items as it works and saves the file periodically. This provides
an auditable trail of what was done and what remains.
