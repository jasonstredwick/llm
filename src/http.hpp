#pragma once


#include <algorithm>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../interface/http.hpp"


using namespace std::literals::string_view_literals;


namespace jai::llm::http {


class Headers {
private:
    inline static const std::string_view WHITESPACE{" \t\f\v"sv};

    std::vector<std::string> headers{};
    std::vector<DroppedHeader> dropped_headers{};

public:
    Headers() = default;
    explicit Headers(const std::vector<std::string>& headers_);
    explicit Headers(const std::vector<std::string_view>& headers_);

    void AddDefaultHeader(const std::string& header) { ProcessDefaultHeaders(std::vector<std::string>{header}); }
    void AddDefaultHeaders(const std::vector<std::string>& headers_) { ProcessDefaultHeaders(headers_); }

    const std::vector<std::string>& GetHeaders() const { return headers; }
    const std::vector<DroppedHeader>& GetDroppedHeaders() const { return dropped_headers; }

private:
    void AddHeader(std::string_view header) {
        auto reason = IsNotValidHeader(header);
        if (reason) { dropped_headers.push_back({std::string{header}, reason.value()}); }
        else        { headers.push_back(std::string{header}); }
    }

    std::vector<std::string> ExtractKeys(const std::vector<std::string>& headers_) const;
    std::optional<DroppedHeader::Reason> IsNotValidHeader(std::string_view header) const;
    void ProcessDefaultHeaders(const std::vector<std::string>& default_headers_);
};


} // namespace jai::llm::http
