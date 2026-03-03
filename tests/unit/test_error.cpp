/***
 * Error handling unit tests — AnnotatedException, FatalInstanceError,
 * Context comparison, and to_string formatting.
 *
 * Tests verify that:
 *   - AnnotatedException stores and retrieves error messages
 *   - Context stacking via AddContext works correctly
 *   - Context equality and ordering operators behave as expected
 *   - FatalInstanceError inherits properly from AnnotatedException
 *   - to_string produces expected output for contexts and exceptions
 *   - what() returns the error message
 *   - Copy and move semantics work correctly
 */

#include <print>
#include <stdexcept>
#include <string>

#include "test_assert.hpp"
#include "../../interface/core/error.hpp"


using namespace jai::llm;


/***
 * AnnotatedException Construction Tests
 */

void test_annotated_exception_basic_construction() {
    std::println("Testing AnnotatedException: basic construction...");

    AnnotatedException e{"Something went wrong"};

    REQUIRE_EQ(e.ErrorMsg(), std::string_view{"Something went wrong"});
    REQUIRE_EQ(std::string{e.what()}, std::string{"Something went wrong"});
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(1));
    // The initial context has an empty message (no context_msg_ provided).
    REQUIRE(e.ErrorContext()[0].msg.empty());

    std::println("  [SUCCESS]");
}


void test_annotated_exception_with_context_msg() {
    std::println("Testing AnnotatedException: construction with context message...");

    AnnotatedException e{"Error occurred", "during initialization"};

    REQUIRE_EQ(e.ErrorMsg(), std::string_view{"Error occurred"});
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(1));
    REQUIRE_EQ(e.ErrorContext()[0].msg, std::string{"during initialization"});

    std::println("  [SUCCESS]");
}


void test_annotated_exception_add_context() {
    std::println("Testing AnnotatedException: AddContext stacking...");

    AnnotatedException e{"Base error"};
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(1));

    e.AddContext("layer 1");
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(2));
    REQUIRE_EQ(e.ErrorContext()[1].msg, std::string{"layer 1"});

    e.AddContext("layer 2");
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(3));
    REQUIRE_EQ(e.ErrorContext()[2].msg, std::string{"layer 2"});

    std::println("  [SUCCESS]");
}


void test_annotated_exception_add_context_no_msg() {
    std::println("Testing AnnotatedException: AddContext with no message...");

    AnnotatedException e{"Error"};
    e.AddContext();  // no message

    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(2));
    REQUIRE(e.ErrorContext()[1].msg.empty());

    std::println("  [SUCCESS]");
}


/***
 * AnnotatedException Copy/Move Tests
 */

void test_annotated_exception_copy() {
    std::println("Testing AnnotatedException: copy construction...");

    AnnotatedException original{"Copy test", "context"};
    original.AddContext("extra context");

    AnnotatedException copy = original;

    REQUIRE_EQ(copy.ErrorMsg(), original.ErrorMsg());
    REQUIRE_EQ(copy.ErrorContext().size(), original.ErrorContext().size());
    REQUIRE(copy == original);

    std::println("  [SUCCESS]");
}


void test_annotated_exception_move() {
    std::println("Testing AnnotatedException: move construction...");

    AnnotatedException original{"Move test", "context"};
    original.AddContext("extra context");

    auto msg = std::string{original.ErrorMsg()};
    auto ctx_size = original.ErrorContext().size();

    AnnotatedException moved = std::move(original);

    REQUIRE_EQ(moved.ErrorMsg(), std::string_view{msg});
    REQUIRE_EQ(moved.ErrorContext().size(), ctx_size);

    std::println("  [SUCCESS]");
}


/***
 * AnnotatedException Equality/Ordering Tests
 */

void test_annotated_exception_equality_same() {
    std::println("Testing AnnotatedException: equality (same)...");

    AnnotatedException a{"Same error"};
    AnnotatedException b{a};  // copy

    REQUIRE(a == b);

    std::println("  [SUCCESS]");
}


