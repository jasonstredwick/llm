"""
Parser for Google Gemini markdown documentation format.

Gemini docs use a different structure from OpenAI/Anthropic:
  - Type definitions are under ## headings (e.g., ## GenerateContentResponse)
  - Fields are inline: `fieldName` `type`\\n Description...
  - Array fields: `fieldName[]` `type`
  - Enums are markdown tables: | Enums || / |---|---| / | `VALUE` | desc |
  - The request body is under ### Request body within ## Method: sections
  - Type references can link to other pages (e.g., /api/caching#Content)
  - Code examples, JSON representation tables, and TOC link blocks are skipped
  - The ## Method: models.streamGenerateContent section is skipped (mirrors generateContent)

Format divergence detection:
  If the document structure shifts toward the OpenAI/Anthropic format (indentation-based
  field lists with `- \\`name: type\\``), the parser raises FormatDivergenceError so the
  pipeline can alert the maintainer to consider migrating to the shared parser.

Produces the same intermediate JSON format as openai_anthropic.py.
"""

import re
from typing import Any


# ---------------------------------------------------------------------------
# Format divergence detection
# ---------------------------------------------------------------------------

class FormatDivergenceError(Exception):
    """Raised when the Gemini doc format appears to have changed significantly."""
    pass


def _check_format_divergence(text: str) -> None:
    """
    Heuristic check: if the document looks like it has switched to the
    OpenAI/Anthropic indentation-based format, raise an error rather than
    silently producing garbage.

    Checks for:
      1. Presence of `### Body Parameters` or `### Returns` section headers
         (OpenAI/Anthropic convention)
      2. High density of `- \\`name: type\\`` field patterns (indentation-based)
      3. Absence of the expected Gemini `## Method:` pattern
    """
    lines = text.split('\n')

    # Check for OpenAI/Anthropic section headers
    oa_section_count = 0
    gemini_method_count = 0
    indented_field_count = 0
    gemini_field_count = 0

    for line in lines:
        stripped = line.strip()
        if re.match(r'^###\s+(Body Parameters|Returns)\s*$', stripped):
            oa_section_count += 1
        if re.match(r'^##\s+Method:', stripped):
            gemini_method_count += 1
        if re.match(r'^\s*-\s+`\w+:\s+', line):
            indented_field_count += 1
        if re.match(r'^`\w+(?:\[\])?\`\s+`', stripped):
            gemini_field_count += 1

    # If we see OpenAI/Anthropic patterns and no Gemini patterns, flag it
    if oa_section_count >= 2 and gemini_method_count == 0:
        raise FormatDivergenceError(
            f"Gemini doc appears to have adopted OpenAI/Anthropic format "
            f"(found {oa_section_count} OA-style sections, 0 Gemini Method: sections). "
            f"Consider using the openai_anthropic parser instead."
        )

    # If indentation-based fields dominate over Gemini-style fields
    if indented_field_count > 20 and gemini_field_count < 5:
        raise FormatDivergenceError(
            f"Gemini doc appears to use indentation-based field definitions "
            f"({indented_field_count} OA-style vs {gemini_field_count} Gemini-style). "
            f"Consider using the openai_anthropic parser instead."
        )


# ---------------------------------------------------------------------------
# Line-level patterns
# ---------------------------------------------------------------------------

# Field definition: `fieldName` `type (Link)` or `fieldName[]` `type`
# Captures:
#   Group 1: field name (e.g., "contents", "tools")
#   Group 2: array marker ([] or empty)
#   Group 3: everything after the field name backtick block = the type info
_FIELD_RE = re.compile(
    r'^`(\w+)((?:\[\])?)`\s+`(.+?)`\s*$'
)

# Alternate field pattern where the type continues with a markdown link after
# the closing backtick: `name` `object (`[Type](url)`)`
# We handle this by also matching lines that start with a field pattern and
# have trailing content.
_FIELD_FULL_RE = re.compile(
    r'^`(\w+)((?:\[\])?)`\s+(.+?)(?:\s\s|$)'
)

# Enum table row: | `VALUE` | description |
_ENUM_ROW_RE = re.compile(
    r'^\|\s*`([^`]+)`\s*\|\s*(.*?)\s*\|'
)

