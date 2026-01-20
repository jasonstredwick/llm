#pragma once


#include <array>
#include <cstdint>
#include <cstddef>
#include <string>
#include <variant>

#include "async.hpp"


namespace jai::llm {


// ModelContract identifies a semantically distinct model interface
// that requires specific request/response and reasoning handling.
enum class ModelContract {
    Anthropic_Claude_Opus_4_5,
    Anthropic_Claude_Sonnet_4_5,
    Gemini_2_5,
    Gemini_3,
    OpenAI_4,
    OpenAI_4o,
    OpenAI_5,
};


struct AuthAPIKey {
    std::string api_key;
};


struct AuthGoogleCredentials {
    std::string credentials;
    std::string project;
    std::string location;
};


using Auth = std::variant<AuthAPIKey, AuthGoogleCredentials>;


struct AnthropicMetadata {};
struct GeminiMetadata {};
struct OpenAIMetadata {};
using Metadata = std::variant<AnthropicMetadata, GeminiMetadata, OpenAIMetadata>;


struct Result {};
struct Request {};
struct Policy {};
constexpr size_t Size = 64;
constexpr size_t Align = 8;

class Client {
private:
    struct VTable {
        Result (*Call)(const void*, const Request&, const Policy&);
        void   (*destroy)(void*);
        void   (*move)(void* dst, void* src);
    };
    const VTable* vtable{nullptr};
    alignas(Align) std::array<std::byte, Size> storage{};

public:
    explicit Client(ModelContract provider_family, Auth auth, Metadata metadata);

    Client(const Client&) = delete;

    Client(Client&& other) noexcept : vtable(other.vtable) {
        if (vtable) {
            vtable->move(std::addressof(storage), std::addressof(other.storage));
            other.vtable = nullptr;
        }
    }

    Client& operator=(const Client&) = delete;

    Client& operator=(Client&& other) noexcept {
        if (this != &other) {
            if (vtable) { vtable->destroy(std::addressof(storage)); }
            vtable = other.vtable;
            if (vtable) {
                vtable->move(std::addressof(storage), std::addressof(other.storage));
                other.vtable = nullptr;
            }
        }
        return *this;
    }

    ~Client() { if (vtable) { vtable->destroy(storage.data()); } }

    Result Call(const Request& r, const Policy& p) const { return vtable->Call(std::addressof(storage), r, p); }
};


} // namespace jai::llm
