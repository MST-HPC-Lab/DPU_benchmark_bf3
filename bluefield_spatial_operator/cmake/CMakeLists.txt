# FILE MODIFIED BY Nathan Tibbetts, July 2024

# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.5.1)

project(BluefieldSpatialOperator C CXX)

include("common.cmake") 

# Set Make's output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../bin")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../lib") # Uncomment if building a library?
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../lib")

# Proto file
get_filename_component(bf_proto "../protos/bluefield_spatial.proto" ABSOLUTE) # Tells it to resolve this file to an absolute path
get_filename_component(bf_proto_path "${bf_proto}" PATH)

# Generated sources
# message(STATUS "CURRENT DIR: ${CMAKE_CURRENT_BINARY_DIR}")
set(bf_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_spatial.pb.cc")
set(bf_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_spatial.pb.h")
set(bf_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_spatial.grpc.pb.cc")
set(bf_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_spatial.grpc.pb.h")
add_custom_command(
      OUTPUT "${bf_proto_srcs}" "${bf_proto_hdrs}" "${bf_grpc_srcs}" "${bf_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${bf_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${bf_proto}"
      DEPENDS "${bf_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# bf_grpc_proto
add_library(bf_grpc_proto
  ${bf_grpc_srcs}
  ${bf_grpc_hdrs}
  ${bf_proto_srcs}
  ${bf_proto_hdrs})
target_link_libraries(bf_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets spatial_(client|server)
foreach(_target
  spatial_client spatial_server )
  add_executable(${_target} "../experiments/${_target}.cc")
  target_link_libraries(${_target}
    bf_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()
