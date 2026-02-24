"""
Stage 1: Fetch — Download raw specs from provider documentation URLs.

Each provider endpoint can have multiple URLs (dependencies first, main spec
last).  All are fetched and saved with name-based filenames.

Usage:
    from scripts.api_parity.fetch import fetch
    fetch("openai")
"""

import subprocess
from pathlib import Path

from . import config


def fetch(provider: str) -> Path:
    """
    Download all raw spec files for a provider.

    Returns the path to the main (last) downloaded file.
    """
    cfg = config.ENDPOINTS[provider]

    last_path: Path | None = None
    for entry in cfg.urls:
        output_path = config.raw_url_path(provider, entry.name)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        print(f"Fetching {cfg.provider}/{entry.name} from {entry.url}")
        result = subprocess.run(
            ["curl", "-fsSL", "-o", str(output_path), entry.url],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            raise RuntimeError(
                f"Failed to fetch {entry.url}: {result.stderr.strip()}"
            )

        size = output_path.stat().st_size
        print(f"  -> {output_path} ({size:,} bytes)")
        last_path = output_path

    assert last_path is not None
    return last_path
