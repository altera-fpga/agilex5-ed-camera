# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************

project(HapiIntelVvpWarp)

set(ip_driver_directory "${CMAKE_CURRENT_SOURCE_DIR}/../intel_vvp_warp_driver")
set(warp_mesh_directory "${CMAKE_CURRENT_SOURCE_DIR}/../intel_vvp_warp_mesh")
set(warp_data_directory "${CMAKE_CURRENT_SOURCE_DIR}/../intel_vvp_warp_data")

# Header files

# intel_vvp_warp Hapi interface headers
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/HapiVvpWarp.h")
set(header_files ${driver_header_files} "${ip_driver_directory}/inc/intel_vvp_warp.h")

# intel_vvp_warp_driver internal headers
set(driver_header_files ${driver_header_files} "${ip_driver_directory}/linux/inc/intel_vvp_warp_io.h")
set(driver_header_files ${driver_header_files} "${ip_driver_directory}/inc/intel_vvp_warp_regs.h")
set(driver_header_files ${driver_header_files} "${ip_driver_directory}/src/intel_vvp_warp_private.h")

# intel_vvp_warp_mesh headers
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/ArbitraryMesh.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/ITransformer.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/MatrixOperations.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/Transformer.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/Triangle.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/WarpConfigurator.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/inc/WarpConfiguratorTypes.h")
set(mesh_header_files ${mesh_header_files} "${warp_mesh_directory}/src/WarpConfiguratorUtils.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpMesh.h")

# intel_vvp_warp_data headers
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpData.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpDataContext.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpDataGenerator.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpDataHelper.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/inc/WarpMesh.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/src/WarpDataUtils.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/src/WarpEngineState.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/src/WarpDataImpl.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/src/WarpMem.h")
set(data_header_files ${data_header_files} "${warp_data_directory}/src/WarpBlockMapping.h")

# Source files

# intel_vvp_warp_driver source files
set(driver_source_files ${driver_source_files} "${ip_driver_directory}/src/intel_vvp_warp.c")

# intel_vvp_warp_mesh source files
set(mesh_source_files ${mesh_source_files} "${warp_mesh_directory}/src/ArbitraryMesh.cpp")
set(mesh_source_files ${mesh_source_files} "${warp_mesh_directory}/src/MatrixOperations.cpp")
set(mesh_source_files ${mesh_source_files} "${warp_mesh_directory}/src/Transformer.cpp")
set(mesh_source_files ${mesh_source_files} "${warp_mesh_directory}/src/WarpConfigurator.cpp")

# intel_vvp_warp_data source files
set(data_source_files ${data_source_files} "${warp_data_directory}/src/WarpDataGenerator.cpp")
set(data_source_files ${data_source_files} "${warp_data_directory}/src/WarpEngineState.cpp")
set(data_source_files ${data_source_files} "${warp_data_directory}/src/WarpDataHelper.cpp")
set(data_source_files ${data_source_files} "${warp_data_directory}/src/WarpDataUtils.cpp")
set(data_source_files ${data_source_files} "${warp_data_directory}/src/WarpMesh.cpp")

# Targets

# intel_vvp_warp_driver Hapi interface target
set(interface_target_name ${PROJECT_NAME}Interface)
add_library(${interface_target_name} INTERFACE)
target_include_directories(${interface_target_name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${interface_target_name} INTERFACE "${ip_driver_directory}/inc")
target_include_directories(${interface_target_name} INTERFACE "${ip_driver_directory}/linux/inc")
target_sources(${interface_target_name} INTERFACE ${header_files})
target_link_libraries(${interface_target_name} INTERFACE HapiCore)

# intel_vvp_warp_data target
set(data_target_name ${PROJECT_NAME}Data)
add_library(${data_target_name} STATIC ${data_header_files} ${data_source_files})
target_include_directories(${data_target_name} PUBLIC "${warp_data_directory}/inc")
target_include_directories(${data_target_name} PUBLIC "${warp_data_directory}/src")
target_link_libraries(${data_target_name} PUBLIC ${interface_target_name})

# intel_vvp_warp_mesh target
set(mesh_target_name ${PROJECT_NAME}Mesh)
add_library(${mesh_target_name} STATIC ${mesh_header_files} ${mesh_source_files})
target_include_directories(${mesh_target_name} PUBLIC "${warp_mesh_directory}/inc")
target_include_directories(${mesh_target_name} PUBLIC "${warp_data_directory}/inc")
target_link_libraries(${mesh_target_name} PUBLIC ${data_target_name})
target_link_libraries(${mesh_target_name} PUBLIC HapiCore)

# intel_vvp_warp Hapi implementation target
add_library(${PROJECT_NAME} STATIC ${driver_header_files} ${driver_source_files})
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "HAPI")
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_link_libraries(${PROJECT_NAME} ${interface_target_name})
target_link_libraries(${PROJECT_NAME} ${mesh_target_name})
target_link_libraries(${PROJECT_NAME} ${data_target_name})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_definitions(${PROJECT_NAME} PRIVATE INTEL_VVP_WARP_ENABLE_LOGGING)
target_compile_definitions(${data_target_name} PRIVATE INTEL_VVP_WARP_DATA_ENABLE_LOGGING)
endif()
