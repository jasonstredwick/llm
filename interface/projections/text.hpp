#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../llm.hpp"


namespace jai::llm::proj::text {


enum class ThinkingEffort { NONE, MINIMAL, LOW, MEDIUM, HIGH, MAX };


struct Image {
    std::string media_type{};
    std::string base64_content{};
};

struct Options {
    std::optional<int64_t> max_output_tokens{};
    std::optional<ThinkingEffort> thinking_effort{};
    std::optional<double> temperature{};
    std::optional<int64_t> top_k{};
    std::optional<double> top_p{};
};

struct Prompt {
    std::string text{};
};


using Block = std::variant<Image, Prompt>;


constexpr const Options default_options{
    .thinking_effort=ThinkingEffort::MINIMAL,
    .temperature=1.0
};


struct Result {
    std::string text{};
};


template <typename Endpoint>
typename Endpoint::Request_t Generate(std::optional<Prompt> system_prompt,
                                      const std::vector<Block>& content,
                                      Options options = default_options);

template <typename Endpoint>
Result Extract(typename Endpoint::Response_t const& response);


// --- ClientHandle ---
// Lightweight wrapper around jai::llm::Instance::ClientHandle that provides a
// simplified text-oriented interface. Generate builds the provider request from
// projection inputs; Extract converts the provider response to text.
//
// Two modes:
//   1) Text extraction — returns Result<Endpoint, Result> where Result.text
//      is the extracted string.
//   2) User transform — user provides Data (*)(const Result&) to convert the
//      extracted text into their own type. Returns Result<Endpoint, Data>.
//      (Requires std::function for composition — deferred.)

template <typename Endpoint>
class ClientHandle {
    jai::llm::Instance::ClientHandle<Endpoint> handle;

public:
    explicit ClientHandle(jai::llm::Instance::ClientHandle<Endpoint> h)
        : handle{std::move(h)} {}

    ClientHandle(const ClientHandle&) = default;
    ClientHandle(ClientHandle&&) noexcept = default;
    ClientHandle& operator=(const ClientHandle&) = default;
    ClientHandle& operator=(ClientHandle&&) noexcept = default;
    ~ClientHandle() = default;

    // --- Text extraction ---

    jai::llm::AsyncResult<Endpoint, Result>
    CallAsync(std::optional<Prompt> system_prompt,
              const std::vector<Block>& content,
              Options options = default_options,
              const jai::llm::AttemptPolicy& policy = {}) const
    {
        auto request = Generate<Endpoint>(
            std::move(system_prompt), content, options);
        return handle.CallAsync(request, &Extract<Endpoint>, policy);
    }

    jai::llm::Result<Endpoint, Result>
    CallSync(std::optional<Prompt> system_prompt,
             const std::vector<Block>& content,
             Options options = default_options,
             const jai::llm::AttemptPolicy& policy = {}) const
    {
        auto request = Generate<Endpoint>(
            std::move(system_prompt), content, options);
        return handle.CallSync(request, &Extract<Endpoint>, policy);
    }

    // --- User transform ---
    // Composes Extract<Endpoint> with the user's transform into a single
    // callable (Response_t → Result → Data) via MoveFunction.

    template <typename Data>
    jai::llm::AsyncResult<Endpoint, Data>
    CallAsync(std::optional<Prompt> system_prompt,
              const std::vector<Block>& content,
              Data (*transform)(const Result&),
              Options options = default_options,
              const jai::llm::AttemptPolicy& policy = {}) const
    {
        auto request = Generate<Endpoint>(
            std::move(system_prompt), content, options);
        auto composed = [transform](typename Endpoint::Response_t const& response) -> Data {
            return transform(Extract<Endpoint>(response));
        };
        return handle.CallAsync(request, std::move(composed), policy);
    }

    template <typename Data>
    jai::llm::Result<Endpoint, Data>
    CallSync(std::optional<Prompt> system_prompt,
             const std::vector<Block>& content,
             Data (*transform)(const Result&),
             Options options = default_options,
             const jai::llm::AttemptPolicy& policy = {}) const
    {
        auto request = Generate<Endpoint>(
            std::move(system_prompt), content, options);
        auto composed = [transform](typename Endpoint::Response_t const& response) -> Data {
            return transform(Extract<Endpoint>(response));
        };
        return handle.CallSync(request, std::move(composed), policy);
    }
};


}
