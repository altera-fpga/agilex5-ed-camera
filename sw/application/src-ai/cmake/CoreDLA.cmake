# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

include(FetchContent)


FetchContent_Declare(
  PugiXML
  OVERRIDE_FIND_PACKAGE 
  URL     https://github.com/zeux/pugixml/releases/download/v1.15/pugixml-1.15.tar.gz
  URL_HASH SHA256=655ade57fa703fb421c2eb9a0113b5064bddb145d415dd1f88c79353d90d511a
)
FetchContent_MakeAvailable(PugiXML)

set(OLD_NO_PROXY $ENV{NO_PROXY})
set(ENV{NO_PROXY} "")
set(ENV{no_proxy} "")
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
MyCoreDLA
OVERRIDE_FIND_PACKAGE 
URL     https://downloads.intel.com/akdlm/software/fpga_ai_suite/2026.1.1/altera-fpga-ai-suite-ubuntu-2026.1.1_amd64.deb
DOWNLOAD_NO_EXTRACT TRUE
PATCH_COMMAND ${PROJECT_SOURCE_DIR}/cmake/CoreDLAPatchCommand.sh <SOURCE_DIR> <DOWNLOADED_FILE>
)
FetchContent_MakeAvailable(MyCoreDLA)
set(FETCHCONTENT_QUIET TRUE)

set(ENV{COREDLA_ROOT} ${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla)

set(ENV{NO_PROXY} ${OLD_NO_PROXY})
set(ENV{no_proxy} ${OLD_NO_PROXY})

add_library(coreDLAPluginConfig INTERFACE ${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla/dla_plugin/inc/dla_plugin_config.hpp)

target_include_directories(coreDLAPluginConfig INTERFACE
  $<BUILD_INTERFACE:${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla/dla_plugin/inc>
)

set(DLA_SETUPVARS ${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla/setupvars.sh)

install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcoreDLAHeteroPlugin.so DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libarchparam.so DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libdla_compiled_result.so DESTINATION lib COMPONENT deploy)

