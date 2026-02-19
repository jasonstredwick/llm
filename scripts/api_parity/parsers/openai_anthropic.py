"""
Parser for the OpenAI / Anthropic markdown documentation format.

Both providers use identical markdown structure:
  - Sections: ### Body Parameters (request), ### Returns (response)
  - Fields: `- \`name: optional type\`` with 2-space indentation per nesting level
  - Named types: `- \`TypeName = object { field1, field2 }\``
  - Named aliases: `- \`TypeName = string\`` or `- \`TypeName = array of Foo\``
  - Enum values: `- \`"literal_value"\``
  - Union types: `type1 or type2`
  - Kind fields: `type: "literal"` (fixed discriminator)

Produces the intermediate JSON format described in docs/api_parity_pipeline.md.
"""

import re
from typing import Any


# ---------------------------------------------------------------------------
# Line classification
# ---------------------------------------------------------------------------

# Match a field definition: `name: optional type_expression`
#   Group 1: indentation
#   Group 2: field name
#   Group 3: everything after the colon (optional + type)
_FIELD_RE = re.compile(
    r'^(\s*)- `(\w+):\s+(.*?)`\s*$'
)

# Match a named type definition: `TypeName = type_expression`
#   Group 1: indentation
#   Group 2: type name
#   Group 3: type expression (e.g., "object { ... }", "string", "array of Foo")
_NAMED_TYPE_RE = re.compile(
    r'^(\s*)- `(\w+)\s*=\s*(.*?)`\s*$'
)

# Match an enum literal value: `"value"`
#   Group 1: indentation
#   Group 2: the literal string value
_ENUM_VALUE_RE = re.compile(
    r'^(\s*)- `"([^"]+)"`\s*$'
)

# Match a section header
_SECTION_RE = re.compile(r'^###\s+(.+)$')


def _indent_level(indent_str: str) -> int:
    """Convert whitespace indentation to a nesting level (2 spaces per level)."""
    return len(indent_str) // 2


def _strip_markdown_links(text: str) -> str:
    """Convert [text](url) to just text."""
    return re.sub(r'\[([^\]]+)\]\([^)]+\)', r'\1', text)


def _clean_description(lines: list[str]) -> str:
    """
    Join description lines into a single string.
    Strip markdown links but preserve full text content.
    """
    text = ' '.join(line.strip() for line in lines if line.strip())
    text = _strip_markdown_links(text)
    # Collapse multiple spaces
    text = re.sub(r'\s+', ' ', text).strip()
    return text


def _parse_type_expr(raw: str) -> tuple[str, bool]:
    """
    Parse a type expression, returning (normalized_type, is_optional).

    Examples:
        "optional boolean"        -> ("boolean", True)
        "string or array of Foo"  -> ("string | array<Foo>", False)
        "optional string or Foo"  -> ("string | Foo", True)
        '"literal_value"'         -> ('"literal_value"', False)
    """
    s = raw.strip()

    # Check for optional prefix
    optional = False
    if s.startswith('optional '):
        optional = True
        s = s[len('optional '):]

    # Normalize the type expression
    s = _normalize_type(s)

    return s, optional


def _normalize_type(s: str) -> str:
    """
    Normalize a type expression to our intermediate notation.

    "array of Foo"       -> "array<Foo>"
    "string or number"   -> "string | number"
    "map[unknown]"       -> "map<string, unknown>"
    "object { a, b }"    -> "object"  (sub-fields parsed separately)
    """
    # Handle "or" unions — split, normalize each part, rejoin
    if ' or ' in s:
        parts = s.split(' or ')
        # Filter out "N more" truncation markers (not real types)
        parts = [p for p in parts if not re.match(r'^\d+\s+more$', p.strip())]
        normalized = [_normalize_type(p.strip()) for p in parts]
        return ' | '.join(normalized)

    # Handle "array of X"
    m = re.match(r'^array\s+of\s+(.+)$', s)
    if m:
        inner = _normalize_type(m.group(1).strip())
        return f"array<{inner}>"

    # Handle "object { ... }" — the fields are parsed from child lines
    if s.startswith('object'):
        return "object"

    # Handle "map[X]"
    m = re.match(r'^map\[(.+)\]$', s)
    if m:
        return f"map<string, {m.group(1)}>"

    # Literal string type (kind discriminator)
    if s.startswith('"') and s.endswith('"'):
        return s  # preserved as-is

    return s


