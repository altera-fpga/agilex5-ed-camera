# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)
project(AppToolkitUtils)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files "")
set(source_files "")

# Header files
set(header_files "")
set(header_files ${header_files} "AppToolkitDataStructureUtils.h")
set(header_files ${header_files} "AppToolkitStringUtils.h")
set(header_files ${header_files} "AppToolkitThread.h")
set(header_files ${header_files} "AppToolkitOS.h")
set(header_files ${header_files} "AppToolkitFile.h")
set(header_files ${header_files} "AppToolkitIJson.h")
set(header_files ${header_files} "AppToolkitJson.h")
set(header_files ${header_files} "AtUtils.h")
set(header_files ${header_files} "AppToolkitMessageQueue.h")
set(header_files ${header_files} "AppToolkitReactorFactory.h")
set(header_files ${header_files} "AppToolkitReactor.h")
set(header_files ${header_files} "AppToolkitReactorInternal.h")

# Source files
set(source_files "")
set(source_files ${source_files} "AppToolkitStringUtils.cpp")
set(source_files ${source_files} "AppToolkitThread.cpp")
set(source_files ${source_files} "AppToolkitReactorFactory.cpp")
set(source_files ${source_files} "AppToolkitOS.cpp")
set(source_files ${source_files} "AppToolkitFile.cpp")
set(source_files ${source_files} "AppToolkitJson.cpp")
set(source_files ${source_files} "AppToolkitMessageQueue.cpp")

if (NOT WIN32)
    set(header_files ${header_files} "AppToolkitVirtualAccess.h")
    set(source_files ${source_files} "AppToolkitVirtualAccess.cpp")
endif()

set(all_files ${header_files} ${source_files})

add_library(${PROJECT_NAME} STATIC ${all_files})
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

if (UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()

if (UNIT_TEST)

  set(unit_test_files "")
  set(unit_test_files ${unit_test_files} "AppToolkitMessageQueue_UnitTest.cpp")
  set(unit_test_files ${unit_test_files} "AppToolkitStringUtils_UnitTest.cpp")

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
