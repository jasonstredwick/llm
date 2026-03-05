# Architecture

This document describes the layered architecture of the `jai::llm` library. It consolidates and
supersedes the design intent from the earlier documents `llm_arch_overview.md`,
`curl_manager_design.md`, `http_manager_design.md`, and `frozen_http_requests.md`.

## Guiding Principles

1. **Transport is mechanical. Policy is intentional. Behavior must be explainable.** Every
   observable behavior traces back to an explicit policy decision or a documented protocol
   constraint.
2. **Call semantics and provider heterogeneity are orthogonal.** How a call is coordinated (single,
   grouped, streaming, race) must work identically regardless of which provider, model, or
   credentials are involved.
3. **Provider-specific APIs are first-class.** The library does not force providers into a lowest
   common denominator. Normalization is layered on top and opt-in.

## Layer Model

The library is structured as four layers, each with strict responsibilities and no upward
dependencies.

```
┌──────────────────────────────────────────────────┐
│  User Code                                       │
├──────────────────────────────────────────────────┤
│  Projections          (optional normalization)    │   interface/projections/
├──────────────────────────────────────────────────┤
│  Client / Coordination                           │   interface/clients/
│    - LLMClient abstraction                       │
│    - Call semantics (single, grouped, stream)     │
│    - Completion signaling, cancellation           │
├──────────────────────────────────────────────────┤
│  Protocol Layer                                  │   interface/protocols/
│    - Provider-specific data structures            │   src/protocols/serialize/
│    - JSON serialization / deserialization          │   src/protocols/deserialize/
│    - Enum/Kind string conversions                 │
├──────────────────────────────────────────────────┤
│  Transport Layer                                 │   src/curl.hpp/cpp
│    - HTTP request/response types                  │   src/http.hpp/cpp
│    - libcurl multi-handle (Interface)             │   src/orchestrator.hpp/cpp
│    - Attempt lifecycle                            │   src/memory.hpp
│    - Connection management                        │
├──────────────────────────────────────────────────┤
│  Dependencies (libcurl, simdjson, nghttp2, etc.) │   deps/
└──────────────────────────────────────────────────┘
```

## Transport Layer

### Frozen HTTP Request Boundary

The transport layer operates exclusively on bytes. HTTP request construction is a pure
transformation from semantic inputs to bytes, performed entirely before the request enters
transport. The result is a "frozen" request: an immutable combination of HTTP headers and a byte
buffer body.

```
Domain Inputs (prompts, images, tool definitions)
        ↓
Protocol Layer (JSON serialization)
        ↓
Frozen HTTP Request (headers + byte buffer)
        ↓
Transport / Attempts (libcurl, retries)
```

Once frozen, the request is never re-serialized. Retries resend the identical byte stream, ensuring
determinism and eliminating drift between attempts.

### curl::Interface

Owns the libcurl multi-handle (`CURLM*`). Manages connection pooling, HTTP/2 multiplexing, and
socket-level I/O. Configured via `ConnectionPolicy` (max connections, max HTTP/2 streams per
connection).

The Interface is not copyable or movable. It is designed to be owned by a single `Orchestrator`.

Key operations: `AddHandle` (register a curl easy handle), `RemoveHandle` (deregister),
`ExecOnce` (drive one iteration of the event loop, returning completed Attempts).

### curl::Attempt

Represents a single HTTP request/response lifecycle. Wraps a curl easy handle and owns a
`curl::Response` that accumulates data through libcurl callbacks.

An Attempt is pinned in memory (no copy, no move) because libcurl holds a pointer to it for
callback dispatch. It is created with a reference to the Interface, an `AttemptPolicy`, the HTTP
method, URL, headers, and body.

Attempt states: `NOT_INITIALIZED` → `IN_PROGRESS` → `COMPLETED` or `FAILED`.

The `Response` has a separate `Availability` progression that tracks what data has been received:
`NOT_INITIALIZED` → `STARTED` → `UPLOAD_COMPLETE` → `DOWNLOAD_COMPLETE` → `FINAL`.