# Enum table header: | Enums || or | Enums | |
_ENUM_HEADER_RE = re.compile(
    r'^\|\s*Enums\s*\|', re.IGNORECASE
)

# Section heading ## TypeName
_H2_RE = re.compile(r'^##\s+(.+)$')

# Sub-section heading ### ...
_H3_RE = re.compile(r'^###\s+(.+)$')

# TOC link line (starts with - [ and links to the same page)
_TOC_LINK_RE = re.compile(r'^-\s+\[.*?\]\(https?://.*?\)')

# JSON representation table header
_JSON_REPR_RE = re.compile(r'^\|\s*JSON representation\s*\|')

# Markdown link: [Text](url)
_MD_LINK_RE = re.compile(r'\[([^\]]+)\]\(([^)]+)\)')


# ---------------------------------------------------------------------------
# Type expression parsing
# ---------------------------------------------------------------------------

def _parse_type_expr(raw: str) -> tuple[str, bool, str | None]:
    """
    Parse a Gemini type expression into (normalized_type, is_optional, url_fragment).

    The url_fragment (e.g., "v1beta.Candidate" or "Candidate") is preserved
    from markdown links so that a later resolution pass can map type names
    to their qualified form (e.g., "TopCandidates::Candidate").

    Input forms:
        `string`
        `integer`
        `number`
        `boolean`
        `object (TypeName)` or `object (`[TypeName](url)`)`
        `enum (EnumName)` or `enum (`[EnumName](url)`)`
        `value (Value format)` — protobuf Value
    """
    s = raw.strip()

    # Remove surrounding backticks if present
    if s.startswith('`') and s.endswith('`'):
        s = s[1:-1].strip()

    # Extract URL fragment from markdown link if present
    fragment: str | None = None
    link_match = _MD_LINK_RE.search(s)
    if link_match:
        url = link_match.group(2)
        if '#' in url:
            fragment = url.split('#', 1)[1]

    # Strip markdown links, keeping just the text
    s = _MD_LINK_RE.sub(r'\1', s)

    # Remove backticks that may remain inside
    s = s.replace('`', '').strip()

    # "object (TypeName)" -> "TypeName"
    m = re.match(r'^object\s*\(\s*(\w+)\s*\)$', s)
    if m:
        return m.group(1), False, fragment

    # "enum (EnumName)" -> "EnumName"
    m = re.match(r'^enum\s*\(\s*(\w+)\s*\)$', s)
    if m:
        return m.group(1), False, fragment

    # "value (Value format)" -> "value"
    if s.startswith('value'):
        return 'value', False, None

    # Primitives
    if s in ('string', 'integer', 'number', 'boolean', 'float'):
        return s, False, None

    # Fallback: return as-is
    return s, False, fragment


def _strip_markdown_links(text: str) -> str:
    """Convert [text](url) to just text."""
    return _MD_LINK_RE.sub(r'\1', text)


# ---------------------------------------------------------------------------
# External reference tracking
# ---------------------------------------------------------------------------

def _extract_external_refs(text: str, self_page: str) -> dict[str, str]:
    """
    Find all type references that point to external pages.

    Returns a dict of {TypeName: page_url} for types not defined on self_page.
    """
    refs: dict[str, str] = {}
    for match in _MD_LINK_RE.finditer(text):
        name = match.group(1)
        url = match.group(2)
        # Only care about ai.google.dev/api/ links
        if 'ai.google.dev/api/' not in url:
            continue
        base = url.split('#')[0]
        if self_page not in base:
            refs[name] = url
    return refs


# ---------------------------------------------------------------------------
# Section splitting and ownership
# ---------------------------------------------------------------------------

def _split_h2_sections(lines: list[str]) -> list[tuple[str, int, int]]:
    """
    Split the document into (heading_text, start_line, end_line) tuples
    for each ## heading. start_line is the line after the heading.
    """
    sections: list[tuple[str, int, int]] = []
    for i, line in enumerate(lines):
        m = _H2_RE.match(line)
        if m:
            if sections:
                # Close previous section
                prev_name, prev_start, _ = sections[-1]
                sections[-1] = (prev_name, prev_start, i)
            sections.append((m.group(1).strip(), i + 1, len(lines)))
    return sections


