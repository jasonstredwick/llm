# Protocol Coverage

Per-provider inventory of implemented data structures, enums, and file locations.

## Anthropic — Messages API (`/v1/messages`)

**Namespace:** `jai::llm::anthropic`

### Files

| File | Contents |
|------|----------|
| `interface/protocols/anthropic/messages.hpp` | All request and response structs |
| `interface/protocols/anthropic/strings.hpp` | `to_string_view` / `from_string_view` for all Kinds and enums |
| `src/protocols/serialize/anthropic_messages.cpp` | JSON serialization |
| `src/protocols/deserialize/anthropic_messages.cpp` | JSON deserialization |
| `tests/unit/test_anthropic.cpp` | Unit tests |

### Kind Tags (31)

KindBase64Source, KindCharLocation, KindContentBlockLocation, KindContentSource, KindCustomTool,
KindDocumentBlock, KindEphemeral, KindImageBlock, KindJsonSchemaType, KindMessage,
KindPageLocation, KindPlainTextSource, KindRedactedThinkingBlock, KindSearchResultBlock,
KindSearchResultLocation, KindServerToolUseBlock, KindStructuredOutputFormat, KindTextBlock,
KindThinkingBlock, KindToolBash20250124Type, KindToolChoiceAny, KindToolChoiceAuto,
KindToolChoiceNone, KindToolChoiceTool, KindToolResultBlock, KindToolTextEditor20250124Name,
KindToolTextEditor20250429Name, KindToolTextEditor20250728Name, KindToolUseBlock, KindUrlSource,
KindWebSearchResultLocation, KindWebSearchTool20250305, KindWebSearchToolResultBlock,
KindWebSearchToolResultError.

### Enums (21)

CacheControlTTL, CitationKinds, DocSrcKind, ImageMediaType, ImageSourceKinds, PDFMediaType,
PlainTextMediaType, ReplaceBasedEditor, ReplaceEditor, RequestServiceTier,
ResponseContentBlockKinds, ResponseRole, Role, StopReason, ThinkingConfigType, ThinkingEffort,
ToolBash20250124Name, UsageServiceTier, UserLocationType, WebSearchName,
WebSearchToolResultErrorCode, WebSearchToolResultErrorType.

### Request Structures

Top-level: `Request`, `MessageParam`, `System` (variant).

Content blocks: `TextBlockParam`, `ImageBlockParam`, `DocumentBlockParam`,
`SearchResultBlockParam`, `ThinkingBlockParam`, `RedactedThinkingBlockParam`,
`ToolUseBlockParam`, `ToolResultBlockParam`, `ServerToolUseBlockParam`,
`WebSearchToolResultBlockParam` → aggregated as `ContentBlockParam` variant.

Image sources: `Base64ImageSource`, `URLImageSource` → `ImageSource` variant.

Document sources: `Base64PDFSource`, `PlainTextSource`, `ContentBlockSource`, `URLPDFSource`
→ `DocumentSource` variant.

Citations: `CitationCharLocationParam`, `CitationPageLocationParam`,
`CitationContentBlockLocationParam`, `CitationWebSearchResultLocationParam`,
`CitationSearchResultLocationParam` → `TextCitationParam` variant.

Tools: `Tool` (custom), `ToolBash20250124`, `ToolTextEditor20250124`, `ToolTextEditor20250429`,
`ToolTextEditor20250728`, `WebSearchTool20250305` → `ToolUnion` variant.

Tool choice: `ToolChoiceAuto`, `ToolChoiceAny`, `ToolChoiceTool`, `ToolChoiceNone`
→ `ToolChoice` variant.

Config: `CacheControlEphemeral`, `Metadata`, `CitationsConfigParam`, `OutputConfig`,
`ThinkingConfigEnabled`, `ThinkingConfigDisabled`, `ThinkingConfigAdaptive`
→ `ThinkingConfig` variant.

### Response Structures

Top-level: `Response`.

Content blocks: `TextBlock`, `ThinkingBlock`, `RedactedThinkingBlock`, `ToolUseBlock`,
`ServerToolUseBlock`, `WebSearchToolResultBlock` → `ResponseContentBlock` variant.

Citations: `CitationCharLocation`, `CitationPageLocation`, `CitationContentBlockLocation`,
`CitationsWebSearchResultLocation`, `CitationsSearchResultLocation` → `TextCitation` variant.

Usage: `Usage`, `CacheCreation`, `ServerToolUsage`.

---

## Gemini — GenerateContent

**Namespace:** `jai::llm::gemini`

### Files

| File | Contents |
|------|----------|
| `interface/protocols/gemini/generate_content.hpp` | All request and response structs |
| `interface/protocols/gemini/strings.hpp` | `to_string_view` / `from_string_view` |
| `src/protocols/serialize/gemini_generate_content.cpp` | JSON serialization |
| `src/protocols/deserialize/gemini_generate_content.cpp` | JSON deserialization |
| `tests/unit/test_gemini.cpp` | Unit tests |

### Enums (26)

