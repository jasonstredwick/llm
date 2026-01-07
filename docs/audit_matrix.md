# API Audit Matrix (2026 Snapshot)

This matrix tracks the granular verification of each provider header against the official API specifications. Each cell represents a thorough audit of that specific "Part" for that provider.

| Provider | Basics | Multimodal | Extended | Tools | Response | Metadata | Safety/Grounding |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **Anthropic Opus** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **Anthropic Sonnet** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **OpenAI 5.2** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **OpenAI 4o** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **OpenAI 4.0** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **Gemini 3.0** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| **Gemini 2.5** | [x] | [x] | [x] | [x] | [x] | [x] | [x] |

## Audit Rubric Definitions

1.  **Basics**: Model IDs, message history vector, core hyperparameters (temp, top_p, etc.), stop sequences.
2.  **Multimodal**: Text, Image (URL/Base64), Audio, Video, and Content Part variants.
3.  **Extended**: Thinking controls (effort/budget), Caching keys, Response formats (JSON Schema), 2026 Speculative features.
4.  **Tools**: Function declarations, Code Interpreter, File Search, Google Search, Tool Choice logic.
5.  **Response**: Candidate/Choice structure, Refusal strings, Finish Reasons, Root identifiers.
6.  **Metadata**: Token usage details (Reasoning, Cached, etc.), Telemetry, Performance headers.
7.  **Safety/Grounding**: Filtering enums, Citations (File/URL), Search grounding, Quality scores.