# ---------------------------------------------------------------------------
# Structural extraction
# ---------------------------------------------------------------------------

class _ParseContext:
    """Holds state during parsing."""

    def __init__(self, lines: list[str]):
        self.lines = lines
        self.pos = 0
        self.objects: dict[str, dict] = {}
        self.enums: dict[str, dict] = {}

    def at_end(self) -> bool:
        return self.pos >= len(self.lines)

    def peek(self) -> str:
        if self.at_end():
            return ''
        return self.lines[self.pos]

    def advance(self) -> str:
        line = self.lines[self.pos]
        self.pos += 1
        return line


def _collect_description(ctx: _ParseContext, base_indent: int) -> str:
    """
    Collect description lines that follow a field definition.
    Description lines are non-empty, non-field lines at indent > base_indent,
    or blank lines between description paragraphs.
    """
    desc_lines: list[str] = []
    while not ctx.at_end():
        line = ctx.peek()

        # Blank line — might be between description paragraphs, peek ahead
        if not line.strip():
            ctx.advance()
            # If next line is still description-level text, include blank
            if not ctx.at_end():
                next_line = ctx.peek()
                if (next_line.strip()
                        and not _FIELD_RE.match(next_line)
                        and not _NAMED_TYPE_RE.match(next_line)
                        and not _ENUM_VALUE_RE.match(next_line)
                        and not _SECTION_RE.match(next_line)):
                    # Check indent
                    stripped = next_line.lstrip()
                    line_indent = len(next_line) - len(stripped)
                    field_char_indent = base_indent * 2 + 2  # field content starts here
                    if line_indent >= field_char_indent:
                        desc_lines.append('')
                        continue
                # Not a continuation — backtrack
                ctx.pos -= 1
                break
            else:
                ctx.pos -= 1
                break

        # Check if this is a new field/type/enum/section
        if (_FIELD_RE.match(line)
                or _NAMED_TYPE_RE.match(line)
                or _ENUM_VALUE_RE.match(line)
                or _SECTION_RE.match(line)):
            break

        # Must be indented description text
        stripped = line.lstrip()
        line_indent = len(line) - len(stripped)
        field_char_indent = base_indent * 2 + 2
        if line_indent >= field_char_indent:
            desc_lines.append(stripped)
            ctx.advance()
        else:
            break

    return _clean_description(desc_lines)


def _parse_fields(ctx: _ParseContext, parent_indent: int, object_name: str) -> dict[str, Any]:
    """
    Parse fields at a given indentation level, returning a fields dict.
    Also registers sub-objects and enums in ctx.
    """
    fields: dict[str, Any] = {}

    while not ctx.at_end():
        line = ctx.peek()

        # Blank line — skip
        if not line.strip():
            ctx.advance()
            continue

        # Section header — stop
        if _SECTION_RE.match(line):
            break

        # Try matching a field definition
        m_field = _FIELD_RE.match(line)
        if m_field:
            indent = _indent_level(m_field.group(1))
            if indent < parent_indent:
                break  # outdented — belongs to parent
            if indent > parent_indent:
                # Deeper than expected — skip (part of a sub-object we already parsed)
                ctx.advance()
                continue
            # This field is at our level
            ctx.advance()
            name = m_field.group(2)
            type_raw = m_field.group(3)
            type_str, optional = _parse_type_expr(type_raw)

            # Collect description
            desc = _collect_description(ctx, indent)

            field_entry: dict[str, Any] = {
                "type": type_str,
                "required": not optional,
            }
            if desc:
                field_entry["description"] = desc

            # Normalize type representations
            if (type_str.startswith('"') and type_str.endswith('"')
                    and ' | ' not in type_str):
                # Single literal → kind/discriminator field
                field_entry["type"] = "kind"
                field_entry["value"] = type_str.strip('"')
            elif ' | ' in type_str:
                parts = [p.strip() for p in type_str.split(' | ')]
                if all(p.startswith('"') and p.endswith('"') for p in parts):
                    # All quoted literals → string enum
                    field_entry["type"] = "string"
                    field_entry["values"] = [p.strip('"') for p in parts]
                else:
                    # Mixed union → union with values list
                    # Preserve quotes on literals to distinguish type refs from strings
                    field_entry["type"] = "union"
                    field_entry["values"] = parts

            fields[name] = field_entry

            # Parse children (enum values or sub-fields)
            _parse_children(ctx, indent, name, object_name, field_entry)
            continue

        # Try matching a named type definition
        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            indent = _indent_level(m_named.group(1))
            if indent < parent_indent:
                break
            if indent > parent_indent:
                ctx.advance()
                continue
            ctx.advance()
            type_name = m_named.group(2)
            type_expr = m_named.group(3)

            # Collect description
            desc = _collect_description(ctx, indent)

            # If it's an object definition, parse its fields
            if type_expr.startswith('object'):
                obj_fields = _parse_fields(ctx, indent + 1, type_name)
                obj_entry: dict[str, Any] = {"fields": obj_fields}
                if desc:
                    obj_entry["description"] = desc
                ctx.objects[type_name] = obj_entry
            # Otherwise it's a type alias (string, array of X, etc.)
            # These are union member annotations — but may contain named objects
            else:
                _scan_children_for_objects(ctx, indent)
            continue

        # Try matching an enum value
        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent < parent_indent:
                break
            # Enum values at this level — skip them (handled by parent)
            ctx.advance()
            continue

        # Unrecognized line — could be description text or continuation
        # Check if indented beyond our level
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= parent_indent * 2 + 2:
            ctx.advance()  # skip description/continuation text
            continue
        else:
            break  # outdented, stop

    return fields


