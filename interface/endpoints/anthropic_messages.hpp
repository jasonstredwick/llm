/***
 * Anthropic Messages endpoint.
 *
 * Include this header to use the Anthropic Messages API through Instance.
 * Provides the endpoint tag type, auth types, and the necessary template
 * specialization declarations for Instance::CreateClient and dispatch.
 *
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include "../protocols/anthropic/messages.hpp"


namespace jai::llm::anthropic {


struct Messages {
    using Request_t = anthropic::Request;
    using Response_t = anthropic::Message;
};


}

