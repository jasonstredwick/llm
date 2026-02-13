#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>


using namespace std::literals::string_view_literals;


namespace jai::llm::http {


template <typename T> concept CharString_c = std::convertible_to<T, std::string_view>;
template <typename R>
concept HeaderKVRange_c = std::ranges::input_range<R> &&
                          requires(std::ranges::range_reference_t<R> e) {
                              { std::get<0>(e) } -> CharString_c; // key   (ASCII text)
                              { std::get<1>(e) } -> CharString_c; // value (ASCII / UTF-8)
                          };
template <typename R>
concept MergedHeaderRange_c = std::ranges::input_range<R> && CharString_c<std::ranges::range_reference_t<R>>;


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


class RequestHeaders {
private:
    std::vector<std::string> entries{};

public:
    RequestHeaders() = default;
    explicit RequestHeaders(MergedHeaderRange_c auto const& in) : entries{RequestHeaders::FromSeq(in)} {}
    explicit RequestHeaders(HeaderKVRange_c auto const& in) : entries{RequestHeaders::FromKVRange(in)} {}
    RequestHeaders(const RequestHeaders&) = default;
    RequestHeaders(RequestHeaders&&) noexcept = default;
    ~RequestHeaders() = default;
    RequestHeaders& operator=(const RequestHeaders&) = default;
    RequestHeaders& operator=(RequestHeaders&&) noexcept = default;

    friend auto operator<=>(RequestHeaders const&, RequestHeaders const&) = default;

    bool Empty() const { return entries.empty(); }
    const auto& Entries() const { return entries; }
    size_t Size() const { return entries.size(); }

private:
    static std::vector<std::string> FromKVRange(HeaderKVRange_c auto const& in);
    static std::vector<std::string> FromSeq(MergedHeaderRange_c auto const& in);
    static void SecurityCheck(std::string_view sv);
    static std::string ToValueStr(auto const& v);
};


class ResponseHeaders {
private:
    inline static const std::string_view WHITESPACE{" \t\f\v"sv};

    std::vector<std::string> headers{};
    std::vector<DroppedHeader> dropped_headers{};

public:
    ResponseHeaders() = default;
    explicit ResponseHeaders(const std::vector<std::string>& headers_);
    explicit ResponseHeaders(const std::vector<std::string_view>& headers_);

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


struct Request {
    RequestHeaders headers;
    Method method;
    std::string url;
    std::vector<std::byte> body;
};


std::vector<std::string> RequestHeaders::FromKVRange(HeaderKVRange_c auto const& in) {
    return
        in |
        std::views::transform([](auto const& kv) {
            auto const& [k, v] = kv;
            std::string str{};
            std::string_view key{k};
            std::string value = RequestHeaders::ToValueStr(v);

            str.reserve(key.size() + 2 + value.size());
            str += key;
            str += ": ";
            str += value;

            RequestHeaders::SecurityCheck(str);

            return str;
        }) |
        std::ranges::to<std::vector<std::string>>();
}


std::vector<std::string> RequestHeaders::FromSeq(MergedHeaderRange_c auto const& in) {
    return
        in |
        std::views::transform([](std::string_view sv) {
            std::string str{sv};
            RequestHeaders::SecurityCheck(str);
            return str;
        }) |
        std::ranges::to<std::vector<std::string>>();
}


std::string RequestHeaders::ToValueStr(auto const& v) {
    if constexpr (std::convertible_to<decltype(v), std::string_view>) {
        return std::string{std::string_view{v}};
    } else {
        std::string_view sv{v};
        return std::string{reinterpret_cast<const char*>(sv.data()), sv.size()};
    }
}


} // namespace jai::llm::http
