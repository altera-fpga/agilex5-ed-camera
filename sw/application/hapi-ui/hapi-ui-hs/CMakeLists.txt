# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiUiHs)

set(header_files "")
set(source_files "")

set(header_files ${header_files} "HsControls.h")
set(header_files ${header_files} "HsPreview.h")

set(source_files ${source_files} "HsControls.cpp")
set(source_files ${source_files} "HsPreview.cpp")

set(all_files ${header_files} ${source_files})

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

target_link_libraries(${PROJECT_NAME} SwApiHs)
target_link_libraries(${PROJECT_NAME} ROIHelper)
target_link_libraries(${PROJECT_NAME} ApplicationToolkit)
target_link_libraries(${PROJECT_NAME} SwApiIspCommon)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