def _parse_children(ctx: _ParseContext, parent_indent: int, field_name: str,
                    object_name: str, field_entry: dict) -> None:
    """
    After parsing a field, handle its children:
    - Enum literal values
    - Named sub-type definitions (objects, union members)
    - Sub-fields (for inline objects)

    Rebuilds the field type from children when they provide more complete
    information than the (potentially truncated) inline type expression.
    """
    child_indent = parent_indent + 1
    collected_enum_values: list[str] = []
    collected_union_members: list[tuple] = []  # ('type', str) or ('enum', list[str])

    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue

        if _SECTION_RE.match(line):
            break

        # Check indentation
        m_field = _FIELD_RE.match(line)
        m_named = _NAMED_TYPE_RE.match(line)
        m_enum = _ENUM_VALUE_RE.match(line)

        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent >= child_indent:
                collected_enum_values.append(m_enum.group(2))
                ctx.advance()
                continue
            else:
                break

        if m_named:
            indent = _indent_level(m_named.group(1))
            if indent >= child_indent:
                ctx.advance()
                type_name = m_named.group(2)
                type_expr = m_named.group(3)
                desc = _collect_description(ctx, indent)

                scan_result = _ScanResult()
                if type_expr.startswith('object'):
                    obj_fields = _parse_fields(ctx, indent + 1, type_name)
                    obj_entry: dict[str, Any] = {"fields": obj_fields}
                    if desc:
                        obj_entry["description"] = desc
                    ctx.objects[type_name] = obj_entry
                else:
                    scan_result = _scan_children_for_objects(ctx, indent)

                # Track UnionMember type expressions for rebuilding unions
                if type_name.startswith('UnionMember'):
                    if scan_result.union_member_types:
                        # Nested UnionMember children rebuilt the type
                        normalized = _normalize_type(type_expr)
                        inner = ' | '.join(scan_result.union_member_types)
                        if normalized.startswith('array<'):
                            collected_union_members.append(
                                ('type', f"array<{inner}>"))
                        else:
                            collected_union_members.append(('type', inner))
                    elif scan_result.enum_values:
                        # Children provided the full enum values
                        collected_union_members.append(
                            ('enum', scan_result.enum_values))
                    else:
                        normalized = _normalize_type(type_expr)
                        collected_union_members.append(('type', normalized))
                continue
            else:
                break

        if m_field:
            indent = _indent_level(m_field.group(1))
            if indent >= child_indent:
                # Sub-fields — these form an inline object.
                # Try to extract the type name from the field's type expression.
                # e.g., "array of MessageParam" -> "MessageParam"
                #        "Metadata" -> "Metadata"
                #        "object" -> fallback to "ObjectName_fieldName"
                ft = field_entry.get("type", "")
                sub_obj_name = _extract_type_name(ft)
                if not sub_obj_name and ft == "union":
                    # For union-typed fields, try extracting from the values
                    for v in field_entry.get("values", []):
                        extracted = _extract_type_name(v)
                        if extracted:
                            sub_obj_name = extracted
                            break
                if not sub_obj_name:
                    sub_obj_name = f"{object_name}_{field_name}"
                sub_fields = _parse_fields(ctx, child_indent, sub_obj_name)
                if sub_fields:
                    ctx.objects[sub_obj_name] = {"fields": sub_fields}
                break
            else:
                break

        # Unrecognized line — check indent
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_indent * 2:
            ctx.advance()
            continue
        else:
            break

    # Rebuild field type from children when available.
    # Don't override kind fields — a single-value child list just confirms
    # the discriminator value already set by inline detection.
    if collected_enum_values and field_entry.get("type") != "kind":
        # Children provide the full list of string enum values
        field_entry["type"] = "string"
        field_entry["values"] = collected_enum_values
        # Also register as a named enum
        enum_name = _infer_enum_name(field_name, object_name)
        ctx.enums[enum_name] = {
            "values": collected_enum_values,
            "source_field": f"{object_name}.{field_name}",
        }
    elif collected_union_members:
        # UnionMember children define the full union type.
        # Each member is either ('type', normalized_str) or ('enum', [values]).
        #
        # Collect all enum values and all type parts, then decide representation:
        # - If only enums (no other types, or only "string") → string enum
        # - Otherwise → union with enum members collapsed to "string"
        all_enum_values: list[str] = []
        type_parts: list[str] = []
        for kind, data in collected_union_members:
            if kind == 'enum':
                all_enum_values.extend(data)
            else:
                type_parts.append(data)

        # Check if enum values + "string" can collapse to a string enum
        non_string_parts = [p for p in type_parts if p != "string"]
        if all_enum_values and not non_string_parts:
            # All members are either enums or "string" → string enum
            field_entry["type"] = "string"
            field_entry["values"] = all_enum_values
        elif not all_enum_values:
            # Pure type union, no enums
            unique_parts = list(dict.fromkeys(type_parts))  # dedup, preserve order
            field_entry["type"] = "union"
            field_entry["values"] = unique_parts
        else:
            # Mixed: enums + non-string types → union with enums as "string"
            parts = list(dict.fromkeys(type_parts + ["string"]))
            field_entry["type"] = "union"
            field_entry["values"] = parts


