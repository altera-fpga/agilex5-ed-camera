# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiUiVvpIspProfileMasterRegionSelector)

set(header_files "")
set(source_files "")

#region Finding source and header files
# Header files
file(GLOB c_header_files *.h)
file(GLOB cpp_header_files *.hpp)
set(header_files ${c_header_files} ${cpp_header_files})
# Source files
file(GLOB cpp_source_files *.cpp)
set(source_files ${cpp_source_files})

set(all_files ${header_files} ${source_files})
#endregion

#region Library target
add_library(${PROJECT_NAME} STATIC ${all_files})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

target_link_libraries(${PROJECT_NAME} HapiUiObservable)
target_link_libraries(${PROJECT_NAME} ApplicationToolkit)

target_link_libraries(${PROJECT_NAME} SwApiTmo)
target_link_libraries(${PROJECT_NAME} SwApiWbs)
target_link_libraries(${PROJECT_NAME} SwApiHs)
target_link_libraries(${PROJECT_NAME} SwApiBls)
target_link_libraries(${PROJECT_NAME} SwApiVvpIspWhiteBalance)
target_link_libraries(${PROJECT_NAME} SwApiVvpIspCalibrationProfile)
target_link_libraries(${PROJECT_NAME} SwApiIspCommon)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
