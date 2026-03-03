# API Protocol Codegen Design

This document captures the design decisions, rules, and guidance for programmatic
translation of LLM API specifications into C++ protocol code. It is the product
of iterative design discussion and reflects the rationale behind each decision.

## Motivation

The previous approach to API parity required a human or agent to manually
integrate changes across five C++ artifacts: struct definitions (header), string
conversions, serializers, deserializers, and tests. This process was error-prone
— field requiredness had to be verified against documentation, enum renames
propagated across files, and ordering constraints in headers required careful
management. The codegen approach eliminates this by generating all artifacts
directly from the parsed API specification.

## Architecture Overview

The pipeline has six stages (see `docs/api_parity_pipeline.md` for full details):

1. **Fetch**: Download the raw API spec from the provider.
2. **Extract**: Transform the spec into a structured intermediate JSON.
3. **Diff**: Compare against previous baseline. If no changes, stop.
4. **Generate**: Emit C++ artifacts from the intermediate JSON.
5. **Deploy**: Copy generated files into the source tree.
6. **Build**: cmake configure + build to verify generated code compiles.
7. **Promote**: Save current extraction as the new baseline.

The intermediate JSON is the contract between parsing and generation. Its design
is the primary focus of this document.

## Design Principles

### Spec Faithfulness

The codegen faithfully represents the API specification as-is. There are no
override tables, no manual type remappings, and no "intent-based" corrections.
If the spec says `number`, the C++ type is `double`. If the spec says `integer`,
the C++ type is `int64_t`. If the spec marks a field as optional, it becomes
`std::optional<T>`.

### Inline Everything (Tree Codegen)

The Anthropic and OpenAI API specifications define all types inline at every
point of use. The codegen follows the spec's own structure: every type is emitted
as a nested struct at its point of use. This eliminates reference resolution,
ordering problems, and trickle-down changes when a shared type's definition
evolves in the spec.

### Flat Schemas (Discovery Codegen)

The Gemini API uses Google's Discovery Document format, where schemas are flat
siblings with `$ref` pointers between them. The flat codegen preserves this
structure, emitting types at namespace level with reachability analysis to
include only schemas relevant to the target endpoint.

### Nesting Over Flattening

The previous C++ layout placed all structs at the top level of the provider
namespace. This caused name collisions requiring artificial prefixes. The
codegen nests structs according to their ownership in the spec, so names
resolve naturally through scope: `Request::EasyInputMessage::Content`.

Enums and Kind types follow the same nesting rule. If an enum is defined
within a struct's field, it is nested inside that struct.

### Structs Are Transport, Not Validation

The generated structs represent the API's data shape. They do not enforce
constraints beyond type safety. There is no validation of mutual exclusion of
subtrees, string length limits, or semantic rules. The structs provide data;
validation is a separate concern.

### Macro Preservation

The serialization and deserialization code uses `BEGIN_SERIALIZE` / `FIELD` /
`END_SERIALIZE` and `BEGIN_DESERIALIZE` / `FIELD` / `END_DESERIALIZE` macro
patterns. These macros are a bridge to C++ reflection. Once reflection is
available, the string conversion and serialization/deserialization code should
largely disappear. The codegen emits code using these existing macros rather
than bypassing them.

### Full Regeneration

When spec changes are detected, the entire set of artifacts for that
provider/endpoint is regenerated. There is no patching or incremental update.
The codegen is fast and deterministic, so full regeneration is both simpler
and safer than trying to patch individual changes.

### Tests Are Separate

Tests are hand-written and not touched by the pipeline. Since the structs and
serialization code are generated from the spec, correctness follows from the
correctness of the generator. The testing strategy is:

- Test the generator itself (does it produce valid C++ for representative inputs).
- A small set of integration-level smoke tests (round-trip serialize/deserialize).
- If the generator runs clean, the output is correct by construction.

### Projections Are a Separate Concern

Projection layers (e.g., `text.hpp`) that consume the generated protocol types
are not part of the codegen pipeline. They will be governed by manifests that
declare which API fields the projection depends on. Manifest design is
deferred to a separate effort.

## Core Invariants

These rules govern the entire pipeline and must not be violated by any fix or
enhancement. They were established through iterative design and are
non-negotiable.

1. **Everything is a variant arm.** The ONLY exception is when a union is 100%
   string literals, which becomes a pure `enum class` (not wrapped in a
   variant). A mix of enums and non-enum types (e.g., enum + string) NEVER
   condenses — it must produce `std::variant<EnumValues, std::string>`.

