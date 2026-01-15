#include "src/curl.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <chrono>

using namespace jai::llm::curl;
using namespace jai::llm::http;
using namespace jai::llm;

void test_header_list() {
    std::cout << "Testing HeaderList..." << std::endl;
    
    Headers headers(std::vector<std::string>{"Content-Type: application/json", "X-Test: value"});
    HeaderList list(headers);
    assert(list.Get() != nullptr);
    
    std::cout << "[SUCCESS] HeaderList passed." << std::endl;
    std::cout.flush();
}

void test_sync_attempt() {
    std::cout << "Testing Sync Attempt (POST)..." << std::endl;
    ConnectionPolicy conn_policy;
    Interface iface(conn_policy);
    
    AttemptPolicy attempt_policy;
    Headers headers(std::vector<std::string>{"Accept: application/json"});
    HeaderList header_list(headers);
    
    std::string url = "http://127.0.0.1:8080/v1/chat/completions";
    std::string json_body = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"Hello\"}]}";
    std::vector<std::byte> body;
    body.resize(json_body.size());
    std::memcpy(body.data(), json_body.data(), json_body.size());
    
    // Attempt is non-copyable/non-movable, created on stack
    Attempt attempt(iface, attempt_policy, Method::POST, url, header_list, body);
    
    while (!attempt.IsDone()) {
        iface.ExecOnce();
    }
    
    if (attempt.IsFailed()) {
        std::cerr << "Attempt failed: " << attempt.GetErrorMessage() << std::endl;
        std::cerr << "Ensure python tests/mock_server.py 8080 is running." << std::endl;
        assert(!attempt.IsFailed());
    }
    
    assert(attempt.IsCompleted());
    const auto& res = attempt.GetResponse();
    assert(res.status_code == 200 || res.status_code == 404);
    assert(!res.body.empty());
    
    std::cout << "[SUCCESS] Sync Attempt passed." << std::endl;
}

int main() {
    try {
        test_header_list();
        std::cout << "Starting test_sync_attempt()..." << std::endl;
        test_sync_attempt();
        std::cout << "finished test_sync_attempt()..." << std::endl;
        std::cout << "ALL CURL UNIT TESTS PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "CAUGHT EXCEPTION" << std::endl;
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        std::cerr.flush();
        return 1;
    } catch (...) {
        std::cout << "CAUGHT UNKNOWN EXCEPTION" << std::endl;
        return 1;
    }
}
