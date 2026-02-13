#include <vector>
#include <string>
#include "test_assert.hpp"
#include <print>
#include <cstring>
#include <simdjson.h>
#include "../../src/curl.hpp"
#include "../../interface/core/error.hpp"

#include "../../interface/protocols/gemini/generate_content.hpp"
#include "../../src/protocols/gemini.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple Gemini Request Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Text{std::string{"Hello, Gemini!"}}
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    REQUIRE(json_str.find("\"role\":\"user\"") != std::string::npos);
    REQUIRE(json_str.find("\"text\":\"Hello, Gemini!\"") != std::string::npos);
    REQUIRE(json_str.find("\"parts\":[") != std::string::npos);
    REQUIRE(json_str.find("\"contents\":[") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

void test_complex_serialization() {
    std::println("Testing Complex Gemini Request Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Text{std::string{"I want to go to Tokyo."}}
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };
    
    // System instruction
    req.systemInstruction = gemini::RequestContent{
        .parts = std::vector<gemini::RequestContent::RequestPart>{
            gemini::RequestContent::RequestPart{
                .data = gemini::RequestContent::RequestPart::Data{
                    gemini::Text{std::string{"You are a helpful travel agent."}}
                },
                .partMetadata = json::Object{},
                .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
            }
        }
    };

    // Tools
    gemini::FunctionDeclaration get_flights{
        .name = Name64(std::string_view("get_flights")),
        .description = std::string{"Search for flights to a destination"}
    };
    
    gemini::Schema params{
        .type = gemini::SchemaType::OBJECT
    };
    
    gemini::Schema dest_prop{
        .type = gemini::SchemaType::STRING
    };
    dest_prop.description = std::string{"The destination city"};
    
    std::map<std::string, gemini::Schema> props;
    props.emplace("destination", std::move(dest_prop));
    params.properties = props;
    params.required = std::vector<std::string>{"destination"};
    
    get_flights.parameters = params;
    req.tools.Value().push_back(get_flights);

    // Safety settings
    req.safetySettings.Value().push_back(gemini::SafetySetting{
        .category = gemini::HarmCategory::HARM_CATEGORY_HARASSMENT,
        .threshold = gemini::HarmBlockThreshold::BLOCK_LOW_AND_ABOVE
    });

    // Generation config
    gemini::GenerationConfig gen_config{
        .stopSequences = std::vector<std::string>{"DONE", "FINISHED"},
        .responseModalities = std::vector<gemini::Modality>{}
    };
    gen_config.temperature = 0.5;
    gen_config.maxOutputTokens = 2048;
    req.generationConfig = gen_config;

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    REQUIRE(json_str.find("\"systemInstruction\":{") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_flights\"") != std::string::npos);
    REQUIRE(json_str.find("\"category\":\"HARM_CATEGORY_HARASSMENT\"") != std::string::npos);
    REQUIRE(json_str.find("\"temperature\":0.5") != std::string::npos);
    REQUIRE(json_str.find("\"maxOutputTokens\":2048") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_part_serialization() {
    std::println("Testing Gemini Part Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    // Blob (image)
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Blob{
                                .mimeType = gemini::MediaType::IMAGE_PNG,
                                .data = std::string{"YmFzZTY0X2RhdGE="}
                            }
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    },
                    // FileData
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::FileData{
                                .fileUri = std::string{"https://example.com/doc.pdf"}
                            }
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    },
                    // FunctionResponse
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::FunctionResponse{
                                .name = Name64(std::string_view("get_weather")),
                                .response = []() {
                                    json::Object o;
                                    o["temp"] = 72.0;
                                    o["condition"] = "Sunny";
                                    return o;
                                }(),
                                .parts = std::vector<gemini::FunctionResponse::Part>{}
                            }
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"mimeType\":\"image/png\"") != std::string::npos);
    REQUIRE(json_str.find("\"fileUri\":\"https://example.com/doc.pdf\"") != std::string::npos);
    REQUIRE(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    REQUIRE(json_str.find("\"temp\":72.0") != std::string::npos);

    std::println("[SUCCESS] Part Serialization passed.");
}

