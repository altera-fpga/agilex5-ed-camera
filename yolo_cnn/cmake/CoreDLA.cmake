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
set(OLD_NO_PROXY $ENV{NO_PROXY})
set(ENV{NO_PROXY} "")
set(ENV{no_proxy} "")
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
MyCoreDLA
URL     https://downloads.intel.com/akdlm/software/fpga_ai_suite/2026.1.1/altera-fpga-ai-suite-ubuntu-2026.1.1_amd64.deb
DOWNLOAD_NO_EXTRACT TRUE
PATCH_COMMAND ${PROJECT_SOURCE_DIR}/cmake/CoreDLAPatchCommand.sh <SOURCE_DIR> <DOWNLOADED_FILE>
)
FetchContent_MakeAvailable(MyCoreDLA)
set(FETCHCONTENT_QUIET TRUE)

set(ENV{COREDLA_ROOT} ${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla)

set(ENV{NO_PROXY} ${OLD_NO_PROXY})
set(ENV{no_proxy} ${OLD_NO_PROXY})

set(DLA_SETUPVARS ${mycoredla_SOURCE_DIR}/opt/altera/fpga_ai_suite_2026.1.1/dla/setupvars.sh)
