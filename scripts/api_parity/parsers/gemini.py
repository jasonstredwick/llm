"""
Parser for Google API Discovery Document JSON.

Reads a discovery document (e.g. from generativelanguage.googleapis.com)
and produces an intermediate JSON suitable for C++ code generation.

The discovery format provides flat schemas with $ref pointers between them.
Unlike the OpenAI/Anthropic markdown format, Gemini types are shared across
request and response (e.g. Content, Part).  This parser preserves that flat
structure rather than forcing everything into a nested tree.

Intermediate output structure:
{
    "meta": { ... },
    "schemas": {
        "TypeName": {
            "name": "TypeName",
            "description": "...",
            "side": "request" | "response" | "shared",
            "fields": [
                {
                    "name": "fieldName",
                    "type": "string" | "int32" | "int64" | "double" | "boolean"
                           | "TypeRef" | "array<TypeRef>" | "map<string, TypeRef>"
                           | "any",
                    "required": true/false,
                    "description": "..."
                },
                ...  # enum fields have "type": "enum", "values": [...]
            ]
        },
        ...
    },
    "request_type": "GenerateContentRequest",
    "response_type": "GenerateContentResponse",
}
"""

import json
import re
import sys
from pathlib import Path
from typing import Any


# ---------------------------------------------------------------------------
# Type mapping from discovery JSON Schema types to our intermediate types
# ---------------------------------------------------------------------------

_TYPE_MAP = {
    "string": "string",
    "boolean": "boolean",
    "integer": "int64",   # default; overridden by format
    "number": "double",   # default; overridden by format
    "object": "object",
    "any": "any",
}

_FORMAT_MAP = {
    "int32": "int32",
    "int64": "int64",
    "uint32": "uint32",
    "float": "float",
    "double": "double",
    "byte": "string",     # base64-encoded bytes → string in C++
    "date-time": "string",
}


def _resolve_prop_type(prop: dict) -> str:
    """Resolve a discovery property definition to our type string."""
    # Direct $ref → named type reference
    if "$ref" in prop:
        return prop["$ref"]

    # Array
    if prop.get("type") == "array":
        items = prop.get("items", {})
        if "$ref" in items:
            return f"array<{items['$ref']}>"
        inner = _resolve_scalar_type(items)
        return f"array<{inner}>"

    # Map (object with additionalProperties)
    if prop.get("type") == "object" and "additionalProperties" in prop:
        ap = prop["additionalProperties"]
        # When additionalProperties is {type: "any"} or untyped, the
        # object is really "arbitrary JSON" → just "object" (json::Object).
        ap_type = ap.get("type", "any") if isinstance(ap, dict) else "any"
        if ap_type == "any":
            return "object"
        if "$ref" in ap:
            return f"map<string, {ap['$ref']}>"
        inner = _resolve_scalar_type(ap)
        return f"map<string, {inner}>"

    return _resolve_scalar_type(prop)


def _resolve_scalar_type(prop: dict) -> str:
    """Resolve a scalar (non-array, non-ref) property type."""
    fmt = prop.get("format")
    if fmt and fmt in _FORMAT_MAP:
        return _FORMAT_MAP[fmt]
    base = prop.get("type", "any")
    return _TYPE_MAP.get(base, base)


# ---------------------------------------------------------------------------
# Reachability analysis
# ---------------------------------------------------------------------------

def _collect_reachable(schema_name: str, schemas: dict[str, dict],
                       visited: set[str] | None = None) -> set[str]:
    """Collect all schema names reachable from *schema_name* via $ref.

    Skips schemas marked deprecated.
    """
    if visited is None:
        visited = set()
    if schema_name in visited or schema_name not in schemas:
        return visited
    if schemas[schema_name].get("deprecated", False):
        return visited
    visited.add(schema_name)
    schema = schemas[schema_name]
    for prop in schema.get("properties", {}).values():
        if "$ref" in prop:
            _collect_reachable(prop["$ref"], schemas, visited)
        items = prop.get("items", {})
        if "$ref" in items:
            _collect_reachable(items["$ref"], schemas, visited)
        ap = prop.get("additionalProperties", {})
        if isinstance(ap, dict) and "$ref" in ap:
            _collect_reachable(ap["$ref"], schemas, visited)
    return visited


# ---------------------------------------------------------------------------
# Schema conversion
# ---------------------------------------------------------------------------

