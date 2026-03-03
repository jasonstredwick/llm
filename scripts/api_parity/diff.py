"""
Stage 3: Diff — Compare current extraction against previous baseline.

Recursively compares the two JSON trees, ignoring the ``meta`` block
(which contains timestamps and other non-structural bookkeeping).
Any structural difference triggers a full regeneration.

Usage:
    from scripts.api_parity.diff import diff
    diff("openai")
"""

import json
from pathlib import Path

from . import config


def _sorted(obj: object) -> object:
    """Return *obj* with all dicts sorted by key (recursively)."""
    if isinstance(obj, dict):
        return {k: _sorted(v) for k, v in sorted(obj.items())}
    if isinstance(obj, list):
        return [_sorted(v) for v in obj]
    return obj


def _changed(baseline: object, current: object) -> bool:
    """Return True on the first structural difference between two trees."""
    return _sorted(baseline) != _sorted(current)


def diff(provider: str) -> bool:
    """
    Compare current extraction against baseline for a provider.

    Returns True if there are structural changes (or no baseline exists).
    """
    json_path = config.extracted_json_path(provider)
    baseline_path = config.baseline_json_path(provider)

    if not json_path.exists():
        raise FileNotFoundError(
            f"Current extraction not found: {json_path}\n"
            f"Run 'extract --provider {provider}' first."
        )

    if not baseline_path.exists():
        print(f"No baseline found for {provider} — treating as new.")
        return True

    current = json.loads(json_path.read_text(encoding="utf-8"))
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))

    # Strip non-structural metadata before comparing.
    current.pop("meta", None)
    baseline.pop("meta", None)

    print(f"Diffing {provider}: {baseline_path.name} vs {json_path.name}")

    changed = _changed(baseline, current)

    if changed:
        print(f"  Changes detected for {provider}.")
    else:
        print(f"  No structural changes for {provider}.")

    return changed
