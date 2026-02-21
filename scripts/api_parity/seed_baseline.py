#!/usr/bin/env python3
"""Seed a baseline from existing C++ implementation.

Reads the full extracted JSON for a provider and trims it to only include
objects/enums/fields that exist in the C++ header.  The result is saved as
the baseline JSON so that subsequent `audit` runs report the unimplemented
additions as the delta.

Usage:
    python scripts/api_parity/seed_baseline.py --provider anthropic
    python scripts/api_parity/seed_baseline.py --provider anthropic --dry-run
"""

from __future__ import annotations

import argparse
import copy
import json
import re
import sys
from pathlib import Path
from typing import Any

# ── Resolve repo root and add to path ────────────────────────────────────────
_REPO_ROOT = Path(__file__).resolve().parent.parent.parent
if str(_REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(_REPO_ROOT))

from scripts.api_parity import config


# ═══════════════════════════════════════════════════════════════════════════════
# C++ HEADER PARSING
# ═══════════════════════════════════════════════════════════════════════════════

def parse_cpp_enums(text: str) -> dict[str, list[str]]:
    """Extract enum class names and their values from C++ text."""
    enums: dict[str, list[str]] = {}
    for m in re.finditer(r'enum\s+class\s+(\w+)\s*\{([^}]+)\}', text, re.DOTALL):
        name = m.group(1)
        vals = [v.strip() for v in m.group(2).split(',') if v.strip()]
        enums[name] = vals
    return enums


def parse_cpp_structs(text: str) -> dict[str, dict[str, dict[str, Any]]]:
    """Extract struct names and their fields from C++ text.

    Returns {qualified_name: {field_name: {type, required}}}.
    Uses brace-depth tracking to correctly scope fields to their
    containing struct.
    """
    structs: dict[str, dict[str, dict[str, Any]]] = {}
    lines = text.split('\n')
    name_stack: list[str] = []
    brace_stack: list[int] = []
    struct_entries: list[tuple[str, int]] = []  # (qualified_name, start_line)

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        if stripped.startswith('//') or stripped.startswith('/***') or stripped.startswith('*'):
            i += 1
            continue

        # Detect struct opening
        sm = re.match(r'\s*struct\s+(\w+)\s*(?::\s*\w+\s*)?\{', line)
        if not sm:
            sm = re.match(r'\s*struct\s+(\w+)\s*(\{)?', line)
            if sm and sm.group(2) is None and not re.match(r'\s*struct\s+\w+\s*;', line):
                pass  # struct without brace on this line
            elif sm and sm.group(2):
                pass
            else:
                sm = None

        if sm and 'Kind' not in sm.group(1) and not stripped.startswith('//'):
            sname = sm.group(1)
            qualified = '::'.join(name_stack + [sname]) if name_stack else sname
            name_stack.append(sname)
            has_brace = '{' in line

            if has_brace:
                depth = line.count('{') - line.count('}')
                if depth <= 0 and '};' in line:
                    structs[qualified] = {}
                    name_stack.pop()
                    i += 1
                    continue
                else:
                    brace_stack.append(depth)
                    struct_entries.append((qualified, i))
            else:
                brace_stack.append(0)
                struct_entries.append((qualified, i))

            i += 1
            continue

        # Track brace depth
        if brace_stack:
            brace_stack[-1] += line.count('{') - line.count('}')

            if brace_stack[-1] <= 0:
                qualified, start = struct_entries.pop()
                brace_stack.pop()
                name_stack.pop()

                # Parse fields belonging to this struct (depth == 1)
                fields: dict[str, dict[str, Any]] = {}
                for j in range(start, i + 1):
                    fline = lines[j]
                    if re.match(r'\s*(struct|using|enum)', fline.strip()):
                        continue
                    field_m = re.match(
                        r'\s+(?:Required<(.+?)>|std::optional<(.+?)>)\s+(\w+)',
                        fline
                    )
                    if field_m:
                        depth_at = 0
                        for k in range(start, j + 1):
                            depth_at += lines[k].count('{') - lines[k].count('}')
                        if depth_at == 1:
                            type_str = field_m.group(1) or field_m.group(2)
                            field_name = field_m.group(3)
                            required = field_m.group(1) is not None
                            fields[field_name] = {
                                'type': type_str,
                                'required': required,
                            }

                structs[qualified] = fields

        i += 1

    return structs


