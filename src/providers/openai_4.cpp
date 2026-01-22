#include "openai_4.hpp"

#include <string_view>

#include <simdjson.h>

#include "../curl.hpp"


namespace jai::llm::openai_4 {


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
        for (bool first = true; const auto& m : r.messages) {
            if (!first) sb.append_comma();
            first = false;

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
                    for (bool f = true; const auto& part : content) {
                        if (!f) sb.append_comma();
                        f = false;

                        sb.start_object();
                        std::visit([&sb](auto const& p) {
                            using PT = std::decay_t<decltype(p)>;
                             if constexpr (std::is_same_v<PT, Text>) {
                                 sb.append_key_value("type", "text");
                                 sb.append_comma();
                                 sb.append_key_value("text", p.text);
                             } else {
                                 sb.append_key_value("type", "image_url");
                                 sb.append_comma();
                                 sb.escape_and_append_with_quotes("image_url");
                                 sb.append_colon();
                                 sb.start_object();
                                 sb.append_key_value("url", p.image_url.url.View());
                                 if (p.image_url.detail) {
                                     sb.append_comma();
                                     sb.append_key_value("detail", jai::llm::to_string_view(*p.image_url.detail));
                                 }
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
                for (bool f = true; const auto& tc : m.tool_calls) {
                    if (!f) sb.append_comma();
                    f = false;

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

    if (r.max_tokens) {
        sb.append_comma();
        sb.append_key_value("max_tokens", *r.max_tokens);
    }
    if (r.temperature) {
        sb.append_comma();
        sb.append_key_value("temperature", *r.temperature);
    }
    if (r.top_p) {
        sb.append_comma();
        sb.append_key_value("top_p", *r.top_p);
    }
    if (r.n) {
        sb.append_comma();
        sb.append_key_value("n", *r.n);
    }
    
    if (!r.stop.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("stop");
        sb.append_colon();
        sb.start_array();
        for (bool first = true; const auto& s : r.stop) {
            if (!first) sb.append_comma();
            first = false;
            sb.escape_and_append_with_quotes(s);
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
        for (bool first = true; const auto& t : r.tools) {
            if (!first) sb.append_comma();
            first = false;

            sb.start_object();
            sb.append_key_value("type", "function");
            sb.append_comma();
            sb.escape_and_append_with_quotes("function");
            sb.append_colon();
            sb.start_object();
            sb.append_key_value("name", t.function.name);
            if (t.function.description) {
                sb.append_comma();
                sb.append_key_value("description", *t.function.description);
            }
            sb.append_comma();
            sb.escape_and_append_with_quotes("parameters");
            sb.append_colon();
            sb.append_raw(t.function.parameters); // Verbatim JSON
            if (t.function.strict) {
                sb.append_comma();
                sb.append_key_value("strict", *t.function.strict);
            }
            sb.end_object();
            sb.end_object();
        }
        sb.end_array();
    }

    sb.end_object();

    std::string str = sb;
    std::vector<std::byte> out;
    out.reserve(str.size());
    for (char c : str) {
        out.push_back(static_cast<std::byte>(c));
    }
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

        if (std::string_view id; doc["id"].get(id) == simdjson::SUCCESS) {
            out.id = std::string(id);
        }
        if (std::string_view model; doc["model"].get(model) == simdjson::SUCCESS) {
            out.model = std::string(model);
        }
        if (uint64_t created; doc["created"].get(created) == simdjson::SUCCESS) {
            out.created = created;
        }
        
        if (std::string_view system_fingerprint; doc["system_fingerprint"].get(system_fingerprint) == simdjson::SUCCESS) {
            out.system_fingerprint = std::string(system_fingerprint);
        }

        if (simdjson::dom::array choices; doc["choices"].get(choices) == simdjson::SUCCESS) {
            for (auto choice : choices) {
                Response::Choice c{};
                if (uint64_t idx; choice["index"].get(idx) == simdjson::SUCCESS) {
                    c.index = static_cast<uint32_t>(idx);
                }
                
                if (std::string_view fr; choice["finish_reason"].get(fr) == simdjson::SUCCESS) {
                    c.finish_reason = from_string_view<FinishReason>(fr).value_or(FinishReason::FINISH_REASON_UNSPECIFIED);
                }

                if (simdjson::dom::element msg; choice["message"].get(msg) == simdjson::SUCCESS) {
                     if (std::string_view role; msg["role"].get(role) == simdjson::SUCCESS) {
                        c.message.role = from_string_view<Role>(role).value_or(Role::ASSISTANT);
                    }
                    if (std::string_view content; msg["content"].get(content) == simdjson::SUCCESS) {
                        c.message.content = std::string(content);
                    }
                    
                    if (simdjson::dom::array tool_calls; msg["tool_calls"].get(tool_calls) == simdjson::SUCCESS) {
                        for (auto tc : tool_calls) {
                            ToolCall call{};
                            if (std::string_view id; tc["id"].get(id) == simdjson::SUCCESS) call.id = std::string(id);
                            
                            if (simdjson::dom::element func; tc["function"].get(func) == simdjson::SUCCESS) {
                                if (std::string_view name; func["name"].get(name) == simdjson::SUCCESS) call.function.name = std::string(name);
                                if (std::string_view args; func["arguments"].get(args) == simdjson::SUCCESS) call.function.arguments = std::string(args);
                            }
                            c.message.tool_calls.push_back(std::move(call));
                        }
                    }
                }
                out.choices.push_back(std::move(c));
            }
        }

        if (simdjson::dom::element usage; doc["usage"].get(usage) == simdjson::SUCCESS) {
            if (uint64_t val; usage["prompt_tokens"].get(val) == simdjson::SUCCESS) out.usage.prompt_tokens = static_cast<uint32_t>(val);
            if (uint64_t val; usage["completion_tokens"].get(val) == simdjson::SUCCESS) out.usage.completion_tokens = static_cast<uint32_t>(val);
            if (uint64_t val; usage["total_tokens"].get(val) == simdjson::SUCCESS) out.usage.total_tokens = static_cast<uint32_t>(val);
            
            if (simdjson::dom::element prompt_details; usage["prompt_tokens_details"].get(prompt_details) == simdjson::SUCCESS) {
                 if (uint64_t val; prompt_details["cached_tokens"].get(val) == simdjson::SUCCESS) out.usage.prompt_tokens_details.cached_tokens = static_cast<uint32_t>(val);
            }
        }

    } catch (const std::exception&) {
    }
    return out;
}


}

