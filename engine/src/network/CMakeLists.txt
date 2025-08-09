cmake_minimum_required(VERSION 3.14)
project(AetherServer LANGUAGES CXX)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(PROTO_FILES ${CMAKE_SOURCE_DIR}/protos/order.proto)
set(PROTO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/protos)

message(STATUS "DEBUG: The path to the proto file is set to: ${PROTO_FILES}")

add_executable(
    AetherServer
    AetherServer.cpp
)

protobuf_generate(
    TARGET AetherServer
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
    IMPORT_DIRS ${PROTO_INCLUDE_DIRS}
)

protobuf_generate(
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    TARGET AetherServer
    PROTOS ${PROTO_FILES}
    IMPORT_DIRS ${PROTO_INCLUDE_DIRS}
)

target_include_directories(
    AetherServer
    PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(
    AetherServer
    PRIVATE
    EngineCore
    gRPC::grpc++
    protobuf::libprotobuf
)
