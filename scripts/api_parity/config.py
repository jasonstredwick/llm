"""Provider configuration for the API parity pipeline."""

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class UrlEntry:
    """A single URL to fetch for a provider endpoint."""
    name: str     # short name used for the downloaded file, e.g. "shared", "responses"
    url: str


@dataclass(frozen=True)
class EndpointConfig:
    """Configuration for a single API endpoint to track.

    *urls* is an ordered list: dependencies first, main spec last.
    The parser processes them in order, building a type registry from
    earlier entries that the main spec can reference.
    """
    provider: str
    endpoint: str
    urls: tuple[UrlEntry, ...]
    parser: str   # parser module to use: "openai_anthropic" or "discovery"
    raw_ext: str  # file extension for the raw downloaded spec: "md" or "json"


# Root of the library repo (two levels up from scripts/api_parity/)
REPO_ROOT = Path(__file__).resolve().parent.parent.parent

# Where approved baseline specs live (top-level, not docs)
SPECS_DIR = REPO_ROOT / "specs"

# Where working files go during pipeline runs
SCRATCH_DIR = REPO_ROOT / "scratch" / "api_parity"

# Endpoints to track
ENDPOINTS = {
    "openai": EndpointConfig(
        provider="openai",
        endpoint="responses",
        urls=(
            UrlEntry("shared", "https://developers.openai.com/api/reference/resources/$shared/index.md"),
            UrlEntry("responses", "https://developers.openai.com/api/reference/resources/responses/methods/create/index.md"),
        ),
        parser="openai_anthropic",
        raw_ext="md",
    ),
    "anthropic": EndpointConfig(
        provider="anthropic",
        endpoint="messages",
        urls=(
            UrlEntry("messages", "https://platform.claude.com/docs/en/api/messages/create.md"),
        ),
        parser="openai_anthropic",
        raw_ext="md",
    ),
    "gemini": EndpointConfig(
        provider="gemini",
        endpoint="generate_content",
        urls=(
            UrlEntry("generate_content", "https://generativelanguage.googleapis.com/$discovery/rest?version=v1beta"),
        ),
        parser="discovery",
        raw_ext="json",
    ),
}


def spec_dir(provider: str) -> Path:
    """Return the approved-spec directory for a provider (specs/<provider>)."""
    cfg = ENDPOINTS[provider]
    return SPECS_DIR / cfg.provider

def scratch_dir(provider: str) -> Path:
    """Return the scratch working directory for a provider."""
    cfg = ENDPOINTS[provider]
    d = SCRATCH_DIR / cfg.provider
    d.mkdir(parents=True, exist_ok=True)
    return d

def raw_spec_path(provider: str) -> Path:
    """Path to the raw downloaded main spec (the last URL entry)."""
    cfg = ENDPOINTS[provider]
    return raw_url_path(provider, cfg.urls[-1].name)


def raw_url_path(provider: str, name: str) -> Path:
    """Path to a specific raw downloaded file by URL entry name."""
    cfg = ENDPOINTS[provider]
    return scratch_dir(provider) / f"{name}.raw.{cfg.raw_ext}"

def extracted_json_path(provider: str) -> Path:
    """Path to the extracted intermediate JSON (scratch working copy)."""
    cfg = ENDPOINTS[provider]
    return scratch_dir(provider) / f"{cfg.endpoint}.json"

def baseline_json_path(provider: str) -> Path:
    """Path to the approved baseline extraction (specs/<provider>/<endpoint>.json)."""
    cfg = ENDPOINTS[provider]
    return spec_dir(provider) / f"{cfg.endpoint}.json"

def baseline_spec_path(provider: str) -> Path:
    """Path to the approved baseline spec snapshot (specs/<provider>/<endpoint>.raw.<ext>).

    The baseline is always the main spec (last URL entry).
    """
    cfg = ENDPOINTS[provider]
    return spec_dir(provider) / f"{cfg.endpoint}.raw.{cfg.raw_ext}"


def baseline_url_path(provider: str, name: str) -> Path:
    """Path to a specific approved baseline file by URL entry name."""
    cfg = ENDPOINTS[provider]
    return spec_dir(provider) / f"{name}.raw.{cfg.raw_ext}"



# ---------------------------------------------------------------------------
# Generated → in-tree path helpers
# ---------------------------------------------------------------------------

def generated_dir(provider: str) -> Path:
    """Return the directory where codegen places its output."""
    return SCRATCH_DIR / "generated" / provider


def deploy_mapping(provider: str) -> list[tuple[Path, Path]]:
    """Return (src, dst) pairs for deploying generated files into the source tree.

    Each codegen run produces four files:
        <endpoint>.hpp            → interface/protocols/<provider>/<endpoint>.hpp
        <endpoint>_strings.hpp    → interface/protocols/<provider>/<endpoint>_strings.hpp
        <endpoint>_serialize.cpp  → src/protocols/serialize/<provider>_<endpoint>.cpp
        <endpoint>_deserialize.cpp→ src/protocols/deserialize/<provider>_<endpoint>.cpp
    """
    cfg = ENDPOINTS[provider]
    ep = cfg.endpoint
    gen = generated_dir(provider)
    hdr = REPO_ROOT / "interface" / "protocols" / cfg.provider
    ser = REPO_ROOT / "src" / "protocols" / "serialize"
    des = REPO_ROOT / "src" / "protocols" / "deserialize"
    stem = f"{cfg.provider}_{ep}"

    return [
        (gen / f"{ep}.hpp",              hdr / f"{ep}.hpp"),
        (gen / f"{ep}_strings.hpp",      hdr / f"{ep}_strings.hpp"),
        (gen / f"{ep}_serialize.cpp",    ser / f"{stem}.cpp"),
        (gen / f"{ep}_deserialize.cpp",  des / f"{stem}.cpp"),
    ]


# Files that were superseded by the new codegen and should be removed.
STALE_FILES: dict[str, list[Path]] = {
    "openai": [
        REPO_ROOT / "interface" / "protocols" / "openai" / "messages.hpp",
        REPO_ROOT / "interface" / "protocols" / "openai" / "strings.hpp",
        REPO_ROOT / "interface" / "protocols" / "openai" / "responses_common.hpp",
        REPO_ROOT / "interface" / "protocols" / "openai" / "responses_enums.hpp",
        REPO_ROOT / "interface" / "protocols" / "openai" / "responses_req.hpp",
        REPO_ROOT / "interface" / "protocols" / "openai" / "responses_res.hpp",
    ],
    "gemini": [
        REPO_ROOT / "interface" / "protocols" / "gemini" / "messages.hpp",
        REPO_ROOT / "interface" / "protocols" / "gemini" / "strings.hpp",
        REPO_ROOT / "interface" / "protocols" / "gemini" / "generate_content.hpp",
    ],
    "anthropic": [
        REPO_ROOT / "interface" / "protocols" / "anthropic" / "messages.hpp",
        REPO_ROOT / "interface" / "protocols" / "anthropic" / "strings.hpp",
    ],
}
