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


// Pre-parsed model identity — computed once at ClientHandle construction,
// passed to Generate for version-gated feature decisions.
struct ModelInfo {
    std::string model;      // raw model string (set in request body)
    std::string family;     // normalized family: "opus", "sonnet", "gemini-2.5-pro", "gpt-5", "o3", ...
    double version{0.0};    // extracted major.minor version number
};

// Per-endpoint parsing — specializations live alongside Generate/Extract.
template <typename Endpoint>
ModelInfo ParseModelInfo(std::string_view model);

template <typename Endpoint>
typename Endpoint::Request_t Generate(const ModelInfo& model_info,
                                      std::optional<Prompt> system_prompt,
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

template <typename Endpoint>
class ClientHandle {
    jai::llm::Instance::ClientHandle<Endpoint> handle;
    ModelInfo info;

public:
    // One-stop construction: creates the underlying client and stores model info.
    template <typename Auth>
    ClientHandle(jai::llm::Instance& instance, Auth auth, std::string model,
                 const jai::llm::ClientPolicy& policy = {})
        : handle{instance.template CreateClient<Endpoint>(std::move(auth), model, policy)}
        , info{ParseModelInfo<Endpoint>(model)}
    {}

    // Wrap a pre-built handle with model info.
    ClientHandle(jai::llm::Instance::ClientHandle<Endpoint> h, std::string model)
        : handle{std::move(h)}
        , info{ParseModelInfo<Endpoint>(model)}
    {}

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
            info, std::move(system_prompt), content, options);
        return handle.CallAsync(request, &Extract<Endpoint>, policy);
    }

    jai::llm::Result<Endpoint, Result>
    CallSync(std::optional<Prompt> system_prompt,
             const std::vector<Block>& content,
             Options options = default_options,
             const jai::llm::AttemptPolicy& policy = {}) const
    {
        auto request = Generate<Endpoint>(
            info, std::move(system_prompt), content, options);
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
            info, std::move(system_prompt), content, options);
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
            info, std::move(system_prompt), content, options);
        auto composed = [transform](typename Endpoint::Response_t const& response) -> Data {
            return transform(Extract<Endpoint>(response));
        };
        return handle.CallSync(request, std::move(composed), policy);
    }
};


}
