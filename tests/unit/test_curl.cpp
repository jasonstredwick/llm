#include <chrono>
#include <cstring>
#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../src/curl.hpp"
#include "../../interface/core/error.hpp"

using namespace jai::llm::curl;
using namespace jai::llm::http;
using namespace jai::llm;


/***
 * Helper
 */
std::vector<std::byte> MakeBody(const std::string& str) {
    std::vector<std::byte> body(str.size());
    std::memcpy(body.data(), str.data(), str.size());
    return body;
}


/***
 * Pure Unit Tests (no network)
 */
void test_header_list() {
    std::println("Testing HeaderList...");

    RequestHeaders headers(std::vector<std::string>{"Content-Type: application/json", "X-Test: value"});
    HeaderList list(headers);
    REQUIRE(list.Get() != nullptr);

    std::println("  [SUCCESS] HeaderList passed.");
}


void test_header_list_empty() {
    std::println("Testing HeaderList from empty headers...");

    RequestHeaders headers;
    HeaderList list(headers);
    // Empty header list: libcurl slist is null, which is valid (means no custom headers)
    REQUIRE(list.Get() == nullptr);

    std::println("  [SUCCESS] HeaderList empty passed.");
}


void test_response_default_state() {
    std::println("Testing Response default state...");

    Response response{};
    REQUIRE_EQ(response.state, Response::State::NOT_INITIALIZED);
    REQUIRE_EQ(response.availability, Response::Availability::NOT_INITIALIZED);
    REQUIRE_EQ(response.status_code, int64_t{-1});
    REQUIRE_EQ(response.http_version, int64_t{-1});
    REQUIRE_EQ(response.redirect_count, int64_t{-1});
    REQUIRE_EQ(response.current_leg_download_progress_bytes, int64_t{-1});
    REQUIRE_EQ(response.current_leg_download_total_estimate_bytes, int64_t{-1});
    REQUIRE_EQ(response.current_leg_upload_progress_bytes, int64_t{-1});
    REQUIRE_EQ(response.current_leg_upload_total_estimate_bytes, int64_t{-1});
    REQUIRE_EQ(response.total_time_us, int64_t{-1});
    REQUIRE_EQ(response.total_wire_bytes_downloaded, int64_t{-1});
    REQUIRE_EQ(response.total_wire_bytes_uploaded, int64_t{-1});
    REQUIRE_EQ(response.body_len, size_t{0});
    REQUIRE(response.body.empty());
    REQUIRE(response.effective_url.empty());
    REQUIRE(response.error_message.empty());
    REQUIRE(response.headers.empty());
    // abnormal_headers is default-initialized to contain one empty vector
    REQUIRE_EQ(response.abnormal_headers.size(), size_t{1});

    std::println("  [SUCCESS] Response default state passed.");
}