# ═══════════════════════════════════════════════════════════════════════════════
# PROVIDER-SPECIFIC CONFIGURATION
# ═══════════════════════════════════════════════════════════════════════════════

# Name mapping: JSON extraction name → C++ struct name
# Only entries where names differ need to be listed.
NAME_MAPS: dict[str, dict[str, str]] = {
    "anthropic": {
        # JSON name → C++ qualified name
        'JSONOutputFormat': 'OutputConfig::Format',
        'Message': 'Response',
        'Tool_input_schema': 'Tool::InputSchema',
        'WebSearchTool20250305_user_location': 'WebSearchTool20250305::UserLocation',
        'WebSearchToolRequestError': 'WebSearchToolResultBlockParam::WebSearchToolRequestError',
        'WebSearchToolResultError': 'WebSearchToolResultBlock::WebSearchToolResultError',
    },
    "openai": {
        # TODO: populate when tackling OpenAI seeded baseline
    },
    "gemini": {
        # TODO: populate when tackling Gemini seeded baseline
        'GenerateContentResponse': 'Response',
        'TopCandidates::Candidate': 'LogprobsResult::Candidate',
        'TopCandidates': 'LogprobsResult::TopCandidate',
        'Web': 'GroundingChunk::Web',
        'RetrievedContext': 'GroundingChunk::RetrievedContext',
        'Maps': 'GroundingChunk::Maps',
        'PlaceAnswerSources': 'GroundingChunk::Maps::PlaceAnswerSources',
        'ReviewSnippet': 'GroundingChunk::Maps::PlaceAnswerSources::ReviewSnippets',
        'Segment': 'GroundingSupport::Segment',
        'RetrievalMetadata': 'GroundingMetadata::RetrievalMetadata',
        'UrlMetadata': 'UrlContextMetadata::UrlMetadata',
        'CitationSource': 'CitationMetadata::CitationSource',
        'VoiceConfig': 'SpeechConfig::VoiceConfig',
        'PrebuiltVoiceConfig': 'SpeechConfig::PrebuiltVoiceConfig',
        'SpeakerVoiceConfig': 'SpeechConfig::SpeakerVoiceConfig',
        'MultiSpeakerVoiceConfig': 'SpeechConfig::MultiSpeakerVoiceConfig',
        'ModalityTokenCount': 'UsageMetadata::ModalityTokenCount',
    },
}

# C++ header paths relative to repo root
HEADER_PATHS: dict[str, str] = {
    "anthropic": "interface/protocols/anthropic/messages.hpp",
    "openai": "interface/protocols/openai/responses.hpp",
    "gemini": "interface/protocols/gemini/generate_content.hpp",
}


# ═══════════════════════════════════════════════════════════════════════════════
# SEEDING LOGIC
# ═══════════════════════════════════════════════════════════════════════════════

def build_reverse_name_map(name_map: dict[str, str]) -> dict[str, str]:
    """Build cpp_name → json_name reverse lookup."""
    return {v: k for k, v in name_map.items()}


