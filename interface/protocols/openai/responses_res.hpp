/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <map>

#include "responses_enums.hpp"
#include "responses_common.hpp"
#include "../../core/types.hpp"
#include "../../core/url.hpp"


namespace jai::llm::openai::response {


struct ContentTypes {
    struct File {
        InputFileKind type = InputFileKind::INPUT_FILE;
        std::string file_data;
        std::string file_id;
        EncodedUrl file_url;
        std::string filename;
    };

    struct Image {
        InputImageKind type = InputImageKind::INPUT_IMAGE;
        Detail detail; // Required defaults to auto, but marked required in doc
        std::string file_id;
        EncodedUrl image_url;
    };

    struct OutputText {
        struct ContainerFileCitation {
            ContainerFileCitationKind type = ContainerFileCitationKind::CONTAINER_FILE_CITATION;
            std::string container_id;
            int64_t end_index;
            std::string file_id;
            std::string filename;
            int64_t start_index;
        };

        struct FileCitation {
            FileCitationKind type = FileCitationKind::FILE_CITATION;
            std::string file_id;
            std::string filename;
            int64_t index;
        };

        struct UrlCitation {
            UrlCitationKind type = UrlCitationKind::URL_CITATION;
            int64_t end_index;
            int64_t start_index;
            std::string title;
            EncodedUrl url;
        };

        struct FilePath {
            FilePathKind type = FilePathKind::FILE_PATH;
            std::string file_id;
            int64_t index;
        };

        struct LogProb {
            struct TopLogprob {
                std::vector<uint8_t> bytes;
                double logprob;
                std::string token;
            };

            std::vector<uint8_t> bytes;
            double logprob;
            std::string token;
            std::vector<TopLogprob> top_logprobs;
        };

        using Annotation = std::variant<FileCitation, UrlCitation, ContainerFileCitation, FilePath>;

        OutputTextKind type = OutputTextKind::OUTPUT_TEXT;
        std::vector<Annotation> annotations;
        std::vector<LogProb> logprobs;
        std::string text;
    };

    struct Refusal {
        RefusalKind type = RefusalKind::REFUSAL;
        std::string refusal;
    };

    struct Text {
        InputTextKind type = InputTextKind::INPUT_TEXT;
        std::string text;
    };
};


struct WebSearchToolActions {
    struct Find {
        FindActionKind type = FindActionKind::FIND;
        std::string pattern;
        EncodedUrl url;
    };

    struct OpenPage {
        OpenPageActionKind type = OpenPageActionKind::OPEN_PAGE;
        EncodedUrl url;
    };

    struct Search {
        struct Source {
            SearchActionSourceKind type = SearchActionSourceKind::URL;
            EncodedUrl url;
        };

        SearchActionKind type = SearchActionKind::SEARCH;
        std::optional<std::vector<std::string>> queries;
        std::optional<std::vector<Source>> sources;
    };

    using All = std::variant<Find, OpenPage, Search>;
};


struct InputTypes {
    using MessageContentUnit = std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>;

    struct Message {
        using Content = std::variant<std::string, std::vector<MessageContentUnit>>;

        Content content;
        RoleInputMessage role;
        InputMessageKind type; // InputMessageKind::MESSAGE
    };

    struct Item {
        struct InputMessage {
            std::vector<MessageContentUnit> content;
            RoleUser role;
            ItemStatus status;
            InputMessageKind type;
        };

        struct OutputMessage {
            using Content = std::variant<ContentTypes::OutputText, ContentTypes::Refusal>;

            std::vector<Content> content;
            std::string id;
            RoleAssistant role;
            ItemStatus status;
            InputMessageKind type;
        };

        struct FileSearchToolCall {
            struct Result {
                std::optional<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>> attributes;
                std::string file_id;
                std::string filename;
                double score;
                std::string text;
            };

            FileSearchToolCallKind type = FileSearchToolCallKind::FILE_SEARCH_CALL;
            std::string id;
            std::vector<std::string> queries;
            FileSearchStatus status;
            std::optional<std::vector<Result>> results;
        };

