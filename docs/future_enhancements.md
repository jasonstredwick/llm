# jai::llm — Future Enhancements

Candidate improvements identified during the comprehensive test suite work (March 2026). These are additions beyond the current scope of calling LLMs directly.

---

## Error Recovery and Resilience

The retry and rate-limit policies are well-structured at the individual attempt level, but there is no higher-level resilience when an entire provider becomes unavailable. A circuit-breaker pattern could track consecutive failures per provider and temporarily remove it from model group rotation, avoiding wasted attempts against a downed endpoint. Graceful degradation across a model group — where all providers are failing simultaneously — currently propagates individual failures up to the caller with no coordinated fallback behavior.

---

## Observability

`TotalUsage` on Instance provides aggregate token counts, but there is no per-endpoint or per-provider breakdown. For users running multiple providers, knowing "Anthropic used X tokens, OpenAI used Y" would be valuable for cost tracking and capacity planning. There is also no callback or hook mechanism for logging individual attempts or failures — everything is fire-and-forget unless the caller inspects the Result after completion. An optional observer/listener interface (attempt started, attempt completed, attempt failed, retry scheduled) would enable integration with external monitoring systems without polluting the core API.

---

## Configuration Validation

The policy types have good runtime validation via bounded integers and validated string types. However, there is no cross-field validation at the Instance or Config level that catches contradictory or nonsensical settings early. For example, setting a retry timeout shorter than the connection timeout, or specifying a rate limit policy with zero retries alongside a non-zero retry-after header. An eager validation pass at construction time would surface these issues before any requests are attempted.

---

## Request Cancellation

Once an Attempt is launched, it runs to completion or timeout with no way for the caller to abort it. For interactive applications or long-running requests, users may want to cancel in-flight work — for example, if a user navigates away from a page while a response is still pending. A cancellation token or cancel() method on the Attempt or Result handle would allow cooperative cancellation, with the curl layer calling curl_multi_remove_handle to release the connection.

---

## Streaming Responses

All three providers (Anthropic, OpenAI, Gemini) support streaming via Server-Sent Events (SSE). The current architecture is strictly request/response — the full body is buffered before the Result is delivered. Adding streaming support would require a different callback or iterator-based result type that delivers partial tokens as they arrive. This is probably the biggest functional gap versus the raw provider APIs, particularly for chat-style applications where time-to-first-token matters.

---

## Auth Token Refresh

The auth identity types (ApiKeyAuth, AzureAuth, VertexAuth) are constructed once and used for the lifetime of the Instance. This works for static API keys, but not for OAuth-based authentication flows where tokens expire. Vertex AI in particular uses short-lived access tokens that need periodic refresh. A hook or callback to refresh credentials between attempts — or a wrapper that handles token lifecycle — would be needed for production use of providers that require rotating credentials.

---

## Custom Allocators

Several hot-path allocations could benefit from pool or arena allocators. The most frequent is the per-request `std::make_shared<ResultSync>` in `SubmitRequest` — every API call heap-allocates a sync block that is shared between the orchestrator slot and the user-facing Result type. Because the Result lives on the user's side of the library boundary (returned from `CallAsync`), it cannot borrow from an internally-owned object; `shared_ptr` is the natural ownership model. `std::allocate_shared` with a pool allocator would keep the interface intact while amortizing allocation cost. Other candidates include the per-request `std::vector<std::byte>` body from `Serialize` (sized predictably per provider), `AttemptMetadata` construction in the completion path, and the slot pool itself (currently a deque with placement-new reuse, but a dedicated arena could reduce fragmentation). These are all dwarfed by network round-trip time at current scale, but would matter for high-throughput batch workloads.

---

## Sanitizer Build Configurations

The threading additions (lifecycle mutex, client storage shared_mutex, dead atomic) would benefit from runtime verification under ThreadSanitizer (TSan). A CMake option like `ENABLE_TSAN` that appends `-fsanitize=thread` to compile and link flags would allow running the existing test suite — particularly the integration tests with concurrent requests — to detect data races automatically. TSan sets a non-zero exit code on race detection, so CTest reports failures with no test code changes. AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) would also be valuable as separate build configurations. Note that TSan and ASan cannot coexist in the same binary. Clang and GCC support all three on macOS and Linux; MSVC support for TSan is newer and may need separate handling.

---

## Functionality to manage async requests such as gather, barrier, etc.

There are number of simple functions that could help with async call management.  For example, something similar to std::barrier.

---

## Priority / Suggested Order

1. **Observability** — lowest friction, highest immediate value for debugging and cost tracking
2. **Configuration validation** — small scope, prevents subtle bugs
3. **Request cancellation** — important for interactive use cases
4. **Error recovery / circuit breaker** — valuable for production reliability
5. **Auth token refresh** — required for Vertex AI / Azure AD production use
6. **Streaming** — largest scope, but high impact for chat applications
