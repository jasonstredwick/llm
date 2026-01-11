cmake_minimum_required(VERSION 3.25)

project(llm
    VERSION 0.1.0
    DESCRIPTION "LLM access management for C/C++"
    LANGUAGES C CXX
)

# Standards baseline
set(CMAKE_CXX_STANDARD 23)
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
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
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
    src/http_manager.cpp
    src/admission_controller.cpp
    src/providers/openai/openai_client.cpp
    src/operation_group.cpp
)

# Add include directories (Unified Sysroot)
target_include_directories(llm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/include>
)

# Link dependencies
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

# Basic compiler warnings configuration
if(MSVC)
    target_compile_options(llm PRIVATE /W4)
    target_compile_definitions(llm PUBLIC NGHTTP2_STATICLIB CURL_STATICLIB NOMINMAX)
else()
    target_compile_options(llm PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Verification Test
add_executable(test_includes tests/test_includes.cpp)
target_link_libraries(test_includes PRIVATE llm)

# ODR Verification Test (Multi-Unit Linking)
add_executable(test_odr tests/test_odr_1.cpp tests/test_odr_2.cpp)
target_link_libraries(test_odr PRIVATE llm)

install(TARGETS test_includes test_odr RUNTIME DESTINATION bin)

# HTTP Manager Verification
add_executable(test_http_simple tests/test_http_simple.cpp)
target_link_libraries(test_http_simple PRIVATE llm)

add_executable(test_transport tests/test_transport.cpp)
target_link_libraries(test_transport PRIVATE llm)

add_executable(test_admission tests/test_admission.cpp)
target_link_libraries(test_admission PRIVATE llm)

add_executable(test_openai tests/test_openai.cpp)
target_link_libraries(test_openai PRIVATE llm)

add_executable(test_operations tests/test_operations.cpp)
target_link_libraries(test_operations PRIVATE llm)
