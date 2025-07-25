cmake_minimum_required(VERSION 3.16)
project(Engine)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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