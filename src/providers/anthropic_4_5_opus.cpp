#include "anthropic_4_5_opus.hpp"

#include <string_view>

#include <simdjson.h>

#include "../curl.hpp"


namespace jai::llm::anthropic_4_5_opus {


constexpr std::string_view ENDPOINT = "https://api.anthropic.com/v1/messages";
constexpr std::string_view API_VERSION = "2023-06-01"; // Default version for 4.5 series


http::Method GenMethod(const Request&) {
    return http::Method::POST;
}


http::RequestHeaders GenRequestHeaders(const Request&) {
    return http::RequestHeaders{std::vector<std::pair<std::string, std::string>>{
        {"Content-Type", "application/json"},
        {"anthropic-version", "2023-06-01"}
    }};
}


std::string GenUrl(const Request&) {
    return std::string{ENDPOINT};
}


std::vector<std::byte> Serialize(const Request& r) {
    simdjson::builder::string_builder sb;
    sb.start_object();
    sb.append_key_value("model", r.model);
    sb.append_comma();
    sb.append_key_value("max_tokens", r.max_tokens);

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
                            } else if constexpr (std::is_same_v<PT, Image>) {
                                sb.append_key_value("type", "image");
                                sb.append_comma();
                                sb.escape_and_append_with_quotes("source");
                                sb.append_colon();
                                sb.start_object();
                                std::visit([&sb](auto const& src) {
                                    using ST = std::decay_t<decltype(src)>;
                                    if constexpr (std::is_same_v<ST, Image::Source::Base64>) {
                                        sb.append_key_value("type", "base64");
                                        sb.append_comma();
                                        sb.append_key_value("media_type", src.media_type);
                                        sb.append_comma();
                                        sb.append_key_value("data", src.data);
                                    } else {
                                        sb.append_key_value("type", "url");
                                        sb.append_comma();
                                        sb.append_key_value("url", src.url.View());
                                    }
                                }, p.source.detail);
                                sb.end_object();
                            } else if constexpr (std::is_same_v<PT, Audio>) {
                                sb.append_key_value("type", "audio");
                                sb.append_comma();
                                sb.escape_and_append_with_quotes("source");
                                sb.append_colon();
                                sb.start_object();
                                sb.append_key_value("type", jai::llm::to_string_view(p.source.type));
                                sb.append_comma();
                                sb.append_key_value("media_type", p.source.media_type);
                                sb.append_comma();
                                sb.append_key_value("data", p.source.data);
                                sb.end_object();
                            } else if constexpr (std::is_same_v<PT, Document>) {
                                sb.append_key_value("type", "document");
                                sb.append_comma();
                                sb.escape_and_append_with_quotes("source");
                                sb.append_colon();
                                sb.start_object();
                                sb.append_key_value("type", jai::llm::to_string_view(p.source.type));
                                sb.append_comma();
                                sb.append_key_value("media_type", p.source.media_type);
                                sb.append_comma();
                                sb.append_key_value("data", p.source.data);
                                sb.end_object();
                            } else if constexpr (std::is_same_v<PT, ToolUse>) {
                                sb.append_key_value("type", "tool_use");
                                sb.append_comma();
                                sb.append_key_value("id", p.id);
                                sb.append_comma();
                                sb.append_key_value("name", p.name);
                                sb.append_comma();
                                sb.escape_and_append_with_quotes("input");
                                sb.append_colon();
                                sb.append_raw(p.input); // Verbatim JSON
                            } else if constexpr (std::is_same_v<PT, ToolResult>) {
                                sb.append_key_value("type", "tool_result");
                                sb.append_comma();
                                sb.append_key_value("tool_use_id", p.tool_use_id);
                                sb.append_comma();
                                sb.append_key_value("content", p.content);
                                if (p.is_error) {
                                    sb.append_comma();
                                    sb.append_key_value("is_error", *p.is_error);
                                }
                            }
                            
