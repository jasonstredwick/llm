# Getting Started with jai::llm

A C++ library for calling LLM APIs from Anthropic, OpenAI, and Google (Gemini). Provider-agnostic interface with managed concurrency, rate limiting, retry, and connection pooling.


## Requirements

- CMake 3.25+
- C++26 (Clang 19+ on macOS/Linux) or C++23 (MSVC on Windows)
- Python 3 (for dependency management during build)
- libcurl (fetched automatically by the build system)
- simdjson (fetched automatically by the build system)


## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build system automatically downloads and configures dependencies (libcurl, simdjson) for your platform via `scripts/manage_deps.py`.


## Core Concepts

The library has four main pieces:

- **Instance** — singleton that owns the event loop and manages all HTTP traffic. One per process.
- **ClientHandle** — lightweight handle to a registered provider endpoint. Cheap to copy. Created via `Instance::CreateClient`.
- **Result** — the response from an API call, carrying the deserialized data, error information, and per-attempt diagnostics.
- **Policies** — cascading configuration (orchestrator → client → call site) for timeouts, retries, rate limits, and connection behavior.


## Quick Example — Synchronous Call

The simplest way to make an API call. Blocks the calling thread until the response arrives. Uses internal threading mode so the library's loop thread handles I/O while `CallSync` waits.

```cpp
#include <jai/llm/llm.hpp>
#include <jai/llm/endpoints/anthropic_messages.hpp>
#include <jai/llm/projections/text.hpp>

#include <iostream>
#include <string>

int main() {
    namespace llm = jai::llm;
    namespace text = llm::projection::text;

    // 1. Create the Instance with internal threading
    llm::Instance instance({.threading = llm::Instance::ThreadingMode::INTERNAL});
    instance.Start();

    // 2. Create a client for the Anthropic Messages endpoint
    auto client = instance.CreateClient<llm::anthropic::Messages>(
        llm::anthropic::ApiKeyAuth{.api_key = "sk-ant-..."},
        "claude-sonnet-4-20250514"
    );

    // 3. Build a request using the text projection (provider-agnostic)
    auto request = text::Generate<llm::anthropic::Request>(
        text::Prompt{"You are a helpful assistant."},  // system prompt
        {text::Prompt{"What is the capital of France?"}},  // user content
        {.max_output_tokens = 1024}
    );

    // 4. Make a synchronous call (blocks until the response arrives)
    auto result = client.CallSync(request);

    // 5. Check the result
    if (result.error) {
        std::cerr << "Error: " << *result.error << "\n";
        instance.Stop();
        return 1;
    }

    // result.data is the provider's native response type (anthropic::Message)
    // Use the text projection to extract just the text:
    auto extracted = text::Extract(*result.data);
    std::cout << extracted.text << "\n";

    instance.Stop();
    return 0;
}
```

**Note:** `CallSync` and `AsyncResult::Get()`/`Take()` block the calling thread until the response is ready. In `INTERNAL` threading mode, the library's loop thread drives I/O in the background. In `MANUAL` mode, these blocking calls automatically drive the event loop internally, so they work correctly in both modes. However, while a blocking call is spinning in manual mode, it processes *all* pending work — not just its own request — so concurrent `CallAsync` results may also complete during that time.


## Threading Modes

### Manual Mode (default)

You drive the event loop by calling `ExecOnce()` in your own loop. This gives you full control over when network I/O happens and is single-threaded — no synchronization overhead.

```cpp
llm::Instance instance;  // defaults to ThreadingMode::MANUAL

auto client = instance.CreateClient<llm::anthropic::Messages>(auth, model);
auto async_result = client.CallAsync(request);

// Drive the event loop until the result is ready
while (!async_result.IsReady()) {
    instance.ExecOnce();
}

auto result = async_result.Take();
```

Manual mode is ideal for applications that already have an event loop (game engines, UI frameworks) or want to interleave LLM I/O with other work.

**Note:** `CallSync` and blocking `Get()`/`Take()` also work in manual mode — they drive the event loop internally until the result is ready. However, for maximum control over scheduling, prefer `CallAsync` with your own `ExecOnce()` loop.

### Internal Threading Mode

The Instance spawns a dedicated loop thread. Submissions and results are synchronized internally. Use this when you want fire-and-forget async behavior.

```cpp
llm::Instance instance({
    .threading = llm::Instance::ThreadingMode::INTERNAL
});
instance.Start();

auto client = instance.CreateClient<llm::anthropic::Messages>(auth, model);

// Submit from any thread — the loop thread handles I/O
auto async_result = client.CallAsync(request);

// Block until ready
auto result = async_result.Get();

// When done:
instance.Stop();
```


