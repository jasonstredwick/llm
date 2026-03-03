#!/usr/bin/env python3
"""
CLI entry point for the API parity pipeline.

Usage:
    python -m scripts.api_parity.run fetch     --provider openai
    python -m scripts.api_parity.run extract   --provider openai
    python -m scripts.api_parity.run diff      --provider openai
    python -m scripts.api_parity.run promote   --provider openai
    python -m scripts.api_parity.run generate  --provider openai   # codegen C++ from extraction
    python -m scripts.api_parity.run deploy    --provider openai   # copy generated files into source tree
    python -m scripts.api_parity.run build                         # cmake configure + build
    python -m scripts.api_parity.run audit     --provider openai   # full pipeline
    python -m scripts.api_parity.run audit     --all               # all providers
    python -m scripts.api_parity.run audit     --all --no-clean    # keep scratch after success

Or from the repo root:
    python scripts/api_parity/run.py fetch --provider openai
"""

import argparse
import shutil
import subprocess
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
from scripts.api_parity.codegen import generate as do_generate_tree
from scripts.api_parity.codegen_flat import generate as do_generate_flat


def do_generate(provider: str) -> None:
    """Generate C++ artifacts, selecting tree or flat codegen based on parser type."""
    cfg = config.ENDPOINTS[provider]
    if cfg.parser == "discovery":
        do_generate_flat(provider)
    else:
        do_generate_tree(provider)


def do_promote(provider: str) -> None:
    """Copy the current extraction and markdown from scratch to specs/ as baseline."""
    json_src = config.extracted_json_path(provider)
    md_src = config.raw_spec_path(provider)
    json_dst = config.baseline_json_path(provider)
    md_dst = config.baseline_spec_path(provider)

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


def do_deploy(provider: str) -> None:
    """Copy generated files into the source tree, removing stale predecessors."""
    mapping = config.deploy_mapping(provider)

    # Verify all source files exist before touching anything
    missing = [src for src, _ in mapping if not src.exists()]
    if missing:
        print(f"Error: generated files not found (run 'generate' first):",
              file=sys.stderr)
        for m in missing:
            print(f"  {m}", file=sys.stderr)
        sys.exit(1)

    # Remove stale files first
    stale = config.STALE_FILES.get(provider, [])
    for sf in stale:
        if sf.exists():
            try:
                sf.unlink()
                print(f"  removed stale  {sf.relative_to(config.REPO_ROOT)}")
            except OSError as e:
                print(f"  WARNING: could not remove stale file "
                      f"{sf.relative_to(config.REPO_ROOT)}: {e}")

    # Copy generated → in-tree
    for src, dst in mapping:
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
        print(f"  deployed  {src.name:25s} → {dst.relative_to(config.REPO_ROOT)}")

    print(f"\nDeployed {len(mapping)} files for {provider}.")


def do_build() -> None:
    """Configure and build the project with cmake to verify generated code compiles."""
    build_dir = config.SCRATCH_DIR / "build"
    build_dir.mkdir(parents=True, exist_ok=True)

    print(f"Configuring cmake in {build_dir.relative_to(config.REPO_ROOT)}...")
    result = subprocess.run(
        ["cmake", str(config.REPO_ROOT), "-DCMAKE_BUILD_TYPE=Release"],
        cwd=build_dir,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print("cmake configure FAILED:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        raise RuntimeError("cmake configure failed")

    print("Building...")
    result = subprocess.run(
        ["cmake", "--build", str(build_dir), "-j"],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        print("Build FAILED:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        raise RuntimeError("Build failed — baseline NOT promoted")

    print("Build succeeded.")


def do_clean() -> None:
    """Remove the scratch directory."""
    if config.SCRATCH_DIR.exists():
        shutil.rmtree(config.SCRATCH_DIR)
        print(f"Cleaned {config.SCRATCH_DIR.relative_to(config.REPO_ROOT)}/")


def do_audit_provider(provider: str) -> bool:
    """Run per-provider stages: fetch → extract → diff → generate → deploy.

    Returns True if changes were detected and deployed, False otherwise.
    """
    do_fetch(provider)
    do_extract(provider)

    changed = do_diff(provider)
    if not changed:
        print(f"\nNo changes for {provider}, skipping generate/deploy.")
        return False

    do_generate(provider)
    do_deploy(provider)
    return True


def do_audit(providers: list[str], *, no_clean: bool = False) -> None:
    """Run the full audit pipeline across one or more providers.

    Per-provider: fetch → extract → diff → generate → deploy
    Then once:    build → promote all → clean
    """
    changed_providers: list[str] = []
    failed_providers: list[tuple[str, Exception]] = []

    for provider in providers:
        print(f"\n{'='*60}")
        print(f"  AUDIT — {provider}")
        print(f"{'='*60}\n")
        try:
            if do_audit_provider(provider):
                changed_providers.append(provider)
        except Exception as e:
            print(f"Error processing {provider}: {e}", file=sys.stderr)
            failed_providers.append((provider, e))

    if failed_providers:
        print(f"\n{'='*60}", file=sys.stderr)
        print(f"  FAILED providers:", file=sys.stderr)
        for name, err in failed_providers:
            print(f"    {name}: {err}", file=sys.stderr)
        print(f"{'='*60}", file=sys.stderr)
        print("\nAborting — will not build or promote with errors.", file=sys.stderr)
        sys.exit(1)

    if not changed_providers:
        print("\nNo providers had changes. Nothing to build or promote.")
        if not no_clean:
            do_clean()
        return

    # Build once after all providers are deployed
    print(f"\n{'='*60}")
    print(f"  BUILD")
    print(f"{'='*60}\n")
    do_build()

    # Promote all changed providers after successful build
    for provider in changed_providers:
        print(f"\n{'='*60}")
        print(f"  PROMOTE — {provider}")
        print(f"{'='*60}\n")
        do_promote(provider)

    # Clean scratch on success (unless --no-clean)
    if not no_clean:
        print(f"\n{'='*60}")
        print(f"  CLEAN")
        print(f"{'='*60}\n")
        do_clean()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="API Parity Pipeline — detect spec changes for C++ protocol structs",
    )
    parser.add_argument(
        "command",
        choices=["fetch", "extract", "diff",
                 "promote", "audit", "generate", "deploy", "build", "clean"],
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
    parser.add_argument(
        "--no-clean",
        action="store_true",
        help="Keep scratch directory after successful audit (default: clean on success)",
    )

    args = parser.parse_args()

    # build and clean don't need a provider
    if args.command in ("build", "clean"):
        print(f"\n{'='*60}")
        print(f"  {args.command.upper()}")
        print(f"{'='*60}\n")
        try:
            if args.command == "build":
                do_build()
            else:
                do_clean()
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)
        return

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
        "promote": do_promote,
        "generate": do_generate,
        "deploy": do_deploy,
    }

    if args.command == "audit":
        do_audit(providers, no_clean=args.no_clean)
    else:
        for provider in providers:
            print(f"\n{'='*60}")
            print(f"  {args.command.upper()} — {provider}")
            print(f"{'='*60}\n")
            try:
                commands[args.command](provider)
            except Exception as e:
                print(f"Error: {e}", file=sys.stderr)
                if len(providers) == 1:
                    sys.exit(1)


if __name__ == "__main__":
    main()
