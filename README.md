# jai::llm

A C++ library for calling LLM provider APIs. Provider-agnostic interface with managed concurrency, rate limiting, retry, and connection pooling — or drop down to the full provider-specific API when you need it.

Supports Anthropic (Messages), OpenAI (Responses), and Google Gemini (GenerateContent).


## Quick Example

```cpp
#include <jai/llm/llm.hpp>
#include <jai/llm/endpoints/anthropic_messages.hpp>
#include <jai/llm/projections/text.hpp>

#include <iostream>

int main() {
    namespace llm = jai::llm;
    namespace text = llm::proj::text;

    // Start the instance (one per process, owns the event loop)
    llm::Instance instance({.threading = llm::Instance::ThreadingMode::INTERNAL});
    instance.Start();

    // Create a text projection client
    auto client = text::ClientHandle<llm::anthropic::Messages>(
        instance,
        llm::anthropic::ApiKeyAuth{.api_key = "sk-ant-..."},
        "claude-sonnet-4.6-20260101"
    );

    // Synchronous call — builds the request, calls the API, extracts text
    auto result = client.CallSync(
        text::Prompt{"You are a helpful assistant."},
        {text::Prompt{"What is the capital of France?"}},
        {.max_output_tokens = 1024}
    );

    if (result.data) {
        std::cout << result.data->text << "\n";
    }

    instance.Stop();
}
```

Swap `anthropic::Messages` for `openai::Responses` or `gemini::GenerateContent` and the rest stays the same.


## Features

- **Three providers, one interface** — Anthropic, OpenAI, and Gemini behind a common text projection, with full provider-specific APIs also available.
- **Managed concurrency** — connection pooling, HTTP/2 multiplexing, and rate limiting driven by provider response headers.
- **Policy-driven behavior** — timeouts, retries, rate limits, and connection settings cascade from instance to client to individual call. All explicit, nothing implicit.
- **Two threading modes** — drive the event loop yourself (manual mode) or let the library spawn its own thread (internal mode). Both support sync and async calls.
- **Modern C++** — C++26 target, C++23 minimum. Designated initializers, `std::variant` for tagged unions, `std::optional` everywhere, no raw pointers in the public API.
- **Static library** — links into your binary with no runtime dependencies beyond the system C++ runtime.


## Providers

| Provider   | Endpoint            | Namespace             | Auth |
|------------|---------------------|-----------------------|------|
| Anthropic  | `/v1/messages`      | `jai::llm::anthropic` | API key |
| OpenAI     | Responses API       | `jai::llm::openai`    | API key, Azure AD |
| Gemini     | `generateContent`   | `jai::llm::gemini`    | API key, Vertex AI |


## Building

### Prerequisites

- CMake 3.25+
- Python 3.10+ (for dependency management during build)
- GitHub CLI (`gh`), authenticated to access the binary repository
- **macOS:** Homebrew LLVM/Clang 18+ recommended for full C++26 support
- **Windows:** Visual Studio 2022 (17.10+)

### Build

```bash
# macOS / Linux
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B build -S .
cmake --build build --config Release

# Windows
cmake -B build -S .
cmake --build build --config Release
```

Dependencies (libcurl, simdjson, nghttp2, zlib, c-ares) are managed automatically via `scripts/manage_deps.py` and `deps.lock.json`. On first build, binaries are downloaded from GitHub or built from source as needed.

### macOS Intel Cross-Compilation

```bash
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -B build_x64 -S .
cmake --build build_x64 --config Release
```

### Tests

```bash
# macOS / Linux
./build/unit_test_http
./build/unit_test_curl

# Windows
.\build\Release\unit_test_http.exe
.\build\Release\unit_test_curl.exe
```


## Documentation

- **[Getting Started](docs/getting_started.md)** — full user guide: threading modes, call styles, policies, error handling, projections, and multi-provider usage.
- **[Architecture](docs/design/architecture.md)** — layer model, data flow, and design principles.
- **[Roadmap](docs/roadmap.md)** — planned enhancements and priorities.
- **[Coding Conventions](docs/coding_conventions.md)** — type patterns, naming rules, and file organization.


## Project Layout

```
interface/           Public headers (the library's API surface)
  core/              Foundational types, error handling, async primitives
  endpoints/         Per-provider endpoint headers (include one to use a provider)
  projections/       Provider-agnostic APIs (e.g. text projection)
  protocols/         Provider-specific request/response data structures

src/                 Implementation (not part of public API)
  protocols/         JSON serialization and deserialization
  projections/       Per-provider Generate/Extract implementations
  curl.hpp/cpp       libcurl wrapper
  http.hpp/cpp       HTTP request/response types
  orchestrator.*     Request lifecycle management

tests/               Unit and integration tests
docs/                Project documentation
scripts/             Build tooling and dependency management
```


## Standards

- **C++ Standard:** C++26 (target), C++23 (minimum fallback)
- **C Standard:** C11
- **Library type:** Static (`.a` on Unix, `.lib` on Windows)
- **Platforms:** macOS ARM64, macOS x64, Windows x64


## License

Apache 2.0 — see [LICENSE](LICENSE).
