cmake_minimum_required(VERSION 3.24)
project(MiniExchange VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(FetchContent)
option(MINI_EXCHANGE_BUILD_BENCHMARKS "Build order processing benchmarks" ON)
option(MINI_EXCHANGE_BUILD_DOCS "Enable the Doxygen docs target" OFF)

function(mini_exchange_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_subdirectory(src)
add_subdirectory(app)

if(BUILD_TESTING)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
        URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()

if(MINI_EXCHANGE_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
        URL_HASH SHA256=b334658edd35efcf06a99d9be21e4e93e092bd5f95074c1673d5c8705d95c104
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(benchmark)
    add_subdirectory(benchmarks)
endif()

if(MINI_EXCHANGE_BUILD_DOCS)
    # We already maintain Doxyfile; FindDoxygen's template generation is unused.
    find_program(DOXYGEN_EXECUTABLE NAMES doxygen REQUIRED)
    # Relative arguments also support Doxygen on Windows in a Cyrillic workspace.
    file(RELATIVE_PATH mini_exchange_docs_output "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}/docs")
    add_custom_target(docs
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/docs"
        COMMAND ${CMAKE_COMMAND} -E env "MINI_EXCHANGE_DOCS_DIR=${mini_exchange_docs_output}"
            ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Generating Doxygen HTML in ${PROJECT_BINARY_DIR}/docs/html"
        VERBATIM
    )
endif()