2. **Children are always authoritative.** When a spec field has both an inline
   type expression and indented child definitions, the children define the true
   structure. The inline expression is used only for structural hints (e.g.,
   detecting `array<>` wrappers).

3. **No encoded strings in the intermediate.** The intermediate JSON must be
   cleanly structured data. No pipe-separated (`|`) or `or`-separated strings
   that downstream code has to re-parse. If a type expression contains
   alternatives, the parser must decompose them into structured member lists.

4. **No one-off/hack code.** Fixes must flow through existing procedures and
   patterns. No special-cased field names, no provider-specific branches based
   on field identity.

5. **No name-based special casing.** Behavior is driven by TYPE EXPRESSION, not
   field or type names.

6. **No `std::unique_ptr` indirection.** Variant members are held directly, not
   behind indirection (except where `ValueBox<T>` is used for recursion).

7. **`unknown` in spec → `object` in intermediate → `jai::llm::json::Object`
   in C++.**

8. **All variants must be named.** No anonymous `std::variant<...>` in
   `BEGIN_DESERIALIZE_VARIANT`. Every variant gets a `using` alias.

## Type System

The codegen recognizes the following field types in the intermediate JSON:

| JSON Type | C++ Type | Notes |
|-----------|----------|-------|
| `kind` | `Kind<T>` struct | A string field with exactly one allowed value. Emitted as a `constexpr` Kind struct. |
| `enum` | `enum class` | A string field with a fixed set of allowed values. |
| `bool` | `bool` | |
| `integer` | `int64_t` | Explicit integer type in the spec. |
| `number` | `double` | Numeric type. Always `double`, no overrides. |
| `string` | `std::string` | |
| `array<T>` | `std::vector<T>` | Element type specified. |
| `array<T>` + `element_members` | `std::vector<Alias>` | Array of variant elements; see Structured Intermediate Fields. |
| `array<string>` + `element_enum_values` | `std::vector<EnumItem>` | Array of enum elements; see Structured Intermediate Fields. |
| `array<T>` + `union_def` | `std::vector<Alias>` | Array of variant elements from `array of X or Y`; see Structured Intermediate Fields. |
| `union` | `std::variant<T...>` | Discriminated union. Members and discriminator field specified. |
| `map<string, T>` | `std::unordered_map<std::string, T>` | Map type. |
| `map<string, T>` + `map_value_members` | `std::unordered_map<std::string, Alias>` | Map with variant values; see Structured Intermediate Fields. |
| `object` | `jai::llm::json::Object` | Untyped JSON object (e.g., `map<string, unknown>`). |
| `struct` | Nested struct | A named type with defined fields. |

### Number Interpretation

The spec type `number` always maps to `double`. The spec type `integer` maps
to `int64_t`. The codegen does not attempt to infer integrality from description
text or field semantics.

## Intermediate JSON Structure

The intermediate JSON is the output of the parser and the input to the code
generator. Its structure mirrors the nested type hierarchy of the spec.

### Tree Format (OpenAI, Anthropic)

**Arrays over objects for ordered collections.** Fields within a struct are
represented as an array of field descriptors, not a keyed object. This preserves
parse order and allows incremental construction.

**Ancestry stack during parsing.** The parser maintains a stack of type names
representing the current nesting path. This provides the fully qualified
ownership path for any type, enum, or kind.

**Field names are not part of the ownership chain.** The ownership path tracks
type names only. A field like `MessageParam.content` of type `TextBlockParam`
produces the ownership chain `MessageParam::TextBlockParam`.

A struct definition:

```json
{
  "name": "MessageParam",
  "kind": null,
  "fields": [
    {
      "name": "content",
      "type": "union",
      "required": true,
      "discriminator": "type",
      "members": [
        { "type": "string" },
        { "type": "array", "element": "ContentBlockParam" }
      ]
    },
    {
      "name": "role",
      "type": "enum",
      "required": true,
      "values": ["user", "assistant"]
    }
  ],
  "children": [
    {
      "name": "ContentBlockParam",
      "kind": null,
      "fields": [ ... ],
      "children": [ ... ]
    }
  ]
}
```

### Flat Format (Gemini)

Schemas are flat siblings with `$ref`-style references between them. The parser
performs reachability analysis from request/response root types to include only
relevant schemas, tagging each as `request`, `response`, or `shared`.

### Structured Intermediate Fields

When a spec type expression contains alternatives (e.g., `string or number` or
`array of X or Y`), the parser decomposes them into structured member lists
attached directly to the field descriptor. The codegen reads these structured
lists; it never splits on separators. This section documents each structured
field and the spec pattern that produces it.

