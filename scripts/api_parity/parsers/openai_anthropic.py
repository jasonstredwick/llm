r"""
Parser for the OpenAI / Anthropic markdown documentation format.

Both providers use identical markdown structure:
  - Sections: ### Body Parameters (request), ### Returns (response)
  - Fields: `- \`name: optional type\`` with 2-space indentation per nesting level
  - Named types: `- \`TypeName = object { field1, field2 }\``
  - Named aliases: `- \`TypeName = string\`` or `- \`TypeName = array of Foo\``
  - Enum values: `- \`"literal_value"\``
  - Union types: `type1 or type2`
  - Kind fields: `type: "literal"` (fixed discriminator)

Produces a nested intermediate JSON tree as described in docs/codegen_design.md.
"""

import re
import sys
from typing import Any


def _pascal(name: str) -> str:
    """snake_case → PascalCase."""
    return "".join(w.capitalize() for w in name.split("_"))


# ---------------------------------------------------------------------------
# Line classification
# ---------------------------------------------------------------------------

# Match a field definition: `name: optional type_expression`
_FIELD_RE = re.compile(
    r'^(\s*)- `(\w+):\s+(.*?)`\s*$'
)

# Match a named type definition: `TypeName = type_expression`
_NAMED_TYPE_RE = re.compile(
    r'^(\s*)- `(\w+)\s*=\s*(.*?)`\s*$'
)

# Match an enum literal value: `"value"`
_ENUM_VALUE_RE = re.compile(
    r'^(\s*)- `"([^"]+)"`\s*$'
)

# Match a bare type name: `string`, `number`, `unknown`,
# `array of string or number`, etc.  These appear as union member
# children in the spec without the ``UnionMemberN = ...`` wrapper
# that older spec versions used.
_BARE_TYPE_RE = re.compile(
    r'^(\s*)- `([^:`"=]+)`\s*$'
)

