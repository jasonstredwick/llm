#include "src/http.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

using namespace jai::llm::http;

void test_headers_construction() {
    std::cout << "Testing Headers construction..." << std::endl;
    
    std::vector<std::string> raw_headers = {
        "Content-Type: application/json",
        "Authorization: Bearer token",
        "MalformedHeader" // Should be dropped
    };
    
    Headers headers(raw_headers);
    assert(headers.GetHeaders().size() == 2);
    assert(headers.GetHeaders()[0] == "Content-Type: application/json");
    assert(headers.GetDroppedHeaders().size() == 1);
    assert(headers.GetDroppedHeaders()[0].reason == DroppedHeader::Reason::MissingColon);
    
    std::cout << "[SUCCESS] Headers construction passed." << std::endl;
}

void test_header_validation() {
    std::cout << "Testing header validation..." << std::endl;
    
    std::vector<std::string> raw_headers = {
        "Valid-Header: value",
        "NoColonHeader",
        "Newline\nHeader: value",
        "Header: value\nwith newline"
    };
    
    Headers headers(raw_headers);
    assert(headers.GetHeaders().size() == 1);
    assert(headers.GetDroppedHeaders().size() == 3);
    
    bool missing_colon = false;
    bool contains_newline = false;
    for (const auto& dropped : headers.GetDroppedHeaders()) {
        if (dropped.reason == DroppedHeader::Reason::MissingColon) missing_colon = true;
        if (dropped.reason == DroppedHeader::Reason::ContainsNewline) contains_newline = true;
    }
    assert(missing_colon);
    assert(contains_newline);
    
    std::cout << "[SUCCESS] Header validation passed." << std::endl;
}

void test_default_headers() {
    std::cout << "Testing default headers..." << std::endl;
    
    std::vector<std::string> initial = { "Content-Type: application/json" };
    Headers headers(initial);
    
    std::vector<std::string> defaults = {
        "Content-Type: text/plain", // Should NOT be added
        "X-Custom-Header: value"    // Should be added
    };
    
    headers.AddDefaultHeaders(defaults);
    
    assert(headers.GetHeaders().size() == 2);
    bool found_custom = false;
    bool found_correct_content_type = false;
    
    for (const auto& h : headers.GetHeaders()) {
        if (h == "Content-Type: application/json") found_correct_content_type = true;
        if (h == "X-Custom-Header: value") found_custom = true;
    }
    
    assert(found_correct_content_type);
    assert(found_custom);
    
    std::cout << "[SUCCESS] Default headers passed." << std::endl;
}

int main() {
    try {
        test_headers_construction();
        test_header_validation();
        test_default_headers();
        std::cout << "ALL HTTP UNIT TESTS PASSED" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
