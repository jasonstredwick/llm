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
        ClickActionKind type = ClickActionKind::CLICK;
        MouseButton button;
        int64_t x;
        int64_t y;
    };

    struct DoubleClick {
        DoubleClickActionKind type = DoubleClickActionKind::DOUBLE_CLICK;
        int64_t x;
        int64_t y;
    };

    struct Drag {
        struct Coordinate {
            int64_t x;
            int64_t y;
        };

        DragActionKind type = DragActionKind::DRAG;
        std::vector<Coordinate> path;
    };

    struct KeyPress {
        KeyPressActionKind type = KeyPressActionKind::KEYPRESS;
        std::vector<std::string> keys;
    };

    struct Move {
        MoveActionKind type = MoveActionKind::MOVE;
        int64_t x;
        int64_t y;
    };

    struct Screenshot {
        ScreenshotActionKind type = ScreenshotActionKind::SCREENSHOT;
    };

    struct Scroll {
        ScrollActionKind type = ScrollActionKind::SCROLL;
        int64_t scroll_x;
        int64_t scroll_y;
        int64_t x;
        int64_t y;
    };

    struct Type {
        TypeActionKind type = TypeActionKind::TYPE;
        std::string text;
    };

    struct Wait {
        WaitActionKind type = WaitActionKind::WAIT;
    };

    using All = std::variant<Click, DoubleClick, Drag, KeyPress, Move, Screenshot, Scroll, Type, Wait>;
};


struct ConversationRef {
    std::string id;
};


struct PatchFileOperations {
    struct Create {
        CreateFileOperationKind type = CreateFileOperationKind::CREATE_FILE;
        std::string diff;
        std::string path;
    };

    struct Delete {
        DeleteFileOperationKind type = DeleteFileOperationKind::DELETE_FILE;
        std::string path;
    };

    struct Update {
        UpdateFileOperationKind type = UpdateFileOperationKind::UPDATE_FILE;
        std::string diff;
        std::string path;
    };
};



struct IncompleteDetails {
    std::string reason;
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
    bool include_obfuscation;
};


struct TextConfig {
    struct FormatText {
        FormatTextKind type = FormatTextKind::TEXT;
    };

    struct FormatJsonSchema {
        FormatJsonSchemaKind type = FormatJsonSchemaKind::JSON_SCHEMA;
        std::string name;
        jai::llm::json::Object schema;
        std::string description;
        bool strict;
    };

    using Format = std::variant<FormatText, FormatJsonSchema>;

    Format format;
    Verbosity verbosity;
};


}
