cmake_minimum_required(VERSION 3.20)
project(grpc_learning_examples LANGUAGES CXX)

# Keep find_package behavior predictable when *_ROOT env vars are present.
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

# This demo uses C++17 across all targets.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core dependencies for gRPC C++ examples.
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Source proto and generated output directory.
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/demo.proto")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_DIR}")

# grpc_cpp_plugin is required for *.grpc.pb.cc code generation.
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
  message(FATAL_ERROR "grpc_cpp_plugin not found in PATH")
endif()

# openssl is used by the optional TLS/mTLS certificate generation target.
find_program(OPENSSL_EXECUTABLE openssl)
if(NOT OPENSSL_EXECUTABLE)
  message(WARNING "openssl not found; TLS cert generation target will fail until openssl is installed")
endif()

set(CERT_DIR "${CMAKE_CURRENT_BINARY_DIR}/certs")

set(PROTO_SRCS "${GENERATED_DIR}/demo.pb.cc")
set(PROTO_HDRS "${GENERATED_DIR}/demo.pb.h")
set(GRPC_SRCS "${GENERATED_DIR}/demo.grpc.pb.cc")
set(GRPC_HDRS "${GENERATED_DIR}/demo.grpc.pb.h")

# Generate protobuf and gRPC stubs from demo.proto.
add_custom_command(
  OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
  COMMAND protobuf::protoc
  ARGS
    --proto_path "${CMAKE_CURRENT_SOURCE_DIR}/proto"
    --cpp_out "${GENERATED_DIR}"
    --grpc_out "${GENERATED_DIR}"
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
    "${PROTO_FILE}"
  DEPENDS "${PROTO_FILE}"
  VERBATIM
)

# Teaching target: create local CA/server/client certs for TLS and mTLS demos.
add_custom_target(generate_certs
  COMMAND ${CMAKE_COMMAND}
    -DOPENSSL_EXECUTABLE=${OPENSSL_EXECUTABLE}
    -DOUTPUT_DIR=${CERT_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_certs.cmake
  BYPRODUCTS
    "${CERT_DIR}/ca.crt"
    "${CERT_DIR}/server.crt"
    "${CERT_DIR}/server.key"
    "${CERT_DIR}/client.crt"
    "${CERT_DIR}/client.key"
  COMMENT "Generating TLS/mTLS demo certificates"
)

# Shared generated proto/grpc sources.
add_library(demo_proto
  "${PROTO_SRCS}"
  "${GRPC_SRCS}"
)

target_include_directories(demo_proto PUBLIC "${GENERATED_DIR}")
target_link_libraries(demo_proto PUBLIC protobuf::libprotobuf gRPC::grpc++ Threads::Threads)

# Insecure demos.
add_executable(sync_server src/sync_server.cc)
add_executable(sync_client src/sync_client.cc)
add_executable(async_cq_server src/async_cq_server.cc)
add_executable(async_cq_client src/async_cq_client.cc)
add_executable(async_callback_server src/async_callback_server.cc)
add_executable(async_callback_client src/async_callback_client.cc)

# TLS/mTLS demos.
add_executable(tls_server src/tls_server.cc)
add_executable(tls_client src/tls_client.cc)

target_link_libraries(sync_server PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(sync_client PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(async_cq_server PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(async_cq_client PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(async_callback_server PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(async_callback_client PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(tls_server PRIVATE demo_proto gRPC::grpc++ Threads::Threads)
target_link_libraries(tls_client PRIVATE demo_proto gRPC::grpc++ Threads::Threads)

# Ensure certificates exist before building secure examples.
add_dependencies(tls_server generate_certs)
add_dependencies(tls_client generate_certs)
