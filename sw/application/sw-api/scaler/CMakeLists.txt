# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************


project(SwApiScaler
    VERSION 0.0.1
    DESCRIPTION "C++ High-level library for the VVP (Video and Vision Processing) Scaler Core"
    HOMEPAGE_URL "https://github.com/altera-innersource/applications.fpga.reference-designs.libraries.sw-api.scaler"
    LANGUAGES CXX)

set(header_files "")
set(source_files "")

# Header files
set(header_files ${header_files} "Scaler.h")
set(header_files ${header_files} "scaler_coeffs.h")


# Source files
set(source_files ${source_files} "Scaler.cpp")
set(source_files ${source_files} "scaler_coeffs.cpp")

set(all_files ${header_files} ${source_files})


#region Library target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
#endregion

#region Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiIntelVvpScaler)
#target_link_libraries(${PROJECT_NAME} SwApiUtils)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#endregion

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)

