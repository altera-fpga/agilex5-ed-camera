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
project(SwApiVideoThrottle
    VERSION 0.0.1
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(${PROJECT_NAME}_IS_TOP_LEVEL ON)
endif()

#region Finding source and header files
file(GLOB headerHFiles *.h)
file(GLOB headerHppFiles *.hpp)
set(headerFiles ${headerHFiles} ${headerHppFiles})

file(GLOB sourceFiles *.cpp)

set(allFiles ${headerFiles} ${sourceFiles})
#endregion

#region Library target
add_library(${PROJECT_NAME} STATIC ${allFiles})

target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
#endregion

#region Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

target_link_libraries(${PROJECT_NAME} HapiVvpThrottle)
#target_link_libraries(${PROJECT_NAME} SwApiUtils)
#target_link_libraries(${PROJECT_NAME} SwApiIspCommon)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#endregion

# compile_commands.json file
if (${${PROJECT_NAME}_IS_TOP_LEVEL})
    include(cmake/CopyCompileCommands.cmake)
endif()

#region CPack
# By default, only disable CPack if we're running tests.
option(DISABLE_CPACK "Disable cpack" ${ENABLE_TESTING})
if(NOT DISABLE_CPACK)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
endif()
#endregion

# Testing
if (${${PROJECT_NAME}_IS_TOP_LEVEL})
    # Only produce tests if we're the top level project.
    option(ENABLE_TESTING "Enable tests" OFF)
    if(ENABLE_TESTING)
        add_subdirectory(test)
        message("Enabled tests.")
    endif()
endif()
