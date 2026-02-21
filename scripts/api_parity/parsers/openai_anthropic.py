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
    "string or number"   -> "string | number"
    "map[unknown]"       -> "map<string, unknown>"
    "object { a, b }"    -> "object"
    """
    if ' or ' in s:
        parts = s.split(' or ')
        parts = [p for p in parts if not re.match(r'^\d+\s+more$', p.strip())]
        normalized = [_normalize_type(p.strip()) for p in parts]
        return ' | '.join(normalized)

    m = re.match(r'^array\s+of\s+(.+)$', s)
    if m:
        inner = _normalize_type(m.group(1).strip())
        return f"array<{inner}>"

    if s.startswith('object'):
        return "object"

    m = re.match(r'^map\[(.+)\]$', s)
    if m:
        return f"map<string, {m.group(1)}>"

    if s.startswith('"') and s.endswith('"'):
        return s

    return s


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
# Parse context
# ---------------------------------------------------------------------------

class _ParseContext:
    """Holds state during parsing."""

    def __init__(self, lines: list[str], line_offset: int = 0):
        self.lines = lines
        self.pos = 0
        self.line_offset = line_offset  # for error reporting

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
    """Collect enum literal values at a given indentation level."""
    values: list[str] = []
    while not ctx.at_end():
        line = ctx.peek()
        if not line.strip():
            ctx.advance()
            continue
        m = _ENUM_VALUE_RE.match(line)
        if m and _indent_level(m.group(1)) >= child_indent:
            values.append(m.group(2))
            ctx.advance()
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

            desc = _collect_description(ctx, indent)

            # Request fields: default to required (docs explicitly mark
            # optional ones with "optional" in the type).
            # Response fields: default to optional (docs don't specify
            # requiredness at all).
            is_response = ancestry[0] == "Response" if ancestry else False
            required = False if is_response else not optional
            field = _make_field(field_name, type_str, required)
            if desc:
                field["description"] = desc

            # Classify the field type
            if (type_str.startswith('"') and type_str.endswith('"')
                    and ' | ' not in type_str):
                # Kind field: single literal string value
                field["type"] = "kind"
                field["value"] = type_str.strip('"')
                # Set the kind on the parent struct
                parent_node["kind"] = type_str.strip('"')
                # Consume any child enum values that just confirm the kind
                _collect_enum_values(ctx, indent + 1)

            elif ' | ' in type_str:
                parts = [p.strip() for p in type_str.split(' | ')]
                if all(p.startswith('"') and p.endswith('"') for p in parts):
                    # All quoted literals → enum
                    field["type"] = "enum"
                    field["values"] = [p.strip('"') for p in parts]
                else:
                    # Union type
                    field["type"] = "union"
                    field["members"] = []
                    for p in parts:
                        member = _classify_union_member(p)
                        field["members"].append(member)
                    # Parse children (UnionMember definitions, inline objects)
                    _parse_union_children(ctx, indent, field, parent_node, ancestry)
            else:
                # Check for child enum values
                child_enums = _collect_enum_values(ctx, indent + 1)
                if child_enums and field["type"] not in ("kind",):
                    field["type"] = "enum"
                    field["values"] = child_enums
                else:
                    # Parse inline child definitions (sub-fields or named types)
                    _parse_field_children(ctx, indent, field, parent_node, ancestry)

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

            if type_name.startswith('UnionMember'):
                # UnionMember — normalize and track
                normalized = _normalize_type(type_expr)
                elem_name = _extract_array_element_name(type_expr)
                if elem_name:
                    # Array type — children define the element type, not the
                    # parent union.  Create the element struct (from inline
                    # fields) or consume object children that form the element
                    # variant.  Either way keep the result as array<X>.
                    created = _maybe_create_array_element_struct(
                        ctx, indent, type_expr, parent_node, ancestry)
                    if not created:
                        # Children are named object types forming X's variant.
                        # Consume them (they become children of parent_node)
                        # but do NOT add them as flat union members.
                        _scan_for_nested_objects(ctx, indent, parent_node,
                                                 ancestry)
                    member = _classify_union_member(normalized)
                    rebuilt_members.append(member)
                else:
                    # Non-array UnionMember — children may define it further
                    child_result = _scan_union_member_children(
                        ctx, indent, parent_node, ancestry)
                    if child_result.get("members"):
                        rebuilt_members.extend(child_result["members"])
                    elif child_result.get("enum_values"):
                        collected_enum_values.extend(child_result["enum_values"])
                    else:
                        member = _classify_union_member(normalized)
                        rebuilt_members.append(member)
            elif type_expr.startswith('object'):
                # Inline object definition for a union arm
                child_node = _make_struct_node(type_name)
                if desc:
                    child_node["description"] = desc
                _parse_fields(ctx, indent + 1, child_node,
                              ancestry + [type_name])
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
                rebuilt_members.append({"type": "struct", "ref": type_name})
            else:
                created = _maybe_create_array_element_struct(
                    ctx, indent, type_expr, parent_node, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, parent_node, ancestry)
                member = _classify_union_member(_normalize_type(type_expr))
                rebuilt_members.append(member)
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
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
                break
            break

        # Unrecognized — check indent
        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_indent * 2:
            ctx.advance()
            continue
        break

    # Rebuild field members from what we found
    if rebuilt_members:
        field["members"] = rebuilt_members
    if collected_enum_values:
        # If we only found enum values and no type members, convert to enum
        non_string = [m for m in field.get("members", [])
                      if m.get("type") != "string" and m.get("type") != "literal"]
        if not non_string:
            field["type"] = "enum"
            field["values"] = collected_enum_values
            if "members" in field:
                del field["members"]
        else:
            # Mixed — keep as union, note the enum values
            field["enum_values"] = collected_enum_values


def _scan_union_member_children(ctx: _ParseContext, parent_indent: int,
                                parent_node: dict,
                                ancestry: list[str]) -> dict:
    """
    Scan children of a UnionMember definition.
    Returns dict with "members" and/or "enum_values".
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
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
                result["members"].append({"type": "struct", "ref": type_name})
            elif type_name.startswith('UnionMember'):
                normalized = _normalize_type(type_expr)
                child_result = _scan_union_member_children(
                    ctx, indent, parent_node, ancestry)
                if child_result["members"]:
                    result["members"].extend(child_result["members"])
                elif child_result["enum_values"]:
                    result["enum_values"].extend(child_result["enum_values"])
                else:
                    member = _classify_union_member(normalized)
                    result["members"].append(member)
            else:
                created = _maybe_create_array_element_struct(
                    ctx, indent, type_expr, parent_node, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, parent_node, ancestry)
                member = _classify_union_member(_normalize_type(type_expr))
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
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
                # Record as a potential union member
                if union_type_name:
                    union_members.append({"type": "struct", "ref": type_name})
            else:
                # Check if this is an array type with inline field children
                # that define the element struct (e.g. "array of WebSearchResultBlock"
                # followed by field definitions for WebSearchResultBlock).
                created = _maybe_create_array_element_struct(
                    ctx, indent, type_expr, parent_node, ancestry)
                if not created:
                    _scan_for_nested_objects(ctx, indent, parent_node, ancestry)
                # Non-object named types under a union-candidate field
                if union_type_name:
                    normalized = _normalize_type(type_expr)
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
                _attach_child_if_new(parent_node, child_node, ancestry,
                                     ctx.source_line())
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

        stripped = line.lstrip()
        line_char_indent = len(line) - len(stripped)
        if line_char_indent >= child_indent * 2:
            ctx.advance()
            continue
        break

    # If we found multiple named-object children under a named-type field,
    # record them as an explicit union definition on the field.
    if union_type_name and len(union_members) >= 2:
        field["union_def"] = {
            "name": union_type_name,
            "members": union_members,
        }


