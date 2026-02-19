#!/usr/bin/env python3
"""
CLI entry point for the API parity pipeline.

Usage:
    python -m scripts.api_parity.run fetch     --provider openai
    python -m scripts.api_parity.run extract   --provider openai
    python -m scripts.api_parity.run diff      --provider openai
    python -m scripts.api_parity.run report    --provider openai
    python -m scripts.api_parity.run checklist --provider openai   # generate integration checklist
    python -m scripts.api_parity.run promote   --provider openai
    python -m scripts.api_parity.run audit     --provider openai   # fetch+extract+diff+report
    python -m scripts.api_parity.run audit     --all               # all providers

Or from the repo root:
    python scripts/api_parity/run.py fetch --provider openai
"""

import argparse
import shutil
import sys
from pathlib import Path

# Ensure repo root is on sys.path so imports work when run as a script
_REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(_REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(_REPO_ROOT))

from scripts.api_parity import config
from scripts.api_parity.fetch import fetch as do_fetch
from scripts.api_parity.extract import extract as do_extract
from scripts.api_parity.diff import diff as do_diff
from scripts.api_parity.report import report as do_report
from scripts.api_parity.checklist import checklist as do_checklist


def do_promote(provider: str) -> None:
    """Copy the current extraction and markdown from scratch to docs/specs as baseline."""
    json_src = config.extracted_json_path(provider)
    md_src = config.raw_md_path(provider)
    json_dst = config.baseline_json_path(provider)
    md_dst = config.baseline_md_path(provider)

    if not json_src.exists():
        print(f"Error: No extraction found at {json_src}", file=sys.stderr)
        sys.exit(1)

    json_dst.parent.mkdir(parents=True, exist_ok=True)

    shutil.copy2(json_src, json_dst)
    print(f"Promoted {json_src.name} -> {json_dst.name}")

    if md_src.exists():
        shutil.copy2(md_src, md_dst)
        print(f"Snapshot  {md_src.name} -> {md_dst.name}")
    else:
        print(f"Warning: No markdown found at {md_src}, skipping snapshot")


def do_audit(provider: str) -> None:
    """Run the full pipeline: fetch, extract, diff, report."""
    do_fetch(provider)
    do_extract(provider)

    # Only diff if baseline exists
    baseline = config.baseline_json_path(provider)
    if baseline.exists():
        do_diff(provider)
        do_report(provider)
    else:
        print(f"\nNo baseline found for {provider}.")
        print(f"Run 'promote --provider {provider}' to establish the initial baseline.")
        print(f"Then run 'audit' again after the next spec update to see changes.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="API Parity Pipeline — detect spec changes for C++ protocol structs",
    )
    parser.add_argument(
        "command",
        choices=["fetch", "extract", "diff", "report", "checklist", "promote", "audit"],
        help="Pipeline stage to run",
    )
    parser.add_argument(
        "--provider",
        choices=list(config.ENDPOINTS.keys()),
        help="Provider to process",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Process all configured providers",
    )

    args = parser.parse_args()

    if args.all:
        providers = list(config.ENDPOINTS.keys())
    elif args.provider:
        providers = [args.provider]
    else:
        parser.error("Specify --provider or --all")
        return

    commands = {
        "fetch": do_fetch,
        "extract": do_extract,
        "diff": do_diff,
        "report": do_report,
        "checklist": do_checklist,
        "promote": do_promote,
        "audit": do_audit,
    }

    handler = commands[args.command]
    for provider in providers:
        print(f"\n{'='*60}")
        print(f"  {args.command.upper()} — {provider}")
        print(f"{'='*60}\n")
        try:
            handler(provider)
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            if len(providers) == 1:
                sys.exit(1)


if __name__ == "__main__":
    main()
