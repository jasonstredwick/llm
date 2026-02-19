"""
Stage 3: Diff — Compare current extraction against previous baseline.

Structural diff only: compares objects, fields (type + required), and enum values.
Description changes are noted but kept separate from structural changes.

Usage:
    from scripts.api_parity.diff import diff
    diff("openai")
"""

import json
from pathlib import Path
from typing import Any

from . import config


def _diff_section(baseline: dict, current: dict) -> dict[str, Any]:
    """
    Diff a single section (request or response).

    Returns a dict with added/removed/changed objects, fields, enums, and
    description changes.
    """
    b_objects = baseline.get("objects", {})
    c_objects = current.get("objects", {})
    b_enums = baseline.get("enums", {})
    c_enums = current.get("enums", {})

    result: dict[str, Any] = {
        "added_objects": [],
        "removed_objects": [],
        "added_fields": [],
        "removed_fields": [],
        "changed_fields": [],
        "added_enum_values": [],
        "removed_enum_values": [],
        "description_changes": [],
    }

    # Object-level diff
    b_obj_names = set(b_objects.keys())
    c_obj_names = set(c_objects.keys())

    result["added_objects"] = sorted(c_obj_names - b_obj_names)
    result["removed_objects"] = sorted(b_obj_names - c_obj_names)

    # Field-level diff for objects present in both
    for obj_name in sorted(b_obj_names & c_obj_names):
        b_fields = b_objects[obj_name].get("fields", {})
        c_fields = c_objects[obj_name].get("fields", {})

        b_field_names = set(b_fields.keys())
        c_field_names = set(c_fields.keys())

        # Added fields
        for fname in sorted(c_field_names - b_field_names):
            f = c_fields[fname]
            entry: dict[str, Any] = {
                "object": obj_name,
                "field": fname,
                "type": f.get("type", "unknown"),
                "required": f.get("required", False),
            }
            if "value" in f:
                entry["value"] = f["value"]
            if "values" in f:
                entry["values"] = f["values"]
            result["added_fields"].append(entry)

        # Removed fields
        for fname in sorted(b_field_names - c_field_names):
            result["removed_fields"].append({
                "object": obj_name,
                "field": fname,
            })

        # Changed fields (present in both)
        for fname in sorted(b_field_names & c_field_names):
            bf = b_fields[fname]
            cf = c_fields[fname]
            changes: dict[str, Any] = {}

            # Structural: type
            if bf.get("type") != cf.get("type"):
                changes["type"] = {"old": bf.get("type"), "new": cf.get("type")}

            # Structural: required
            if bf.get("required") != cf.get("required"):
                changes["required"] = {"old": bf.get("required"), "new": cf.get("required")}

            # Structural: value (kind discriminator)
            if bf.get("value") != cf.get("value"):
                changes["value"] = {"old": bf.get("value"), "new": cf.get("value")}

            # Structural: values (string enum)
            if bf.get("values") != cf.get("values"):
                changes["values"] = {"old": bf.get("values"), "new": cf.get("values")}

            if changes:
                result["changed_fields"].append({
                    "object": obj_name,
                    "field": fname,
                    "changes": changes,
                })

            # Description changes (non-structural, tracked separately)
            b_desc = bf.get("description", "")
            c_desc = cf.get("description", "")
            if b_desc != c_desc:
                result["description_changes"].append({
                    "object": obj_name,
                    "field": fname,
                    "old_desc": b_desc,
                    "new_desc": c_desc,
                })

    # Enum-level diff
    b_enum_names = set(b_enums.keys())
    c_enum_names = set(c_enums.keys())

    for enum_name in sorted(c_enum_names - b_enum_names):
        for val in c_enums[enum_name].get("values", []):
            result["added_enum_values"].append({"enum": enum_name, "value": val})

    for enum_name in sorted(b_enum_names - c_enum_names):
        for val in b_enums[enum_name].get("values", []):
            result["removed_enum_values"].append({"enum": enum_name, "value": val})

    for enum_name in sorted(b_enum_names & c_enum_names):
        b_vals = set(b_enums[enum_name].get("values", []))
        c_vals = set(c_enums[enum_name].get("values", []))
        for val in sorted(c_vals - b_vals):
            result["added_enum_values"].append({"enum": enum_name, "value": val})
        for val in sorted(b_vals - c_vals):
            result["removed_enum_values"].append({"enum": enum_name, "value": val})

    return result


def diff(provider: str) -> Path:
    """
    Compare current extraction against baseline for a provider.

    Returns the path to the diff JSON file.
    """
    json_path = config.extracted_json_path(provider)
    baseline_path = config.baseline_json_path(provider)
    diff_path = config.diff_json_path(provider)

    if not json_path.exists():
        raise FileNotFoundError(
            f"Current extraction not found: {json_path}\n"
            f"Run 'extract --provider {provider}' first."
        )
    if not baseline_path.exists():
        raise FileNotFoundError(
            f"Baseline not found: {baseline_path}\n"
            f"Run 'promote --provider {provider}' to establish a baseline first."
        )

    current = json.loads(json_path.read_text(encoding="utf-8"))
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))

    print(f"Diffing {provider}: {baseline_path.name} vs {json_path.name}")

    result = {
        "meta": {
            "provider": provider,
            "baseline_file": str(baseline_path.name),
            "current_file": str(json_path.name),
            "baseline_fetched_at": baseline.get("meta", {}).get("fetched_at", "unknown"),
            "current_fetched_at": current.get("meta", {}).get("fetched_at", "unknown"),
        },
        "request": _diff_section(
            baseline.get("request", {}),
            current.get("request", {}),
        ),
        "response": _diff_section(
            baseline.get("response", {}),
            current.get("response", {}),
        ),
    }

    diff_path.parent.mkdir(parents=True, exist_ok=True)
    diff_path.write_text(
        json.dumps(result, indent=2, ensure_ascii=False) + '\n',
        encoding="utf-8",
    )

    # Summary
    for section in ("request", "response"):
        s = result[section]
        total = (len(s["added_objects"]) + len(s["removed_objects"])
                 + len(s["added_fields"]) + len(s["removed_fields"])
                 + len(s["changed_fields"])
                 + len(s["added_enum_values"]) + len(s["removed_enum_values"]))
        desc_changes = len(s["description_changes"])
        print(f"  {section}: {total} structural changes, {desc_changes} description changes")

    print(f"  -> {diff_path}")
    return diff_path
