"""
Stage 2: Extract — Parse raw markdown into structured intermediate JSON.

Usage:
    from scripts.api_parity.extract import extract
    extract("openai")
"""

import json
from pathlib import Path

from . import config


def extract(provider: str) -> Path:
    """
    Parse the raw markdown spec for a provider into intermediate JSON.

    Returns the path to the extracted JSON file.
    """
    cfg = config.ENDPOINTS[provider]
    md_path = config.raw_md_path(provider)
    json_path = config.extracted_json_path(provider)

    if not md_path.exists():
        raise FileNotFoundError(
            f"Raw spec not found: {md_path}\n"
            f"Run 'fetch --provider {provider}' first."
        )

    print(f"Extracting {cfg.provider}/{cfg.endpoint} from {md_path}")
    md_text = md_path.read_text(encoding="utf-8")

    # Select parser
    if cfg.parser == "openai_anthropic":
        from .parsers.openai_anthropic import extract as parse_extract
    elif cfg.parser == "gemini":
        from .parsers.gemini import extract as parse_extract
    else:
        raise ValueError(f"Unknown parser: {cfg.parser}")

    result = parse_extract(
        markdown_text=md_text,
        provider=cfg.provider,
        endpoint=cfg.endpoint,
        source_url=cfg.url,
        raw_file=str(md_path.relative_to(config.REPO_ROOT)),
    )

    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(
        json.dumps(result, indent=2, ensure_ascii=False) + '\n',
        encoding="utf-8",
    )

    # Summary stats
    req_objs = len(result.get("request", {}).get("objects", {}))
    req_enums = len(result.get("request", {}).get("enums", {}))
    resp_objs = len(result.get("response", {}).get("objects", {}))
    resp_enums = len(result.get("response", {}).get("enums", {}))
    print(f"  -> {json_path}")
    print(f"     Request:  {req_objs} objects, {req_enums} enums")
    print(f"     Response: {resp_objs} objects, {resp_enums} enums")

    return json_path
