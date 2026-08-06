# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiIntelAddrSpanExpander)

set(header_files "")
set(source_files "")

# Header files
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/HapiAddrSpanExpander.h")
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/intel_addr_span_expander_io.h")
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/../inc/intel_addr_span_expander_regs.h")
set(header_files ${header_files} "${CMAKE_CURRENT_SOURCE_DIR}/../inc/intel_addr_span_expander.h")

# Source files
set(source_files ${source_files} "${CMAKE_CURRENT_SOURCE_DIR}/../src/intel_addr_span_expander.c")

set(all_files ${header_files} ${source_files})

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../inc")
target_link_libraries(${PROJECT_NAME} HapiCore)
target_link_libraries(${PROJECT_NAME} HapiCoreOCS)

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "HAPI")
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
