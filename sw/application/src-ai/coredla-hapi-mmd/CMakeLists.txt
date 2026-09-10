# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(coredla_hapi_mmd)
set (TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

# DLA specific modifications made to the MMD
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDLA_MMD")


if(RUNTIME_POLLING)
add_definitions(-DCOREDLA_RUNTIME_POLLING)
add_definitions(-DRUNTIME_POLLING)
endif(RUNTIME_POLLING)

set(MMD_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/host/acl_hapi.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/host/mmd_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/host/uio_device.cpp
)

set(MMD_INC
  ${CMAKE_CURRENT_SOURCE_DIR}/host/acl_hapi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/host/hapi_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/host/mmd_device.h
  ${CMAKE_CURRENT_SOURCE_DIR}/host/uio_device.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/aocl_mmd.h
)
set(all_files ${MMD_SRC} ${MMD_INC})
add_library(${TARGET_NAME} SHARED ${MMD_SRC})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/host)

target_link_libraries(${TARGET_NAME}
    HapiCoreOCS
    HapiIntelCoreDLA
    HapiIntelOnChipMemII
    HapiIntelPio
    SwApiUtils
)

set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "./lib")
target_compile_options(${TARGET_NAME} PRIVATE ${common_compile_options} -Wno-stringop-overread)

install(TARGETS ${TARGET_NAME} DESTINATION lib COMPONENT deploy)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
