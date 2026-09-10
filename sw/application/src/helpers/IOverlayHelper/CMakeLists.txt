# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(IOverlayHelper)

set(header_files "")
set(source_files "")

set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/IOverlayHelper.h")

set(source_files ${source_files} "${CMAKE_CURRENT_SOURCE_DIR}/IOverlayHelper.cpp")

set(all_files ${header_files} ${source_files})

if(NOT "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")

    # We do not include lvgl in cpack builds
    
    include(FetchContent)
    set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
    set(LV_CONF_BUILD_DISABLE_DEMOS ON)

    FetchContent_Declare(
        lvgl
        URL https://github.com/lvgl/lvgl/archive/refs/tags/v9.2.1.tar.gz
        PATCH_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/patches/lvglPatchCommand.sh <SOURCE_DIR>
    )
    FetchContent_MakeAvailable(lvgl)
endif()

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

file(GLOB patches "${CMAKE_CURRENT_SOURCE_DIR}/patches/*")
list(APPEND all_files ${patches})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

target_link_libraries(${PROJECT_NAME} PUBLIC VvpIspDemoInterface)
target_link_libraries(${PROJECT_NAME} PUBLIC lvgl)
target_link_libraries(${PROJECT_NAME} PUBLIC drmHelper)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
