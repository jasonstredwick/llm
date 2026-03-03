/***
 * Projection unit tests — Generate and Extract for projection::text.
 *
 * Tests verify that:
 *   - Generate produces correct provider-specific requests from projection inputs
 *   - Extract produces correct projection::text::Result from provider responses
 *   - Stop reason / finish reason mappings are correct
 *   - Usage fields are extracted properly per provider
 *   - Empty/missing content is handled gracefully
 *
 * These tests exercise Generate and Extract directly (no network needed).
 * The Tier 3 Call functions are thin wrappers over Tier 2 + Generate/Extract,
 * so proving these components are correct covers the full path.
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
namespace text = jai::llm::projection::text;


// ===== Generate tests =====


void test_generate_anthropic() {
    std::println("Testing Generate<anthropic::Request>: basic text prompt...");

    auto request = text::Generate<anthropic::Request>(
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
    std::println("Testing Generate<gemini::Request>: basic text prompt...");

    auto request = text::Generate<gemini::GenerateContentRequest>(
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
    std::println("Testing Generate<openai::Request>: basic text prompt...");

    auto request = text::Generate<openai::Request>(
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
    auto anthropic_req = text::Generate<anthropic::Request>(
        std::nullopt,
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{.max_output_tokens = 100}
    );
    REQUIRE(!anthropic_req.system.has_value());

    // Gemini — systemInstruction should be nullopt
    auto gemini_req = text::Generate<gemini::GenerateContentRequest>(
        std::nullopt,
        std::vector<text::Block>{text::Prompt{.text = "Hello"}},
        text::Options{.max_output_tokens = 100}
    );
    REQUIRE(!gemini_req.systemInstruction.has_value());

    // OpenAI — input should have only 1 item (user message, no system)
    auto openai_req = text::Generate<openai::Request>(
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
    std::println("Testing Extract<anthropic::Message>: success path...");

    anthropic::Message response;
    response.id = "msg_123";
    response.model = anthropic::Message::Model{std::string{"claude-sonnet-4-20250514"}};
    response.stop_reason = anthropic::Message::StopReason::END_TURN;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "Hello, world!"}
    };
    response.usage = anthropic::Message::Usage{
        .input_tokens = 10.0,
        .output_tokens = 5.0
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.text, std::string{"Hello, world!"});
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::COMPLETE);
    REQUIRE(result.diagnostics.warnings.empty());
    REQUIRE(!result.diagnostics.refusal.has_value());
    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(10));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(5));
    REQUIRE_EQ(result.metadata.id, std::string{"msg_123"});
    REQUIRE_EQ(result.metadata.model, std::string{"claude-sonnet-4-20250514"});

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_max_tokens() {
    std::println("Testing Extract<anthropic::Message>: MAX_TOKENS stop reason...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::MAX_TOKENS;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "Truncated text"}
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::MAX_TOKENS);
    REQUIRE_EQ(result.text, std::string{"Truncated text"});

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_refusal() {
    std::println("Testing Extract<anthropic::Message>: REFUSAL stop reason...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::REFUSAL;
    response.content = std::vector<anthropic::Message::ContentBlock>{};

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::CONTENT_FILTERED);
    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_tool_use_warning() {
    std::println("Testing Extract<anthropic::Message>: TOOL_USE gives UNEXPECTED + warning...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::TOOL_USE;
    response.content = std::vector<anthropic::Message::ContentBlock>{};

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::UNEXPECTED);
    REQUIRE(!result.diagnostics.warnings.empty());

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_empty_content() {
    std::println("Testing Extract<anthropic::Message>: empty content...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::END_TURN;
    // No content set (nullopt)

    auto result = text::Extract(response);

    REQUIRE(result.text.empty());
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::COMPLETE);

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_multi_text_blocks() {
    std::println("Testing Extract<anthropic::Message>: multiple text blocks joined with newline...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::END_TURN;
    response.content = std::vector<anthropic::Message::ContentBlock>{
        anthropic::Message::TextBlock{.text = "First block"},
        anthropic::Message::TextBlock{.text = "Second block"}
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.text, std::string{"First block\nSecond block"});

    std::println("  [SUCCESS]");
}


void test_extract_anthropic_usage() {
    std::println("Testing Extract<anthropic::Message>: usage fields...");

    anthropic::Message response;
    response.stop_reason = anthropic::Message::StopReason::END_TURN;
    response.content = std::vector<anthropic::Message::ContentBlock>{};
    response.usage = anthropic::Message::Usage{
        .cache_creation_input_tokens = 100.0,
        .cache_read_input_tokens = 50.0,
        .input_tokens = 200.0,
        .output_tokens = 80.0
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(200));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(80));
    REQUIRE(result.usage.cache_read_tokens.has_value());
    REQUIRE_EQ(*result.usage.cache_read_tokens, static_cast<int64_t>(50));
    REQUIRE(result.usage.cache_creation_tokens.has_value());
    REQUIRE_EQ(*result.usage.cache_creation_tokens, static_cast<int64_t>(100));

    std::println("  [SUCCESS]");
}


// ===== Extract tests — Gemini =====


void test_extract_gemini_success() {
    std::println("Testing Extract<gemini::Response>: success path...");

    gemini::GenerateContentResponse response;
    response.responseId = "resp_456";
    response.modelVersion = "gemini-2.5-pro";
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{
            .content = gemini::Content{
                .parts = std::vector<gemini::Part>{
                    gemini::Part{.text = "Hello from Gemini!"}
                }
            },
            .finishReason = gemini::FinishReason::STOP
        }
    };
    response.usageMetadata = gemini::UsageMetadata{
        .promptTokenCount = 15,
        .candidatesTokenCount = 8,
        .thoughtsTokenCount = 3
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.text, std::string{"Hello from Gemini!"});
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::COMPLETE);
    REQUIRE(result.diagnostics.warnings.empty());
    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(15));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(8));
    REQUIRE(result.usage.thinking_tokens.has_value());
    REQUIRE_EQ(*result.usage.thinking_tokens, static_cast<int64_t>(3));
    REQUIRE_EQ(result.metadata.id, std::string{"resp_456"});
    REQUIRE_EQ(result.metadata.model, std::string{"gemini-2.5-pro"});

    std::println("  [SUCCESS]");
}


void test_extract_gemini_safety_filtered() {
    std::println("Testing Extract<gemini::Response>: SAFETY finish reason...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{
            .finishReason = gemini::FinishReason::SAFETY
        }
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::CONTENT_FILTERED);
    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_gemini_max_tokens() {
    std::println("Testing Extract<gemini::Response>: MAX_TOKENS finish reason...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{
            .content = gemini::Content{
                .parts = std::vector<gemini::Part>{
                    gemini::Part{.text = "Truncated"}
                }
            },
            .finishReason = gemini::FinishReason::MAX_TOKENS
        }
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::MAX_TOKENS);
    REQUIRE_EQ(result.text, std::string{"Truncated"});

    std::println("  [SUCCESS]");
}


void test_extract_gemini_prompt_blocked() {
    std::println("Testing Extract<gemini::Response>: prompt blocked (no candidates)...");

    gemini::GenerateContentResponse response;
    // No candidates
    response.promptFeedback = gemini::PromptFeedback{
        .blockReason = gemini::BlockReason::SAFETY
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::CONTENT_FILTERED);
    REQUIRE(!result.diagnostics.warnings.empty());
    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_gemini_empty_candidates() {
    std::println("Testing Extract<gemini::Response>: empty candidates vector...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{};

    auto result = text::Extract(response);

    REQUIRE(result.text.empty());
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::UNEXPECTED);

    std::println("  [SUCCESS]");
}


void test_extract_gemini_usage() {
    std::println("Testing Extract<gemini::Response>: usage fields with cache...");

    gemini::GenerateContentResponse response;
    response.candidates = std::vector<gemini::Candidate>{
        gemini::Candidate{.finishReason = gemini::FinishReason::STOP}
    };
    response.usageMetadata = gemini::UsageMetadata{
        .promptTokenCount = 20,
        .cachedContentTokenCount = 10,
        .candidatesTokenCount = 12,
        .thoughtsTokenCount = 5
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(20));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(12));
    REQUIRE(result.usage.cache_read_tokens.has_value());
    REQUIRE_EQ(*result.usage.cache_read_tokens, static_cast<int64_t>(10));
    REQUIRE(result.usage.thinking_tokens.has_value());
    REQUIRE_EQ(*result.usage.thinking_tokens, static_cast<int64_t>(5));

    std::println("  [SUCCESS]");
}


// ===== Extract tests — OpenAI =====


void test_extract_openai_success() {
    std::println("Testing Extract<openai::Response>: success path...");

    openai::Response response;
    response.id = "resp_789";
    response.model = openai::Response::ResponsesModel{std::string{"gpt-5"}};
    response.status = openai::Response::Status::COMPLETED;
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputText{
                    .text = "Hello from OpenAI!"
                }
            }
        }
    };
    response.usage = openai::Response::ResponseUsage{
        .input_tokens = 12.0,
        .output_tokens = 6.0
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.text, std::string{"Hello from OpenAI!"});
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::COMPLETE);
    REQUIRE(result.diagnostics.warnings.empty());
    REQUIRE(!result.diagnostics.refusal.has_value());
    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(12));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(6));
    REQUIRE_EQ(result.metadata.id, std::string{"resp_789"});
    REQUIRE_EQ(result.metadata.model, std::string{"gpt-5"});

    std::println("  [SUCCESS]");
}


void test_extract_openai_refusal() {
    std::println("Testing Extract<openai::Response>: refusal content...");

    openai::Response response;
    response.status = openai::Response::Status::COMPLETED;
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputRefusal{
                    .refusal = "I cannot help with that."
                }
            }
        }
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::CONTENT_FILTERED);
    REQUIRE(result.diagnostics.refusal.has_value());
    REQUIRE_EQ(*result.diagnostics.refusal, std::string{"I cannot help with that."});

    std::println("  [SUCCESS]");
}


void test_extract_openai_incomplete() {
    std::println("Testing Extract<openai::Response>: INCOMPLETE with max_output_tokens...");

    openai::Response response;
    response.status = openai::Response::Status::INCOMPLETE;
    response.incomplete_details = openai::Response::Response_incomplete_details{
        .reason = openai::Response::Response_incomplete_details::Reason::MAX_OUTPUT_TOKENS
    };
    response.output = std::vector<openai::Response::ResponseOutputItem>{
        openai::Response::ResponseOutputMessage{
            .content = std::vector<openai::Response::ResponseOutputMessage::Content>{
                openai::Response::ResponseOutputMessage::ResponseOutputText{
                    .text = "Truncated text"
                }
            }
        }
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::MAX_TOKENS);
    REQUIRE_EQ(result.text, std::string{"Truncated text"});
    REQUIRE(!result.diagnostics.warnings.empty());

    std::println("  [SUCCESS]");
}


void test_extract_openai_failed() {
    std::println("Testing Extract<openai::Response>: FAILED status...");

    openai::Response response;
    response.status = openai::Response::Status::FAILED;
    response.error = openai::Response::ResponseError{
        .message = "Server error occurred"
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::CONTENT_FILTERED);
    REQUIRE(!result.diagnostics.warnings.empty());
    REQUIRE(result.text.empty());

    std::println("  [SUCCESS]");
}


void test_extract_openai_empty_output() {
    std::println("Testing Extract<openai::Response>: no output...");

    openai::Response response;
    response.status = openai::Response::Status::COMPLETED;
    // No output set

    auto result = text::Extract(response);

    REQUIRE(result.text.empty());
    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::COMPLETE);

    std::println("  [SUCCESS]");
}


void test_extract_openai_usage() {
    std::println("Testing Extract<openai::Response>: usage fields with reasoning + cache...");

    openai::Response response;
    response.status = openai::Response::Status::COMPLETED;
    response.output = std::vector<openai::Response::ResponseOutputItem>{};
    response.usage = openai::Response::ResponseUsage{
        .input_tokens = 25.0,
        .input_tokens_details = openai::Response::ResponseUsage::ResponseUsage_input_tokens_details{
            .cached_tokens = 10.0
        },
        .output_tokens = 15.0,
        .output_tokens_details = openai::Response::ResponseUsage::ResponseUsage_output_tokens_details{
            .reasoning_tokens = 8.0
        }
    };

    auto result = text::Extract(response);

    REQUIRE_EQ(result.usage.input_tokens, static_cast<int64_t>(25));
    REQUIRE_EQ(result.usage.output_tokens, static_cast<int64_t>(15));
    REQUIRE(result.usage.cache_read_tokens.has_value());
    REQUIRE_EQ(*result.usage.cache_read_tokens, static_cast<int64_t>(10));
    REQUIRE(result.usage.thinking_tokens.has_value());
    REQUIRE_EQ(*result.usage.thinking_tokens, static_cast<int64_t>(8));

    std::println("  [SUCCESS]");
}


void test_extract_openai_no_status() {
    std::println("Testing Extract<openai::Response>: no status field...");

    openai::Response response;
    // status not set

    auto result = text::Extract(response);

    REQUIRE_EQ(result.diagnostics.stop_reason, projection::StopReason::UNEXPECTED);

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
    run(test_extract_anthropic_max_tokens);
    run(test_extract_anthropic_refusal);
    run(test_extract_anthropic_tool_use_warning);
    run(test_extract_anthropic_empty_content);
    run(test_extract_anthropic_multi_text_blocks);
    run(test_extract_anthropic_usage);

    std::println("\n===== Extract: Gemini Tests =====");
    run(test_extract_gemini_success);
    run(test_extract_gemini_safety_filtered);
    run(test_extract_gemini_max_tokens);
    run(test_extract_gemini_prompt_blocked);
    run(test_extract_gemini_empty_candidates);
    run(test_extract_gemini_usage);

    std::println("\n===== Extract: OpenAI Tests =====");
    run(test_extract_openai_success);
    run(test_extract_openai_refusal);
    run(test_extract_openai_incomplete);
    run(test_extract_openai_failed);
    run(test_extract_openai_empty_output);
    run(test_extract_openai_usage);
    run(test_extract_openai_no_status);

    std::println("\n===== Results: {} failed =====", failed);
    return failed;
}
