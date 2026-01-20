#pragma once

#include <cstdio>
#include <string>
#include <string_view>


namespace jai::llm {


inline std::string JsonEscape(std::string_view sv) {
    std::string out;
    out.reserve(sv.size());
    for (char c : sv) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (unsigned(c) < 32) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", unsigned(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}


}
