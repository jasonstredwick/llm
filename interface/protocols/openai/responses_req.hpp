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


namespace jai::llm::openai::request {


struct ContentTypes {
    struct File {
        Required<KindInputFile> type{{}};
        std::optional<std::string> file_data{};
        std::optional<std::string> file_id{};
        std::optional<EncodedUrl> file_url{};
        std::optional<std::string> filename{};
    };

    struct Image {
        Required<KindInputImage> type{{}};
        Required<Detail> detail; // Required defaults to auto, but marked required in doc
        std::optional<std::string> file_id{};
        std::optional<EncodedUrl> image_url{};
    };

    struct OutputText {
        struct ContainerFileCitation {
            Required<KindContainerFileCitation> type{{}};
            Required<std::string> container_id;
            Required<int64_t> end_index;
            Required<std::string> file_id;
            Required<std::string> filename;
            Required<int64_t> start_index;
        };

        struct FileCitation {
            Required<KindFileCitation> type{{}};
            Required<std::string> file_id;
            Required<std::string> filename;
            Required<int64_t> index;
        };

        struct UrlCitation {
            Required<KindUrlCitation> type{{}};
            Required<int64_t> end_index;
            Required<int64_t> start_index;
            Required<std::string> title;
            Required<EncodedUrl> url;
        };

        struct FilePath {
            Required<KindFilePath> type{{}};
            Required<std::string> file_id;
            Required<int64_t> index;
        };

        struct LogProb {
            struct TopLogprob {
                Required<std::vector<std::byte>> bytes;
                Required<double> logprob;
                Required<std::string> token;
            };

            Required<std::vector<std::byte>> bytes;
            Required<double> logprob;
            Required<std::string> token;
            Required<std::vector<TopLogprob>> top_logprobs;
        };

        using Annotation = std::variant<FileCitation, UrlCitation, ContainerFileCitation, FilePath>;

        Required<KindOutputText> type{{}};
        Required<std::vector<Annotation>> annotations;
        Required<std::vector<LogProb>> logprobs;
        Required<std::string> text;
    };

    struct Refusal {
        Required<KindRefusal> type{{}};
        Required<std::string> refusal;
    };

    struct Text {
        Required<KindInputText> type{{}};
        Required<std::string> text;
    };
};


struct WebSearchToolActions {
    struct Find {
        Required<KindFindAction> type{{}};
        Required<std::string> pattern;
        Required<EncodedUrl> url;
    };

    struct OpenPage {
        Required<KindOpenPageAction> type{{}};
        Required<EncodedUrl> url;
    };

    struct Search {
        struct Source {
            Required<KindSearchActionSource> type{{}};
            Required<EncodedUrl> url;
        };

        Required<KindSearchAction> type{{}};
        std::optional<std::vector<std::string>> queries{};
        std::optional<std::vector<Source>> sources{};
    };

    using All = std::variant<Find, OpenPage, Search>;
};


struct InputTypes {
    using MessageContentUnit = std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>;

    struct Message {
        using Content = std::variant<std::string, std::vector<MessageContentUnit>>;

        Required<KindInputMessage> type{{}};
        Required<Content> content;
        Required<RoleInputMessage> role;
    };

    struct Item {
        struct InputMessage {
            Required<std::vector<MessageContentUnit>> content;
            Required<RoleUser> role;
            std::optional<ItemStatus> status{};
            std::optional<KindInputMessage> type{};
        };

        struct OutputMessage {
            using Content = std::variant<ContentTypes::OutputText, ContentTypes::Refusal>;

            Required<std::vector<Content>> content;
            Required<std::string> id;
            Required<RoleAssistant> role;
            std::optional<ItemStatus> status{};
            std::optional<KindInputMessage> type{};
        };

        struct FileSearchToolCall {
            struct Result {
                std::optional<std::map<NameLen<64>, std::variant<NameLen<512>, bool, double>>> attributes{};
                std::optional<std::string> file_id{};
                std::optional<std::string> filename{};
                std::optional<double> score{};
                std::optional<std::string> text{};
            };

            Required<KindFileSearchToolCall> type{{}};
            Required<std::string> id;
            Required<std::vector<std::string>> queries;
            Required<FileSearchStatus> status;
            std::optional<std::vector<Result>> results{};
        };

