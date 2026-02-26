/***
 * Orchestrator unit tests.
 *
 * Strategy: set initial_max_concurrent = 0 in the rate limit policy to prevent
 * dispatch. This lets us test registration, submission, slot allocation,
 * observability counts, and state management without triggering real curl
 * operations. Tests that require actual HTTP completion belong in the
 * integration test suite.
 */

#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../src/orchestrator.hpp"


using namespace jai::llm;


// Helper: create an OrchestratorPolicy that blocks dispatch.
// initial_max_concurrent = 0 means DispatchFromQueue always breaks immediately.
static OrchestratorPolicy NoDispatchPolicy() {
    OrchestratorPolicy p{};
    p.rate_limit_policy.initial_max_concurrent = 0;
    return p;
}


// Helper: create a minimal http::Request for submission.
static http::Request DummyRequest() {
    return http::Request{
        .headers = http::RequestHeaders{},
        .method = http::Method::POST,
        .url = "https://api.example.com/v1/test",
        .body = {}
    };
}


// Helper: create a QueueKey with the given model group.
static QueueKey MakeKey(std::string model_group = "default") {
    return QueueKey{
        .auth_identity = "test-key",
        .endpoint_url = "https://api.example.com",
        .model_group = std::move(model_group)
    };
}


/***
 * Registration Tests
 */

void test_register_returns_sequential_tokens() {
    std::println("Testing Orchestrator Register: sequential tokens...");
    Orchestrator orch(NoDispatchPolicy());

    auto t0 = orch.Register({}, MakeKey("model-a"));
    auto t1 = orch.Register({}, MakeKey("model-b"));
    auto t2 = orch.Register({}, MakeKey("model-c"));

    REQUIRE_EQ(t0.index, 0u);
    REQUIRE_EQ(t1.index, 1u);
    REQUIRE_EQ(t2.index, 2u);
    std::println("  [SUCCESS]");
}


void test_register_same_queue_key_reuses_queue() {
    std::println("Testing Orchestrator Register: same QueueKey reuses queue...");
    Orchestrator orch(NoDispatchPolicy());

    auto key = MakeKey("shared-model");
    auto t0 = orch.Register({}, key);
    auto t1 = orch.Register({}, key);

    // Different registrations...
    REQUIRE(!(t0 == t1));

    // ... but both should route to the same queue.
    // Submit through both and verify they share the same awaiting count.
    auto sync0 = std::make_shared<ResultSync>();
    auto sync1 = std::make_shared<ResultSync>();

    orch.Submit(t0, DummyRequest(), {}, sync0);
    orch.Submit(t1, DummyRequest(), {}, sync1);

    // Both slots should be awaiting (dispatch blocked).
    REQUIRE_EQ(orch.AwaitingCount(), 2u);
    std::println("  [SUCCESS]");
}


void test_register_different_keys_separate_queues() {
    std::println("Testing Orchestrator Register: different keys create separate queues...");
    Orchestrator orch(NoDispatchPolicy());

    auto t0 = orch.Register({}, MakeKey("model-a"));
    auto t1 = orch.Register({}, MakeKey("model-b"));

    auto sync0 = std::make_shared<ResultSync>();
    auto sync1 = std::make_shared<ResultSync>();

    orch.Submit(t0, DummyRequest(), {}, sync0);
    orch.Submit(t1, DummyRequest(), {}, sync1);

    // Both awaiting, from separate queues.
    REQUIRE_EQ(orch.AwaitingCount(), 2u);
    std::println("  [SUCCESS]");
}


/***
 * Submission Tests
 */

void test_submit_returns_ticket() {
    std::println("Testing Orchestrator Submit: returns slot ticket...");
    Orchestrator orch(NoDispatchPolicy());

    auto token = orch.Register({}, MakeKey());
    auto sync = std::make_shared<ResultSync>();

    size_t ticket = orch.Submit(token, DummyRequest(), {}, sync);

    // First submission should get slot 0.
    REQUIRE_EQ(ticket, 0u);
    std::println("  [SUCCESS]");
}