# TOC link with capture: - [TypeName](url) — captures name and indentation
_TOC_TYPE_LINK_RE = re.compile(
    r'^(\s*)-\s+\[(\w+)\]\(https?://[^)]*#([^)]+)\)'
)


def _build_ownership_map(
    lines: list[str],
    sections: list[tuple[str, int, int]]
) -> dict[tuple[str, int], str]:
    """
    Build a map from (type_name, occurrence_index) to its owning parent type.

    Parses the TOC link blocks at the start of each ## section to determine
    which child types are nested under which parent. This is used to produce
    qualified names for duplicate ## headings.

    For example, if ## LogprobsResult's TOC lists:
      - [TopCandidates](...)
        - [Candidate](...)  ← depth 1 child
    Then the second ## Candidate heading gets parent "LogprobsResult".

    Returns:
        Dict mapping (heading_text, section_index) -> owning_parent_name.
        Only entries for types that appear more than once are included.
    """
    from collections import Counter

    # First pass: count heading occurrences to find duplicates
    heading_counts = Counter(h for h, _, _ in sections)
    duplicate_headings = {h for h, c in heading_counts.items() if c > 1}

    if not duplicate_headings:
        return {}

    # Build a map: for each section, parse its TOC to find what type names
    # it claims as direct children (top-level TOC entries, ignoring JSON repr)
    section_children: dict[str, list[str]] = {}
    for heading, sec_start, sec_end in sections:
        if heading.startswith('Method:') or heading.startswith('REST Resource'):
            continue

        children: list[str] = []
        for i in range(sec_start, min(sec_end, sec_start + 60)):
            line = lines[i]
            stripped = line.strip()

            # Stop at non-TOC content
            if stripped and not _TOC_LINK_RE.match(stripped):
                break
            if not stripped:
                # Blank line inside TOC — could be end of TOC
                # Peek ahead
                if i + 1 < sec_end and not _TOC_LINK_RE.match(lines[i + 1].strip()):
                    break
                continue

            m = _TOC_TYPE_LINK_RE.match(line)
            if m:
                indent = len(m.group(1))
                child_name = m.group(2)
                fragment = m.group(3)

                # Skip JSON representation links and code snippet links
                if 'SCHEMA_REPRESENTATION' in fragment:
                    continue
                if 'codeSnippets' in fragment:
                    continue

                # All TOC entries (any indent) are descendants of this section
                if child_name != heading:
                    children.append(child_name)

        if children:
            section_children[heading] = children

    # Now for each duplicate heading, find which parent owns it
    # A type is owned by the nearest preceding section whose TOC lists it
    ownership: dict[tuple[str, int], str] = {}
    occurrence_counter: dict[str, int] = {}

    for idx, (heading, _, _) in enumerate(sections):
        count = occurrence_counter.get(heading, 0)
        occurrence_counter[heading] = count + 1

        if heading not in duplicate_headings:
            continue

        occurrence = count  # 0-based

        if occurrence == 0:
            # First occurrence — no parent qualifier needed
            continue

        # Find the parent using two strategies:
        #
        # 1. TOC-based: scan backwards for a section (with a different name)
        #    whose TOC lists this heading as a child.
        # 2. Field-reference fallback: scan backwards for the nearest section
        #    that references this type in one of its field definitions.
        #
        # Strategy 1 (TOC)
        found = False
        for prev_idx in range(idx - 1, -1, -1):
            prev_heading = sections[prev_idx][0]
            if prev_heading == heading:
                continue
            children = section_children.get(prev_heading, [])
            if heading in children:
                ownership[(heading, occurrence)] = prev_heading
                found = True
                break

        if not found:
            # Strategy 2 (field reference): check the immediately preceding
            # section's body for a field whose type links to this heading.
            for prev_idx in range(idx - 1, max(-1, idx - 4), -1):
                prev_heading = sections[prev_idx][0]
                if prev_heading == heading:
                    continue
                _, prev_start, prev_end = sections[prev_idx]
                # Scan the section body for field type references
                for li in range(prev_start, prev_end):
                    line = lines[li]
                    # Look for type references like `object (Heading)` or
                    # markdown links [Heading](url#fragment)
                    if re.search(
                        rf'\[{re.escape(heading)}\]\(', line
                    ):
                        ownership[(heading, occurrence)] = prev_heading
                        found = True
                        break
                if found:
                    break

    return ownership


