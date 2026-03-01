/***
 * Gemini GenerateContent endpoint.
 *
 * Include this header to use the Gemini GenerateContent API through Instance.
 * Provides the endpoint tag type, auth types, and the necessary template
 * specialization declarations for Instance::CreateClient and dispatch.
 *
 * Gemini supports two auth modes — ApiKeyAuth (Google AI Studio) and
 * VertexAuth (Google Cloud Vertex AI). Both are declared here.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "../protocols/gemini/generate_content.hpp"


namespace jai::llm::gemini {


struct GenerateContent {
    using Request_t = gemini::GenerateContentRequest;
    using Response_t = gemini::GenerateContentResponse;
};


}
