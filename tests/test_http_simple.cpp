#include <print>
#include <thread>
#include <chrono>
#include "../src/http_manager.hpp"

int main() {
    std::print("Starting HTTP Manager Test...\n");

    jai::llm::HttpRequestManager manager;
    bool complete = false;

    // Create a request
    jai::llm::HttpRequest req;
    req.url = "https://www.google.com";
    req.method = "GET";
    req.on_complete = [&](const jai::llm::HttpResponse& res) {
        std::print("Request Complete!\n");
        std::print("Status Code: {}\n", res.status_code);
        std::print("Body Size: {} bytes\n", res.body.size());
        if (!res.error_message.empty()) {
            std::print("Error: {}\n", res.error_message);
        }
        complete = true;
    };

    manager.Enqueue(std::move(req));
    std::print("Request Enqueued. Entering Event Loop...\n");

    // Simple Event Loop
    while (!complete) {
        size_t active = manager.Update();
        // std::print("Active Requests: {}\n", active); // functional logging
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::print("Test Finished.\n");
    return 0;
}
