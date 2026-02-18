# API Parity Pipeline

Design document for the automated workflow that detects API specification changes and helps
propagate them into the C++ protocol layer.

Supersedes `docs/specs/api-parity.md` (which can be removed once this is in place).

## Problem

Provider APIs change over time: new fields appear, types change, enum values are added. The
library's C++ protocol structs must stay in sync. Currently this is a manual audit process.
We need a repeatable, scriptable workflow that detects structural changes and produces
actionable reports.

## Non-Goals

- Running as part of the library build. This is a developer/maintainer tool only.
- Handling streaming or event-based API specs (future work).

## Pipeline Stages

```
Stage 1: Fetch          Download raw markdown from provider doc URLs
            ↓
Stage 2: Extract        Parse markdown → structured JSON (intermediate format)
            ↓
Stage 3: Diff           Compare current extraction against previous baseline
            ↓
Stage 4: Report         Generate human-readable change report
            ↓
Stage 5: Checklist      Generate integration checklist from diff (see below)
            ↓
Stage 6: Agent          Work through checklist to update C++ structures
```

Stages 1–4 are standalone Python scripts that read files and write files. No stage depends
on network access except Fetch. Stages can be run independently.

Stage 5 (Checklist) is also a standalone Python script. It reads the diff JSON and current
extraction, then generates a markdown checklist in scratch. Each diff item becomes one or
more checklist entries with the specific C++ files that need updating. Nothing from the diff
is deferred or skipped. See `docs/api_parity_integration.md` for the full integration process.

Stage 6 (Agent) is a separate concern — an agent (human or AI) works through the checklist
to propagate changes into the C++ protocol layer. This step involves judgment calls (e.g.,
choosing `Name64` over `std::string`, or `Int64Bounded<1,10>` over `int64_t`) and is not
automated by the pipeline itself.

## Provider Doc Sources

### OpenAI (Responses API)

**URL:** `https://developers.openai.com/api/reference/resources/responses/methods/create/index.md`

**Format:** Markdown with indentation-based field hierarchy. Sections: `### Body Parameters`
(request), `### Returns` (response), `### Example`.

**Field pattern:** `- \`name: optional type_or_union\`` with 2-space indentation per nesting
level. Union types use `or`. Enum values listed as quoted string bullets. Named sub-types use
`TypeName = object { field1, field2, ... }`.

### Anthropic (Messages API)

**URL:** `https://platform.claude.com/docs/en/api/messages/create.md`

**Format:** Identical structure to OpenAI. Same section names, same field definition syntax,
same indentation conventions. Likely the same doc tooling.

### Gemini (GenerateContent)

**Top-level URL:** `https://ai.google.dev/api/generate-content`
**Shared types URL:** `https://ai.google.dev/api/caching.md.txt#Content` (and similar)

**Format:** Different from OpenAI/Anthropic. Documentation is spread across multiple pages.
Shared types (like `Content`, `Part`) appear in multiple API pages. Has its own dedicated
parser (`parsers/gemini.py`) that produces the same intermediate JSON format.

## Intermediate Format (Extracted JSON)

Each extraction produces a JSON file representing the structural content of one API endpoint.
This is the unit of comparison for diffs.

### Schema

```json
{
  "meta": {
    "provider": "openai",
    "endpoint": "responses",
    "source_url": "https://...",
    "fetched_at": "2026-02-17T12:00:00Z",
    "raw_file": "docs/specs/protocols/openai/responses.md"
  },
  "request": {
    "objects": { ... },
    "enums": { ... }
  },
  "response": {
    "objects": { ... },
    "enums": { ... }
  }
}
```

### Object Definition

```json
{
  "Request": {
    "fields": {
      "model": {
        "type": "string",
        "required": true,
        "description": "Model ID used for this response."
      },
      "temperature": {
        "type": "number",
        "required": false,
        "description": "Sampling temperature between 0 and 2."
      },
      "input": {
        "type": "string | array<ResponseInputItem>",
        "required": false,
        "description": "Text, image, or file inputs to the model."
      },
      "tools": {
        "type": "array<Tool>",
        "required": false,
        "description": "Tools available to the model."
      }
    }
  }
}
```

### Field Type System

Each field has a `type` and `required` flag, plus optional extra keys depending on the type:

**Fundamental types:** `"string"`, `"number"`, `"boolean"`, `"object"`, `"unknown"`.