# Match a compound enum value expression:
#   - `"gpt-5.4" or "gpt-5.4-mini" or "gpt-5.4-nano" or 75 more`
# These appear in the new spec format where a quoted-value union
# summary replaces the old ``UnionMember1 = "value" or ...`` pattern.
# The actual enum values are listed as children underneath.
_COMPOUND_ENUM_RE = re.compile(
    r'^(\s*)- `"[^"]*"(?:\s+or\s+.+)*`\s*$'
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
    """Join description lines into a single string."""
    text = ' '.join(line.strip() for line in lines if line.strip())
    text = _strip_markdown_links(text)
    text = re.sub(r'\s+', ' ', text).strip()
    return text


def _parse_type_expr(raw: str) -> tuple[str, bool]:
    """
    Parse a type expression, returning (normalized_type, is_optional).

    Examples:
        "optional boolean"        -> ("boolean", True)
        "string or array of Foo"  -> ("string | array<Foo>", False)
    """
    s = raw.strip()
    optional = False
    if s.startswith('optional '):
        optional = True
        s = s[len('optional '):]
    s = _normalize_type(s)
    return s, optional


def _normalize_type(s: str) -> str:
    """
    Normalize a type expression to our intermediate notation.

    "array of Foo"       -> "array<Foo>"
    "map[unknown]"       -> "map<string, object>"
    "object { a, b }"    -> "object"

    The inline type expression is only used for structural hints (array,
    map, object wrappers).  ``or`` alternatives are **never** split here;
    the child definitions underneath each field are the authoritative
    source for union/enum members.
    """
    # Strip trailing "or N more" (truncated inline summaries).
    s = re.sub(r'\s+or\s+\d+\s+more\s*$', '', s)

    m = re.match(r'^map\[(.+)\]$', s)
    if m:
        inner = _normalize_type(m.group(1).strip())
        return f"map<string, {inner}>"

    m = re.match(r'^array\s+of\s+(.+)$', s)
    if m:
        inner_raw = m.group(1).strip()
        # If the inner expression contains 'or', it's a complex type
        # whose structure is defined by children — keep only the first
        # token as a structural hint (or discard if it's a literal).
        if ' or ' in inner_raw:
            first = inner_raw.split(' or ')[0].strip()
            if first.startswith('"'):
                # ``array of "a" or "b"`` → children define element enum
                return "array<string>"
            inner = _normalize_type(first)
        else:
            inner = _normalize_type(inner_raw)
        return f"array<{inner}>"

    if s.startswith('object'):
        return "object"

    # "unknown" is an opaque JSON blob — same as "object" in intermediate.
    if s == 'unknown':
        return "object"

    return s


def _has_top_level_union(type_str: str) -> bool:
    """Return True if *type_str* contains a ``|`` outside of angle brackets."""
    depth = 0
    for ch in type_str:
        if ch == '<':
            depth += 1
        elif ch == '>':
            depth -= 1
        elif ch == '|' and depth == 0:
            return True
    return False


def _split_top_level_union(type_str: str) -> list[str]:
    """Split *type_str* on ``|`` only at the top level (outside ``<>``)."""
    parts: list[str] = []
    depth = 0
    current: list[str] = []
    i = 0
    while i < len(type_str):
        ch = type_str[i]
        if ch == '<':
            depth += 1
            current.append(ch)
        elif ch == '>':
            depth -= 1
            current.append(ch)
        elif ch == '|' and depth == 0:
            parts.append(''.join(current).strip())
            current = []
        else:
            current.append(ch)
        i += 1
    parts.append(''.join(current).strip())
    return parts


# ---------------------------------------------------------------------------
# Struct node builder
# ---------------------------------------------------------------------------

def _make_struct_node(name: str) -> dict[str, Any]:
    """Create a new struct node for the output tree."""
    return {
        "name": name,
        "kind": None,         # Set if the struct has a Kind type field
        "fields": [],         # Array of field descriptors
        "children": [],       # Nested struct definitions
    }


def _make_field(name: str, field_type: str, required: bool) -> dict[str, Any]:
    """Create a new field descriptor."""
    return {
        "name": name,
        "type": field_type,
        "required": required,
    }


def _find_child(node: dict, name: str) -> dict | None:
    """Find a child struct by name in a node's children list."""
    for child in node["children"]:
        if child["name"] == name:
            return child
    return None


def _attach_child_if_new(parent_node: dict, child_node: dict,
                         ancestry: list[str], line_num: int) -> bool:
    """Attach *child_node* to *parent_node* unless a same-named child exists.

    The caller has already fully parsed the child (consuming all its input
    lines).  This function simply decides whether to keep or discard the
    result.  Returns True if the child was attached.
    """
    name = child_node["name"]
    existing = _find_child(parent_node, name)
    if existing is not None:
        path = "::".join(ancestry + [name])
        print(f"  [skip] duplicate '{name}' at scope '{path}' "
              f"(source line {line_num})", file=sys.stderr)
        return False
    parent_node["children"].append(child_node)
    return True


# ---------------------------------------------------------------------------
# Type registry (shared / dependency types)
# ---------------------------------------------------------------------------

def _resolve_from_registry(type_str: str, registry: dict[str, str]) -> str:
    """Substitute *type_str* with its registry definition, if present.

    Handles bare names (``Metadata``), ``array<Name>``, and
    ``map<string, Name>`` wrappers.
    """
    if not registry:
        return type_str

    # Bare name
    if type_str in registry:
        return registry[type_str]

    # array<Name>
    m = re.match(r'^array<(.+)>$', type_str)
    if m:
        inner = m.group(1).strip()
        if inner in registry:
            return f"array<{registry[inner]}>"

    # map<string, Name>
    m = re.match(r'^map<string,\s*(.+)>$', type_str)
    if m:
        inner = m.group(1).strip()
        if inner in registry:
            return f"map<string, {registry[inner]}>"

    return type_str


def _simplify_type_def(type_str: str) -> str:
    """Simplify a top-level type definition from the shared types spec.

    For ``map[T]``, resolve to ``map<string, T>``.
    Unions with literals are preserved (never collapsed with ``string``)
    so codegen can emit ``std::variant<EnumValues, std::string>``.
    Object types are skipped (returned as-is) — they're full struct defs
    that the main spec already parses inline.
    """
    s = type_str.strip()

    # map[T] → map<string, T>
    m = re.match(r'^map\[(.+)\]$', s)
    if m:
        inner = _normalize_type(m.group(1).strip())
        return f"map<string, {inner}>"

    # Normalize through the standard pipeline first
    normalized = _normalize_type(s)

    # If the result still contains 'or' alternatives after normalisation,
    # strip any trailing "N more" placeholders.
    # String literals are enum values — a DISTINCT type from "string".
    # - All literals (no bare ``string``): pure enum → keep as-is (skip
    #   from registry; the main spec children define the enum).
    # - Literals + bare ``string``: variant<enum, string> → keep as-is
    #   so codegen can emit both arms.  NEVER collapse to just "string".
    # - All bare scalars, no literals: keep as-is.
    if ' or ' in normalized:
        parts = [p.strip() for p in normalized.split(' or ')]
        parts = [p for p in parts if not re.match(r'^\d+\s+more$', p)]

        has_literals = any(
            p.startswith('"') and p.endswith('"') for p in parts
        )
        has_bare_string = 'string' in parts

        if has_literals and has_bare_string:
            # Mixed literals + bare string → keep BOTH so codegen can
            # emit  std::variant<EnumValues, std::string>.
            # Literals are NEVER collapsed with string.
            # Use ' or ' here because the caller (parse_type_registry)
            # checks for quotes to skip these entries.
            return ' or '.join(parts)

        if has_literals and not has_bare_string:
            # Pure enum — keep the 'or' expression so parse_type_registry
            # can skip it (contains quotes).
            return ' or '.join(parts)

        # No literals — rejoin with pipe separator (the canonical
        # internal separator for type alternatives).
        normalized = ' | '.join(parts)

    return normalized


def parse_type_registry(markdown_text: str) -> dict[str, str]:
    """Parse a shared types markdown file into a type name → resolved type map.

    The shared types file uses the same markdown conventions as the main spec
    but defines types with ``Name = type_expression`` instead of fields.
    Only simple type aliases (scalars, maps, string-collapsible unions) are
    included; complex object types are omitted — they're parsed inline
    from the main spec.
    """
    registry: dict[str, str] = {}

    for line in markdown_text.split('\n'):
        m = _NAMED_TYPE_RE.match(line)
        if not m:
            continue
        indent = _indent_level(m.group(1))
        # Only top-level entries (indent 0)
        if indent != 0:
            continue
        name = m.group(2)
        type_expr = m.group(3).strip()

        # Skip complex object types — the main spec parses these inline.
        if type_expr.startswith('object {'):
            continue
        # Skip compound union types with objects
        if ' or object' in type_expr:
            continue

        resolved = _simplify_type_def(type_expr)

        # Skip types that contain string literals — they represent enums
        # (or variant<Enum, string>) and need richer representation than
        # a simple string alias in the registry.
        if '"' in resolved:
            continue

        # Only store if it resolved to something useful (not still a name)
        if resolved:
            registry[name] = resolved

    return registry


# ---------------------------------------------------------------------------
# Parse context
# ---------------------------------------------------------------------------

class _ParseContext:
    """Holds state during parsing."""

    def __init__(self, lines: list[str], line_offset: int = 0,
                 type_registry: dict[str, str] | None = None):
        self.lines = lines
        self.pos = 0
        self.line_offset = line_offset  # for error reporting
        self.type_registry = type_registry or {}  # name → resolved type

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

    def source_line(self) -> int:
        """Current line number in the original document."""
        return self.pos + self.line_offset


# ---------------------------------------------------------------------------
# Description collector
# ---------------------------------------------------------------------------

def _collect_description(ctx: _ParseContext, base_indent: int) -> str:
    """Collect description lines that follow a field definition."""
    desc_lines: list[str] = []
    while not ctx.at_end():
        line = ctx.peek()

        if not line.strip():
            ctx.advance()
            if not ctx.at_end():
                next_line = ctx.peek()
                if (next_line.strip()
                        and not _FIELD_RE.match(next_line)
                        and not _NAMED_TYPE_RE.match(next_line)
                        and not _ENUM_VALUE_RE.match(next_line)
                        and not _BARE_TYPE_RE.match(next_line)
                        and not _COMPOUND_ENUM_RE.match(next_line)
                        and not _SECTION_RE.match(next_line)):
                    stripped = next_line.lstrip()
                    line_indent = len(next_line) - len(stripped)
                    field_char_indent = base_indent * 2 + 2
                    if line_indent >= field_char_indent:
                        desc_lines.append('')
                        continue
                ctx.pos -= 1
                break
            else:
                ctx.pos -= 1
                break

        if (_FIELD_RE.match(line)
                or _NAMED_TYPE_RE.match(line)
                or _ENUM_VALUE_RE.match(line)
                or _BARE_TYPE_RE.match(line)
                or _COMPOUND_ENUM_RE.match(line)
                or _SECTION_RE.match(line)):
            break

        stripped = line.lstrip()
        line_indent = len(line) - len(stripped)
        field_char_indent = base_indent * 2 + 2
        if line_indent >= field_char_indent:
            desc_lines.append(stripped)
            ctx.advance()
        else:
            break

    return _clean_description(desc_lines)


# ---------------------------------------------------------------------------
# Core parsing
# ---------------------------------------------------------------------------

def _collect_enum_values(ctx: _ParseContext, child_indent: int) -> list[str]:
    """Collect enum literal values at a given indentation level.

    Enum values may be followed by description lines that are indented
    deeper than the enum value itself.  These descriptions are consumed
    and discarded so that the next enum value can be found.
    """
    values: list[str] = []
    # Character indent of the enum bullet (``child_indent`` is the
    # logical indent level; each level is 2 characters of leading space
    # plus the ``- `` bullet prefix).
    enum_char_indent = child_indent * 2
    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue
        m = _ENUM_VALUE_RE.match(line)
        if m and _indent_level(m.group(1)) >= child_indent:
            values.append(m.group(2))
            ctx.advance()
            # Consume any trailing description lines for this value.
            # They are indented deeper than the enum bullet.
            while not ctx.at_end():
                desc_line = ctx.peek()
                if not desc_line.strip():
                    ctx.advance()
                    continue
                stripped = desc_line.lstrip()
                line_char_indent = len(desc_line) - len(stripped)
                if line_char_indent > enum_char_indent:
                    # Description line — skip it.
                    ctx.advance()
                    continue
                break
            continue
        break
    return values


def _parse_fields(ctx: _ParseContext, parent_indent: int,
                  parent_node: dict, ancestry: list[str]) -> None:
    """
    Parse fields at a given indentation level, populating parent_node.

    Fields are appended to parent_node["fields"].
    Child type definitions are appended to parent_node["children"].
    """
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
                break
            if indent > parent_indent:
                ctx.advance()
                continue
            ctx.advance()
            field_name = m_field.group(2)
            type_raw = m_field.group(3)
            type_str, optional = _parse_type_expr(type_raw)

            # Check for 'or' in the RAW expression BEFORE normalization.
            # _normalize_type strips 'or' alternatives from array types
            # (e.g. "array of X or Y" → "array<X>"), so we must check
            # the original text to know whether children define a union.
            raw_stripped = type_raw.strip()
            if raw_stripped.startswith('optional '):
                raw_stripped = raw_stripped[len('optional '):]
            has_or = ' or ' in raw_stripped

            desc = _collect_description(ctx, indent)

            # Request fields: default to required (spec marks optional ones
            # explicitly with "optional" keyword).
            # Response fields: default to optional (spec doesn't annotate
            # requiredness consistently).
            # Kind fields are always required (promoted below after
            # classification; uses Required<T> with default init).
            is_response = ancestry[0] == "Response" if ancestry else False
            required = False if is_response else not optional
            field = _make_field(field_name, type_str, required)
            if desc:
                field["description"] = desc

            # Classify the field type.
            #
            # The inline type expression is used ONLY for structural
            # hints: whether the field is a kind, whether it wraps an
            # array/map, etc.  As soon as the expression contains
            # ``or`` alternatives or a named non-fundamental type, the
            # **children** underneath the field are the sole authority.
            #
            # Priority:
            #   1. Children (always authoritative when present)
            #   2. Registry fallback (only when no children define the type)
            #   3. Inline type expression (structural hints only)
            # Check if 'or' is at top level vs inside brackets.
            # ``map[string or number]`` has 'or' inside brackets — not
            # a top-level union.  Normalize it as a map type directly.
            normalized = _normalize_type(type_str)
            is_map_with_inner_or = (
                has_or
                and re.match(r'^map<string,\s*', type_str)
            )

            if is_map_with_inner_or:
                # map[T or U or V] → structured map_value_members.
                # Extract the raw inner, split on 'or', and classify
                # each alternative into a structured member descriptor.
                m_raw = re.match(r'^map\[(.+)\]$', raw_stripped)
                raw_inner = m_raw.group(1) if m_raw else ""
                parts = [p.strip() for p in raw_inner.split(' or ')]
                mvms = [_classify_union_member(_normalize_type(p))
                        for p in parts]
                # Store the first alternative as a structural hint in
                # the type string; the authoritative value type info
                # lives in map_value_members.
                first = _normalize_type(parts[0]) if parts else "string"
                field["type"] = f"map<string, {first}>"
                field["map_value_members"] = mvms
                # Children may define struct types for map values — scan
                # them so they get attached to the field's scope.
                field.setdefault("children", [])
                _scan_for_nested_objects(ctx, indent, field, ancestry)

            elif (type_str.startswith('"') and type_str.endswith('"')
                    and not has_or):
                # Kind field: single literal string value
                field["type"] = "kind"
                field["value"] = type_str.strip('"')
                # Set the kind on the parent struct
                parent_node["kind"] = type_str.strip('"')
                # Consume any child enum values that just confirm the kind
                _collect_enum_values(ctx, indent + 1)

            elif has_or:
                # The inline type contains 'or' alternatives.  Always
                # defer to child definitions for the actual structure.
                child_enums = _collect_enum_values(ctx, indent + 1)
                if child_enums:
                    if type_str.startswith('array<'):
                        # ``array of "a" or "b" or "c"`` with child enum
                        # values → array of enum elements.  Store the
                        # enum values so codegen can emit an enum class
                        # and use it as the array element type.
                        field["type"] = "array<string>"
                        field["element_enum_values"] = child_enums
                    else:
                        field["type"] = "enum"
                        field["values"] = child_enums
                else:
                    # Children are UnionMember / struct definitions.
                    field["type"] = "union"
                    field["members"] = []
                    _parse_union_children(ctx, indent, field,
                                          parent_node, ancestry)
                    if type_str.startswith('array<'):
                        # ``array of X or Y`` where X, Y are struct
                        # types → the union describes the *element*
                        # type.  Wrap via union_def so codegen emits
                        # a named variant alias and the field becomes
                        # ``std::vector<VariantAlias>``.
                        variant_name = _pascal(field_name)
                        field["union_def"] = {
                            "name": variant_name,
                            "members": field.pop("members"),
                        }
                        if "discriminator" in field:
                            field["union_def"]["discriminator"] = (
                                field.pop("discriminator"))
                        field["type"] = f"array<{variant_name}>"
            else:
                # Try children first — they are always authoritative.
                child_enums = _collect_enum_values(ctx, indent + 1)
                if child_enums and field["type"] not in ("kind",):
                    field["type"] = "enum"
                    field["values"] = child_enums
                else:
                    # Try parsing inline child definitions (sub-fields,
                    # named types, union members).
                    saved_pos = ctx.pos
                    _parse_field_children(ctx, indent, field, parent_node, ancestry)
                    children_found = ctx.pos > saved_pos

                    # If no children defined the type AND it's a named
                    # non-fundamental type, fall back to the registry.
                    if (not children_found
                            and not field.get("union_def")
                            and field["type"] == type_str):
                        resolved = _resolve_from_registry(
                            type_str, ctx.type_registry)
                        if resolved != type_str:
                            field["type"] = resolved

            parent_node["fields"].append(field)
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
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node, ancestry + [type_name])
                _attach_child_if_new(parent_node, child_node, ancestry, ctx.source_line())
            else:
                # Type alias or UnionMember — check for array element struct first
                created = _maybe_create_array_element_struct(
                    ctx, indent, type_expr, parent_node, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, parent_node, ancestry)
            continue

        # Try matching an enum value
        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent < parent_indent:
                break
            ctx.advance()
            continue

        # Unrecognized line — check indent
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= parent_indent * 2 + 2:
            ctx.advance()
            continue
        else:
            break