        struct ComputerToolCall {
            struct PendingSafetyCheck {
                Required<std::string> id;
                std::optional<std::string> code{};
                std::optional<std::string> message{};
            };

            Required<KindComputerCall> type{{}};
            Required<ComputerToolActions::All> action;
            Required<std::string> call_id;
            Required<std::string> id;
            Required<std::vector<PendingSafetyCheck>> pending_safety_checks;
            Required<ItemStatus> status;
        };

        struct ComputerToolCallOutput {
            struct ComputerScreenshot {
                Required<KindComputerScreenshot> type{{}};
                std::optional<std::string> file_id{};
                std::optional<EncodedUrl> image_url{};
            };

            struct AcknowledgedSafetyCheck {
                Required<std::string> id;
                std::optional<std::string> code{};
                std::optional<std::string> message{};
            };

            Required<KindComputerCallOutput> type{{}};
            Required<std::string> call_id;
            Required<ComputerScreenshot> output;
            std::optional<std::vector<AcknowledgedSafetyCheck>> acknowledged_safety_checks{};
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
        };

        struct WebSearchToolCall {
            Required<KindWebSearchCall> type{{}};
            Required<WebSearchToolActions::All> action;
            Required<std::string> id;
            Required<ItemStatus> status; // Doc says "status" but doesn't list enum. Assuming ItemStatus.
        };

        struct FunctionToolCall {
            Required<KindFunctionCall> type{{}};
            Required<std::string> arguments;
            Required<std::string> call_id;
            Required<std::string> name;
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
        };

        struct FunctionToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            Required<KindFunctionCallOutput> type{{}};
            Required<std::string> call_id;
            Required<Output> output;
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
        };

        struct Reasoning {
            struct Summary {
                Required<KindReasoningSummaryText> type{{}};
                Required<std::string> text;
            };

            struct Content {
                Required<KindReasoningText> type{{}};
                Required<std::string> text;
            };

            Required<KindReasoningItem> type{{}};
            Required<std::string> id;
            Required<std::vector<Summary>> summary;
            std::optional<std::vector<Content>> content{};
            std::optional<std::string> encrypted_content{};
            std::optional<ItemStatus> status{};
        };

        struct CompactionItem {
            Required<KindCompactionItem> type{{}};
            Required<std::string> encrypted_content;
            std::optional<std::string> id{};
        };

        struct ImageGenerationCall {
            Required<KindImageGenerationCall> type{{}};
            Required<std::string> id;
            Required<std::string> result;
            Required<std::string> status;
        };

        struct CodeInterpreterToolCall {
            struct CodeInterpreterOutputLog {
                Required<CodeInterpreterOutputType> type;
                Required<std::string> logs;
            };

            struct CodeInterpreterOutputImage {
                Required<CodeInterpreterOutputType> type;
                Required<EncodedUrl> url;
            };

            using Output = std::variant<CodeInterpreterOutputLog, CodeInterpreterOutputImage>;

            Required<KindCodeInterpreterCall> type{{}};
            Required<std::string> code;
            Required<std::string> container_id;
            Required<std::string> id;
            Required<std::vector<Output>> outputs;
            Required<CodeInterpreterStatus> status;
        };

        struct LocalShellCall {
            struct Action {
                Required<KindLocalShellAction> type{{}};
                Required<std::vector<std::string>> command;
                Required<std::map<std::string, std::string>> env;
                std::optional<int64_t> timeout_ms{};
                std::optional<std::string> user{};
                std::optional<std::string> working_directory{};
            };

            Required<KindLocalShellCall> type{{}};
            Required<Action> action;
            Required<std::string> call_id;
            Required<std::string> id;
            Required<ItemStatus> status;
        };

        struct LocalShellCallOutput {
            Required<KindLocalShellCallOutput> type{{}};
            Required<std::string> id;
            Required<std::string> output;
            std::optional<ItemStatus> status{};
        };

        struct ShellToolCall {
            struct Action {
                Required<std::vector<std::string>> commands;
                std::optional<int64_t> max_output_length{};
                std::optional<int64_t> timeout_ms{};
            };

