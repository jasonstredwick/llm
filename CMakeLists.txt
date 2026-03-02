cmake_minimum_required(VERSION 3.25)

# Hard-set defaults for MacOS BEFORE the project() call
if(APPLE)
    if(NOT DEFINED CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "clang" CACHE STRING "")
    endif()
    if(NOT DEFINED CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "")
    endif()
endif()

project(llm
    VERSION 0.1.0
    DESCRIPTION "LLM access management for C/C++"
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CONFIGURATION_TYPES)
  # Multi-config generators (Visual Studio, Xcode)
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

  if(DEFINED CMAKE_BUILD_TYPE)
    message(FATAL_ERROR
      "CMAKE_BUILD_TYPE must not be set for multi-config generators")
  endif()

  message(STATUS "Build policy: multi-config generator, Release-only enforced")

else()
  # Single-config generators (Ninja, NMake)
  if(NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR
      "\n❌ No build type specified.\n"
      "This project requires Release builds.\n"
      "Use:\n"
      "  cmake -DCMAKE_BUILD_TYPE=Release\n")
  elseif(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR
      "\n❌ Build type '${CMAKE_BUILD_TYPE}' is forbidden.\n"
      "Only Release builds are supported.\n")
  endif()

  message(STATUS "Build policy: single-config generator, Release-only enforced")
endif()

# Standards baseline
if (APPLE)
    message(STATUS "Using C++26 on macOS (clang++)")
    set(CMAKE_CXX_STANDARD 26)
elseif (MSVC)
    message(STATUS "Using C++26 on MSVC (set 23 to set /std:c++latest)")
    set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Python for dependency management
find_package(Python3 REQUIRED)
message(STATUS "Found Python3: ${Python3_EXECUTABLE} (version ${Python3_VERSION})")

# Map CMake platform to deps.lock.json keys
if(WIN32)
    set(LLM_PLATFORM "win11-x64")
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
        set(LLM_PLATFORM "macos-x64")
    elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(LLM_PLATFORM "macos-arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(LLM_PLATFORM "macos-arm64")
    else()
        set(LLM_PLATFORM "macos-x64")
    endif()
endif()

# Run the dependency orchestrator
message(STATUS "Running dependency manager for ${LLM_PLATFORM}...")
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/manage_deps.py"
            --platform ${LLM_PLATFORM}
            --root "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE DEPS_RESULT
)

if(NOT DEPS_RESULT EQUAL 0)
    message(FATAL_ERROR "Dependency management failed with exit code ${DEPS_RESULT}")
endif()

# Define the library
add_library(llm STATIC
    src/instance.cpp
    src/results.cpp
    src/curl.cpp
    src/http.cpp
    src/orchestrator.cpp
    src/policy.cpp
    src/endpoints/anthropic_messages.cpp
    src/endpoints/gemini_generate_content.cpp
    src/endpoints/openai_responses.cpp
    src/protocols/serialize/anthropic_messages.cpp
    src/protocols/serialize/gemini_generate_content.cpp
    src/protocols/serialize/openai_responses.cpp
    src/protocols/deserialize/anthropic_messages.cpp
    src/protocols/deserialize/gemini_generate_content.cpp
    src/protocols/deserialize/openai_responses.cpp
)

# Add include directories (Unified Sysroot)
target_include_directories(llm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/include>
)

# Link dependencies
if(WIN32)
    target_link_libraries(llm PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libcurl.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/nghttp2.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/zlibstatic.lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/simdjson.lib"
        # Winsock for Curl
        ws2_32
        crypt32
        wldap32
        normaliz
        secur32
        iphlpapi
    )
elseif(APPLE)
    target_link_libraries(llm PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libcurl.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libnghttp2.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libz.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libsimdjson.a"
        "-framework Security"
        "-framework CoreFoundation"
        "-framework SystemConfiguration"
    )
else()
    target_link_libraries(llm PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libcurl.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libnghttp2.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libz.a"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/lib/libsimdjson.a"
    )
endif()


# Basic compiler warnings configuration
if(MSVC)
    target_compile_options(llm PRIVATE /W4)
    target_compile_definitions(llm PUBLIC NGHTTP2_STATICLIB CURL_STATICLIB NOMINMAX)
else()
    target_compile_options(llm PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Production Tests
add_executable(test_http tests/unit/test_http.cpp)
target_link_libraries(test_http PRIVATE llm)

add_executable(test_curl tests/unit/test_curl.cpp)
target_link_libraries(test_curl PRIVATE llm)

add_executable(test_policy tests/unit/test_policy.cpp)
target_link_libraries(test_policy PRIVATE llm)

add_executable(test_model_group tests/unit/test_model_group.cpp)
target_link_libraries(test_model_group PRIVATE llm)

add_executable(test_orchestrator tests/unit/test_orchestrator.cpp)
target_link_libraries(test_orchestrator PRIVATE llm)

add_executable(test_results tests/unit/test_results.cpp)
target_link_libraries(test_results PRIVATE llm)

# CTest registration
enable_testing()
add_test(NAME test_http          COMMAND test_http)
add_test(NAME test_curl          COMMAND test_curl)
add_test(NAME test_policy        COMMAND test_policy)
add_test(NAME test_model_group   COMMAND test_model_group)
add_test(NAME test_orchestrator  COMMAND test_orchestrator)
add_test(NAME test_results       COMMAND test_results)

# Label curl tests as integration (require mock server)
set_tests_properties(test_curl PROPERTIES LABELS "integration")
