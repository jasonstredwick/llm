#include "openai_4o.hpp"

#include <string_view>

#include <simdjson.h>


namespace jai::llm::openai_4o {


constexpr std::string_view ENDPOINT = "https://api.openai.com/v1/chat/completions";


http::Method GenMethod(const Request&) {
    return http::Method::POST;
}


http::RequestHeaders GenRequestHeaders(const Request&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"}
    }};
}


std::string GenUrl(const Request&) {
    return std::string{ENDPOINT};
}


std::vector<std::byte> Serialize(const Request& r) {
    simdjson::builder::string_builder sb;
    sb.start_object();
    sb.append_key_value("model", r.model);

    if (!r.messages.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("messages");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.messages.size(); ++i) {
            if (i > 0) sb.append_comma();
            const auto& m = r.messages[i];
            sb.start_object();
            sb.append_key_value("role", jai::llm::to_string_view(m.role));
            
            sb.append_comma();
            std::visit([&sb](auto const& content) {
                using T = std::decay_t<decltype(content)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    sb.append_key_value("content", content);
                } else {
                    sb.escape_and_append_with_quotes("content");
                    sb.append_colon();
                    sb.start_array();
                    for (size_t j = 0; j < content.size(); ++j) {
                        if (j > 0) sb.append_comma();
                        const auto& part = content[j];
                        sb.start_object();
                        std::visit([&sb](auto const& p) {
                            using PT = std::decay_t<decltype(p)>;
                             if constexpr (std::is_same_v<PT, Text>) {
                                 sb.append_key_value("type", "text");
                                 sb.append_comma();
                                 sb.append_key_value("text", p.text);
                             } else if constexpr (std::is_same_v<PT, Image>) {
                                 sb.append_key_value("type", "image_url");
                                 sb.append_comma();
                                 sb.escape_and_append_with_quotes("image_url");
                                 sb.append_colon();
                                 sb.start_object();
                                 sb.append_key_value("url", p.image_url.url);
                                 if (p.image_url.detail) {
                                     sb.append_comma();
                                     sb.append_key_value("detail", jai::llm::to_string_view(*p.image_url.detail));
                                 }
                                 sb.end_object();
                             } else if constexpr (std::is_same_v<PT, Audio>) {
                                 sb.append_key_value("type", "input_audio");
                                 sb.append_comma();
                                 sb.escape_and_append_with_quotes("input_audio");
                                 sb.append_colon();
                                 sb.start_object();
                                 sb.append_key_value("data", p.audio.data);
                                 sb.append_comma();
                                 sb.append_key_value("format", p.audio.format);
                                 sb.end_object();
                             } else if constexpr (std::is_same_v<PT, Video>) {
                                 sb.append_key_value("type", "video");
                                 sb.append_comma();
                                 sb.escape_and_append_with_quotes("video_url");
                                 sb.append_colon();
                                 sb.start_object();
                                 sb.append_key_value("url", p.video_url.url);
                                 sb.end_object();
                             }
                        }, part);
                        sb.end_object();
                    }
                    sb.end_array();
                }
            }, m.content);

            if (m.name) {
                sb.append_comma();
                sb.append_key_value("name", *m.name);
            }
            if (m.tool_call_id) {
                sb.append_comma();
                sb.append_key_value("tool_call_id", *m.tool_call_id);
            }
            
            if (!m.tool_calls.empty()) {
                sb.append_comma();
                sb.escape_and_append_with_quotes("tool_calls");
                sb.append_colon();
                sb.start_array();
                for (size_t j = 0; j < m.tool_calls.size(); ++j) {
                    if (j > 0) sb.append_comma();
                    const auto& tc = m.tool_calls[j];
                    sb.start_object();
                    sb.append_key_value("id", tc.id);
                    sb.append_comma();
                    sb.append_key_value("type", "function");
                    sb.append_comma();
                    sb.escape_and_append_with_quotes("function");
                    sb.append_colon();
                    sb.start_object();
                    sb.append_key_value("name", tc.function.name);
                    sb.append_comma();
                    sb.append_key_value("arguments", tc.function.arguments);
                    sb.end_object();
                    sb.end_object();
                }
                sb.end_array();
            }
            sb.end_object();
        }
        sb.end_array();
    }

    if (r.max_completion_tokens) {
        sb.append_comma();
        sb.append_key_value("max_completion_tokens", *r.max_completion_tokens);
    }
    if (r.temperature) {
        sb.append_comma();
        sb.append_key_value("temperature", *r.temperature);
    }
    if (r.top_p) {
        sb.append_comma();
        sb.append_key_value("top_p", *r.top_p);
    }
    if (r.reasoning_effort) {
        sb.append_comma();
        sb.append_key_value("reasoning_effort", jai::llm::to_string_view(*r.reasoning_effort));
    }
    if (r.verbosity) {
        sb.append_comma();
        sb.append_key_value("verbosity", jai::llm::to_string_view(*r.verbosity));
    }
    if (r.compaction) {
        sb.append_comma();
        sb.append_key_value("compaction", *r.compaction);
    }
    if (r.prompt_cache_key) {
        sb.append_comma();
        sb.append_key_value("prompt_cache_key", *r.prompt_cache_key);
    }
    if (r.prompt_cache_retention) {
        sb.append_comma();
        sb.append_key_value("prompt_cache_retention", jai::llm::to_string_view(*r.prompt_cache_retention));
    }
    
    if (!r.stop.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("stop");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.stop.size(); ++i) {
            if (i > 0) sb.append_comma();
            sb.escape_and_append_with_quotes(r.stop[i]);
        }
        sb.end_array();
    }

    if (r.presence_penalty) {
        sb.append_comma();
        sb.append_key_value("presence_penalty", *r.presence_penalty);
    }
    if (r.frequency_penalty) {
        sb.append_comma();
        sb.append_key_value("frequency_penalty", *r.frequency_penalty);
    }
    if (r.user) {
        sb.append_comma();
        sb.append_key_value("user", *r.user);
    }
    if (r.seed) {
        sb.append_comma();
        sb.append_key_value("seed", *r.seed);
    }

    if (r.response_format) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("response_format");
        sb.append_colon();
        sb.start_object();
        std::visit([&sb](auto const& fmt) {
            using FT = std::decay_t<decltype(fmt)>;
            if constexpr (std::is_same_v<FT, ResponseFormat::Text>) {
                sb.append_key_value("type", "text");
            } else if constexpr (std::is_same_v<FT, ResponseFormat::JsonObject>) {
                sb.append_key_value("type", "json_object");
            } else {
                sb.append_key_value("type", "json_schema");
                sb.append_comma();
                sb.escape_and_append_with_quotes("json_schema");
                sb.append_colon();
                sb.start_object();
                sb.append_key_value("name", fmt.name);
                if (fmt.description) {
                    sb.append_comma();
                    sb.append_key_value("description", *fmt.description);
                }
                sb.append_comma();
                sb.escape_and_append_with_quotes("schema");
                sb.append_colon();
                sb.append_raw(fmt.schema); // Verbatim JSON
                if (fmt.strict) {
                    sb.append_comma();
                    sb.append_key_value("strict", *fmt.strict);
                }
                sb.end_object();
            }
        }, r.response_format->detail);
        sb.end_object();
    }

    if (!r.tools.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("tools");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.tools.size(); ++i) {
            if (i > 0) sb.append_comma();
            const auto& t = r.tools[i];
            sb.start_object();
            std::visit([&sb](auto const& det) {
                using DT = std::decay_t<decltype(det)>;
                if constexpr (std::is_same_v<DT, Tool::Function>) {
                    sb.append_key_value("type", "function");
                    sb.append_comma();
                    sb.escape_and_append_with_quotes("function");
                    sb.append_colon();
                    sb.start_object();
                    sb.append_key_value("name", det.name);
                    if (det.description) {
                        sb.append_comma();
                        sb.append_key_value("description", *det.description);
                    }
                    sb.append_comma();
                    sb.escape_and_append_with_quotes("parameters");
                    sb.append_colon();
                    sb.append_raw(det.parameters); // Verbatim JSON
                    if (det.strict) {
                        sb.append_comma();
                        sb.append_key_value("strict", *det.strict);
                    }
                    sb.end_object();
                } else if constexpr (std::is_same_v<DT, Tool::CodeInterpreter>) {
                    sb.append_key_value("type", "code_interpreter");
                } else if constexpr (std::is_same_v<DT, Tool::FileSearch>) {
                    sb.append_key_value("type", "file_search");
                    sb.append_comma();
                    sb.escape_and_append_with_quotes("file_search");
                    sb.append_colon();
                    sb.start_object();
                    sb.append_key_value("max_num_results", det.max_num_results);
                    sb.end_object();
                }
            }, t.detail);
            sb.end_object();
        }
        sb.end_array();
    }

    std::visit([&sb](auto const& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>) {
            sb.append_comma();
            sb.append_key_value("tool_choice", val);
        } else if constexpr (std::is_same_v<T, ToolChoiceSpecific>) {
            sb.append_comma();
            sb.escape_and_append_with_quotes("tool_choice");
            sb.append_colon();
            sb.start_object();
            sb.append_key_value("type", "function");
            sb.append_comma();
            sb.escape_and_append_with_quotes("function");
            sb.append_colon();
            sb.start_object();
            sb.append_key_value("name", val.function.name);
            sb.end_object();
            sb.end_object();
        }
    }, r.tool_choice);

    if (r.parallel_tool_calls) {
        sb.append_comma();
        sb.append_key_value("parallel_tool_calls", *r.parallel_tool_calls);
    }
    
    if (r.prediction) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("prediction");
        sb.append_colon();
        sb.start_object();
        sb.append_key_value("type", "content");
        sb.append_comma();
        std::visit([&sb](auto const& content) {
            using T = std::decay_t<decltype(content)>;
            if constexpr (std::is_same_v<T, std::string>) {
                sb.append_key_value("content", content);
            } else {
                sb.escape_and_append_with_quotes("content");
                sb.append_colon();
                sb.start_array();
                for (size_t j = 0; j < content.size(); ++j) {
                    if (j > 0) sb.append_comma();
                    const auto& part = content[j];
                    sb.start_object();
                    std::visit([&sb](auto const& p) {
                        using PT = std::decay_t<decltype(p)>;
                         if constexpr (std::is_same_v<PT, Text>) {
                             sb.append_key_value("type", "text");
                             sb.append_comma();
                             sb.append_key_value("text", p.text);
                         } else if constexpr (std::is_same_v<PT, Image>) {
                             sb.append_key_value("type", "image_url");
                             sb.append_comma();
                             sb.escape_and_append_with_quotes("image_url");
                             sb.append_colon();
                             sb.start_object();
                             sb.append_key_value("url", p.image_url.url);
                             sb.end_object();
                         }
                    }, part);
                    sb.end_object();
                }
                sb.end_array();
            }
        }, r.prediction->content);
        sb.end_object();
    }

    if (r.audio) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("audio");
        sb.append_colon();
        sb.start_object();
        sb.append_key_value("voice", r.audio->voice);
        sb.append_comma();
        sb.append_key_value("format", r.audio->format);
        sb.end_object();
    }

    if (!r.modalities.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("modalities");
        sb.append_colon();
        sb.start_array();
        for (size_t i = 0; i < r.modalities.size(); ++i) {
            if (i > 0) sb.append_comma();
            sb.escape_and_append_with_quotes(jai::llm::to_string_view(r.modalities[i]));
        }
        sb.end_array();
    }

    if (r.store) {
        sb.append_comma();
        sb.append_key_value("store", *r.store);
    }

    if (!r.metadata.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("metadata");
        sb.append_colon();
        sb.start_object();
        for (size_t i = 0; i < r.metadata.size(); ++i) {
            if (i > 0) sb.append_comma();
            sb.append_key_value(r.metadata[i].key, r.metadata[i].value);
        }
        sb.end_object();
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

        out.id = std::string(doc["id"].get_string().value());
        out.model = std::string(doc["model"].get_string().value());
        out.created = doc["created"].get_uint64().value();
        
        auto fingerprint = doc["system_fingerprint"];
        if (fingerprint.error() == simdjson::SUCCESS && !fingerprint.is_null()) {
            out.system_fingerprint = std::string(fingerprint.get_string().value());
        }

        auto choices = doc["choices"].get_array();
        for (auto choice : choices) {
            Choice c;
            c.index = static_cast<uint32_t>(choice["index"].get_uint64().value());
            
            std::string_view fr = choice["finish_reason"].get_string().value();
            if (fr == "stop") c.finish_reason = FinishReason::STOP;
            else if (fr == "length") c.finish_reason = FinishReason::LENGTH;
            else if (fr == "content_filter") c.finish_reason = FinishReason::CONTENT_FILTER;
            else if (fr == "tool_calls") c.finish_reason = FinishReason::TOOL_CALLS;
            else c.finish_reason = FinishReason::FINISH_REASON_UNSPECIFIED;

            auto msg = choice["message"];
            c.message.role = Role::ASSISTANT;
            
            auto content = msg["content"];
            if (content.error() == simdjson::SUCCESS && !content.is_null()) {
                c.message.content = std::string(content.get_string().value());
            }
            
            auto refusal = msg["refusal"];
            if (refusal.error() == simdjson::SUCCESS && !refusal.is_null()) {
                c.message.refusal = std::string(refusal.get_string().value());
            }

            auto reasoning = msg["reasoning_content"];
            if (reasoning.error() == simdjson::SUCCESS && !reasoning.is_null()) {
                c.message.reasoning_content = std::string(reasoning.get_string().value());
            }

            auto tool_calls = msg["tool_calls"];
            if (tool_calls.error() == simdjson::SUCCESS && tool_calls.type() == simdjson::ondemand::json_type::array) {
                for (auto tc : tool_calls.get_array()) {
                    ToolCall call;
                    call.id = std::string(tc["id"].get_string().value());
                    call.function.name = std::string(tc["function"]["name"].get_string().value());
                    call.function.arguments = std::string(tc["function"]["arguments"].get_string().value());
                    c.message.tool_calls.push_back(std::move(call));
                }
            }
            
            // Annotations and Audio could be added here if needed
            
            out.choices.push_back(std::move(c));
        }

        auto usage = doc["usage"];
        out.usage.prompt_tokens = static_cast<uint32_t>(usage["prompt_tokens"].get_uint64().value());
        out.usage.completion_tokens = static_cast<uint32_t>(usage["completion_tokens"].get_uint64().value());
        out.usage.total_tokens = static_cast<uint32_t>(usage["total_tokens"].get_uint64().value());
        
        auto prompt_details = usage["prompt_tokens_details"];
        if (prompt_details.error() == simdjson::SUCCESS && !prompt_details.is_null()) {
             out.usage.prompt_tokens_details.cached_tokens = static_cast<uint32_t>(prompt_details["cached_tokens"].get_uint64().value());
        }

    } catch (const std::exception&) {
    }
    return out;
}


}