def _classify_union_member(type_str: str) -> dict[str, Any]:
    """Classify a single union member type string into a member descriptor."""
    # array<TypeName>
    m = re.match(r'^array<(\w+)>$', type_str)
    if m:
        inner = m.group(1)
        if inner.lower() in ('string', 'number', 'boolean', 'unknown'):
            return {"type": "array", "element_type": inner}
        return {"type": "array", "element_type": inner}

    # Quoted literal → just a string value (part of enum-like union)
    if type_str.startswith('"') and type_str.endswith('"'):
        return {"type": "literal", "value": type_str.strip('"')}

    # Primitive
    if type_str.lower() in ('string', 'number', 'boolean', 'integer'):
        return {"type": type_str.lower()}

    # Named struct reference
    if re.match(r'^[A-Z]\w*$', type_str):
        return {"type": "struct", "ref": type_str}

    # Fallback
    return {"type": type_str}


def _classify_union_member_multi(type_str: str) -> list[dict[str, Any]]:
    """Classify a type string that may contain ``or``-separated alternatives.

    Returns a list of member descriptors (one per alternative).
    """
    # If the string contains '" or "', split into individual parts.
    if '" or "' in type_str or "' or '" in type_str:
        parts = [p.strip() for p in type_str.split(' or ')]
        results: list[dict[str, Any]] = []
        for p in parts:
            results.append(_classify_union_member(p))
        return results
    return [_classify_union_member(type_str)]