**Compound types:** `"array<Foo>"`, `"map<string, unknown>"`.

**Named type references:** `"CacheControlEphemeral"`, `"ContentBlockParam"` — refers to another
object definition in the same extraction.

**Kind (discriminator):** A field whose value is always a fixed string literal.
```json
{ "type": "kind", "required": true, "value": "text" }
```

**String enum:** A field whose value is one of a known set of string literals.
```json
{ "type": "string", "required": false, "values": ["5m", "1h"] }
```

**Union:** A field whose value can be one of several distinct types. Type references are
unquoted; string literals are quoted to distinguish them from type names.
```json
{ "type": "union", "required": true, "values": ["string", "array<ContentBlockParam>"] }
{ "type": "union", "required": false, "values": ["object", "\"always\"", "\"never\""] }
```

**Derivation rule:** The inline type expression in the docs is treated as a summary. When
children are present (enum value bullets, `UnionMember` definitions), the children are
the authoritative source for the field's type. This handles truncated inline expressions
(e.g., `"foo" or "bar" or 3 more`) and ambiguous `array of X or Y` constructs.

### Enum Definition

```json
{
  "StopReason": {
    "values": ["end_turn", "max_tokens", "stop_sequence", "tool_use"],
    "source_field": "Response.stop_reason"
  }
}
```

Enum values are the literal strings from the spec. The `source_field` records where the enum
was first encountered (for traceability).

### Kind / Discriminator Capture

When a field has a fixed literal value (e.g., `type: "text"`), it is captured as:

```json
{
  "type": {
    "type": "kind",
    "required": true,
    "description": "Always text.",
    "value": "text"
  }
}
```

The `type: "kind"` distinguishes discriminator constants from regular string fields.

### Description Capture

