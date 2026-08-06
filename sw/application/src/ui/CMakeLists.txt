# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

add_subdirectory(LogoControls)
add_subdirectory(MultiChannelAutoExposureControls)
add_subdirectory(MultiChannelWhiteBalanceControls)
add_subdirectory(OutputControls)
add_subdirectory(ProfileBlackLevelControls)
add_subdirectory(StitchControls)
add_subdirectory(VfwControls)
add_subdirectory(VfrInputControls)
if(ISP_AI_BUILD OR CPACK_ONLY)
    add_subdirectory(AIRuntimeControls)
endif()

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)