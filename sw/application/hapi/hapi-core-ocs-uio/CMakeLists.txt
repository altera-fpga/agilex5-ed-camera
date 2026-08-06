# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
project(HapiCoreOCS)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files "")
set(source_files "")

# Header files
set(header_files ${header_files} "include/HapiOCS.h")
set(header_files ${header_files} "source/IpCapabilityNames.h")
set(header_files ${header_files} "source/OcsUioHapiCore.h")
set(header_files ${header_files} "source/OcsUioHapiItem.h")
set(header_files ${header_files} "source/OcsUioHapiItemTunnel.h")

# Source files
set(source_files ${source_files} "source/IpCapabilityNames.cpp")
set(source_files ${source_files} "source/OcsUioHapiCore.cpp")
set(source_files ${source_files} "source/OcsUioHapiItem.cpp")
set(source_files ${source_files} "source/OcsUioHapiItemTunnel.cpp")

set(all_files ${header_files} ${source_files})

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

# Your application must add the HapiCoreCommon submodule (project name HapiCore)
target_link_libraries(${PROJECT_NAME} HapiCore)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/source")

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "HAPI")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
