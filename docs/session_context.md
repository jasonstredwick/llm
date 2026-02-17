# Session Context

Start-of-session reference for AI assistants working on this project. Read this file first, then
follow pointers to other docs as needed.

## Project Identity

**Name:** `jai::llm`
**Repository root:** The folder containing this `docs/` directory.
**Language:** C++26 (clang on macOS), C++23 fallback (MSVC on Windows). C11 for any C code.
**Library type:** Static library (`.a` / `.lib`).
**Build system:** CMake 3.25+, single-config Release-only.
**Author:** Jason Stredwick (jason.stredwick@gmail.com)

## What This Library Does

Provides a C++ interface for calling LLM provider APIs over HTTP. The library handles JSON
serialization/deserialization of provider-specific request and response structures, HTTP request
construction, and transport via libcurl. It is designed for high-concurrency workloads with
explicit, policy-driven control over transport behavior.

The library exposes two API surfaces: explicit provider APIs that map directly to each provider's
native capabilities, and an optional normalized "projection" API for common cross-provider usage.

## Current Provider Support

| Provider   | API Endpoint              | Namespace               | Status               |
|------------|---------------------------|-------------------------|----------------------|
| Anthropic  | `/v1/messages`            | `jai::llm::anthropic`   | Structs + ser/de     |
| Gemini     | `generateContent`         | `jai::llm::gemini`      | Structs + ser/de     |
| OpenAI     | Responses API             | `jai::llm::openai`      | Structs + ser/de     |

## Project Status (High Level)

The protocol layer (data structures, serialization, deserialization) is substantially complete for
all three providers. The transport layer (curl wrapper, HTTP request/response, attempt lifecycle) is
implemented and functional. The upper layers — Client, Orchestrator, async execution, projections —
are skeletal or in early development.

See `docs/roadmap.md` for detailed status.

## Key Documents

| Document                     | Purpose                                                    |
|------------------------------|-------------------------------------------------------------|
| `docs/architecture.md`       | Layer model, component responsibilities, data flow          |
| `docs/coding_conventions.md` | Type patterns, naming rules, file organization              |
| `docs/serialization.md`      | simdjson-based ser/de framework                             |
| `docs/protocols.md`          | Per-provider struct coverage and file map                   |
| `docs/transport.md`          | curl/http layer: Interface, Attempt, Response lifecycle     |
| `docs/roadmap.md`            | What's done, what's remaining, prioritized task list        |
| `docs/decisions.md`          | Running log of design decisions with rationale              |
| `docs/specs/api-parity.md`   | Workflow for auditing protocol structs against live APIs     |

## Directory Layout

```
interface/              Public headers (the library's API surface)
  clients/              Client, policy, provider-specific client headers
  core/                 Foundational types, error, async, string conversions
  projections/          Normalized cross-provider APIs (e.g. text projection)
  protocols/            Provider-specific request/response data structures
    anthropic/          messages.hpp, strings.hpp
    gemini/             generate_content.hpp, strings.hpp
    openai/             responses.hpp, responses_*.hpp, strings.hpp

src/                    Private implementation (not part of public API)
  protocols/
    serialize/          Per-provider JSON serialization (.cpp files)
    deserialize/        Per-provider JSON deserialization (.cpp files)
  curl.hpp/cpp          libcurl wrapper (Interface, Attempt, HeaderList)
  http.hpp/cpp          HTTP request/response types, header processing
  orchestrator.hpp/cpp  Request orchestration (in progress)
  memory.hpp            SlotPool allocator

tests/
  unit/                 Per-component unit tests
  mock_server.py        Python mock server for integration tests
  run_integration_tests.py

docs/                   Project documentation
scratch/                Working files, drafts, transient artifacts (not in git)
deps/                   Third-party dependencies (managed by scripts/manage_deps.py)
scripts/                Build and dependency tooling
```

## Dependencies

All statically linked. Managed via `deps.lock.json` and `scripts/manage_deps.py`.

| Library    | Purpose                        |
|------------|--------------------------------|
| libcurl    | HTTP transport                 |
| nghttp2    | HTTP/2 support for libcurl     |
| zlib       | Compression                    |
| simdjson   | JSON parsing and serialization |
| c-ares     | Async DNS resolution           |

## Conventions Quick Reference

These are described fully in `docs/coding_conventions.md`. The short version:

- **Required fields** use `Required<T>`. **Optional fields** use `std::optional<T>` initialized with `{}`.
- **No naked types** in protocol structs. Every member must be wrapped.
- **JSON discriminator fields** use `Kind` tag structs with a static `value` member.
- **Enum conversions** use `to_string_view()` / `from_string_view<T>()` free functions.
- **Validated domain types**: `EncodedUrl`, `Name64`, `NameLen<N>`, `Int64Bounded<Lo, Hi>`, `Int64Str`.
- **Recursive types** use `ValueBox<T>` (heap-allocated value semantics).
- Namespaces: `jai::llm` (core), `jai::llm::anthropic`, `jai::llm::gemini`, `jai::llm::openai`,
  `jai::llm::projection::text`, `jai::llm::curl`, `jai::llm::http`.

## Working With This Project

- **scratch/** is for transient working files and is not checked into git.
- **docs/** is the source of truth for design decisions and project state.
- The owner (Jason) manages git directly. Do not run git commands or delete tracked files.
- When design decisions are made during a session, record them in `docs/decisions.md`.
- When tasks are completed or status changes, update `docs/roadmap.md`.