def _convert_schema(name: str, schema: dict, side: str,
                    all_schemas: dict | None = None) -> dict[str, Any]:
    """Convert a discovery schema to our intermediate format.

    Skips deprecated fields and fields referencing deprecated schemas.
    """
    result: dict[str, Any] = {
        "name": name,
        "side": side,
        "fields": [],
    }

    desc = schema.get("description", "")
    if desc:
        result["description"] = desc

    properties = schema.get("properties", {})
    required_fields = set(schema.get("required", []))

    _all = all_schemas or {}

    for prop_name, prop in properties.items():
        # Skip deprecated fields
        if prop.get("deprecated", False):
            continue

        # Skip fields that reference deprecated schemas
        ref = prop.get("$ref", "")
        if not ref:
            items = prop.get("items", {})
            ref = items.get("$ref", "") if isinstance(items, dict) else ""
        if ref and ref in _all and _all[ref].get("deprecated", False):
            continue

        field: dict[str, Any] = {"name": prop_name}

        prop_desc = prop.get("description", "")

        # Check for enum
        if "enum" in prop:
            field["type"] = "enum"
            field["values"] = prop["enum"]
            if "enumDescriptions" in prop:
                field["enum_descriptions"] = prop["enumDescriptions"]
        else:
            field["type"] = _resolve_prop_type(prop)

        # Requiredness: discovery docs use "Required." / "Optional." in
        # description, plus explicit "required" array, plus readOnly flag.
        is_read_only = prop.get("readOnly", False)
        explicitly_required = prop_name in required_fields
        desc_says_required = prop_desc.startswith("Required.")
        desc_says_optional = prop_desc.startswith("Optional.")

        if side == "response" or is_read_only:
            # Response fields default to optional (populated by server)
            field["required"] = False
        elif explicitly_required or desc_says_required:
            field["required"] = True
        elif desc_says_optional:
            field["required"] = False
        else:
            # Default: required for request, optional for response/shared
            field["required"] = side == "request"

        if prop_desc:
            field["description"] = prop_desc

        result["fields"].append(field)

    return result


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def extract(discovery_path: str, provider: str, endpoint: str,
            source_url: str) -> dict[str, Any]:
    """
    Parse a Google Discovery Document and return intermediate JSON.

    Parameters
    ----------
    discovery_path : str
        Path to the discovery JSON file.
    provider : str
        Provider name (e.g. "gemini").
    endpoint : str
        Endpoint name (e.g. "generate_content").
    source_url : str
        URL the discovery document was fetched from.

    Returns
    -------
    dict
        Intermediate JSON with flat schema list.
    """
    from datetime import datetime, timezone

    path = Path(discovery_path)
    doc = json.loads(path.read_text(encoding="utf-8"))
    schemas = doc.get("schemas", {})

    # Determine request/response type names from the method definition
    req_type = "GenerateContentRequest"
    resp_type = "GenerateContentResponse"

    # Reachability analysis
    req_reachable = _collect_reachable(req_type, schemas)
    resp_reachable = _collect_reachable(resp_type, schemas)
    shared = req_reachable & resp_reachable
    req_only = req_reachable - shared
    resp_only = resp_reachable - shared
    all_needed = req_reachable | resp_reachable

    print(f"  Reachable from request:  {len(req_reachable)} types", file=sys.stderr)
    print(f"  Reachable from response: {len(resp_reachable)} types", file=sys.stderr)
    print(f"  Shared:                  {len(shared)} types", file=sys.stderr)
    print(f"  Request-only:            {len(req_only)} types", file=sys.stderr)
    print(f"  Response-only:           {len(resp_only)} types", file=sys.stderr)
    print(f"  Total needed:            {len(all_needed)} types", file=sys.stderr)

    # Convert each reachable schema
    converted: dict[str, Any] = {}
    for name in sorted(all_needed):
        if name not in schemas:
            print(f"  [warn] Referenced schema '{name}' not found in document",
                  file=sys.stderr)
            continue
        if name in shared:
            side = "shared"
        elif name in req_only:
            side = "request"
        else:
            side = "response"
        converted[name] = _convert_schema(name, schemas[name], side,
                                           all_schemas=schemas)

    # Stats
    total_fields = sum(len(s["fields"]) for s in converted.values())
    total_enums = sum(
        1 for s in converted.values()
        for f in s["fields"] if f["type"] == "enum"
    )
    print(f"  Converted: {len(converted)} schemas, "
          f"{total_fields} fields, {total_enums} enum fields",
          file=sys.stderr)

    result: dict[str, Any] = {
        "meta": {
            "provider": provider,
            "endpoint": endpoint,
            "source_url": source_url,
            "fetched_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "parser": "discovery",
            "discovery_version": doc.get("version", ""),
            "discovery_revision": doc.get("revision", ""),
        },
        "schemas": converted,
        "request_type": req_type,
        "response_type": resp_type,
    }

    return result
