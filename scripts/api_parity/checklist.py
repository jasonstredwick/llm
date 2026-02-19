"""
Stage 5 Prep: Generate an integration checklist from a diff JSON.

Reads the diff produced by Stage 3 and the current extraction JSON, then generates
a markdown checklist in scratch/api_parity/checklists/. Each diff item becomes one
or more checklist entries with the specific C++ files that need updating.

The checklist is the agent's work plan. Nothing in the diff is deferred or skipped.

Usage:
    from scripts.api_parity.checklist import checklist
    checklist("anthropic")
"""

import json
from pathlib import Path
from typing import Any

from . import config


# C++ file templates per provider
_FILE_TEMPLATES = {
    "anthropic": {
        "header": "interface/protocols/anthropic/messages.hpp",
        "strings": "interface/protocols/anthropic/strings.hpp",
        "serializer": "src/protocols/serialize/anthropic_messages.cpp",
        "deserializer": "src/protocols/deserialize/anthropic_messages.cpp",
        "tests": "tests/unit/test_anthropic.cpp",
    },
    "openai": {
        "header": "interface/protocols/openai/responses.hpp",
        "strings": "interface/protocols/openai/strings.hpp",
        "serializer": "src/protocols/serialize/openai_responses.cpp",
        "deserializer": "src/protocols/deserialize/openai_responses.cpp",
        "tests": "tests/unit/test_openai.cpp",
    },
    "gemini": {
        "header": "interface/protocols/gemini/generate_content.hpp",
        "strings": "interface/protocols/gemini/strings.hpp",
        "serializer": "src/protocols/serialize/gemini_generate_content.cpp",
        "deserializer": "src/protocols/deserialize/gemini_generate_content.cpp",
        "tests": "tests/unit/test_gemini.cpp",
    },
}


def _short(path: str) -> str:
    """Shorten a file path for display."""
    return path.rsplit("/", 1)[-1]


def _object_has_kind_field(obj_fields: dict) -> bool:
    """Check if an object definition has a 'type' field that is a kind discriminator."""
    type_field = obj_fields.get("type", {})
    return type_field.get("type") == "kind"


def _get_kind_value(obj_fields: dict) -> str | None:
    """Get the kind discriminator value if present."""
    type_field = obj_fields.get("type", {})
    if type_field.get("type") == "kind":
        return type_field.get("value")
    return None


def _classify_object(obj_name: str, extraction: dict, section: str) -> dict[str, Any]:
    """Classify an object to determine what files it touches."""
    obj = extraction.get(section, {}).get("objects", {}).get(obj_name, {})
    fields = obj.get("fields", {})

    info: dict[str, Any] = {
        "name": obj_name,
        "has_kind": _object_has_kind_field(fields),
        "kind_value": _get_kind_value(fields),
        "field_count": len(fields),
        "fields": fields,
    }
    return info


def _determine_variant_membership(obj_name: str, extraction: dict, section: str) -> list[str]:
    """
    Determine which variant types an object might belong to by scanning all
    objects in the extraction for union fields that reference it.
    """
    variants = []
    objects = extraction.get(section, {}).get("objects", {})

    for parent_name, parent_obj in objects.items():
        for field_name, field_def in parent_obj.get("fields", {}).items():
            ftype = field_def.get("type", "")
            values = field_def.get("values", [])

            # Check union values
            if ftype == "union" and obj_name in values:
                variants.append(f"{parent_name}.{field_name}")

            # Check array<ObjName>
            if ftype == f"array<{obj_name}>":
                variants.append(f"{parent_name}.{field_name}")

            # Check direct reference
            if ftype == obj_name:
                variants.append(f"{parent_name}.{field_name}")

    return variants


def _files_for_new_object(
    section: str, obj_info: dict, files: dict[str, str]
) -> list[str]:
    """Determine which files need updating for a new object."""
    affected = [_short(files["header"])]

    if obj_info["has_kind"]:
        affected.append(_short(files["strings"]))

    if section == "request":
        affected.append(_short(files["serializer"]))
    else:
        affected.append(_short(files["deserializer"]))

    affected.append(_short(files["tests"]))
    return affected


