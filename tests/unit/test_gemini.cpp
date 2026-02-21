#include <vector>
#include <string>
#include "test_assert.hpp"
#include <print>
#include <cstring>
#include <simdjson.h>
#include "../../src/curl.hpp"
#include "../../interface/core/error.hpp"

#include "../../interface/protocols/gemini/generate_content.hpp"
#include "../../interface/protocols/gemini/generate_content_strings.hpp"
#include "../../src/protocols/gemini.hpp"

using namespace jai::llm;

// ── Type Aliases ────────────────────────────────────────
// Flat generated types — using aliases to keep test code unchanged.

using Req         = gemini::Request;
using ReqSafety   = gemini::SafetySetting;
using ReqGenCfg   = gemini::GenerationConfig;
using ThinkCfg    = gemini::ThinkingConfig;
using SpeechCfg   = gemini::SpeechConfig;
using ImageCfg    = gemini::ImageConfig;

using Resp           = gemini::Response;
using Cand           = gemini::Candidate;
using CandSafety     = gemini::SafetyRating;
using Citation       = gemini::CitationMetadata;
using CitationSrc    = gemini::CitationSource;
using Grounding      = gemini::GroundingMetadata;
using GChunk         = gemini::GroundingChunk;
using GSupport       = gemini::GoogleAiGenerativelanguageV1betaGroundingSupport;
using Segment        = gemini::GoogleAiGenerativelanguageV1betaSegment;
using GWeb           = gemini::Web;
using GMaps          = gemini::Maps;
using LogResult      = gemini::LogprobsResult;
using TopCands       = gemini::TopCandidates;
using LogCandidate   = gemini::LogprobsResultCandidate;
using UrlCtx         = gemini::UrlContextMetadata;
using UrlMeta        = gemini::UrlMetadata;
using PFeedback      = gemini::PromptFeedback;
using Usage          = gemini::UsageMetadata;
using ModalityTC     = gemini::ModalityTokenCount;
using ModelStat      = gemini::ModelStatus;

// ── Serialization Tests ─────────────────────────────────

void test_generation_config_serialization() {
    std::println("Testing Gemini GenerationConfig Serialization (Generated Types)...");

    // We can't construct a full Request because contents requires Content type.
    // Instead, we verify that the GenerationConfig struct is well-formed with
    // the generated field types.

    ReqGenCfg gen_config{};
    gen_config.stopSequences = std::vector<std::string>{"STOP", "DONE"};
    gen_config.responseMimeType = std::string{"application/json"};
    gen_config.candidateCount = 1;
    gen_config.maxOutputTokens = 2048;
    gen_config.temperature = 0.7;
    gen_config.topP = 0.95;
    gen_config.topK = 40;
    gen_config.seed = 12345;
    gen_config.presencePenalty = 0.1;
    gen_config.frequencyPenalty = 0.2;
    gen_config.responseLogprobs = true;
    gen_config.logprobs = 5;
    gen_config.enableEnhancedCivicAnswers = true;

    // Verify fields stored correctly
    REQUIRE(gen_config.stopSequences.has_value());
    REQUIRE(gen_config.stopSequences.value().size() == 2);
    REQUIRE(gen_config.temperature.has_value());
    REQUIRE(gen_config.temperature.value() == 0.7);
    REQUIRE(gen_config.maxOutputTokens.value() == 2048);
    REQUIRE(gen_config.topK.value() == 40);
    REQUIRE(gen_config.seed.value() == 12345);

    std::println("[SUCCESS] GenerationConfig Serialization passed.");
}

