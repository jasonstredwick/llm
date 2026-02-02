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


namespace jai::llm::openai::request {


struct ContentTypes {
    struct File {
        InputFileKind type = InputFileKind::INPUT_FILE;
        std::optional<std::string> file_data{};
        std::optional<std::string> file_id{};
        std::optional<EncodedUrl> file_url{};
        std::optional<std::string> filename{};
    };

    struct Image {
        InputImageKind type = InputImageKind::INPUT_IMAGE;
        Detail detail; // Required defaults to auto, but marked required in doc
        std::optional<std::string> file_id{};
        std::optional<EncodedUrl> image_url{};
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
        std::optional<std::vector<std::string>> queries{};
        std::optional<std::vector<Source>> sources{};
    };

    using All = std::variant<Find, OpenPage, Search>;
};


struct InputTypes {
    using MessageContentUnit = std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>;

    struct Message {
        using Content = std::variant<std::string, std::vector<MessageContentUnit>>;

        Content content;
        RoleInputMessage role;
        std::optional<InputMessageKind> type{}; // InputMessageKind::MESSAGE
    };

    struct Item {
        struct InputMessage {
            std::vector<MessageContentUnit> content;
            RoleUser role;
            std::optional<ItemStatus> status{};
            std::optional<InputMessageKind> type{};
        };

        struct OutputMessage {
            using Content = std::variant<ContentTypes::OutputText, ContentTypes::Refusal>;

            std::vector<Content> content;
            std::string id;
            RoleAssistant role;
            std::optional<ItemStatus> status{};
            std::optional<InputMessageKind> type{};
        };

        struct FileSearchToolCall {
            struct Result {
                std::optional<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>> attributes{};
                std::optional<std::string> file_id{};
                std::optional<std::string> filename{};
                std::optional<double> score{};
                std::optional<std::string> text{};
            };

            FileSearchToolCallKind type = FileSearchToolCallKind::FILE_SEARCH_CALL;
            std::string id;
            std::vector<std::string> queries;
            FileSearchStatus status;
            std::optional<std::vector<Result>> results{};
        };

        struct ComputerToolCall {
            struct PendingSafetyCheck {
                std::string id;
                std::optional<std::string> code{};
                std::optional<std::string> message{};
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
                std::optional<std::string> file_id{};
                std::optional<EncodedUrl> image_url{};
            };

            struct AcknowledgedSafetyCheck {
                std::string id;
                std::optional<std::string> code{};
                std::optional<std::string> message{};
            };

            ComputerCallOutputKind type = ComputerCallOutputKind::COMPUTER_CALL_OUTPUT;
            std::string call_id;
            ComputerScreenshot output;
            std::optional<std::vector<AcknowledgedSafetyCheck>> acknowledged_safety_checks{};
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
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
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
        };

        struct FunctionToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            FunctionCallOutputKind type = FunctionCallOutputKind::FUNCTION_CALL_OUTPUT;
            std::string call_id;
            Output output;
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
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
            std::optional<std::vector<Content>> content{};
            std::optional<std::string> encrypted_content{};
            std::optional<ItemStatus> status{};
        };

        struct CompactionItem {
            CompactionItemKind type = CompactionItemKind::COMPACTION;
            std::string encrypted_content;
            std::optional<std::string> id{};
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
                std::optional<int64_t> timeout_ms{};
                std::optional<std::string> user{};
                std::optional<std::string> working_directory{};
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
            std::optional<ItemStatus> status{};
        };

        struct ShellToolCall {
            struct Action {
                std::vector<std::string> commands;
                std::optional<int64_t> max_output_length{};
                std::optional<int64_t> timeout_ms{};
            };

            ShellCallKind type = ShellCallKind::SHELL_CALL;
            Action action;
            std::string call_id;
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
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
            };

            ShellCallOutputKind type = ShellCallOutputKind::SHELL_CALL_OUTPUT;
            std::string call_id;
            std::vector<Output> output;
            std::optional<std::string> id{};
            std::optional<int64_t> max_output_length{};
            std::optional<ItemStatus> status{};
        };

