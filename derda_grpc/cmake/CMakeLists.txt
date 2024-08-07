# MODIFIED BY Nathan Tibbetts, July 2024
# NOTE: To add files to the project, you will need to add them to the foreach
#   target list at the bottom of this file.
################################################################################

# ORIGINAL LICENSE NOTICE:

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

project(BLUEFIELD_GRPC C CXX)



# FROM common.cmake -------------------------------------------------------------

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set (CMAKE_CXX_STANDARD 17)
endif()

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

find_package(Threads REQUIRED)

# if(GRPC_AS_SUBMODULE)
#   # One way to build a projects that uses gRPC is to just include the
#   # entire gRPC project tree via "add_subdirectory".
#   # This approach is very simple to use, but the are some potential
#   # disadvantages:
#   # * it includes gRPC's CMakeLists.txt directly into your build script
#   #   without and that can make gRPC's internal setting interfere with your
#   #   own build.
#   # * depending on what's installed on your system, the contents of submodules
#   #   in gRPC's third_party/* might need to be available (and there might be
#   #   additional prerequisites required to build them). Consider using
#   #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
#   #
#   # A more robust approach to add dependency on gRPC is using
#   # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).

#   # Include the gRPC's cmake build (normally grpc source code would live
#   # in a git submodule called "third_party/grpc", but this example lives in
#   # the same repository as gRPC sources, so we just look a few directories up)
#   add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
#   message(STATUS "Using gRPC via add_subdirectory.")

#   # After using add_subdirectory, we can now use the grpc targets directly from
#   # this build.
#   set(_PROTOBUF_LIBPROTOBUF libprotobuf)
#   set(_REFLECTION grpc++_reflection)
#   if(CMAKE_CROSSCOMPILING)
#     find_program(_PROTOBUF_PROTOC protoc)
#   else()
#     set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
#   endif()
#   set(_GRPC_GRPCPP grpc++)
#   if(CMAKE_CROSSCOMPILING)
#     find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
#   else()
#     set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
#   endif()
# elseif(GRPC_FETCHCONTENT)
#   # Another way is to use CMake's FetchContent module to clone gRPC at
#   # configure time. This makes gRPC's source code available to your project,
#   # similar to a git submodule.
#   message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
#   include(FetchContent)
#   FetchContent_Declare(
#     grpc
#     GIT_REPOSITORY https://github.com/grpc/grpc.git
#     # when using gRPC, you will actually set this to an existing tag, such as
#     # v1.25.0, v1.26.0 etc..
#     # For the purpose of testing, we override the tag used to the commit
#     # that's currently under test.
#     GIT_TAG        vGRPC_TAG_VERSION_OF_YOUR_CHOICE)
#   FetchContent_MakeAvailable(grpc)

#   # Since FetchContent uses add_subdirectory under the hood, we can use
#   # the grpc targets directly from this build.
#   set(_PROTOBUF_LIBPROTOBUF libprotobuf)
#   set(_REFLECTION grpc++_reflection)
#   set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
#   set(_GRPC_GRPCPP grpc++)
#   if(CMAKE_CROSSCOMPILING)
#     find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
#   else()
#     set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
#   endif()
# else()
  # This branch assumes that gRPC and all its dependencies are already installed
  # on this system, so they can be located by find_package().

  # Find Protobuf installation
  # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
  set(protobuf_MODULE_COMPATIBLE TRUE)
  find_package(Protobuf CONFIG REQUIRED)
  message(STATUS "Using protobuf ${Protobuf_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_REFLECTION gRPC::grpc++_reflection)
  if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC protoc)
  else()
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  endif()

  # Find gRPC installation
  # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_GRPC_GRPCPP gRPC::grpc++)
  if(CMAKE_CROSSCOMPILING)
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
  else()
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  endif()
# endif()

# END OF common.cmake CONTENTS ------------------------------------------------------------
# CONTINUING ORIGINAL FILE, with modifications


# Set Make's output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../bin")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../lib") # Uncomment if building a library?
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../lib")

# Proto file
get_filename_component(bf_proto "../protos/bluefield_grpc.proto" ABSOLUTE)
get_filename_component(bf_proto_path "${bf_proto}" PATH)

# Generated sources
set(bf_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_grpc.pb.cc")
set(bf_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bluefield_grpc.pb.h")
set(bf_grpc_srcs  "${CMAKE_CURRENT_BINARY_DIR}/bluefield_grpc.grpc.pb.cc")
set(bf_grpc_hdrs  "${CMAKE_CURRENT_BINARY_DIR}/bluefield_grpc.grpc.pb.h")
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


# ATTENTION: Here's where you need to list the files you want compiled!
# Targets files
foreach(_target
  exp1_blocking_rtree
  exp1_non_blocking_rtree
  exp2_multiple_query_multiple_thread_floating_point_client
  exp2_multiple_query_multiple_thread_floating_point_dpu
  exp2_multiple_query_multiple_thread_floating_point_server
  exp3_multiple_query_multiple_thread_client
  exp3_multiple_query_multiple_thread_server
  exp4_multiple_query_client
  exp4_multiple_query_dpu
  exp4_multiple_query_server
  exp5_multiple_thread_client
  exp5_multiple_thread_dpu
  exp5_multiple_thread_server
  exp6_load_balancing_client
  exp6_load_balancing_server
  # geosbm_client # Has a BUG
  # geosbm_server
  greeter_client
  greeter_server
  lb_mt_client
  lb_mt_server
  lb_mt_tfr_client
  lb_mt_tfr_server
  # z_lb_mt_fr_sc
  # z_lb_mt_fr_server # Has a BUG
  zzz_fpc_lb_mt_mfr_sc
  )
  add_executable(${_target} "../experiments/${_target}.cc")
  target_link_libraries(${_target}
    bf_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    geos_c) # NATHAN ADDED to make GEOS library usage work. Apparently if GEOS is installed correctly it knows where to find it based on the name geos_c
endforeach()