void test_thinking_config_construction() {
    std::println("Testing Gemini ThinkingConfig Construction (Generated Types)...");

    ThinkCfg thinking{.includeThoughts = true, .thinkingBudget = 4096};
    thinking.thinkingLevel = gemini::ThinkingLevel::HIGH;

    REQUIRE(thinking.includeThoughts.value() == true);
    REQUIRE(thinking.thinkingBudget.value() == 4096);
    REQUIRE(thinking.thinkingLevel.has_value());
    REQUIRE(thinking.thinkingLevel.value() == gemini::ThinkingLevel::HIGH);

    // Test all thinking levels
    thinking.thinkingLevel = gemini::ThinkingLevel::MINIMAL;
    REQUIRE(thinking.thinkingLevel.value() == gemini::ThinkingLevel::MINIMAL);

    thinking.thinkingLevel = gemini::ThinkingLevel::MEDIUM;
    REQUIRE(thinking.thinkingLevel.value() == gemini::ThinkingLevel::MEDIUM);

    std::println("[SUCCESS] ThinkingConfig Construction passed.");
}

void test_safety_setting_construction() {
    std::println("Testing Gemini SafetySetting Construction (Generated Types)...");

    ReqSafety safety{
        .category = gemini::Category::HARM_CATEGORY_HARASSMENT,
        .threshold = gemini::Threshold::BLOCK_LOW_AND_ABOVE,
    };

    REQUIRE(safety.category.value() == gemini::Category::HARM_CATEGORY_HARASSMENT);
    REQUIRE(safety.threshold.value() == gemini::Threshold::BLOCK_LOW_AND_ABOVE);

    // Test other category values
    safety.category = gemini::Category::HARM_CATEGORY_HATE_SPEECH;
    REQUIRE(safety.category.value() == gemini::Category::HARM_CATEGORY_HATE_SPEECH);

    safety.category = gemini::Category::HARM_CATEGORY_DANGEROUS_CONTENT;
    REQUIRE(safety.category.value() == gemini::Category::HARM_CATEGORY_DANGEROUS_CONTENT);

    // Test threshold values
    safety.threshold = gemini::Threshold::BLOCK_NONE;
    REQUIRE(safety.threshold.value() == gemini::Threshold::BLOCK_NONE);

    safety.threshold = gemini::Threshold::BLOCK_MEDIUM_AND_ABOVE;
    REQUIRE(safety.threshold.value() == gemini::Threshold::BLOCK_MEDIUM_AND_ABOVE);

    std::println("[SUCCESS] SafetySetting Construction passed.");
}

// ── Response Deserialization Tests ───────────────────────
// NOTE: Candidate.content is UNRESOLVED (maps to Required<std::string>), so
// tests that include a "content" object in JSON will fail at deserialization
// because the deserializer expects a string, not a JSON object. We test only
// response shapes that avoid or tolerate this, or focus on sub-objects that
// are fully resolved.

void test_usage_metadata_deserialization() {
    std::println("Testing Gemini UsageMetadata Deserialization (Generated Types)...");

    // Provide a response with empty candidates to avoid content issues
    std::string json_response = R"json({
        "candidates": [],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 50,
            "cachedContentTokenCount": 10,
            "candidatesTokenCount": 20,
            "toolUsePromptTokenCount": 5,
            "thoughtsTokenCount": 8,
            "totalTokenCount": 93,
            "promptTokensDetails": [
                {"modality": "TEXT", "tokenCount": 40},
                {"modality": "IMAGE", "tokenCount": 10}
            ],
            "cacheTokensDetails": [
                {"modality": "TEXT", "tokenCount": 10}
            ],
            "candidatesTokensDetails": [
                {"modality": "TEXT", "tokenCount": 20}
            ],
            "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-2.0-flash",
        "responseId": "resp_usage_test",
        "modelStatus": {
            "modelStage": "STABLE",
            "retirementTime": "2099-01-01T00:00:00Z",
            "message": "Model is active"
        }
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    // UsageMetadata
    auto usage = resp.usageMetadata.value();
    REQUIRE(usage.promptTokenCount.value() == 50);
    REQUIRE(usage.cachedContentTokenCount.value() == 10);
    REQUIRE(usage.candidatesTokenCount.value() == 20);
    REQUIRE(usage.toolUsePromptTokenCount.value() == 5);
    REQUIRE(usage.thoughtsTokenCount.value() == 8);
    REQUIRE(usage.totalTokenCount.value() == 93);

    // ModalityTokenCount details
    auto prompt_details = usage.promptTokensDetails.value();
    REQUIRE(prompt_details.size() == 2);
    REQUIRE(prompt_details[0].modality.value() == gemini::Modality::TEXT);
    REQUIRE(prompt_details[0].tokenCount.value() == 40);
    REQUIRE(prompt_details[1].modality.value() == gemini::Modality::IMAGE);
    REQUIRE(prompt_details[1].tokenCount.value() == 10);

    auto cache_details = usage.cacheTokensDetails.value();
    REQUIRE(cache_details.size() == 1);
    REQUIRE(cache_details[0].modality.value() == gemini::Modality::TEXT);
    REQUIRE(cache_details[0].tokenCount.value() == 10);

    auto cand_details = usage.candidatesTokensDetails.value();
    REQUIRE(cand_details.size() == 1);
    REQUIRE(cand_details[0].modality.value() == gemini::Modality::TEXT);
    REQUIRE(cand_details[0].tokenCount.value() == 20);

    // ModelVersion & ResponseId
    REQUIRE(resp.modelVersion.value() == "gemini-2.0-flash");
    REQUIRE(resp.responseId.value() == "resp_usage_test");

    std::println("[SUCCESS] UsageMetadata Deserialization passed.");
}