            Required<KindShellCall> type{{}};
            Required<Action> action;
            Required<std::string> call_id;
            std::optional<std::string> id{};
            std::optional<ItemStatus> status{};
        };

        struct ShellToolCallOutput {
            struct ShellCallExitOutcome {
                Required<KindShellExitOutcome> type{{}};
                Required<int64_t> exit_code;
            };

            struct ShellCallTimeoutOutcome {
                Required<KindShellTimeoutOutcome> type{{}};
            };

            struct Output {
                using Outcome = std::variant<ShellCallExitOutcome, ShellCallTimeoutOutcome>;

                Required<Outcome> outcome;
                Required<std::string> std_err;
                Required<std::string> std_out;
            };

            Required<KindShellCallOutput> type{{}};
            Required<std::string> call_id;
            Required<std::vector<Output>> output;
            std::optional<std::string> id{};
            std::optional<int64_t> max_output_length{};
            std::optional<ItemStatus> status{};
        };

        struct ApplyPatchToolCall {
            using ApplyPatchOperation = std::variant<PatchFileOperations::Create,
                                                     PatchFileOperations::Delete,
                                                     PatchFileOperations::Update>;
            Required<KindApplyPatchCall> type{{}};
            Required<std::string> call_id;
            Required<ApplyPatchOperation> operation;
            Required<ApplyPatchCallStatus> status;
            std::optional<std::string> id{};
        };

        struct ApplyPatchToolCallOutput {
            Required<KindApplyPatchCallOutput> type{{}};
            std::optional<std::string> call_id;
            Required<ApplyPatchCallOutputStatus> status;
            std::optional<std::string> id{};
            std::optional<std::string> output{};
        };

        struct MCPListTools {
            struct ToolDef {
                Required<jai::llm::json::Object> input_schema;
                Required<std::string> name;
                std::optional<jai::llm::json::Object> annotations{};
                std::optional<std::string> description{};
            };

            Required<KindMCPListTools> type{{}};
            Required<std::string> id;
            Required<std::string> server_label;
            Required<std::vector<ToolDef>> tools;
            std::optional<std::string> error{};
        };

        struct MCPApprovalRequest {
            Required<KindMCPApprovalRequest> type{{}};
            Required<std::string> arguments;
            Required<std::string> id;
            Required<std::string> name;
            Required<std::string> server_label;
        };

        struct MCPApprovalResponse {
            Required<KindMCPApprovalResponse> type{{}};
            Required<std::string> approval_request_id;
            Required<bool> approve;
            std::optional<std::string> id{};
            std::optional<std::string> reason{};
        };

        struct MCPToolCall {
            Required<KindMCPCall> type{{}};
            Required<std::string> arguments;
            Required<std::string> id;
            Required<std::string> name;
            Required<std::string> server_label;
            std::optional<std::string> approval_request_id{};
            std::optional<std::string> error{};
            std::optional<std::string> output{};
            std::optional<CallStatus> status{};
        };

        struct CustomToolCallOutput {
            using Output = std::variant<std::string,
                                        std::vector<
                                            std::variant<ContentTypes::Text, ContentTypes::Image, ContentTypes::File>>>;
            Required<KindCustomToolCallOutput> type{{}};
            Required<std::string> call_id;
            Required<Output> output;
            std::optional<std::string> id{};
        };

        struct CustomToolCall {
            Required<KindCustomToolCall> type{{}};
            Required<std::string> call_id;
            Required<std::string> input;
            Required<std::string> name;
            std::optional<std::string> id{};
        };
    };
};


struct ItemReference {
    std::optional<KindItemReference> type{};
    Required<std::string> id;
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

    Required<std::string> id;
    std::optional<std::map<std::string, VariableTypes>> variables{};
    std::optional<std::string> version{};
};


struct ToolTypes {
    struct Function {
        Required<KindFunctionTool> type{{}};
        Required<std::string> name;
        Required<jai::llm::json::Object> parameters;
        Required<bool> strict;
        std::optional<std::string> description{};
    };

    struct FileSearch {
        struct ComparisonFilter {
            using ValueType = std::variant<std::string, double, bool,
                                           std::vector<std::string>,
                                           std::vector<double>,
                                           std::vector<bool>>;
            Required<std::string> key;
            Required<FilterOperator> type;
            Required<ValueType> value;
        };