        struct ComputerToolCall {
            struct PendingSafetyCheck {
                std::string id;
                std::string code;
                std::string message;
            };

            ComputerCallKind type = ComputerCallKind::COMPUTER_CALL;
            ComputerToolActions::All action;
            std::string call_id;
            std::string id;
            std::vector<PendingSafetyCheck> pending_safety_checks;
            ItemStatus status;
        };

        struct ComputerToolCallOutput {
            struct ComputerScreenshot {
                ComputerScreenshotKind type = ComputerScreenshotKind::COMPUTER_SCREENSHOT;
                std::string file_id;
                EncodedUrl image_url;
            };

            struct AcknowledgedSafetyCheck {
                std::string id;
                std::string code;
                std::string message;
            };

            ComputerCallOutputKind type = ComputerCallOutputKind::COMPUTER_CALL_OUTPUT;
            std::string call_id;
            ComputerScreenshot output;
            std::optional<std::vector<AcknowledgedSafetyCheck>> acknowledged_safety_checks;
            std::string id;
            ItemStatus status;
        };

        struct WebSearchToolCall {
            WebSearchCallKind type = WebSearchCallKind::WEB_SEARCH_CALL;
            WebSearchToolActions::All action;
            std::string id;
            ItemStatus status; // Doc says "status" but doesn't list enum. Assuming ItemStatus.
        };

        struct FunctionToolCall {
            FunctionCallKind type = FunctionCallKind::FUNCTION_CALL;
            std::string arguments;
            std::string call_id;
            std::string name;
            std::string id;
            ItemStatus status;
        };

        struct FunctionToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            FunctionCallOutputKind type = FunctionCallOutputKind::FUNCTION_CALL_OUTPUT;
            std::string call_id;
            Output output;
            std::string id;
            ItemStatus status;
        };

        struct Reasoning {
            struct Summary {
                ReasoningSummaryTextKind type = ReasoningSummaryTextKind::SUMMARY_TEXT;
                std::string text;
            };

            struct Content {
                ReasoningTextKind type = ReasoningTextKind::REASONING_TEXT;
                std::string text;
            };

            ReasoningItemKind type = ReasoningItemKind::REASONING;
            std::string id;
            std::vector<Summary> summary;
            std::optional<std::vector<Content>> content;
            std::string encrypted_content;
            ItemStatus status;
        };

        struct CompactionItem {
            CompactionItemKind type = CompactionItemKind::COMPACTION;
            std::string encrypted_content;
            std::string id;
            std::string created_by;
        };

        struct ImageGenerationCall {
            ImageGenerationCallKind type = ImageGenerationCallKind::IMAGE_GENERATION_CALL;
            std::string id;
            std::string result;
            std::string status;
        };

        struct CodeInterpreterToolCall {
            struct CodeInterpreterOutputLog {
                CodeInterpreterOutputType type = CodeInterpreterOutputType::LOGS;
                std::string logs;
            };

            struct CodeInterpreterOutputImage {
                CodeInterpreterOutputType type = CodeInterpreterOutputType::IMAGE;
                EncodedUrl url;
            };

            using Output = std::variant<CodeInterpreterOutputLog, CodeInterpreterOutputImage>;

            CodeInterpreterCallKind type = CodeInterpreterCallKind::CODE_INTERPRETER_CALL;
            std::string code;
            std::string container_id;
            std::string id;
            std::vector<Output> outputs;
            CodeInterpreterStatus status;
        };

        struct LocalShellCall {
            struct Action {
                LocalShellActionKind type = LocalShellActionKind::EXEC;
                std::vector<std::string> command;
                std::map<std::string, std::string> env;
                int64_t timeout_ms;
                std::string user;
                std::string working_directory;
            };

            LocalShellCallKind type = LocalShellCallKind::LOCAL_SHELL_CALL;
            Action action;
            std::string call_id;
            std::string id;
            ItemStatus status;
        };

        struct LocalShellCallOutput {
            LocalShellCallOutputKind type = LocalShellCallOutputKind::LOCAL_SHELL_CALL_OUTPUT;
            std::string id;
            std::string output;
            ItemStatus status;
        };