AspectRatio, Behavior, BlockReason, CodeLanguage, DynamicRetrievalMode, Environment,
ExecutionOutcome, FinishReason, HarmBlockThreshold, HarmCategory, HarmProbability, ImageDim,
MediaResolution, MediaType, Modality, ModelStage, ResponseMimeType, Role, Scheduling, SchemaType,
ThinkingLevel, ToolMode, UrlRetrievalStatus.

### Request Structures

Top-level: `Request`.

Content: `RequestContent`, `RequestContent::RequestPart`. Part data variant includes `Blob`,
`CodeExecutionResult`, `FileData`, `FunctionResponse`, `Text`. Metadata variant: `VideoMetadata`.

Tools: `CodeExecution`, `ComputerUse`, `FileSearch`, `FunctionDeclaration`, `GoogleMaps`,
`GoogleSearch`, `GoogleSearchRetrieval`, `UrlContext` → `Tool` variant.

Config: `GenerationConfig`, `ThinkingConfig`, `SpeechConfig` (with `VoiceConfig`,
`MultiSpeakerVoiceConfig`), `ImageConfig`, `SafetySetting`, `ToolConfig` (with
`FunctionCallingConfig`, `RetrievalConfig`).

Schema: `Schema` (recursive via `ValueBox<Schema>`).

### Response Structures

Top-level: `Response`.

Content: `ResponseContent`, `ResponseContent::ResponsePart`. Part data variant includes `Blob`,
`ExecutableCode`, `FileData`, `FunctionCall`, `Text`.

Candidates: `Candidate` (with `CitationMetadata`, `GroundingMetadata`, `LogprobsResult`,
`SafetyRating`, `UrlContextMetadata`).

Grounding: `GroundingChunk` (Maps, RetrievedContext, Web variants), `GroundingSupport`,
`SearchEntryPoint`.

Metadata: `UsageMetadata` (with `ModalityTokenCount`), `PromptFeedback`, `ModelStatus`.

---

## OpenAI — Responses API

**Namespace:** `jai::llm::openai`

### Files

| File | Contents |
|------|----------|
| `interface/protocols/openai/responses_enums.hpp` | Kind tags and enum definitions |
| `interface/protocols/openai/responses_common.hpp` | Shared substructures |
| `interface/protocols/openai/responses_req.hpp` | Request-specific types (namespace `request`) |
| `interface/protocols/openai/responses_res.hpp` | Response-specific types (namespace `response`) |
| `interface/protocols/openai/responses.hpp` | Top-level `Request` and `Response` |
| `interface/protocols/openai/strings.hpp` | `to_string_view` / `from_string_view` |
| `src/protocols/serialize/openai_responses.cpp` | JSON serialization |
| `src/protocols/deserialize/openai_responses.cpp` | JSON deserialization |
| `tests/unit/test_openai.cpp` | Unit tests |

### Kind Tags (~60+)

The OpenAI Responses API has the largest surface area. Kind tags cover: tool calls (function,
computer use, code interpreter, file search, web search, image generation, local shell, MCP,
apply patch, shell), input/output message types, content types, computer actions, patch file
operations, citations, reasoning items, and more.

### Enums (~55+)

Covers: action types, call statuses, tool types, format types, service tiers, reasoning settings,
image generation options, search settings, role types, and many more.

### Request Structures (namespace `openai::request`)

Content types: `ContentTypes::File`, `ContentTypes::Image`, `ContentTypes::OutputText` (with
citation sub-types), `ContentTypes::Text`.

Input items: `InputTypes::Message`, plus tool call types for ApplyPatch, CodeInterpreter,
Computer, CustomTool, FileSearch, FunctionCall, ImageGeneration, LocalShell, MCP,
MCPApproval, Reasoning, Shell, WebSearch — each with their output counterparts.

Tools: ApplyPatch, CodeInterpreter, ComputerUse, CustomTool, FileSearch, Function,
ImageGeneration, LocalShell, MCP, Shell, WebSearch, WebSearchPreview → `Tool` variant.

Tool choice: Mode-based (none/auto/required) and specific tool choices → `ToolChoice` variant.

### Response Structures (namespace `openai::response`)

Output items mirror the input items but with response-specific fields and statuses.

Tools: Response-side tool definitions.

Prompt: Response-side prompt representation.

### Common Structures

`ComputerToolActions` (Click, DoubleClick, Drag, KeyPress, Move, Screenshot, Scroll, Type, Wait),
`ConversationRef`, `ContextManagement`, `Reasoning`, `ResponseUsage`, `TextConfig`,
`PatchFileOperations`.

---

## Projections

**Namespace:** `jai::llm::proj::text`

### File

`interface/projections/text.hpp`

### Coverage

Provides `Generate<T>()` template specializations for `anthropic::Request`, `gemini::Request`,
and `openai::Request`. Maps a normalized `Options` + `Prompt` + `Block` (text/image) into
provider-specific request structures.

Currently handles: text prompts, base64 images, system prompts, thinking effort mapping,
temperature, top_k, top_p, max_output_tokens.

Not yet handled: tools, streaming, multi-turn conversations, document/file content.

---

## API Spec Snapshots

The `docs/specs/` directory contains local snapshots of provider API documentation used during
struct auditing. See `docs/specs/api-parity.md` for the workflow.

Currently present: `docs/specs/protocols/openai/responses.md` (OpenAI Responses API snapshot).
