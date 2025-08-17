# protos/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# find_package(Protobuf REQUIRED)
# find_package(gRPC REQUIRED)

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTO_FILES
    ${PROTO_DIR}/order_management.proto
    ${PROTO_DIR}/market_data.proto
)

# Resolve protoc (CONFIG target) and include dirs for well-known types
get_target_property(_PROTOBUF_INCLUDES protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
if(NOT _PROTOBUF_INCLUDES)
  set(_PROTOBUF_INCLUDES "")
endif()

# Resolve grpc_cpp_plugin: use target if exported, else find the program
set(_GRPC_CPP_PLUGIN_EXEC)
if(TARGET gRPC::grpc_cpp_plugin)
  set(_GRPC_CPP_PLUGIN_EXEC $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
  # Search common prefixes & PATH
  set(_hint_bins "")
  foreach(p ${CMAKE_PREFIX_PATH})
    list(APPEND _hint_bins "${p}/bin")
  endforeach()
  find_program(_GRPC_CPP_PLUGIN_EXEC
    NAMES grpc_cpp_plugin
    HINTS ${_hint_bins}
    PATHS ${_hint_bins}
    DOC "Path to grpc_cpp_plugin"
  )
  if(NOT _GRPC_CPP_PLUGIN_EXEC)
    message(FATAL_ERROR "grpc_cpp_plugin not found. Add its bin dir to CMAKE_PREFIX_PATH or PATH.")
  endif()
endif()

# Helper to generate C++ gRPC stubs
function(generate_grpc_cpp OUT_SRCS OUT_HDRS)
  set(gen_srcs)
  set(gen_hdrs)
  foreach(proto ${ARGN})
    get_filename_component(fname ${proto} NAME_WE)
    set(pb_cc   ${CMAKE_CURRENT_BINARY_DIR}/${fname}.pb.cc)
    set(pb_h    ${CMAKE_CURRENT_BINARY_DIR}/${fname}.pb.h)
    set(grpc_cc ${CMAKE_CURRENT_BINARY_DIR}/${fname}.grpc.pb.cc)
    set(grpc_h  ${CMAKE_CURRENT_BINARY_DIR}/${fname}.grpc.pb.h)

    add_custom_command(
      OUTPUT ${pb_cc} ${pb_h} ${grpc_cc} ${grpc_h}
      COMMAND $<TARGET_FILE:protobuf::protoc>
              --proto_path=${PROTO_DIR}
              --proto_path=${_PROTOBUF_INCLUDES}
              --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
              --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
              --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXEC}
              ${proto}
      DEPENDS ${proto}
      COMMENT "Generating C++ & gRPC sources for ${fname}.proto"
      VERBATIM
    )

    list(APPEND gen_srcs ${pb_cc} ${grpc_cc})
    list(APPEND gen_hdrs ${pb_h} ${grpc_h})
  endforeach()
  set(${OUT_SRCS} "${gen_srcs}" PARENT_SCOPE)
  set(${OUT_HDRS} "${gen_hdrs}" PARENT_SCOPE)
endfunction()

generate_grpc_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(aether_protos STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(aether_protos PUBLIC protobuf::libprotobuf gRPC::grpc++)
target_include_directories(aether_protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})