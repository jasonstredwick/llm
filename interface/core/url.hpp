#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>


namespace jai::llm {


/**
 * @brief Represents a URL that is expected to be ASCII-only (already percent-encoded).
 */
class EncodedUrl {
private:
    std::string value{};

public:
    explicit EncodedUrl(std::string v) : value{std::move(v)} { EncodedUrl::VerifyWireSafe(value); }
    explicit EncodedUrl(std::string_view v) : value{v} { EncodedUrl::VerifyWireSafe(value); }

    EncodedUrl(const EncodedUrl&) = default;
    EncodedUrl(EncodedUrl&&) noexcept = default;
    ~EncodedUrl() noexcept = default;
    EncodedUrl& operator=(const EncodedUrl&) = default;
    EncodedUrl& operator=(EncodedUrl&&) noexcept = default;

    friend bool operator<=>(const EncodedUrl&, const EncodedUrl&) = default;

    std::string_view View() const noexcept { return value; }

private:
    static void VerifyWireSafe(std::string_view sv) {
        if (sv.empty()) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: empty string provided."};
        } else if (!std::ranges::all_of(sv, [](auto c) { return static_cast<unsigned char>(c) <= 127; })) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: contains non-ASCII characters."};
        } else if (sv.find('\0') != std::string_view::npos) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: contains NUL (\\0) character"};
        } else if (sv.find('\n') != std::string_view::npos) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: contains newline (\\n) character"};
        } else if (sv.find('\r') != std::string_view::npos) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: contains carriage return (\\r) character"};
        } else if (sv.find(' ') != std::string_view::npos) {
            throw std::invalid_argument{"EncodedUrl must already be ASCII-encoded: contains space character"};
        }
    }
};


}