def _parse_union_children(ctx: _ParseContext, parent_indent: int,
                          field: dict, parent_node: dict,
                          ancestry: list[str]) -> None:
    """
    Parse children of a union-typed field. These may be:
    - UnionMember definitions (type aliases expanding the union)
    - Named object definitions (inline struct definitions for union arms)
    - Enum values (refining string arms)
    """
    child_indent = parent_indent + 1
    rebuilt_members: list[dict] = []
    collected_enum_values: list[str] = []

    # Union member type definitions (inline structs) belong to the field
    # that defines the union, NOT to the parent struct.  This preserves
    # the spec's nesting structure and avoids name collisions when the
    # parent struct has an unrelated same-named child.
    field.setdefault("children", [])

    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue
        if _SECTION_RE.match(line):
            break

        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            indent = _indent_level(m_named.group(1))
            if indent < child_indent:
                break
            ctx.advance()
            type_name = m_named.group(2)
            type_expr = m_named.group(3)
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                # Inline object definition for a union arm
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node,
                              ancestry + [type_name])
                _attach_child_if_new(field, child_node, ancestry,
                                     ctx.source_line())
                rebuilt_members.append({"type": "struct", "ref": type_name})
            else:
                # Union member alias — may be named ``UnionMember0``,
                # ``StringOutput``, ``OutputContentList``, etc.  The
                # name is irrelevant; the type expression determines
                # how we classify the member.
                normalized = _normalize_type(type_expr)
                elem_name = _extract_array_element_name(type_expr)
                if elem_name:
                    # Array type — children define the element type, not the
                    # parent union.  Create the element struct (from inline
                    # fields) or consume object children that form the element
                    # variant.  Either way keep the result as array<X>.
                    children_before = {c["name"] for c in field.get("children", [])}
                    created = _maybe_create_array_element_struct(
                        ctx, indent, type_expr, field, ancestry)
                    if not created:
                        # Children are named object types forming X's variant.
                        # Consume them (they become children of field)
                        # but do NOT add them as flat union members.
                        _scan_for_nested_objects(ctx, indent, field,
                                                 ancestry)
                    # If multiple kind-bearing children were added, the element
                    # type is a variant of those children — not just the first
                    # name from the inline string.  Record the specific child
                    # refs so the codegen can build the correct variant.
                    new_children = [c for c in field.get("children", [])
                                    if c["name"] not in children_before
                                    and c.get("kind") is not None]
                    if len(new_children) >= 2:
                        variant_elem = _pascal(field["name"]) + "Item"
                        rebuilt_members.append({
                            "type": "array",
                            "element_type": variant_elem,
                            "element_variant_refs": [c["name"] for c in new_children],
                        })
                    else:
                        member = _classify_union_member(normalized)
                        rebuilt_members.append(member)
                elif normalized.startswith('array<'):
                    # Array whose element type is described by children.
                    # Consume children (they become children of the field)
                    # but produce a **single** array member — never flatten
                    # into the parent union.
                    #
                    # First try to create element structs from children.
                    created = _maybe_create_array_element_struct(
                        ctx, indent, type_expr, field, ancestry)
                    if not created:
                        _scan_for_nested_objects(ctx, indent, field,
                                                 ancestry)
                    # Check whether the raw expression has ``or``
                    # alternatives for the element type, e.g.
                    # ``array of string or number``.  If so, record
                    # structured element_members so codegen can emit
                    # a variant for the element.
                    m_arr_raw = re.match(
                        r'^array\s+of\s+(.+)$', type_expr)
                    if m_arr_raw and ' or ' in m_arr_raw.group(1):
                        parts = [_normalize_type(p.strip())
                                 for p in m_arr_raw.group(1).split(' or ')]
                        elem_members = [_classify_union_member(p)
                                        for p in parts]
                        rebuilt_members.append(
                            {"type": "array",
                             "element_members": elem_members})
                    else:
                        m_arr = re.match(r'^array<(.+)>$', normalized)
                        elem = m_arr.group(1) if m_arr else "UNKNOWN"
                        rebuilt_members.append(
                            {"type": "array", "element_type": elem})
                else:
                    # Non-array union member — children may define it further
                    child_result = _scan_union_member_children(
                        ctx, indent, field, ancestry)
                    if child_result.get("members"):
                        rebuilt_members.extend(child_result["members"])
                    elif child_result.get("enum_values"):
                        collected_enum_values.extend(child_result["enum_values"])
                    else:
                        rebuilt_members.extend(
                            _classify_union_member_multi(normalized))
            continue

        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent >= child_indent:
                collected_enum_values.append(m_enum.group(2))
                ctx.advance()
                continue
            break

        m_field = _FIELD_RE.match(line)
        if m_field:
            indent = _indent_level(m_field.group(1))
            if indent >= child_indent:
                # Sub-fields indicate inline object — extract the type name
                ft = field.get("type", "")
                sub_name = _extract_type_name_from_field(field)
                if not sub_name:
                    sub_name = f"{ancestry[-1]}_{field['name']}" if ancestry else field['name']
                child_node = _make_struct_node(sub_name)
                _parse_fields(ctx, child_indent, child_node,
                              ancestry + [sub_name])
                _attach_child_if_new(field, child_node, ancestry,
                                     ctx.source_line())
                _update_field_type_ref(field, sub_name)
                break
            break

        # Bare type name: `string`, `unknown`, `array of string or number`
        m_bare = _BARE_TYPE_RE.match(line)
        if m_bare:
            indent = _indent_level(m_bare.group(1))
            if indent >= child_indent:
                raw_type = m_bare.group(2).strip()
                normalized = _normalize_type(raw_type)
                if normalized.startswith('array<'):
                    # Array with possible ``or`` alternatives in element type
                    m_arr_raw = re.match(
                        r'^array\s+of\s+(.+)$', raw_type)
                    if m_arr_raw and ' or ' in m_arr_raw.group(1):
                        parts = [_normalize_type(p.strip())
                                 for p in m_arr_raw.group(1).split(' or ')]
                        elem_members = [_classify_union_member(p)
                                        for p in parts]
                        rebuilt_members.append(
                            {"type": "array",
                             "element_members": elem_members})
                    else:
                        m_arr = re.match(r'^array<(.+)>$', normalized)
                        elem = m_arr.group(1) if m_arr else "UNKNOWN"
                        rebuilt_members.append(
                            {"type": "array", "element_type": elem})
                else:
                    rebuilt_members.extend(
                        _classify_union_member_multi(normalized))
                ctx.advance()
                # Parse children of this bare type — they may contain
                # inline struct definitions (e.g. array element types)
                # that need to be attached to the field.
                created = _maybe_create_array_element_struct(
                    ctx, indent, raw_type, field, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, field, ancestry)
                continue
            break

        # Compound enum expression: `"value1" or "value2" or ... or N more`
        m_compound = _COMPOUND_ENUM_RE.match(line)
        if m_compound:
            indent = _indent_level(m_compound.group(1))
            if indent >= child_indent:
                ctx.advance()
                # The actual enum values are listed as children
                child_enums = _collect_enum_values(ctx, indent + 1)
                collected_enum_values.extend(child_enums)
                continue
            break

        # Unrecognized — check indent
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_indent * 2:
            ctx.advance()
            continue
        break

    # Rebuild field members, extracting string literals into an enum.
    #
    # Any ``literal`` members are string enum values — a DISTINCT type
    # from bare ``string``.  Collect them into a single enum member:
    #   - Only literal members → field becomes an enum (not a union).
    #   - Literal + string members → variant<Enum, string>.
    #   - Literal + struct/array + … → variant<Enum, …other types…>.
    if rebuilt_members:
        literal_members = [m for m in rebuilt_members
                           if m.get("type") == "literal"]
        other_members = [m for m in rebuilt_members
                         if m.get("type") != "literal"]
        # Also merge any enum values collected from direct enum children
        # into the literal pool.
        all_enum_values = list(collected_enum_values)
        for lm in literal_members:
            v = lm.get("value")
            if v and v not in all_enum_values:
                all_enum_values.append(v)

        if all_enum_values and not other_members:
            # Pure enum — no other types in the union.
            field["type"] = "enum"
            field["values"] = all_enum_values
        elif all_enum_values:
            # Mixed: enum + other types → keep as union with an enum
            # member replacing all the literal members.
            enum_member = {"type": "enum", "values": all_enum_values}
            field["members"] = [enum_member] + other_members
        else:
            field["members"] = rebuilt_members
    elif collected_enum_values:
        # Only direct enum values, no UnionMember definitions at all.
        field["type"] = "enum"
        field["values"] = collected_enum_values


