#include "src/operation_group.hpp"
#include "src/providers/openai/openai_client.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>

using namespace jai::llm;

void test_operation_group() {
    std::cout << "--- Testing OperationGroup (Barrier) ---" << std::endl;
    auto transport = std::make_shared<HttpRequestManager>();
    auto admission = std::make_shared<AdmissionController>(transport);

    ClientConfig config;
    config.api_key = "sk-test";
    config.base_url = "http://127.0.0.1:8080/v1/chat/completions";
    OpenAIClient client(admission, config);

    std::atomic<bool> all_done{false};
    OperationGroup group([&](const std::vector<ChatResponse>& results) {
        std::cout << "Group Callback Fired with " << results.size() << " results." << std::endl;
        for (const auto& res : results) {
            std::cout << "  Result: " << (res.success ? "Success" : "Fail") << " - " << res.content << std::endl;
        }
        all_done = true;
    });

    ChatRequest req1; req1.model = "model-1"; req1.messages.push_back({Role::User, "Hi 1"});
    ChatRequest req2; req2.model = "model-2"; req2.messages.push_back({Role::User, "Hi 2"});

    group.Add(client, req1);
    group.Add(client, req2);

    auto start = std::chrono::steady_clock::now();
    while (!all_done) {
        admission->Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() > 5) {
            std::cout << "OperationGroup timed out." << std::endl;
            exit(1);
        }
    }
    std::cout << "[SUCCESS] OperationGroup verified." << std::endl;
}

void test_race_operation() {
    std::cout << "--- Testing RaceOperation (First Success) ---" << std::endl;
    auto transport = std::make_shared<HttpRequestManager>();
    auto admission = std::make_shared<AdmissionController>(transport);

    ClientConfig config;
    config.api_key = "sk-test";

    OpenAIClient client(admission, config);

    std::atomic<bool> winner_found{false};
    RaceOperation race([&](const ChatResponse& res) {
        std::cout << "Race Winner: " << res.content << " (Model: " << res.model << ")" << std::endl;
        winner_found = true;
    });

    // Slow request (1 second delay)
    ChatRequest slow_req;
    slow_req.model = "slow-model";
    slow_req.messages.push_back({Role::User, "I am slow"});

    // Fast request (no delay)
    ChatRequest fast_req;
    fast_req.model = "fast-model";
    fast_req.messages.push_back({Role::User, "I am fast"});

    // We need to pass the delay via URL, but AIClient uses config.base_url.
    // Let's just create two clients or re-configure.

    ClientConfig config_slow = config;
    config_slow.base_url = "http://127.0.0.1:8080/v1/chat/completions?delay_ms=1000";
    OpenAIClient client_slow(admission, config_slow);

    ClientConfig config_fast = config;
    config_fast.base_url = "http://127.0.0.1:8080/v1/chat/completions?delay_ms=0";
    OpenAIClient client_fast(admission, config_fast);

    race.Add(client_slow, slow_req);
    race.Add(client_fast, fast_req);

    auto start = std::chrono::steady_clock::now();
    while (!winner_found) {
        admission->Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() > 5) {
            std::cout << "RaceOperation timed out." << std::endl;
            exit(1);
        }
    }
    std::cout << "[SUCCESS] RaceOperation verified." << std::endl;
}

int main() {
    test_operation_group();
    test_race_operation();
    std::cout << "ALL PHASE 4 TESTS PASSED" << std::endl;
    return 0;
}