The `description` field contains the **full verbatim text** of the field's documentation from
the spec. Markdown links are stripped, but all substantive prose is preserved including
constraint details (e.g., "maximum 64 characters", "must be a valid URL", "Unix timestamp in
seconds"), default values, and behavioral notes.

Descriptions are stored in the intermediate JSON and are available to the agent for making
type mapping decisions, but they are **not compared during structural diffs**. Only type,
required status, enum values, and object/field presence are diffed. This avoids noisy diffs
from editorial prose changes while preserving all information the agent needs.

## Diff Output

The diff stage compares two extracted JSON files (previous baseline vs current) and produces
a structured diff:

```json
{
  "added_objects": ["NewObjectType"],
  "removed_objects": ["DeprecatedType"],
  "added_fields": [
    { "object": "Request", "field": "new_param", "type": "string", "required": false },
    { "object": "Request", "field": "speed", "type": "string", "required": false,
      "values": ["standard", "fast"] },
    { "object": "Tool", "field": "type", "type": "kind", "required": true,
      "value": "custom" }
  ],
  "removed_fields": [
    { "object": "Request", "field": "old_param" }
  ],
  "changed_fields": [
    {
      "object": "Request",
      "field": "temperature",
      "changes": {
        "type": { "old": "number", "new": "number | null" },
        "required": { "old": true, "new": false }
      }
    },
    {
      "object": "Request",
      "field": "speed",
      "changes": {
        "values": { "old": ["standard"], "new": ["standard", "fast"] }
      }
    }
  ],
  "added_enum_values": [
    { "enum": "StopReason", "value": "new_reason" }
  ],
  "removed_enum_values": [],
  "description_changes": [
    {
      "object": "Request",
      "field": "user",
      "old_desc": "...",
      "new_desc": "..."
    }
  ]
}
```

## Report Output

A human-readable markdown report generated from the diff:

```markdown
# API Parity Report: OpenAI Responses
Generated: 2026-02-17
Baseline: 2026-01-15  |  Current: 2026-02-17

## New Fields (3)
- Request.new_param: string (optional)
- Response.new_field: number (required)
- OutputMessage.metadata: object (optional)

## Removed Fields (1)
- Request.deprecated_field

## Changed Fields (1)
- Request.temperature: required → optional

## New Enum Values (2)
- StopReason: "new_reason"
- ServiceTier: "premium"

## Description Changes (1)
- Request.user: description changed (see diff JSON for details)

## No Changes
(listed if nothing changed — confirms the audit ran successfully)
```

## File Layout

```
scripts/
  api_parity/
    __init__.py
    config.py              Provider URLs, endpoint definitions, path helpers
    fetch.py               Stage 1: download raw markdown
    extract.py             Stage 2: markdown → intermediate JSON
    diff.py                Stage 3: compare two extractions
    report.py              Stage 4: diff → human-readable report
    checklist.py           Stage 5: diff → integration checklist
    run.py                 CLI entry point: run full pipeline or individual stages
    parsers/
      __init__.py
      openai_anthropic.py  Shared parser for OpenAI/Anthropic markdown format
      gemini.py            Gemini-specific parser

docs/specs/
  protocols/
    openai/
      responses.baseline.json   Approved baseline extraction
      responses.baseline.md     Markdown snapshot paired with baseline
    anthropic/
      messages.baseline.json
      messages.baseline.md
    gemini/
      generate_content.baseline.json
      generate_content.baseline.md

scratch/
  api_parity/               Working files during pipeline runs (all transient)
    openai/                 Fetched markdown + extracted JSON
    anthropic/              Fetched markdown + extracted JSON
    gemini/                 Fetched markdown + extracted JSON
    diffs/                  Diff output JSONs
    reports/                Generated markdown reports
    checklists/             Generated integration checklists
```

## Workflow (Manual)

### Initial Setup (One Time)

```bash
# Fetch current specs and extract (writes to scratch/)
python scripts/api_parity/run.py fetch --provider openai
python scripts/api_parity/run.py extract --provider openai

# Promote scratch extraction + markdown to docs/specs/ as approved baseline
python scripts/api_parity/run.py promote --provider openai
```

Promote copies the extracted JSON as `<endpoint>.baseline.json` and the fetched markdown
as `<endpoint>.baseline.md` into `docs/specs/protocols/<provider>/`. The markdown snapshot
ensures the baseline can be traced back to the exact documentation version it was derived from.

### Regular Audit

```bash
# Fetch latest, extract, diff against baseline, generate report
# All working files land in scratch/api_parity/
python scripts/api_parity/run.py audit --provider openai

# Review the report
cat scratch/api_parity/reports/openai_responses_report.md

# Generate integration checklist from the diff
python scripts/api_parity/run.py checklist --provider openai

# Review the checklist — this is the agent's work plan
cat scratch/api_parity/checklists/openai_responses_checklist.md

# After incorporating changes into C++, promote as new baseline
python scripts/api_parity/run.py promote --provider openai
```

### Full Pipeline (All Providers)

```bash
python scripts/api_parity/run.py audit --all
```

## Checklist Step (Stage 5)

The checklist is generated mechanically from the diff JSON. Each item in the diff becomes
one or more checklist entries with the specific C++ files that need updating. Nothing from
the diff is deferred or skipped.

```bash
python scripts/api_parity/run.py checklist --provider openai
```

Output: `scratch/api_parity/checklists/<provider>_<endpoint>_checklist.md`

The checklist groups items by change type (new objects, new fields, new enum values, etc.)
and lists the affected files for each. It also includes variant membership analysis — when
a new type needs to be added to an existing `std::variant`, the checklist flags it.

See `docs/api_parity_integration.md` for the full integration process, file mapping, type
mapping conventions, and recommended ordering.

## Agent Step (Stage 6)

The agent works through the checklist item by item, marking each done as it goes. The
checklist is the single source of truth — the agent does not make decisions about what to
implement and what to skip.

**Inputs:** The checklist (markdown), the current extracted JSON (for field descriptions
and type details), and the existing C++ source files.

**Agent responsibilities:**
- Work through every checklist item in order
- Add new structs/fields to protocol headers
- Remove deprecated fields
- Update type mappings (using description hints like "max 64 characters" → `Name64`)
- Update enum/kind definitions and string conversions
- Add/update ser/de macros for new fields
- Update unit tests
- Mark items done and save the checklist periodically

**Verification:** After all items are done, the agent builds, runs tests, and re-audits
to confirm the diff is now empty.

## Future Extensions

- **GitHub Actions**: Scheduled weekly audit that opens an issue or PR when changes are detected.
- **LLM agent integration**: Feed report + JSON + C++ headers to an LLM to generate patches.
- **Streaming/event specs**: Extend the intermediate format to capture SSE event types.
- **Deprecated field filtering**: Detect and flag deprecated fields in extractions.
- **Nullable field tracking**: Distinguish nullable from optional in the intermediate format.
