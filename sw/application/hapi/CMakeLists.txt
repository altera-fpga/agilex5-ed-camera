# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(HapiISP)

add_subdirectory(hapi-core-common)
add_subdirectory(hapi-core-ocs-uio)

add_subdirectory(intel-vvp-sw-common/sw/hapi)
add_subdirectory(intel-vvp-3d-lut/sw/hapi)
add_subdirectory(intel-vvp-tone-mapping/sw/hapi)
add_subdirectory(intel-vvp-warp/sw/hapi)
add_subdirectory(intel-vvp-tpg/sw/hapi)
add_subdirectory(intel-vvp-switch/sw/hapi)
add_subdirectory(intel-vvp-clipper/sw/hapi)
add_subdirectory(intel-vvp-demosaic/sw/hapi)
add_subdirectory(intel-vvp-dpc/sw/hapi)
add_subdirectory(intel-vvp-blc/sw/hapi)
add_subdirectory(intel-vvp-wbc/sw/hapi)
add_subdirectory(intel-vvp-wbs/sw/hapi)
add_subdirectory(intel-vvp-anr/sw/hapi)
add_subdirectory(intel-vvp-bls/sw/hapi)
add_subdirectory(intel-vvp-mixer/sw/hapi)
add_subdirectory(intel-vvp-vc/sw/hapi)
add_subdirectory(intel-vvp-csc/sw/hapi)
add_subdirectory(intel-vvp-crs/sw/hapi)
add_subdirectory(intel-vvp-usm/sw/hapi)
add_subdirectory(intel-vvp-1dlut/sw/hapi)
add_subdirectory(intel-vvp-hs/sw/hapi)
add_subdirectory(intel-vvp-pip-conv/sw/hapi)
add_subdirectory(intel-vvp-vfb/sw/hapi)
add_subdirectory(intel-vvp-vfr/sw/hapi)
add_subdirectory(intel-vvp-vfw/sw/hapi)
add_subdirectory(intel-vvp-cpm/sw/hapi)
add_subdirectory(intel-addr-span-exp/sw/hapi)
add_subdirectory(intel-pio/sw/hapi)
add_subdirectory(intel-vvp-exposure-fusion/sw/hapi)
add_subdirectory(intel-vvp-snoop/sw/hapi)
add_subdirectory(intel-vvp-scaler/sw/hapi)
add_subdirectory(intel-vvp-protocol-conv/sw/hapi)
add_subdirectory(intel-coredla/sw/hapi)
add_subdirectory(intel-on-chip-mem-ii/sw/hapi)
add_subdirectory(altera-vvp-throttle/sw/hapi)

add_subdirectory(altera-vvp-alpha-channel-generator/sw/hapi)
add_subdirectory(altera-vvp-roi/sw/hapi)

add_custom_command(
    OUTPUT  dummy.cpp
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
)

set(all_files dummy.cpp)

# Target
add_library(${PROJECT_NAME} STATIC ${all_files})

target_link_libraries(${PROJECT_NAME} PUBLIC HapiCore)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiCoreOCS)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvp3dLut)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpWarp)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpToneMapping)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpTpg)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpSwitch)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpClipper)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpDemosaic)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpDpc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpBlc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpWbs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpWbc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpAnr)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpBls)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpMixer)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpCsc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpVc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpHs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpSwCommon)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpCsc)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpCrs)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpUsm)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpLut1D)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpPipConv)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpVfb)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpVfw)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpCpm)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelAddrSpanExpander)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelPio)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpExposureFusion)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiVvpSnoop)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiIntelVvpScaler)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiVvpProtocolConv)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiAlteraVvpAlphaChannelGenerator)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiAlteraVvpROI)

# Get the relative directory
file( RELATIVE_PATH dest_dir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt DESTINATION ${dest_dir} COMPONENT SRC_DELIVERY)
