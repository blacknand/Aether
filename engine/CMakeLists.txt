cmake_minimum_required(VERSION 4.0.1)
project(Engine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)
FetchContent_MakeAvailable(benchmark)

add_executable(
    AetherEngine
    src/main.cpp
    src/core/OrderBook.cpp
)

target_include_directories(
    AetherEngine
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# *** GoogleTest Setup *** #
enable_testing()
include(GoogleTest)

add_executable(
    OrderBookTests
    tests/OrderBookTests.cpp
    src/core/OrderBook.cpp
)

target_include_directories(
    OrderBookTests
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    OrderBookTests
    GTest::gtest_main
)

gtest_discover_tests(OrderBookTests)

# *** GoogleBenchmark Setup *** #
add_executable(
    Benchmarks
    benchmarks/MatchingBenchmarks.cpp
    src/core/OrderBook.cpp
)

target_link_libraries(
    Benchmarks
    benchmark::benchmark
)

target_include_directories(
    Benchmarks
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)