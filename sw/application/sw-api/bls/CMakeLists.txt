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
project(SwApiBls
    VERSION 0.0.1
    DESCRIPTION "C++ High-level library for the VVP (Video and Vision Processing) Colour Space Converter"
    HOMEPAGE_URL "https://github.com/intel-innersource/applications.fpga.reference-designs.libraries.sw-api.bls"
    LANGUAGES CXX C)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(${PROJECT_NAME}_IS_TOP_LEVEL ON)
endif()

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

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
#endregion

#region Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiIntelVvpBls)
target_link_libraries(${PROJECT_NAME} SwApiUtils)
target_link_libraries(${PROJECT_NAME} SwApiIspCommon)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#endregion

# compile_commands.json file
if (${${PROJECT_NAME}_IS_TOP_LEVEL})
    include(cmake/CopyCompileCommands.cmake)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)

# Testing
if (${${PROJECT_NAME}_IS_TOP_LEVEL})
    # Only produce tests if we're the top level project.
    option(ENABLE_TESTING "Enable tests" OFF)
    if(ENABLE_TESTING)
        add_subdirectory(test)
        message("Enabled tests.")
    endif()
endif()
