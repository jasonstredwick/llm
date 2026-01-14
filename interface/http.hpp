/***
 * Primary interface for LLM usage management.
 * @author jason.stredwick@gmail.com
 */

#pragma once


#include <string>


namespace jai::llm::http {


enum class Method {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    TRACE
};


struct DroppedHeader {
    enum class Reason { MissingColon, ContainsNewline };
    std::string header;
    Reason reason;
};


} // namespace jai::llm::http
