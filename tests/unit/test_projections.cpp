/***
 * Projection unit tests — Generate and Extract for proj::text.
 *
 * Tests verify that:
 *   - Generate produces correct provider-specific requests from projection inputs
 *   - Extract produces correct proj::text::Result from provider responses
 *   - Text is extracted from the correct content blocks per provider
 *   - Empty/missing content is handled gracefully (yields empty text)
 *
 * These tests exercise Generate and Extract directly (no network needed).
 *
 * Note: diagnostics, usage, and metadata are now captured by the core library
 * in AttemptMetadata — the text projection's Extract only returns text.
 */

#include <print>
#include <string>
#include <vector>

#include "test_assert.hpp"
#include "../../interface/projections/text.hpp"
#include "../../interface/endpoints/anthropic_messages.hpp"
#include "../../interface/endpoints/gemini_generate_content.hpp"
#include "../../interface/endpoints/openai_responses.hpp"


using namespace jai::llm;
namespace text = jai::llm::proj::text;


// ===== Generate tests =====


void test_generate_anthropic() {
    std::println("Testing Generate<anthropic::Messages>: basic text prompt...");

    auto request = text::Generate<anthropic::Messages>(
        text::Prompt{.text = "You are a helpful assistant."},
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{
            .max_output_tokens = 1024,
            .thinking_effort = text::ThinkingEffort::MEDIUM,
            .temperature = 0.7
        }
    );

    // Check max_tokens
    REQUIRE_EQ(static_cast<double>(*request.max_tokens), 1024.0);

    // Check messages — should have one user message
    REQUIRE_EQ((*request.messages).size(), static_cast<size_t>(1));
    auto const& msg = (*request.messages)[0];
    REQUIRE_EQ(msg.role, anthropic::Request::MessageParam::Role::USER);

    // Check content — one text block inside the variant
    auto const& content_variant = static_cast<const anthropic::Request::MessageParam::Content&>(*msg.content);
    auto const& content_items = std::get<std::vector<anthropic::Request::MessageParam::ContentItem>>(content_variant);
    REQUIRE_EQ(content_items.size(), static_cast<size_t>(1));

    // Check system prompt was set
    REQUIRE(request.system.has_value());

    // Check temperature
    REQUIRE(request.temperature.has_value());
    REQUIRE_EQ(*request.temperature, 0.7);

    // Check model is empty string (filled by orchestrator)
    auto const& model_variant = static_cast<const anthropic::Request::Model&>(*request.model);
    REQUIRE_EQ(std::get<std::string>(model_variant), std::string{""});

    std::println("  [SUCCESS]");
}


void test_generate_gemini() {
    std::println("Testing Generate<gemini::GenerateContent>: basic text prompt...");

    auto request = text::Generate<gemini::GenerateContent>(
        text::Prompt{.text = "You are a helpful assistant."},
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{
            .max_output_tokens = 1024,
            .temperature = 0.7
        }
    );

    // Check system instruction
    REQUIRE(request.systemInstruction.has_value());
    REQUIRE(request.systemInstruction->parts.has_value());
    REQUIRE_EQ(request.systemInstruction->parts->size(), static_cast<size_t>(1));

    // Check contents — one user turn
    REQUIRE_EQ((*request.contents).size(), static_cast<size_t>(1));
    auto const& content = (*request.contents)[0];
    REQUIRE(content.role.has_value());
    REQUIRE_EQ(*content.role, std::string{"user"});
    REQUIRE(content.parts.has_value());
    REQUIRE_EQ(content.parts->size(), static_cast<size_t>(1));

    // Check generation config
    REQUIRE(request.generationConfig.has_value());
    REQUIRE(request.generationConfig->maxOutputTokens.has_value());
    REQUIRE_EQ(*request.generationConfig->maxOutputTokens, static_cast<int64_t>(1024));
    REQUIRE(request.generationConfig->temperature.has_value());
    REQUIRE_EQ(*request.generationConfig->temperature, 0.7);

    // Check model is empty (filled by orchestrator)
    REQUIRE_EQ(std::string{*request.model}, std::string{""});

    std::println("  [SUCCESS]");
}


