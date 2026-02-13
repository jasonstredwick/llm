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

void test_thinking_config_serialization() {
    std::println("Testing Gemini ThinkingConfig Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Text{std::string{"Solve this math problem."}}
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };

    req.generationConfig = gemini::GenerationConfig{
        .thinkingConfig = gemini::ThinkingConfig{
            .thinkingBudget = 2048,
            .thinkingLevel = gemini::ThinkingLevel::HIGH,
            .includeThoughts = true
        }
    };

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"thinkingConfig\":{") != std::string::npos);
    REQUIRE(json_str.find("\"thinkingBudget\":2048") != std::string::npos);
    REQUIRE(json_str.find("\"thinkingLevel\":\"HIGH\"") != std::string::npos);
    REQUIRE(json_str.find("\"includeThoughts\":true") != std::string::npos);

    std::println("[SUCCESS] ThinkingConfig Serialization passed.");
}

void test_tool_config_serialization() {
    std::println("Testing Gemini ToolConfig Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Text{std::string{"Hello"}}
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };

    req.toolConfig = gemini::ToolConfig{
        .functionCallingConfig = gemini::ToolConfig::FunctionCallingConfig{
            .mode = gemini::ToolMode::ANY,
            .allowedFunctionNames = std::vector<std::string>{"func1", "func2"}
        },
        .retrievalConfig = gemini::ToolConfig::RetrievalConfig{
            .latLng = gemini::ToolConfig::RetrievalConfig::LatLng{
                .latitude = 37.7749,
                .longitude = -122.4194
            },
            .languageCode = std::string{"en-US"}
        }
    };

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"functionCallingConfig\":{") != std::string::npos);
    REQUIRE(json_str.find("\"mode\":\"ANY\"") != std::string::npos);
    REQUIRE(json_str.find("\"allowedFunctionNames\":[\"func1\",\"func2\"]") != std::string::npos);
    REQUIRE(json_str.find("\"retrievalConfig\":{") != std::string::npos);
    REQUIRE(json_str.find("\"latLng\":{") != std::string::npos);
    REQUIRE(json_str.find("\"latitude\":37.7749") != std::string::npos);
    REQUIRE(json_str.find("\"languageCode\":\"en-US\"") != std::string::npos);

    std::println("[SUCCESS] ToolConfig Serialization passed.");
}

void test_tool_variants_serialization() {
    std::println("Testing Gemini Tool Variants Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{gemini::Text{std::string{"Hi"}}},
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                }
            }
        }
    };

    // 1. CodeExecution
    req.tools.Value().clear();
    req.tools.Value().push_back(gemini::CodeExecution{});
    {
        auto serialized = gemini::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        REQUIRE(json_str.find("\"codeExecution\":{}") != std::string::npos);
    }

    // 2. ComputerUse
    req.tools.Value().clear();
    req.tools.Value().push_back(gemini::ComputerUse{
        .environment = gemini::Environment::ENVIRONMENT_BROWSER,
        .excludedPredefinedFunctions = std::vector<std::string>{"exclude_me"}
    });
    {
        auto serialized = gemini::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        REQUIRE(json_str.find("\"computerUse\":{") != std::string::npos);
        REQUIRE(json_str.find("\"environment\":\"ENVIRONMENT_BROWSER\"") != std::string::npos);
        REQUIRE(json_str.find("\"excludedPredefinedFunctions\":[\"exclude_me\"]") != std::string::npos);
    }

    // 3. GoogleSearch
    req.tools.Value().clear();
    req.tools.Value().push_back(gemini::GoogleSearch{});
    {
        auto serialized = gemini::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        REQUIRE(json_str.find("\"googleSearch\":{}") != std::string::npos);
    }

    // 4. GoogleSearchRetrieval
    req.tools.Value().clear();
    req.tools.Value().push_back(gemini::GoogleSearchRetrieval{
        .dynamicRetrievalConfig = gemini::GoogleSearchRetrieval::Config{
            .mode = gemini::DynamicRetrievalMode::MODE_DYNAMIC,
            .dynamicThreshold = 75
        }
    });
    {
        auto serialized = gemini::Serialize(req);
        std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());
        REQUIRE(json_str.find("\"googleSearchRetrieval\":{") != std::string::npos);
        REQUIRE(json_str.find("\"mode\":\"MODE_DYNAMIC\"") != std::string::npos);
        REQUIRE(json_str.find("\"dynamicThreshold\":75") != std::string::npos);
    }

    std::println("[SUCCESS] Tool Variants Serialization passed.");
}

