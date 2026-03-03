# Client and Orchestrator Design

Design document for the transport and concurrency layer of the `jai::llm` library. This covers how
typed LLM requests become HTTP calls, how those calls are managed concurrently, and how responses
flow back to the user.

## Goals

1. Massive parallel HTTP calls to LLM providers with dynamic rate limit management.
2. Provider-agnostic transport — the orchestrator knows HTTP, not Anthropic/Gemini/OpenAI.
3. Support both projection-based usage (constrained, easy) and raw API usage (full control).
4. Sync and async call patterns (targeting C++26 coroutines, with sync fallback).
5. Single-threaded curl event loop now, multi-threaded support later.

## Layering

```
User
  │
  ├─ Projection path:  projection::text::{Generate, Extract}
  │                     ↕
  ├─ Raw API path:      provider::{Request, Response} directly
  │                     ↕
  ▼
Client  ── adapter between typed world and HTTP world
  │         (holds auth, model, endpoint, policy)
  │         (serializes request, deserializes response)
  ▼
Orchestrator  ── owns transport, concurrency, retry, rate limits
  │               (handle-based: Submit → Poll → GetResponse)
  ▼
curl::Interface  ── libcurl multi handle, connection pooling
```

## Component Responsibilities

### User

The user provides prompts/content and receives results. For projection-based usage, the user works
with `projection::text::Result` and never touches provider types. For raw API usage, the user works
directly with provider request/response types (e.g., `anthropic::Request` → `anthropic::Message`).

If the LLM response contains structured data (e.g., JSON), the user parses it themselves. The
library's job ends at delivering the response string or typed response object.

### Client

The client is an adapter — lightweight state plus provider-specific translation functions. Each
provider has its own typed client class (e.g., `anthropic::Client`, `gemini::Client`,
`openai::Client`). The client does not manage concurrency, retry, or transport — it holds a
reference to a shared `Orchestrator` and delegates transport through it.

**State:**
- Reference to the Orchestrator (not owned)
- Authentication credentials (API key or Google credentials)
- Model string
- Endpoint URL (default per provider, overridable)
- ClientPolicy (AttemptPolicy + ConnectionPolicy)

**Pre-processing (request → HTTP):**
- Serialize the typed request to bytes (`Serialize`)
- Generate the HTTP method, URL, and headers (`GenMethod`, `GenUrl`, `GenRequestHeaders`)
- Package everything into an `http::Request`

**Post-processing (HTTP → result):**
- Deserialize response bytes into the typed provider response (`Deserialize`)
- For projections: extract the projection result (`Extract`)
- Return the typed result to the user

The client does not call curl directly and does not see orchestrator internals. It submits an
`http::Request` to the orchestrator, receives a `Handle`, waits for completion, and retrieves
the `curl::Response` for deserialization.

### Orchestrator

The orchestrator is a managed HTTP transport engine. It is provider-agnostic — it operates on
`http::Request` objects and returns `curl::Response` objects. The orchestrator uses a handle-based
pull model: the client submits work and receives a handle, then retrieves the result after
completion. No callbacks cross the orchestrator boundary.

**Interface:**
- `Submit(http::Request, AttemptPolicy, QueueKey) → Handle`
- `Poll() → size_t` (single event loop iteration, returns remaining count)
- `RunUntilComplete()` (blocks until all work reaches a terminal state)
- `GetStatus(Handle) → SlotState`
- `GetResponse(Handle) → curl::Response const&` (on success)
- `GetError(Handle) → string_view` (on failure)
- `Release(Handle)` (return slot to pool)

**Request lifecycle (internal):**
1. `Submit` allocates a Slot in stable storage (deque), assigns it to a queue.
2. Queue dispatch: when rate limits and concurrency caps allow, the orchestrator constructs
   a `curl::Attempt` from the slot's `http::Request` data and adds it to the `curl::Interface`.
3. `Poll()` calls `curl::Interface::ExecOnce()` to drive transfers.
4. On attempt completion: check HTTP status code.
5. If retryable (429, 5xx, network error): consult retry policy, schedule retry or mark failed.
6. If successful: transition slot to `DONE_SUCCESS`. The `curl::Response` is available to the
   caller through `GetResponse(handle)`.
7. If failed (all retries exhausted): transition slot to `DONE_FAILURE`.

**Concurrency management:**
- Queue(s) keyed by `QueueKey{auth_identity, endpoint_url}` — requests sharing a queue share
  rate limit state.
