# API Parity Pipeline

Design document for the automated workflow that detects API specification changes
and regenerates C++ protocol code.

## Problem

Provider APIs change over time: new fields appear, types change, enum values are added. The
library's C++ protocol structs must stay in sync. This pipeline automates detection of
structural changes and regenerates the C++ artifacts when changes are found.

## Non-Goals

- Running as part of the library build. This is a developer/maintainer tool only.
- Handling streaming or event-based API specs (future work).
- Patching or incremental updates. Changes trigger a full regeneration.

## Pipeline Stages

```
Stage 1: Fetch          Download raw spec from provider doc URLs
            ↓
Stage 2: Extract        Parse spec → structured JSON (intermediate format)
            ↓
Stage 3: Diff           Compare current extraction against previous baseline
            ↓                (no baseline = everything is new)
Stage 4: Generate       Emit C++ artifacts from intermediate JSON
            ↓
Stage 5: Deploy         Copy generated files into the source tree
            ↓
Stage 6: Build          cmake configure + build to verify generated code compiles
            ↓
Stage 7: Promote        Save current extraction as the new baseline
            ↓
Stage 8: Clean          Remove scratch directory
```

Stages 1–2 run for every provider. Stage 3 returns a boolean: `True` if there are
structural changes (or no baseline exists). Stages 4–7 only run when the diff is `True`.
Stage 8 runs on success unless `--no-clean` is set.

When running with `--all`, stages 1–5 run per-provider first, then stages 6–8 run once
after all providers are deployed. This ensures the build sees all generated code before
any baselines are promoted. If any provider fails during stages 1–5, the pipeline aborts
before build/promote.

The `audit` command runs the full pipeline end-to-end:

```bash
python scripts/api_parity/run.py audit --provider openai
python scripts/api_parity/run.py audit --all
python scripts/api_parity/run.py audit --all --no-clean    # keep scratch for inspection
```

Individual stages can also be run independently:

```bash
python scripts/api_parity/run.py fetch     --provider openai
python scripts/api_parity/run.py extract   --provider openai
python scripts/api_parity/run.py diff      --provider openai
python scripts/api_parity/run.py generate  --provider openai
python scripts/api_parity/run.py deploy    --provider openai
python scripts/api_parity/run.py build
python scripts/api_parity/run.py clean
python scripts/api_parity/run.py promote   --provider openai
```

## Provider Doc Sources

### OpenAI (Responses API)

**URL:** `https://developers.openai.com/api/reference/resources/responses/methods/create/index.md`

**Format:** Markdown with indentation-based field hierarchy. Sections: `### Body Parameters`
(request), `### Returns` (response). Field pattern: `- \`name: optional type_or_union\``
with 2-space indentation per nesting level.

**Parser:** `parsers/openai_anthropic.py`

### Anthropic (Messages API)

**URL:** `https://platform.claude.com/docs/en/api/messages/create.md`

**Format:** Identical structure to OpenAI. Same section names, same field definition syntax,
same indentation conventions.

**Parser:** `parsers/openai_anthropic.py`

### Gemini (GenerateContent)

**URL:** `https://generativelanguage.googleapis.com/$discovery/rest?version=v1beta`

**Format:** Google API Discovery Document (JSON). Flat schema definitions with `$ref` pointers
between them. The parser performs reachability analysis from the request/response types to
include only schemas relevant to the target endpoint.

**Parser:** `parsers/gemini.py`

## Intermediate Format (Extracted JSON)

Each extraction produces a JSON file representing the structural content of one API endpoint.
This is the unit of comparison for diffs.

### Tree Format (OpenAI, Anthropic)

```json
{
  "meta": {
    "provider": "anthropic",
    "endpoint": "messages",
    "source_url": "...",
    "fetched_at": "..."
  },
  "request": {
    "root": { "name": "Request", "fields": [...], "children": [...] }
  },
  "response": {
    "root": { "name": "Message", "fields": [...], "children": [...] }
  }
}
```

Types are nested within the tree through `children` arrays, mirroring the spec's inline
type definitions.

### Flat Format (Gemini)

```json
{
  "meta": { ... },
  "schemas": {
    "GenerateContentRequest": { "name": "...", "side": "request", "fields": [...] },
    "Content": { "name": "...", "side": "shared", "fields": [...] },
    ...
  },
  "request_type": "GenerateContentRequest",
  "response_type": "GenerateContentResponse"
}
```

