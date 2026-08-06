# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************

# This tool should be targetted for the build system, so typically
# Linux_x84_64 or Windows, and not cross-compiled (eg don't use
# arm-poky-linux-gnueabi toolchain)

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

project(FilePacker)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} FilePacker.cpp)

if (MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