                            if (p.cache_control) {
                                sb.append_comma();
                                sb.escape_and_append_with_quotes("cache_control");
                                sb.append_colon();
                                sb.start_object();
                                sb.append_key_value("type", jai::llm::to_string_view(p.cache_control->type));
                                sb.end_object();
                            }
                        }, part);
                        sb.end_object();
                    }
                    sb.end_array();
                }
            }, m.content);
            sb.end_object();
        }
        sb.end_array();
    }

    std::visit([&sb](auto const& sys) {
        using T = std::decay_t<decltype(sys)>;
        if constexpr (std::is_same_v<T, std::string>) {
            if (!sys.empty()) {
                sb.append_comma();
                sb.append_key_value("system", sys);
            }
        } else {
            if (!sys.empty()) {
                sb.append_comma();
                sb.escape_and_append_with_quotes("system");
                sb.append_colon();
                sb.start_array();
                for (bool first = true; const auto& item : sys) {
                    if (!first) sb.append_comma();
                    first = false;

                    sb.start_object();
                    sb.append_key_value("type", "text");
                    sb.append_comma();
                    sb.append_key_value("text", item.text);
                    if (item.cache_control) {
                        sb.append_comma();
                        sb.escape_and_append_with_quotes("cache_control");
                        sb.append_colon();
                        sb.start_object();
                        sb.append_key_value("type", jai::llm::to_string_view(item.cache_control->type));
                        sb.end_object();
                    }
                    sb.end_object();
                }
                sb.end_array();
            }
        }
    }, r.system);

    if (r.thinking) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("thinking");
        sb.append_colon();
        sb.start_object();
        sb.append_key_value("type", "enabled");
        sb.append_comma();
        sb.append_key_value("budget_tokens", r.thinking->budget_tokens);
        sb.end_object();
    }
    if (r.effort) {
        sb.append_comma();
        sb.append_key_value("effort", jai::llm::to_string_view(*r.effort));
    }
    if (r.temperature) {
        sb.append_comma();
        sb.append_key_value("temperature", *r.temperature);
    }
    if (r.top_p) {
        sb.append_comma();
        sb.append_key_value("top_p", *r.top_p);
    }
    if (r.top_k) {
        sb.append_comma();
        sb.append_key_value("top_k", *r.top_k);
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
                sb.append_key_value("json_schema", fmt.schema);
            }
        }, r.response_format->detail);
        sb.end_object();
    }

    if (!r.stop_sequences.empty()) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("stop_sequences");
        sb.append_colon();
        sb.start_array();
        for (bool first = true; const auto& s : r.stop_sequences) {
            if (!first) sb.append_comma();
            first = false;
            sb.escape_and_append_with_quotes(s);
        }
        sb.end_array();
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
            std::visit([&sb](auto const& cfg) {
                using T = std::decay_t<decltype(cfg)>;
                if constexpr (std::is_same_v<T, Tool::Custom>) {
                    sb.append_key_value("name", cfg.name);
                    sb.append_comma();
                    sb.append_key_value("description", cfg.description);
                    sb.append_comma();
                    sb.escape_and_append_with_quotes("input_schema");
                    sb.append_colon();
                    sb.append_raw(cfg.input_schema); // Verbatim JSON
                } else if constexpr (std::is_same_v<T, Tool::Computer>) {
                    sb.append_key_value("type", "computer_20241022");
                    sb.append_comma();
                    sb.append_key_value("name", "computer");
                    sb.append_comma();
                    sb.append_key_value("display_width_px", cfg.display_width_px);
                    sb.append_comma();
                    sb.append_key_value("display_height_px", cfg.display_height_px);
                    if (cfg.display_number) {
                        sb.append_comma();
                        sb.append_key_value("display_number", *cfg.display_number);
                    }
                } else if constexpr (std::is_same_v<T, Tool::Bash>) {
                    sb.append_key_value("type", "bash_20241022");
                    sb.append_comma();
                    sb.append_key_value("name", "bash");
                } else if constexpr (std::is_same_v<T, Tool::TextEditor>) {
                    sb.append_key_value("type", "text_editor_20250124");
                    sb.append_comma();
                    sb.append_key_value("name", "str_replace_editor");
                }
            }, t.config);
            if (t.cache_control) {
                sb.append_comma();
                sb.escape_and_append_with_quotes("cache_control");
                sb.append_colon();
                sb.start_object();
                sb.append_key_value("type", jai::llm::to_string_view(t.cache_control->type));
                sb.end_object();
            }
            sb.end_object();
        }
        sb.end_array();
    }

    if (r.tool_choice) {
        sb.append_comma();
        sb.escape_and_append_with_quotes("tool_choice");
        sb.append_colon();
        sb.start_object();
        sb.append_key_value("type", jai::llm::to_string_view(r.tool_choice->type));
        if (r.tool_choice->name) {
            sb.append_comma();
            sb.append_key_value("name", *r.tool_choice->name);
        }
        if (r.tool_choice->disable_parallel_tool_use) {
            sb.append_comma();
            sb.append_key_value("disable_parallel_tool_use", *r.tool_choice->disable_parallel_tool_use);
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


Response Deserialize(const curl::Response& response) {
    if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
        throw std::runtime_error("Simdjson padding check failed");
    }

    static thread_local simdjson::ondemand::parser parser{};
    Response out{};

    try {
        const char* data_ptr = reinterpret_cast<const char*>(response.body.data());
        auto doc = parser.iterate(data_ptr, response.body_len, response.body.size());

        out.id = doc["id"].get_string().value();
        out.model = doc["model"].get_string().value();
        
        if (auto role = doc["role"].get_string(); role.error() == simdjson::SUCCESS) {
            out.role = from_string_view<Role>(role.value()).value_or(Role::ASSISTANT);
        }

        auto content = doc["content"].get_array();
        for (auto block : content) {
            std::optional<ContentBlockType> type{};
            if (auto type_str = block["type"].get_string(); type_str.error() == simdjson::SUCCESS) {
                type = from_string_view<ContentBlockType>(type_str.value());
            }

            if (!type) { continue; }

            if (type == ContentBlockType::TEXT) {
                out.content.emplace_back(ContentBlock::Text{std::string(block["text"].get_string().value())});
            } else if (type == ContentBlockType::THINKING) {
                std::string signature{};
                if (auto sig = block["signature"].get_string(); sig.error() == simdjson::SUCCESS) {
                    signature = sig.value();
                }
                out.content.emplace_back(ContentBlock::Thinking{
                    .thinking{std::string(block["thinking"].get_string().value())},
                    .signature{std::move(signature)}
                });
            } else if (type == ContentBlockType::TOOL_USE) {
                out.content.emplace_back(ContentBlock::ToolUse{
                    .id{std::string(block["id"].get_string().value())},
                    .name{std::string(block["name"].get_string().value())},
                    .input{simdjson::to_json_string(block["input"]).value()}
                });
            }

            if (auto citations = block["citations"]; citations.error() == simdjson::SUCCESS) {
                auto& back = out.content.back();
                for (auto cit : citations) {
                    auto& c = back.citations.emplace_back();
                    if (auto type_str = cit["type"].get_string(); type_str.error() == simdjson::SUCCESS) {
                        c.type = from_string_view<CitationType>(type_str.value()).value_or(CitationType::CHAR_LOCATION);
                    }
                    if (auto cited_text = cit["cited_text"].get_string(); cited_text.error() == simdjson::SUCCESS) {
                        c.cite = cited_text.value();
                    }
                    if (auto doc_idx = cit["document_index"].get_uint64(); doc_idx.error() == simdjson::SUCCESS) {
                        c.document_index = doc_idx.value();
                    }
                    if (auto doc_title = cit["document_title"].get_string(); doc_title.error() == simdjson::SUCCESS) {
                        c.document_title = std::string(doc_title.value());
                    }
                    if (auto start_index = cit["start_char_index"].get_uint64(); start_index.error() == simdjson::SUCCESS) {
                        c.start_index = start_index.value();
                    }
                    if (auto end_index = cit["end_char_index"].get_uint64(); end_index.error() == simdjson::SUCCESS) {
                        c.end_index = end_index.value();
                    }
                    if (auto start_page = cit["start_page_number"].get_uint64(); start_page.error() == simdjson::SUCCESS) {
                        c.start_page_number = start_page.value();
                    }
                    if (auto end_page = cit["end_page_number"].get_uint64(); end_page.error() == simdjson::SUCCESS) {
                        c.end_page_number = end_page.value();
                    }
                }
            }
        }

        Stop stop{};
        if (auto stop_reason = doc["stop_reason"]; stop_reason.error() == simdjson::SUCCESS && !stop_reason.is_null()) {
            stop.reason = from_string_view<StopReason>(stop_reason.get_string().value());
        }
        if (auto stop_seq = doc["stop_sequence"]; stop_seq.error() == simdjson::SUCCESS && !stop_seq.is_null()) {
            stop.sequence = stop_seq.get_string().value();
        }
        if (stop.reason.has_value() || stop.sequence.has_value()) {
            out.stop = std::move(stop);
        }

        auto usage = doc["usage"];
        out.usage.input_tokens = usage["input_tokens"].get_uint64().value();
        out.usage.output_tokens = usage["output_tokens"].get_uint64().value();
        if (auto thinking_tokens = usage["thinking_tokens"]; thinking_tokens.error() == simdjson::SUCCESS) {
            out.usage.thinking_tokens = thinking_tokens.get_uint64().value();
        }
        if (auto cache_create = usage["cache_creation_input_tokens"]; cache_create.error() == simdjson::SUCCESS) {
            out.usage.cache_creation_input_tokens = cache_create.get_uint64().value();
        }
        if (auto cache_read = usage["cache_read_input_tokens"]; cache_read.error() == simdjson::SUCCESS) {
            out.usage.cache_read_input_tokens = cache_read.get_uint64().value();
        }
    } catch (const std::exception&) {
    }

    return out;
}


}
