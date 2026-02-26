/***
 * Type-erased Client — optional convenience wrapper.
 *
 * Allows calling code to hold a single client value regardless of provider.
 * The primary client types are the provider-specific ones (anthropic::Client,
 * gemini::Client, openai::Client). This wrapper exists for cases where the
 * provider is selected at runtime and the caller wants a uniform handle.
 *
 * If you know the provider at compile time, prefer the typed client directly.
 *
 * Authentication is provider-specific. Each provider defines its own auth
 * struct(s) in its header (e.g., gemini::ApiKeyAuth, gemini::VertexAuth).
 * See the provider-specific client headers for details.
 *
 * TODO: Revisit whether this is needed once the provider clients are fully
 *       fleshed out. It may be that a simple std::variant<anthropic::Client,
 *       gemini::Client, openai::Client> suffices.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


namespace jai::llm {


// ModelContract identifies a semantically distinct model interface
// that requires specific request/response and reasoning handling.
enum class ModelContract {
    Anthropic,
    Gemini,
    OpenAI
};


// Placeholder — the shape of the type-erased client is still being determined.
// See the provider-specific clients (anthropic::Client, etc.) for the
// current working interface.


}
