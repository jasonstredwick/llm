# Provider API Shells

The JAI LLM project uses a "Shared-Nothing" architecture for its provider API shells. This ensures maximum isolation, avoids fragile base class hierarchies, and eliminates linker collisions.

## Core Features
- **Isolated Headers**: Each provider (Gemini, OpenAI, Anthropic) has its own header file containing all necessary structs and enums.
- **`constexpr` Converters**: String conversions (e.g., `to_string_view`) are implemented as `constexpr` template specializations.
- **ODR Safety**: By using `constexpr` and fully qualified types in specializations, the library is safe to include in multiple translation units.
- **Zero Centralization**: No central `enum_converters.cpp` or `base.hpp` registry is required for conversions.

## Architecture Guidelines
1. **Public Interface**: The base conversion template resides in `interface/llm.hpp` under the `jai::llm` namespace.
2. **Provider Namespace**: Header logic is encapsulated in `jai::llm::providers::[provider_name]`.
3. **Include Order**:
    - System headers (e.g., `<string>`, `<vector>`)
    - Project interface headers (`../../interface/llm.hpp`)
4. **Qualification**: Always use full qualification for types in template specializations to avoid ambiguity.

## Supported Providers (Current Version Shells)
- **Gemini**: 3.0, 2.5
- **OpenAI**: 5.2, 4o, 4.0
- **Anthropic**: 4.5 Opus, 4.5 Sonnet