- Track provider rate limit headers from responses (`x-ratelimit-remaining-requests`, etc.).
- Dynamic send rate: start aggressive (`initial_max_concurrent`), seed watermarks from first
  response headers, back off on 429.
- Respect `curl::Interface` connection limits (`ConnectionPolicy`).

**Rate limit strategy:**
- All three providers return rate limit headers on every response.
- Use these to proactively gate new requests rather than purely reacting to 429 errors.
- Maintain per-queue watermarks: remaining requests, remaining tokens, reset timestamps.
- On 429: honor `Retry-After` header if present, otherwise exponential backoff.

**Retry policy:**
- Retryable conditions: 429 (rate limited), 5xx (server error), network errors, deserialization
  failures (optional, configurable — see note below).
- Non-retryable: 4xx other than 429 (bad request, auth failure, etc.).
- Configurable: max retries, backoff strategy (exponential, linear, fixed), jitter.
- Note: retry-on-deserialization-failure requires a callback mechanism that is not yet
  implemented. The initial version retries only on transport-level failures.

### curl::Interface (existing)

Wraps libcurl's multi handle. Provides:
- `AddHandle` / `RemoveHandle` for managing easy handles.
- `ExecOnce()` — single poll iteration, returns list of completed `Attempt*` pointers.
- Connection pooling governed by `ConnectionPolicy`.

The orchestrator drives `ExecOnce()` in a loop (sync) or yields between iterations (async).

### Slot Pool

`curl::Attempt` objects are pinned (no copy/move) because libcurl locks the pointer. The
orchestrator stores all per-request state in `Slot` objects held in a `std::deque<Slot>`.
Deque provides stable references (no invalidation on push_back), and slots are never moved
after construction. Each slot contains:

- The `http::Request` data (owned, preserved for retries)
- The `AttemptPolicy` (for creating curl::Attempt)
- The `QueueKey` (for queue assignment)
- A `curl::HeaderList` (prepared once from request headers)
- An `std::optional<curl::Attempt>` (emplaced when dispatched, reset on completion)
- State tracking: `SlotState`, retry count, retry-after timestamp
- Terminal state: error message (on failure)

Freed slot indices are tracked in a free list for reuse. The caller releases slots through
`Release(Handle)` after reading results.

## Data Flow

### Request path (user → wire)

```
User provides: content, options (projection) or typed Request (raw API)
     │
     ▼
Client pre-processes:
  Projection: Generate<ProviderRequest>(system_prompt, content, options)
  Raw API:    user provides Request directly
     │
     ▼
Client serializes:
  provider::Serialize(request) → bytes
  provider::GenMethod(request) → http::Method
  provider::GenUrl(request) → string
  provider::GenRequestHeaders(request) → http::RequestHeaders
     │
     ▼
Client assembles: http::Request{headers, method, url, body}
     │
     ▼
Client calls: orchestrator.Submit(http_request, attempt_policy, queue_key) → Handle
```

### Response path (wire → user)

```
Orchestrator: curl::Interface completes an Attempt
     │
     ▼
Orchestrator reads curl::Response (status code, headers, body)
     │
     ├─ HTTP error (429, 5xx, network) → consult retry policy → re-queue or fail
     │
     ▼
Orchestrator transitions slot to DONE_SUCCESS or DONE_FAILURE
     │
     ▼
Client checks: orchestrator.IsTerminal(handle)
     │
     ▼
Client reads: orchestrator.GetResponse(handle) → curl::Response const&
     │
     ▼
Client deserializes:
  provider::Deserialize(curl_response) → typed response
  For projections: Extract(typed_response) → projection::text::Result
     │
     ▼
Client calls: orchestrator.Release(handle)
     │
     ▼
Client returns typed result to user
```

### Sync call flow (single request)

```cpp
// Inside anthropic::Client::CallSync(const Request& req):
auto http_req = http::Request{
    .headers = anthropic::GenRequestHeaders(req),
    .method  = anthropic::GenMethod(req),
    .url     = anthropic::GenUrl(req),
    .body    = anthropic::Serialize(req)
};
auto key = QueueKey{.auth_identity=api_key, .endpoint_url=endpoint_url};
auto handle = orchestrator.Submit(std::move(http_req), policy.attempt_policy, key);
orchestrator.RunUntilComplete();

if (!orchestrator.IsTerminal(handle)) { /* should not happen */ }
if (orchestrator.GetStatus(handle) == Orchestrator::SlotState::DONE_FAILURE) {
    throw AnnotatedException{std::string{orchestrator.GetError(handle)}};
}

auto response = anthropic::Deserialize(orchestrator.GetResponse(handle));
orchestrator.Release(handle);
return response;
```

