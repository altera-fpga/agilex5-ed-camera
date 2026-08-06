# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(CoreDLAProcessor)

set(TARGET_NAME ${PROJECT_NAME})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Header files
set(header_files )
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/CoreDLAProcessor.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/ICoreDLAIntf.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/ICoreDlaRuntime.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/IInputCallback.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/ImageReader.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/InferenceTypes.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/IResultProcessor.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/IResultsCallback.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/OpenVinoIE.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/Semaphore.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloClassificationItem.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloClassificationResult.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloDetectionObject.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloDetectionProcessor.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloNames.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloOutputBlob.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloParams.h)
list(APPEND header_files ${CMAKE_CURRENT_SOURCE_DIR}/inc/YoloPoseProcessor.h)
# Source files
set(source_files )
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/CoreDLAProcessor.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenVinoIE.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/YoloNames.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/YoloClassificationItem.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/YoloClassificationResult.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/YoloDetectionProcessor.cpp)
list(APPEND source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/YoloPoseProcessor.cpp)
 
source_group(include FILES ${header_files})
source_group(src FILES ${source_files})

set(all_files ${header_files} ${source_files})

# Target
add_library(${TARGET_NAME} STATIC ${all_files})


# Compile options
target_compile_options(${TARGET_NAME} PRIVATE "${common_compile_options}" "--warn-no-missing-field-initializers" "--warn-no-unused-result" "--warn-no-pedantic" )

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/inc 
	$ENV{COREDLA_ROOT}/inc
	$ENV{COREDLA_ROOT}/thirdparty/openvino_dev_api/dev_api
	$ENV{COREDLA_ROOT}/thirdparty/openvino_dev_api/plugin_api
	$ENV{COREDLA_ROOT}/thirdparty/openvino_dev_api/plugin_api/openvino/runtime
	$ENV{COREDLA_ROOT}/thirdparty/openvino_dev_api/openvino/inc
	  )

set(compile_definitions ${common_compile_definitions})
target_compile_definitions(${TARGET_NAME} PRIVATE "${compile_definitions}")

set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "./lib:.")

# Copy asstes
set(assets )
list(APPEND assets ${CMAKE_CURRENT_SOURCE_DIR}/plugins.xml)
list(APPEND all_files ${assets})

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
	install(FILES ${asset_out} DESTINATION . COMPONENT deploy)
endforeach()
add_custom_target(${TARGET_NAME}-copy-assets DEPENDS ${assets_out})

# dependencies
add_dependencies(${TARGET_NAME} ${TARGET_NAME}-copy-assets)

set(dependencies )
if(NOT CPACK_ONLY)
	list(APPEND dependencies TBB::tbb)
	list(APPEND dependencies openvino::runtime)
	list(APPEND dependencies coreDLAPluginConfig) 
	list(APPEND dependencies Altera_FPGA_AI_Suite_OpenVINO_plugin_context)
endif()
list(APPEND dependencies lvgl)
list(APPEND dependencies HapiIntelCoreDLA)
list(APPEND dependencies SwApiVvpIspFrameQueueConfig)
list(APPEND dependencies AiResultsRenderer)
list(APPEND dependencies Logging)

target_link_libraries(${TARGET_NAME} PUBLIC ${dependencies} pthread rt)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