void test_generation_config_serialization() {
    std::println("Testing Gemini GenerationConfig Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{gemini::Text{std::string{"Msg"}}},
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                }
            }
        }
    };

    req.generationConfig = gemini::GenerationConfig{
        .stopSequences = std::vector<std::string>{"STOP"},
        .responseMimeType = gemini::ResponseMimeType::APPLICATION_JSON,
        .responseModalities = std::vector<gemini::Modality>{gemini::Modality::TEXT, gemini::Modality::IMAGE},
        .candidateCount = 1,
        .maxOutputTokens = 100,
        .temperature = 0.9,
        .topP = 0.95,
        .topK = 40,
        .seed = 12345,
        .presencePenalty = 0.1,
        .responseLogprobs = true,
        .logprobs = 5,
        .enableEnhancedCivicAnswers = true
    };

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    REQUIRE(json_str.find("\"responseMimeType\":\"application/json\"") != std::string::npos);
    REQUIRE(json_str.find("\"candidateCount\":1") != std::string::npos);
    REQUIRE(json_str.find("\"topP\":0.95") != std::string::npos);
    REQUIRE(json_str.find("\"topK\":40") != std::string::npos);
    REQUIRE(json_str.find("\"seed\":12345") != std::string::npos);
    REQUIRE(json_str.find("\"presencePenalty\":0.1") != std::string::npos);
    REQUIRE(json_str.find("\"responseLogprobs\":true") != std::string::npos);
    REQUIRE(json_str.find("\"logprobs\":5") != std::string::npos);

    std::println("[SUCCESS] GenerationConfig Serialization passed.");
}

void test_code_execution_result_serialization() {
    std::println("Testing Gemini CodeExecutionResult Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::CodeExecutionResult{
                                .outcome = gemini::ExecutionOutcome::OK,
                                .output = std::string{"Output string"}
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

    REQUIRE(json_str.find("\"codeExecutionResult\":{") != std::string::npos);
    REQUIRE(json_str.find("\"outcome\":\"OUTCOME_OK\"") != std::string::npos);
    REQUIRE(json_str.find("\"output\":\"Output string\"") != std::string::npos);

    std::println("[SUCCESS] CodeExecutionResult Serialization passed.");
}
void test_full_request_serialization() {
    std::println("Testing Gemini Full Request Serialization...");

    gemini::Request req{
        .contents = std::vector<gemini::RequestContent>{
            gemini::RequestContent{
                .parts = std::vector<gemini::RequestContent::RequestPart>{
                    gemini::RequestContent::RequestPart{
                        .data = gemini::RequestContent::RequestPart::Data{
                            gemini::Text{std::string{"Analyze this."}}
                        },
                        .partMetadata = json::Object{},
                        .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
                    }
                },
                .role = gemini::Role::USER
            }
        }
    };

    // System Instruction
    req.systemInstruction = gemini::RequestContent{
        .parts = std::vector<gemini::RequestContent::RequestPart>{
            gemini::RequestContent::RequestPart{
                .data = gemini::RequestContent::RequestPart::Data{
                    gemini::Text{std::string{"Be precise."}}
                },
                .partMetadata = json::Object{},
                .metadata = gemini::RequestContent::RequestPart::Metadata{gemini::VideoMetadata{}}
            }
        }
    };

    // Tools
    req.tools.Value().push_back(gemini::CodeExecution{});
    req.tools.Value().push_back(gemini::GoogleSearchRetrieval{
        .dynamicRetrievalConfig = gemini::GoogleSearchRetrieval::Config{
            .mode = gemini::DynamicRetrievalMode::MODE_DYNAMIC,
            .dynamicThreshold = 0.7
        }
    });

    // Tool Config
    req.toolConfig = gemini::ToolConfig{
        .functionCallingConfig = gemini::ToolConfig::FunctionCallingConfig{
            .mode = gemini::ToolMode::ANY,
            .allowedFunctionNames = std::vector<std::string>{"func1"}
        }
    };

    // Safety Settings
    req.safetySettings.Value().push_back(gemini::SafetySetting{
        .category = gemini::HarmCategory::HARM_CATEGORY_HATE_SPEECH,
        .threshold = gemini::HarmBlockThreshold::BLOCK_LOW_AND_ABOVE
    });

    // Generation Config
    req.generationConfig = gemini::GenerationConfig{
        .stopSequences = std::vector<std::string>{"STOP"},
        .responseMimeType = gemini::ResponseMimeType::APPLICATION_JSON,
        .candidateCount = 2,
        .maxOutputTokens = 100,
        .temperature = 0.5,
        .topP = 0.8,
        .topK = 10
    };

    // Cached Content
    req.cachedContent = "cached_content_id_123";

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    REQUIRE(json_str.find("\"systemInstruction\":{") != std::string::npos);
    REQUIRE(json_str.find("\"codeExecution\":{}") != std::string::npos);
    REQUIRE(json_str.find("\"googleSearchRetrieval\":{") != std::string::npos);
    REQUIRE(json_str.find("\"toolConfig\":{") != std::string::npos);
    REQUIRE(json_str.find("\"safetySettings\":[{") != std::string::npos);
    REQUIRE(json_str.find("\"generationConfig\":{") != std::string::npos);
    REQUIRE(json_str.find("\"cachedContent\":\"cached_content_id_123\"") != std::string::npos);
    REQUIRE(json_str.find("\"temperature\":0.5") != std::string::npos);

    std::println("[SUCCESS] Full Request Serialization passed.");
}



void test_executable_code_deserialization() {
    std::println("Testing Gemini ExecutableCode Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [
                        {
                            "executableCode": {
                                "language": "PYTHON",
                                "code": "print('Hello')"
                            },
                            "partMetadata": {}
                        }
                    ]
                },
                "finishReason": "STOP",
                "index": 0,
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 10,
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
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0, "totalTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_exec_code",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto parts = resp.candidates.Value()[0].content.Value().parts.Value();
    REQUIRE(parts.size() == 1);

    auto data = parts[0].data.Value();
    auto* exec_code = std::get_if<gemini::ExecutableCode>(&data);
    REQUIRE(exec_code != nullptr);
    REQUIRE(exec_code->language.Value() == gemini::CodeLanguage::PYTHON);
    REQUIRE(exec_code->code.Value() == "print('Hello')");

    std::println("[SUCCESS] ExecutableCode Deserialization passed.");
}

