#pragma once

#include "../../llm_client.hpp"

namespace jai::llm {

    /**
     * @class OpenAIClient
     * @brief Provider-specific implementation for OpenAI's API.
     */
    class OpenAIClient : public LLMClient {
    public:
        OpenAIClient(std::shared_ptr<AdmissionController> admission, ClientConfig config);

        void Chat(ChatRequest req, ChatCallback cb) override;

    private:
        std::string SerializeChatRequest(const ChatRequest& req);
        ChatResponse DeserializeChatResponse(std::string_view json);
    };

} // namespace jai::llm
