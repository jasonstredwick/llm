/***
 * Primary interface for LLM usage management.
 * @author jason.stredwick@gmail.com
 */

#pragma once


namespace jai::llm {


struct DroppedHeader {
    enum class Reason { MissingColon, ContainsNewline };
    std::string header;
    Reason reason;
};


} // namespace jai::llm
