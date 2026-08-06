# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.10)

project(coredla_runtime)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*|aarch64.*|AARCH64.*)")
  set(ARM ON)
endif()

set (TARGET_NAME Altera_FPGA_AI_Suite_OpenVINO_plugin)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if (NOT DEFINED DLA_OV_DEVICE_NAME)
  set(DLA_OV_DEVICE_NAME "FPGA")
endif()

#
# Adds compiler flags to C / C++ sources
#
macro(dla_add_compiler_flags)
    foreach(flag ${ARGN})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    endforeach()
endmacro()

####################################################################
## SDL required compiler flags
####################################################################
# Needed for all builds

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat -Wformat-security")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Wformat-security")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIE")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIE")

# Release build only
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fstack-protector-strong")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -z noexecstack -z relro -z now")

# Release build only
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -z noexecstack -z relro -z now")

set (CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fno-strict-overflow -fno-delete-null-pointer-checks -fwrapv")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fno-strict-overflow -fno-delete-null-pointer-checks -fwrapv")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -fno-strict-overflow -fno-delete-null-pointer-checks -fwrapv")

####################################################################

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3")

#### Sanitizer settings ####
# Address
set(CMAKE_C_FLAGS_ASAN "-O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_ASAN "-O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")

# Memory
set(CMAKE_C_FLAGS_MSAN "-O1 -g -fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_MSAN "-O1 -g -fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls")

# Thread
set(CMAKE_C_FLAGS_TSAN "-O1 -g -fsanitize=thread -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_TSAN "-O1 -g -fsanitize=thread -fno-omit-frame-pointer -fno-optimize-sibling-calls")


#set (CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# Enable all warnings except unknown-pragmas.  Wunknown-pragmas must be excluded because
# it is triggered by header file included from OpenCL runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unknown-pragmas")

# Might be too strict for wide deployment, but easy to disable if it causes problems.
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

# Edwinzha: With OV 2023.3.0 LTS a warning is thrown everytime old api is used. Suppress for now until we do a full uplift.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cpp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# This is required on Ubuntu 18; the new linker behaviour transforms
# RPATH into RUNPATH (which can be seen in the output of 'readelf -d').
# However, RUNPATH does not work recursively, so when OpenVINO reads
# the plugins.xml file and searches for the specified libcoreDlaRuntimePlugin.so
# library, it fails.  The --disable-new-dtags option causes the linker
# to keep RPATH as RPATH (rather than morphing to RUNPATH).
#
# References:
#  https://stackoverflow.com/questions/52018092/how-to-set-rpath-and-runpath-with-gcc-ld
#  https://stackoverflow.com/questions/59248421/c-secondary-dependency-resolution-with-runpath
#
# The solution below seems preferable to setting LD_LIBRARY_PATH, if only barely.
# For additional motivation, go ahead and throw away part of your day reading either
# of the screeds:
#  http://xahlee.info/UnixResource_dir/_/ldpath.html
#  https://gms.tf/ld_library_path-considered-harmful.html
# You may find that neither is fully convincing, of course.
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--disable-new-dtags")

# DLA specific modifications made to the MMD
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDLA_MMD")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHPS_PLATFORM")

# Flag to disable JIT mode
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLE_JIT")

# Build the runtime variant of the DLA plugin
add_definitions(-DRUNTIME_DLA_PLUGIN)

# Build the runtime variant of the DLA plugin
add_definitions(-DDLA_OV_DEVICE_NAME="${DLA_OV_DEVICE_NAME}")

set(STREAM_CONTROLLER_ACCESS ON)

# Gets a build version of {git branch}-{git hash} as an identifier for the plugin build version. Defaults to "Custom Build"
function(get_build_version output_variable)
  # Get branch name
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE result_var_branch
    OUTPUT_VARIABLE branch_name
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  # Get hash
  execute_process(
    COMMAND git rev-parse --short=10 HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE result_var_hash
    OUTPUT_VARIABLE git_hash
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT ${result_var_branch} EQUAL 0 OR NOT ${result_var_hash} EQUAL 0)
    set(${output_variable} "Custom build" PARENT_SCOPE)
  else()
    set(${output_variable} "${branch_name}-${git_hash}" PARENT_SCOPE)
  endif()
endfunction()
# Call function to get Git branch name
get_build_version(PLUGIN_BUILD_VERSION)

# Pass PLUGIN_BUILD_VERSION to the source code via a define
add_definitions(-DPLUGIN_BUILD_VERSION="${PLUGIN_BUILD_VERSION}")

SET (INFO 0)
SET (WARNING 1)
SET (ERROR 2)
SET (FATAL 3)

if(DEBUG_RUNTIME)
  add_definitions(-DDEBUG_RUNTIME)
endif(DEBUG_RUNTIME)
if(DEBUG_RUNTIME_MEMORY_TEST)
  add_definitions(-DDEBUG_RUNTIME_MEMORY_TEST)
endif(DEBUG_RUNTIME_MEMORY_TEST)
if(RUNTIME_VERBOSITY)
  add_definitions(-DENABLE_LOGGING)
  add_definitions(-DRUNTIME_VERBOSITY=${${RUNTIME_VERBOSITY}})
endif(RUNTIME_VERBOSITY)

if(RUNTIME_POLLING)
  add_definitions(-DCOREDLA_RUNTIME_POLLING)
endif(RUNTIME_POLLING)

file(GLOB SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/coredla_batch_job.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/coredla_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/coredla_graph_job.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/device_memory_allocator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mmd_wrapper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stream_controller_comms.cpp
  $ENV{COREDLA_ROOT}/dla_plugin/src/*.cpp
  $ENV{COREDLA_ROOT}/util/src/dla_numeric_utils.cpp
)

file(GLOB HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/context/*.h
  $ENV{COREDLA_ROOT}/dla_plugin/inc/dlia/*.hpp
  $ENV{COREDLA_ROOT}/dla_plugin/inc/*.hpp
  $ENV{COREDLA_ROOT}/dla_plugin/inc/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/stream_controller/app/*.h
)
add_library(${TARGET_NAME} SHARED ${SOURCES} ${HEADERS})

set(all_files ${SOURCES} ${HEADERS} ${THIRDPARTY})

target_compile_definitions(${TARGET_NAME} PRIVATE "${common_compile_definitions};STREAM_CONTROLLER_ACCESS")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lrt" )
set(MMD_LIB_NAME coredla_hapi_mmd)

target_include_directories(${TARGET_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/context
  ${CMAKE_CURRENT_SOURCE_DIR}/stream_controller/app
  $ENV{COREDLA_ROOT}/util/inc
  $ENV{COREDLA_ROOT}/util/transformations/inc
  $ENV{COREDLA_ROOT}/dla_plugin/model_analyzer/inc
  $ENV{COREDLA_ROOT}/dla_plugin
  $ENV{COREDLA_ROOT}/dla_plugin/inc
)

if(NOT CPACK_ONLY)
target_link_libraries(${TARGET_NAME} PUBLIC
  ${CMAKE_DL_LIBS}
  openvino::runtime
  openvino_dev_api
  dliaPluginIOTransformations
  dla_op_transformation
  dla_compiled_result
  ${MMD_LIB_NAME}
)
endif()

  # Needed for coredla_device/inc/dla_dma_constants.h to find dla_dma_constants.svh, since
# the cmake description (find_package(CoreDLA)) does not know about this dependency.
#
# Also needed for a variety of .h files in the DISABLE_JIT case, where we do not
# use the CoreDLA package.
if (EXISTS $ENV{COREDLA_ROOT}/inc)
  target_include_directories(${TARGET_NAME} PUBLIC $ENV{COREDLA_ROOT}/inc)
else()
  target_include_directories(${TARGET_NAME} PUBLIC $ENV{COREDLA_ROOT}/build/coredla/dla/inc)
endif()

# include ./lib seaching for coredla_hapi_mmd.so
set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "./lib:.")

install(TARGETS ${TARGET_NAME} DESTINATION lib COMPONENT deploy)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(${TARGET_NAME}_context INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/context/stream_controller_IRemoteContext.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/context/stream_controller_RemoteContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/context/stream_controller_userMessages.h
  )
target_include_directories(${TARGET_NAME}_context INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/inc/context)
if(NOT CPACK_ONLY)
  target_link_libraries(${TARGET_NAME}_context INTERFACE openvino_dev_api)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
