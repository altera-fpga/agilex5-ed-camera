# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

add_subdirectory(dmaOverlayHelper)
add_subdirectory(drmHelper)
add_subdirectory(IOverlayHelper)
add_subdirectory(MultiChannelAutoExposure)
add_subdirectory(MultiChannelWhiteBalance)
add_subdirectory(IspMixerHelper)
add_subdirectory(lvglLogoHelper)
add_subdirectory(logging)
add_subdirectory(ROIHelper)
add_subdirectory(stitch-adapter)
add_subdirectory(IspVfrInput)

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)