# ---------------------------------------------------------------------------
# Field parsing
# ---------------------------------------------------------------------------

def _parse_fields_block(lines: list[str], start: int, end: int,
                        type_name: str) -> tuple[dict[str, Any], int]:
    """
    Parse a block of Gemini field definitions.

    Returns (fields_dict, position_after_last_field).
    """
    fields: dict[str, Any] = {}
    i = start

    while i < end:
        line = lines[i]
        stripped = line.strip()

        # Stop at JSON representation table
        if _JSON_REPR_RE.match(stripped):
            break

        # Stop at next ## heading
        if _H2_RE.match(stripped):
            break

        # Skip blank lines
        if not stripped:
            i += 1
            continue

        # Skip TOC link lines
        if _TOC_LINK_RE.match(stripped):
            i += 1
            continue

        # Skip ### sub-headings that aren't field-related
        if _H3_RE.match(stripped):
            break

        # Try to match a field definition
        field_match = _match_field_line(stripped)
        if field_match:
            field_name, is_array, type_raw = field_match
            type_str, _, fragment = _parse_type_expr(type_raw)

            if is_array:
                type_str = f"array<{type_str}>"

            i += 1

            # Collect description lines
            desc_lines: list[str] = []
            while i < end:
                dline = lines[i]
                dstripped = dline.strip()

                # Stop at next field, heading, enum table, JSON repr, or TOC
                if _match_field_line(dstripped):
                    break
                if _H2_RE.match(dstripped) or _H3_RE.match(dstripped):
                    break
                if _JSON_REPR_RE.match(dstripped):
                    break
                if _ENUM_HEADER_RE.match(dstripped):
                    break
                if not dstripped:
                    # Blank line — peek ahead
                    if i + 1 < end and _match_field_line(lines[i + 1].strip()):
                        break
                    if i + 1 < end and not lines[i + 1].strip():
                        # Two blank lines — likely end of field descriptions
                        break
                    desc_lines.append('')
                    i += 1
                    continue

                desc_lines.append(dstripped)
                i += 1

            desc = _clean_description(desc_lines)

            # Determine required/optional from description
            optional = _is_optional(desc)

            field_entry: dict[str, Any] = {
                "type": type_str,
                "required": not optional,
            }
            if desc:
                field_entry["description"] = desc
            # Store fragment for later type reference resolution
            if fragment:
                field_entry["_fragment"] = fragment

            fields[field_name] = field_entry
            continue

        # Unrecognized line — skip
        i += 1

    return fields, i


def _match_field_line(stripped: str) -> tuple[str, bool, str] | None:
    """
    Try to match a Gemini field definition line.

    Returns (field_name, is_array, type_raw) or None.
    """
    # Pattern: `fieldName[]` `type...`  or `fieldName` `type...`
    # The type portion may contain markdown links and backticks mixed in.

    # First try the simple backtick-enclosed pattern
    m = _FIELD_RE.match(stripped)
    if m:
        name = m.group(1)
        is_array = bool(m.group(2))
        type_raw = m.group(3)
        return name, is_array, type_raw

    # Try the fuller pattern where type has markdown links
    m = _FIELD_FULL_RE.match(stripped)
    if m:
        name = m.group(1)
        is_array = bool(m.group(2))
        type_raw = m.group(3)
        return name, is_array, type_raw

    return None


def _is_optional(desc: str) -> bool:
    """Check if a field is optional based on description text."""
    lower = desc.lower()
    if lower.startswith('optional'):
        return True
    if lower.startswith('optional.'):
        return True
    if 'optional.' in lower[:30]:
        return True
    return False


def _clean_description(lines: list[str]) -> str:
    """Join and clean description lines."""
    text = ' '.join(line for line in lines if line).strip()
    text = _strip_markdown_links(text)
    text = re.sub(r'\s+', ' ', text).strip()
    return text


# ---------------------------------------------------------------------------
# Enum parsing
# ---------------------------------------------------------------------------