### curl::HeaderList

RAII wrapper around `curl_slist`. Constructed from `http::RequestHeaders`.

### Threading Model

The library core is single-threaded. All transport operations are driven by a single owning thread
per Interface. Parallelism is achieved by running multiple independent Interfaces on separate
threads when required. The library does not perform implicit multi-threading.

### Global Initialization

libcurl requires a single process-wide initialization (`curl_global_init`). This is handled by the
`Interface::Global` static member and is not restartable within a process.

## Protocol Layer

Each provider has a self-contained set of data structures in `interface/protocols/<provider>/` that
model the API's request and response payloads as C++ types. These are semantic models, not
transport contracts — they represent the meaning of the API fields, not the wire format.

Serialization (struct → JSON bytes) and deserialization (JSON bytes → struct) are implemented in
`src/protocols/serialize/` and `src/protocols/deserialize/` respectively. Both use simdjson: the
`string_builder` API for serialization and the `dom` API for deserialization.

See `docs/serialization.md` for the framework details and `docs/protocols.md` for per-provider
coverage.

## Client / Coordination Layer

### LLMClient (interface/clients/client.hpp)

An LLMClient represents a fixed execution context for a class of LLM requests. It encapsulates:
provider identity, authentication material, base endpoint, and provider-specific protocol decisions.

Once created, an LLMClient is immutable. It is not a scheduler, queue, thread, transport engine,
rate limiter, or workflow executor.

The current `Client` implementation uses type erasure with a vtable and small-buffer optimization
(64 bytes, 8-byte aligned) rather than virtual inheritance. This avoids the overhead of virtual
dispatch and heap allocation for small client implementations.

### Call Semantics

Call semantics describe how LLM invocations are coordinated:
- **Single call** — one invocation, completion is per request.
- **Independent concurrent calls** — multiple calls, each completes independently.
- **Grouped / barrier calls** — a set that completes as a group.
- **First-success / race** — completion on first success.
- **Streaming calls** — partial results produced incrementally.
- **Detached calls** — execution continues independently of the caller.

These operate identically regardless of provider heterogeneity.

### Policy

Policy is a first-class concept. All non-trivial behaviors are policy-declared, not implicit.

`AttemptPolicy` controls per-request behavior: timeouts, HTTP version, redirects, TLS, debugging.
`ConnectionPolicy` controls per-Interface behavior: max connections, HTTP/2 stream limits.
`ClientPolicy` bundles both.

Future policy dimensions (retry, jitter, rate limiting, network smoothing) are planned but not yet
implemented. See `docs/roadmap.md`.

## Projections Layer

Projections provide optional normalized APIs that map common cross-provider operations into a
single interface. The `proj::text` namespace implements this for basic text/image prompting.

A projection's `Generate<T>()` function template takes a provider-neutral description (system
prompt, content blocks, options) and produces a provider-specific `Request` struct. This is a pure
transformation — it does not perform any I/O.

Projections are convenience, not abstraction. They do not hide provider capabilities and are not
required.

## Orchestrator

The `Orchestrator` (in `src/orchestrator.hpp`) ties together the transport layer components. It
owns a `curl::Interface` and a `SlotPool<Attempt>` for managing Attempt lifetimes.

This component is in early development. Its intended responsibilities include: creating Attempts
from frozen requests, driving the Interface event loop, managing Attempt lifecycle, and
coordinating with the Client layer.

## Data Flow (End to End)

1. User constructs a provider-specific `Request` struct (directly or via a projection).
2. The Protocol Layer serializes the struct to JSON bytes, producing headers + body.
3. The frozen request enters the Transport Layer.
4. The Orchestrator creates an `Attempt` and registers it with the `Interface`.
5. `Interface::ExecOnce()` drives libcurl I/O.
6. On completion, the `Attempt` is finalized and its `Response` populated.
7. The Protocol Layer deserializes the response body into a provider-specific `Response` struct.
8. The result is returned to the user.