void test_grounding_support_deserialization() {
    std::println("Testing Gemini GroundingSupport Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [{"text": "The price is $100.", "partMetadata": {}}]
                },
                "finishReason": "STOP",
                "groundingMetadata": {
                    "groundingChunks": [
                        {
                            "web": {"uri": "http://example.com", "title": "Example"}
                        }
                    ],
                    "groundingSupports": [
                        {
                            "segment": {
                                "partIndex": 0,
                                "startIndex": 4,
                                "endIndex": 9,
                                "text": "price"
                            },
                            "groundingChunkIndices": [0],
                            "confidenceScores": [0.95]
                        }
                    ],
                    "webSearchQueries": []
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
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "totalTokenCount": 0, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_grounding",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto candidates = resp.candidates.Value();
    REQUIRE(candidates.size() == 1);
    
    auto grounding = candidates[0].groundingMetadata.Value();
    auto supports = grounding.groundingSupports.Value();
    REQUIRE(supports.size() == 1);

    auto& supp = supports[0];
    REQUIRE(supp.segment.Value().text.Value() == "price");
    REQUIRE(supp.segment.Value().startIndex.Value() == 4);
    REQUIRE(supp.segment.Value().endIndex.Value() == 9);
    
    REQUIRE(supp.groundingChunkIndices.has_value());
    REQUIRE(supp.groundingChunkIndices.value().size() == 1);
    REQUIRE(supp.groundingChunkIndices.value()[0] == 0);
    
    REQUIRE(supp.confidenceScores.has_value());
    REQUIRE(supp.confidenceScores.value().size() == 1);
    REQUIRE(supp.confidenceScores.value()[0] == 0.95);

    std::println("[SUCCESS] GroundingSupport Deserialization passed.");
}

void test_citation_source_deserialization() {
    std::println("Testing Gemini CitationSource Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [{"text": "Just a fact.", "partMetadata": {}}]
                },
                "finishReason": "STOP",
                "citationMetadata": {
                    "citationSources": [
                        {
                            "startIndex": 0,
                            "endIndex": 10,
                            "uri": "http://example.com/source",
                            "license": "CC-BY-4.0"
                        }
                    ]
                },
                "index": 0,
                "safetyRatings": [],
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "tokenCount": 5,
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "totalTokenCount": 0, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_citation",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto citations = resp.candidates.Value()[0].citationMetadata.Value().citationSources.Value();
    REQUIRE(citations.size() == 1);
    REQUIRE(citations[0].startIndex.value() == 0);
    REQUIRE(citations[0].endIndex.value() == 10);
    REQUIRE(citations[0].uri.has_value());
    REQUIRE(citations[0].uri.value().Get() == "http://example.com/source");
    REQUIRE(citations[0].license.value() == "CC-BY-4.0");

    std::println("[SUCCESS] CitationSource Deserialization passed.");
}

void test_maps_grounding_deserialization() {
    std::println("Testing Gemini Maps Grounding Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [{"text": "It's near this place.", "partMetadata": {}}]
                },
                "finishReason": "STOP",
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
                            }
                        }
                    ],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "index": 0,
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "tokenCount": 5,
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "totalTokenCount": 0, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
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

    auto chunks = resp.candidates.Value()[0].groundingMetadata.Value().groundingChunks.Value();
    REQUIRE(chunks.size() == 1);
    
    auto chunk_type = chunks[0].chunk_type.Value();
    auto* maps = std::get_if<gemini::GroundingChunk::Maps>(&chunk_type);
    REQUIRE(maps != nullptr);
    REQUIRE(maps->title.Value() == "Coffee Shop");
    REQUIRE(maps->placeId.Value() == "place_123");
    REQUIRE(maps->placeAnswerSources.Value().reviewSnippets.Value().size() == 1);
    REQUIRE(maps->placeAnswerSources.Value().reviewSnippets.Value()[0].reviewId.Value() == "rev_001");

    std::println("[SUCCESS] Maps Grounding Deserialization passed.");
}

