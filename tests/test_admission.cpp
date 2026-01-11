#include "src/admission_controller.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

using namespace jai::llm;

void test_retries() {
    std::cout << "--- Testing Retries (429 -> 200) ---" << std::endl;
    auto transport = std::make_shared<HttpRequestManager>();
    AdmissionController controller(transport);

    Policy policy;
    policy.retry.max_attempts = 2;
    policy.retry.initial_backoff = std::chrono::milliseconds(100);

    HttpRequest req;
    req.url = "http://127.0.0.1:8080/simulate_429"; // This server should return 429

    std::atomic<int> completions{0};
    int attempts_seen = 0;

    req.on_complete = [&](const HttpResponse& res) {
        std::cout << "Final Completion Status: " << res.status_code << std::endl;
        completions++;
    };

    controller.Submit(std::move(req), std::move(policy));

    auto start = std::chrono::steady_clock::now();
    while (completions == 0) {
        controller.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 5) {
            std::cout << "[FAILURE] Retry test timed out." << std::endl;
            exit(1);
        }
    }
    std::cout << "[SUCCESS] Retry test completed." << std::endl;
}

void test_jitter() {
    std::cout << "--- Testing Jitter Delay ---" << std::endl;
    auto transport = std::make_shared<HttpRequestManager>();
    AdmissionController controller(transport);

    Policy policy;
    policy.jitter.min_delay = std::chrono::milliseconds(500);
    policy.jitter.max_delay = std::chrono::milliseconds(500); // Fixed 500ms jitter

    HttpRequest req;
    req.url = "http://127.0.0.1:8080/stream";

    std::atomic<bool> completed{false};
    auto start = std::chrono::steady_clock::now();

    req.on_complete = [&](const HttpResponse&) { completed = true; };

    controller.Submit(std::move(req), std::move(policy));

    while (!completed) {
        controller.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Elapsed time with jitter: " << elapsed_ms << "ms" << std::endl;
    if (elapsed_ms >= 500) {
        std::cout << "[SUCCESS] Jitter enforced delay." << std::endl;
    } else {
        std::cout << "[FAILURE] Jitter did not delay request enough: " << elapsed_ms << "ms" << std::endl;
        exit(1);
    }
}

int main() {
    // Note: mock_server.py must be running
    try {
        test_retries();
        test_jitter();
        std::cout << "ALL PHASE 2 TESTS PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
