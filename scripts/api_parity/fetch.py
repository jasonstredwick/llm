"""
Stage 1: Fetch — Download raw markdown from provider documentation URLs.

Usage:
    from scripts.api_parity.fetch import fetch
    fetch("openai")
"""

import subprocess
from pathlib import Path

from . import config


def fetch(provider: str) -> Path:
    """
    Download the raw markdown spec for a provider.

    Returns the path to the downloaded file.
    """
    cfg = config.ENDPOINTS[provider]
    output_path = config.raw_md_path(provider)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    print(f"Fetching {cfg.provider}/{cfg.endpoint} from {cfg.url}")
    result = subprocess.run(
        ["curl", "-fsSL", "-o", str(output_path), cfg.url],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Failed to fetch {cfg.url}: {result.stderr.strip()}"
        )

    size = output_path.stat().st_size
    print(f"  -> {output_path} ({size:,} bytes)")
    return output_path
