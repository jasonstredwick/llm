/***
 * Authentication types for all supported LLM providers.
 *
 * Each provider's auth types live in their own namespace to avoid
 * name collisions (e.g., multiple providers define ApiKeyAuth).
 *
 * Endpoint headers include this file — users get the auth types
 * automatically when they include an endpoint header.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include <string>
#include <string_view>


namespace jai::llm::anthropic {

    // API key authentication for the direct Anthropic API.
    // The key is passed in the x-api-key header.
    // The version field controls the anthropic-version header; override it
    // to opt into beta features (e.g., extended thinking, computer use).
    struct ApiKeyAuth {
        std::string api_key;
        std::string version{"2023-06-01"};
    };

}


namespace jai::llm::openai {

    // API key authentication for the direct OpenAI API.
    // The key is passed as a Bearer token in the Authorization header.
    struct ApiKeyAuth {
        std::string api_key;
    };

    // Azure OpenAI Service authentication.
    // Uses a separate host, deployment-based URL, and API version query parameter.
    // Supports both API key (api-key header) and
    // Azure AD bearer token (Authorization header).
    struct AzureAuth {
        std::string api_key;        // API key or Azure AD access token
        std::string resource_name;  // e.g. "my-resource" -> my-resource.openai.azure.com
        std::string deployment;     // deployment name (maps to a model)
        std::string api_version{"2024-02-01"};
        bool use_bearer_token{false}; // true = Azure AD token in Authorization header
    };

}


namespace jai::llm::gemini {

    // Well-known Vertex AI location for the global endpoint.
    // Routes requests to the most available region automatically.
    // See: https://cloud.google.com/vertex-ai/generative-ai/docs/learn/locations
    inline constexpr std::string_view LOCATION_GLOBAL = "global";

    // API key authentication for Google AI Studio (Generative Language API).
    // The key is passed as a URL query parameter.
    struct ApiKeyAuth {
        std::string api_key;
    };

    // Vertex AI authentication via short-lived OAuth2 bearer token.
    // The token is passed in the Authorization header.
    // Location defaults to the global endpoint; set to a specific region
    // (e.g., "us-central1") for data residency requirements.
    struct VertexAuth {
        std::string access_token;
        std::string project;
        std::string location{LOCATION_GLOBAL};
    };

}