        struct ApplyPatchToolCall {
            using ApplyPatchOperation = std::variant<PatchFileOperations::Create,
                                                     PatchFileOperations::Delete,
                                                     PatchFileOperations::Update>;
            ApplyPatchCallKind type = ApplyPatchCallKind::APPLY_PATCH_CALL;
            std::string call_id;
            ApplyPatchOperation operation;
            ApplyPatchCallStatus status;
            std::optional<std::string> id{};
        };

        struct ApplyPatchToolCallOutput {
            ApplyPatchCallOutputKind type = ApplyPatchCallOutputKind::APPLY_PATCH_CALL_OUTPUT;
            std::optional<std::string> call_id;
            ApplyPatchCallOutputStatus status;
            std::optional<std::string> id{};
            std::optional<std::string> output{};
        };

        struct MCPListTools {
            struct ToolDef {
                jai::llm::json::Object input_schema;
                std::string name;
                std::optional<jai::llm::json::Object> annotations{};
                std::optional<std::string> description{};
            };

            MCPListToolsKind type = MCPListToolsKind::MCP_LIST_TOOLS;
            std::string id;
            std::string server_label;
            std::vector<ToolDef> tools;
            std::optional<std::string> error{};
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
            std::optional<std::string> id{};
            std::optional<std::string> reason{};
        };

        struct MCPToolCall {
            MCPCallKind type = MCPCallKind::MCP_CALL;
            std::string arguments;
            std::string id;
            std::string name;
            std::string server_label;
            std::optional<std::string> approval_request_id{};
            std::optional<std::string> error{};
            std::optional<std::string> output{};
            std::optional<CallStatus> status{};
        };

        struct CustomToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            CustomToolCallOutputKind type = CustomToolCallOutputKind::CUSTOM_TOOL_CALL_OUTPUT;
            std::string call_id;
            Output output;
            std::optional<std::string> id{};
        };

        struct CustomToolCall {
            CustomToolCallKind type = CustomToolCallKind::CUSTOM_TOOL_CALL;
            std::string call_id;
            std::string input;
            std::string name;
            std::optional<std::string> id{};
        };
    };
};


struct ItemReference {
    std::optional<ItemReferenceKind> type{}; // ItemReferenceKind::ITEM_REFERENCE
    std::string id;
};


using Item = std::variant<
    InputTypes::Item::InputMessage,
    InputTypes::Item::OutputMessage,
    InputTypes::Item::FileSearchToolCall,
    InputTypes::Item::ComputerToolCall,
    InputTypes::Item::ComputerToolCallOutput,
    InputTypes::Item::WebSearchToolCall,
    InputTypes::Item::FunctionToolCall,
    InputTypes::Item::FunctionToolCallOutput,
    InputTypes::Item::Reasoning,
    InputTypes::Item::CompactionItem,
    InputTypes::Item::ImageGenerationCall,
    InputTypes::Item::CodeInterpreterToolCall,
    InputTypes::Item::LocalShellCall,
    InputTypes::Item::LocalShellCallOutput,
    InputTypes::Item::ShellToolCall,
    InputTypes::Item::ShellToolCallOutput,
    InputTypes::Item::ApplyPatchToolCall,
    InputTypes::Item::ApplyPatchToolCallOutput,
    InputTypes::Item::MCPListTools,
    InputTypes::Item::MCPApprovalRequest,
    InputTypes::Item::MCPApprovalResponse,
    InputTypes::Item::MCPToolCall,
    InputTypes::Item::CustomToolCallOutput,
    InputTypes::Item::CustomToolCall
>;
using InputItemList = std::variant<InputTypes::Message, Item, ItemReference>;
using Input = std::variant<std::string, std::vector<InputItemList>>;


struct Prompt {
    using VariableTypes = std::variant<std::string, ContentTypes::Text, ContentTypes::Image, ContentTypes::File>;