def _parse_enum_table(lines: list[str], start: int, end: int) -> list[str]:
    """
    Parse a Gemini enum table starting at the | Enums || header line.

    Returns list of enum value strings.
    """
    values: list[str] = []
    i = start

    # Skip the header row and separator row
    while i < end:
        stripped = lines[i].strip()
        if _ENUM_HEADER_RE.match(stripped):
            i += 1
            continue
        if stripped.startswith('|---') or stripped == '|---|---|':
            i += 1
            continue
        break

    # Parse value rows
    while i < end:
        stripped = lines[i].strip()
        if not stripped or not stripped.startswith('|'):
            break
        m = _ENUM_ROW_RE.match(stripped)
        if m:
            values.append(m.group(1))
        i += 1

    return values


# ---------------------------------------------------------------------------
# Section classification and parsing
# ---------------------------------------------------------------------------

def _parse_type_section(lines: list[str], start: int, end: int,
                        heading: str) -> tuple[dict | None, dict | None]:
    """
    Parse a ## TypeName section, returning (object_entry, enum_entry).
    Exactly one will be non-None.
    """
    # Scan for enum table vs fields
    has_enum_table = False
    has_fields = False
    enum_table_start = -1
    fields_start = -1

    # Skip TOC links at the beginning
    i = start
    while i < end:
        stripped = lines[i].strip()
        if not stripped or _TOC_LINK_RE.match(stripped):
            i += 1
            continue
        break

    # Now i points to the first content line after TOC
    content_start = i

    # Scan for structural markers
    for j in range(content_start, end):
        stripped = lines[j].strip()
        if _ENUM_HEADER_RE.match(stripped):
            has_enum_table = True
            enum_table_start = j
            break
        if stripped.startswith('Fields ') or stripped == 'Fields':
            has_fields = True
            fields_start = j
            break

    if has_enum_table:
        # Parse enum
        values = _parse_enum_table(lines, enum_table_start, end)
        if values:
            # Collect description (text between content_start and enum table)
            desc_lines = []
            for j in range(content_start, enum_table_start):
                stripped = lines[j].strip()
                if stripped and not _TOC_LINK_RE.match(stripped):
                    desc_lines.append(stripped)
            desc = _clean_description(desc_lines)

            enum_entry: dict[str, Any] = {"values": values}
            if desc:
                enum_entry["description"] = desc
            return None, enum_entry

    if has_fields:
        # The "Fields" marker may be on the same line as the first field
        # e.g., "Fields `contents[]` `object (Content)`"
        # Or on its own line followed by field lines
        first_field_line = lines[fields_start].strip()

        # Strip the "Fields" prefix if it's on the same line as a field
        if first_field_line.startswith('Fields '):
            remaining = first_field_line[len('Fields '):]
            # Replace the line temporarily for parsing
            original = lines[fields_start]
            lines[fields_start] = remaining
            fields, _ = _parse_fields_block(lines, fields_start, end, heading)
            lines[fields_start] = original  # restore
        else:
            fields, _ = _parse_fields_block(lines, fields_start + 1, end, heading)

        if fields:
            # Collect description (text between content_start and fields_start)
            desc_lines = []
            for j in range(content_start, fields_start):
                stripped = lines[j].strip()
                if stripped and not _TOC_LINK_RE.match(stripped):
                    desc_lines.append(stripped)
            desc = _clean_description(desc_lines)

            obj_entry: dict[str, Any] = {"fields": fields}
            if desc:
                obj_entry["description"] = desc
            return obj_entry, None

    return None, None


# ---------------------------------------------------------------------------
# Request body extraction
# ---------------------------------------------------------------------------