        struct CompoundFilter {
            Required<FilterCompoundType> type;
            Required<std::vector<std::variant<ComparisonFilter, CompoundFilter>>> filters;
        };

        struct RankingOptions {
            struct HybridSearch {
                Required<double> embedding_weight;
                Required<double> text_weight;
            };

            std::optional<HybridSearch> hybrid_search{};
            std::optional<std::string> ranker{};
            std::optional<double> score_threshold{};
        };

        Required<KindFileSearchTool> type{{}};
        Required<std::vector<std::string>> vector_store_ids;
        std::optional<std::variant<ComparisonFilter, CompoundFilter>> filters;
        std::optional<int64_t> max_num_results{};
        std::optional<RankingOptions> ranking_options{};
    };

    struct ComputerUse {
        Required<KindComputerUseTool> type{{}};
        Required<int64_t> display_height;
        Required<int64_t> display_width;
        Required<std::string> environment;
    };

    struct WebSearch {
        struct Filters {
            std::optional<std::vector<std::string>> allowed_domains{};
        };

        struct Location {
            Required<LocationType> type;
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
        };

        Required<WebSearchToolKind> type;
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

        Required<KindMCPTool> type{{}};
        Required<std::string> server_label;
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
            Required<KindContainerConfig> type{{}};
            std::optional<std::vector<std::string>> file_ids{};
            std::optional<std::string> memory_limit{};
        };
        using Container = std::variant<std::string, ContainerConfig>;

        Required<KindCodeInterpreterTool> type{{}};
        Required<Container> container;
    };

    struct ImageGeneration {
        struct Mask {
            std::optional<std::string> file_id{};
            std::optional<EncodedUrl> image_url{};
        };

        Required<KindImageGenerationTool> type{{}};
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
        Required<KindLocalShellTool> type{{}};
    };

    struct Shell {
        Required<KindShellTool> type{{}};
    };

    struct Custom {
        struct GrammarFormat {
            Required<KindCustomToolGrammarFormat> type{{}};
            Required<std::string> definition;
            Required<GrammarSyntax> syntax;
        };

        struct TextFormat {
            Required<KindCustomToolTextFormat> type{{}};
        };

        using Format = std::variant<GrammarFormat, TextFormat>;

        Required<KindCustomTool> type{{}};
        Required<std::string> name;
        std::optional<std::string> description{};
        std::optional<Format> format{};
    };

    struct WebSearchPreview {
        struct Location {
            Required<LocationType> type;
            std::optional<std::string> city{};
            std::optional<std::string> country{};
            std::optional<std::string> region{};
            std::optional<std::string> timezone{};
        };

        Required<WebSearchPreviewToolKind> type;
        std::optional<SearchContextSize> search_context_size{};
        std::optional<Location> user_location{};
    };

    struct ApplyPatch {
        Required<KindApplyPatchTool> type{{}};
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
            Required<std::string> type;
            std::optional<std::string> name{};
            std::optional<std::string> server_label{};
        };

        Required<KindAllowedToolsChoice> type{{}};
        Required<ToolChoiceModeNotNone> mode;
        Required<std::vector<RestrictedTool>> tools;
    };

    struct Custom {
        Required<KindCustomToolChoice> type{{}};
        Required<std::string> name;
    };

    struct Function {
        Required<KindFunctionToolChoice> type{{}};
        Required<std::string> name;
    };

    struct Hosted {
        Required<HostedToolMode> type;
    };

    struct MCP {
        Required<KindMCPToolChoice> type{{}};
        Required<std::string> server_label;
        std::optional<std::string> name{};
    };

    struct SpecificApplyPatch {
        Required<KindSpecificApplyPatchToolChoice> type{{}};
    };

    struct SpecificShell {
        Required<KindSpecificShellToolChoice> type{{}};
    };
};


using ToolChoice = std::variant<ToolChoiceMode,
                                ToolsChoiceTypes::Allowed, ToolsChoiceTypes::Custom, ToolsChoiceTypes::Function,
                                ToolsChoiceTypes::Hosted, ToolsChoiceTypes::MCP, ToolsChoiceTypes::SpecificApplyPatch,
                                ToolsChoiceTypes::SpecificShell>;


}
