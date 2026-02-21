"""
Stage 2: Extract — Parse raw markdown into structured intermediate JSON.

Usage:
    from scripts.api_parity.extract import extract
    extract("openai")
"""

import json
from pathlib import Path

from . import config


def _extract_discovery(provider: str, cfg: config.EndpointConfig) -> Path:
    """Extract from a Google Discovery Document JSON."""
    from .parsers.gemini import extract as discovery_extract

    # The fetch stage saves the discovery JSON via raw_spec_path()
    discovery_path = config.raw_spec_path(provider)
    json_path = config.extracted_json_path(provider)

    if not discovery_path.exists():
        raise FileNotFoundError(
            f"Discovery document not found: {discovery_path}\n"
            f"Run 'fetch --provider {provider}' first."
        )

    print(f"Extracting {cfg.provider}/{cfg.endpoint} from {discovery_path}")

    result = discovery_extract(
        discovery_path=str(discovery_path),
        provider=cfg.provider,
        endpoint=cfg.endpoint,
        source_url=cfg.url,
    )

    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(
        json.dumps(result, indent=2, ensure_ascii=False) + '\n',
        encoding="utf-8",
    )

    # Summary stats
    schemas = result.get("schemas", {})
    total_fields = sum(len(s.get("fields", [])) for s in schemas.values())
    total_enums = sum(
        1 for s in schemas.values()
        for f in s.get("fields", []) if f.get("type") == "enum"
    )
    print(f"  -> {json_path}")
    print(f"     Schemas: {len(schemas)}, Fields: {total_fields}, Enum fields: {total_enums}")

    return json_path


def extract(provider: str) -> Path:
    """
    Parse the raw markdown spec for a provider into intermediate JSON.

    Returns the path to the extracted JSON file.
    """
    cfg = config.ENDPOINTS[provider]
    md_path = config.raw_spec_path(provider)
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
    elif cfg.parser == "discovery":
        return _extract_discovery(provider, cfg)
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
    print(f"  -> {json_path}")
    for section in ("request", "response"):
        root = result.get(section, {}).get("root")
        if root:
            if cfg.parser == "openai_anthropic":
                from .parsers.openai_anthropic import _count_tree
                structs, fields, enums = _count_tree(root)
                print(f"     {section.capitalize()}: {structs} structs, "
                      f"{fields} fields, {enums} enums")
            else:
                print(f"     {section.capitalize()}: (tree format)")

    return json_path
