# Transport Layer

The transport layer handles HTTP request execution via libcurl. It operates exclusively on bytes —
it has no knowledge of JSON, prompts, or provider semantics.

## Files

| File | Contents |
|------|----------|
| `src/curl.hpp` | `Interface`, `Attempt`, `HeaderList`, `Response` |
| `src/curl.cpp` | Implementation |
| `src/http.hpp` | `Request`, `RequestHeaders`, `ResponseHeaders`, HTTP `Method` |
| `src/http.cpp` | Implementation |
| `src/orchestrator.hpp` | `Orchestrator` (in progress) |
| `src/orchestrator.cpp` | Implementation |
| `src/memory.hpp` | `SlotPool` allocator |
| `interface/core/policy.hpp` | `AttemptPolicy`, `ConnectionPolicy`, `ClientPolicy` |

## http::Request

A frozen HTTP request ready for transport:

```cpp
struct Request {
    RequestHeaders headers;
    Method method;
    std::string url;
    std::vector<std::byte> body;
};
```

This is the boundary between the protocol layer and the transport layer. Once constructed, the
request is immutable. Retries reuse the same frozen request without re-serialization.

## http::RequestHeaders

RAII container for validated HTTP headers. Constructed from either key-value pairs
(`HeaderKVRange_c`) or pre-merged "Key: Value" strings (`MergedHeaderRange_c`).

Headers are validated on construction:
- Security check rejects headers containing newlines or other injection vectors.
- Each entry is stored as a single `std::string` in "Key: Value" format.

## http::ResponseHeaders

Processes response headers received from libcurl. Headers that fail validation (missing colon,
embedded newlines) are moved to a `dropped_headers` list with a reason code rather than
silently discarded.

Supports `AddDefaultHeader`/`AddDefaultHeaders` for injecting headers that should be present
if the server didn't send them.

## curl::Interface

Owns the libcurl multi-handle (`CURLM*`). This is the central transport engine.

```cpp
class Interface {
    explicit Interface(const ConnectionPolicy& policy);

    std::string AddHandle(void* curl_easy_handle);
    std::vector<Attempt*> ExecOnce();
    std::string RemoveHandle(void* curl_easy_handle) noexcept;
};
```

**Construction:** Takes a `ConnectionPolicy` to configure connection limits and HTTP/2 stream
multiplexing. Performs global libcurl initialization on first use (via the static `Global` member).

**AddHandle:** Registers a curl easy handle (owned by an `Attempt`) with the multi-handle.
Returns an error string (empty on success).

**ExecOnce:** Drives one iteration of the libcurl event loop:
1. Calls `curl_multi_perform` to progress active transfers.
2. Calls `curl_multi_info_read` to collect completed transfers.
3. Finalizes completed Attempts and returns pointers to them.

This is a non-blocking poll. The caller decides when and how often to call it.

**RemoveHandle:** Deregisters a curl easy handle from the multi-handle. Called during Attempt
cleanup.

### Constraints

- Not copyable or movable (the multi-handle has process-global state dependencies).
- Designed to be owned by a single Orchestrator.
- Single-threaded: all calls must come from the same thread.

## curl::Attempt

Represents a single HTTP request/response lifecycle. Wraps a curl easy handle.

```cpp
class Attempt {
    explicit Attempt(Interface& interface,
                     const AttemptPolicy& policy,
                     http::Method method,
                     const std::string& url,
                     const HeaderList& header_list,
                     const std::vector<std::byte>& body);

    void Finalize(Interface& interface, const std::string& result_error_str) noexcept;

    const Response& GetResponse() const;
    Response::State GetState() const;
    bool IsCompleted() const;
    bool IsDone() const;
    bool IsFailed() const;
};
```

**Construction:** Creates a curl easy handle, configures it with the policy settings (timeouts,
TLS, redirects, HTTP version), sets the URL, headers, and body, and registers libcurl callbacks.

**Pinned in memory:** Attempts cannot be copied or moved because libcurl holds a pointer to the
Attempt for callback dispatch. The `SlotPool` in `Orchestrator` manages Attempt lifetimes.

**Callbacks:** Four libcurl callbacks route through the Attempt:
- `OnHeader` — accumulates response headers.
- `OnRead` — provides request body data to libcurl.
- `OnWrite` — accumulates response body data.
- `OnXferInfo` — progress reporting (download/upload bytes).

**Finalize:** Called exactly once when libcurl reports `CURLMSG_DONE`. Extracts metadata from
the easy handle (status code, HTTP version, redirect count, timing, effective URL) and transitions
the Attempt to `COMPLETED` or `FAILED`.

## curl::Response

Accumulated data from a completed (or failed) Attempt:

