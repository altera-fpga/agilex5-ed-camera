# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)
cmake_policy(SET CMP0079 NEW)

project(ApplicationToolkit)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(header_files "")
set(source_files "")

# Header files
set(header_files ${header_files} "CommonCommands.h")
set(header_files ${header_files} "DataSegment.h")
set(header_files ${header_files} "FileTransfer.h")
set(header_files ${header_files} "FileTransferWebSocketCommandProcessor.h")
set(header_files ${header_files} "CommonMessageIDs.h")
set(header_files ${header_files} "ICommand.h")
set(header_files ${header_files} "ParamList.h")
set(header_files ${header_files} "UiElements.h")
set(header_files ${header_files} "CommonUiLayer.h")
set(header_files ${header_files} "CommonUiUpdate.h")
set(header_files ${header_files} "UiWebSocketCommandProcessor.h")
set(header_files ${header_files} "WebServer.h")
set(header_files ${header_files} "WebSocketCommandProcessor.h")
set(header_files ${header_files} "CommonApplicationBase.h")
set(header_files ${header_files} "IFileImportDestination.h")
set(header_files ${header_files} "CommonTypes.h")
set(header_files ${header_files} "ObserverPattern.h")
set(header_files ${header_files} "Variant.h")
set(header_files ${header_files} "CommandLine.h")
set(header_files ${header_files} "IpUiControls.h")
set(header_files ${header_files} "LED.h")
set(header_files ${header_files} "PictureWebSocketCommandProcessor.h")
set(header_files ${header_files} "AppToolkitRawRgbImage.h")
set(header_files ${header_files} "PictureInPictureHandler.h")
set(header_files ${header_files} "ColourPickerHandler.h")
set(header_files ${header_files} "PictureCaptureHandler.h")
set(header_files ${header_files} "Settings.h")
set(header_files ${header_files} "IDialogHandler.h")
set(header_files ${header_files} "DialogFactory.h")
set(header_files ${header_files} "UiConfig.h")
set(header_files ${header_files} "DataTransferWebSocketCommandProcessor.h")
set(header_files ${header_files} "IWebAPI.h")
set(header_files ${header_files} "LoggerControl.h")

# Source files
set(source_files ${source_files} "CommonCommands.cpp")
set(source_files ${source_files} "DataSegment.cpp")
set(source_files ${source_files} "FileTransfer.cpp")
set(source_files ${source_files} "FileTransferWebSocketCommandProcessor.cpp")
set(source_files ${source_files} "ICommand.cpp")
set(source_files ${source_files} "ParamList.cpp")
set(source_files ${source_files} "UiElements.cpp")
set(source_files ${source_files} "CommonUiLayer.cpp")
set(source_files ${source_files} "CommonUiUpdate.cpp")
set(source_files ${source_files} "UiWebSocketCommandProcessor.cpp")
set(source_files ${source_files} "WebServer.cpp")
set(source_files ${source_files} "WebSocketCommandProcessor.cpp")
set(source_files ${source_files} "Variant.cpp")
set(source_files ${source_files} "CommonApplicationBase.cpp")
set(source_files ${source_files} "CommandLine.cpp")
set(source_files ${source_files} "IpUiControls.cpp")
set(source_files ${source_files} "LED.cpp")
set(source_files ${source_files} "PictureWebSocketCommandProcessor.cpp")
set(source_files ${source_files} "ColourPickerHandler.cpp")
set(source_files ${source_files} "PictureInPictureHandler.cpp")
set(source_files ${source_files} "PictureCaptureHandler.cpp")
set(source_files ${source_files} "Settings.cpp")
set(source_files ${source_files} "DataTransferWebSocketCommandProcessor.cpp")
set(source_files ${source_files} "LoggerControl.cpp")

set(all_files ${header_files} ${source_files})
add_library(${PROJECT_NAME} STATIC ${all_files})

# Override in the application CMakeLists.txt to minify JS/CSS assets before packing
option(WEBTOOLKIT_MINIFY_ASSETS "Minify WebToolkit JS/CSS assets before packing" OFF)
# Override in the application CMakeLists.txt to use python version of FilePacker
option(WEBTOOLKIT_PYTHON_PACKER "Use WebToolkit python FilePacker" OFF)

if (NOT UNIT_TEST)

# Embed Web UI into the application toolkit library
include(cmake/embed_webui.cmake)
embed_webui(${PROJECT_NAME})

endif()

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE WebToolkit.rc)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (UNIX)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wno-psabi)
endif()

# Common options and definitions from parent
target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "${common_compile_definitions}")

if (WIN32)
    target_link_libraries(${PROJECT_NAME} Winmm.lib)
endif()

if (UNIT_TEST)
	target_compile_definitions(${PROJECT_NAME} PRIVATE UNIT_TEST)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} HttpServer)

add_subdirectory(http-server)
add_subdirectory(settings-file-fuzzer)

if (UNIT_TEST)

    set(unit_test_files "")
    set(unit_test_files ${unit_test_files} "AppToolkitRawRgbImage_UnitTest.cpp")
    set(unit_test_files ${unit_test_files} "CommandLine_UnitTest.cpp")
    set(unit_test_files ${unit_test_files} "DataSegment_UnitTest.cpp")
    set(unit_test_files ${unit_test_files} "UiElements_UnitTest.cpp")
    set(unit_test_files ${unit_test_files} "Variant_UnitTest.cpp")

    list(TRANSFORM unit_test_files PREPEND unit-test/)

    set(test_exe ${PROJECT_NAME}_UnitTest)

    add_executable(${test_exe} EXCLUDE_FROM_ALL ${unit_test_files} ${source_files})
    target_include_directories(${test_exe} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(${test_exe} PRIVATE TestAssertion)
    target_link_libraries(${test_exe} PRIVATE AppToolkitUtils_UnitTest_Headers)
    target_link_libraries(${test_exe} PRIVATE gmock_main)
    target_link_libraries(${test_exe} PRIVATE HttpServer)
    target_compile_options(${test_exe} PRIVATE "${common_compile_options}")
    target_compile_definitions(${test_exe} PRIVATE "${common_compile_definitions}")
    target_compile_definitions(${test_exe} PRIVATE UNIT_TEST)
    target_link_options(${test_exe} PRIVATE "${common_link_options}")

    add_dependencies(Test ${test_exe})

endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
