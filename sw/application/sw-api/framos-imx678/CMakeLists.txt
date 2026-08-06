# Copyright (C) Altera Corporation
#
# SPDX-License-Identifier: GPL-2.0-only */

project(SwApiFramosIMX678)

set(header_files "")
set(source_files "")


#region Finding source and header files
file(GLOB headerHFiles *.h)
file(GLOB headerHppFiles *.hpp)
set(header_files ${headerHFiles} ${headerHppFiles})

file(GLOB source_files *.cpp)

set(all_files ${header_files} ${source_files})
#endregion

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} SwApiI2C)
target_link_libraries(${PROJECT_NAME} SwApiIspCommon)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
