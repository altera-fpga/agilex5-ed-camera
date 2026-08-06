# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************

cmake_minimum_required(VERSION 3.16)
project(SwApiSwitchUiState
    VERSION 0.0.1
    DESCRIPTION "C++ High-level library for the VVP (Video and Vision Processing) Video Switching IP core"
    HOMEPAGE_URL "https://github.com/intel-innersource/applications.fpga.reference-designs.libraries.sw-api.switch"
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(${PROJECT_NAME}_IS_TOP_LEVEL ON)
endif()

# Header files
file(GLOB c_header_files *.h)
file(GLOB cpp_header_files *.hpp)
set(header_files ${c_header_files} ${cpp_header_files})
# Source files
file(GLOB cpp_source_files *.cpp)
set(source_files ${cpp_source_files})

set(all_files ${header_files} ${source_files})

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiIntelVvpSwitch)
target_link_libraries(${PROJECT_NAME} SwApiUtils)
target_link_libraries(${PROJECT_NAME} SwApiSwitch)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# CPack: by default, only disable CPack if we're running tests.
option(DISABLE_CPACK "Disable cpack" ${ENABLE_TESTING})
if(NOT DISABLE_CPACK)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
endif()
