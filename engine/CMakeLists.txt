add_library(
    EngineCore
    src/core/OrderBook.cpp
)

target_include_directories(
    EngineCore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

enable_testing()
include(GoogleTest)

add_executable(
    OrderBookTests
    tests/OrderBookTests.cpp
)

target_link_libraries(
    OrderBookTests
    PRIVATE
    EngineCore
    GTest::gtest_main
)

gtest_discover_tests(OrderBookTests)

add_executable(
    Benchmarks
    benchmarks/MatchingBenchmarks.cpp
)

target_link_libraries(
    Benchmarks
    PRIVATE
    EngineCore
    benchmark::benchmark
)

add_subdirectory(src/network)