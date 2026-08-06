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
project(HttpServer)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT USE_OPENSSL)
	set(USE_OPENSSL False)
endif()

if (USE_OPENSSL)
    if ("${open_ssl_folder}" STREQUAL "")
        if (UNIX)
            set(open_ssl_folder "${CMAKE_CURRENT_SOURCE_DIR}/../OpenSSL/openssl-1.1.1g-linux-poky/include")
        else()
            set(open_ssl_folder "${CMAKE_CURRENT_SOURCE_DIR}/../OpenSSL/openssl-1.1.1g-windows/include")
        endif()
    endif()
endif()

# Header files
set(header_files "")
set(header_files ${header_files} "Include/IHttpServer.h")
set(header_files ${header_files} "Include/IHttpServerHost.h")
set(header_files ${header_files} "Source/Encoding.h")
set(header_files ${header_files} "Source/HttpCommand.h")
set(header_files ${header_files} "Source/HttpResponse.h")
set(header_files ${header_files} "Source/HttpServer.h")
set(header_files ${header_files} "Source/HttpStatusCode.h")
set(header_files ${header_files} "Source/OSocket.h")
set(header_files ${header_files} "Source/OTime.h")
set(header_files ${header_files} "Source/SelectEvent.h")
set(header_files ${header_files} "Source/SHA1.h")
set(header_files ${header_files} "Source/StreamReader.h")
set(header_files ${header_files} "Source/StringBuilder.h")
set(header_files ${header_files} "Source/targetver.h")
set(header_files ${header_files} "Source/WebSocket.h")
set(header_files ${header_files} "Source/WebSocketCommands.h")
set(header_files ${header_files} "Source/WebSocketMessageIDs.h")
set(header_files ${header_files} "Source/EmbeddedFile.h")

# Source files
set(source_files "")
set(source_files ${source_files} "Source/HttpCommand.cpp")
set(source_files ${source_files} "Source/HttpResponse.cpp")
set(source_files ${source_files} "Source/HttpServer.cpp")
set(source_files ${source_files} "Source/IWebSocketCallback.cpp")
set(source_files ${source_files} "Source/OSocket.cpp")
set(source_files ${source_files} "Source/OTime.cpp")
set(source_files ${source_files} "Source/SelectEvent.cpp")
set(source_files ${source_files} "Source/SHA1.cpp")
set(source_files ${source_files} "Source/StreamReader.cpp")
set(source_files ${source_files} "Source/StringBuilder.cpp")
set(source_files ${source_files} "Source/WebSocket.cpp")
set(source_files ${source_files} "Source/WebSocketCommands.cpp")
set(source_files ${source_files} "Source/EmbeddedFile.cpp")

set(all_files ${header_files} ${source_files})

add_library(${PROJECT_NAME} STATIC ${all_files})
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")
if (USE_OPENSSL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "USE_OPENSSL")
    target_link_libraries(${PROJECT_NAME} OpenSSL)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
target_link_libraries(${PROJECT_NAME} AppToolkitUtils)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} Ws2_32.lib)
endif()

if (USE_OPENSSL)
    target_include_directories(${PROJECT_NAME} PRIVATE "${open_ssl_folder}")
endif()

# Compile options
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")

add_subdirectory(AppToolkitUtils)

if (UNIT_TEST)

  set(unit_test_files "")
  set(unit_test_files ${unit_test_files} "WebSocketCommands_UnitTest.cpp")
  set(unit_test_files ${unit_test_files} "HttpCommand_UnitTest.cpp")
  set(unit_test_files ${unit_test_files} "HttpResponse_UnitTest.cpp")

  list(TRANSFORM unit_test_files PREPEND unit-test/)

  set(test_exe ${PROJECT_NAME}_UnitTest)

  add_executable(${test_exe} EXCLUDE_FROM_ALL ${unit_test_files} ${source_files})
  target_include_directories(${test_exe} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Include")
  target_include_directories(${test_exe} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
  target_link_libraries(${test_exe} PRIVATE AppToolkitUtils)
  target_link_libraries(${test_exe} PRIVATE AppToolkitUtils_UnitTest_Headers)
  target_link_libraries(${test_exe} PRIVATE gmock_main)
  target_compile_options(${test_exe} PRIVATE "${common_compile_options}")
  target_compile_definitions(${test_exe} PRIVATE "${common_compile_definitions}")
  target_compile_definitions(${test_exe} PRIVATE UNIT_TEST)
  target_link_options(${test_exe} PRIVATE "${common_link_options}")

  add_dependencies(Test ${test_exe})

endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