void test_url_context_metadata_deserialization() {
    std::println("Testing Gemini UrlContextMetadata Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [{"text": "Content", "partMetadata": {}}]
                },
                "finishReason": "STOP",
                "urlContextMetadata": {
                    "urlMetadata": [
                        {
                            "retrievedUrl": "http://example.com/page",
                            "urlRetrievalStatus": "URL_RETRIEVAL_STATUS_SUCCESS"
                        },
                        {
                            "retrievedUrl": "http://example.com/error",
                            "urlRetrievalStatus": "URL_RETRIEVAL_STATUS_ERROR"
                        }
                    ]
                },
                "index": 0,
                "safetyRatings": [],
                "citationMetadata": {"citationSources": []},
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": []
                },
                "tokenCount": 5
            }
        ],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 0, "cachedContentTokenCount": 0, "candidatesTokenCount": 0,
            "totalTokenCount": 0, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
        },
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_url_ctx",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto urls = resp.candidates.Value()[0].urlContextMetadata.Value().urlMetadata.Value();
    REQUIRE(urls.size() == 2);
    REQUIRE(urls[0].retrievedUrl.Value() == "http://example.com/page");
    REQUIRE(urls[0].urlRetrievalStatus.Value() == gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_SUCCESS);
    REQUIRE(urls[1].retrievedUrl.Value() == "http://example.com/error");
    REQUIRE(urls[1].urlRetrievalStatus.Value() == gemini::UrlRetrievalStatus::URL_RETRIEVAL_STATUS_ERROR);

    std::println("[SUCCESS] UrlContextMetadata Deserialization passed.");
}

void test_modality_token_count_deserialization() {
    std::println("Testing Gemini ModalityTokenCount Deserialization...");

    std::string json_response = R"json({
        "candidates": [],
        "promptFeedback": {"safetyRatings": []},
        "usageMetadata": {
            "promptTokenCount": 50,
            "cachedContentTokenCount": 10,
            "candidatesTokenCount": 20,
            "toolUsePromptTokenCount": 0,
            "thoughtsTokenCount": 0,
            "totalTokenCount": 80,
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
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_tokens",
        "modelStatus": {"modelStage": "STABLE", "retirementTime": "2099-01-01T00:00:00Z", "message": ""}
    })json";

    curl::Response res;
    res.state = curl::Response::State::COMPLETED;
    res.availability = curl::Response::Availability::FINAL;
    res.body_len = json_response.size();
    res.body.resize(res.body_len + simdjson::SIMDJSON_PADDING);
    std::memcpy(res.body.data(), json_response.data(), res.body_len);

    auto resp = gemini::Deserialize(res);

    auto usage = resp.usageMetadata.Value();
    REQUIRE(usage.promptTokenCount.Value() == 50);
    REQUIRE(usage.totalTokenCount.Value() == 80);

    auto details = usage.promptTokensDetails.Value();
    REQUIRE(details.size() == 2);
    REQUIRE(details[0].modality.Value() == gemini::Modality::TEXT);
    REQUIRE(details[0].tokenCount.Value() == 40);
    REQUIRE(details[1].modality.Value() == gemini::Modality::IMAGE);
    REQUIRE(details[1].tokenCount.Value() == 10);

    auto cache_details = usage.cacheTokensDetails.Value();
    REQUIRE(cache_details.size() == 1);
    REQUIRE(cache_details[0].modality.Value() == gemini::Modality::TEXT);

    std::println("[SUCCESS] ModalityTokenCount Deserialization passed.");
}