void test_response_deserialization() {
    std::println("Testing Gemini Response Deserialization...");

    std::string json_response = R"({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [
                        {
                            "text": "I've checked the weather for you.",
                            "partMetadata": {"videoMetadata": {}}
                        }
                    ]
                },
                "finishReason": "STOP",
                "index": 0,
                "safetyRatings": [
                    {"category": "HARM_CATEGORY_HARASSMENT", "probability": "NEGLIGIBLE", "blocked": false}
                ],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 20,
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {
            "safetyRatings": []
        },
        "usageMetadata": {
            "promptTokenCount": 10,
            "cachedContentTokenCount": 0,
            "candidatesTokenCount": 20,
            "toolUsePromptTokenCount": 0,
            "thoughtsTokenCount": 0,
            "totalTokenCount": 30,
            "promptTokensDetails": [],
            "cacheTokensDetails": [],
            "candidatesTokensDetails": [],
            "toolUsePromptTokensDetails": []
        },
        "responseId": "resp_123",
        "modelVersion": "gemini-1.5-pro",
        "modelStatus": {
            "modelStage": "STABLE",
            "retirementTime": "2025-12-31T23:59:59Z",
            "message": "Model is active"
        }
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto candidates = resp.candidates.Value();
    REQUIRE(candidates.size() == 1);
    auto content = candidates[0].content.Value();
    REQUIRE(content.role == gemini::Role::MODEL);
    
    auto parts = content.parts.Value();
    auto data = parts[0].data.Value();
    auto* text_part = std::get_if<gemini::Text>(&data);
    REQUIRE(text_part != nullptr);
    REQUIRE(text_part->text.Value() == "I've checked the weather for you.");

    REQUIRE(candidates[0].finishReason == gemini::FinishReason::STOP);
    auto safety_ratings = candidates[0].safetyRatings.Value();
    REQUIRE(safety_ratings.size() == 1);
    REQUIRE(safety_ratings[0].category.Value() == gemini::HarmCategory::HARM_CATEGORY_HARASSMENT);
    
    REQUIRE(resp.usageMetadata.Value().totalTokenCount.Value() == 30);

    std::println("[SUCCESS] Response Deserialization passed.");
}

