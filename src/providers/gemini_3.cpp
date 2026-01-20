#include "gemini_3.hpp"

#include <string_view>

#include <simdjson.h>


namespace jai::llm::gemini_3 {


constexpr std::string_view ENDPOINT_BASE = "https://generativelanguage.googleapis.com/v1beta/models/{MODEL_ID}:generateContent";
constexpr std::string_view ENDPOINT_VERTEX_BASE = "https://{REGION}-aiplatform.googleapis.com/v1/projects/{PROJECT_ID}/locations/{REGION}/publishers/google/models/{MODEL_ID}:generateContent";


constexpr std::string_view BASE_URL = "https://generativelanguage.googleapis.com/v1beta/models/";


http::Method GenMethod(const Request&) {
    return http::Method::POST;
}


http::RequestHeaders GenRequestHeaders(const Request&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"}
    }};
}


std::string GenUrl(const Request& r) {
    std::stringstream ss;
    ss << BASE_URL << r.model << ":generateContent";
    return ss.str();
}


std::vector<std::byte> Serialize(const Request& r) {
    simdjson::builder::string_builder sb;
    sb.start_object();

    if (!r.contents.empty()) {
        sb.escape_and_append_with_quotes("contents");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.contents.size(); ++i) {
            if (i > 0) sb.append_comma();
            const auto& c = r.contents[i];
            sb.start_object();
            sb.append_key_value("role", to_string_view(c.role));
            sb.append_comma();
            sb.escape_and_append_with_quotes("parts");
            sb.append_colon();
            sb.start_array();
            for (size_t j = 0; j < c.parts.size(); ++j) {
                if (j > 0) sb.append_comma();
                const auto& p = c.parts[j];
                sb.start_object();
                std::visit([&sb](auto const& part) {
                    using T = std::decay_t<decltype(part)>;
                    if constexpr (std::is_same_v<T, Content::ContentPart::TextPart>) {
                        sb.append_key_value("text", part.text);
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::InlineImagePart>) {
                        sb.escape_and_append_with_quotes("inlineData");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("mimeType", to_string_view(part.mime_type));
                        sb.append_comma();
                        sb.append_key_value("data", part.base64_data);
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::FileData>) {
                        sb.escape_and_append_with_quotes("fileData");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("mimeType", to_string_view(part.mime_type));
                        sb.append_comma();
                        sb.append_key_value("fileUri", part.file_uri);
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::FunctionCall>) {
                        sb.escape_and_append_with_quotes("functionCall");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("name", part.name);
                        sb.append_comma();
                        sb.escape_and_append_with_quotes("args");
                        sb.append_colon();
                        sb.append_raw(part.args); // Verbatim JSON
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::FunctionResponse>) {
                        sb.escape_and_append_with_quotes("functionResponse");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("name", part.name);
                        sb.append_comma();
                        sb.escape_and_append_with_quotes("response");
                        sb.append_colon();
                        sb.start_object();
                        sb.escape_and_append_with_quotes("content");
                        sb.append_colon();
                        sb.append_raw(part.response); // Verbatim JSON
                        sb.end_object();
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::ExecutableCode>) {
                        sb.escape_and_append_with_quotes("executableCode");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("language", to_string_view(part.language));
                        sb.append_comma();
                        sb.append_key_value("code", part.code);
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::CodeExecutionResult>) {
                        sb.escape_and_append_with_quotes("codeExecutionResult");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("outcome", to_string_view(part.outcome));
                        sb.append_comma();
                        sb.append_key_value("output", part.output);
                        sb.end_object();
                    } else if constexpr (std::is_same_v<T, Content::ContentPart::ThoughtSignature>) {
                        sb.escape_and_append_with_quotes("thought");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("signature", part.signature);
                        sb.end_object();
                    }
                }, p);
                sb.end_object();
            }
            sb.end_array();
            sb.end_object();
        }
        sb.end_array();
    }

    if (r.system_instruction) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("systemInstruction");
        sb.append_colon();
        sb.start_object();
        sb.escape_and_append_with_quotes("parts");
        sb.append_colon();
        sb.start_array();
        sb.start_object();
        sb.append_key_value("text", *r.system_instruction);
        sb.end_object();
        sb.end_array();
        sb.end_object();
    }

    if (r.generation_config) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("generationConfig");
        sb.append_colon();
        sb.start_object();
        bool first = true;
        if (r.generation_config->temperature) {
            if (!first) sb.append_comma();
            sb.append_key_value("temperature", *r.generation_config->temperature);
            first = false;
        }
        if (r.generation_config->top_p) {
            if (!first) sb.append_comma();
            sb.append_key_value("topP", *r.generation_config->top_p);
            first = false;
        }
        if (r.generation_config->top_k) {
            if (!first) sb.append_comma();
            sb.append_key_value("topK", *r.generation_config->top_k);
            first = false;
        }
        if (r.generation_config->max_output_tokens) {
            if (!first) sb.append_comma();
            sb.append_key_value("maxOutputTokens", *r.generation_config->max_output_tokens);
            first = false;
        }
        if (!r.generation_config->stop_sequences.empty()) {
            if (!first) sb.append_comma();
            sb.escape_and_append_with_quotes("stopSequences");
            sb.append_colon();
            sb.start_array();
            for (size_t i = 0; i < r.generation_config->stop_sequences.size(); ++i) {
                if (i > 0) sb.append_comma();
                sb.escape_and_append_with_quotes(r.generation_config->stop_sequences[i]);
            }
            sb.end_array();
            first = false;
        }
        if (r.generation_config->response_mime_type) {
            if (!first) sb.append_comma();
            sb.append_key_value("responseMimeType", to_string_view(*r.generation_config->response_mime_type));
            first = false;
        }
        if (r.generation_config->response_schema) {
            if (!first) sb.append_comma();
            sb.escape_and_append_with_quotes("responseSchema");
            sb.append_colon();
            sb.append_raw(*r.generation_config->response_schema); // Verbatim JSON schema
            first = false;
        }
        if (r.generation_config->thinking_config) {
            if (!first) sb.append_comma();
            sb.escape_and_append_with_quotes("thinkingConfig");
            sb.append_colon();
            sb.start_object();
            sb.append_key_value("thinkingLevel", to_string_view(r.generation_config->thinking_config->thinking_level));
            if (r.generation_config->thinking_config->include_thoughts) {
                sb.append_comma();
                sb.append_key_value("includeThoughts", *r.generation_config->thinking_config->include_thoughts);
            }
            sb.end_object();
            first = false;
        }
        sb.end_object();
    }

    if (!r.safety_settings.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("safetySettings");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.safety_settings.size(); ++i) {
            if (i > 0) sb.append_comma();
            sb.start_object();
            sb.append_key_value("category", to_string_view(r.safety_settings[i].category));
            sb.append_comma();
            sb.append_key_value("threshold", to_string_view(r.safety_settings[i].threshold));
            sb.end_object();
        }
        sb.end_array();
    }

    if (!r.tools.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("tools");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.tools.size(); ++i) {
            if (i > 0) sb.append_comma();
            sb.start_object();
            bool tool_first = true;
            if (!r.tools[i].function_declarations.empty()) {
                sb.escape_and_append_with_quotes("functionDeclarations");
                sb.append_colon();
                sb.start_array();
                for (size_t j = 0; j < r.tools[i].function_declarations.size(); ++j) {
                    if (j > 0) sb.append_comma();
                    const auto& fd = r.tools[i].function_declarations[j];
                    sb.start_object();
                    sb.append_key_value("name", fd.name);
                    if (fd.description) {
                        sb.append_comma();
                        sb.append_key_value("description", *fd.description);
                    }
                    if (fd.parameters) {
                        sb.append_comma();
                        sb.escape_and_append_with_quotes("parameters");
                        sb.append_colon();
                        sb.append_raw(*fd.parameters); // Verbatim JSON
                    }
                    sb.end_object();
                }
                sb.end_array();
                tool_first = false;
            }
            if (r.tools[i].code_execution) {
                if (!tool_first) sb.append_comma();
                sb.escape_and_append_with_quotes("codeExecution");
                sb.append_colon();
                sb.start_object();
                sb.end_object();
                tool_first = false;
            }
            if (r.tools[i].google_search) {
                if (!tool_first) sb.append_comma();
                sb.escape_and_append_with_quotes("googleSearch");
                sb.append_colon();
                sb.start_object();
                if (r.tools[i].google_search->dynamic_retrieval_config) {
                    sb.escape_and_append_with_quotes("dynamicRetrievalConfig");
                    sb.append_colon();
                    sb.start_object();
                    sb.append_key_value("mode", jai::llm::to_string_view(r.tools[i].google_search->dynamic_retrieval_config->mode));
                    if (r.tools[i].google_search->dynamic_retrieval_config->dynamic_threshold) {
                        sb.append_comma();
                        sb.append_key_value("dynamicThreshold", *r.tools[i].google_search->dynamic_retrieval_config->dynamic_threshold);
                    }
                    sb.end_object();
                }
                sb.end_object();
                tool_first = false;
            }
            sb.end_object();
        }
        sb.end_array();
    }

    if (r.tool_config) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("toolConfig");
        sb.append_colon();
        sb.start_object();
        sb.escape_and_append_with_quotes("functionCallingConfig");
        sb.append_colon();
        sb.start_object();
        sb.append_key_value("mode", to_string_view(r.tool_config->function_calling_config->mode));
        if (!r.tool_config->function_calling_config->allowed_function_names.empty()) {
            sb.append_comma();
            sb.escape_and_append_with_quotes("allowedFunctionNames");
            sb.append_colon();
            sb.start_array();
            for (size_t i = 0; i < r.tool_config->function_calling_config->allowed_function_names.size(); ++i) {
                if (i > 0) sb.append_comma();
                sb.escape_and_append_with_quotes(r.tool_config->function_calling_config->allowed_function_names[i]);
            }
            sb.end_array();
        }
        sb.end_object();
        sb.end_object();
    }

    if (r.cached_content) {
        sb.append_comma();
        sb.append_key_value("cachedContent", *r.cached_content);
    }

    sb.end_object();

    std::string str = sb;
    std::vector<std::byte> out;
    out.reserve(str.size());
    for (char c : str) out.push_back(static_cast<std::byte>(c));
    return out;
}