        struct ShellToolCall {
            struct Action {
                std::vector<std::string> commands;
                int64_t max_output_length;
                int64_t timeout_ms;
            };

            ShellCallKind type = ShellCallKind::SHELL_CALL;
            Action action;
            std::string call_id;
            std::string id;
            ItemStatus status;
            std::string created_by;
        };

        struct ShellToolCallOutput {
            struct ShellCallExitOutcome {
                ShellExitOutcomeKind type = ShellExitOutcomeKind::EXIT;
                int64_t exit_code;
            };

            struct ShellCallTimeoutOutcome {
                ShellTimeoutOutcomeKind type = ShellTimeoutOutcomeKind::TIMEOUT;
            };

            struct Output {
                using Outcome = std::variant<ShellCallExitOutcome, ShellCallTimeoutOutcome>;

                Outcome outcome;
                std::string std_err;
                std::string std_out;
                std::string created_by;
            };

            ShellCallOutputKind type = ShellCallOutputKind::SHELL_CALL_OUTPUT;
            std::string call_id;
            std::vector<Output> output;
            std::string id;
            int64_t max_output_length;
            ItemStatus status;
            std::string created_by;
        };

        struct ApplyPatchToolCall {
            using ApplyPatchOperation = std::variant<PatchFileOperations::Create,
                                                     PatchFileOperations::Delete,
                                                     PatchFileOperations::Update>;
            ApplyPatchCallKind type = ApplyPatchCallKind::APPLY_PATCH_CALL;
            std::string call_id;
            ApplyPatchOperation operation;
            ApplyPatchCallStatus status;
            std::string id;
            std::string created_by;
        };

        struct ApplyPatchToolCallOutput {
            ApplyPatchCallOutputKind type = ApplyPatchCallOutputKind::APPLY_PATCH_CALL_OUTPUT;
            std::string call_id;
            ApplyPatchCallOutputStatus status;
            std::string id;
            std::string output;
            std::string created_by;
        };

        struct MCPListTools {
            struct ToolDef {
                jai::llm::json::Object input_schema;
                std::string name;
                jai::llm::json::Object annotations;
                std::string description;
            };

            MCPListToolsKind type = MCPListToolsKind::MCP_LIST_TOOLS;
            std::string id;
            std::string server_label;
            std::vector<ToolDef> tools;
            std::string error;
        };

        struct MCPApprovalRequest {
            MCPApprovalRequestKind type = MCPApprovalRequestKind::MCP_APPROVAL_REQUEST;
            std::string arguments;
            std::string id;
            std::string name;
            std::string server_label;
        };

        struct MCPApprovalResponse {
            MCPApprovalResponseKind type = MCPApprovalResponseKind::MCP_APPROVAL_RESPONSE;
            std::string approval_request_id;
            bool approve;
            std::string id;
            std::string reason;
        };

        struct MCPToolCall {
            MCPCallKind type = MCPCallKind::MCP_CALL;
            std::string arguments;
            std::string id;
            std::string name;
            std::string server_label;
            std::string approval_request_id;
            std::string error;
            std::string output;
            CallStatus status;
        };

        struct CustomToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            CustomToolCallOutputKind type = CustomToolCallOutputKind::CUSTOM_TOOL_CALL_OUTPUT;
            std::string call_id;
            Output output;
            std::string id;
        };

        struct CustomToolCall {
            CustomToolCallKind type = CustomToolCallKind::CUSTOM_TOOL_CALL;
            std::string call_id;
            std::string input;
            std::string name;
            std::string id;
        };
    };
};


struct ItemReference {
    ItemReferenceKind type; // ItemReferenceKind::ITEM_REFERENCE
    std::string id;
};