void test_connection_refused() {
    std::println("Testing connection refused...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    attempt_policy.total_timeout = std::chrono::milliseconds(3000);

    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:19999/nothing";
    std::vector<std::byte> body{};

    Attempt attempt(iface, attempt_policy, Method::GET, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(attempt.IsFailed());
    REQUIRE(!attempt.GetErrorMessage().empty());

    std::println("  [SUCCESS] Connection refused passed. Error: {}", attempt.GetErrorMessage());
}


/***
 * Integration Tests (require mock server on port 8080)
 */
void test_sync_post() {
    std::println("Testing sync POST...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    auto body = MakeBody(R"({"model": "gpt-3.5-turbo", "messages": [{"role": "user", "content": "Hello"}]})");

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(!attempt.IsFailed());
    REQUIRE(attempt.IsCompleted());

    const auto& res = attempt.GetResponse();
    REQUIRE_EQ(res.status_code, int64_t{200});
    REQUIRE(res.body_len > 0);
    REQUIRE(!res.body.empty());

    // Metadata assertions
    REQUIRE_EQ(res.effective_url, url);
    REQUIRE(res.http_version > 0);
    REQUIRE(res.total_time_us > 0);
    REQUIRE(res.total_wire_bytes_downloaded > 0);
    REQUIRE(res.total_wire_bytes_uploaded > 0);
    REQUIRE_EQ(res.redirect_count, int64_t{0});
    REQUIRE_EQ(res.availability, Response::Availability::FINAL);

    std::println("  [SUCCESS] Sync POST passed. status={}, body_len={}", res.status_code, res.body_len);
}


void test_sync_post_response_headers() {
    std::println("Testing sync POST response headers...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    auto body = MakeBody(R"({"model": "test", "messages": [{"role": "user", "content": "Hi"}]})");

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(attempt.IsCompleted());
    const auto& res = attempt.GetResponse();

    // Should have at least one set of response headers
    REQUIRE(!res.headers.empty());

    // Check that the headers contain Content-Type (as raw bytes)
    bool found_content_type = false;
    for (const auto& header_block : res.headers) {
        std::string_view block_sv(reinterpret_cast<const char*>(header_block.data()), header_block.size());
        if (block_sv.find("Content-Type") != std::string_view::npos ||
            block_sv.find("content-type") != std::string_view::npos) {
            found_content_type = true;
        }
    }
    REQUIRE(found_content_type);

    std::println("  [SUCCESS] Sync POST response headers passed. header_blocks={}", res.headers.size());
}


void test_sync_post_empty_body() {
    std::println("Testing sync POST with empty body...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    std::vector<std::byte> body{}; // empty

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    // The important part is that curl doesn't crash or hang.
    // Server may return any status for a malformed/empty-body request.
    REQUIRE(attempt.IsDone());

    std::println("  [SUCCESS] Sync POST empty body passed. status={}",
                 attempt.GetResponse().status_code);
}


void test_sync_get() {
    std::println("Testing sync GET...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/health";
    std::vector<std::byte> body{};

    Attempt attempt(iface, attempt_policy, Method::GET, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(attempt.IsCompleted());
    const auto& res = attempt.GetResponse();
    REQUIRE_EQ(res.status_code, int64_t{200});
    REQUIRE(res.body_len > 0);

    // Body should contain "ok"
    std::string_view body_sv(reinterpret_cast<const char*>(res.body.data()), res.body_len);
    REQUIRE(body_sv.find("ok") != std::string_view::npos);

    std::println("  [SUCCESS] Sync GET passed. status={}, body_len={}", res.status_code, res.body_len);
}


void test_http_429() {
    std::println("Testing HTTP 429...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/simulate_429";
    auto body = MakeBody(R"({"test": true})");

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    // 429 is a valid HTTP response, not a curl failure
    REQUIRE(attempt.IsCompleted());
    const auto& res = attempt.GetResponse();
    REQUIRE_EQ(res.status_code, int64_t{429});
    REQUIRE(res.body_len > 0);

    // Body should contain error message
    std::string_view body_sv(reinterpret_cast<const char*>(res.body.data()), res.body_len);
    REQUIRE(body_sv.find("Rate limit") != std::string_view::npos);

    std::println("  [SUCCESS] HTTP 429 passed. status={}", res.status_code);
}


void test_timeout() {
    std::println("Testing timeout via AttemptPolicy...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    attempt_policy.total_timeout = std::chrono::milliseconds(200); // very short

    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    // Request a 5-second delay from the mock server
    std::string url = "http://127.0.0.1:8080/v1/chat/completions?delay_ms=5000";
    auto body = MakeBody(R"({"model": "test", "messages": [{"role": "user", "content": "Hi"}]})");

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(attempt.IsFailed());
    REQUIRE(!attempt.GetErrorMessage().empty());

    std::println("  [SUCCESS] Timeout passed. Error: {}", attempt.GetErrorMessage());
}


void test_multiple_concurrent_attempts() {
    std::println("Testing multiple concurrent attempts...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    auto body = MakeBody(R"({"model": "test", "messages": [{"role": "user", "content": "concurrent"}]})");

    // Create 3 simultaneous attempts
    Attempt a1(iface, attempt_policy, Method::POST, url, header_list, body);
    Attempt a2(iface, attempt_policy, Method::POST, url, header_list, body);
    Attempt a3(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!a1.IsDone() || !a2.IsDone() || !a3.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(a1.IsCompleted());
    REQUIRE(a2.IsCompleted());
    REQUIRE(a3.IsCompleted());
    REQUIRE_EQ(a1.GetResponse().status_code, int64_t{200});
    REQUIRE_EQ(a2.GetResponse().status_code, int64_t{200});
    REQUIRE_EQ(a3.GetResponse().status_code, int64_t{200});

    std::println("  [SUCCESS] Multiple concurrent attempts passed.");
}


void test_attempt_state_accessors() {
    std::println("Testing Attempt state accessors...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/health";
    std::vector<std::byte> body{};

    Attempt attempt(iface, attempt_policy, Method::GET, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE_EQ(attempt.GetState(), Response::State::COMPLETED);
    REQUIRE(attempt.IsUnhooked());
    REQUIRE(attempt.GetErrorMessage().empty());

    std::println("  [SUCCESS] Attempt state accessors passed.");
}


void test_http_404() {
    std::println("Testing HTTP 404...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    // Request an unknown path — mock server returns 404
    std::string url = "http://127.0.0.1:8080/nonexistent/path";
    std::vector<std::byte> body{};

    Attempt attempt(iface, attempt_policy, Method::GET, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    // 404 is a valid HTTP response, not a curl error
    REQUIRE(attempt.IsCompleted());
    REQUIRE(!attempt.IsFailed());
    REQUIRE_EQ(attempt.GetResponse().status_code, int64_t{404});

    std::println("  [SUCCESS] HTTP 404 passed. status={}", attempt.GetResponse().status_code);
}


void test_response_body_content_post() {
    std::println("Testing POST response body content...");

    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);

    AttemptPolicy attempt_policy;
    RequestHeaders headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);

    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    auto body = MakeBody(R"({"model": "gpt-4o", "messages": [{"role": "user", "content": "test"}]})");

    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);

    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }

    REQUIRE(attempt.IsCompleted());
    const auto& res = attempt.GetResponse();
    REQUIRE(res.body_len > 0);

    // Decode body and verify expected JSON fields
    std::string_view body_sv(reinterpret_cast<const char*>(res.body.data()), res.body_len);
    REQUIRE(body_sv.find("\"id\"") != std::string_view::npos);
    REQUIRE(body_sv.find("\"model\"") != std::string_view::npos);
    REQUIRE(body_sv.find("\"choices\"") != std::string_view::npos);
    REQUIRE(body_sv.find("gpt-4o") != std::string_view::npos);

    std::println("  [SUCCESS] POST response body content passed.");
}


int main() {
    try {
        std::println("=== Pure Unit Tests ===");
        test_header_list();
        test_header_list_empty();
        test_response_default_state();
        test_connection_refused();

        std::println("");
        std::println("=== Integration Tests (mock server on port 8080) ===");
        test_sync_post();
        test_sync_post_response_headers();
        test_sync_post_empty_body();
        test_sync_get();
        test_http_429();
        test_timeout();
        test_multiple_concurrent_attempts();
        test_attempt_state_accessors();
        test_http_404();
        test_response_body_content_post();

        std::println("");
        std::println("ALL {} CURL TESTS PASSED", 14);
        return 0;
    } catch (const AnnotatedException& e) {
        std::println("[ERROR] AnnotatedException\n{}", to_string(e));
        return 1;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