void test_gemini_doc_examples() {
    std::println("Testing Gemini Documentation Examples...");

    // 1. Multimodal Request (from docs)
    {
        gemini::Request req{
            .contents = std::vector<gemini::RequestContent>{
                gemini::RequestContent{
                    .parts = std::vector<gemini::RequestContent::RequestPart>{
                        gemini::RequestContent::RequestPart{
                            .data = gemini::RequestContent::RequestPart::Data{
                                gemini::Text{std::string{"What is this?"}}
                            },
                            .partMetadata = json::Object{},
                            .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                        },
                        gemini::RequestContent::RequestPart{
                            .data = gemini::RequestContent::RequestPart::Data{
                                gemini::Blob{
                                    .mimeType = gemini::MediaType::IMAGE_JPEG,
                                    .data = std::string{"YmFzZTY0X2RhdGE="}
                                }
                            },
                            .partMetadata = json::Object{},
                            .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                        }
                    },
                    .role = gemini::Role::USER
                }
            }
        };

        auto serialized = gemini::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        
        REQUIRE(json_str.find("\"text\":\"What is this?\"") != std::string::npos);
        REQUIRE(json_str.find("\"inlineData\":") != std::string::npos);
        REQUIRE(json_str.find("\"mimeType\":\"image/jpeg\"") != std::string::npos);
    }

    // 2. Full Response (from docs)
    {
        std::string json_response = R"({
            "candidates": [
                {
                    "content": {
                        "parts": [
                            {
                                "text": "It's an ant.",
                                "partMetadata": {}
                            }
                        ],
                        "role": "model"
                    },
                    "finishReason": "STOP",
                    "index": 0,
                    "safetyRatings": [
                        {
                            "category": "HARM_CATEGORY_HARASSMENT",
                            "probability": "NEGLIGIBLE",
                            "blocked": false
                        }
                    ],
                    "citationMetadata": {"citationSources": []},
                    "tokenCount": 5,
                    "groundingMetadata": {
                        "groundingChunks": [],
                        "groundingSupports": [],
                        "webSearchQueries": []
                    },
                    "urlContextMetadata": {"urlMetadata": []}
                }
            ],
            "promptFeedback": {
                "safetyRatings": []
            },
            "usageMetadata": {
                "promptTokenCount": 10,
                "cachedContentTokenCount": 0,
                "candidatesTokenCount": 5,
                "toolUsePromptTokenCount": 0,
                "thoughtsTokenCount": 0,
                "totalTokenCount": 15,
                "promptTokensDetails": [],
                "cacheTokensDetails": [],
                "candidatesTokensDetails": [],
                "toolUsePromptTokensDetails": []
            },
            "modelVersion": "gemini-1.5-pro",
            "responseId": "resp_456",
            "modelStatus": {
                "modelStage": "STABLE",
                "retirementTime": "2025-12-31T23:59:59Z",
                "message": "Model is active"
            }
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = gemini::Deserialize(res);

        auto candidates = resp.candidates.Value();
        REQUIRE(candidates.size() == 1);
        auto content = candidates[0].content.Value();
        REQUIRE(content.role == gemini::Role::MODEL);
        auto parts = content.parts.Value();
        REQUIRE(parts.size() == 1);
        
        auto data = parts[0].data.Value();
        auto* text_part = std::get_if<gemini::Text>(&data);
        REQUIRE(text_part != nullptr);
        REQUIRE(text_part->text.Value() == "It's an ant.");
        
        auto safety_ratings = candidates[0].safetyRatings.Value();
        REQUIRE(safety_ratings.size() == 1);
        REQUIRE(safety_ratings[0].category.Value() == gemini::HarmCategory::HARM_CATEGORY_HARASSMENT);
        
        REQUIRE(resp.usageMetadata.Value().totalTokenCount.Value() == 15);
    }

    // 3. Advanced Grounding (from docs)
    {
        std::println("Testing Gemini Advanced Documentation Examples (Grounding)...");
        std::string json_response = R"({
            "candidates": [
                {
                    "content": {
                        "parts": [{"text": "The price of AAPL is $180.", "partMetadata": {}}],
                        "role": "model"
                    },
                    "finishReason": "STOP",
                    "groundingMetadata": {
                        "groundingChunks": [
                            {
                                "web": {"uri": "https://google.com/finance", "title": "Google Finance"}
                            }
                        ],
                        "groundingSupports": [],
                        "webSearchQueries": ["AAPL stock price"],
                        "searchEntryPoint": {"renderedContent": "<html>Search on Google</html>"}
                    },
                    "index": 0,
                    "safetyRatings": [],
                    "citationMetadata": {"citationSources": []},
                    "tokenCount": 10,
                    "urlContextMetadata": {"urlMetadata": []}
                }
            ],
            "promptFeedback": {"safetyRatings": []},
            "usageMetadata": {
                "promptTokenCount": 5, 
                "candidatesTokenCount": 10, 
                "totalTokenCount": 15,
                "cachedContentTokenCount": 0,
                "toolUsePromptTokenCount": 0,
                "thoughtsTokenCount": 0,
                "promptTokensDetails": [],
                "cacheTokensDetails": [],
                "candidatesTokensDetails": [],
                "toolUsePromptTokensDetails": []
            },
            "modelVersion": "gemini-1.5-flash",
            "responseId": "resp_999",
            "modelStatus": {"modelStage": "STABLE", "retirementTime": "2025-12-31T23:59:59Z", "message": ""}
        })";

        curl::Response res;
        res.state = curl::Response::State::COMPLETED;
        res.availability = curl::Response::Availability::FINAL;
        res.body_len = json_response.size();
        res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
        std::memcpy(res.body.data(), json_response.data(), res.body_len);

        auto resp = gemini::Deserialize(res);

        auto candidates = resp.candidates.Value();
        REQUIRE(candidates.size() == 1);
        auto& cand = candidates[0];
        auto grounding = cand.groundingMetadata.Value();
        REQUIRE(grounding.webSearchQueries.Value().size() == 1);
        REQUIRE(grounding.webSearchQueries.Value()[0] == "AAPL stock price");
        REQUIRE(grounding.groundingChunks.Value().size() == 1);
        
        auto chunks = grounding.groundingChunks.Value();
        auto chunk_type = chunks[0].chunk_type.Value();
        auto const* web = std::get_if<gemini::GroundingChunk::Web>(&chunk_type);
        REQUIRE(web != nullptr);
        REQUIRE(web->uri.Value().Get() == "https://google.com/finance");

        std::println("  [SUCCESS] Gemini Advanced Documentation Examples passed.");
    }

    std::println("[SUCCESS] Gemini Documentation Examples passed.");
}

