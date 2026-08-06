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
project(SwApiCrs
    VERSION 0.0.1
    DESCRIPTION "C++ High-level library for the VVP (Video and Vision Processing) Chroma Resampling Core"
    HOMEPAGE_URL "https://github.com/intel-innersource/applications.fpga.reference-designs.libraries.sw-api.crs"
    LANGUAGES CXX C)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(${PROJECT_NAME}_IS_TOP_LEVEL ON)
endif()

#region Finding source and header files
file(GLOB headerHFiles *.h)
file(GLOB headerHppFiles *.hpp)
set(headerFiles ${headerHFiles} ${headerHppFiles})

file(GLOB sourceFiles *.cpp)

set(all_files ${headerFiles} ${sourceFiles})
#endregion

#region Library target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
#endregion

#region Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiIntelVvpCrs)
target_link_libraries(${PROJECT_NAME} SwApiUtils)

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
