#include <vector>
#include <string>
#include <cassert>
#include <print>
#include <cstring>
#include <simdjson.h>
#include "../../src/curl.hpp"

#include "../../interface/protocols/gemini/generate_content.hpp"
#include "../../src/protocols/gemini.hpp"

using namespace jai::llm;

void test_simple_serialization() {
    std::println("Testing Simple Gemini Request Serialization...");

    gemini::Request req;
    
    gemini::RequestContent content;
    content.role = gemini::Role::USER;
    
    gemini::RequestContent::RequestPart part;
    part.data = gemini::Text{"Hello, Gemini!"};
    content.parts.push_back(part);
    
    req.contents.push_back(content);

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Basic structural checks
    assert(json_str.find("\"role\":\"user\"") != std::string::npos);
    assert(json_str.find("\"text\":\"Hello, Gemini!\"") != std::string::npos);
    assert(json_str.find("\"parts\":[") != std::string::npos);
    assert(json_str.find("\"contents\":[") != std::string::npos);

    std::println("[SUCCESS] Simple Serialization passed.");
}

void test_complex_serialization() {
    std::println("Testing Complex Gemini Request Serialization...");

    gemini::Request req;
    
    // System instruction
    gemini::RequestContent system_instr;
    gemini::RequestContent::RequestPart system_part;
    system_part.data = gemini::Text{"You are a helpful travel agent."};
    system_instr.parts.push_back(system_part);
    req.systemInstruction = system_instr;

    // Contents
    gemini::RequestContent user_content;
    user_content.role = gemini::Role::USER;
    
    gemini::RequestContent::RequestPart user_part;
    user_part.data = gemini::Text{"I want to go to Tokyo."};
    user_content.parts.push_back(user_part);
    
    req.contents.push_back(user_content);

    // Tools
    gemini::FunctionDeclaration get_flights{
        .name = Name64(std::string_view("get_flights")),
        .description = "Search for flights to a destination"
    };
    
    gemini::Schema params;
    params.type = gemini::SchemaType::OBJECT;
    
    gemini::Schema dest_prop;
    dest_prop.type = gemini::SchemaType::STRING;
    dest_prop.description = "The destination city";
    
    std::map<std::string, gemini::Schema> props;
    props["destination"] = dest_prop;
    params.properties = props;
    params.required = std::vector<std::string>{"destination"};
    
    get_flights.parameters = params;
    req.tools.push_back(get_flights);

    // Safety settings
    gemini::SafetySetting safety;
    safety.category = gemini::HarmCategory::HARM_CATEGORY_HARASSMENT;
    safety.threshold = gemini::HarmBlockThreshold::BLOCK_LOW_AND_ABOVE;
    req.safetySettings.push_back(safety);

    // Generation config
    gemini::GenerationConfig gen_config;
    gen_config.temperature = 0.5;
    gen_config.maxOutputTokens = 2048;
    gen_config.stopSequences = {"DONE", "FINISHED"};
    req.generationConfig = gen_config;

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    // Structural checks
    assert(json_str.find("\"systemInstruction\":{") != std::string::npos);
    assert(json_str.find("\"name\":\"get_flights\"") != std::string::npos);
    assert(json_str.find("\"category\":\"HARM_CATEGORY_HARASSMENT\"") != std::string::npos);
    assert(json_str.find("\"temperature\":0.5") != std::string::npos);
    assert(json_str.find("\"maxOutputTokens\":2048") != std::string::npos);

    std::println("[SUCCESS] Complex Serialization passed.");
}

void test_part_serialization() {
    std::println("Testing Gemini Part Serialization...");

    gemini::Request req;
    gemini::RequestContent content;
    content.role = gemini::Role::USER;

    // Blob (image)
    gemini::RequestContent::RequestPart blob_part;
    blob_part.data = gemini::Blob{gemini::MediaType::IMAGE_PNG, "YmFzZTY0X2RhdGE="};
    content.parts.push_back(blob_part);

    // FileData
    gemini::RequestContent::RequestPart file_part;
    file_part.data = gemini::FileData{gemini::MediaType::APPLICATION_PDF, "https://example.com/doc.pdf"};
    content.parts.push_back(file_part);

    // FunctionResponse
    gemini::RequestContent::RequestPart func_part;
    gemini::FunctionResponse fr{
        .name = Name64(std::string_view("get_weather")),
        .response = []() {
            json::Object o;
            o["temp"] = 72.0;
            o["condition"] = "Sunny";
            return o;
        }()
    };
    func_part.data = fr;
    content.parts.push_back(func_part);

    req.contents.push_back(content);

    auto serialized = gemini::Serialize(req);
    std::string json_str(reinterpret_cast<const char*>(serialized.data()), serialized.size());

    std::println("Serialized JSON: {}", json_str);

    assert(json_str.find("\"mimeType\":\"image/png\"") != std::string::npos);
    assert(json_str.find("\"fileUri\":\"https://example.com/doc.pdf\"") != std::string::npos);
    assert(json_str.find("\"name\":\"get_weather\"") != std::string::npos);
    assert(json_str.find("\"temp\":72.0") != std::string::npos);

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
                            "data": {"text": {"text": "I've checked the weather for you."}},
                            "partMetadata": {"videoMetadata": {}}
                        }
                    ]
                },
                "index": 0,
                "safetyRatings": [],
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
        "modelVersion": "gemini-1.5-pro",
        "responseId": "resp_123",
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

    assert(resp.candidates.size() == 1);
    assert(resp.candidates[0].content.role == gemini::Role::MODEL);
    
    auto* text_part = std::get_if<gemini::Text>(&resp.candidates[0].content.parts[0].data);
    assert(text_part != nullptr);
    assert(text_part->text == "I've checked the weather for you.");

    assert(resp.candidates[0].finishReason == gemini::FinishReason::STOP);
    assert(resp.candidates[0].safetyRatings.size() == 1);
    assert(resp.candidates[0].safetyRatings[0].category == gemini::HarmCategory::HARM_CATEGORY_HARASSMENT);
    
    assert(resp.usageMetadata.totalTokenCount == 30);

    std::println("[SUCCESS] Response Deserialization passed.");
}

int main() {
    try {
        test_simple_serialization();
        test_complex_serialization();
        test_part_serialization();
        test_response_deserialization();
        std::println("ALL GEMINI UNIT TESTS PASSED");
        return 0;
    } catch (const std::exception& e) {
        std::println("[ERROR] Test failed with exception: {}", e.what());
        return 1;
    } catch (...) {
        std::println("[ERROR] Test failed with unknown exception");
        return 1;
    }
}