using Item = std::variant<
    //InputTypes::Item::InputMessage,
    InputTypes::Item::OutputMessage,
    InputTypes::Item::FileSearchToolCall,
    InputTypes::Item::ComputerToolCall,
    //InputTypes::Item::ComputerToolCallOutput,
    InputTypes::Item::WebSearchToolCall,
    InputTypes::Item::FunctionToolCall,
    //InputTypes::Item::FunctionToolCallOutput,
    InputTypes::Item::Reasoning,
    InputTypes::Item::CompactionItem,
    InputTypes::Item::ImageGenerationCall,
    InputTypes::Item::CodeInterpreterToolCall,
    InputTypes::Item::LocalShellCall,
    //InputTypes::Item::LocalShellCallOutput,
    InputTypes::Item::ShellToolCall,
    InputTypes::Item::ShellToolCallOutput,
    InputTypes::Item::ApplyPatchToolCall,
    InputTypes::Item::ApplyPatchToolCallOutput,
    InputTypes::Item::MCPListTools,
    InputTypes::Item::MCPApprovalRequest,
    //InputTypes::Item::MCPApprovalResponse,
    InputTypes::Item::MCPToolCall,
    //InputTypes::Item::CustomToolCallOutput,
    InputTypes::Item::CustomToolCall
>;
using OutputItemList = std::variant<InputTypes::Message, Item, ItemReference>;
using Output = std::variant<std::string, std::vector<OutputItemList>>;


struct Prompt {
    using VariableTypes = std::variant<std::string, ContentTypes::Text, ContentTypes::Image, ContentTypes::File>;

    std::string id;
    std::optional<std::map<std::string, VariableTypes>> variables;
    std::optional<std::string> version;
};


struct ToolTypes {
    struct Function {
        FunctionToolKind type = FunctionToolKind::FUNCTION;
        std::string name;
        jai::llm::json::Object parameters;
        bool strict;
        std::string description;
    };

    struct FileSearch {
        struct ComparisonFilter {
            using ValueType = std::variant<std::string, double, bool,
                                           std::vector<std::string>,
                                           std::vector<double>,
                                           std::vector<bool>>;
            std::string key;
            FilterOperator type;
            ValueType value;
        };

        struct CompoundFilter {
            FilterCompoundType type;
            std::vector<std::variant<ComparisonFilter, CompoundFilter>> filters;
        };

        struct RankingOptions {
            struct HybridSearch {
                double embedding_weight;
                double text_weight;
            };

            HybridSearch hybrid_search;
            std::string ranker;
            double score_threshold;
        };

        FileSearchToolKind type = FileSearchToolKind::FILE_SEARCH;
        std::vector<std::string> vector_store_ids;
        std::optional<std::variant<ComparisonFilter, CompoundFilter>> filters;
        int64_t max_num_results;
        RankingOptions ranking_options;
    };

    struct ComputerUse {
        ComputerUseToolKind type = ComputerUseToolKind::COMPUTER_USE_PREVIEW;
        int64_t display_height;
        int64_t display_width;
        std::string environment;
    };

    struct WebSearch {
        struct Filters {
            std::optional<std::vector<std::string>> allowed_domains;
        };

        struct Location {
            LocationType type = LocationType::APPROXIMATE;
            std::string city;
            std::string country;
            std::string region;
            std::string timezone;
        };

        WebSearchToolKind type = WebSearchToolKind::WEB_SEARCH;
        Filters filters;
        SearchContextSize search_context_size;
        Location user_location;
    };

    struct MCP {
        struct Filter {
            bool read_only;
            std::optional<std::vector<std::string>> tool_names;
        };

        struct ApprovalFilter {
            Filter always;
            Filter never;
        };

        using AllowedTools = std::variant<std::vector<std::string>, Filter>;
        using RequiredApproval = std::variant<MCPApprovalSetting, ApprovalFilter>;

        MCPToolKind type = MCPToolKind::MCP;
        std::string server_label;
        AllowedTools allowed_tools;
        std::string authorization;
        ConnectId connector_id;
        std::optional<std::map<std::string, std::string>> headers;
        RequiredApproval require_approval;
        std::string server_description;
        EncodedUrl server_url;
    };