### Parallel call flow (multiple requests)

```cpp
// Submit N requests (same or different providers)
auto handles = std::vector<Orchestrator::Handle>{};
for (auto const& req : requests) {
    auto http_req = /* serialize */;
    handles.push_back(orchestrator.Submit(std::move(http_req), policy, key));
}

// Wait for all
orchestrator.RunUntilComplete();

// Collect results
for (auto h : handles) {
    if (orchestrator.GetStatus(h) == Orchestrator::SlotState::DONE_SUCCESS) {
        auto response = provider::Deserialize(orchestrator.GetResponse(h));
        // use response...
    }
    orchestrator.Release(h);
}
```

## Policy Configuration

### AttemptPolicy (per-request, existing)

Curl-level settings for individual HTTP requests: timeouts, TLS, redirect handling, HTTP version.
Applied when creating each `Attempt`.

### ConnectionPolicy (per-Interface, existing)

Curl multi handle settings: max total connections, max HTTP/2 streams per connection.
Applied when creating the `curl::Interface`.

### RetryPolicy (new, per-orchestrator)

Governs retry behavior:
- `max_retries` — maximum number of retry attempts per request (default: 3).
- `backoff_strategy` — EXPONENTIAL, LINEAR, or FIXED (default: EXPONENTIAL).
- `base_delay` — initial delay before first retry (default: 500ms).
- `max_delay` — cap on backoff delay (default: 30s).
- `jitter` — randomize delay to avoid thundering herd (default: true).
- `retry_on_deserialize_failure` — reserved for future callback mechanism (default: false).
- `retryable_status_codes` — which HTTP status codes trigger retry (default: {429, 500..599}).

### RateLimitPolicy (new, per-orchestrator, applied per-queue)

Governs proactive rate limiting:
- `initial_max_concurrent` — starting concurrency before first response headers arrive (default: 8).
- `use_provider_headers` — whether to read and honor rate limit headers (default: true).
- `min_remaining_before_backoff` — threshold for preemptive slowdown (default: 2).
- `backoff_floor` — minimum delay when rate-limited (default: 100ms).

## Sync vs Async

**Sync:** The client calls `orchestrator.RunUntilComplete()`, which drives `ExecOnce()` in a
blocking loop until all submitted requests complete. The client then reads results through handles.

**Async (C++26):** The client calls `orchestrator.Poll()` between coroutine yields, allowing the
caller's event loop to interleave other work. Results are read through handles when slots reach
terminal state. The coroutine machinery resumes the awaiting coroutine when its handle becomes
terminal.

Both modes use the same orchestrator internals. The difference is only in how the outer loop is
driven (blocking vs yielding).

## Threading Model

Current: single-threaded. The curl multi handle and orchestrator run on one thread. This is
libcurl's preferred mode.

Future: the orchestrator will support running on a user-specified thread. Internal state will need
synchronization at that point. Design now with the awareness that shared state (queues, rate limit
watermarks, retry counters) will eventually need protection, but do not add synchronization
prematurely.

## Queue Keying

Requests are grouped into queues that share rate limit state. The key is
`QueueKey{auth_identity, endpoint_url}` since providers scope their rate limits to the API key and
endpoint. Two different API keys hitting the same provider have independent limits and should use
separate queues.

Model-level sub-limits exist for some providers but are reported in the same rate limit headers.
Sub-partitioning by model can be added later if needed.

Queues are created lazily on first `Submit` for a given key.

## Open Questions

- Should the orchestrator support priority levels within a queue?
- How should the pool grow? (Currently: deque grows on demand, freed indices are recycled.
  Consider an upper bound to prevent runaway memory.)
- Provider-level fallback (if Anthropic fails, try OpenAI): user-managed for now, potentially
  a higher-level policy later.
- Retry-on-deserialization-failure: requires a callback from client → orchestrator. Deferred
  until the need is demonstrated. For now, transport-only retries.
- Per-queue override of RetryPolicy/RateLimitPolicy (currently orchestrator-wide).
