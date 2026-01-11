#include "openai_client.hpp"
#include "simdjson.h"
#include <sstream>
#include <iomanip>

namespace jai::llm {

    OpenAIClient::OpenAIClient(std::shared_ptr<AdmissionController> admission, ClientConfig config)
        : LLMClient(std::move(admission), std::move(config)) {}

    void OpenAIClient::Chat(ChatRequest req, ChatCallback cb) {
        HttpRequest http_req;
        http_req.url = config_.base_url.empty() ? "https://api.openai.com/v1/chat/completions" : config_.base_url;
        http_req.method = "POST";
        http_req.headers.emplace("Authorization", "Bearer " + config_.api_key);
        http_req.headers.emplace("Content-Type", "application/json");

        if (!config_.organization_id.empty()) {
            http_req.headers.emplace("OpenAI-Organization", config_.organization_id);
        }

        http_req.credential_id = "openai:" + config_.api_key.substr(0, std::min(size_t(8), config_.api_key.size()));
        http_req.body = SerializeChatRequest(req);

        // Wrap the completion to deserialize
        http_req.on_complete = [this, cb](const HttpResponse& res) {
            if (res.status_code == 200) {
                cb(DeserializeChatResponse(res.body));
            } else {
                ChatResponse fail;
                fail.success = false;
                fail.error = res.error_message.empty() ? "HTTP Error " + std::to_string(res.status_code) : res.error_message;
                fail.status_code = res.status_code; // Wait, I didn't add status_code to ChatResponse. Let's fix that.
                cb(fail);
            }
        };

        Submit(std::move(http_req));
    }

    std::string OpenAIClient::SerializeChatRequest(const ChatRequest& req) {
        std::stringstream ss;
        ss << "{"
           << "\"model\":\"" << req.model << "\","
           << "\"messages\": [";

        for (size_t i = 0; i < req.messages.size(); ++i) {
            const auto& msg = req.messages[i];
            ss << "{\"role\":\"";
            switch (msg.role) {
                case Role::System: ss << "system"; break;
                case Role::User: ss << "user"; break;
                case Role::Assistant: ss << "assistant"; break;
                case Role::Tool: ss << "tool"; break;
            }
            ss << "\",\"content\":\"";
            // Simple escaping helper for quotes and backslashes
            for (char c : msg.content) {
                if (c == '\"') ss << "\\\"";
                else if (c == '\\') ss << "\\\\";
                else if (c == '\n') ss << "\\n";
                else ss << c;
            }
            ss << "\"}";
            if (i < req.messages.size() - 1) ss << ",";
        }

        ss << "],"
           << "\"temperature\":" << req.temperature << ","
           << "\"max_tokens\":" << req.max_tokens << ","
           << "\"stream\":" << (req.stream ? "true" : "false")
           << "}";

        return ss.str();
    }

    ChatResponse OpenAIClient::DeserializeChatResponse(std::string_view json) {
        ChatResponse out;
        simdjson::ondemand::parser parser;
        try {
            auto doc = parser.iterate(json.data(), json.size(), json.size() + simdjson::SIMDJSON_PADDING);

            // OpenAI format: choice[0].message.content
            auto choices = doc["choices"].get_array();
            for (auto choice : choices) {
                out.content = std::string(choice["message"]["content"].get_string().value());
                break; // Just get first choice
            }

            auto usage = doc["usage"];
            out.usage_prompt_tokens = static_cast<int>(usage["prompt_tokens"].get_uint64().value());
            out.usage_completion_tokens = static_cast<int>(usage["completion_tokens"].get_uint64().value());

            out.model = std::string(doc["model"].get_string().value());
            out.success = true;
        } catch (const std::exception& e) {
            out.success = false;
            out.error = "JSON Parsing Error: " + std::string(e.what());
        }
        return out;
    }

} // namespace jai::llm