def _scan_union_member_children(ctx: _ParseContext, parent_indent: int,
                                attach_target: dict,
                                ancestry: list[str]) -> dict:
    """
    Scan children of a UnionMember definition.
    Returns dict with "members" and/or "enum_values".

    *attach_target* is the node whose ``"children"`` list receives any
    inline struct definitions discovered here (typically the union field).
    """
    child_indent = parent_indent + 1
    result: dict[str, Any] = {"members": [], "enum_values": []}

    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue

        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent < child_indent * 2:
            break

        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            indent = _indent_level(m_named.group(1))
            if indent < child_indent:
                break
            ctx.advance()
            type_name = m_named.group(2)
            type_expr = m_named.group(3)
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node,
                              ancestry + [type_name])
                _attach_child_if_new(attach_target, child_node, ancestry,
                                     ctx.source_line())
                result["members"].append({"type": "struct", "ref": type_name})
            else:
                # Union member alias — name is irrelevant; the type
                # expression determines how we classify the member.
                normalized = _normalize_type(type_expr)
                child_result = _scan_union_member_children(
                    ctx, indent, attach_target, ancestry)
                if child_result["members"]:
                    result["members"].extend(child_result["members"])
                elif child_result["enum_values"]:
                    result["enum_values"].extend(child_result["enum_values"])
                else:
                    created = _maybe_create_array_element_struct(
                        ctx, indent, type_expr, attach_target, ancestry)
                    if not created:
                        _scan_for_nested_objects(ctx, indent, attach_target,
                                                 ancestry)
                    member = _classify_union_member(normalized)
                    result["members"].append(member)
            continue

        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent >= child_indent:
                result["enum_values"].append(m_enum.group(2))
                ctx.advance()
                continue
            break

        # Bare type name: `string`, `unknown`, `array of X or Y`
        m_bare = _BARE_TYPE_RE.match(line)
        if m_bare:
            indent = _indent_level(m_bare.group(1))
            if indent >= child_indent:
                raw_type = m_bare.group(2).strip()
                normalized = _normalize_type(raw_type)
                member = _classify_union_member(normalized)
                result["members"].append(member)
                ctx.advance()
                # Parse children of this bare type — may contain
                # inline struct definitions.
                created = _maybe_create_array_element_struct(
                    ctx, indent, raw_type, attach_target, ancestry)
                if not created:
                    _scan_for_nested_objects(
                        ctx, indent, attach_target, ancestry)
                continue
            break

        # Compound enum expression
        m_compound = _COMPOUND_ENUM_RE.match(line)
        if m_compound:
            indent = _indent_level(m_compound.group(1))
            if indent >= child_indent:
                ctx.advance()
                child_enums = _collect_enum_values(ctx, indent + 1)
                result["enum_values"].extend(child_enums)
                continue
            break

        ctx.advance()

    return result


