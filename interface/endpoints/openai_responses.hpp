/***
 * OpenAI Responses endpoint.
 *
 * Include this header to use the OpenAI Responses API through Instance.
 * Provides the endpoint tag type, auth types, and the necessary template
 * specialization declarations for Instance::CreateClient and dispatch.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "../protocols/openai/responses.hpp"


namespace jai::llm::openai {


struct Responses {
    using Request_t = openai::Request;
    using Response_t = openai::Response;
};


}

