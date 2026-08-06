# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiUiCsc)

set(all_files 
    CscControls.h 
    CscControls.cpp)

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

target_link_libraries(${PROJECT_NAME} SwApiCsc)
target_link_libraries(${PROJECT_NAME} ApplicationToolkit)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
