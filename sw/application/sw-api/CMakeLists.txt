# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(SwApiISP)

# SW API libraries
add_subdirectory(warp-adapter)
add_subdirectory(sw-utils)
add_subdirectory(video-common)
add_subdirectory(isp-common)
add_subdirectory(ICameraProxy)
add_subdirectory(access-logging)
add_subdirectory(i2c)
add_subdirectory(3d-lut)
add_subdirectory(tpg)
add_subdirectory(tpg-ui-state)
add_subdirectory(switch)
add_subdirectory(switch-ui-state)
add_subdirectory(clipper)
add_subdirectory(demosaic)
add_subdirectory(remosaic)
add_subdirectory(dpc)
add_subdirectory(blc)
add_subdirectory(wbc)
add_subdirectory(wbs)
add_subdirectory(anr)
add_subdirectory(ccm)
add_subdirectory(csc)
add_subdirectory(crs)
add_subdirectory(bls)
add_subdirectory(mixer)
add_subdirectory(ccm-coeff-gen)
add_subdirectory(vc)
add_subdirectory(vc-mesh-utils)
add_subdirectory(hs)
add_subdirectory(scaler)
add_subdirectory(tone-mapping)
add_subdirectory(1dlut)
add_subdirectory(1d-lut-lutgen)
add_subdirectory(vfr)
add_subdirectory(vfw)
add_subdirectory(vvpisp-autoexposure)
add_subdirectory(vvpisp-calibration-profile)
add_subdirectory(vvpisp-white-balance)
add_subdirectory(vvpisp-wbs-switch-router)
foreach (CAMERA_SOURCE_DIR ${CAMERA_SOURCE_DIRS})
    add_subdirectory(${CAMERA_SOURCE_DIR})
endforeach()
add_subdirectory(video-throttle)

add_custom_command(
    OUTPUT  dummy.cpp
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

set(all_files dummy.cpp)

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} PUBLIC SwApi3dLut)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiI2C)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiAccessLogging)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiUtils)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVideoCommon)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiIspCommon)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiWarpAdapter)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiTpg)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiTpgUiState)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiSwitch)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiSwitchUiState)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiClipper)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiDemosaic)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiRemosaic)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiDpc)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiBlc)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiWbs)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiWbc)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiAnr)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiCsc)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiCrs)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiBls)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiMixer)
target_link_libraries(${PROJECT_NAME} PUBLIC CcmCoeffGen)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVc)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVcMeshUtils)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiHs)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiTmo)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiLut1D)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiLut1dUtils)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVfr)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVfw)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiCcm)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiScaler)
foreach (CAMERA_LIB ${CAMERA_LIBS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CAMERA_LIB}Proxy)
endforeach()

target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVvpIspAutoExposure)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVvpIspCalibrationProfile)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVvpIspWhiteBalance)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVvpIspSwitchRouter)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiVideoThrottle)

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)