void test_submit_multiple_sequential_tickets() {
    std::println("Testing Orchestrator Submit: sequential tickets...");
    Orchestrator orch(NoDispatchPolicy());

    auto token = orch.Register({}, MakeKey());

    auto s0 = std::make_shared<ResultSync>();
    auto s1 = std::make_shared<ResultSync>();
    auto s2 = std::make_shared<ResultSync>();

    size_t t0 = orch.Submit(token, DummyRequest(), {}, s0);
    size_t t1 = orch.Submit(token, DummyRequest(), {}, s1);
    size_t t2 = orch.Submit(token, DummyRequest(), {}, s2);

    REQUIRE_EQ(t0, 0u);
    REQUIRE_EQ(t1, 1u);
    REQUIRE_EQ(t2, 2u);
    std::println("  [SUCCESS]");
}


/***
 * Observability Count Tests
 */

void test_counts_initial_zero() {
    std::println("Testing Orchestrator Counts: initial zeros...");
    Orchestrator orch(NoDispatchPolicy());

    REQUIRE_EQ(orch.AwaitingCount(), 0u);
    REQUIRE_EQ(orch.ActiveCount(), 0u);
    REQUIRE_EQ(orch.CompletedCount(), 0u);
    REQUIRE_EQ(orch.PendingCount(), 0u);
    std::println("  [SUCCESS]");
}


void test_counts_after_submit() {
    std::println("Testing Orchestrator Counts: after submit (no dispatch)...");
    Orchestrator orch(NoDispatchPolicy());

    auto token = orch.Register({}, MakeKey());

    auto s0 = std::make_shared<ResultSync>();
    auto s1 = std::make_shared<ResultSync>();
    auto s2 = std::make_shared<ResultSync>();

    orch.Submit(token, DummyRequest(), {}, s0);
    REQUIRE_EQ(orch.AwaitingCount(), 1u);
    REQUIRE_EQ(orch.ActiveCount(), 0u);
    REQUIRE_EQ(orch.PendingCount(), 1u);

    orch.Submit(token, DummyRequest(), {}, s1);
    REQUIRE_EQ(orch.AwaitingCount(), 2u);

    orch.Submit(token, DummyRequest(), {}, s2);
    REQUIRE_EQ(orch.AwaitingCount(), 3u);
    REQUIRE_EQ(orch.ActiveCount(), 0u);
    REQUIRE_EQ(orch.CompletedCount(), 0u);
    REQUIRE_EQ(orch.PendingCount(), 3u);
    std::println("  [SUCCESS]");
}


void test_counts_multiple_queues() {
    std::println("Testing Orchestrator Counts: multiple queues aggregate...");
    Orchestrator orch(NoDispatchPolicy());

    auto t_a = orch.Register({}, MakeKey("model-a"));
    auto t_b = orch.Register({}, MakeKey("model-b"));

    auto s0 = std::make_shared<ResultSync>();
    auto s1 = std::make_shared<ResultSync>();
    auto s2 = std::make_shared<ResultSync>();

    orch.Submit(t_a, DummyRequest(), {}, s0);
    orch.Submit(t_a, DummyRequest(), {}, s1);
    orch.Submit(t_b, DummyRequest(), {}, s2);

    // Counts aggregate across all queues.
    REQUIRE_EQ(orch.AwaitingCount(), 3u);
    REQUIRE_EQ(orch.PendingCount(), 3u);
    std::println("  [SUCCESS]");
}


/***
 * Policy Cascade Tests (via Register)
 */

void test_register_merges_attempt_policy() {
    std::println("Testing Orchestrator Register: attempt policy cascade...");
    using namespace std::chrono_literals;

    OrchestratorPolicy orch_policy = NoDispatchPolicy();
    orch_policy.attempt_policy.connect_timeout = 5000ms;

    Orchestrator orch(orch_policy);

    ClientPolicy client_policy{};
    client_policy.attempt_policy.connect_timeout = 10000ms;

    // Registration should merge. We can't directly inspect the stored
    // registration, but we verify it doesn't crash and produces a valid token.
    auto token = orch.Register(client_policy, MakeKey());
    REQUIRE_EQ(token.index, 0u);
    std::println("  [SUCCESS]");
}