#### `element_members` — Array of variant elements

Produced when an array field's element type is itself a union (e.g.,
`array of string or number`). Each member is classified by `_classify_union_member`.

```json
{
  "name": "values",
  "type": "array<string>",
  "element_members": [
    { "type": "string" },
    { "type": "number" }
  ]
}
```

Codegen emits a `using ValuesElement = std::variant<std::string, double>;` and
the field becomes `std::vector<ValuesElement>`. A dispatch is registered for
deserialization using JSON type checks derived from the member types.

#### `element_enum_values` — Array of enum elements

Produced when a field is `array of "literal_a" or "literal_b" or ...` — all
children are string literals. The parser stores the literal values directly.

```json
{
  "name": "allowed_callers",
  "type": "array<string>",
  "element_enum_values": ["direct", "code_execution_20250825", "code_execution_20260120"]
}
```

Codegen emits an `enum class AllowedCallersItem { DIRECT, CODE_EXECUTION_20250825, ... };`
and the field becomes `std::vector<AllowedCallersItem>`.

#### `map_value_members` — Map with variant values

Produced when a map field's value type contains alternatives (e.g.,
`map[string or number or boolean]`). Each member is classified by
`_classify_union_member`.

```json
{
  "name": "attributes",
  "type": "map<string, string>",
  "map_value_members": [
    { "type": "string" },
    { "type": "number" },
    { "type": "boolean" }
  ]
}
```

Codegen emits a `using AttributesValue = std::variant<std::string, double, bool>;`
and the field becomes `std::unordered_map<std::string, AttributesValue>`. A
dispatch is registered for deserialization using JSON type checks.

#### `union_def` — Named variant from `array of X or Y` (struct members)

Produced when a field is `array of StructA or StructB` where the children are
struct (non-enum) union members. The parser calls `_parse_union_children` to
collect the members, then wraps them in a `union_def` and rewrites the field
type to `array<VariantName>`.

```json
{
  "name": "content",
  "type": "array<Content>",
  "union_def": {
    "name": "Content",
    "discriminator": "type",
    "members": [
      { "type": "struct", "ref": "ResponseOutputText", "discriminator_value": "output_text" },
      { "type": "struct", "ref": "ResponseOutputRefusal", "discriminator_value": "refusal" }
    ]
  }
}
```

Codegen emits `using Content = std::variant<ResponseOutputText, ResponseOutputRefusal>;`
and the field becomes `std::vector<Content>`. The `union_def` mechanism is also
used by `_parse_field_children` for similar array-of-variant patterns discovered
during child parsing.

## Variant Naming and Aliasing

Every variant type must have a named `using` alias. No anonymous
`std::variant<A, B, C>` may appear in `BEGIN_DESERIALIZE_VARIANT` blocks.

For inline union fields, the alias name is derived from `_pascal(field_name)`.
For array element variants, the alias is `<ParentAlias>Element` (e.g.,
`ValuesElement`). For map value variants, the alias is `<PascalFieldName>Value`
(e.g., `AttributesValue`). For `union_def` variants, the alias name comes from
the `union_def["name"]` field.

When an element variant is used inside a larger union, the codegen emits the
element alias first (e.g., `using ValuesElement = std::variant<...>;`), then
references it as `std::vector<ValuesElement>` in the parent variant's member
list. An `_alias_to_concrete` mapping tracks these substitutions so the
deduplication key expands aliases back to concrete types — C++ template
specializations see through `using` aliases, so `DeserializeTo<std::vector<ValuesElement>>`
and `DeserializeTo<std::vector<std::variant<std::string, double>>>` would collide.

## Variant Dispatch

C++ `std::variant` deserialization requires knowing which type to construct
from a JSON discriminator field (typically `"type"`). The codegen synthesizes
dispatch enums automatically.

For each union field, the codegen:

1. Identifies the discriminator field (usually `"type"`).
2. For each member struct, reads its Kind field value (the discriminator string).
3. Emits a dispatch enum with entries mapping discriminator strings to variant indices.
4. Emits `from_string_view` / `to_string_view` for the dispatch enum.
5. Emits the `DESERIALIZE_VARIANT` block.

### Variant Type Deduplication

When multiple scopes contain structurally identical variants, only one
`BEGIN_DESERIALIZE_VARIANT` block should be emitted. Deduplication uses a
fully-qualified variant key (`fq_variant_key`) built as follows:

1. Start with the concrete C++ member types (not aliased names).
2. Fully qualify each member via `_qualify_cpp_type(m, parent_path)`, which
   recursively qualifies types inside `std::vector<>` and `std::variant<>`.
3. Join as `std::variant<qualified_member_1, qualified_member_2, ...>`.

The `_alias_to_concrete` mapping is critical here: when a variant member is
`std::vector<ValuesElement>`, the dedup key must use the expanded
`std::vector<std::variant<std::string, double>>` instead, because C++ sees
through `using` aliases for template specialization purposes.

Struct members without discriminator values use `is_object()` type checks
instead of discriminator-based dispatch.

## C++ Keyword Handling

A static lookup table maps field names that conflict with C++ reserved words
to safe alternatives:

| JSON Field Name | C++ Member Name | Notes |
|----------------|-----------------|-------|
| `default` | `default_value` | C++ keyword |
| `enum` | `enum_value` | C++ keyword |
| `class` | `class_value` | C++ keyword |
| `stderr` | `stderr_` | Macro conflict |
| `stdout` | `stdout_` | Macro conflict |

The serialization layer uses `FIELD_ALT` to map the C++ member name back to
the original JSON key name.

## Requiredness

Each field in the intermediate JSON carries a `required` boolean taken directly
from the API specification. The codegen maps:

- `required: true` → `Required<T>`
- `required: false` → `std::optional<T>` (initialized with `{}`)

The codegen does not second-guess the spec. If the spec says a field is
required, it is `Required`. If the spec says optional, it is `std::optional`.

Optional request fields should be omitted from JSON (never set to placeholder
values). They must be default-initialized in the struct (`std::optional<T> field{}`).

## Generated Artifacts

All generated files begin with a banner identifying them as auto-generated and
warning against hand-editing. Changes are overwritten on the next audit run.

From the intermediate JSON, the codegen emits four files per provider/endpoint:

1. **Header** (`<endpoint>.hpp`): Nested struct definitions with `Required<T>`
   and `std::optional<T>` fields, nested `enum class` definitions, nested Kind
   structs.

2. **String conversions** (`<endpoint>_strings.hpp`): `from_string_view` and
   `to_string_view` for all enum types and dispatch enums.

3. **Serializer** (`<provider>_<endpoint>.cpp`): `BEGIN_SERIALIZE` / `FIELD` /
   `END_SERIALIZE` blocks for all request-side types.

4. **Deserializer** (`<provider>_<endpoint>.cpp`): `BEGIN_DESERIALIZE` / `FIELD` /
   `END_DESERIALIZE` blocks for all response-side types, plus variant dispatch blocks.

## Parser Classification Logic

The parser (`parsers/openai_anthropic.py`) classifies each field based on its
raw type expression and child definitions. The classification follows a
priority chain; this section documents the key branches.

### The `has_or` branch

When a raw field type expression contains ` or ` (e.g., `string or number`,
`array of "a" or "b"`, `array of ResponseOutputText or ResponseOutputRefusal`),
the parser enters the `has_or` branch. This branch defers to children for
structure, then classifies based on what the children contain:

1. **Children are all string literals** (`child_enums` is non-empty):
   - If the type starts with `array<`: the field becomes `array<string>` with
     `element_enum_values` storing the literal values.
   - Otherwise: the field becomes a pure `enum` with `values`.

2. **Children are struct/union members** (`child_enums` is empty):
   - The parser calls `_parse_union_children` to collect structured members.
   - If the type starts with `array<`: the union describes the *element* type.
     The parser creates a `union_def`, moves the members into it, and rewrites
     the field type to `array<VariantName>`.
   - Otherwise: the field becomes a bare `union` with `members`.

The `has_or` check must be performed on the RAW type expression, BEFORE
`_normalize_type` runs, because normalization may strip the ` or ` alternatives.

### Map inner type alternatives

When the raw type is `map[X or Y or Z]`, the `is_map_with_inner_or` branch
extracts the alternatives, classifies each via `_classify_union_member`, and
stores the result as `map_value_members`. The normalized type string uses only
the first alternative as a structural placeholder; the authoritative value type
information lives in the structured `map_value_members` list.

## Provider Assumptions

This design supports three provider patterns:

- **Anthropic**: Inlines all type definitions. Uses `"type"` as the universal
  discriminator. Request and response in separate sections.
- **OpenAI**: Same markdown format as Anthropic. Same parser.
- **Gemini**: Google Discovery Document with flat `$ref`-based schemas. Separate
  codegen (`codegen_flat.py`) handles the flat structure.