void test_generate_openai() {
    std::println("Testing Generate<openai::Responses>: basic text prompt...");

    auto request = text::Generate<openai::Responses>(
        text::Prompt{.text = "You are a helpful assistant."},
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{
            .max_output_tokens = 1024,
            .temperature = 0.7
        }
    );

    // Check input — should have system message + user message
    REQUIRE(request.input.has_value());
    auto const& input_items = std::get<std::vector<openai::Request::InputItem>>(*request.input);
    REQUIRE_EQ(input_items.size(), static_cast<size_t>(2));

    // Check temperature
    REQUIRE(request.temperature.has_value());
    REQUIRE_EQ(*request.temperature, 0.7);

    // Check max_output_tokens
    REQUIRE(request.max_output_tokens.has_value());
    REQUIRE_EQ(*request.max_output_tokens, 1024.0);

    std::println("  [SUCCESS]");
}


void test_generate_no_system_prompt() {
    std::println("Testing Generate: no system prompt...");

    // Anthropic — system should be nullopt
    auto anthropic_req = text::Generate<anthropic::Messages>(
        std::nullopt,
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{.max_output_tokens = 100}
    );
    REQUIRE(!anthropic_req.system.has_value());

    // Gemini — systemInstruction should be nullopt
    auto gemini_req = text::Generate<gemini::GenerateContent>(
        std::nullopt,
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{.max_output_tokens = 100}
    );
    REQUIRE(!gemini_req.systemInstruction.has_value());

    // OpenAI — input should have only 1 item (user message, no system)
    auto openai_req = text::Generate<openai::Responses>(
        std::nullopt,
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{.max_output_tokens = 100}
    );
    auto const& openai_items = std::get<std::vector<openai::Request::InputItem>>(*openai_req.input);
    REQUIRE_EQ(openai_items.size(), static_cast<size_t>(1));

    std::println("  [SUCCESS]");
}


// ===== Extract tests — Anthropic =====


void test_extract_anthropic_success() {
    std::println("Testing Extract<anthropic::Messages>: text extraction...");

    anthropic::Message response;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "Hello, world!"}
    };

    auto result = text::Extract<anthropic::Messages>(response);

    REQUIRE_EQ(result.text, std::string{"Hello, world!"});

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_empty_content() {
    std::println("Testing Extract<anthropic::Messages>: empty content...");

    anthropic::Message response;
    // No content set (nullopt)

    auto result = text::Extract<anthropic::Messages>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_empty_content_vector() {
    std::println("Testing Extract<anthropic::Messages>: empty content vector...");

    anthropic::Message response;
    response.content = std::vector<anthropic::Message::ContentBlock>{};

    auto result = text::Extract<anthropic::Messages>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_multi_text_blocks() {
    std::println("Testing Extract<anthropic::Messages>: multiple text blocks joined with newline...");

    anthropic::Message response;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "First block"},
        anthropic::Message::TextBlock{.text = "Second block"}
    };

    auto result = text::Extract<anthropic::Messages>(response);

    REQUIRE_EQ(result.text, std::string{"First block\nSecond block"});

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_non_text_blocks_ignored() {
    std::println("Testing Extract<anthropic::Messages>: non-text blocks silently ignored...");

    anthropic::Message response;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "Hello"},
        anthropic::Message::ToolUseBlock{.id = "tool_1", .name = "search"},
        anthropic::Message::TextBlock{.text = "World"}
    };

    auto result = text::Extract<anthropic::Messages>(response);

    REQUIRE_EQ(result.text, std::string{"Hello\nWorld"});

    std::println("  [SUCCESS]");
}


// ===== Extract tests — Gemini =====


void test_extract_gemini_success() {
    std::println("Testing Extract<gemini::GenerateContent>: text extraction...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{
            .content = gemini::Content{
                .parts = std::vector<gemini::Part>{
                    gemini::Part{.text = "Hello from Gemini!"}
                }
            }
        }
    };

    auto result = text::Extract<gemini::GenerateContent>(response);

    REQUIRE_EQ(result.text, std::string{"Hello from Gemini!"});

    std::println("  [SUCCESS]");
}


