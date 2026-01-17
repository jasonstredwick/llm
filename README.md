# llm - Unified LLM Interface

A C++ project for a unified Large Language Model access layer, designed for high performance and modern C++ standards.

## Project Structure

- **`deps/`**: (Ignored by Git) The Unified Sysroot. Contains all third-party headers and binaries.
- **`scripts/`**: Contains `manage_deps.py`, the orchestrator for the dependency factory.
- **`deps.lock.json`**: The single source of truth for all project dependencies and versions.
- **`interface/`**: Public headers for the library.
- **`src/`**: Implementation files.

## Prerequisites

- **CMake**: 3.25+
- **Python**: 3.10+
- **GitHub CLI (gh)**: Authenticated to access the binary repository.
- **Compiler Requirements**:
  - **Windows**: Visual Studio 2022 (Version 17.10 or later) is the minimum recommended version to support the C++26/23 features used in this project.
  - **MacOS**: Homebrew LLVM/Clang (version 18+) is recommended for full C++26 feature support.

## Dependency Management

The project uses a custom dependency manager (`scripts/manage_deps.py`) that runs automatically during the CMake configuration phase. 

1. **Automatic Sync**: When you run `cmake`, the script checks your local `deps/` folder against `deps.lock.json`.
2. **Missing Binaries**: If binaries for your platform are missing from the local folder, the script attempts to download them from GitHub.
3. **Build from Source**: If no pre-built binary is found on GitHub, the system automatically downloads the source, builds it locally in `third_party_workspace/`, and installs it into your `deps/` folder.
4. **Binary Uploads**: While anyone can build dependencies locally, and the system will attempt to upload new artifacts to the GitHub Release to share them with other contributors, this step requires appropriate GitHub authentication and write permissions to the repository. If unauthorized, the system will simply skip the upload and use the local build.

## Building the Library

### MacOS (ARM64 / Apple Silicon)

By default, CMake will detect your native architecture:

```bash
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B build -S .
cmake --build build --config Release
```

### MacOS (Intel / x86_64 Cross-Compilation)

You can build for Intel on an ARM Mac by explicitly setting the target architecture. The dependency manager will automatically switch to the `macos-x64` profile:

```bash
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -B build_x64 -S .
cmake --build build_x64 --config Release
```

### Windows (x64)

```powershell
cmake -B build -S .
cmake --build build --config Release
```

## Running Tests

Once built, you can run the verification tests:

```bash
# MacOS / Linux
./build/unit_test_http
./build/unit_test_curl

# Windows
.\build\Release\unit_test_http.exe
.\build\Release\unit_test_curl.exe
```

## Standards & Configuration

- **C++ Standard**: C++26 (Targeted), C++23 (Minimum fallback).
- **C Standard**: C11.
- **Library Type**: Static (`.a` on Unix, `.lib` on Windows).
- **Platform Keys**:
  - `win11-x64`: Windows 11 64-bit.
  - `macos-arm64`: Apple Silicon (M1/M2/M3).
  - `macos-x64`: Intel-based Macs or cross-compiled Intel binaries.
