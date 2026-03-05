# Roadmap

Planned enhancements and future work for `jai::llm`. Items are grouped by theme and roughly ordered by suggested priority within each group.

Last updated: 2026-03-05


## Near-Term

### Multi-segment model versioning

`ModelInfo::version` is currently a `double`, which handles major.minor (e.g., 4.6) but not deeper version strings like 4.6.1. Replace with a `std::vector<int>` that compares left-to-right. Consider dash-suffixed versions (e.g., `model-4.6-preview`) where the suffix is stored but excluded from numeric comparison.

### Projection completeness

The `proj::text` projection covers text and image input with text output. Missing projections include tool use (function calling), multi-turn conversation management, and document/file content. Each would follow the same Generate/Extract pattern with its own types.

### Observability improvements

`TotalUsage` on Instance provides aggregate token counts but no per-endpoint or per-provider breakdown. For users running multiple providers, per-provider usage would be valuable for cost tracking. An optional observer/listener interface (attempt started, attempt completed, attempt failed, retry scheduled) would enable integration with external monitoring without polluting the core API.

### Configuration validation

No cross-field validation at the Instance or Config level currently catches contradictory settings — for example, a retry timeout shorter than the connection timeout, or a rate-limit policy with zero retries alongside a non-zero retry-after header. An eager validation pass at construction time would surface these early.


## Medium-Term

### Streaming responses

All three providers support streaming via Server-Sent Events (SSE). The current architecture is strictly request/response — the full body is buffered before the Result is delivered. Streaming would require a callback or iterator-based result type that delivers partial tokens as they arrive. This is the biggest functional gap versus the raw provider APIs, particularly for chat-style applications where time-to-first-token matters.

### Request cancellation

Once an Attempt is launched, it runs to completion or timeout. For interactive applications, users may want to cancel in-flight work. A cancellation token or `Cancel()` method on AsyncResult would allow cooperative cancellation, with the curl layer calling `curl_multi_remove_handle` to release the connection.

### Error recovery and resilience

The retry and rate-limit policies handle individual attempt failures, but there is no higher-level resilience when an entire provider becomes unavailable. A circuit-breaker pattern could track consecutive failures per provider and temporarily remove it from model group rotation. Graceful degradation across a model group — where all providers are failing simultaneously — currently propagates individual failures with no coordinated fallback.

### Auth token refresh

Auth types (ApiKeyAuth, AzureAuth, VertexAuth) are constructed once and used for the lifetime of the Instance. This works for static API keys but not for OAuth flows where tokens expire. Vertex AI in particular uses short-lived access tokens. A hook or callback to refresh credentials between attempts would be needed for production use of these providers.

### Async coordination utilities

Simple functions for managing groups of async requests — gather (wait for all), barrier (synchronize a batch), race (first to complete). Similar in spirit to `std::barrier` but for `AsyncResult` objects.


## Longer-Term

### Sanitizer build configurations

A CMake option like `ENABLE_TSAN` that appends `-fsanitize=thread` would allow running the test suite under ThreadSanitizer to detect data races automatically. AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) would also be valuable as separate build configurations. Note that TSan and ASan cannot coexist in the same binary.

### Custom allocators

Several hot-path allocations could benefit from pool or arena allocators — the per-request `shared_ptr` in `SubmitRequest`, the per-request body vector from `Serialize`, and `AttemptMetadata` construction. These are all dwarfed by network round-trip time at current scale but would matter for high-throughput batch workloads.

### Coroutine / executor integration

Coroutine support (CoroResult, CallCoro) was removed because C++ coroutines require an executor/scheduler to control thread affinity, which the library does not provide. Revisit if/when `std::execution` (P2300) matures or if user-supplied executor integration is needed.

### API parity audits

The Python pipeline (`scripts/api_parity/`) is implemented for OpenAI and Anthropic with baselines established. The Gemini extractor is deferred (different doc format). See `docs/design/api_parity_pipeline.md` for the full design including the agent step for propagating changes to C++ code.

### Additional providers

The architecture supports arbitrary providers, but only Anthropic, OpenAI, and Gemini are implemented. Adding a provider requires: protocol structs, serialization/deserialization, endpoint header, and (optionally) projection specializations.