void test_extract_gemini_no_candidates() {
    std::println("Testing Extract<gemini::GenerateContent>: no candidates...");

    gemini::GenerateContentResponse response;
    // No candidates set (nullopt)

    auto result = text::Extract<gemini::GenerateContent>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_gemini_empty_candidates() {
    std::println("Testing Extract<gemini::GenerateContent>: empty candidates vector...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{};

    auto result = text::Extract<gemini::GenerateContent>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_gemini_no_content() {
    std::println("Testing Extract<gemini::GenerateContent>: candidate with no content...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{}
    };

    auto result = text::Extract<gemini::GenerateContent>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_gemini_multi_parts() {
    std::println("Testing Extract<gemini::GenerateContent>: multiple text parts joined with newline...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{
            .content = gemini::Content{
                .parts = std::vector<gemini::Part>{
                    gemini::Part{.text = "First part"},
                    gemini::Part{.text = "Second part"}
                }
            }
        }
    };

    auto result = text::Extract<gemini::GenerateContent>(response);

    REQUIRE_EQ(result.text, std::string{"First part\nSecond part"});

    std::println("  [SUCCESS]");
}


// ===== Extract tests — OpenAI =====


void test_extract_openai_success() {
    std::println("Testing Extract<openai::Responses>: text extraction...");

    openai::Response response;
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputText{
                    .text = "Hello from OpenAI!"
                }
            }
        }
    };

    auto result = text::Extract<openai::Responses>(response);

    REQUIRE_EQ(result.text, std::string{"Hello from OpenAI!"});

    std::println("  [SUCCESS]");
}


void test_extract_openai_no_output() {
    std::println("Testing Extract<openai::Responses>: no output...");

    openai::Response response;
    // No output set (nullopt)

    auto result = text::Extract<openai::Responses>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_openai_empty_output() {
    std::println("Testing Extract<openai::Responses>: empty output vector...");

    openai::Response response;
    response.output = std::vector<openai::Response::ResponseOutputItem>{};

    auto result = text::Extract<openai::Responses>(response);

    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_openai_refusal_ignored() {
    std::println("Testing Extract<openai::Responses>: refusal block yields no text...");

    openai::Response response;
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputRefusal{
                    .refusal = "I cannot help with that."
                }
            }
        }
    };

    auto result = text::Extract<openai::Responses>(response);

    // Refusal blocks are not text — Extract ignores them
    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_openai_multi_text_blocks() {
    std::println("Testing Extract<openai::Responses>: multiple text blocks joined with newline...");

    openai::Response response;
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputText{
                    .text = "First"
                },
                openai::Response::ResponseOutputMessage::ResponseOutputText{
                    .text = "Second"
                }
            }
        }
    };

    auto result = text::Extract<openai::Responses>(response);

    REQUIRE_EQ(result.text, std::string{"First\nSecond"});

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

    std::println("===== Generate Tests =====");
    run(test_generate_anthropic);
    run(test_generate_gemini);
    run(test_generate_openai);
    run(test_generate_no_system_prompt);

    std::println("\n===== Extract: Anthropic Tests =====");
    run(test_extract_anthropic_success);
    run(test_extract_anthropic_empty_content);
    run(test_extract_anthropic_empty_content_vector);
    run(test_extract_anthropic_multi_text_blocks);
    run(test_extract_anthropic_non_text_blocks_ignored);

    std::println("\n===== Extract: Gemini Tests =====");
    run(test_extract_gemini_success);
    run(test_extract_gemini_no_candidates);
    run(test_extract_gemini_empty_candidates);
    run(test_extract_gemini_no_content);
    run(test_extract_gemini_multi_parts);

    std::println("\n===== Extract: OpenAI Tests =====");
    run(test_extract_openai_success);
    run(test_extract_openai_no_output);
    run(test_extract_openai_empty_output);
    run(test_extract_openai_refusal_ignored);
    run(test_extract_openai_multi_text_blocks);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