def _parse_field_children(ctx: _ParseContext, parent_indent: int,
                          field: dict, parent_node: dict,
                          ancestry: list[str]) -> None:
    """
    Parse children of a non-union, non-enum field.
    Handles inline object definitions and named type definitions.

    When the field references a named type (e.g. ``TextCitationParam``) and the
    children are ``Name = object { ... }`` definitions, those children form a
    **union** for the referenced type.  In that case we attach an explicit
    ``union_def`` to the field so the codegen never has to guess.
    """
    child_indent = parent_indent + 1

    # All child type definitions belong to this field's scope — the spec
    # defines them as aggregates under the field, not shared with siblings.
    field.setdefault("children", [])

    # Determine if this field references a named type that could be a union.
    # Pattern: field type is "FooBar" or "array<FooBar>" where FooBar starts
    # with an uppercase letter and is NOT a primitive.
    union_type_name = _extract_type_name_from_field(field)
    # Track named-object children discovered under this field.
    # If we find ≥ 2 we know the referenced type is a union.
    union_members: list[dict] = []

    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue
        if _SECTION_RE.match(line):
            break

        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            indent = _indent_level(m_named.group(1))
            if indent < child_indent:
                break
            ctx.advance()
            type_name = m_named.group(2)
            type_expr = m_named.group(3)
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node,
                              ancestry + [type_name])
                _attach_child_if_new(field, child_node, ancestry,
                                     ctx.source_line())
                # Record as a potential union member
                if union_type_name:
                    union_members.append({"type": "struct", "ref": type_name})
            else:
                # The inline type expression may contain string literals
                # (e.g. ``UnionMember0 = "a" or "b" or "c" or 19 more``).
                # The *children* are authoritative — consume them first.
                normalized = _normalize_type(type_expr)
                child_enums = _collect_enum_values(ctx, indent + 1)

                if child_enums:
                    # Children are string literal values → enum member.
                    # Carry the values so the union_def can produce a
                    # proper enum type in the codegen.
                    if union_type_name:
                        union_members.append({
                            "type": "enum",
                            "values": child_enums,
                        })
                else:
                    # Check if this is an array type with inline field
                    # children that define the element struct.
                    created = _maybe_create_array_element_struct(
                        ctx, indent, type_expr, field, ancestry)
                    if not created:
                        _scan_for_nested_objects(
                            ctx, indent, field, ancestry)
                    # Non-object named types under a union-candidate field
                    if union_type_name:
                        member = _classify_union_member(normalized)
                        union_members.append(member)
            continue

        m_field = _FIELD_RE.match(line)
        if m_field:
            indent = _indent_level(m_field.group(1))
            if indent >= child_indent:
                # Sub-fields — inline object
                sub_name = _extract_type_name_from_field(field)
                if not sub_name:
                    sub_name = f"{ancestry[-1]}_{field['name']}" if ancestry else field['name']
                child_node = _make_struct_node(sub_name)
                _parse_fields(ctx, child_indent, child_node,
                              ancestry + [sub_name])
                _attach_child_if_new(field, child_node, ancestry,
                                     ctx.source_line())
                _update_field_type_ref(field, sub_name)
                break
            break

        m_enum = _ENUM_VALUE_RE.match(line)
        if m_enum:
            indent = _indent_level(m_enum.group(1))
            if indent >= child_indent:
                # Enum values under a regular field — collect them
                values = [m_enum.group(2)]
                ctx.advance()
                values.extend(_collect_enum_values(ctx, child_indent))
                field["type"] = "enum"
                field["values"] = values
                break
            break

        # Bare type name: `string`, `unknown`, etc.
        m_bare = _BARE_TYPE_RE.match(line)
        if m_bare:
            indent = _indent_level(m_bare.group(1))
            if indent >= child_indent:
                raw_type = m_bare.group(2).strip()
                normalized = _normalize_type(raw_type)
                if union_type_name:
                    member = _classify_union_member(normalized)
                    union_members.append(member)
                ctx.advance()
                # Parse children of this bare type — may contain
                # inline struct definitions.
                created = _maybe_create_array_element_struct(
                    ctx, indent, raw_type, field, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, field, ancestry)
                continue
            break

        # Compound enum expression: `"value1" or "value2" or ... or N more`
        m_compound = _COMPOUND_ENUM_RE.match(line)
        if m_compound:
            indent = _indent_level(m_compound.group(1))
            if indent >= child_indent:
                ctx.advance()
                child_enums = _collect_enum_values(ctx, indent + 1)
                if child_enums and union_type_name:
                    union_members.append({
                        "type": "enum",
                        "values": child_enums,
                    })
                continue
            break

        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_indent * 2:
            ctx.advance()
            continue
        break

    # Analyse the collected union members.
    if union_type_name and union_members:
        # If the only member is an enum (all children were literals),
        # promote the field directly to enum type.
        if (len(union_members) == 1
                and union_members[0].get("type") == "enum"):
            field["type"] = "enum"
            field["values"] = union_members[0]["values"]
        elif len(union_members) >= 2:
            # Multiple members → record as a union definition.
            # The variant is unnamed in the spec — the extracted
            # ``union_type_name`` may actually be a member name
            # (e.g. ``array<Foo>`` extracts ``Foo`` which is one of the
            # variant members).  Use the pascal-cased field name
            # instead, matching how explicit union fields are named.
            member_refs = {m.get("ref") for m in union_members
                          if m.get("type") == "struct"}
            if union_type_name in member_refs:
                variant_name = _pascal(field["name"])
            else:
                variant_name = union_type_name
            field["union_def"] = {
                "name": variant_name,
                "members": union_members,
            }
            # Finalize the field type to reference the variant name,
            # preserving any structural wrapper (array<>, map<>).
            ft = field["type"]
            m_arr = re.match(r'^array<.+>$', ft)
            if m_arr:
                field["type"] = f"array<{variant_name}>"
            elif ft == union_type_name:
                field["type"] = variant_name
            # (map<> wrappers would be handled similarly if needed)


