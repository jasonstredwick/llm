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
| `array` | `std::vector<T>` | Element type specified. |
| `union` | `std::variant<T...>` | Discriminated union. Members and discriminator field specified. |
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

Variant type deduplication uses fully-qualified type keys to avoid false matches
across different parent scopes. Struct members without discriminator values use
`is_object()` type checks.

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

## Provider Assumptions

This design supports three provider patterns:

- **Anthropic**: Inlines all type definitions. Uses `"type"` as the universal
  discriminator. Request and response in separate sections.
- **OpenAI**: Same markdown format as Anthropic. Same parser.
- **Gemini**: Google Discovery Document with flat `$ref`-based schemas. Separate
  codegen (`codegen_flat.py`) handles the flat structure.
