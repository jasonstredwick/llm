/***
 * Instance unit tests — lifecycle, singleton enforcement, threading modes,
 * and observability.
 *
 * Tests verify that:
 *   - Default and configured construction work
 *   - Singleton enforcement prevents concurrent instances
 *   - ExecOnce returns 0 when no work is pending
 *   - PendingCount and IsRunning reflect correct state
 *   - TotalUsage starts at zero
 *   - MANUAL mode: Start() throws
 *   - INTERNAL mode: Start()/Stop() lifecycle works
 *   - Stop() is safe to call in MANUAL mode (no-op)
 *   - Destruction cleans up the singleton
 *
 * Note: Tests that exercise real HTTP calls belong in integration tests.
 * These tests verify the Instance shell behavior without network.
 */

#include <print>
#include <stdexcept>
#include <string>

#include "test_assert.hpp"
#include "../../interface/llm.hpp"


using namespace jai::llm;


/***
 * Construction Tests
 */

void test_instance_default_construction() {
    std::println("Testing Instance: default construction...");

    Instance inst;
    // Should construct without throwing.
    REQUIRE_EQ(inst.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


void test_instance_configured_construction_manual() {
    std::println("Testing Instance: configured construction (MANUAL)...");

    Instance::Config config{
        .threading = Instance::ThreadingMode::MANUAL,
        .policy = {}
    };

    Instance inst(config);
    REQUIRE_EQ(inst.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


void test_instance_configured_construction_internal() {
    std::println("Testing Instance: configured construction (INTERNAL)...");

    Instance::Config config{
        .threading = Instance::ThreadingMode::INTERNAL,
        .policy = {}
    };

    Instance inst(config);
    REQUIRE_EQ(inst.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


/***
 * Singleton Enforcement Tests
 */

void test_instance_singleton_enforcement() {
    std::println("Testing Instance: singleton — second instance throws...");

    Instance first;

    bool threw = false;
    try {
        Instance second;
    } catch (const AnnotatedException&) {
        threw = true;
    } catch (const std::exception&) {
        threw = true;  // might throw a different exception type
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_instance_singleton_reuse_after_destruction() {
    std::println("Testing Instance: singleton — new instance after destruction...");

    {
        Instance first;
        REQUIRE_EQ(first.PendingCount(), static_cast<size_t>(0));
    }
    // first is destroyed here

    // Should be able to construct another
    Instance second;
    REQUIRE_EQ(second.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


/***
 * ExecOnce Tests
 */

void test_instance_exec_once_no_work() {
    std::println("Testing Instance: ExecOnce with no work returns 0...");

    Instance inst;
    size_t result = inst.ExecOnce();
    REQUIRE_EQ(result, static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


void test_instance_exec_once_multiple_calls() {
    std::println("Testing Instance: multiple ExecOnce calls are idempotent with no work...");

    Instance inst;

    for (int i = 0; i < 10; ++i) {
        size_t result = inst.ExecOnce();
        REQUIRE_EQ(result, static_cast<size_t>(0));
    }

    std::println("  [SUCCESS]");
}


/***
 * Observability Tests
 */

void test_instance_pending_count_initial() {
    std::println("Testing Instance: PendingCount starts at 0...");

    Instance inst;
    REQUIRE_EQ(inst.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


void test_instance_is_running_manual() {
    std::println("Testing Instance: IsRunning in MANUAL mode...");

    Instance inst;
    // In MANUAL mode, IsRunning depends on whether there's active work.
    // With no work, it should not be running.
    REQUIRE_EQ(inst.IsRunning(), false);

    std::println("  [SUCCESS]");
}


void test_instance_total_usage_initial() {
    std::println("Testing Instance: TotalUsage starts at zero...");

    Instance inst;
    auto usage = inst.TotalUsage();

    REQUIRE(!usage.input_tokens.has_value());
    REQUIRE(!usage.output_tokens.has_value());
    REQUIRE(!usage.total_tokens.has_value());
    REQUIRE(!usage.cache_creation_tokens.has_value());
    REQUIRE(!usage.cache_read_tokens.has_value());
    REQUIRE(!usage.reasoning_tokens.has_value());
    REQUIRE(!usage.tool_use_tokens.has_value());

    std::println("  [SUCCESS]");
}


/***
 * Threading Mode Tests
 */

void test_instance_manual_start_throws() {
    std::println("Testing Instance: Start() in MANUAL mode throws...");

    Instance::Config config{
        .threading = Instance::ThreadingMode::MANUAL,
        .policy = {}
    };
    Instance inst(config);

    bool threw = false;
    try {
        inst.Start();
    } catch (const AnnotatedException&) {
        threw = true;
    } catch (const std::exception&) {
        threw = true;
    }
    REQUIRE(threw);

    std::println("  [SUCCESS]");
}


void test_instance_manual_stop_noop() {
    std::println("Testing Instance: Stop() in MANUAL mode is no-op...");

    Instance inst;
    // Should not throw or hang.
    inst.Stop();

    std::println("  [SUCCESS]");
}


void test_instance_internal_start_stop() {
    std::println("Testing Instance: INTERNAL mode Start()/Stop()...");

    Instance::Config config{
        .threading = Instance::ThreadingMode::INTERNAL,
        .policy = {}
    };
    Instance inst(config);

    inst.Start();
    REQUIRE_EQ(inst.IsRunning(), true);

    inst.Stop();
    REQUIRE_EQ(inst.IsRunning(), false);

    std::println("  [SUCCESS]");
}


void test_instance_internal_double_stop() {
    std::println("Testing Instance: INTERNAL mode double Stop() is safe...");

    Instance::Config config{
        .threading = Instance::ThreadingMode::INTERNAL,
        .policy = {}
    };
    Instance inst(config);

    inst.Start();
    inst.Stop();
    inst.Stop();  // second stop should be safe

    REQUIRE_EQ(inst.IsRunning(), false);

    std::println("  [SUCCESS]");
}


/***
 * Policy Passthrough Tests
 */

void test_instance_custom_policy() {
    std::println("Testing Instance: custom policy construction...");

    using namespace std::chrono_literals;

    Instance::Config config{
        .threading = Instance::ThreadingMode::MANUAL,
        .policy = {
            .connection_policy = {
                .max_total_connections = 32
            },
            .attempt_policy = {
                .connect_timeout = 5000ms,
                .total_timeout = 30000ms,
                .verify_peer = true
            },
            .retry_policy = {
                .max_retries = 3
            },
            .rate_limit_policy = {
                .initial_max_concurrent = 8
            }
        }
    };

    Instance inst(config);
    // Verify construction succeeds with non-trivial policy.
    REQUIRE_EQ(inst.PendingCount(), static_cast<size_t>(0));

    std::println("  [SUCCESS]");
}


/***
 * Main
 *
 * IMPORTANT: Because Instance is a singleton, each test that creates an
 * Instance must let it go out of scope before the next test. The tests
 * are structured as individual functions, and the singleton is destroyed
 * at the end of each function. The run() lambda ensures no leaks even
 * on exception.
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

    std::println("===== Instance Construction Tests =====");
    run(test_instance_default_construction);
    run(test_instance_configured_construction_manual);
    run(test_instance_configured_construction_internal);

    std::println("\n===== Singleton Enforcement Tests =====");
    run(test_instance_singleton_enforcement);
    run(test_instance_singleton_reuse_after_destruction);

    std::println("\n===== ExecOnce Tests =====");
    run(test_instance_exec_once_no_work);
    run(test_instance_exec_once_multiple_calls);

    std::println("\n===== Observability Tests =====");
    run(test_instance_pending_count_initial);
    run(test_instance_is_running_manual);
    run(test_instance_total_usage_initial);

    std::println("\n===== Threading Mode Tests =====");
    run(test_instance_manual_start_throws);
    run(test_instance_manual_stop_noop);
    run(test_instance_internal_start_stop);
    run(test_instance_internal_double_stop);

    std::println("\n===== Policy Passthrough Tests =====");
    run(test_instance_custom_policy);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