def _extract_array_element_name(type_expr: str) -> str | None:
    """Extract element type name from 'array of Foo' or 'array<Foo>'.

    The expression may contain ``or`` alternatives after the first type
    name (e.g. ``array of Foo or Bar or Baz``).  Only the first token
    is considered — the children are authoritative for the full list.
    """
    _PRIMITIVES = {'string', 'number', 'boolean', 'unknown', 'object'}
    # ``array of Foo`` or ``array of Foo or Bar or ...``
    m = re.match(r'^array\s+of\s+(\w+)', type_expr)
    if m:
        name = m.group(1)
        if name[0].isupper() and name.lower() not in _PRIMITIVES:
            return name
    # ``array<Foo>``
    m = re.match(r'^array<(\w+)>$', type_expr)
    if m:
        name = m.group(1)
        if name[0].isupper() and name.lower() not in _PRIMITIVES:
            return name
    return None


def _maybe_create_array_element_struct(
    ctx: _ParseContext, indent: int, type_expr: str,
    parent_node: dict, ancestry: list[str],
) -> str | None:
    """If *type_expr* is ``array of ElementType`` and the next lines are
    field definitions, create a child struct for the element type and parse
    the fields into it.  Returns the element-type name on success, else None.
    """
    elem_name = _extract_array_element_name(type_expr)
    if not elem_name:
        return None

    # Peek: are the next non-blank lines field definitions?
    child_indent = indent + 1
    saved = ctx.pos  # save position in case we need to roll back
    while not ctx.at_end() and not ctx.peek().strip():
        ctx.advance()
    if ctx.at_end():
        ctx.pos = saved
        return None
    if not _FIELD_RE.match(ctx.peek()):
        ctx.pos = saved
        return None
    # Yes — create a struct and parse those fields into it
    child_node = _make_struct_node(elem_name)
    _parse_fields(ctx, child_indent, child_node, ancestry + [elem_name])
    _attach_child_if_new(parent_node, child_node, ancestry, ctx.source_line())
    return elem_name


def _scan_for_nested_objects(ctx: _ParseContext, parent_indent: int,
                             parent_node: dict, ancestry: list[str]) -> None:
    """Scan children for object definitions without consuming non-object lines."""
    child_char_indent = (parent_indent + 1) * 2

    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue

        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent < child_char_indent:
            break

        m_named = _NAMED_TYPE_RE.match(line)
        if m_named:
            ctx.advance()
            indent = _indent_level(m_named.group(1))
            type_name = m_named.group(2)
            type_expr = m_named.group(3)
            desc = _collect_description(ctx, indent)

            if type_expr.startswith('object'):
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node,
                              ancestry + [type_name])
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
            else:
                created = _maybe_create_array_element_struct(
                    ctx, indent, type_expr, parent_node, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, parent_node, ancestry)
            continue

        ctx.advance()


def _update_field_type_ref(field: dict, child_name: str) -> None:
    """Update a field's type to reference a synthesized child struct name.

    Handles ``"object"`` → ``"ChildName"`` and ``"array<object>"`` →
    ``"array<ChildName>"``.  If the type already contains a named reference
    (not ``object``) it is left unchanged.
    """
    ft = field.get("type", "")
    if ft == "object":
        field["type"] = child_name
    elif ft == "array<object>":
        field["type"] = f"array<{child_name}>"
    # Otherwise the field already has a named type — leave it alone.


