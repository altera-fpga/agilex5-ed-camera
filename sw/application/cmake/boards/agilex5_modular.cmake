# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

list(APPEND CAMERA_SOURCE_DIRS "framos-gmsl")
list(APPEND CAMERA_SOURCE_DIRS "framos-imx676c")
list(APPEND CAMERA_SOURCE_DIRS "framos-imx678")
list(APPEND CAMERA_LIBS "SwApiFramosGMSL")
list(APPEND CAMERA_LIBS "SwApiFramosIMX676")
list(APPEND CAMERA_LIBS "SwApiFramosIMX678")
list(APPEND CAMERA_DEFINITIONS "CAMERA_FramosGMSL")
list(APPEND CAMERA_DEFINITIONS "CAMERA_FramosIMX676")
list(APPEND CAMERA_DEFINITIONS "CAMERA_FramosIMX678")
if(ISP_AI_BUILD OR ISP_STITCH_BUILD)
    set(AWB_PROFILE awb_profile_imx678_sdr.json)
else()
    set(AWB_PROFILE awb_profile_imx678_hdr.json)
endif()
