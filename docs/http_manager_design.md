# Async HTTP/2 Request Manager Design

## Objective
Implement a high-performance, asynchronous HTTP/2 client capable of managing thousands of concurrent requests across multiple providers (OpenAI, Anthropic, Google) using a single `libcurl` multi-handle event loop.

## Core Architecture

### 1. The "Shared-Nothing" Manager
The `HttpRequestManager` will be a singleton-like (or strictly scoped) entity that owns the `CURLM*` (multi-handle).
- **Thread Safety**: It will run its event loop on a *dedicated background thread*.
- **Interaction**: Providers interact with it strictly via a thread-safe `EnqueueRequest` method.
- **Callback**: Responses are delivered via `std::function` callbacks executed on the manager's thread (or potentially dispatched to a thread pool if processing is heavy, but for now, manager thread for simplicity).

### 2. Connection & Stream Pooling
`libcurl` handles connection pooling internally, but we need to manage the *concurrency limit* to ensure we maximize HTTP/2 multiplexing without hitting server-side limits (e.g., 100 streams per connection).
- **Strategy**:
    - We will configure `CURLMOPT_MAX_HOST_CONNECTIONS` to limit connections per host.
    - We will rely on `libcurl`'s internal HTTP/2 multiplexing logic.
    - If `libcurl`'s internal logic is insufficient for "dynamic" stream limits (e.g., if a server sends `MAX_CONCURRENT_STREAMS`), we will implement a lightweight logical queue wrapper.

### 3. Proposed Interface (`src/http_manager.hpp`)

```cpp
#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <map>

// Forward declarations to avoid exposing curl headers in the public API
typedef void CURLM;

namespace jai::llm {

    struct HttpResponse {
        int status_code = 0;
        std::string body;
        std::multimap<std::string, std::string> headers;
        std::string error_message; // Empty if success
    };

    using RequestCallback = std::function<void(const HttpResponse&)>;

    struct HttpRequest {
        std::string method = "POST";
        std::string url;
        std::multimap<std::string, std::string> headers;
        std::string body;

        // Timeout configuration
        long timeout_ms = 30000;

        // Callback to invoke on completion
        RequestCallback on_complete;
    };

    class HttpRequestManager {
    public:
        HttpRequestManager();
        ~HttpRequestManager();

        // Non-copyable/movable to prevent lifecycle issues with background thread
        HttpRequestManager(const HttpRequestManager&) = delete;
        HttpRequestManager& operator=(const HttpRequestManager&) = delete;

        // Initialize the background thread and curl multi-handle
        void Start();

        // Stop the background thread (graceful shutdown)
        void Stop();

        // Thread-safe request submission
        void MakeRequest(HttpRequest request);

    private:
        // The background worker loop
        void EventLoop();

        // Internal helper to process the queue and add to curl
        void ProcessQueue();

        struct Impl; // PIMPL idiom to hide std::thread, std::mutex, CURL dependencies
        std::unique_ptr<Impl> impl_;
    };

} // namespace jai::llm
```

## Implementation Details

### The PIMPL Idiom (Pointer to Implementation)
We will use PIMPL (`struct Impl`) to:
1.  Hide `<curl/curl.h>` from the public header (avoiding `ssize_t` and `windows.h` macro pollution in consumer code).
2.  Hide `std::thread`, `std::mutex`, `std::condition_variable` details.
3.  Maintain ABI stability.

### The Event Loop
The background thread will run a loop effectively doing:
1.  `curl_multi_wait`: Sleep until socket activity or timeout.
2.  `curl_multi_perform`: Read/Write data to sockets.
3.  `curl_multi_info_read`: Check for completed transfers.
4.  **Queue Processing**: Check a thread-safe `std::queue<HttpRequest>` for new items to add to the multi-handle.

### Handling Windows Quirks
- The `.cpp` implementation file will include `platform_compat.hpp` (or the equivalent defines) BEFORE including `curl.h`.
- The public `.hpp` file remains clean C++23.