    struct CodeInterpreter {
        struct ContainerConfig {
            ContainerConfigKind type = ContainerConfigKind::AUTO;
            std::optional<std::vector<std::string>> file_ids;
            std::string memory_limit;
        };
        using Container = std::variant<std::string, ContainerConfig>;

        CodeInterpreterToolKind type = CodeInterpreterToolKind::CODE_INTERPRETER;
        Container container;
    };

    struct ImageGeneration {
        struct Mask {
            std::string file_id;
            EncodedUrl image_url;
        };

        ImageGenerationToolKind type = ImageGenerationToolKind::IMAGE_GENERATION;
        std::string action;
        ImageGenerationBackground background;
        ImageGenerationFidelity input_fidelity;
        Mask input_image_mask;
        std::string model;
        std::string moderation;
        int64_t output_compression;
        ImageGenerationFormat output_format;
        int64_t partial_images;
        ImageGenerationQuality quality;
        ImageGenerationSize size;
    };

    struct LocalShell {
        LocalShellToolKind type = LocalShellToolKind::LOCAL_SHELL;
    };

    struct Shell {
        ShellToolKind type = ShellToolKind::SHELL;
    };

    struct Custom {
        struct GrammarFormat {
            CustomToolGrammarFormatKind type = CustomToolGrammarFormatKind::GRAMMAR;
            std::string definition;
            GrammarSyntax syntax;
        };

        struct TextFormat {
            CustomToolTextFormatKind type = CustomToolTextFormatKind::TEXT;
        };

        using Format = std::variant<GrammarFormat, TextFormat>;

        CustomToolKind type = CustomToolKind::CUSTOM;
        std::string name;
        std::string description;
        Format format;
    };

    struct WebSearchPreview {
        struct Location {
            LocationType type = LocationType::APPROXIMATE;
            std::string city;
            std::string country;
            std::string region;
            std::string timezone;
        };

        WebSearchPreviewToolKind type;
        SearchContextSize search_context_size;
        Location user_location;
    };

    struct ApplyPatch {
        ApplyPatchToolKind type = ApplyPatchToolKind::APPLY_PATCH;
    };
};


using Tool = std::variant<
    ToolTypes::Function, ToolTypes::FileSearch, ToolTypes::ComputerUse, ToolTypes::WebSearch, ToolTypes::MCP,
    ToolTypes::CodeInterpreter, ToolTypes::ImageGeneration, ToolTypes::LocalShell, ToolTypes::Shell, ToolTypes::Custom,
    ToolTypes::WebSearchPreview, ToolTypes::ApplyPatch
>;


struct ToolsChoiceTypes {
    struct Allowed {
        struct RestrictedTool {
            std::string type;
            std::string name;
            std::string server_label;
        };

        AllowedToolsChoiceKind type = AllowedToolsChoiceKind::ALLOWED_TOOLS;
        ToolChoiceModeNotNone mode;
        std::vector<RestrictedTool> tools;
    };

    struct Custom {
        CustomToolChoiceKind type = CustomToolChoiceKind::CUSTOM;
        std::string name;
    };

    struct Function {
        FunctionToolChoiceKind type = FunctionToolChoiceKind::FUNCTION;
        std::string name;
    };

    struct Hosted {
        HostedToolMode type;
    };

    struct MCP {
        MCPToolChoiceKind type = MCPToolChoiceKind::MCP;
        std::string server_label;
        std::string name;
    };

    struct SpecificApplyPatch {
        SpecificApplyPatchToolChoiceKind type = SpecificApplyPatchToolChoiceKind::APPLY_PATCH;
    };

    struct SpecificShell {
        SpecificShellToolChoiceKind type = SpecificShellToolChoiceKind::SHELL;
    };
};


using ToolChoice = std::variant<ToolChoiceMode,
                                ToolsChoiceTypes::Allowed, ToolsChoiceTypes::Custom, ToolsChoiceTypes::Function,
                                ToolsChoiceTypes::Hosted, ToolsChoiceTypes::MCP, ToolsChoiceTypes::SpecificApplyPatch,
                                ToolsChoiceTypes::SpecificShell>;


}