## Three Ways to Call

Every ClientHandle offers three call styles. All three deliver the same `Result` type.

### CallSync — blocking

Blocks the calling thread until the response arrives. Works in both threading modes.

```cpp
auto result = client.CallSync(request);
```

### CallAsync — non-blocking with polling

Returns an `AsyncResult` immediately. Poll with `IsReady()`, or block with `Get()` / `Take()`.

```cpp
auto async = client.CallAsync(request);

// Submit more work, do other things...

// Then retrieve when ready:
auto result = async.Take();  // blocks if not yet ready
```

## Working with Results

Every call returns a `Result<Endpoint, Data>` (or `Result<Endpoint, void>` for Tier 1 calls). It carries three things:

```cpp
auto result = client.CallSync(request);

// 1. The response data (nullopt on failure)
if (result.data) {
    // result.data is std::optional<Endpoint::Response_t>
    // e.g., anthropic::Message, openai::Response, gemini::GenerateContentResponse
}

// 2. Error string (nullopt on success)
if (result.error) {
    std::cerr << *result.error << "\n";
}

// 3. Per-attempt diagnostics — one entry per counted attempt
for (const auto& attempt : result.attempts) {
    // attempt.status_code   — HTTP status (-1 for transport failure)
    // attempt.duration_us   — round-trip time in microseconds
    // attempt.outcome       — SUCCESS, TRANSPORT_ERROR, HTTP_ERROR, DESERIALIZATION_ERROR
    // attempt.usage         — token counts (if extraction succeeded)
}
```


## The Text Projection — Provider-Agnostic Calls

The raw protocol types (`anthropic::Request`, `openai::Request`, etc.) mirror each provider's API exactly. They are auto-generated and comprehensive, but verbose.

For the common case of sending text (with optional images) and getting text back, the `projection::text` layer provides a provider-agnostic interface:

```cpp
#include <jai/llm/projections/text.hpp>

namespace text = jai::llm::projection::text;

// Build a request for any provider from the same inputs
auto request = text::Generate<llm::anthropic::Request>(
    text::Prompt{"You are a helpful assistant."},
    {text::Prompt{"Explain quicksort briefly."}},
    {.max_output_tokens = 2048, .thinking_effort = text::ThinkingEffort::LOW}
);

// ... make the call ...

// Extract a normalized result from any provider's response
auto extracted = text::Extract(*result.data);
std::cout << extracted.text << "\n";
std::cout << "Tokens: " << extracted.usage.input_tokens
          << " in, " << extracted.usage.output_tokens << " out\n";
```

The text projection also provides Tier 3 call functions that combine Generate + Call + Extract in one step:

```cpp
// One-liner: builds the request, calls the API, extracts text
auto result = text::CallSync(client,
    text::Prompt{"You are helpful."},
    {text::Prompt{"Hello!"}},
    {.max_output_tokens = 512});

if (result.data) {
    std::cout << result.data->text << "\n";
}
```

Tier 3 works with `CallAsync` and `CallSync`.


## Multiple Providers

Using multiple providers simultaneously is straightforward. Each gets its own client; the Instance manages them all.

```cpp
#include <jai/llm/endpoints/anthropic_messages.hpp>
#include <jai/llm/endpoints/openai_responses.hpp>
#include <jai/llm/endpoints/gemini_generate_content.hpp>

llm::Instance instance({.threading = llm::Instance::ThreadingMode::INTERNAL});
instance.Start();

auto anthropic = instance.CreateClient<llm::anthropic::Messages>(
    llm::anthropic::ApiKeyAuth{.api_key = "sk-ant-..."},
    "claude-sonnet-4-20250514");

auto openai = instance.CreateClient<llm::openai::Responses>(
    llm::openai::ApiKeyAuth{.api_key = "sk-..."},
    "gpt-4o");

auto gemini = instance.CreateClient<llm::gemini::GenerateContent>(
    llm::gemini::ApiKeyAuth{.api_key = "AIza..."},
    "gemini-2.5-flash");

// Fire all three concurrently
auto a1 = anthropic.CallAsync(anthropic_request);
auto a2 = openai.CallAsync(openai_request);
auto a3 = gemini.CallAsync(gemini_request);

// Collect results (blocks until each is ready)
auto r1 = a1.Take();
auto r2 = a2.Take();
auto r3 = a3.Take();

instance.Stop();
```

