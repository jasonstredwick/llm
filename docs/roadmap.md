# Roadmap

Current status of each component, and remaining work.

Last updated: 2026-02-17

## Status Legend

- **Done** — Implemented and tested.
- **Functional** — Implemented, may need refinement or additional tests.
- **Skeletal** — Structure exists, implementation is partial or placeholder.
- **Not started** — Identified as needed, no code yet.

## Component Status

### Protocol Layer

| Component | Status | Notes |
|-----------|--------|-------|
| Anthropic Messages structs | Done | Full request/response coverage |
| Anthropic serialization | Done | |
| Anthropic deserialization | Done | |
| Anthropic string conversions | Done | All Kinds + enums |
| Anthropic unit tests | Functional | `tests/unit/test_anthropic.cpp` |
| Gemini GenerateContent structs | Done | Full request/response coverage |
| Gemini serialization | Done | |
| Gemini deserialization | Done | |
| Gemini string conversions | Done | All enums (no Kind tags — Gemini uses field presence) |
| Gemini unit tests | Functional | `tests/unit/test_gemini.cpp` |
| OpenAI Responses structs | Done | Full request/response coverage across 4 sub-headers |
| OpenAI serialization | Done | |
| OpenAI deserialization | Done | |
| OpenAI string conversions | Done | All Kinds + enums |
| OpenAI unit tests | Functional | `tests/unit/test_openai.cpp` |

### Core Types

| Component | Status | Notes |
|-----------|--------|-------|
| `Required<T>` | Done | |
| `EncodedUrl` | Done | |
| `Name64`, `NameLen<N>` | Done | |
| `Int64`, `Int64Bounded`, `Int64Str` | Done | |
| `Timestamp`, `RFC3339Timestamp` | Done | |
| `ValueBox<T>` | Done | |
| `json::Value` / `Array` / `Object` | Done | |
| `AnnotatedException` | Done | With context chain and source_location |
| Concepts (`Kind_c`, `Required_c`, etc.) | Done | |

### Transport Layer

| Component | Status | Notes |
|-----------|--------|-------|
| `http::Request` | Done | Frozen request boundary |
| `http::RequestHeaders` | Done | With security validation |
| `http::ResponseHeaders` | Functional | With dropped header tracking |
| `curl::Interface` | Functional | Multi-handle wrapper |
| `curl::Attempt` | Functional | Callback-based lifecycle |
| `curl::HeaderList` | Done | RAII curl_slist wrapper |
| `curl::Response` | Functional | Availability-staged data |
| `curl::Interface::Global` | Functional | Process-wide init/cleanup |
| Unit tests (http) | Functional | `tests/unit/test_http.cpp` |
| Unit tests (curl) | Functional | `tests/unit/test_curl.cpp` (integration, needs mock server) |

### Client Layer

| Component | Status | Notes |
|-----------|--------|-------|
| `Client` (type-erased) | Skeletal | Vtable + SBO storage defined, placeholder types for Request/Response/Result |
| `ModelContract` enum | Done | Anthropic, Gemini, OpenAI |
| `Auth` variants | Done | `AuthAPIKey`, `AuthGoogleCredentials` |
| `Metadata` variants | Skeletal | Empty structs |
| `CallAsync` / `CallSync` | Skeletal | Declared, not implemented |

### Orchestration

| Component | Status | Notes |
|-----------|--------|-------|
| `Orchestrator` | Skeletal | Owns Interface + SlotPool, methods declared |
| `SlotPool` | Skeletal | Type defined, allocation logic not implemented |
| `AsyncTask` | Removed | Coroutine support removed; may revisit with executor/scheduler integration |

### Projections

| Component | Status | Notes |
|-----------|--------|-------|
| `proj::text` types | Done | `Options`, `Prompt`, `Image`, `Block` |
| `Generate<anthropic::Request>` | Functional | Text, images, thinking effort mapping |
| `Generate<gemini::Request>` | Functional | Text, images, thinking effort mapping |
| `Generate<openai::Request>` | Functional | Text, images, thinking effort mapping |
| `Call()` function | Skeletal | Signature only, empty body |

### Top-Level

| Component | Status | Notes |
|-----------|--------|-------|
| `llm.hpp` | Skeletal | Namespace declaration only |

### Build & Infrastructure

| Component | Status | Notes |
|-----------|--------|-------|
| CMakeLists.txt | Done | Multi-platform, Release-only |
| Dependency manager | Done | `scripts/manage_deps.py` with lockfile |
| Integration test harness | Functional | `tests/mock_server.py`, `tests/run_integration_tests.py` |

## Remaining Work (Prioritized)

This is an inferred priority list based on code analysis. Adjust as needed.

### High Priority

1. **Orchestrator implementation** — Complete the request lifecycle: creation, polling loop,
   finalization, cleanup. The SlotPool needs its allocation/free logic.

2. **Client implementation** — Wire the type-erased Client to actual provider-specific
   implementations. Replace placeholder Request/Response/Result types with real ones.
   Connect Client → Protocol Layer (serialize) → Transport → Protocol Layer (deserialize).

3. **End-to-end request flow** — Get a single synchronous call working: construct request,
   serialize, transport, deserialize response, return to caller.

4. **`llm.hpp` top-level API** — Define the user-facing entry points.

### Medium Priority

5. **Retry and backpressure policy** — The design docs describe retry, jitter, and network
   smoothing policies but none are implemented yet.

6. **Streaming support** — The architecture identifies streaming as a call semantic but no
   streaming infrastructure exists yet.

7. **Coroutine / async execution** — Coroutine support (CoroResult, CallCoro) was removed. C++ coroutines require an executor/scheduler to control thread affinity, which the library does not provide. Revisit if/when `std::execution` (P2300) matures or if user-supplied executor integration is needed. For Python asyncio integration, `AsyncResult` provides the necessary building blocks (IsReady, SyncBlock, Take).

8. **Projection completeness** — `proj::text::Call()` is empty. Tools, multi-turn,
   document content, and streaming projections are not started.

9. **Error response handling** — Provider-specific error responses (4xx/5xx with JSON error
   bodies) need deserialization and structured error propagation.

### Lower Priority

10. **API parity audits** — The Python pipeline (`scripts/api_parity/`) is implemented and
    tested for OpenAI and Anthropic. Baselines are established for both providers. Gemini
    extractor is deferred (different doc format). See `docs/api_parity_pipeline.md` for the
    full design including the agent step for propagating changes to C++ code.

11. **Additional providers** — The architecture supports arbitrary providers, but only three
    are implemented.

12. **Documentation** — API-level doc comments in headers. Usage examples.

## Files Potentially Superseded

The following existing docs may be candidates for removal now that consolidated docs exist.
These are noted for the owner to evaluate — no files have been deleted.

| File | Superseded by |
|------|---------------|
| `docs/llm_arch_overview.md` | `docs/architecture.md` |
| `docs/http_manager_design.md` | `docs/architecture.md` + `docs/transport.md` |
| `docs/curl_manager_design.md` | `docs/architecture.md` + `docs/transport.md` |
| `docs/frozen_http_requests.md` | `docs/architecture.md` (frozen request boundary section) |

Note: `http_manager_design.md` describes a `HttpRequestManager` with PIMPL, background thread,
and `std::function` callbacks — this design has been superseded by the current `curl::Interface`
+ `Orchestrator` approach. The design intent is preserved in the new docs but the specific
implementation plan in that file no longer matches the codebase.
