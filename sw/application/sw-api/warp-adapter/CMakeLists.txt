# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************

project(SwApiWarpAdapter)

set(header_files "")
set(source_files "")

# Header files
set(header_files ${header_files} "WarpAdapter.h")
set(header_files ${header_files} "WarpAdapterRawRgbImage.h")
set(header_files ${header_files} "OWarpFile.h")

# Source files
set(source_files ${source_files} "WarpAdapter.cpp")
set(source_files ${source_files} "OWarpFile.cpp")

set(all_files ${header_files} ${source_files})

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiIntelVvpWarp)
target_link_libraries(${PROJECT_NAME} HapiIntelVvpWarpData)
target_link_libraries(${PROJECT_NAME} HapiIntelVvpWarpMesh)
target_link_libraries(${PROJECT_NAME} SwApiUtils)
target_link_libraries(${PROJECT_NAME} HapiCoreOCS)

target_link_libraries(${PROJECT_NAME} ApplicationToolkit)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