void test_model_status_deserialization() {
    std::println("Testing Gemini ModelStatus Deserialization (Generated Types)...");

    std::string json_response = R"json({
        "candidates": [],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0, "totalTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [],
            "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-2.5-pro",
        "responseId": "resp_model_status",
        "modelStatus": {
            "modelStage": "EXPERIMENTAL",
            "retirementTime": "2026-06-01T00:00:00Z",
            "message": "Experimental model"
        }
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto status = resp.modelStatus.value();
    REQUIRE(status.modelStage.value() == gemini::ModelStage::EXPERIMENTAL);
    REQUIRE(status.message.value() == "Experimental model");

    std::println("[SUCCESS] ModelStatus Deserialization passed.");
}

void test_prompt_feedback_deserialization() {
    std::println("Testing Gemini PromptFeedback Deserialization (Generated Types)...");

    // NOTE: PromptFeedback.safetyRatings references SafetyRating which may
    // be out of scope (defined under Candidate). This test may need adjustment
    // once the type resolution is fixed. Using empty array for now.

    std::string json_response = R"json({
        "candidates": [],
        "promptFeedback": {
            "blockReason": "SAFETY",
            "safetyRatings": []
        },
        "usageMetadata": {
            "promptTokenCount": 10, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0, "totalTokenCount": 10,
            "promptTokensDetails": [], "cacheTokensDetails": [],
            "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_blocked",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    REQUIRE(resp.promptFeedback.value().blockReason.has_value());
    REQUIRE(resp.promptFeedback.value().blockReason.value() == gemini::BlockReason::SAFETY);

    std::println("[SUCCESS] PromptFeedback Deserialization passed.");
}

void test_candidate_metadata_deserialization() {
    std::println("Testing Gemini Candidate Metadata Deserialization (Generated Types)...");

    // Candidate with content as a simple string (since content is UNRESOLVED
    // and mapped to Required<std::string>). We provide a string value.
    // Content is a proper object in the generated types.
    std::string json_response = R"json({
        "candidates": [
            {
                "content": {"parts": [{"text": "placeholder"}], "role": "model"},
                "finishReason": "STOP",
                "safetyRatings": [
                    {"category": "HARM_CATEGORY_HARASSMENT", "probability": "NEGLIGIBLE", "blocked": false}
                ],
                "citationMetadata": {
                    "citationSources": [
                        {"startIndex": 0, "endIndex": 10, "uri": "http://example.com/src", "license": "CC-BY-4.0"}
                    ]
                },
                "tokenCount": 25,
                "groundingAttributions": [],
                "groundingMetadata": {
                    "groundingChunks": [
                        {"web": {"uri": "https://example.com", "title": "Example"}, "chunk_type": "web"}
                    ],
                    "groundingSupports": [
                        {
                            "segment": {"partIndex": 0, "startIndex": 0, "endIndex": 5, "text": "hello"},
                            "groundingChunkIndices": [0],
                            "confidenceScores": [0.9]
                        }
                    ],
                    "webSearchQueries": ["test query"]
                },
                "avgLogprobs": -0.05,
                "logprobsResult": {
                    "topCandidates": [
                        {
                            "candidates": [
                                {"token": "The", "tokenId": 100, "logProbability": -0.01},
                                {"token": "A", "tokenId": 101, "logProbability": -0.5}
                            ]
                        }
                    ],
                    "chosenCandidates": [
                        {"token": "The", "tokenId": 100, "logProbability": -0.01}
                    ],
                    "logProbabilitySum": -0.01
                },
                "urlContextMetadata": {
                    "urlMetadata": [
                        {"retrievedUrl": "http://example.com/page", "urlRetrievalStatus": "URL_RETRIEVAL_STATUS_SUCCESS"},
                        {"retrievedUrl": "http://example.com/err", "urlRetrievalStatus": "URL_RETRIEVAL_STATUS_ERROR"}
                    ]
                },
                "index": 0
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 10, "cachedContentTokenCount": 0, "candidatesTokenCount": 25,
            "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0, "totalTokenCount": 35,
            "promptTokensDetails": [], "cacheTokensDetails": [],
            "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-2.0-flash",
        "responseId": "resp_cand_meta",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": "OK"}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto candidates = resp.candidates.value();
    REQUIRE(candidates.size() == 1);
    auto& cand = candidates[0];

    // FinishReason
    REQUIRE(cand.finishReason.has_value());
    REQUIRE(cand.finishReason.value() == gemini::FinishReason::STOP);

    // TokenCount
    REQUIRE(cand.tokenCount.value() == 25);

    // Index
    REQUIRE(cand.index.value() == 0);

    // SafetyRatings
    auto ratings = cand.safetyRatings.value();
    REQUIRE(ratings.size() == 1);
    REQUIRE(ratings[0].category.value() == gemini::Category::HARM_CATEGORY_HARASSMENT);
    REQUIRE(ratings[0].probability.value() == gemini::Probability::NEGLIGIBLE);
    REQUIRE(ratings[0].blocked.value() == false);

    // CitationMetadata
    auto citations = cand.citationMetadata.value().citationSources.value();
    REQUIRE(citations.size() == 1);
    REQUIRE(citations[0].startIndex.value() == 0);
    REQUIRE(citations[0].endIndex.value() == 10);
    REQUIRE(citations[0].uri.has_value());
    REQUIRE(citations[0].uri.value() == "http://example.com/src");
    REQUIRE(citations[0].license.value() == "CC-BY-4.0");

    // GroundingMetadata
    auto grounding = cand.groundingMetadata.value();
    REQUIRE(grounding.webSearchQueries.value().size() == 1);
    REQUIRE(grounding.webSearchQueries.value()[0] == "test query");

    // GroundingChunks
    auto chunks = grounding.groundingChunks.value();
    REQUIRE(chunks.size() == 1);
    REQUIRE(chunks[0].web.value().uri.value() == "https://example.com");
    REQUIRE(chunks[0].web.value().title.value() == "Example");

    // GroundingSupports
    auto supports = grounding.groundingSupports.value();
    REQUIRE(supports.size() == 1);
    REQUIRE(supports[0].segment.value().text.value() == "hello");
    REQUIRE(supports[0].segment.value().startIndex.value() == 0);
    REQUIRE(supports[0].segment.value().endIndex.value() == 5);
    REQUIRE(supports[0].groundingChunkIndices.has_value());
    REQUIRE(supports[0].groundingChunkIndices.value().size() == 1);
    REQUIRE(supports[0].groundingChunkIndices.value()[0] == 0);
    REQUIRE(supports[0].confidenceScores.has_value());
    REQUIRE(supports[0].confidenceScores.value()[0] == 0.9);

    // AvgLogprobs
    REQUIRE(cand.avgLogprobs.value() == -0.05);

    // LogprobsResult
    auto logprobs = cand.logprobsResult.value();
    REQUIRE(logprobs.topCandidates.value().size() == 1);
    auto top = logprobs.topCandidates.value()[0].candidates.value();
    REQUIRE(top.size() == 2);
    REQUIRE(top[0].token.value() == "The");
    REQUIRE(top[0].tokenId.value() == 100);
    REQUIRE(top[0].logProbability.value() == -0.01);
    REQUIRE(top[1].token.value() == "A");
    REQUIRE(logprobs.chosenCandidates.value().size() == 1);
    REQUIRE(logprobs.logProbabilitySum.value() == -0.01);

    // UrlContextMetadata
    auto urls = cand.urlContextMetadata.value().urlMetadata.value();
    REQUIRE(urls.size() == 2);
    REQUIRE(urls[0].retrievedUrl.value() == "http://example.com/page");
    REQUIRE(urls[0].urlRetrievalStatus.value() == gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_SUCCESS);
    REQUIRE(urls[1].retrievedUrl.value() == "http://example.com/err");
    REQUIRE(urls[1].urlRetrievalStatus.value() == gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_ERROR);

    std::println("[SUCCESS] Candidate Metadata Deserialization passed.");
}

void test_maps_grounding_deserialization() {
    std::println("Testing Gemini Maps Grounding Deserialization (Generated Types)...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {"parts": [{"text": "placeholder"}], "role": "model"},
                "finishReason": "STOP",
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 5,
                "groundingAttributions": [],
                "groundingMetadata": {
                    "groundingChunks": [
                        {
                            "maps": {
                                "uri": "https://maps.google.com/place/123",
                                "title": "Coffee Shop",
                                "text": "Best coffee in town",
                                "placeId": "place_123",
                                "placeAnswerSources": {
                                    "reviewSnippets": [
                                        {
                                            "reviewId": "rev_001",
                                            "googleMapsUri": "https://maps.google.com/review/001",
                                            "title": "Great Review"
                                        }
                                    ]
                                }
                            },
                            "chunk_type": "maps"
                        }
                    ],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "index": 0,
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "totalTokenCount": 0, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [],
            "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_maps",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto chunks = resp.candidates.value()[0].groundingMetadata.value().groundingChunks.value();
    REQUIRE(chunks.size() == 1);

    // Maps-specific fields
    REQUIRE(chunks[0].maps.has_value());
    auto& maps = chunks[0].maps.value();
    REQUIRE(maps.title.value() == "Coffee Shop");
    REQUIRE(maps.placeId.value() == "place_123");
    REQUIRE(maps.text.value() == "Best coffee in town");
    REQUIRE(maps.uri.value() == "https://maps.google.com/place/123");

    auto& sources = maps.placeAnswerSources.value();
    auto snippets = sources.reviewSnippets.value();
    REQUIRE(snippets.size() == 1);
    REQUIRE(snippets[0].reviewId.value() == "rev_001");
    REQUIRE(snippets[0].googleMapsUri.value() == "https://maps.google.com/review/001");
    REQUIRE(snippets[0].title.value() == "Great Review");

    std::println("[SUCCESS] Maps Grounding Deserialization passed.");
}

void test_enum_string_conversions() {
    std::println("Testing Gemini Enum String Conversions (Generated Types)...");

    // Request enums
    {
        auto cat = from_string_view<gemini::Category>("HARM_CATEGORY_HARASSMENT");
        REQUIRE(cat.has_value());
        REQUIRE(cat.value() == gemini::Category::HARM_CATEGORY_HARASSMENT);
        REQUIRE(to_string_view(gemini::Category::HARM_CATEGORY_HATE_SPEECH) == "HARM_CATEGORY_HATE_SPEECH");

        auto thresh = from_string_view<gemini::Threshold>("BLOCK_LOW_AND_ABOVE");
        REQUIRE(thresh.has_value());
        REQUIRE(thresh.value() == gemini::Threshold::BLOCK_LOW_AND_ABOVE);
        REQUIRE(to_string_view(gemini::Threshold::BLOCK_NONE) == "BLOCK_NONE");

        auto level = from_string_view<gemini::ThinkingLevel>("HIGH");
        REQUIRE(level.has_value());
        REQUIRE(level.value() == gemini::ThinkingLevel::HIGH);
        REQUIRE(to_string_view(gemini::ThinkingLevel::MINIMAL) == "MINIMAL");

        // Invalid values
        REQUIRE(!from_string_view<gemini::Category>("INVALID").has_value());
        REQUIRE(!from_string_view<gemini::Threshold>("INVALID").has_value());
    }

    // Response enums
    {
        auto finish = from_string_view<gemini::FinishReason>("STOP");
        REQUIRE(finish.has_value());
        REQUIRE(finish.value() == gemini::FinishReason::STOP);
        REQUIRE(to_string_view(gemini::FinishReason::MAX_TOKENS) == "MAX_TOKENS");
        REQUIRE(to_string_view(gemini::FinishReason::SAFETY) == "SAFETY");

        auto cat = from_string_view<gemini::Category>("HARM_CATEGORY_HARASSMENT");
        REQUIRE(cat.has_value());
        REQUIRE(cat.value() == gemini::Category::HARM_CATEGORY_HARASSMENT);

        auto prob = from_string_view<gemini::Probability>("NEGLIGIBLE");
        REQUIRE(prob.has_value());
        REQUIRE(prob.value() == gemini::Probability::NEGLIGIBLE);

        auto block = from_string_view<gemini::BlockReason>("SAFETY");
        REQUIRE(block.has_value());
        REQUIRE(block.value() == gemini::BlockReason::SAFETY);

        auto url_stat = from_string_view<gemini::UrlRetrievalStatus>("URL_RETRIEVAL_STATUS_SUCCESS");
        REQUIRE(url_stat.has_value());
        REQUIRE(url_stat.value() == gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_SUCCESS);

        auto modality = from_string_view<gemini::Modality>("TEXT");
        REQUIRE(modality.has_value());
        REQUIRE(modality.value() == gemini::Modality::TEXT);
        REQUIRE(to_string_view(gemini::Modality::IMAGE) == "IMAGE");
        REQUIRE(to_string_view(gemini::Modality::AUDIO) == "AUDIO");
        REQUIRE(to_string_view(gemini::Modality::VIDEO) == "VIDEO");

        auto stage = from_string_view<gemini::ModelStage>("STABLE");
        REQUIRE(stage.has_value());
        REQUIRE(stage.value() == gemini::ModelStage::STABLE);
        REQUIRE(to_string_view(gemini::ModelStage::DEPRECATED) == "DEPRECATED");
        REQUIRE(to_string_view(gemini::ModelStage::EXPERIMENTAL) == "EXPERIMENTAL");
    }

    std::println("[SUCCESS] Enum String Conversions passed.");
}

void test_full_response_deserialization() {
    std::println("Testing Gemini Full Response Deserialization (Generated Types)...");

    // Full response with all top-level fields.
    std::string json_response = R"json({
        "candidates": [
            {
                "content": {"parts": [{"text": "placeholder_content"}], "role": "model"},
                "finishReason": "STOP",
                "safetyRatings": [
                    {"category": "HARM_CATEGORY_HATE_SPEECH", "probability": "NEGLIGIBLE", "blocked": false}
                ],
                "citationMetadata": {
                    "citationSources": [{"startIndex": 0, "endIndex": 5, "uri": "http://source.com"}]
                },
                "tokenCount": 20,
                "groundingAttributions": [],
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": ["search query"]
                },
                "avgLogprobs": -0.1,
                "logprobsResult": {
                    "topCandidates": [
                        {"candidates": [{"token": "The", "tokenId": 123, "logProbability": -0.01}]}
                    ],
                    "chosenCandidates": [{"token": "The", "tokenId": 123, "logProbability": -0.01}],
                    "logProbabilitySum": -0.01
                },
                "urlContextMetadata": {"urlMetadata": []},
                "index": 0
            }
        ],
        "promptFeedback": {
            "blockReason": "SAFETY",
            "safetyRatings": []
        },
        "usageMetadata": {
            "promptTokenCount": 10, "cachedContentTokenCount": 0, "candidatesTokenCount": 20,
            "totalTokenCount": 30, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [],
            "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_full",
        "modelStatus": {
            "modelStage": "STABLE",
            "retirementTime": "2099-01-01T00:00:00Z",
            "message": "OK"
        }
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    // Top-level fields
    REQUIRE(resp.modelVersion.value() == "gemini-1.5-pro");
    REQUIRE(resp.responseId.value() == "resp_full");

    // PromptFeedback
    REQUIRE(resp.promptFeedback.value().blockReason.has_value());
    REQUIRE(resp.promptFeedback.value().blockReason.value() == gemini::BlockReason::SAFETY);

    // Candidates
    auto candidates = resp.candidates.value();
    REQUIRE(candidates.size() == 1);
    REQUIRE(candidates[0].finishReason.value() == gemini::FinishReason::STOP);
    REQUIRE(candidates[0].tokenCount.value() == 20);
    REQUIRE(candidates[0].index.value() == 0);

    // Content is now a struct — just verify it has a value
    REQUIRE(candidates[0].content.has_value());

    // SafetyRatings
    auto ratings = candidates[0].safetyRatings.value();
    REQUIRE(ratings.size() == 1);
    REQUIRE(ratings[0].category.value() == gemini::Category::HARM_CATEGORY_HATE_SPEECH);

    // Logprobs
    REQUIRE(candidates[0].avgLogprobs.value() == -0.1);
    auto logprobs = candidates[0].logprobsResult.value();
    REQUIRE(logprobs.topCandidates.value().size() == 1);
    REQUIRE(logprobs.chosenCandidates.value().size() == 1);

    // Grounding
    auto queries = candidates[0].groundingMetadata.value().webSearchQueries.value();
    REQUIRE(queries.size() == 1);
    REQUIRE(queries[0] == "search query");

    // UsageMetadata
    REQUIRE(resp.usageMetadata.value().totalTokenCount.value() == 30);
    REQUIRE(resp.usageMetadata.value().promptTokenCount.value() == 10);
    REQUIRE(resp.usageMetadata.value().candidatesTokenCount.value() == 20);

    // ModelStatus
    REQUIRE(resp.modelStatus.value().modelStage.value() == gemini::ModelStage::STABLE);
    REQUIRE(resp.modelStatus.value().message.value() == "OK");

    std::println("[SUCCESS] Full Response Deserialization passed.");
}


int main() {
    try {
        // Construction tests (no serialization to JSON since Request needs
        // Content/Tool types that are UNRESOLVED)
        test_generation_config_serialization();
        test_thinking_config_construction();
        test_safety_setting_construction();

        // Response deserialization tests
        test_usage_metadata_deserialization();
        test_model_status_deserialization();
        test_prompt_feedback_deserialization();
        test_candidate_metadata_deserialization();
        test_maps_grounding_deserialization();

        // Enum string conversion tests
        test_enum_string_conversions();

        // Full response test
        test_full_response_deserialization();

        std::println("ALL GEMINI UNIT TESTS PASSED");
        return 0;
    } catch (const jai::llm::AnnotatedException& e) {
        std::println("[ERROR] AnnotatedException\n{}", jai::llm::to_string(e));
        return 1;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