def _files_for_new_field(
    section: str, field_def: dict, files: dict[str, str]
) -> list[str]:
    """Determine which files need updating for a new field."""
    affected = [_short(files["header"])]

    ftype = field_def.get("type", "")
    has_values = "values" in field_def

    # If it's a string enum field, strings.hpp also needs updating
    if has_values and ftype == "string":
        affected.append(_short(files["strings"]))

    if section == "request":
        affected.append(_short(files["serializer"]))
    else:
        affected.append(_short(files["deserializer"]))

    # Required response fields affect all existing test JSON
    if section == "response" and field_def.get("required", False):
        affected.append(_short(files["tests"]))

    return affected


def _format_field_type(field_def: dict) -> str:
    """Format a field's type for display in the checklist."""
    ftype = field_def.get("type", "unknown")
    required = field_def.get("required", False)
    req_str = "required" if required else "optional"

    parts = [ftype]

    if "value" in field_def:
        parts.append(f'kind="{field_def["value"]}"')

    if "values" in field_def:
        parts.append(f'{len(field_def["values"])} values')

    return f"({', '.join(parts)}, {req_str})"


def _generate_section(
    section_name: str,
    section_diff: dict,
    extraction: dict,
    files: dict[str, str],
) -> list[str]:
    """Generate checklist lines for one section (request or response)."""
    lines: list[str] = []
    label = section_name.capitalize()

    # --- New objects ---
    added_objs = section_diff.get("added_objects", [])
    if added_objs:
        lines.append(f"## {label}: New Objects ({len(added_objs)})")
        lines.append("")
        for obj_name in sorted(added_objs):
            obj_info = _classify_object(obj_name, extraction, section_name)
            affected = _files_for_new_object(section_name, obj_info, files)
            files_str = ", ".join(affected)
            kind_note = ""
            if obj_info["kind_value"]:
                kind_note = f' (kind="{obj_info["kind_value"]}")'
            lines.append(
                f"- [ ] `{obj_name}`{kind_note} [{obj_info['field_count']} fields] "
                f"— {files_str}"
            )

            # Check variant membership
            variants = _determine_variant_membership(
                obj_name, extraction, section_name
            )
            for v in variants:
                lines.append(f"  - [ ] Add to variant `{v}` — {_short(files['header'])}")

        lines.append("")

    # --- Removed objects ---
    removed_objs = section_diff.get("removed_objects", [])
    if removed_objs:
        lines.append(f"## {label}: Removed Objects ({len(removed_objs)})")
        lines.append("")
        for obj_name in sorted(removed_objs):
            lines.append(
                f"- [ ] Remove `{obj_name}` — {_short(files['header'])}, "
                f"{_short(files['strings'])}, "
                f"{_short(files['serializer'] if section_name == 'request' else files['deserializer'])}, "
                f"{_short(files['tests'])}"
            )
        lines.append("")

    # --- New fields on existing objects ---
    added_fields = section_diff.get("added_fields", [])
    if added_fields:
        lines.append(f"## {label}: New Fields ({len(added_fields)})")
        lines.append("")
        for f in added_fields:
            type_str = _format_field_type(f)
            affected = _files_for_new_field(section_name, f, files)
            files_str = ", ".join(affected)
            lines.append(
                f"- [ ] `{f['object']}.{f['field']}` {type_str} — {files_str}"
            )
        lines.append("")

    # --- Removed fields ---
    removed_fields = section_diff.get("removed_fields", [])
    if removed_fields:
        lines.append(f"## {label}: Removed Fields ({len(removed_fields)})")
        lines.append("")
        for f in removed_fields:
            lines.append(
                f"- [ ] Remove `{f['object']}.{f['field']}` — "
                f"{_short(files['header'])}, "
                f"{_short(files['serializer'] if section_name == 'request' else files['deserializer'])}, "
                f"{_short(files['tests'])}"
            )
        lines.append("")

    # --- Changed fields ---
    changed_fields = section_diff.get("changed_fields", [])
    if changed_fields:
        lines.append(f"## {label}: Changed Fields ({len(changed_fields)})")
        lines.append("")
        for f in changed_fields:
            changes = f["changes"]
            change_parts = []
            if "type" in changes:
                change_parts.append(
                    f"type: `{changes['type']['old']}` → `{changes['type']['new']}`"
                )
            if "required" in changes:
                old_r = "required" if changes["required"]["old"] else "optional"
                new_r = "required" if changes["required"]["new"] else "optional"
                change_parts.append(f"{old_r} → {new_r}")
            if "values" in changes:
                old_v = changes["values"].get("old", [])
                new_v = changes["values"].get("new", [])
                added_v = set(new_v or []) - set(old_v or [])
                removed_v = set(old_v or []) - set(new_v or [])
                if added_v:
                    change_parts.append(f'+[{", ".join(sorted(added_v))}]')
                if removed_v:
                    change_parts.append(f'-[{", ".join(sorted(removed_v))}]')
            if "value" in changes:
                change_parts.append(
                    f'kind: "{changes["value"]["old"]}" → "{changes["value"]["new"]}"'
                )
            change_str = "; ".join(change_parts)
            lines.append(
                f"- [ ] `{f['object']}.{f['field']}` — {change_str} — "
                f"{_short(files['header'])}"
            )
        lines.append("")

    # --- New enums ---
    added_enums = section_diff.get("added_enum_values", [])
    if added_enums:
        # Group by enum name and count values
        by_enum: dict[str, list[str]] = {}
        for e in added_enums:
            by_enum.setdefault(e["enum"], []).append(e["value"])

        lines.append(f"## {label}: New Enums ({len(by_enum)})")
        lines.append("")
        for enum_name in sorted(by_enum):
            # Convert :: to _ for C++ naming convention
            cpp_name = enum_name.replace("::", "_")
            lines.append(
                f"- [ ] `{cpp_name}` "
                f"— {_short(files['header'])}, {_short(files['strings'])}"
            )
            for val in by_enum[enum_name]:
                lines.append(f"  - [ ] `{val}`")
        lines.append("")

    # --- Removed enums ---
    removed_enums = section_diff.get("removed_enum_values", [])
    if removed_enums:
        by_enum: dict[str, list[str]] = {}
        for e in removed_enums:
            by_enum.setdefault(e["enum"], []).append(e["value"])

        lines.append(f"## {label}: Removed Enums ({len(by_enum)})")
        lines.append("")
        for enum_name in sorted(by_enum):
            # Convert :: to _ for C++ naming convention
            cpp_name = enum_name.replace("::", "_")
            lines.append(
                f"- [ ] Remove `{cpp_name}` "
                f"— {_short(files['header'])}, {_short(files['strings'])}"
            )
            for val in by_enum[enum_name]:
                lines.append(f"  - [ ] `{val}`")
        lines.append("")

    return lines


