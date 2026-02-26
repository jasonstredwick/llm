#include <print>
#include <string>

#include "test_assert.hpp"
#include "../../interface/clients/anthropic.hpp"
#include "../../interface/clients/gemini.hpp"
#include "../../interface/clients/openai.hpp"


/***
 * Anthropic ModelGroup Tests
 */

void test_anthropic_opus_new_naming() {
    std::println("Testing Anthropic ModelGroup: opus (new naming)...");
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-opus-4-20250514"), std::string{"opus"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-opus-4.5-20251101"), std::string{"opus"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-opus-4.6-20260101"), std::string{"opus"});
    std::println("  [SUCCESS]");
}


void test_anthropic_sonnet_new_naming() {
    std::println("Testing Anthropic ModelGroup: sonnet (new naming)...");
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-sonnet-4-20250514"), std::string{"sonnet"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-sonnet-4.5-20250929"), std::string{"sonnet"});
    std::println("  [SUCCESS]");
}


void test_anthropic_haiku_new_naming() {
    std::println("Testing Anthropic ModelGroup: haiku (new naming)...");
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-haiku-4-20250514"), std::string{"haiku"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-haiku-4.5-20251001"), std::string{"haiku"});
    std::println("  [SUCCESS]");
}


void test_anthropic_legacy_naming() {
    std::println("Testing Anthropic ModelGroup: legacy naming...");
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-3-5-sonnet-20241022"), std::string{"sonnet"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-3-opus-20240229"), std::string{"opus"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-3-haiku-20240307"), std::string{"haiku"});
    std::println("  [SUCCESS]");
}


void test_anthropic_unknown_model() {
    std::println("Testing Anthropic ModelGroup: unknown model passthrough...");
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("some-unknown-model"), std::string{"some-unknown-model"});
    REQUIRE_EQ(jai::llm::anthropic::ModelGroup("claude-new-family-5"), std::string{"claude-new-family-5"});
    std::println("  [SUCCESS]");
}


/***
 * OpenAI ModelGroup Tests
 */

void test_openai_strip_date() {
    std::println("Testing OpenAI ModelGroup: strip date suffix...");
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4o-2024-08-06"), std::string{"gpt-4o"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("o3-2025-04-16"), std::string{"o3"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4-turbo-2024-04-09"), std::string{"gpt-4-turbo"});
    std::println("  [SUCCESS]");
}


void test_openai_no_date() {
    std::println("Testing OpenAI ModelGroup: no date suffix...");
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4o"), std::string{"gpt-4o"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("o3"), std::string{"o3"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4.1"), std::string{"gpt-4.1"});
    std::println("  [SUCCESS]");
}


void test_openai_variant_preserved() {
    std::println("Testing OpenAI ModelGroup: variants preserved...");
    REQUIRE_EQ(jai::llm::openai::ModelGroup("o3-mini"), std::string{"o3-mini"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4.1-mini"), std::string{"gpt-4.1-mini"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4.1-nano"), std::string{"gpt-4.1-nano"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4o-mini"), std::string{"gpt-4o-mini"});
    std::println("  [SUCCESS]");
}


void test_openai_variant_with_date() {
    std::println("Testing OpenAI ModelGroup: variant + date...");
    REQUIRE_EQ(jai::llm::openai::ModelGroup("gpt-4o-mini-2024-07-18"), std::string{"gpt-4o-mini"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("o3-mini-2025-01-31"), std::string{"o3-mini"});
    std::println("  [SUCCESS]");
}


void test_openai_short_string() {
    std::println("Testing OpenAI ModelGroup: short string (no crash)...");
    REQUIRE_EQ(jai::llm::openai::ModelGroup("o3"), std::string{"o3"});
    REQUIRE_EQ(jai::llm::openai::ModelGroup(""), std::string{""});
    REQUIRE_EQ(jai::llm::openai::ModelGroup("a"), std::string{"a"});
    std::println("  [SUCCESS]");
}


/***
 * Gemini ModelGroup Tests
 */

void test_gemini_preview_with_date() {
    std::println("Testing Gemini ModelGroup: preview with date...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-pro-preview-05-06"), std::string{"gemini-2.5-pro"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-flash-preview-04-17"), std::string{"gemini-2.5-flash"});
    std::println("  [SUCCESS]");
}


void test_gemini_preview_no_date() {
    std::println("Testing Gemini ModelGroup: preview without date...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-pro-preview"), std::string{"gemini-2.5-pro"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-3.1-pro-preview"), std::string{"gemini-3.1-pro"});
    std::println("  [SUCCESS]");
}


void test_gemini_experimental() {
    std::println("Testing Gemini ModelGroup: experimental...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.0-flash-exp-0121"), std::string{"gemini-2.0-flash"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-exp-1206"), std::string{"gemini"});
    std::println("  [SUCCESS]");
}


void test_gemini_point_release() {
    std::println("Testing Gemini ModelGroup: point-release suffix...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-flash-001"), std::string{"gemini-2.5-flash"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-pro-002"), std::string{"gemini-2.5-pro"});
    std::println("  [SUCCESS]");
}


void test_gemini_stable() {
    std::println("Testing Gemini ModelGroup: stable (no suffix)...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-flash"), std::string{"gemini-2.5-flash"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-pro"), std::string{"gemini-2.5-pro"});
    std::println("  [SUCCESS]");
}


void test_gemini_variant_preserved() {
    std::println("Testing Gemini ModelGroup: variants preserved...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.5-flash-lite"), std::string{"gemini-2.5-flash-lite"});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini-2.0-flash-lite"), std::string{"gemini-2.0-flash-lite"});
    std::println("  [SUCCESS]");
}


void test_gemini_empty_and_short() {
    std::println("Testing Gemini ModelGroup: edge cases...");
    REQUIRE_EQ(jai::llm::gemini::ModelGroup(""), std::string{""});
    REQUIRE_EQ(jai::llm::gemini::ModelGroup("gemini"), std::string{"gemini"});
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

    std::println("===== Anthropic ModelGroup Tests =====");
    run(test_anthropic_opus_new_naming);
    run(test_anthropic_sonnet_new_naming);
    run(test_anthropic_haiku_new_naming);
    run(test_anthropic_legacy_naming);
    run(test_anthropic_unknown_model);

    std::println("\n===== OpenAI ModelGroup Tests =====");
    run(test_openai_strip_date);
    run(test_openai_no_date);
    run(test_openai_variant_preserved);
    run(test_openai_variant_with_date);
    run(test_openai_short_string);

    std::println("\n===== Gemini ModelGroup Tests =====");
    run(test_gemini_preview_with_date);
    run(test_gemini_preview_no_date);
    run(test_gemini_experimental);
    run(test_gemini_point_release);
    run(test_gemini_stable);
    run(test_gemini_variant_preserved);
    run(test_gemini_empty_and_short);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