Response Deserialize(const std::vector<std::byte>& raw_response_bytes) {
    std::string json(reinterpret_cast<const char*>(raw_response_bytes.data()), raw_response_bytes.size());
    Response out;
    simdjson::ondemand::parser parser;
    try {
        auto doc = parser.iterate(json.data(), json.size(), json.size() + simdjson::SIMDJSON_PADDING);

        auto candidates = doc["candidates"].get_array();
        for (auto cand : candidates) {
            Candidate c;
            
            auto finish_reason = cand["finishReason"];
            if (finish_reason.error() == simdjson::SUCCESS) {
                std::string_view fr = finish_reason.get_string().value();
                if (fr == "STOP") c.finish_reason = FinishReason::STOP;
                else if (fr == "MAX_TOKENS") c.finish_reason = FinishReason::MAX_TOKENS;
                else if (fr == "SAFETY") c.finish_reason = FinishReason::SAFETY;
                else if (fr == "RECITATION") c.finish_reason = FinishReason::RECITATION;
                else if (fr == "OTHER") c.finish_reason = FinishReason::OTHER;
                else if (fr == "BLOCKLIST") c.finish_reason = FinishReason::BLOCKLIST;
                else if (fr == "PROHIBITED_CONTENT") c.finish_reason = FinishReason::PROHIBITED_CONTENT;
                else if (fr == "SPII") c.finish_reason = FinishReason::SPII;
                else if (fr == "MALFORMED_FUNCTION_CALL") c.finish_reason = FinishReason::MALFORMED_FUNCTION_CALL;
                else c.finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;
            }

            auto content = cand["content"];
            if (content.error() == simdjson::SUCCESS) {
                std::string_view role = content["role"].get_string().value();
                if (role == "user") c.content.role = Role::USER;
                else if (role == "model") c.content.role = Role::MODEL;
                else if (role == "system") c.content.role = Role::SYSTEM;

                auto parts = content["parts"].get_array();
                for (auto part : parts) {
                    auto text = part["text"];
                    if (text.error() == simdjson::SUCCESS) {
                        Content::ContentPart::TextPart tp;
                        tp.text = std::string(text.get_string().value());
                        c.content.parts.push_back(tp);
                    }
                    
                    auto func_call = part["functionCall"];
                    if (func_call.error() == simdjson::SUCCESS) {
                        Content::ContentPart::FunctionCall fc;
                        fc.name = std::string(func_call["name"].get_string().value());
                        auto args_obj = func_call["args"];
                        fc.args = std::string(simdjson::to_json_string(args_obj).value());
                        c.content.parts.push_back(fc);
                    }
                }
            }
            
            auto thought = cand["thought"];
            if (thought.error() == simdjson::SUCCESS) {
                c.thought = std::string(thought.get_string().value());
            }

            out.candidates.push_back(std::move(c));
        }

        auto usage = doc["usageMetadata"];
        if (usage.error() == simdjson::SUCCESS) {
            UsageMetadata um;
            um.prompt_token_count = static_cast<uint32_t>(usage["promptTokenCount"].get_uint64().value());
            um.candidates_token_count = static_cast<uint32_t>(usage["candidatesTokenCount"].get_uint64().value());
            um.total_token_count = static_cast<uint32_t>(usage["totalTokenCount"].get_uint64().value());
            
            auto reasoning = usage["reasoningTokenCount"];
            if (reasoning.error() == simdjson::SUCCESS) {
                um.reasoning_token_count = static_cast<uint32_t>(reasoning.get_uint64().value());
            }
            out.usage_metadata = um;
        }

    } catch (const std::exception&) {
    }
    return out;
}


}
