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
        Required<KindClickAction> type{{}};
        Required<MouseButton> button;
        Required<int64_t> x;
        Required<int64_t> y;
    };

    struct DoubleClick {
        Required<KindDoubleClickAction> type{{}};
        Required<int64_t> x;
        Required<int64_t> y;
    };

    struct Drag {
        struct Coordinate {
            Required<int64_t> x;
            Required<int64_t> y;
        };

        Required<KindDragAction> type{{}};
        Required<std::vector<Coordinate>> path;
    };

    struct KeyPress {
        Required<KindKeyPressAction> type{{}};
        Required<std::vector<std::string>> keys;
    };

    struct Move {
        Required<KindMoveAction> type{{}};
        Required<int64_t> x;
        Required<int64_t> y;
    };

    struct Screenshot {
        Required<KindScreenshotAction> type{{}};
    };

    struct Scroll {
        Required<KindScrollAction> type{{}};
        Required<int64_t> scroll_x;
        Required<int64_t> scroll_y;
        Required<int64_t> x;
        Required<int64_t> y;
    };

    struct Type {
        Required<KindTypeAction> type{{}};
        Required<std::string> text;
    };

    struct Wait {
        Required<KindWaitAction> type{{}};
    };

    using All = std::variant<Click, DoubleClick, Drag, KeyPress, Move, Screenshot, Scroll, Type, Wait>;
};


struct ConversationRef {
    Required<std::string> id;
};


struct IncompleteDetails {
    Required<std::string> reason;
};


struct PatchFileOperations {
    struct Create {
        Required<KindCreateFileOperation> type{{}};
        Required<std::string> diff;
        Required<std::string> path;
    };

    struct Delete {
        Required<KindDeleteFileOperation> type{{}};
        Required<std::string> path;
    };

    struct Update {
        Required<KindUpdateFileOperation> type{{}};
        Required<std::string> diff;
        Required<std::string> path;
    };
};



struct Reasoning {
    Required<ReasoningEffort> effort;
    Required<ReasoningSummary> summary;
};


struct ResponseError {
    Required<std::string> code;
    Required<std::string> message;
};


struct ResponseUsage {
    struct InputTokenDetails {
        Required<int64_t> cached_tokens;
    };

    struct OutputTokenDetails {
        Required<int64_t> reasoning_tokens;
    };

    Required<int64_t> input_tokens;
    Required<InputTokenDetails> input_tokens_details;
    Required<int64_t> output_tokens;
    Required<OutputTokenDetails> output_tokens_details;
    Required<int64_t> total_tokens;
};


struct StreamOptions {
    std::optional<bool> include_obfuscation;
};


struct TextConfig {
    struct FormatText {
        Required<KindFormatText> type{{}};
    };

    struct FormatJsonSchema {
        Required<KindFormatJsonSchema> type{{}};
        Required<std::string> name;
        Required<jai::llm::json::Object> schema;
        std::optional<std::string> description{};
        std::optional<bool> strict{};
    };

    using Format = std::variant<FormatText, FormatJsonSchema>;

    std::optional<Format> format{};
    std::optional<Verbosity> verbosity{};
};


}