```cpp
struct Response {
    State state;                  // NOT_INITIALIZED, IN_PROGRESS, COMPLETED, FAILED
    Availability availability;    // Monotonic progression of available data

    int64_t status_code;
    int64_t http_version;
    int64_t redirect_count;
    int64_t total_time_us;
    int64_t total_wire_bytes_downloaded;
    int64_t total_wire_bytes_uploaded;
    size_t body_len;

    std::string effective_url;
    std::string error_message;

    std::vector<std::byte> body;
    std::vector<std::vector<std::byte>> headers;
    std::vector<std::vector<std::byte>> abnormal_headers;

    // Progress estimates (available during transfer)
    int64_t current_leg_download_progress_bytes;
    int64_t current_leg_download_total_estimate_bytes;
    int64_t current_leg_upload_progress_bytes;
    int64_t current_leg_upload_total_estimate_bytes;
};
```

### Availability Progression

Data becomes available in stages:

```
NOT_INITIALIZED → STARTED → UPLOAD_COMPLETE → DOWNLOAD_COMPLETE → FINAL
```

- **NOT_INITIALIZED:** Only `state` and `error_message` are meaningful.
- **STARTED:** Progress estimates available.
- **UPLOAD_COMPLETE:** Request body fully sent.
- **DOWNLOAD_COMPLETE:** Response headers and abnormal_headers available.
- **FINAL:** All data available including body, status_code, timing, etc.

The body is deferred to FINAL because simdjson (and other JSON parsers) require padding bytes
appended to the buffer. This padding is added during finalization.

### Headers

Response headers are stored as raw byte vectors (`std::vector<std::byte>`), one per header line.
Abnormal headers (those that don't follow standard formatting) are stored separately. The
`abnormal_headers` vector has one more entry than the number of actual abnormal headers
(initialized with one empty entry).

## curl::HeaderList

RAII wrapper around libcurl's `curl_slist` linked list:

```cpp
class HeaderList {
    explicit HeaderList(const http::RequestHeaders& headers);
    void* Get() const noexcept;
};
```

Move-only. The underlying `curl_slist` is freed on destruction via a custom deleter.

## AttemptPolicy

Per-request configuration (from `interface/core/policy.hpp`):

| Field | Type | Purpose |
|-------|------|---------|
| `connect_timeout` | `optional<milliseconds>` | TCP connect timeout |
| `total_timeout` | `optional<milliseconds>` | Total request timeout |
| `low_speed_time` | `optional<seconds>` | Duration for low-speed check |
| `low_speed_bytes_per_sec` | `optional<size_t>` | Minimum bytes/sec threshold |
| `http_version` | `optional<HTTPVersion>` | Force HTTP/1.1 or HTTP/2 (no fallback) |
| `follow_redirects` | `optional<bool>` | Enable/disable redirect following |
| `max_redirects` | `optional<size_t>` | Maximum redirect hops |
| `ca_bundle_path` | `optional<path>` | Custom CA certificate bundle |
| `client_cert_path` | `optional<path>` | Client certificate for mTLS |
| `client_key_path` | `optional<path>` | Client private key for mTLS |
| `tls_min_version` | `optional<TLSVersion>` | Minimum TLS version (1.2 or 1.3) |
| `verify_peer` | `optional<bool>` | Verify server certificate |
| `verify_host` | `optional<bool>` | Verify hostname matches certificate |
| `forbid_connection_reuse` | `optional<bool>` | Disable keep-alive |
| `fail_on_http_error` | `optional<bool>` | Treat 4xx/5xx as curl errors |
| `enable_debugging` | `optional<bool>` | Enable verbose libcurl output |

All fields are optional. When unset, libcurl's defaults apply.

## ConnectionPolicy

Per-Interface configuration:

| Field | Type | Purpose |
|-------|------|---------|
| `max_total_connections` | `optional<size_t>` | Global connection pool limit |
| `max_http2_streams_per_connection` | `optional<size_t>` | HTTP/2 multiplexing limit |

## Orchestrator

Ties the transport components together (in progress):

```cpp
class Orchestrator {
    explicit Orchestrator(const ConnectionPolicy& policy);

    Attempt MakeAttempt(const AttemptPolicy& policy,
                        http::Method method,
                        const std::string& url,
                        const HeaderList& header_list,
                        const std::vector<std::byte>& body) const;

    std::vector<Attempt*> SyncAttempt(Attempt& attempt);
};
```

Owns a `curl::Interface` and a `SlotPool<Attempt>` for lifetime management. The `SlotPool` uses
a variant-based free list (`EmptySlot` + active types) backed by a `std::deque` for stable
addresses.

Current status: skeletal. The `MakeAttempt` and `SyncAttempt` methods exist but the full
lifecycle management (creation, polling, finalization, cleanup) is not yet implemented.