Requests to the same provider share rate limiting and connection pooling automatically. Requests to different providers run fully in parallel.


## Authentication

Each provider has its own auth types, included automatically when you include the endpoint header.

### Anthropic

```cpp
llm::anthropic::ApiKeyAuth{
    .api_key = "sk-ant-...",
    .version = "2023-06-01"  // optional, this is the default
};
```

### OpenAI

```cpp
// Direct API
llm::openai::ApiKeyAuth{.api_key = "sk-..."};

// Azure OpenAI
llm::openai::AzureAuth{
    .api_key = "...",
    .resource_name = "my-resource",
    .deployment = "gpt-4o",
    .api_version = "2024-02-01",
    .use_bearer_token = false  // true for Azure AD tokens
};
```

### Gemini

```cpp
// Google AI Studio
llm::gemini::ApiKeyAuth{.api_key = "AIza..."};

// Vertex AI
llm::gemini::VertexAuth{
    .access_token = "ya29...",
    .project = "my-gcp-project",
    .location = "us-central1"  // defaults to "global"
};
```


## Policy Configuration

Policies cascade in three levels: orchestrator (global defaults) → client (overrides) → call site (per-request). All fields are `std::optional`; unset fields inherit from the level above. Unset at all levels resolves to hardcoded defaults.

### Instance-level (OrchestratorPolicy)

```cpp
llm::Instance instance({
    .threading = llm::Instance::ThreadingMode::INTERNAL,
    .policy = {
        .connection_policy = {
            .max_total_connections = 64
        },
        .attempt_policy = {
            .connect_timeout = std::chrono::seconds{10},
            .total_timeout = std::chrono::seconds{120}
        },
        .retry_policy = {
            .max_retries = 3,
            .retryable_status_codes = std::vector<int64_t>{500, 502, 503, 529}
        },
        .rate_limit_policy = {
            .initial_max_concurrent = 8,
            .use_provider_headers = true
        }
    }
});
```

### Client-level (ClientPolicy)

Override specific settings for a particular provider or model:

```cpp
auto client = instance.CreateClient<llm::anthropic::Messages>(
    auth, model,
    llm::ClientPolicy{
        .attempt_policy = {
            .total_timeout = std::chrono::seconds{300}  // longer for opus
        },
        .rate_limit_policy = {
            .initial_max_concurrent = 2  // conservative for expensive model
        }
    }
);
```

### Per-call (AttemptPolicy)

Override for a single request:

```cpp
auto result = client.CallSync(request, llm::AttemptPolicy{
    .total_timeout = std::chrono::seconds{30}  // short timeout for this call
});
```


## Error Handling

The library uses exceptions for programming errors and unrecoverable conditions, and result-based errors for expected failures (HTTP errors, transport timeouts, deserialization problems).

### Result errors

Check `result.error` after any call. The `attempts` vector tells you what happened:

```cpp
auto result = client.CallSync(request);
if (result.error) {
    std::cerr << "Final error: " << *result.error << "\n";
    for (const auto& a : result.attempts) {
        std::cerr << "  Attempt: HTTP " << a.status_code
                  << " (" << a.duration_us << " us) - " << a.error << "\n";
    }
}
```

### Exceptions

- **`AnnotatedException`** — thrown for programming errors (no Instance exists, invalid client ID, etc.). Carries source location context for debugging.
- **`FatalInstanceError`** — thrown when the Instance is unrecoverably broken (event loop crash, curl multi handle failure). All pending requests are drained. Catch this to detect the condition and construct a new Instance.

```cpp
try {
    instance.ExecOnce();
} catch (const llm::FatalInstanceError& e) {
    // Instance is dead — reconstruct to continue
    std::cerr << "Fatal: " << e.what() << "\n";
}
```


## Observability

```cpp
// Total token usage across all calls since construction
auto usage = instance.TotalUsage();
// usage.input_tokens, usage.output_tokens, etc. (all std::optional<int64_t>)

// Number of requests currently in-flight or queued
size_t pending = instance.PendingCount();

// Whether the internal loop thread is running
bool running = instance.IsRunning();
```


## Lifetime Rules

- Only one `Instance` may exist per process. Attempting to construct a second throws.
- `Instance` must outlive all `ClientHandle`s and all outstanding `AsyncResult` objects.
- `ClientHandle` is cheap to copy and safe to use from multiple threads (the underlying submission path is synchronized).
- `AsyncResult` is not thread-safe — each should be owned by a single thread.
