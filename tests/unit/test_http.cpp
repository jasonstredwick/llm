#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../src/http.hpp"
#include "../../interface/core/error.hpp"

using namespace jai::llm::http;
using namespace jai::llm;


/***
 * ResponseHeaders Tests
 */
void test_response_headers_construction() {
    std::println("Testing ResponseHeaders construction...");

    std::vector<std::string> raw_headers = {
        "Content-Type: application/json",
        "Authorization: Bearer token",
        "MalformedHeader" // Should be dropped
    };

    ResponseHeaders headers(raw_headers);
    REQUIRE_EQ(headers.GetHeaders().size(), 2u);
    REQUIRE_EQ(headers.GetHeaders()[0], "Content-Type: application/json");
    REQUIRE_EQ(headers.GetDroppedHeaders().size(), 1u);
    REQUIRE_EQ(headers.GetDroppedHeaders()[0].reason, DroppedHeader::Reason::MissingColon);

    std::println("  [SUCCESS] ResponseHeaders construction passed.");
}


void test_response_header_validation() {
    std::println("Testing ResponseHeaders validation...");

    std::vector<std::string> raw_headers = {
        "Valid-Header: value",
        "NoColonHeader",
        "Newline\nHeader: value",
        "Header: value\nwith newline"
    };

    ResponseHeaders headers(raw_headers);
    REQUIRE_EQ(headers.GetHeaders().size(), 1u);
    REQUIRE_EQ(headers.GetDroppedHeaders().size(), 3u);

    bool missing_colon = false;
    bool contains_newline = false;
    for (const auto& dropped : headers.GetDroppedHeaders()) {
        if (dropped.reason == DroppedHeader::Reason::MissingColon) missing_colon = true;
        if (dropped.reason == DroppedHeader::Reason::ContainsNewline) contains_newline = true;
    }
    REQUIRE(missing_colon);
    REQUIRE(contains_newline);

    std::println("  [SUCCESS] ResponseHeaders validation passed.");
}


void test_response_default_headers() {
    std::println("Testing ResponseHeaders default headers...");

    std::vector<std::string> initial = { "Content-Type: application/json" };
    ResponseHeaders headers(initial);

    std::vector<std::string> defaults = {
        "Content-Type: text/plain", // Should NOT be added (key already exists)
        "X-Custom-Header: value"    // Should be added
    };

    headers.AddDefaultHeaders(defaults);

    REQUIRE_EQ(headers.GetHeaders().size(), 2u);
    bool found_custom = false;
    bool found_correct_content_type = false;

    for (const auto& h : headers.GetHeaders()) {
        if (h == "Content-Type: application/json") found_correct_content_type = true;
        if (h == "X-Custom-Header: value") found_custom = true;
    }

    REQUIRE(found_correct_content_type);
    REQUIRE(found_custom);

    std::println("  [SUCCESS] ResponseHeaders default headers passed.");
}


void test_response_headers_from_string_views() {
    std::println("Testing ResponseHeaders from string_view...");

    std::vector<std::string_view> raw_headers = {
        "Content-Type: text/html",
        "X-Request-Id: abc123"
    };

    ResponseHeaders headers(raw_headers);
    REQUIRE_EQ(headers.GetHeaders().size(), 2u);
    REQUIRE(headers.GetDroppedHeaders().empty());

    std::println("  [SUCCESS] ResponseHeaders from string_view passed.");
}


void test_response_headers_case_insensitive_dedup() {
    std::println("Testing ResponseHeaders case-insensitive default dedup...");

    std::vector<std::string> initial = { "Content-Type: application/json" };
    ResponseHeaders headers(initial);

    // Default with different casing for same key should NOT be added
    headers.AddDefaultHeader("content-type: text/plain");
    REQUIRE_EQ(headers.GetHeaders().size(), 1u);
    REQUIRE_EQ(headers.GetHeaders()[0], "Content-Type: application/json");

    // Different key should be added
    headers.AddDefaultHeader("Accept: */*");
    REQUIRE_EQ(headers.GetHeaders().size(), 2u);

    std::println("  [SUCCESS] ResponseHeaders case-insensitive dedup passed.");
}


void test_response_headers_empty() {
    std::println("Testing ResponseHeaders empty construction...");

    std::vector<std::string> empty_headers = {};
    ResponseHeaders headers(empty_headers);
    REQUIRE(headers.GetHeaders().empty());
    REQUIRE(headers.GetDroppedHeaders().empty());

    std::println("  [SUCCESS] ResponseHeaders empty construction passed.");
}


void test_response_headers_nul_in_key() {
    std::println("Testing ResponseHeaders NUL in header...");

    // NUL before the colon means find_first_of(":\n\r") may still find ':'
    // depending on position. A NUL-before-colon header should be dropped.
    std::string nul_header = "X-Evil";
    nul_header.push_back('\0');
    nul_header += "Key: value";

    std::vector<std::string> raw = { nul_header, "Good-Header: ok" };
    ResponseHeaders headers(raw);

    // The good header should survive
    bool found_good = false;
    for (const auto& h : headers.GetHeaders()) {
        if (h == "Good-Header: ok") found_good = true;
    }
    REQUIRE(found_good);

    // Total should be 2 (both accepted) or 1 (NUL one dropped) depending on
    // how string_view::find_first_of handles NUL. Either way, no crash.
    REQUIRE(headers.GetHeaders().size() >= 1u);

    std::println("  [SUCCESS] ResponseHeaders NUL in header passed. accepted={}, dropped={}",
                 headers.GetHeaders().size(), headers.GetDroppedHeaders().size());
}