def _generate_test_section(
    diff_data: dict, files: dict[str, str]
) -> list[str]:
    """Generate the unit test update section."""
    lines: list[str] = []
    test_file = _short(files["tests"])

    # Collect all new Required response fields — these affect existing tests
    required_response_fields = []
    for f in diff_data.get("response", {}).get("added_fields", []):
        if f.get("required", False):
            required_response_fields.append(f)

    new_response_objects = diff_data.get("response", {}).get("added_objects", [])
    new_request_objects = diff_data.get("request", {}).get("added_objects", [])

    if required_response_fields or new_response_objects or new_request_objects:
        lines.append("## Unit Test Updates")
        lines.append("")

        if required_response_fields:
            lines.append(
                f"- [ ] Update **all** existing response deserialization test JSON "
                f"with new Required fields — {test_file}"
            )
            for f in required_response_fields:
                lines.append(
                    f"  - `{f['object']}.{f['field']}` ({f.get('type', 'unknown')})"
                )

        if new_response_objects:
            lines.append(
                f"- [ ] Add deserialization tests for new response types — {test_file}"
            )
            for obj in sorted(new_response_objects):
                lines.append(f"  - `{obj}`")

        if new_request_objects:
            lines.append(
                f"- [ ] Add serialization tests for new request types — {test_file}"
            )
            for obj in sorted(new_request_objects):
                lines.append(f"  - `{obj}`")

        lines.append("")

    return lines


