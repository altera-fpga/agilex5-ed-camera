# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
project(SwApiUtils)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT DEFINED SW_API_UTILS_MEM_SHARED_LIBRARY)
    set(SW_API_UTILS_MEM_SHARED_LIBRARY OFF)
endif()
set(header_files "")
set(source_files "")

# Header files
set(header_files "")
set(header_files ${header_files} "SwUtilsDataStructureUtils.h")
set(header_files ${header_files} "SwUtilsStringUtils.h")
set(header_files ${header_files} "SwUtilsThread.h")
set(header_files ${header_files} "SwUtilsOS.h")
set(header_files ${header_files} "SwUtilsFile.h")
set(header_files ${header_files} "SwUtilsIJson.h")
set(header_files ${header_files} "SwUtilsJson.h")
set(header_files ${header_files} "SwUtils.h")
set(header_files ${header_files} "SwUtilsMessageQueue.h")
set(header_files ${header_files} "SwUtilsReactorFactory.h")
set(header_files ${header_files} "SwUtilsReactor.h")
set(header_files ${header_files} "SwUtilsReactorInternal.h")
set(header_files ${header_files} "SwUtilsMemBuffer.h")
set(header_files ${header_files} "SwUtilsBitfield.h")
set(header_files ${header_files} "SwUtilsLockedHandle.h")

# Source files
set(source_files "")
set(source_files ${source_files} "SwUtilsStringUtils.cpp")
set(source_files ${source_files} "SwUtilsThread.cpp")
set(source_files ${source_files} "SwUtilsReactorFactory.cpp")
set(source_files ${source_files} "SwUtilsOS.cpp")
set(source_files ${source_files} "SwUtilsFile.cpp")
set(source_files ${source_files} "SwUtilsJson.cpp")
set(source_files ${source_files} "SwUtilsMessageQueue.cpp")

if (UNIX)
    set(header_files ${header_files} "SwUtilsVirtualAccess.h")
    set(source_files ${source_files} "SwUtilsVirtualAccess.cpp")
    if(SW_API_UTILS_MEM_SHARED_LIBRARY)
        set(mem_header_files ${mem_header_files} "SwUtilsMemTransfer.h")
        set(mem_source_files ${mem_source_files} "SwUtilsMemTransfer.cpp")
    else()
        set(header_files ${header_files} "SwUtilsMemTransfer.h")
        set(source_files ${source_files} "SwUtilsMemTransfer.cpp")
    endif()
endif()

add_library(${PROJECT_NAME} STATIC ${header_files} ${source_files})
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}"  -fvisibility=hidden -fvisibility-inlines-hidden)

if (UNIX)
	target_link_libraries(${PROJECT_NAME} PUBLIC pthread)
endif()

if(SW_API_UTILS_MEM_SHARED_LIBRARY)
    add_library(${PROJECT_NAME}Mem SHARED ${mem_header_files} ${mem_source_files})
    target_compile_definitions(${PROJECT_NAME}Mem PRIVATE "${common_compile_definitions}")

    # Include directories
    target_include_directories(${PROJECT_NAME}Mem PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # Compile options
    target_compile_options(${PROJECT_NAME}Mem PRIVATE "${common_compile_options}")

    target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}Mem)

    install(TARGETS ${PROJECT_NAME}Mem DESTINATION lib COMPONENT deploy)
endif()

set(all_files ${header_files} ${source_files} ${mem_header_files} ${mem_source_files})

if (UNIT_TEST)

  set(unit_test_files "")
  set(unit_test_files ${unit_test_files} "SwUtilsMessageQueue_UnitTest.cpp")
  set(unit_test_files ${unit_test_files} "SwUtilsStringUtils_UnitTest.cpp")

  list(TRANSFORM unit_test_files PREPEND unit-test/)

  set(test_exe ${PROJECT_NAME}_UnitTest)

  add_executable(${test_exe} EXCLUDE_FROM_ALL ${unit_test_files} ${source_files})
  target_include_directories(${test_exe} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(${test_exe} gmock_main)
  target_compile_options(${test_exe} PRIVATE "${common_compile_options}")
  target_compile_definitions(${test_exe} PRIVATE "${common_compile_definitions}")
  target_link_options(${test_exe} PRIVATE "${common_link_options}")

  add_dependencies(Test ${test_exe})

  # Export unit testing headers (mocks etc) as an interface-only library
  set (test_headers ${test_exe}_Headers)
  add_library(${test_headers} INTERFACE)
  target_include_directories(${test_headers} INTERFACE unit-test/)

  # Library to override assertion failure with a custom exception (NOTE: not for C code that uses assert)
  add_library(TestAssertion unit-test/TestAssertion.cpp)
  target_include_directories(TestAssertion PUBLIC unit-test/)

endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