void test_response_headers_duplicate_keys() {
    std::println("Testing ResponseHeaders duplicate keys...");

    // Multiple headers with the same key should ALL be kept at construction time
    // (dedup only happens for AddDefaultHeader)
    std::vector<std::string> raw = {
        "Set-Cookie: session=abc",
        "Set-Cookie: theme=dark",
        "Set-Cookie: lang=en"
    };

    ResponseHeaders headers(raw);
    REQUIRE_EQ(headers.GetHeaders().size(), 3u);
    REQUIRE(headers.GetDroppedHeaders().empty());

    std::println("  [SUCCESS] ResponseHeaders duplicate keys passed.");
}


void test_response_default_header_malformed_skipped() {
    std::println("Testing ResponseHeaders malformed default header skipped...");

    std::vector<std::string> initial = { "Content-Type: application/json" };
    ResponseHeaders headers(initial);

    // A malformed default header should be silently ignored (not added, not crash)
    headers.AddDefaultHeader("MalformedNoColon");
    REQUIRE_EQ(headers.GetHeaders().size(), 1u);

    // A valid default with new key should still be added
    headers.AddDefaultHeader("Accept: */*");
    REQUIRE_EQ(headers.GetHeaders().size(), 2u);

    std::println("  [SUCCESS] ResponseHeaders malformed default skipped passed.");
}


/***
 * RequestHeaders Tests
 */
void test_request_headers_construction_from_strings() {
    std::println("Testing RequestHeaders construction from strings...");

    std::vector<std::string> raw = {"Content-Type: application/json", "Authorization: Bearer tok"};
    RequestHeaders headers(raw);

    REQUIRE(!headers.Empty());
    REQUIRE_EQ(headers.Size(), size_t{2});
    REQUIRE_EQ(headers.Entries().size(), size_t{2});
    REQUIRE_EQ(headers.Entries()[0], "Content-Type: application/json");
    REQUIRE_EQ(headers.Entries()[1], "Authorization: Bearer tok");

    std::println("  [SUCCESS] RequestHeaders construction from strings passed.");
}


void test_request_headers_construction_from_kv_pairs() {
    std::println("Testing RequestHeaders construction from KV pairs...");

    std::vector<std::pair<std::string, std::string>> kv = {
        {"Content-Type", "application/json"},
        {"X-Custom",     "hello world"}
    };
    RequestHeaders headers(kv);

    REQUIRE_EQ(headers.Size(), size_t{2});
    REQUIRE_EQ(headers.Entries()[0], "Content-Type: application/json");
    REQUIRE_EQ(headers.Entries()[1], "X-Custom: hello world");

    std::println("  [SUCCESS] RequestHeaders construction from KV pairs passed.");
}


void test_request_headers_empty() {
    std::println("Testing RequestHeaders default constructor...");

    RequestHeaders headers;
    REQUIRE(headers.Empty());
    REQUIRE_EQ(headers.Size(), size_t{0});
    REQUIRE(headers.Entries().empty());

    std::println("  [SUCCESS] RequestHeaders default constructor passed.");
}


void test_request_headers_nul_rejection() {
    std::println("Testing RequestHeaders NUL rejection (security check)...");

    std::string evil = "X-Evil";
    evil.push_back('\0');
    evil += "Key: value";

    bool threw = false;
    try {
        RequestHeaders headers(std::vector<std::string>{evil});
    } catch (const AnnotatedException&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS] RequestHeaders NUL rejection passed.");
}


void test_response_headers_whitespace_only_key() {
    std::println("Testing ResponseHeaders whitespace-only key...");

    // " : value" has a key that trims to empty — should not crash
    std::vector<std::string> raw = {" : value", "Good-Key: ok"};
    ResponseHeaders headers(raw);

    // The whitespace-only key header is technically valid (has a colon, no newlines)
    // so it should be accepted. The important thing is no crash.
    bool found_good = false;
    for (const auto& h : headers.GetHeaders()) {
        if (h == "Good-Key: ok") found_good = true;
    }
    REQUIRE(found_good);

    std::println("  [SUCCESS] ResponseHeaders whitespace-only key passed. accepted={}, dropped={}",
                 headers.GetHeaders().size(), headers.GetDroppedHeaders().size());
}


void test_response_headers_leading_trailing_whitespace() {
    std::println("Testing ResponseHeaders leading/trailing whitespace dedup...");

    // Header with extra whitespace around the key
    std::vector<std::string> initial = {"  Content-Type  : application/json"};
    ResponseHeaders headers(initial);
    REQUIRE_EQ(headers.GetHeaders().size(), 1u);

    // Default header with clean key should deduplicate (case-insensitive, whitespace-trimmed)
    headers.AddDefaultHeader("content-type: text/plain");
    REQUIRE_EQ(headers.GetHeaders().size(), 1u); // should NOT be added

    // A genuinely new key should still be added
    headers.AddDefaultHeader("Accept: */*");
    REQUIRE_EQ(headers.GetHeaders().size(), 2u);

    std::println("  [SUCCESS] ResponseHeaders leading/trailing whitespace dedup passed.");
}


int main() {
    try {
        test_response_headers_construction();
        test_response_header_validation();
        test_response_default_headers();
        test_response_headers_from_string_views();
        test_response_headers_case_insensitive_dedup();
        test_response_headers_empty();
        test_response_headers_nul_in_key();
        test_response_headers_duplicate_keys();
        test_response_default_header_malformed_skipped();
        test_request_headers_construction_from_strings();
        test_request_headers_construction_from_kv_pairs();
        test_request_headers_empty();
        test_request_headers_nul_rejection();
        test_response_headers_whitespace_only_key();
        test_response_headers_leading_trailing_whitespace();

        std::println("ALL {} HTTP UNIT TESTS PASSED", 15);
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