def seed_baseline(
    provider: str,
    dry_run: bool = False,
) -> dict[str, Any]:
    """Build a seeded baseline for a provider.

    Returns a summary dict with counts.
    """
    # Load full extraction
    json_path = config.extracted_json_path(provider)
    if not json_path.exists():
        print(f"ERROR: No extracted JSON at {json_path}")
        print(f"Run: python scripts/api_parity/run.py extract --provider {provider}")
        sys.exit(1)

    with open(json_path) as f:
        full_json = json.load(f)

    # Load C++ header
    header_path = _REPO_ROOT / HEADER_PATHS[provider]
    if not header_path.exists():
        print(f"ERROR: No C++ header at {header_path}")
        sys.exit(1)

    with open(header_path) as f:
        cpp_text = f.read()

    # Parse C++
    cpp_structs = parse_cpp_structs(cpp_text)
    cpp_enums = parse_cpp_enums(cpp_text)

    name_map = NAME_MAPS.get(provider, {})

    # Build the seeded baseline — start from the full extraction and trim
    seeded = copy.deepcopy(full_json)

    summary = {
        'kept_objects': 0,
        'dropped_objects': 0,
        'kept_enums': 0,
        'dropped_enums': 0,
        'dropped_fields': 0,
        'kept_fields': 0,
        'details': [],
    }

    for section in ['request', 'response']:
        section_data = seeded.get(section, {})

        # ── Filter objects ────────────────────────────────────────────────
        objects = section_data.get('objects', {})
        to_remove_objs = []
        for json_name, obj_def in objects.items():
            cpp_name = name_map.get(json_name, json_name)
            if cpp_name not in cpp_structs:
                to_remove_objs.append(json_name)
                summary['dropped_objects'] += 1
                summary['details'].append(f"  DROP {section}.objects.{json_name} (no C++ match)")
            else:
                summary['kept_objects'] += 1
                # Filter fields to match C++
                cpp_fields = cpp_structs[cpp_name]
                json_fields = obj_def.get('fields', {})
                to_remove_fields = []
                for field_name in json_fields:
                    if field_name not in cpp_fields:
                        to_remove_fields.append(field_name)
                        summary['dropped_fields'] += 1
                        summary['details'].append(
                            f"  DROP {section}.objects.{json_name}.{field_name} (no C++ field)"
                        )
                    else:
                        summary['kept_fields'] += 1
                for f in to_remove_fields:
                    del json_fields[f]

        for name in to_remove_objs:
            del objects[name]

        # ── Filter enums ──────────────────────────────────────────────────
        enums = section_data.get('enums', {})
        to_remove_enums = []
        for enum_name in enums:
            # Extraction uses '::' as ownership separator;
            # C++ uses '_' (e.g., Request::ServiceTier → Request_ServiceTier)
            cpp_name = enum_name.replace('::', '_')
            matched = (enum_name in cpp_enums or cpp_name in cpp_enums)
            if not matched:
                to_remove_enums.append(enum_name)
                summary['dropped_enums'] += 1
                summary['details'].append(f"  DROP {section}.enums.{enum_name} (no C++ match)")
            else:
                summary['kept_enums'] += 1

        for name in to_remove_enums:
            del enums[name]

    # ── Write baseline ────────────────────────────────────────────────────────
    baseline_path = config.baseline_json_path(provider)
    if dry_run:
        print(f"\n[DRY RUN] Would write seeded baseline to: {baseline_path}")
    else:
        with open(baseline_path, 'w') as f:
            json.dump(seeded, f, indent=2)
            f.write('\n')
        print(f"\nWrote seeded baseline to: {baseline_path}")

    return summary


def print_summary(summary: dict[str, Any], verbose: bool = False) -> None:
    """Print the seed summary."""
    print(f"\n{'─' * 60}")
    print("SEED BASELINE SUMMARY")
    print(f"{'─' * 60}")
    print(f"  Objects kept:   {summary['kept_objects']}")
    print(f"  Objects dropped:{summary['dropped_objects']}")
    print(f"  Fields kept:    {summary['kept_fields']}")
    print(f"  Fields dropped: {summary['dropped_fields']}")
    print(f"  Enums kept:     {summary['kept_enums']}")
    print(f"  Enums dropped:  {summary['dropped_enums']}")

    if verbose and summary['details']:
        print(f"\n{'─' * 60}")
        print("DETAILS")
        print(f"{'─' * 60}")
        for line in summary['details']:
            print(line)


# ═══════════════════════════════════════════════════════════════════════════════
# CLI
# ═══════════════════════════════════════════════════════════════════════════════

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Seed a baseline from C++ implementation"
    )
    parser.add_argument(
        '--provider', required=True,
        choices=list(config.ENDPOINTS.keys()),
        help='Provider to seed baseline for',
    )
    parser.add_argument(
        '--dry-run', action='store_true',
        help='Show what would be done without writing',
    )
    parser.add_argument(
        '--verbose', '-v', action='store_true',
        help='Show detailed drop list',
    )
    args = parser.parse_args()

    summary = seed_baseline(args.provider, dry_run=args.dry_run)
    print_summary(summary, verbose=args.verbose)


if __name__ == '__main__':
    main()
