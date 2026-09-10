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

project(SwApiVvpIspFrameQueue
    VERSION 0.0.1
    LANGUAGES CXX C)

set(TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(${TARGET_NAME}_IS_TOP_LEVEL ON)
endif()

# Header files
set(headerFiles )
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/FrameQueue.h)
list(APPEND headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/inc/FrameQueueConfig.h)
# Source files
set(sourceFiles )
list(APPEND sourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameQueue.cpp)
 
source_group(include FILES ${headerFiles})
source_group(src FILES ${sourceFiles})

set(all_files ${headerFiles} ${sourceFiles})

#region Library target
add_library(${TARGET_NAME} STATIC ${all_files})

add_library(${TARGET_NAME}Config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inc/FrameQueueConfig.h)

# Copy asstes
set(assets )

set(assets_out )
foreach(asset ${assets})
	string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${INSTALL_PREFIX} asset_out ${asset})
	get_filename_component(asset_out_dir ${asset_out} DIRECTORY)
	add_custom_command(OUTPUT ${asset_out}
					   COMMAND ${CMAKE_COMMAND} -E make_directory ${asset_out_dir}
	                   COMMAND ${CMAKE_COMMAND} -E copy ${asset}
	                                                    ${asset_out}
	                   DEPENDS ${asset})
	list(APPEND assets_out ${asset_out})
endforeach()
add_custom_target(${TARGET_NAME}-copy-assets DEPENDS ${assets_out})

# dependencies
add_dependencies(${TARGET_NAME} ${TARGET_NAME}-copy-assets)

set(compile_definitions ${common_compile_definitions})
target_compile_definitions(${TARGET_NAME} PRIVATE "${compile_definitions}")

#endregion

#region Compile options
target_compile_options(${TARGET_NAME} PRIVATE "${common_compile_options}")
target_compile_options(${TARGET_NAME} PRIVATE "-Wno-pedantic")
target_compile_definitions(${TARGET_NAME} PRIVATE "${common_compile_definitions}")

target_link_libraries(${TARGET_NAME} HapiIntelCoreDLA)

target_link_libraries(${TARGET_NAME} HapiIntelVvpVfw)
target_link_libraries(${TARGET_NAME} HapiIntelVvpVfr)
target_link_libraries(${TARGET_NAME} HapiIntelVvpScaler)
target_link_libraries(${TARGET_NAME} HapiIntelVvpSwitch)
target_link_libraries(${TARGET_NAME} SwApiUtils)
target_link_libraries(${TARGET_NAME} SwApiIspCommon)
target_link_libraries(${TARGET_NAME} CoreDLAProcessor)
target_link_libraries(${TARGET_NAME} drmHelper)

if(NOT CPACK_ONLY)
	target_link_libraries(${TARGET_NAME} Altera_FPGA_AI_Suite_OpenVINO_plugin)
endif()

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_include_directories(${TARGET_NAME}Config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

#endregion

if (${${TARGET_NAME}_IS_TOP_LEVEL})
    include(cmake/CopyCompileCommands.cmake)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