def _count_items(lines: list[str]) -> int:
    """Count the number of checklist items (lines starting with '- [ ]')."""
    return sum(1 for line in lines if line.lstrip().startswith("- [ ]"))


def checklist(provider: str) -> Path:
    """
    Generate an integration checklist from the diff JSON for a provider.

    Returns the path to the generated checklist markdown file.
    """
    diff_path = config.diff_json_path(provider)
    json_path = config.extracted_json_path(provider)
    checklist_path = config.checklist_md_path(provider)

    if not diff_path.exists():
        raise FileNotFoundError(
            f"Diff not found: {diff_path}\n"
            f"Run 'audit --provider {provider}' first."
        )
    if not json_path.exists():
        raise FileNotFoundError(
            f"Extraction not found: {json_path}\n"
            f"Run 'extract --provider {provider}' first."
        )

    diff_data = json.loads(diff_path.read_text(encoding="utf-8"))
    extraction = json.loads(json_path.read_text(encoding="utf-8"))

    cfg = config.ENDPOINTS[provider]
    files = _FILE_TEMPLATES.get(provider)
    if files is None:
        raise ValueError(
            f"No C++ file template defined for provider '{provider}'. "
            f"Add an entry to _FILE_TEMPLATES in checklist.py."
        )

    meta = diff_data.get("meta", {})

    # Build the checklist
    lines: list[str] = [
        f"# Integration Checklist: {cfg.provider.capitalize()} {cfg.endpoint.capitalize()}",
        f"Generated from: `{diff_path.name}`",
        f"Baseline: {meta.get('baseline_fetched_at', 'unknown')} | "
        f"Current: {meta.get('current_fetched_at', 'unknown')}",
        "",
        "See `docs/api_parity_integration.md` for the integration process and file mapping.",
        "",
    ]

    # Request section
    req_lines = _generate_section("request", diff_data.get("request", {}), extraction, files)
    lines.extend(req_lines)

    # Response section
    resp_lines = _generate_section("response", diff_data.get("response", {}), extraction, files)
    lines.extend(resp_lines)

    # Test section
    test_lines = _generate_test_section(diff_data, files)
    lines.extend(test_lines)

    # Summary
    total = _count_items(lines)
    no_changes = total == 0

    if no_changes:
        lines.append("## No Changes")
        lines.append("")
        lines.append("The diff contains no structural changes. Nothing to integrate.")
        lines.append("")
    else:
        # Insert summary after the header
        summary = f"**Total items: {total}**"
        lines.insert(5, summary)
        lines.insert(6, "")

    # Final verification step
    if not no_changes:
        lines.append("## Verification")
        lines.append("")
        lines.append("- [ ] Build: `cmake --build build --config Release` compiles cleanly")
        lines.append("- [ ] Tests: unit tests pass")
        lines.append("- [ ] Re-audit: `python scripts/api_parity/run.py audit --provider "
                      f"{provider}` shows 0 structural changes")
        lines.append("- [ ] Promote: `python scripts/api_parity/run.py promote --provider "
                      f"{provider}`")
        lines.append("")

    checklist_path.parent.mkdir(parents=True, exist_ok=True)
    checklist_path.write_text("\n".join(lines), encoding="utf-8")

    print(f"Generated checklist: {checklist_path}")
    print(f"  {total} items to integrate")

    return checklist_path