def _extract_type_name_from_field(field: dict) -> str | None:
    """
    Extract a named type from a field's type to use as sub-object name.
    "array<MessageParam>" -> "MessageParam"
    "Metadata" -> "Metadata"
    """
    ft = field.get("type", "")

    m = re.match(r'^array<(\w+)>$', ft)
    if m:
        name = m.group(1)
        if name.lower() in ('string', 'number', 'boolean', 'unknown', 'object'):
            return None
        return name

    if (re.match(r'^[A-Z]\w+$', ft)
            and '|' not in ft
            and '<' not in ft
            and '"' not in ft):
        return ft

    return None


# ---------------------------------------------------------------------------
# Post-processing: enrich union fields with discriminator info
# ---------------------------------------------------------------------------

def _enrich_unions(node: dict) -> None:
    """
    Walk the tree and add discriminator information to union fields.

    For each union field (explicit ``"type": "union"`` or ``union_def``),
    look at the member structs (in children) and check if they have a Kind
    field.  If so, record the discriminator field name and per-member
    discriminator values.
    """
    children_by_name = {c["name"]: c for c in node.get("children", [])}

    for field in node.get("fields", []):
        # Build lookup that includes field-scoped children (union member
        # type definitions now live on the field, not the parent node).
        field_children_by_name = {c["name"]: c
                                  for c in field.get("children", [])}

        # Enrich explicit union fields — member structs are in field children
        if field.get("type") == "union" and "members" in field:
            # Merge: field-level children take priority (they are the union's
            # own member types), fall back to node-level children.
            merged = {**children_by_name, **field_children_by_name}
            _enrich_member_list(field["members"], field, merged)

        # Enrich union_def (synthesised from _parse_field_children) —
        # member structs are still at node level for these.
        udef = field.get("union_def")
        if udef and "members" in udef:
            merged = {**children_by_name, **field_children_by_name}
            _enrich_member_list(udef["members"], udef, merged)

        # Recurse into field-level children
        for child in field.get("children", []):
            _enrich_unions(child)

    # Recurse into node-level children
    for child in node.get("children", []):
        _enrich_unions(child)


def _enrich_member_list(members: list[dict], target: dict,
                        children_by_name: dict) -> None:
    """Add discriminator values to *members* list, set ``target["discriminator"]``.

    Discriminator selection policy (no heuristics):
      1. One kind field   → use it.
      2. Multiple kind fields, one is ``type`` → use ``type``.
      3. Multiple kind fields, none is ``type`` → halt with error.
    """
    discriminator_field = None
    for member in members:
        ref = member.get("ref")
        if ref and ref in children_by_name:
            child = children_by_name[ref]
            if child.get("kind") is not None:
                kind_fields = [cf for cf in child.get("fields", [])
                               if cf.get("type") == "kind"]
                if not kind_fields:
                    continue

                if len(kind_fields) == 1:
                    best = kind_fields[0]
                else:
                    # Multiple kind fields — require "type" to be among them.
                    type_fields = [kf for kf in kind_fields
                                   if kf["name"] == "type"]
                    if type_fields:
                        best = type_fields[0]
                    else:
                        names = [kf["name"] for kf in kind_fields]
                        raise ValueError(
                            f"Multiple kind fields {names} on '{ref}' but "
                            f"none is 'type'. Cannot auto-select discriminator."
                        )

                discriminator_field = best["name"]
                member["discriminator_value"] = best["value"]
    if discriminator_field:
        target["discriminator"] = discriminator_field


# ---------------------------------------------------------------------------
# Stats collection
# ---------------------------------------------------------------------------

def _count_tree(node: dict) -> tuple[int, int, int]:
    """Count structs, fields, and enums in a tree node (recursive)."""
    structs = 1  # this node itself
    fields = len(node.get("fields", []))
    enums = sum(1 for f in node.get("fields", []) if f.get("type") == "enum")

    for child in node.get("children", []):
        cs, cf, ce = _count_tree(child)
        structs += cs
        fields += cf
        enums += ce

    return structs, fields, enums


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def extract(markdown_text: str, provider: str, endpoint: str,
            source_url: str, raw_file: str,
            type_registry: dict[str, str] | None = None) -> dict[str, Any]:
    """
    Parse provider markdown documentation and return the intermediate JSON tree.

    *type_registry* is an optional mapping of shared type names to resolved
    type strings (built from dependency specs parsed earlier).
    """
    from datetime import datetime, timezone

    lines = markdown_text.split('\n')

    # Find section boundaries
    request_start = None
    response_start = None
    example_start = None
    for i, line in enumerate(lines):
        m = _SECTION_RE.match(line)
        if m:
            section_name = m.group(1).strip()
            if section_name == 'Body Parameters':
                request_start = i + 1
            elif section_name == 'Returns':
                response_start = i + 1
            elif section_name == 'Example':
                example_start = i

    result: dict[str, Any] = {
        "meta": {
            "provider": provider,
            "endpoint": endpoint,
            "source_url": source_url,
            "fetched_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "raw_file": raw_file,
        },
        "request": {"root": None},
        "response": {"root": None},
    }

    # Parse request section
    if request_start is not None:
        end = response_start if response_start is not None else (
            example_start if example_start is not None else len(lines))
        section_lines = lines[request_start:end]
        ctx = _ParseContext(section_lines, line_offset=request_start,
                            type_registry=type_registry)
        root = _make_struct_node("Request")
        _parse_fields(ctx, 0, root, ["Request"])
        _enrich_unions(root)
        result["request"]["root"] = root

    # Parse response section
    if response_start is not None:
        end = example_start if example_start is not None else len(lines)
        section_lines = lines[response_start:end]
        ctx = _ParseContext(section_lines, line_offset=response_start,
                            type_registry=type_registry)

        # The response often starts with "Message = object { ... }"
        # which _parse_fields handles via _NAMED_TYPE_RE
        root = _make_struct_node("Response")
        _parse_fields(ctx, 0, root, ["Response"])
        _enrich_unions(root)
        result["response"]["root"] = root

    return result
