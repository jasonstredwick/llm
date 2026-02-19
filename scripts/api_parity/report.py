"""
Stage 4: Report — Generate a human-readable markdown report from the diff.

Usage:
    from scripts.api_parity.report import report
    report("openai")
"""

import json
from datetime import datetime, timezone
from pathlib import Path

from . import config


def _section_report(section_name: str, data: dict) -> str:
    """Generate report text for one section (request or response)."""
    lines: list[str] = []

    # Added objects
    added_objs = data.get("added_objects", [])
    if added_objs:
        lines.append(f"### New Objects ({len(added_objs)}) — {section_name}")
        for name in added_objs:
            lines.append(f"- {name}")
        lines.append("")

    # Removed objects
    removed_objs = data.get("removed_objects", [])
    if removed_objs:
        lines.append(f"### Removed Objects ({len(removed_objs)}) — {section_name}")
        for name in removed_objs:
            lines.append(f"- {name}")
        lines.append("")

    # Added fields
    added_fields = data.get("added_fields", [])
    if added_fields:
        lines.append(f"### New Fields ({len(added_fields)}) — {section_name}")
        for f in added_fields:
            req = "required" if f["required"] else "optional"
            type_display = f['type']
            if f.get("value"):
                type_display = f"kind={f['value']}"
            elif f.get("values"):
                vals = ', '.join(f['values'])
                if f['type'] == 'union':
                    type_display = f"union({vals})"
                else:
                    type_display = f"string [{vals}]"
            lines.append(f"- {f['object']}.{f['field']}: {type_display} ({req})")
        lines.append("")

    # Removed fields
    removed_fields = data.get("removed_fields", [])
    if removed_fields:
        lines.append(f"### Removed Fields ({len(removed_fields)}) — {section_name}")
        for f in removed_fields:
            lines.append(f"- {f['object']}.{f['field']}")
        lines.append("")

    # Changed fields
    changed_fields = data.get("changed_fields", [])
    if changed_fields:
        lines.append(f"### Changed Fields ({len(changed_fields)}) — {section_name}")
        for f in changed_fields:
            changes = f["changes"]
            parts = []
            if "type" in changes:
                parts.append(f"type: {changes['type']['old']} → {changes['type']['new']}")
            if "required" in changes:
                old_r = "required" if changes["required"]["old"] else "optional"
                new_r = "required" if changes["required"]["new"] else "optional"
                parts.append(f"{old_r} → {new_r}")
            if "value" in changes:
                parts.append(f"kind: {changes['value']['old']} → {changes['value']['new']}")
            if "values" in changes:
                old_v = ', '.join(changes['values']['old'] or [])
                new_v = ', '.join(changes['values']['new'] or [])
                parts.append(f"values: [{old_v}] → [{new_v}]")
            lines.append(f"- {f['object']}.{f['field']}: {', '.join(parts)}")
        lines.append("")

    # Added enum values
    added_enums = data.get("added_enum_values", [])
    if added_enums:
        lines.append(f"### New Enum Values ({len(added_enums)}) — {section_name}")
        for e in added_enums:
            lines.append(f"- {e['enum']}: \"{e['value']}\"")
        lines.append("")

    # Removed enum values
    removed_enums = data.get("removed_enum_values", [])
    if removed_enums:
        lines.append(f"### Removed Enum Values ({len(removed_enums)}) — {section_name}")
        for e in removed_enums:
            lines.append(f"- {e['enum']}: \"{e['value']}\"")
        lines.append("")

    # Description changes
    desc_changes = data.get("description_changes", [])
    if desc_changes:
        lines.append(f"### Description Changes ({len(desc_changes)}) — {section_name}")
        for d in desc_changes:
            lines.append(f"- {d['object']}.{d['field']}")
        lines.append("")

    return '\n'.join(lines)


def _has_changes(data: dict) -> bool:
    """Check if a section has any changes."""
    for key in ("added_objects", "removed_objects", "added_fields",
                "removed_fields", "changed_fields", "added_enum_values",
                "removed_enum_values", "description_changes"):
        if data.get(key):
            return True
    return False


def report(provider: str) -> Path:
    """
    Generate a markdown report from the diff output.

    Returns the path to the generated report.
    """
    cfg = config.ENDPOINTS[provider]
    diff_path = config.diff_json_path(provider)
    report_path = config.report_md_path(provider)

    if not diff_path.exists():
        raise FileNotFoundError(
            f"Diff not found: {diff_path}\n"
            f"Run 'diff --provider {provider}' first."
        )

    diff_data = json.loads(diff_path.read_text(encoding="utf-8"))
    meta = diff_data.get("meta", {})

    lines: list[str] = []
    lines.append(f"# API Parity Report: {cfg.provider.title()} {cfg.endpoint.title()}")
    lines.append(f"Generated: {datetime.now(timezone.utc).strftime('%Y-%m-%d')}")
    lines.append(f"Baseline: {meta.get('baseline_fetched_at', 'unknown')}  |  "
                 f"Current: {meta.get('current_fetched_at', 'unknown')}")
    lines.append("")

    req_data = diff_data.get("request", {})
    resp_data = diff_data.get("response", {})

    has_any = _has_changes(req_data) or _has_changes(resp_data)

    if _has_changes(req_data):
        lines.append("## Request Changes")
        lines.append("")
        lines.append(_section_report("Request", req_data))

    if _has_changes(resp_data):
        lines.append("## Response Changes")
        lines.append("")
        lines.append(_section_report("Response", resp_data))

    if not has_any:
        lines.append("## No Changes")
        lines.append("")
        lines.append("No structural or description changes detected between baseline and current.")
        lines.append("")

    report_text = '\n'.join(lines)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(report_text, encoding="utf-8")

    print(f"Report written to {report_path}")
    if not has_any:
        print("  No changes detected.")
    return report_path
