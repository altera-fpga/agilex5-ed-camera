# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(SwApiIspCommon)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files "")
set(source_files "")

# Library headers
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/IspCommon.h")
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/ICamera.h")
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/VvpCoreBase.h")

# Library sources
set(source_files ${source_files} "${CMAKE_CURRENT_SOURCE_DIR}/IspCommon.cpp")
set(source_files ${source_files} "${CMAKE_CURRENT_SOURCE_DIR}/VvpCoreBase.cpp")

set(all_files ${header_files} ${source_files})
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} ApplicationToolkit)
target_link_libraries(${PROJECT_NAME} SwApiVideoCommon)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

add_library(ICamera INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ICamera.h")
target_include_directories(ICamera INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)