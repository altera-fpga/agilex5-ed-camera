# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.16)

project(ICameraProxyClient)
    
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files "")
set(source_files "")

# Library headers
list(APPEND header_files ICameraProxyClient.h)

# Library sources
list(APPEND source_files ICameraProxyClient.cpp)

set(all_files ${header_files} ${source_files})
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} ICamera ICameraProxyCommon)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)

foreach (CAMERA_LIB ${CAMERA_LIBS})
    string(REPLACE "SwApi" "" CAMERA_FILENAME ${CAMERA_LIB})

    set(TARGET_NAME ${CAMERA_LIB}Proxy)
    set(camera_header_files "")
    set(camera_source_files "")

    # Library headers
    list(APPEND camera_header_files ${CAMERA_FILENAME}.h)

    # Library sources
    list(APPEND camera_source_files ${CAMERA_FILENAME}.cpp)

    set(camera_all_files ${camera_header_files} ${camera_source_files})
    add_library(${TARGET_NAME} STATIC ${camera_all_files})
    set(all_files ${all_files} ${camera_all_files})

    target_link_libraries(${TARGET_NAME} ${PROJECT_NAME})

    # Include directories
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # Compile options
    target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

endforeach()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
