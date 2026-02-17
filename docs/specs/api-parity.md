---
description: how to audit and update C++ protocol structs for API parity
---
# API Parity Workflow
This workflow describes the methodology for ensuring C++ structures (specifically the OpenAI Responses protocol) precisely match the live API documentation.
## 1. Documentation Snapshotting
Instead of live browser auditing, always capture a static, fully-expanded markdown version of the API documentation.
- **URL Pattern**: Use the `index.md` endpoint of the OpenAI developer portal (e.g., `https://developers.openai.com/api/reference/resources/responses/methods/create/index.md`).
- **Capture**: Use `curl -o docs/specs/protocols/openai/responses.md [URL]`.
- **Reference**: Always reference this local `responses.md` file for field names, types, and requirement status to save quota and ensure accuracy.
## 2. Coding Standards for Protocol Structs
Follow these rules when mapping API objects to C++. These rules apply strictly to the protocol translation layer (`interface/protocols/...`).
### Rule Zero: No Naked Types
Every member variable in a protocol struct **must** be wrapped. Never use raw types (e.g., `int64_t x;`) as they lead to uninitialized "garbage" values and debugging nightmares.
- **Goal**: Ensure the struct cannot be constructed in an invalid or "garbage" state.
### Request Structs (Request Body)
*   **Spec-Required Fields**: Wrap in `Required<T>`. 
    - *Rationale*: Forces explicit initialization in code. The struct cannot be aggregate-initialized without providing these.
*   **Spec-Optional Fields**: Wrap in `std::optional<T>` and initialize with `{}`.
    - *Rationale*: We use **omission** as the default. We let the server decide its own default values by simply not sending the key.
### Response Structs (Returns)
*   **Spec-Required Fields**: Wrap in `Required<T>`.
    - *Rationale*: Tells the deserializer that this field **must** be present in the JSON.
*   **Spec-Optional Fields (All)**: Wrap in `std::optional<T>` and initialize with `{}`.
    - *Rationale*: Because the deserializer uses aggregate initialization/positional extraction, `std::optional` is necessary to notify the parser that the field may be missing. 
    - *Note*: Even if the spec has a default (e.g., `1.0`), we currently use `std::optional` to handle the "missing" state safely during parsing.
### Kind/Type Fields
*   Always wrap in `Required<KindStruct> type{{}};`.
*   *Rationale*: This is a structural constant. Using `Required` ensures it's part of the layout, and `{{}}` value-initializes it to the correct static value defined in the `Kind` struct.
### Data Types
*   **Timestamps**: Always use `int64_t` for Unix timestamps.
*   **URLs**: Use `EncodedUrl`.
*   **Enums**: Map fixed string sets to `enum class` in `responses_enums.hpp`.
### File Organization
1. `responses_enums.hpp`: Shared enums and `Kind` struct definitions.
2. `responses_common.hpp`: Shared objects (Reasoning, Usage, ToolActions, etc).
3. `responses_req.hpp`: Request tools and input items.
4. `responses.hpp`: Top-level `Request` and `Response` aggregator structs.
## 3. Audit Procedure
1. Pull a fresh `responses.md`.
2. Use `grep_search` to find the object in the spec.
3. Verify every field in the `.hpp` matches the `responses.md`:
   - Name match?
   - Type match (string -> string, number -> int64_t/double)?
   - Required status match?
4. Look for new "expandable" sections in the spec that might be missed by a casual glance.