Schemas are flat siblings with `side` indicating request/response/shared.

### Field Type System

Each field has a `type` and `required` flag:

- **Fundamental types:** `"string"`, `"number"`, `"boolean"`, `"object"`, `"unknown"`.
- **Compound types:** `"array<Foo>"`, `"map<string, unknown>"`.
- **Named type references:** `"CacheControlEphemeral"` — refers to another type definition.
- **Kind (discriminator):** `{ "type": "kind", "required": true, "value": "text" }`
- **String enum:** `{ "type": "string", "required": false, "values": ["5m", "1h"] }`
- **Union:** `{ "type": "union", "required": true, "members": [...] }`

Fields may carry additional structured metadata when the type expression
contains alternatives. These are documented in detail in `docs/codegen_design.md`
under "Structured Intermediate Fields":

- **`element_members`** — array element union members (e.g., `array of string or number`)
- **`element_enum_values`** — array of literal enums (e.g., `array of "a" or "b"`)
- **`map_value_members`** — map value union members (e.g., `map[string or number]`)
- **`union_def`** — named variant definition for array-of-struct unions (e.g., `array of X or Y`)

## Generated Artifacts

For each provider/endpoint, the codegen produces four files:

| Generated File | Deployed Location |
|---------------|-------------------|
| `<endpoint>.hpp` | `interface/protocols/<provider>/<endpoint>.hpp` |
| `<endpoint>_strings.hpp` | `interface/protocols/<provider>/<endpoint>_strings.hpp` |
| `<endpoint>_serialize.cpp` | `src/protocols/serialize/<provider>_<endpoint>.cpp` |
| `<endpoint>_deserialize.cpp` | `src/protocols/deserialize/<provider>_<endpoint>.cpp` |

For example, OpenAI produces `responses.hpp`, `responses_strings.hpp`,
`openai_responses.cpp` (serialize), and `openai_responses.cpp` (deserialize).

## Diff Output

The diff stage compares two extracted JSON files (baseline vs current) and produces a
structured diff JSON in `scratch/api_parity/diffs/`. The diff tracks:

- Added/removed objects (structs)
- Added/removed/changed fields (type, required status, enum values)
- Added/removed enum values
- Description changes (tracked separately, non-structural)

If no baseline exists, the diff returns `True` (everything is new) without writing a
diff file.

## File Layout

```
scripts/
  api_parity/
    __init__.py
    config.py              Provider URLs, endpoint definitions, path helpers
    fetch.py               Stage 1: download raw spec
    extract.py             Stage 2: spec → intermediate JSON
    diff.py                Stage 3: compare against baseline (returns bool)
    report.py              Generate human-readable report from diff
    checklist.py           Generate integration checklist from diff
    codegen.py             Stage 4 (tree): generate C++ from tree JSON
    codegen_flat.py        Stage 4 (flat): generate C++ from flat JSON
    run.py                 CLI entry point
    seed_baseline.py       Seed baseline from existing C++ headers
    parsers/
      __init__.py
      openai_anthropic.py  Shared parser for OpenAI/Anthropic markdown format
      gemini.py            Parser for Google Discovery JSON format

specs/
  openai/
    responses.json         Approved baseline extraction
    responses.raw.md       Raw spec snapshot paired with baseline
  anthropic/
    messages.json
    messages.raw.md
  gemini/
    generate_content.json
    generate_content.raw.json   Raw Discovery Document snapshot

scratch/
  api_parity/              Working files during pipeline runs (cleaned on success)
    openai/                Fetched spec + extracted JSON
    anthropic/
    gemini/
    generated/             Codegen output (before deploy)
    diffs/                 Diff output JSONs
    build/                 cmake build directory for verification
    reports/               Generated markdown reports
    checklists/            Generated integration checklists
```

## Future Extensions

- **GitHub Actions**: Scheduled audit that opens a PR when changes are detected.
- **Streaming/event specs**: Extend the intermediate format to capture SSE event types.
- **Deprecated field filtering**: Detect and flag deprecated fields in extractions.
- **Nullable field tracking**: Distinguish nullable from optional in the intermediate format.
