#include "src/providers/openai/openai_client.hpp"
#include <iostream>
#include <atomic>
#include <thread>

using namespace jai::llm;

int main() {
    auto transport = std::make_shared<HttpRequestManager>();
    auto admission = std::make_shared<AdmissionController>(transport);

    ClientConfig config;
    config.api_key = "sk-test-123";
    config.base_url = "http://127.0.0.1:8080/v1/chat/completions";

    OpenAIClient client(admission, config);

    ChatRequest req;
    req.model = "gpt-4";
    req.messages.push_back({Role::User, "Hello OpenAI!"});

    std::atomic<bool> completed{false};

    std::cout << "--- Testing OpenAIClient Chat ---" << std::endl;
    client.Chat(req, [&](const ChatResponse& res) {
        if (res.success) {
            std::cout << "[SUCCESS] Response: " << res.content << std::endl;
            std::cout << "Model: " << res.model << std::endl;
            std::cout << "Usage: P:" << res.usage_prompt_tokens << " C:" << res.usage_completion_tokens << std::endl;
        } else {
            std::cout << "[FAILURE] Error: " << res.error << std::endl;
        }
        completed = true;
    });

    auto start = std::chrono::steady_clock::now();
    while (!completed) {
        admission->Update(); // Drives the whole stack
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 5) {
            std::cout << "Test timed out." << std::endl;
            return 1;
        }
    }

    return 0;
}