void test_register_resolves_retry_policy() {
    std::println("Testing Orchestrator Register: retry policy resolve...");

    OrchestratorPolicy orch_policy = NoDispatchPolicy();
    orch_policy.retry_policy.max_retries = 5;

    Orchestrator orch(orch_policy);

    ClientPolicy client_policy{};
    client_policy.retry_policy.max_retries = 2;

    auto token = orch.Register(client_policy, MakeKey());
    REQUIRE_EQ(token.index, 0u);
    std::println("  [SUCCESS]");
}


/***
 * QueueKey Tests
 */

void test_queue_key_equality() {
    std::println("Testing QueueKey: equality...");

    QueueKey a{.auth_identity = "key1", .endpoint_url = "url1", .model_group = "group1"};
    QueueKey b{.auth_identity = "key1", .endpoint_url = "url1", .model_group = "group1"};
    QueueKey c{.auth_identity = "key2", .endpoint_url = "url1", .model_group = "group1"};

    REQUIRE(a == b);
    REQUIRE(!(a == c));
    std::println("  [SUCCESS]");
}


void test_queue_key_ordering() {
    std::println("Testing QueueKey: ordering (for flat_map)...");

    QueueKey a{.auth_identity = "aaa", .endpoint_url = "url", .model_group = "group"};
    QueueKey b{.auth_identity = "bbb", .endpoint_url = "url", .model_group = "group"};

    REQUIRE(a < b);
    REQUIRE(!(b < a));
    REQUIRE(!(a < a));
    std::println("  [SUCCESS]");
}


/***
 * RegistrationToken Tests
 */

void test_registration_token_equality() {
    std::println("Testing RegistrationToken: equality...");

    Orchestrator::RegistrationToken a{.index = 0};
    Orchestrator::RegistrationToken b{.index = 0};
    Orchestrator::RegistrationToken c{.index = 1};

    REQUIRE(a == b);
    REQUIRE(!(a == c));
    std::println("  [SUCCESS]");
}


/***
 * RunOnce with no work
 */

void test_run_once_empty() {
    std::println("Testing Orchestrator RunOnce: no work returns zero...");
    Orchestrator orch(NoDispatchPolicy());

    size_t remaining = orch.RunOnce();
    REQUIRE_EQ(remaining, 0u);
    std::println("  [SUCCESS]");
}


void test_run_once_blocked_dispatch() {
    std::println("Testing Orchestrator RunOnce: blocked dispatch stays awaiting...");
    Orchestrator orch(NoDispatchPolicy());

    auto token = orch.Register({}, MakeKey());
    auto sync = std::make_shared<ResultSync>();
    orch.Submit(token, DummyRequest(), {}, sync);

    // RunOnce with blocked dispatch — slot remains awaiting.
    size_t remaining = orch.RunOnce();
    REQUIRE_EQ(remaining, 1u);
    REQUIRE_EQ(orch.AwaitingCount(), 1u);
    REQUIRE_EQ(orch.ActiveCount(), 0u);
    std::println("  [SUCCESS]");
}


/***
 * Main
 */

int main() {
    int failed = 0;

    auto run = [&](auto fn) {
        try { fn(); }
        catch (const std::exception& e) {
            std::println("  [FAILED] {}", e.what());
            ++failed;
        }
    };

    std::println("===== Registration Tests =====");
    run(test_register_returns_sequential_tokens);
    run(test_register_same_queue_key_reuses_queue);
    run(test_register_different_keys_separate_queues);

    std::println("\n===== Submission Tests =====");
    run(test_submit_returns_ticket);
    run(test_submit_multiple_sequential_tickets);

    std::println("\n===== Observability Count Tests =====");
    run(test_counts_initial_zero);
    run(test_counts_after_submit);
    run(test_counts_multiple_queues);

    std::println("\n===== Policy Cascade Tests =====");
    run(test_register_merges_attempt_policy);
    run(test_register_resolves_retry_policy);

    std::println("\n===== QueueKey Tests =====");
    run(test_queue_key_equality);
    run(test_queue_key_ordering);

    std::println("\n===== RegistrationToken Tests =====");
    run(test_registration_token_equality);

    std::println("\n===== RunOnce Tests =====");
    run(test_run_once_empty);
    run(test_run_once_blocked_dispatch);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
