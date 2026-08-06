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
if(${CMAKE_VERSION} VERSION_GREATER "3.24.0")
	cmake_policy(SET CMP0135 NEW) 
endif()

project(AiResultsRenderer
    VERSION 0.0.1
    LANGUAGES CXX C)

set(TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# Header files
set(headerFiles )
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/AiResultsRenderer.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/IAiResultsRenderer.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/lv_labelled_box_private.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/lv_labelled_box.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/lv_skeleton_private.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/lv_skeleton.h)
# Source files
set(sourceFiles )
list(APPEND sourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/AiResultsRenderer.cpp)
list(APPEND sourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/lv_labelled_box.cpp)
list(APPEND sourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/lv_skeleton.cpp)
 
source_group(include FILES ${headerFiles})
source_group(src FILES ${sourceFiles})

set(all_files ${headerFiles} ${sourceFiles})

#region Library target
add_library(${TARGET_NAME} STATIC ${all_files})

#endregion

#region Compile options
target_compile_options(${TARGET_NAME} PRIVATE "${common_compile_options}")
target_compile_options(${TARGET_NAME} PRIVATE "-Wno-pedantic")
target_compile_definitions(${TARGET_NAME} PRIVATE "${common_compile_definitions}")
if(SUPPORT_DMA)
target_compile_definitions(${TARGET_NAME} PRIVATE SUPPORT_DMA)
endif()

target_link_libraries(${TARGET_NAME} HapiIntelCoreDLA)

target_link_libraries(${TARGET_NAME} HapiIntelVvpVfw)
target_link_libraries(${TARGET_NAME} HapiIntelVvpVfr)
target_link_libraries(${TARGET_NAME} HapiIntelVvpScaler)
target_link_libraries(${TARGET_NAME} HapiIntelVvpSwitch)
target_link_libraries(${TARGET_NAME} SwApiUtils)
target_link_libraries(${TARGET_NAME} SwApiIspCommon)
target_link_libraries(${TARGET_NAME} CoreDLAProcessor)
target_link_libraries(${TARGET_NAME} SwApiVvpIspFrameQueue)

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

#endregion

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
