/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <variant>

#include "responses_enums.hpp"
#include "../../core/types.hpp"


namespace jai::llm::openai {


struct ComputerToolActions {
    struct Click {
        KindClickAction type{};
        MouseButton button;
        int64_t x;
        int64_t y;
    };

    struct DoubleClick {
        KindDoubleClickAction type{};
        int64_t x;
        int64_t y;
    };

    struct Drag {
        struct Coordinate {
            int64_t x;
            int64_t y;
        };

        KindDragAction type{};
        std::vector<Coordinate> path;
    };

    struct KeyPress {
        KindKeyPressAction type{};
        std::vector<std::string> keys;
    };

    struct Move {
        KindMoveAction type{};
        int64_t x;
        int64_t y;
    };

    struct Screenshot {
        KindScreenshotAction type{};
    };

    struct Scroll {
        KindScrollAction type{};
        int64_t scroll_x;
        int64_t scroll_y;
        int64_t x;
        int64_t y;
    };

    struct Type {
        KindTypeAction type{};
        std::string text;
    };

    struct Wait {
        KindWaitAction type{};
    };

    using All = std::variant<Click, DoubleClick, Drag, KeyPress, Move, Screenshot, Scroll, Type, Wait>;
};


struct ConversationRef {
    std::string id;
};


struct IncompleteDetails {
    std::string reason;
};


struct PatchFileOperations {
    struct Create {
        KindCreateFileOperation type{};
        std::string diff;
        std::string path;
    };

    struct Delete {
        KindDeleteFileOperation type{};
        std::string path;
    };

    struct Update {
        KindUpdateFileOperation type{};
        std::string diff;
        std::string path;
    };
};



struct Reasoning {
    ReasoningEffort effort;
    ReasoningSummary summary;
};


struct ResponseError {
    std::string code;
    std::string message;
};


struct ResponseUsage {
    struct InputTokenDetails {
        int64_t cached_tokens;
    };

    struct OutputTokenDetails {
        int64_t reasoning_tokens;
    };

    int64_t input_tokens;
    InputTokenDetails input_tokens_details;
    int64_t output_tokens;
    OutputTokenDetails output_tokens_details;
    int64_t total_tokens;
};


struct StreamOptions {
    std::optional<bool> include_obfuscation;
};


struct TextConfig {
    struct FormatText {
        KindFormatText type{};
    };

    struct FormatJsonSchema {
        KindFormatJsonSchema type{};
        std::string name;
        jai::llm::json::Object schema;
        std::optional<std::string> description{};
        std::optional<bool> strict{};
    };

    using Format = std::variant<FormatText, FormatJsonSchema>;

    std::optional<Format> format{};
    std::optional<Verbosity> verbosity{};
};


}