def _extract_type_name(type_str: str) -> str | None:
    """
    Extract a named type from a type expression to use as the sub-object name.

    "array<MessageParam>"     -> "MessageParam"
    "array<ToolUnion>"        -> "ToolUnion"
    "Metadata"                -> "Metadata"
    "string | Foo"            -> None (ambiguous union)
    "object"                  -> None (anonymous)
    "string"                  -> None (primitive)
    """
    if not type_str:
        return None

    # array<TypeName> -> TypeName
    m = re.match(r'^array<(\w+)>$', type_str)
    if m:
        name = m.group(1)
        # Skip primitives
        if name.lower() in ('string', 'number', 'boolean', 'unknown', 'object'):
            return None
        return name

    # Single named type (capitalized, no angle brackets, no pipes, no quotes)
    if (re.match(r'^[A-Z]\w+$', type_str)
            and '|' not in type_str
            and '<' not in type_str
            and '"' not in type_str):
        return type_str

    return None


def _infer_enum_name(field_name: str, object_name: str) -> str:
    """Generate a qualified enum name: ObjectName::CamelCaseFieldName.

    Uses '::' as the ownership separator during parsing/extraction to avoid
    ambiguity with underscores in field names.  Downstream tools (checklist,
    C++ code generation) convert '::' to '_' per the C++ naming convention.

    Examples:
        Request.service_tier  → Request::ServiceTier
        Base64ImageSource.media_type → Base64ImageSource::MediaType
        WebSearchTool20250305.name → WebSearchTool20250305::Name
    """
    parts = field_name.split('_')
    camel = ''.join(p.capitalize() for p in parts)
    return f"{object_name}::{camel}"


def _skip_children(ctx: _ParseContext, parent_indent: int) -> None:
    """Skip all lines that are children (more indented) of the given level."""
    child_char_indent = (parent_indent + 1) * 2
    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_char_indent:
            ctx.advance()
        else:
            break


class _ScanResult:
    """Results from scanning children of a non-object named type."""
    __slots__ = ('enum_values', 'union_member_types')

    def __init__(self) -> None:
        self.enum_values: list[str] = []
        self.union_member_types: list[str] = []  # normalized types from UnionMember children


