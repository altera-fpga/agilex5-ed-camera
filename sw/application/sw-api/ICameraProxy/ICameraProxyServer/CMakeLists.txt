# Copyright (C) Altera Corporation
#
# SPDX-License-Identifier: GPL-2.0-only

cmake_minimum_required(VERSION 3.16)

project(ICameraProxyServer)
    
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(fuse3 fuse3)

set(header_files "")
set(source_files "")

# Library headers
list(APPEND header_files ICameraProxyServer.h)

# Library sources
list(APPEND source_files main.cpp)
list(APPEND source_files ICameraProxyServer.cpp)

set(all_files ${header_files} ${source_files})

if(NOT fuse3_FOUND)
    message(WARNING "Cannot build ${PROJECT_NAME} without fuse3 support in sdk")
else()
    add_executable(${PROJECT_NAME} ${all_files})

    target_link_libraries(${PROJECT_NAME} ICamera ICameraProxyCommon fuse3)
    target_link_libraries(${PROJECT_NAME} ${CAMERA_LIBS})

    # Include directories
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # Compile options
    target_compile_options(${PROJECT_NAME} PRIVATE ${common_compile_options})

    # Compile definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${CAMERA_DEFINITIONS})

    install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_NAME} COMPONENT deploy)
    install(FILES LICENSE DESTINATION ${PROJECT_NAME} COMPONENT deploy)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