void test_full_response_deserialization() {
    std::println("Testing Gemini Full Response Deserialization...");

    std::string json_response = R"json({
        "candidates": [
            {
                "content": {
                    "role": "model",
                    "parts": [
                        {"text": "Part 1", "partMetadata": {}},
                        {
                            "functionCall": {
                                "name": "get_weather",
                                "args": {"location": "London"}
                            },
                             "partMetadata": {}
                        }
                    ]
                },
                "finishReason": "STOP",
                "index": 0,
                "safetyRatings": [
                    {"category": "HARM_CATEGORY_HATE_SPEECH", "probability": "NEGLIGIBLE", "blocked": false}
                ],
                "citationMetadata": {
                    "citationSources": [{"startIndex": 0, "endIndex": 5, "uri": "http://source.com"}]
                },
                "tokenCount": 20,
                "groundingMetadata": {
                    "groundingChunks": [],
                    "groundingSupports": [],
                    "webSearchQueries": ["search query"]
                },
                "avgLogprobs": -0.1,
                "logprobsResult": {
                    "topCandidates": [
                        {
                            "candidates": [
                                {"token": "The", "tokenId": 123, "logProbability": -0.01},
                                {"token": "A", "tokenId": 124, "logProbability": -0.5}
                            ]
                        }
                    ],
                    "chosenCandidates": [{"token": "The", "tokenId": 123, "logProbability": -0.01}],
                    "logProbabilitySum": -0.01
                },
                "urlContextMetadata": {"urlMetadata": []}
            }
        ],
        "promptFeedback": {
            "blockReason": "SAFETY",
            "safetyRatings": []
        },
        "usageMetadata": {
            "promptTokenCount": 10, "cachedContentTokenCount": 0, "candidatesTokenCount": 20,
            "totalTokenCount": 30, "toolUsePromptTokenCount": 0, "thoughtsTokenCount": 0,
            "promptTokensDetails": [], "cacheTokensDetails": [], "candidatesTokensDetails": [], "toolUsePromptTokensDetails": []
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

    REQUIRE(resp.modelVersion.Value() == "gemini-1.5-pro");
    REQUIRE(resp.responseId.Value() == "resp_full");
    
    // Prompt Feedback
    REQUIRE(resp.promptFeedback.Value().blockReason.has_value());
    REQUIRE(resp.promptFeedback.Value().blockReason.value() == gemini::BlockReason::SAFETY);

    // Candidates
    auto candidates = resp.candidates.Value();
    REQUIRE(candidates.size() == 1);
    
    // Logprobs
    REQUIRE(candidates[0].avgLogprobs.has_value());
    REQUIRE(candidates[0].avgLogprobs.value() == -0.1);
    REQUIRE(candidates[0].logprobsResult.has_value());
    REQUIRE(candidates[0].logprobsResult.value().topCandidates.Value().size() == 1);
    REQUIRE(candidates[0].logprobsResult.value().chosenCandidates.Value().size() == 1);

    // Content Parts
    auto parts = candidates[0].content.Value().parts.Value();
    REQUIRE(parts.size() == 2);
    REQUIRE(std::get_if<gemini::Text>(&parts[0].data.Value()) != nullptr);
    REQUIRE(std::get_if<gemini::FunctionCall>(&parts[1].data.Value()) != nullptr);

    // Grounding
    auto web_queries = candidates[0].groundingMetadata.Value().webSearchQueries.Value();
    REQUIRE(web_queries.size() == 1);
    REQUIRE(web_queries[0] == "search query");

    std::println("[SUCCESS] Full Response Deserialization passed.");
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
        test_thinking_config_serialization();
        test_tool_config_serialization();
        test_tool_variants_serialization();
        test_generation_config_serialization();
        test_code_execution_result_serialization();
        test_full_request_serialization();
        test_executable_code_deserialization();
        test_grounding_support_deserialization();
        test_citation_source_deserialization();
        test_maps_grounding_deserialization();
        test_url_context_metadata_deserialization();
        test_modality_token_count_deserialization();
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
