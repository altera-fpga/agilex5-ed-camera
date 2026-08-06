# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(AI_ISP)

option(RUNTIME_POLLING "Use runetime polling instead of ISR" OFF)
option(STREAM_CONTROLLER_ACCESS "Support stream controller" ON)

if(NOT CPACK_ONLY)
include(cmake/OpenVINO.cmake OPTIONAL)
include(cmake/CoreDLA.cmake OPTIONAL)
endif()

# AI libraries
add_subdirectory(ai-results-renderer)
add_subdirectory(coredla-processor)
add_subdirectory(coredla-runtime)
add_subdirectory(coredla-hapi-mmd)
add_subdirectory(frame-queue)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cnn_networks)
    add_subdirectory(cnn_networks)
endif()

file(RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(DEFAULT_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLA.cmake DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLA.patch DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLAPatchCommand.sh DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY PERMISSIONS ${DEFAULT_PERMISSIONS})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLAPerformance.patch DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLABypassOLT.patch DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OpenVINO.cmake DESTINATION "${dest_dir}/cmake" COMPONENT SRC_DELIVERY)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/arch/generated_arch.arch DESTINATION "${dest_dir}/arch" COMPONENT SRC_DELIVERY)

add_custom_command(
    OUTPUT  dummy.cpp
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

set(all_files dummy.cpp)

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} PUBLIC AiResultsRenderer)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVvpIspFrameQueue)

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)