def _find_request_body(lines: list[str], method_start: int,
                       method_end: int) -> tuple[int, int] | None:
    """
    Find the ### Request body section within a Method section.
    Returns (fields_start, section_end) or None.
    """
    for i in range(method_start, method_end):
        m = _H3_RE.match(lines[i])
        if m and m.group(1).strip() == 'Request body':
            # Find where this subsection ends (next ### or ## or end)
            sub_end = method_end
            for j in range(i + 1, method_end):
                if _H3_RE.match(lines[j]) or _H2_RE.match(lines[j]):
                    sub_end = j
                    break
            return i + 1, sub_end
    return None


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def _resolve_name(heading: str, occurrence: int,
                  ownership: dict[tuple[str, int], str]) -> str:
    """
    Resolve a type name, qualifying duplicates with their owning parent.

    First occurrence of a name is used as-is.  Subsequent occurrences are
    prefixed with the parent type from the TOC ownership map, falling back
    to a numeric suffix if no parent was found.

    Examples:
        ("Candidate", 0, ...) -> "Candidate"
        ("Candidate", 1, {("Candidate",1): "LogprobsResult"})
            -> "LogprobsResult::Candidate"
        ("Modality", 1, {("Modality",1): "ModalityTokenCount"})
            -> "ModalityTokenCount::Modality"

    The "::" separator is chosen to mirror C++ nesting syntax and avoid
    ambiguity with underscores that may appear in type names.
    """
    if occurrence == 0:
        return heading
    parent = ownership.get((heading, occurrence))
    if parent:
        return f"{parent}::{heading}"
    return f"{heading}::{occurrence + 1}"


def extract(markdown_text: str, provider: str, endpoint: str,
            source_url: str, raw_file: str) -> dict[str, Any]:
    """
    Parse Gemini markdown documentation and return the intermediate JSON structure.

    Produces the same format as openai_anthropic.extract().
    """
    from datetime import datetime, timezone

    # Format divergence check
    _check_format_divergence(markdown_text)

    lines = markdown_text.split('\n')
    sections = _split_h2_sections(lines)
    ownership = _build_ownership_map(lines, sections)

    req_objects: dict[str, Any] = {}
    req_enums: dict[str, Any] = {}
    resp_objects: dict[str, Any] = {}
    resp_enums: dict[str, Any] = {}

    # Track which sections are Method sections vs type definitions
    # The response type boundary: everything from GenerateContentResponse onward
    # is response-side. The request body fields come from the Method section.
    response_section_started = False

    # Track per-heading occurrence index for duplicate resolution
    heading_occurrence: dict[str, int] = {}

    for heading, sec_start, sec_end in sections:
        # Skip the stream method — it mirrors generateContent
        if heading.startswith('Method:') and 'stream' in heading.lower():
            continue

        # Method section — extract request body
        if heading.startswith('Method:'):
            req_body = _find_request_body(lines, sec_start, sec_end)
            if req_body:
                rb_start, rb_end = req_body
                # Find "Fields" within the request body section
                for k in range(rb_start, rb_end):
                    stripped = lines[k].strip()
                    if stripped.startswith('Fields ') or stripped == 'Fields':
                        if stripped.startswith('Fields '):
                            remaining = stripped[len('Fields '):]
                            original = lines[k]
                            lines[k] = remaining
                            fields, _ = _parse_fields_block(
                                lines, k, rb_end, "Request")
                            lines[k] = original
                        else:
                            fields, _ = _parse_fields_block(
                                lines, k + 1, rb_end, "Request")
                        if fields:
                            req_objects["Request"] = {"fields": fields}
                        break
            continue

        # REST Resource section — skip
        if heading.startswith('REST Resource'):
            continue

        # Resource: CachedContent etc — skip
        if heading.startswith('Resource:'):
            continue

        # GenerateContentResponse marks the start of response types
        if heading == 'GenerateContentResponse':
            response_section_started = True

        # Track occurrence index for this heading
        occ = heading_occurrence.get(heading, 0)
        heading_occurrence[heading] = occ + 1
        key = _resolve_name(heading, occ, ownership)

        # Parse the type section
        obj_entry, enum_entry = _parse_type_section(
            lines, sec_start, sec_end, heading)

        if response_section_started:
            if obj_entry:
                resp_objects[key] = obj_entry
            if enum_entry:
                resp_enums[key] = enum_entry
        else:
            if obj_entry:
                req_objects[key] = obj_entry
            if enum_entry:
                req_enums[key] = enum_entry

    # --- Type reference resolution pass ---
    # Build a fragment → qualified_name map from all parsed types.
    # Fragments come from the URL anchors in Google's docs (e.g.,
    # "v1beta.Candidate" or "Candidate" or "FinishReason").
    # When a fragment uniquely identifies a qualified name that differs
    # from the bare type name, we can resolve the reference.
    _resolve_type_references(req_objects, resp_objects, req_enums, resp_enums)

    # Collect external references for metadata
    external_refs = _extract_external_refs(
        markdown_text,
        self_page='generate-content'
    )

    result: dict[str, Any] = {
        "meta": {
            "provider": provider,
            "endpoint": endpoint,
            "source_url": source_url,
            "fetched_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "raw_file": raw_file,
        },
        "request": {"objects": req_objects, "enums": req_enums},
        "response": {"objects": resp_objects, "enums": resp_enums},
    }

    if external_refs:
        result["meta"]["external_type_refs"] = external_refs

    return result


