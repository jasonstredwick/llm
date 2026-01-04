# llm - Unified LLM Interface

A C++ project for a unified Large Language Model access layer.

## Project Structure

- **`deps/`**: (Ignored by Git) The Unified Sysroot. Contains all third-party headers and binaries.
- **`scripts/`**: Contains `manage_deps.py`, the orchestrator for the dependency factory.
- **`deps.lock.json`**: the single source of truth for all project dependencies and versions.
- **`third_party_workspace/`**: (Ignored by Git) A sandbox for building and packaging dependency artifacts.

## Getting Started

### Prerequisites
- CMake 3.25+
- Python 3.10+
- [GitHub CLI (gh)](https://cli.github.com/) (Authenticated to access the binary repository)

### Setup
Simply run CMake to configure the project. The dependency manager will automatically sync your local `deps/` folder with the latest binaries from GitHub:

```bash
cmake -B build -S .
```

If a binary is missing for your platform, the system will automatically:
1. Download the source archive.
2. Build it in the factory workspace.
3. Package it as an individual artifact.
4. Upload it to GitHub (if authorized).
5. Install it into your local `deps/` folder.

## Verification
You can verify your environment by building the included header test:

```bash
cmake --build build --target test_includes
.\build\Release\test_includes.exe
```
