/***
 * Test assertion macros that work in Release builds (where assert() is a no-op).
 *
 * These macros throw on failure, so the top-level catch block in each test's
 * main() can report the failure. This is a lightweight stepping stone toward
 * a full test framework (Catch2, Google Test, etc.).
 */

#pragma once

#include <format>
#include <stdexcept>


#define REQUIRE(expr)                                                          \
    do {                                                                       \
        if (!(expr)) {                                                         \
            throw std::runtime_error(                                          \
                std::format("REQUIRE failed: {} ({}:{})", #expr, __FILE__, __LINE__)); \
        }                                                                      \
    } while (0)


#define REQUIRE_EQ(a, b)                                                       \
    do {                                                                       \
        if (!((a) == (b))) {                                                   \
            throw std::runtime_error(                                          \
                std::format("REQUIRE_EQ failed: {} != {} ({}:{})", #a, #b, __FILE__, __LINE__)); \
        }                                                                      \
    } while (0)