void test_function_call_deserialization() {
    std::println("Testing Gemini FunctionCall Response Deserialization...");

    std::string json_response = R"({
        "responseId": "resp_123",
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [
                        {
                            "functionCall": {
                                "id": "call_001",
                                "name": "get_weather",
                                "args": {"location": "Tokyo", "unit": "celsius"}
                            },
                            "partMetadata": {}
                        }
                    ]
                },
                "finishReason": "STOP",
                "index": 0,
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 15,
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 10,
            "cachedContentTokenCount": 0,
            "candidatesTokenCount": 15,
            "toolUsePromptTokenCount": 0,
            "thoughtsTokenCount": 0,
            "totalTokenCount": 25,
            "promptTokensDetails": [],
            "cacheTokensDetails": [],
            "candidatesTokensDetails": [],
            "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-2.0-flash",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto candidates = resp.candidates.Value();
    REQUIRE(candidates.size() == 1);
    auto parts = candidates[0].content.Value().parts.Value();
    REQUIRE(parts.size() == 1);

    auto data = parts[0].data.Value();
    auto* fc = std::get_if<gemini::FunctionCall>(&data);
    REQUIRE(fc != nullptr);
    REQUIRE(fc->name.Value().Get() == "get_weather");
    REQUIRE(fc->id.has_value());
    REQUIRE(fc->id.value() == "call_001");

    // Verify args
    auto& args = fc->args.Value();
    REQUIRE(std::get<std::string>(args.at("location").data) == "Tokyo");
    REQUIRE(std::get<std::string>(args.at("unit").data) == "celsius");

    std::println("[SUCCESS] FunctionCall Response Deserialization passed.");
}


void test_thought_deserialization() {
    std::println("Testing Gemini Thought/ThoughtSignature Deserialization...");

    std::string json_response = R"({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [
                        {
                            "text": "Let me think about this...",
                            "partMetadata": {},
                            "thought": true,
                            "thoughtSignature": "sig_abc123"
                        },
                        {
                            "text": "The answer is 42.",
                            "partMetadata": {}
                        }
                    ]
                },
                "finishReason": "STOP",
                "index": 0,
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 25,
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 5,
            "cachedContentTokenCount": 0,
            "candidatesTokenCount": 25,
            "toolUsePromptTokenCount": 0,
            "thoughtsTokenCount": 10,
            "totalTokenCount": 30,
            "promptTokensDetails": [],
            "cacheTokensDetails": [],
            "candidatesTokensDetails": [],
            "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-2.5-pro",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""},
        "responseId": "resp_123"
    })";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto parts = resp.candidates.Value()[0].content.Value().parts.Value();
    REQUIRE(parts.size() == 2);

    // First part: thought
    REQUIRE(parts[0].thought.has_value());
    REQUIRE(parts[0].thought.value() == true);
    REQUIRE(parts[0].thoughtSignature.has_value());
    REQUIRE(parts[0].thoughtSignature.value() == "sig_abc123");
    auto* thought_text = std::get_if<gemini::Text>(&parts[0].data.Value());
    REQUIRE(thought_text != nullptr);
    REQUIRE(thought_text->text.Value() == "Let me think about this...");

    // Second part: no thought fields
    REQUIRE(!parts[1].thought.has_value());
    REQUIRE(!parts[1].thoughtSignature.has_value());
    auto* answer_text = std::get_if<gemini::Text>(&parts[1].data.Value());
    REQUIRE(answer_text != nullptr);
    REQUIRE(answer_text->text.Value() == "The answer is 42.");

    // Verify thoughtsTokenCount in usage
    REQUIRE(resp.usageMetadata.Value().thoughtsTokenCount.Value() == 10);

    std::println("[SUCCESS] Thought/ThoughtSignature Deserialization passed.");
}


int main() {
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_part_serialization();
        test_response_deserialization();
        test_gemini_doc_examples();
        test_function_call_deserialization();
        test_thought_deserialization();
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