void test_annotated_exception_inequality_different_msg() {
    std::println("Testing AnnotatedException: inequality (different msg)...");

    AnnotatedException a{"Error A"};
    AnnotatedException b{"Error B"};

    REQUIRE(!(a == b));

    std::println("  [SUCCESS]");
}


/***
 * AnnotatedException Caught as std::exception
 */

void test_annotated_exception_is_std_exception() {
    std::println("Testing AnnotatedException: caught as std::exception...");

    bool caught_as_std = false;
    try {
        throw AnnotatedException{"Test throw"};
    } catch (const std::exception& e) {
        caught_as_std = true;
        REQUIRE_EQ(std::string{e.what()}, std::string{"Test throw"});
    }
    REQUIRE(caught_as_std);

    std::println("  [SUCCESS]");
}


/***
 * Context Tests
 */

void test_context_equality() {
    std::println("Testing Context: equality...");

    // Contexts created at the same source location with the same msg are equal.
    AnnotatedException::Context a{.msg = "test"};
    AnnotatedException::Context b{.msg = "test"};

    REQUIRE(a == b);

    std::println("  [SUCCESS]");
}


void test_context_inequality_different_msg() {
    std::println("Testing Context: inequality (different msg)...");

    AnnotatedException::Context a{.msg = "alpha"};
    AnnotatedException::Context b{.msg = "beta"};

    REQUIRE(!(a == b));

    std::println("  [SUCCESS]");
}


void test_context_ordering() {
    std::println("Testing Context: ordering...");

    AnnotatedException::Context a{.msg = "alpha"};
    AnnotatedException::Context b{.msg = "beta"};

    // "alpha" < "beta" lexicographically, so a < b.
    REQUIRE((a <=> b) < 0);
    REQUIRE((b <=> a) > 0);

    std::println("  [SUCCESS]");
}


/***
 * FatalInstanceError Tests
 */

void test_fatal_instance_error_basic() {
    std::println("Testing FatalInstanceError: basic construction...");

    FatalInstanceError e{"Instance is broken"};

    REQUIRE_EQ(e.ErrorMsg(), std::string_view{"Instance is broken"});
    REQUIRE_EQ(std::string{e.what()}, std::string{"Instance is broken"});
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(1));

    std::println("  [SUCCESS]");
}


void test_fatal_instance_error_with_context() {
    std::println("Testing FatalInstanceError: with context message...");

    FatalInstanceError e{"Curl handle failed", "during event loop"};

    REQUIRE_EQ(e.ErrorMsg(), std::string_view{"Curl handle failed"});
    REQUIRE_EQ(e.ErrorContext().size(), static_cast<size_t>(1));
    REQUIRE_EQ(e.ErrorContext()[0].msg, std::string{"during event loop"});

    std::println("  [SUCCESS]");
}


void test_fatal_instance_error_caught_as_annotated() {
    std::println("Testing FatalInstanceError: caught as AnnotatedException...");

    bool caught_as_annotated = false;
    try {
        throw FatalInstanceError{"Fatal"};
    } catch (const AnnotatedException& e) {
        caught_as_annotated = true;
        REQUIRE_EQ(e.ErrorMsg(), std::string_view{"Fatal"});
    }
    REQUIRE(caught_as_annotated);

    std::println("  [SUCCESS]");
}


void test_fatal_instance_error_caught_as_std_exception() {
    std::println("Testing FatalInstanceError: caught as std::exception...");

    bool caught_as_std = false;
    try {
        throw FatalInstanceError{"Fatal std"};
    } catch (const std::exception& e) {
        caught_as_std = true;
        REQUIRE_EQ(std::string{e.what()}, std::string{"Fatal std"});
    }
    REQUIRE(caught_as_std);

    std::println("  [SUCCESS]");
}


