# Decision Log

Running record of design decisions. New entries are appended at the bottom with a date and context.

## Format

Each entry:
```
### YYYY-MM-DD — Short title
**Context:** What problem or question prompted this decision.
**Decision:** What was decided.
**Rationale:** Why this choice over alternatives.
**Alternatives considered:** What else was evaluated (if applicable).
```

---

### Pre-2026 — Type erasure over virtual inheritance for Client

**Context:** The Client class needs to hold provider-specific implementations behind a uniform
interface without exposing provider details to users.

**Decision:** Use a vtable-based type erasure pattern with small-buffer optimization (64 bytes,
8-byte aligned) instead of virtual base classes.

**Rationale:** Avoids heap allocation for small client implementations. Eliminates virtual
dispatch overhead. Keeps the Client class a value type (movable, no shared ownership). The SBO
size is large enough for typical client state (auth credentials, endpoint URL, metadata).

**Alternatives considered:** `std::variant` (too rigid — can't add providers without modifying
the variant), virtual base class + `unique_ptr` (heap allocation per client, virtual dispatch
overhead), `std::function`-like erasure (doesn't naturally support multiple operations).

---

### Pre-2026 — Required<T> wrapper for non-optional fields

**Context:** Protocol structs have many fields. Distinguishing required vs optional at the type
level prevents bugs where required fields are accidentally left uninitialized.

**Decision:** `Required<T>` is a non-default-constructible wrapper. `std::optional<T>` marks
optional fields. No naked types.

**Rationale:** Compile-time enforcement. Aggregate initialization of a struct with `Required`
members fails if any are omitted. The serializer and deserializer dispatch differently on
`Required` vs `std::optional`, making the field's wire behavior (always present vs omit-if-absent)
follow directly from its type.

---

### Pre-2026 — simdjson for both serialization and deserialization

**Context:** Need a fast JSON library that works for both parsing API responses and building API
requests.

**Decision:** Use simdjson for both directions. `simdjson::dom` for deserialization,
`simdjson::builder::string_builder` for serialization.

**Rationale:** simdjson is the fastest JSON parser available. Using a single library avoids
dependency bloat. The `string_builder` API avoids constructing an intermediate DOM for
serialization — it writes directly to a string buffer.

**Alternatives considered:** nlohmann/json (convenient but slow), rapidjson (fast but API is
cumbersome), separate libraries for read vs write (unnecessary complexity).

---

### Pre-2026 — Static linking for all dependencies

**Context:** The library targets macOS and Windows. Dynamic linking introduces deployment
complexity (finding .dylib/.dll at runtime, version mismatches).

**Decision:** All dependencies (libcurl, nghttp2, zlib, simdjson, c-ares) are statically linked.

**Rationale:** Single static library output. No runtime dependency resolution. Predictable
behavior across environments. The dependency manager builds and caches static archives per
platform.

---

### Pre-2026 — Release-only build configuration

**Context:** The library is a production artifact, not a development tool.

**Decision:** CMake enforces Release-only builds. Debug and other configurations are rejected.

**Rationale:** Simplifies the build matrix. Avoids shipping debug builds accidentally. Testing
is done in Release mode to match production behavior. If debug builds are needed in the future,
this can be relaxed.

---

### Pre-2026 — Frozen HTTP request boundary

**Context:** The transport layer (libcurl) operates on bytes. Higher-level code operates on
semantic structures (prompts, images, JSON). Mixing these concerns in the transport layer would
complicate retries and testing.

**Decision:** Introduce a "frozen request" concept: once a request enters the transport layer,
it is an immutable bundle of headers + byte buffer. All semantic processing (JSON serialization,
base64 encoding) happens before freezing.

**Rationale:** Retries resend identical bytes (no re-serialization drift). The transport layer
is simple and testable. The serialization layer is independently testable. Clear separation of
concerns.

---

### Pre-2026 — Kind tag structs for JSON discriminators

**Context:** Many API types are discriminated unions identified by a "type" field in JSON. Need
a compile-time mechanism to associate types with their discriminator strings.

**Decision:** Use `struct KindFoo : Kind { static constexpr string_view value = "foo"; };` as
tag types. These are used as `Required<KindFoo> type{{}};` in struct definitions.

**Rationale:** The Kind struct carries its wire value as a compile-time constant. The `Kind`
base class enables concept-based dispatch in the serialization framework. The `Required` wrapper
ensures the discriminator is always present in serialized output.

**Alternatives considered:** Using enum values as discriminators (loses the per-type association),
string constants (no type safety), template parameters (harder to read).

---

### Pre-2026 — Single-threaded transport with explicit user threading

**Context:** Need to support high concurrency (thousands of requests) without imposing a
threading model on users.

**Decision:** The library core is single-threaded. All transport operations are driven by a
single thread per `curl::Interface`. Users may run multiple Interfaces on separate threads.

**Rationale:** Simplifies correctness (no internal locking). Users choose their own concurrency
model. libcurl's multi-handle is inherently single-threaded per handle. HTTP/2 multiplexing
provides concurrency without threads.

**Alternatives considered:** Background thread with queue (the earlier `HttpRequestManager`
design — abandoned in favor of explicit user control), thread pool (unnecessary complexity for
the library's scope).

---

### Pre-2026 — Provider-specific APIs as first-class, normalization as opt-in

**Context:** Each LLM provider has unique features. A lowest-common-denominator API would lose
access to provider-specific capabilities.

**Decision:** The primary API surface exposes provider-specific request/response types directly.
A "projection" layer provides optional cross-provider normalization for common use cases.

**Rationale:** Users who need provider-specific features (Anthropic thinking, Gemini grounding,
OpenAI computer use) get direct access. Users who just need basic text completion can use the
projection layer. Neither constrains the other.

---

### Pre-2026 — Naming convention: match JSON field names in C++ members

**Context:** Protocol structs have dozens of fields. Naming them differently from the JSON keys
creates a mapping burden.

**Decision:** C++ member names match JSON field names where possible. Anthropic and OpenAI use
snake_case (matches C++ convention). Gemini uses camelCase (preserved as-is to match the API).

**Rationale:** The `FIELD(obj, member)` macro uses `#member` as the JSON key. Matching names
means the macro works without a separate name argument. `FIELD_ALT` exists for the rare cases
where names must differ.

---

### 2026-02-17 — Documentation structure established

**Context:** The project is ~75% complete with knowledge spread across existing docs, code, and
the author's memory. Need a structure that supports AI-assisted development across sessions.

**Decision:** Established the following documentation set:
- `docs/session_context.md` — starter doc for each AI session
- `docs/architecture.md` — consolidated architecture reference
- `docs/coding_conventions.md` — type patterns and rules
- `docs/serialization.md` — ser/de framework guide
- `docs/protocols.md` — per-provider coverage map
- `docs/transport.md` — curl/http layer reference
- `docs/roadmap.md` — status tracking and remaining work
- `docs/decisions.md` — this file
- `scratch/` — transient working files (not in git)

**Rationale:** A single `session_context.md` entry point gives AI assistants everything they need
to be productive quickly. Detailed docs are split by concern so they can be read selectively.
The `scratch/` folder keeps working artifacts visible and accessible without polluting git.