def _scan_children_for_objects(ctx: _ParseContext, parent_indent: int) -> _ScanResult:
    """
    Scan through children of a non-object named type (e.g., UnionMember1 = array of X)
    looking for named object definitions to extract, while also collecting any
    enum literal values and UnionMember type expressions found at this level.

    Returns a _ScanResult with enum values and union member types.
    """
    child_char_indent = (parent_indent + 1) * 2
    result = _ScanResult()

    while not ctx.at_end():
        line = ctx.peek()

        # Blank line — skip
        if not line.strip():
            ctx.advance()
            continue

        # Check indentation — stop if outdented
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent < child_char_indent:
            break

        # Check for named type definition (the objects we want to extract)
        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            ctx.advance()
            indent = _indent_level(m_named.group(1))
            type_name = m_named.group(2)
            type_expr = m_named.group(3)
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                # Found an object — extract its fields
                obj_fields = _parse_fields(ctx, indent + 1, type_name)
                obj_entry: dict[str, Any] = {"fields": obj_fields}
                if desc:
                    obj_entry["description"] = desc
                ctx.objects[type_name] = obj_entry
            else:
                # Non-object alias (another UnionMember, etc.) — recurse
                child_result = _scan_children_for_objects(ctx, indent)
                # If this is a UnionMember, rebuild its type from children if possible
                if type_name.startswith('UnionMember'):
                    if child_result.union_member_types:
                        # Nested union members define the inner type
                        inner = ' | '.join(child_result.union_member_types)
                        # Check if this was an array wrapper
                        normalized = _normalize_type(type_expr)
                        if normalized.startswith('array<'):
                            result.union_member_types.append(f"array<{inner}>")
                        else:
                            result.union_member_types.append(inner)
                    elif child_result.enum_values:
                        # Enum values define a string type
                        result.union_member_types.append('string')
                    else:
                        normalized = _normalize_type(type_expr)
                        result.union_member_types.append(normalized)
            continue

        # Check for enum literal values
        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            val_indent = _indent_level(m_enum.group(1))
            if val_indent >= parent_indent + 1:
                result.enum_values.append(m_enum.group(2))
                ctx.advance()
                continue
            else:
                break

        # Everything else (fields, description text) — skip
        ctx.advance()

    return result


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def extract(markdown_text: str, provider: str, endpoint: str,
            source_url: str, raw_file: str) -> dict[str, Any]:
    """
    Parse provider markdown documentation and return the intermediate JSON structure.

    Args:
        markdown_text: Full markdown text of the API documentation
        provider: Provider name (e.g., "openai")
        endpoint: Endpoint name (e.g., "responses")
        source_url: URL the markdown was fetched from
        raw_file: Relative path to the raw markdown file

    Returns:
        Intermediate JSON dict with meta, request, and response sections.
    """
    from datetime import datetime, timezone

    lines = markdown_text.split('\n')

    # Find section boundaries
    request_start = None
    response_start = None
    for i, line in enumerate(lines):
        m = _SECTION_RE.match(line)
        if m:
            section_name = m.group(1).strip()
            if section_name == 'Body Parameters':
                request_start = i + 1
            elif section_name == 'Returns':
                response_start = i + 1

    result: dict[str, Any] = {
        "meta": {
            "provider": provider,
            "endpoint": endpoint,
            "source_url": source_url,
            "fetched_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "raw_file": raw_file,
        },
        "request": {"objects": {}, "enums": {}},
        "response": {"objects": {}, "enums": {}},
    }

    # Parse request section
    if request_start is not None:
        end = response_start if response_start is not None else len(lines)
        section_lines = lines[request_start:end]
        ctx = _ParseContext(section_lines)
        request_fields = _parse_fields(ctx, 0, "Request")

        # The top-level fields form the Request object
        req_objects = dict(ctx.objects)
        req_objects["Request"] = {"fields": request_fields}
        result["request"]["objects"] = req_objects
        result["request"]["enums"] = dict(ctx.enums)

    # Parse response section
    if response_start is not None:
        section_lines = lines[response_start:]
        ctx = _ParseContext(section_lines)
        response_fields = _parse_fields(ctx, 0, "Response")

        resp_objects = dict(ctx.objects)
        # For the response, the top-level is often a named type like "Response = object { ... }"
        # which will be captured by _parse_fields via _NAMED_TYPE_RE.
        # If there are also bare fields at level 0, add them as "Response"
        if response_fields:
            resp_objects["Response"] = {"fields": response_fields}
        result["response"]["objects"] = resp_objects
        result["response"]["enums"] = dict(ctx.enums)

    return result