    std::string id;
    std::optional<std::map<std::string, VariableTypes>> variables{};
    std::optional<std::string> version{};
};


struct ToolTypes {
    struct Function {
        FunctionToolKind type = FunctionToolKind::FUNCTION;
        std::string name;
        jai::llm::json::Object parameters;
        bool strict;
        std::optional<std::string> description{};
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

            std::optional<HybridSearch> hybrid_search{};
            std::optional<std::string> ranker{};
            std::optional<double> score_threshold{};
        };

        FileSearchToolKind type = FileSearchToolKind::FILE_SEARCH;
        std::vector<std::string> vector_store_ids;
        std::optional<std::variant<ComparisonFilter, CompoundFilter>> filters;
        std::optional<int64_t> max_num_results{};
        std::optional<RankingOptions> ranking_options{};
    };

    struct ComputerUse {
        ComputerUseToolKind type = ComputerUseToolKind::COMPUTER_USE_PREVIEW;
        int64_t display_height;
        int64_t display_width;
        std::string environment;
    };

    struct WebSearch {
        struct Filters {
            std::optional<std::vector<std::string>> allowed_domains{};
        };

        struct Location {
            LocationType type = LocationType::APPROXIMATE;
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
        };

        WebSearchToolKind type = WebSearchToolKind::WEB_SEARCH;
        std::optional<Filters> filters{};
        std::optional<SearchContextSize> search_context_size{};
        std::optional<Location> user_location{};
    };

    struct MCP {
        struct Filter {
            std::optional<bool> read_only{};
            std::optional<std::vector<std::string>> tool_names{};
        };

        struct ApprovalFilter {
            std::optional<Filter> always{};
            std::optional<Filter> never{};
        };

        using AllowedTools = std::variant<std::vector<std::string>, Filter>;
        using RequiredApproval = std::variant<MCPApprovalSetting, ApprovalFilter>;

        MCPToolKind type = MCPToolKind::MCP;
        std::string server_label;
        std::optional<AllowedTools> allowed_tools{};
        std::optional<std::string> authorization{};
        std::optional<ConnectId> connector_id{};
        std::optional<std::map<std::string, std::string>> headers{};
        std::optional<RequiredApproval> require_approval{};
        std::optional<std::string> server_description{};
        std::optional<EncodedUrl> server_url{};
    };

    struct CodeInterpreter {
        struct ContainerConfig {
            ContainerConfigKind type = ContainerConfigKind::AUTO;
            std::optional<std::vector<std::string>> file_ids{};
            std::optional<std::string> memory_limit{};
        };
        using Container = std::variant<std::string, ContainerConfig>;

        CodeInterpreterToolKind type = CodeInterpreterToolKind::CODE_INTERPRETER;
        Container container;
    };

    struct ImageGeneration {
        struct Mask {
            std::optional<std::string> file_id{};
            std::optional<EncodedUrl> image_url{};
        };

        ImageGenerationToolKind type = ImageGenerationToolKind::IMAGE_GENERATION;
        std::optional<std::string> action{};
        std::optional<ImageGenerationBackground> background{};
        std::optional<ImageGenerationFidelity> input_fidelity{};
        std::optional<Mask> input_image_mask{};
        std::optional<std::string> model{};
        std::optional<std::string> moderation{};
        std::optional<int64_t> output_compression{};
        std::optional<ImageGenerationFormat> output_format{};
        std::optional<int64_t> partial_images{};
        std::optional<ImageGenerationQuality> quality{};
        std::optional<ImageGenerationSize> size{};
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
        std::optional<std::string> description{};
        std::optional<Format> format{};
    };

    struct WebSearchPreview {
        struct Location {
            LocationType type = LocationType::APPROXIMATE;
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
        };

        WebSearchPreviewToolKind type;
        std::optional<SearchContextSize> search_context_size{};
        std::optional<Location> user_location{};
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
            std::optional<std::string> name{};
            std::optional<std::string> server_label{};
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
        std::optional<std::string> name{};
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