def _extract_array_element_name(type_expr: str) -> str | None:
    """Extract element type name from 'array of Foo' or 'array<Foo>'."""
    m = re.match(r'^array\s+of\s+(\w+)$', type_expr)
    if m:
        name = m.group(1)
        if name[0].isupper() and name.lower() not in (
                'string', 'number', 'boolean', 'unknown', 'object'):
            return name
    m = re.match(r'^array<(\w+)>$', type_expr)
    if m:
        name = m.group(1)
        if name[0].isupper() and name.lower() not in (
                'string', 'number', 'boolean', 'unknown', 'object'):
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
        # Enrich explicit union fields
        if field.get("type") == "union" and "members" in field:
            _enrich_member_list(field["members"], field, children_by_name)

        # Enrich union_def (synthesised from _parse_field_children)
        udef = field.get("union_def")
        if udef and "members" in udef:
            _enrich_member_list(udef["members"], udef, children_by_name)

    # Recurse into children
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
            source_url: str, raw_file: str) -> dict[str, Any]:
    """
    Parse provider markdown documentation and return the intermediate JSON tree.
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
        ctx = _ParseContext(section_lines, line_offset=request_start)
        root = _make_struct_node("Request")
        _parse_fields(ctx, 0, root, ["Request"])
        _enrich_unions(root)
        result["request"]["root"] = root

    # Parse response section
    if response_start is not None:
        end = example_start if example_start is not None else len(lines)
        section_lines = lines[response_start:end]
        ctx = _ParseContext(section_lines, line_offset=response_start)

        # The response often starts with "Message = object { ... }"
        # which _parse_fields handles via _NAMED_TYPE_RE
        root = _make_struct_node("Response")
        _parse_fields(ctx, 0, root, ["Response"])
        _enrich_unions(root)
        result["response"]["root"] = root

    return result
