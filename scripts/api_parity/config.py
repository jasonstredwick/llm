"""Provider configuration for the API parity pipeline."""

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class EndpointConfig:
    """Configuration for a single API endpoint to track."""
    provider: str
    endpoint: str
    url: str
    parser: str  # parser module to use: "openai_anthropic" or "gemini"


# Root of the library repo (two levels up from scripts/api_parity/)
REPO_ROOT = Path(__file__).resolve().parent.parent.parent

# Where raw markdown specs are stored (Stage 1 output)
SPECS_DIR = REPO_ROOT / "docs" / "specs" / "protocols"

# Where working files go during pipeline runs
SCRATCH_DIR = REPO_ROOT / "scratch" / "api_parity"

# Endpoints to track
ENDPOINTS = {
    "openai": EndpointConfig(
        provider="openai",
        endpoint="responses",
        url="https://developers.openai.com/api/reference/resources/responses/methods/create/index.md",
        parser="openai_anthropic",
    ),
    "anthropic": EndpointConfig(
        provider="anthropic",
        endpoint="messages",
        url="https://platform.claude.com/docs/en/api/messages/create.md",
        parser="openai_anthropic",
    ),
    "gemini": EndpointConfig(
        provider="gemini",
        endpoint="generate_content",
        url="https://ai.google.dev/api/generate-content.md.txt",
        parser="gemini",
    ),
}


def spec_dir(provider: str) -> Path:
    """Return the approved-spec directory for a provider (docs/specs/protocols/<provider>)."""
    cfg = ENDPOINTS[provider]
    return SPECS_DIR / cfg.provider

def scratch_dir(provider: str) -> Path:
    """Return the scratch working directory for a provider."""
    cfg = ENDPOINTS[provider]
    d = SCRATCH_DIR / cfg.provider
    d.mkdir(parents=True, exist_ok=True)
    return d

def raw_md_path(provider: str) -> Path:
    """Path to the raw downloaded markdown spec (scratch working copy)."""
    cfg = ENDPOINTS[provider]
    return scratch_dir(provider) / f"{cfg.endpoint}.md"

def extracted_json_path(provider: str) -> Path:
    """Path to the extracted intermediate JSON (scratch working copy)."""
    cfg = ENDPOINTS[provider]
    return scratch_dir(provider) / f"{cfg.endpoint}.json"

def baseline_json_path(provider: str) -> Path:
    """Path to the approved baseline extraction (docs/specs)."""
    cfg = ENDPOINTS[provider]
    return spec_dir(provider) / f"{cfg.endpoint}.baseline.json"

def baseline_md_path(provider: str) -> Path:
    """Path to the approved baseline markdown snapshot (docs/specs)."""
    cfg = ENDPOINTS[provider]
    return spec_dir(provider) / f"{cfg.endpoint}.baseline.md"

def diff_json_path(provider: str) -> Path:
    """Path to the diff output JSON (scratch)."""
    cfg = ENDPOINTS[provider]
    d = SCRATCH_DIR / "diffs"
    d.mkdir(parents=True, exist_ok=True)
    return d / f"{cfg.provider}_{cfg.endpoint}_diff.json"

def report_md_path(provider: str) -> Path:
    """Path to the generated report (scratch)."""
    cfg = ENDPOINTS[provider]
    d = SCRATCH_DIR / "reports"
    d.mkdir(parents=True, exist_ok=True)
    return d / f"{cfg.provider}_{cfg.endpoint}_report.md"

def checklist_md_path(provider: str) -> Path:
    """Path to the generated integration checklist (scratch)."""
    cfg = ENDPOINTS[provider]
    d = SCRATCH_DIR / "checklists"
    d.mkdir(parents=True, exist_ok=True)
    return d / f"{cfg.provider}_{cfg.endpoint}_checklist.md"