void test_fatal_instance_error_distinguished_from_annotated() {
    std::println("Testing FatalInstanceError: distinguished from AnnotatedException...");

    bool caught_fatal = false;
    bool caught_annotated = false;

    // FatalInstanceError should be caught by the more specific catch first.
    try {
        throw FatalInstanceError{"Fatal"};
    } catch (const FatalInstanceError&) {
        caught_fatal = true;
    } catch (const AnnotatedException&) {
        caught_annotated = true;
    }

    REQUIRE(caught_fatal);
    REQUIRE(!caught_annotated);

    std::println("  [SUCCESS]");
}


/***
 * to_string Tests
 */

void test_to_string_context() {
    std::println("Testing to_string(Context): non-empty msg...");

    AnnotatedException::Context ctx{.msg = "test context"};
    auto str = to_string(ctx);

    // Should contain the context message.
    REQUIRE(str.find("test context") != std::string::npos);

    std::println("  [SUCCESS]");
}


void test_to_string_context_empty_msg() {
    std::println("Testing to_string(Context): empty msg...");

    AnnotatedException::Context ctx{.msg = ""};
    auto str = to_string(ctx);

    // Should still produce output (file/line info) but no prefix line.
    REQUIRE(!str.empty());
    // Should NOT start with a newline (no msg prefix).
    REQUIRE(str.find("    [") != std::string::npos);

    std::println("  [SUCCESS]");
}


void test_to_string_exception() {
    std::println("Testing to_string(AnnotatedException): full format...");

    AnnotatedException e{"Test error", "context info"};
    auto str = to_string(e);

    REQUIRE(str.find("Exception-") != std::string::npos);
    REQUIRE(str.find("Reason: Test error") != std::string::npos);
    REQUIRE(str.find("Context-") != std::string::npos);

    std::println("  [SUCCESS]");
}


void test_to_string_exception_range() {
    std::println("Testing to_string(range of contexts)...");

    AnnotatedException e{"Error"};
    e.AddContext("ctx 1");
    e.AddContext("ctx 2");

    auto contexts = e.ErrorContext();
    auto str = to_string(contexts);

    // Should contain both context messages.
    REQUIRE(str.find("ctx 1") != std::string::npos);
    REQUIRE(str.find("ctx 2") != std::string::npos);

    std::println("  [SUCCESS]");
}


/***
 * NormalizeFileName / NormalizeFunctionName Tests (via to_string indirectly)
 */

void test_normalize_via_to_string() {
    std::println("Testing NormalizeFileName/NormalizeFunctionName: via to_string...");

    // Create an exception at this source location.
    AnnotatedException e{"Normalize test"};
    auto str = to_string(e);

    // The output should contain normalized file/function info.
    // At minimum, it should not crash and should produce non-empty output.
    REQUIRE(!str.empty());
    REQUIRE(str.find("Normalize test") != std::string::npos);

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

    std::println("===== AnnotatedException Construction Tests =====");
    run(test_annotated_exception_basic_construction);
    run(test_annotated_exception_with_context_msg);
    run(test_annotated_exception_add_context);
    run(test_annotated_exception_add_context_no_msg);

    std::println("\n===== AnnotatedException Copy/Move Tests =====");
    run(test_annotated_exception_copy);
    run(test_annotated_exception_move);

    std::println("\n===== AnnotatedException Equality/Ordering Tests =====");
    run(test_annotated_exception_equality_same);
    run(test_annotated_exception_inequality_different_msg);

    std::println("\n===== AnnotatedException as std::exception Tests =====");
    run(test_annotated_exception_is_std_exception);

    std::println("\n===== Context Tests =====");
    run(test_context_equality);
    run(test_context_inequality_different_msg);
    run(test_context_ordering);

    std::println("\n===== FatalInstanceError Tests =====");
    run(test_fatal_instance_error_basic);
    run(test_fatal_instance_error_with_context);
    run(test_fatal_instance_error_caught_as_annotated);
    run(test_fatal_instance_error_caught_as_std_exception);
    run(test_fatal_instance_error_distinguished_from_annotated);

    std::println("\n===== to_string Tests =====");
    run(test_to_string_context);
    run(test_to_string_context_empty_msg);
    run(test_to_string_exception);
    run(test_to_string_exception_range);
    run(test_normalize_via_to_string);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