def _resolve_type_references(
    req_objects: dict, resp_objects: dict,
    req_enums: dict, resp_enums: dict,
) -> None:
    """
    Post-processing pass: resolve type references in field entries using
    the URL fragments and containing-object context stored during parsing.

    Resolution strategy:
      1. v1beta.X fragment → always resolves to the un-qualified name "X"
      2. Bare X fragment where X has qualified variants:
         - If the containing object is the parent in "Parent::X", resolve
           to "Parent::X" (the type is local to this object's subtree)
         - Otherwise resolve to the un-qualified "X"
      3. Bare X fragment with no qualified variants → "X" (no ambiguity)

    Removes temporary _fragment keys from field entries.
    """
    # Collect all known qualified names (both objects and enums, both sides)
    all_qualified: set[str] = set()
    for d in (req_objects, resp_objects, req_enums, resp_enums):
        all_qualified.update(d.keys())

    # Build lookup structures:
    # - v1beta_map: "v1beta.X" → "X" (the un-qualified form)
    # - qualified_by_bare: "X" → ["Parent::X", ...] (all qualified forms)
    v1beta_map: dict[str, str] = {}
    qualified_by_bare: dict[str, list[str]] = {}

    for qname in all_qualified:
        if '::' in qname:
            bare = qname.split('::')[-1]
            qualified_by_bare.setdefault(bare, []).append(qname)
        else:
            v1beta_map[f"v1beta.{qname}"] = qname

    # Walk all field entries and resolve type references
    for d in (req_objects, resp_objects):
        for obj_name, obj_entry in d.items():
            fields = obj_entry.get("fields", {})
            for field_name, field_entry in fields.items():
                fragment = field_entry.pop("_fragment", None)
                if not fragment:
                    continue

                # Strategy 1: v1beta. prefix → un-qualified name
                if fragment in v1beta_map:
                    # The v1beta. form always means the primary/un-qualified type.
                    # No change needed since the type string already uses the
                    # un-qualified name.
                    continue

                # Extract the type name from the fragment (strip any prefix)
                # Fragments are like "Candidate", "FinishReason", etc.
                frag_name = fragment.split('.')[-1] if '.' in fragment else fragment

                # Strategy 2: bare fragment with qualified variants
                variants = qualified_by_bare.get(frag_name, [])
                if not variants:
                    continue  # No ambiguity

                # Check if the containing object is the parent of a variant,
                # or is in the same ownership subtree (ancestor relationship).
                # e.g., obj_name="TopCandidates" and variant="TopCandidates::Candidate"
                # or obj_name="LogprobsResult" which owns TopCandidates which owns Candidate
                resolved = None
                for variant in variants:
                    parent = variant.split('::')[0]
                    if parent == obj_name:
                        # Direct parent match
                        resolved = variant
                        break
                    # Check if obj_name is an ancestor: does obj_name's
                    # subtree contain the parent of this variant?
                    # We check if parent appears as a qualified child of obj_name
                    # or if obj_name's type section listed parent as a child.
                    # Simple heuristic: if obj_name has a field whose type
                    # is or contains the parent name, it's an ancestor.
                    obj_fields = d.get(obj_name, {}).get("fields", {})
                    for f in obj_fields.values():
                        ftype = f.get("type", "")
                        if parent in ftype:
                            resolved = variant
                            break
                    if resolved:
                        break

                if not resolved:
                    # Not in the ownership subtree — leave as un-qualified
                    continue

                # Replace the bare type name with the qualified one
                old_type = field_entry["type"]
                bare_name = frag_name
                if bare_name in old_type:
                    field_entry["type"] = old_type.replace(bare_name, resolved)
