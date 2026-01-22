#include "gemini_3.hpp"

#include <string_view>

#include <simdjson.h>

#include "../curl.hpp"


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
        for (bool first = true; const auto& c : r.contents) {
            if (!first) sb.append_comma();
            first = false;

            sb.start_object();
            sb.append_key_value("role", to_string_view(c.role));
            sb.append_comma();
            sb.escape_and_append_with_quotes("parts");
            sb.append_colon();
            sb.start_array();
            for (bool f = true; const auto& p : c.parts) {
                if (!f) sb.append_comma();
                f = false;

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
                        sb.append_key_value("fileUri", part.file_uri.View());
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
            for (bool f = true; const auto& s : r.generation_config->stop_sequences) {
                if (!f) sb.append_comma();
                f = false;
                sb.escape_and_append_with_quotes(s);
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
        for (bool first = true; const auto& s : r.safety_settings) {
            if (!first) sb.append_comma();
            first = false;
            sb.start_object();
            sb.append_key_value("category", to_string_view(s.category));
            sb.append_comma();
            sb.append_key_value("threshold", to_string_view(s.threshold));
            sb.end_object();
        }
        sb.end_array();
    }

    if (!r.tools.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("tools");
        sb.append_colon();
        sb.start_array();
        for (bool first = true; const auto& tool : r.tools) {
            if (!first) sb.append_comma();
            first = false;

            sb.start_object();
            bool tool_first = true;
            if (!tool.function_declarations.empty()) {
                sb.escape_and_append_with_quotes("functionDeclarations");
                sb.append_colon();
                sb.start_array();
                for (bool f = true; const auto& fd : tool.function_declarations) {
                    if (!f) sb.append_comma();
                    f = false;
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
            if (tool.code_execution) {
                if (!tool_first) sb.append_comma();
                sb.escape_and_append_with_quotes("codeExecution");
                sb.append_colon();
                sb.start_object();
                sb.end_object();
                tool_first = false;
            }
            if (tool.google_search) {
                if (!tool_first) sb.append_comma();
                sb.escape_and_append_with_quotes("googleSearch");
                sb.append_colon();
                sb.start_object();
                if (tool.google_search->dynamic_retrieval_config) {
                    sb.escape_and_append_with_quotes("dynamicRetrievalConfig");
                    sb.append_colon();
                    sb.start_object();
                    sb.append_key_value("mode", jai::llm::to_string_view(tool.google_search->dynamic_retrieval_config->mode));
                    if (tool.google_search->dynamic_retrieval_config->dynamic_threshold) {
                        sb.append_comma();
                        sb.append_key_value("dynamicThreshold", *tool.google_search->dynamic_retrieval_config->dynamic_threshold);
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
            for (bool first = true; const auto& name : r.tool_config->function_calling_config->allowed_function_names) {
                if (!first) sb.append_comma();
                first = false;
                sb.escape_and_append_with_quotes(name);
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


Response Deserialize(const curl::Response& response) {
    if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
        throw std::runtime_error("Simdjson padding check failed");
    }

    static thread_local simdjson::dom::parser parser{};
    Response out{};

    try {
        simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()), response.body_len);

        if (simdjson::dom::array candidates_arr; doc["candidates"].get(candidates_arr) == simdjson::SUCCESS) {
            for (auto cand : candidates_arr) {
                Candidate& c = out.candidates.emplace_back();

                if (uint64_t idx; cand["index"].get(idx) == simdjson::SUCCESS) {
                    c.index = static_cast<uint32_t>(idx);
                }

                if (std::string_view fr; cand["finishReason"].get(fr) == simdjson::SUCCESS) {
                    c.finish_reason = from_string_view<FinishReason>(fr).value_or(FinishReason::FINISH_REASON_UNSPECIFIED);
                }

                if (simdjson::dom::element content; cand["content"].get(content) == simdjson::SUCCESS) {
                    c.content.role = Role::MODEL;
                    if (std::string_view role; content["role"].get(role) == simdjson::SUCCESS) {
                        c.content.role = from_string_view<Role>(role).value_or(Role::MODEL);
                    }
                    if (simdjson::dom::array parts; content["parts"].get(parts) == simdjson::SUCCESS) {
                        for (auto part : parts) {
                            if (std::string_view text; part["text"].get(text) == simdjson::SUCCESS) {
                                c.content.parts.push_back(Content::ContentPart::TextPart{std::string(text)});
                            } else if (simdjson::dom::element func_call; part["functionCall"].get(func_call) == simdjson::SUCCESS) {
                                Content::ContentPart::FunctionCall fc{};
                                if (std::string_view name; func_call["name"].get(name) == simdjson::SUCCESS) {
                                    fc.name = std::string(name);
                                }
                                if (simdjson::dom::element args; func_call["args"].get(args) == simdjson::SUCCESS) {
                                    fc.args = std::string(simdjson::minify(args));
                                }
                                c.content.parts.push_back(std::move(fc));
                            } else if (simdjson::dom::element exec_code; part["executableCode"].get(exec_code) == simdjson::SUCCESS) {
                                Content::ContentPart::ExecutableCode ec{};
                                if (std::string_view lang; exec_code["language"].get(lang) == simdjson::SUCCESS) {
                                    ec.language = from_string_view<CodeLanguage>(lang).value_or(CodeLanguage::UNSPECIFIED);
                                }
                                if (std::string_view code; exec_code["code"].get(code) == simdjson::SUCCESS) {
                                    ec.code = std::string(code);
                                }
                                c.content.parts.push_back(std::move(ec));
                            } else if (std::string_view thought_text; part["thought"].get(thought_text) == simdjson::SUCCESS) {
                                c.content.parts.push_back(Content::ContentPart::Thought{std::string(thought_text)});
                            } else if (simdjson::dom::element thought_sig; part["thoughtSignature"].get(thought_sig) == simdjson::SUCCESS) {
                                Content::ContentPart::ThoughtSignature ts{};
                                if (std::string_view sig; thought_sig["signature"].get(sig) == simdjson::SUCCESS) {
                                    ts.signature = std::string(sig);
                                }
                                c.content.parts.push_back(std::move(ts));
                            }
                        }
                    }
                }

                if (simdjson::dom::array ratings; cand["safetyRatings"].get(ratings) == simdjson::SUCCESS) {
                    for (auto rating : ratings) {
                        SafetyRating sr{};
                        if (std::string_view cat; rating["category"].get(cat) == simdjson::SUCCESS) {
                            sr.category = from_string_view<HarmCategory>(cat).value_or(HarmCategory::UNSPECIFIED);
                        }
                        if (std::string_view prob; rating["probability"].get(prob) == simdjson::SUCCESS) {
                            sr.probability = from_string_view<HarmProbability>(prob).value_or(HarmProbability::UNSPECIFIED);
                        }
                        if (bool blocked; rating["blocked"].get(blocked) == simdjson::SUCCESS) {
                            sr.blocked = blocked;
                        }
                        c.safety_ratings.push_back(std::move(sr));
                    }
                }

                if (simdjson::dom::element citation_meta; cand["citationMetadata"].get(citation_meta) == simdjson::SUCCESS) {
                    CitationMetadata cm{};
                    if (simdjson::dom::array sources; citation_meta["citationSources"].get(sources) == simdjson::SUCCESS) {
                        for (auto src : sources) {
                            uint32_t start = 0, end = 0;
                            std::string uri_str{}, license{};
                            if (uint64_t val; src["startIndex"].get(val) == simdjson::SUCCESS) start = static_cast<uint32_t>(val);
                            if (uint64_t val; src["endIndex"].get(val) == simdjson::SUCCESS) end = static_cast<uint32_t>(val);
                            if (std::string_view val; src["uri"].get(val) == simdjson::SUCCESS) uri_str = std::string(val);
                            if (std::string_view val; src["license"].get(val) == simdjson::SUCCESS) license = std::string(val);
                            
                            if (!uri_str.empty()) {
                                cm.citation_sources.emplace_back(start, end, EncodedUrl{uri_str},std::move(license));
                            }
                        }
                    }
                    c.citation_metadata = std::move(cm);
                }

                if (simdjson::dom::element grounding_meta; cand["groundingMetadata"].get(grounding_meta) == simdjson::SUCCESS) {
                    GroundingMetadata gm{};
                    if (simdjson::dom::array chunks; grounding_meta["groundingChunks"].get(chunks) == simdjson::SUCCESS) {
                        for (auto chunk : chunks) {
                            if (simdjson::dom::element web; chunk["web"].get(web) == simdjson::SUCCESS) {
                                std::string uri_str{}, title{};
                                if (std::string_view val; web["uri"].get(val) == simdjson::SUCCESS) uri_str = std::string(val);
                                if (std::string_view val; web["title"].get(val) == simdjson::SUCCESS) title = std::string(val);
                                if (!uri_str.empty()) {
                                    gm.grounding_chunks.push_back(GroundingMetadata::GroundingChunk{
                                        .web = { .uri = EncodedUrl{uri_str}, .title = std::move(title) }
                                    });
                                }
                            }
                        }
                    }
                    if (simdjson::dom::array supports; grounding_meta["groundingSupports"].get(supports) == simdjson::SUCCESS) {
                        for (auto support : supports) {
                            GroundingMetadata::GroundingSupport gs{};
                            if (simdjson::dom::array indices; support["groundingChunkIndices"].get(indices) == simdjson::SUCCESS) {
                                for (auto idx : indices) {
                                    uint64_t val; 
                                    if (idx.get(val) == simdjson::SUCCESS) gs.grounding_chunk_indices.push_back(static_cast<uint32_t>(val));
                                }
                            }
                            if (double score; support["confidenceScore"].get(score) == simdjson::SUCCESS) gs.confidence_score = score;
                            if (uint64_t val; support["segment"]["startIndex"].get(val) == simdjson::SUCCESS) gs.segment_start_index = static_cast<uint32_t>(val);
                            if (uint64_t val; support["segment"]["endIndex"].get(val) == simdjson::SUCCESS) gs.segment_end_index = static_cast<uint32_t>(val);
                            if (std::string_view val; support["segment"]["text"].get(val) == simdjson::SUCCESS) gs.segment_text = std::string(val);
                            gm.grounding_supports.push_back(std::move(gs));
                        }
                    }
                    if (simdjson::dom::array queries; grounding_meta["webSearchQueries"].get(queries) == simdjson::SUCCESS) {
                        for (auto query : queries) {
                            std::string_view q;
                            if (query.get(q) == simdjson::SUCCESS) gm.web_search_queries.emplace_back(q);
                        }
                    }
                    if (simdjson::dom::element sep; grounding_meta["searchEntryPoint"].get(sep) == simdjson::SUCCESS) {
                        GroundingMetadata::SearchEntryPoint s{};
                        if (std::string_view val; sep["renderedContent"].get(val) == simdjson::SUCCESS) s.rendered_content = std::string(val);
                        gm.search_entry_point = std::move(s);
                    }
                    c.grounding_metadata = std::move(gm);
                }

                if (simdjson::dom::element logprobs_json; cand["logprobsResult"].get(logprobs_json) == simdjson::SUCCESS) {
                    LogprobsResult lr{};
                    auto parse_lpc = [](simdjson::dom::element json_obj) -> LogprobsResult::LogprobCandidate {
                        LogprobsResult::LogprobCandidate lpc{};
                        if (std::string_view val; json_obj["token"].get(val) == simdjson::SUCCESS) lpc.token = std::string(val);
                        if (double lp; json_obj["logProbability"].get(lp) == simdjson::SUCCESS) lpc.log_probability = lp;
                        // Google Gemini REST API docs don't mention 'bytes' as an array of numbers, but if they appear in future:
                        if (simdjson::dom::array b; json_obj["bytes"].get(b) == simdjson::SUCCESS) {
                            for (auto byte_val : b) {
                                uint64_t v;
                                if (byte_val.get(v) == simdjson::SUCCESS) lpc.bytes.push_back(static_cast<uint8_t>(v));
                            }
                        }
                        return lpc;
                    };
                    if (simdjson::dom::array chosen; logprobs_json["chosenCandidates"].get(chosen) == simdjson::SUCCESS) {
                        for (auto cand_obj : chosen) lr.chosen_candidates.push_back(parse_lpc(cand_obj));
                    }
                    if (simdjson::dom::array top; logprobs_json["topCandidates"].get(top) == simdjson::SUCCESS) {
                        for (auto top_step : top) {
                            auto& out_vec = lr.top_candidates.emplace_back();
                            if (simdjson::dom::array candidates; top_step["candidates"].get(candidates) == simdjson::SUCCESS) {
                                for (auto cand_obj : candidates) out_vec.push_back(parse_lpc(cand_obj));
                            }
                        }
                    }
                    c.logprobs_result = std::move(lr);
                }

                if (double avg_lp; cand["avgLogprobs"].get(avg_lp) == simdjson::SUCCESS) {
                    c.avg_logprobs = avg_lp;
                }

                if (std::string_view thought_sv; cand["thought"].get(thought_sv) == simdjson::SUCCESS) {
                    c.thought = std::string(thought_sv);
                }
            }
        }

        if (simdjson::dom::element feedback; doc["promptFeedback"].get(feedback) == simdjson::SUCCESS) {
            PromptFeedback pf{};
            if (std::string_view br; feedback["blockReason"].get(br) == simdjson::SUCCESS) {
                pf.block_reason = std::string(br);
            }
            if (simdjson::dom::array ratings; feedback["safetyRatings"].get(ratings) == simdjson::SUCCESS) {
                for (auto rating : ratings) {
                    PromptFeedback::SafetyRatingDetail srd{};
                    if (std::string_view val; rating["category"].get(val) == simdjson::SUCCESS) {
                        srd.category = from_string_view<HarmCategory>(val).value_or(HarmCategory::UNSPECIFIED);
                    }
                    if (std::string_view val; rating["probability"].get(val) == simdjson::SUCCESS) {
                        srd.probability = from_string_view<HarmProbability>(val).value_or(HarmProbability::UNSPECIFIED);
                    }
                    if (bool blocked; rating["blocked"].get(blocked) == simdjson::SUCCESS) {
                        srd.blocked = blocked;
                    }
                    pf.safety_ratings.push_back(std::move(srd));
                }
            }
            out.prompt_feedback = std::move(pf);
        }

        if (simdjson::dom::element usage; doc["usageMetadata"].get(usage) == simdjson::SUCCESS) {
            UsageMetadata um{};
            if (uint64_t val; usage["promptTokenCount"].get(val) == simdjson::SUCCESS) um.prompt_token_count = val;
            if (uint64_t val; usage["candidatesTokenCount"].get(val) == simdjson::SUCCESS) um.candidates_token_count = val;
            if (uint64_t val; usage["totalTokenCount"].get(val) == simdjson::SUCCESS) um.total_token_count = val;
            if (uint64_t val; usage["reasoningTokenCount"].get(val) == simdjson::SUCCESS) um.reasoning_token_count = val;
            if (uint64_t val; usage["cachedContentTokenCount"].get(val) == simdjson::SUCCESS) um.cached_content_token_count = val;
            
            auto parse_details = [](simdjson::dom::element json_arr_elem, std::vector<UsageMetadata::TokenCountDetails>& out_vec) {
                simdjson::dom::array arr;
                if (json_arr_elem.get(arr) != simdjson::SUCCESS) return;
                for (auto details : arr) {
                    UsageMetadata::TokenCountDetails d{};
                    if (uint64_t v; details["textTokenCount"].get(v) == simdjson::SUCCESS) d.text_token_count = v;
                    if (uint64_t v; details["imageTokenCount"].get(v) == simdjson::SUCCESS) d.image_token_count = v;
                    if (uint64_t v; details["videoTokenCount"].get(v) == simdjson::SUCCESS) d.video_token_count = v;
                    if (uint64_t v; details["audioTokenCount"].get(v) == simdjson::SUCCESS) d.audio_token_count = v;
                    out_vec.push_back(d);
                }
            };
            parse_details(usage["promptTokenCountDetails"], um.prompt_token_count_details);
            parse_details(usage["candidatesTokenCountDetails"], um.candidates_token_count_details);
            
            out.usage_metadata = std::move(um);
        }

        if (std::string_view val; doc["modelVersion"].get(val) == simdjson::SUCCESS) {
            out.model_version = std::string(val);
        }

    } catch (const std::exception&) {
    }
    return out;
}


}
