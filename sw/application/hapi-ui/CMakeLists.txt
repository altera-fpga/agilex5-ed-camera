# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiUiISP)

# HAPI UI libraries
add_subdirectory(hapi-ui-warp-full)
add_subdirectory(hapi-ui-i2c)
add_subdirectory(hapi-ui-3d-lut)
add_subdirectory(hapi-ui-tone-mapping)
add_subdirectory(hapi-ui-tpg)
add_subdirectory(hapi-ui-switch)
add_subdirectory(hapi-ui-clipper)
add_subdirectory(hapi-ui-demosaic)
add_subdirectory(hapi-ui-dpc)
add_subdirectory(hapi-ui-blc)
add_subdirectory(hapi-ui-wbs)
add_subdirectory(hapi-ui-wbc)
add_subdirectory(hapi-ui-anr)
add_subdirectory(hapi-ui-bls)
add_subdirectory(hapi-ui-ccm)
add_subdirectory(hapi-ui-vc)
add_subdirectory(hapi-ui-hs)
add_subdirectory(hapi-ui-observable)
add_subdirectory(hapi-ui-csc)
add_subdirectory(hapi-ui-crs)
add_subdirectory(hapi-ui-usm)
add_subdirectory(hapi-ui-1dlut)

add_subdirectory(hapi-ui-vvpisp-source-select)
add_subdirectory(hapi-ui-vvpisp-auto-white-balance)
add_subdirectory(hapi-ui-vvpisp-auto-exposure)
add_subdirectory(hapi-ui-vvpisp-warp-output)

add_subdirectory(hapi-ui-vvpisp-profile-loader)
add_subdirectory(hapi-ui-vvpisp-profile-overview)
add_subdirectory(hapi-ui-vvpisp-profile-region-select)
add_subdirectory(hapi-ui-vvpisp-profile-vignette-correction)
add_subdirectory(hapi-ui-vvpisp-profile-white-balance)
add_subdirectory(hapi-ui-vvpisp-profile-colour-correction)
add_subdirectory(hapi-ui-vvpisp-camera)

add_custom_command(
    OUTPUT  dummy.cpp
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

set(all_files dummy.cpp)

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} PUBLIC HapiVvpThrottle)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUi3dLut)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiI2C)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiToneMapping)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiWarpFull)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiTpg)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiSwitch)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiClipper)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiDemosaic)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiDpc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiBlc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiWbs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiWbc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiAnr)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiBls)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiCcm)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiHs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspSourceSelect)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspAutoWhiteBalance)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspAutoExposure)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiObservable)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiCsc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiCrs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiUsm)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiLut1D)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspWarpOutput)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileLoader)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileOverview)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileMasterRegionSelector)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileVignetteCorrection)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileWhiteBalance)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiVvpIspProfileColourCorrection)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiCameraUiControls)

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)