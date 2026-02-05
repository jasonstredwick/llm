/***
 * This file defines the OpenAI Responses interaction protocol.
 * It is a semantic model, not a transport or REST contract.
 * Not all structures are valid in all modes or providers.
 *
 * See https://platform.openai.com/docs/api-reference/responses/create
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <map>

#include "responses_enums.hpp"
#include "responses_common.hpp"
#include "../../core/types.hpp"


namespace jai::llm::openai::response {


struct ContentTypes {
    struct File {
        KindInputFile type{};
        std::string file_data;
        std::string file_id;
        EncodedUrl file_url;
        std::string filename;
    };

    struct Image {
        KindInputImage type{};
        Detail detail; // Required defaults to auto, but marked required in doc
        std::string file_id;
        EncodedUrl image_url;
    };

    struct OutputText {
        struct ContainerFileCitation {
            KindContainerFileCitation type{};
            std::string container_id;
            int64_t end_index;
            std::string file_id;
            std::string filename;
            int64_t start_index;
        };

        struct FileCitation {
            KindFileCitation type{};
            std::string file_id;
            std::string filename;
            int64_t index;
        };

        struct UrlCitation {
            KindUrlCitation type{};
            int64_t end_index;
            int64_t start_index;
            std::string title;
            EncodedUrl url;
        };

        struct FilePath {
            KindFilePath type{};
            std::string file_id;
            int64_t index;
        };

        struct LogProb {
            struct TopLogprob {
                std::vector<std::byte> bytes;
                double logprob;
                std::string token;
            };

            std::vector<std::byte> bytes;
            double logprob;
            std::string token;
            std::vector<TopLogprob> top_logprobs;
        };

        using Annotation = std::variant<FileCitation, UrlCitation, ContainerFileCitation, FilePath>;

        KindOutputText type{};
        std::vector<Annotation> annotations;
        std::vector<LogProb> logprobs;
        std::string text;
    };

    struct Refusal {
        KindRefusal type{};
        std::string refusal;
    };

    struct Text {
        KindInputText type{};
        std::string text;
    };
};


struct WebSearchToolActions {
    struct Find {
        KindFindAction type{};
        std::string pattern;
        EncodedUrl url;
    };

    struct OpenPage {
        KindOpenPageAction type{};
        EncodedUrl url;
    };

    struct Search {
        struct Source {
            KindSearchActionSource type{};
            EncodedUrl url;
        };

        KindSearchAction type{};
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
        KindInputMessage type{};
    };

    struct Item {
        struct InputMessage {
            std::vector<MessageContentUnit> content;
            RoleUser role;
            ItemStatus status;
            KindInputMessage type{};
        };

        struct OutputMessage {
            using Content = std::variant<ContentTypes::OutputText, ContentTypes::Refusal>;

            std::vector<Content> content;
            std::string id;
            RoleAssistant role;
            ItemStatus status;
            KindOutputMessage type{};
        };

        struct FileSearchToolCall {
            struct Result {
                std::optional<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>> attributes;
                std::string file_id;
                std::string filename;
                double score;
                std::string text;
            };

            KindFileSearchToolCall type{};
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

            KindComputerCall type{};
            ComputerToolActions::All action;
            std::string call_id;
            std::string id;
            std::vector<PendingSafetyCheck> pending_safety_checks;
            ItemStatus status;
        };

        struct ComputerToolCallOutput {
            struct ComputerScreenshot {
                KindComputerScreenshot type{};
                std::string file_id;
                EncodedUrl image_url;
            };

            struct AcknowledgedSafetyCheck {
                std::string id;
                std::string code;
                std::string message;
            };

            KindComputerCallOutput type{};
            std::string call_id;
            ComputerScreenshot output;
            std::optional<std::vector<AcknowledgedSafetyCheck>> acknowledged_safety_checks;
            std::string id;
            ItemStatus status;
        };

        struct WebSearchToolCall {
            KindWebSearchCall type{};
            WebSearchToolActions::All action;
            std::string id;
            ItemStatus status; // Doc says "status" but doesn't list enum. Assuming ItemStatus.
        };

        struct FunctionToolCall {
            KindFunctionCall type{};
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
            KindFunctionCallOutput type{};
            std::string call_id;
            Output output;
            std::string id;
            ItemStatus status;
        };

        struct Reasoning {
            struct Summary {
                KindReasoningSummaryText type{};
                std::string text;
            };

            struct Content {
                KindReasoningText type{};
                std::string text;
            };

            KindReasoningItem type{};
            std::string id;
            std::vector<Summary> summary;
            std::optional<std::vector<Content>> content;
            std::string encrypted_content;
            ItemStatus status;
        };

        struct CompactionItem {
            KindCompactionItem type{};
            std::string encrypted_content;
            std::string id;
            std::string created_by;
        };

        struct ImageGenerationCall {
            KindImageGenerationCall type{};
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

            KindCodeInterpreterCall type{};
            std::string code;
            std::string container_id;
            std::string id;
            std::vector<Output> outputs;
            CodeInterpreterStatus status;
        };

        struct LocalShellCall {
            struct Action {
                KindLocalShellAction type{};
                std::vector<std::string> command;
                std::map<std::string, std::string> env;
                int64_t timeout_ms;
                std::string user;
                std::string working_directory;
            };

            KindLocalShellCall type{};
            Action action;
            std::string call_id;
            std::string id;
            ItemStatus status;
        };

        struct LocalShellCallOutput {
            KindLocalShellCallOutput type{};
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

            KindShellCall type{};
            Action action;
            std::string call_id;
            std::string id;
            ItemStatus status;
            std::string created_by;
        };

        struct ShellToolCallOutput {
            struct ShellCallExitOutcome {
                KindShellExitOutcome type{};
                int64_t exit_code;
            };

            struct ShellCallTimeoutOutcome {
                KindShellTimeoutOutcome type{};
            };

            struct Output {
                using Outcome = std::variant<ShellCallExitOutcome, ShellCallTimeoutOutcome>;

                Outcome outcome;
                std::string std_err;
                std::string std_out;
                std::string created_by;
            };

            KindShellCallOutput type{};
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
            KindApplyPatchCall type{};
            std::string call_id;
            ApplyPatchOperation operation;
            ApplyPatchCallStatus status;
            std::string id;
            std::string created_by;
        };

        struct ApplyPatchToolCallOutput {
            KindApplyPatchCallOutput type{};
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

            KindMCPListTools type{};
            std::string id;
            std::string server_label;
            std::vector<ToolDef> tools;
            std::string error;
        };

        struct MCPApprovalRequest {
            KindMCPApprovalRequest type{};
            std::string arguments;
            std::string id;
            std::string name;
            std::string server_label;
        };

        struct MCPApprovalResponse {
            KindMCPApprovalResponse type{};
            std::string approval_request_id;
            bool approve;
            std::string id;
            std::string reason;
        };

        struct MCPToolCall {
            KindMCPCall type{};
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
            KindCustomToolCallOutput type{};
            std::string call_id;
            Output output;
            std::string id;
        };

        struct CustomToolCall {
            KindCustomToolCall type{};
            std::string call_id;
            std::string input;
            std::string name;
            std::string id;
        };
    };
};


struct ItemReference {
    KindItemReference type{};
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
        KindFunctionTool type{};
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

        KindFileSearchTool type{};
        std::vector<std::string> vector_store_ids;
        std::optional<std::variant<ComparisonFilter, CompoundFilter>> filters;
        int64_t max_num_results;
        RankingOptions ranking_options;
    };

    struct ComputerUse {
        KindComputerUseTool type{};
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

        KindMCPTool type{};
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
            KindContainerConfig type{};
            std::optional<std::vector<std::string>> file_ids;
            std::string memory_limit;
        };
        using Container = std::variant<std::string, ContainerConfig>;

        KindCodeInterpreterTool type{};
        Container container;
    };

    struct ImageGeneration {
        struct Mask {
            std::string file_id;
            EncodedUrl image_url;
        };

        KindImageGenerationTool type{};
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
        KindLocalShellTool type{};
    };

    struct Shell {
        KindShellTool type{};
    };

    struct Custom {
        struct GrammarFormat {
            KindCustomToolGrammarFormat type{};
            std::string definition;
            GrammarSyntax syntax;
        };

        struct TextFormat {
            KindCustomToolTextFormat type{};
        };

        using Format = std::variant<GrammarFormat, TextFormat>;

        KindCustomTool type{};
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
        KindApplyPatchTool type{};
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

        KindAllowedToolsChoice type{};
        ToolChoiceModeNotNone mode;
        std::vector<RestrictedTool> tools;
    };

    struct Custom {
        KindCustomToolChoice type{};
        std::string name;
    };

    struct Function {
        KindFunctionToolChoice type{};
        std::string name;
    };

    struct Hosted {
        HostedToolMode type;
    };

    struct MCP {
        KindMCPToolChoice type{};
        std::string server_label;
        std::string name;
    };

    struct SpecificApplyPatch {
        KindSpecificApplyPatchToolChoice type{};
    };

    struct SpecificShell {
        KindSpecificShellToolChoice type{};
    };
};


using ToolChoice = std::variant<ToolChoiceMode,
                                ToolsChoiceTypes::Allowed, ToolsChoiceTypes::Custom, ToolsChoiceTypes::Function,
                                ToolsChoiceTypes::Hosted, ToolsChoiceTypes::MCP, ToolsChoiceTypes::SpecificApplyPatch,
                                ToolsChoiceTypes::SpecificShell>;


}
