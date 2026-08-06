# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

project(VvpIspDemo)

set(header_files "")
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/VvpIspDemo.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/IFrameCapture.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/ILogoControl.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/IUIConnection.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/CoreIspPipeline.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/UdxVvpIspPipeline.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/TabbedUiControls.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineFlags.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/CompileTimeConfiguration.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/TaskHandler.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/Screensaver.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/VideoPipeline/PipelineCore.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/system.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src/dptx_formats.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src/dptx_app_defs.h)
set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src/intel_axi2cv_modes.h)

set(source_files "")
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/VvpIspDemo.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/CoreIspPipeline.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/UdxVvpIspPipeline.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/TabbedUiControls.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/CommandLineFlags.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/CompileTimeConfiguration.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/TaskHandler.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/VideoPipeline/PipelineCore.cpp)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src/dptx_formats.c)
set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp)

if(ISP_AI_BUILD OR CPACK_ONLY)
    set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/AiPipeline.h)
    set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/AiTabbedUiControls.h)
    set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/AiPipeline.cpp)
    set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/AiTabbedUiControls.cpp)
endif()

if(ISP_STITCH_BUILD OR CPACK_ONLY)
    set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/StitchPipeline.h)
    set(header_files ${header_files} ${CMAKE_CURRENT_SOURCE_DIR}/StitchTabbedUiControls.h)
    set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/StitchPipeline.cpp)
    set(source_files ${source_files} ${CMAKE_CURRENT_SOURCE_DIR}/StitchTabbedUiControls.cpp)
endif()

set(image_files "")
set(image_files ${image_files} ${CMAKE_CURRENT_SOURCE_DIR}/images/altera-144.bmp)

set(all_files ${header_files} ${source_files} ${image_files})

foreach( image_file ${image_files} )
    FILE(COPY ${image_file} DESTINATION ${OUTPUT_ROOT}/${BIN_FOLDER}/${CMAKE_BUILD_TYPE}/images)
endforeach()
install(FILES ${image_files} DESTINATION "images" COMPONENT deploy)

add_executable(${PROJECT_NAME} ${all_files} ${generated_files})

set(interface_header_files "")
set(interface_header_files ${interface_header_files} ${CMAKE_CURRENT_SOURCE_DIR}/IFrameCapture.h)
set(interface_header_files ${interface_header_files} ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src/dptx_formats.h)

add_library(${PROJECT_NAME}Interface INTERFACE ${interface_header_files})
target_include_directories(${PROJECT_NAME}Interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME}Interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/VideoPipeline)
target_include_directories(${PROJECT_NAME}Interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/dp_tx_app_src)
target_link_libraries(${PROJECT_NAME}Interface INTERFACE SwApiVfw)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/VideoPipeline)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/dp_tx_app_src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE "${common_compile_options}")
set(mixer_definitions "")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(mixer_definitions "MIXER_DEBUG")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE ${common_compile_definitions} ${CAMERA_DEFINITIONS} ${mixer_definitions})

if(SUPPORT_DMA)
target_compile_definitions(${PROJECT_NAME} PRIVATE SUPPORT_DMA)
endif()

if(ISP_AI_BUILD)
target_compile_definitions(${PROJECT_NAME} PRIVATE ISP_AI_BUILD)
target_compile_definitions(${PROJECT_NAME} PRIVATE MAX_30_FPS)
endif()

if(ISP_STITCH_BUILD)
target_compile_definitions(${PROJECT_NAME} PRIVATE ISP_STITCH_BUILD)
endif()


#region Subdirectories
add_subdirectory(ui)
add_subdirectory(helpers)
#endregion

#region Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC Logging)
target_link_libraries(${PROJECT_NAME} PUBLIC ApplicationToolkit)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiISP)
target_link_libraries(${PROJECT_NAME} PUBLIC HapiUiISP)
target_link_libraries(${PROJECT_NAME} PUBLIC SwApiISP)


# ui
target_link_libraries(${PROJECT_NAME} PUBLIC LogoControls)
target_link_libraries(${PROJECT_NAME} PUBLIC MultiChannelAutoExposureControls)
target_link_libraries(${PROJECT_NAME} PUBLIC MultiChannelWhiteBalanceControls)
target_link_libraries(${PROJECT_NAME} PUBLIC OutputControls)
target_link_libraries(${PROJECT_NAME} PUBLIC ProfileBlackLevelControls)
target_link_libraries(${PROJECT_NAME} PUBLIC StitchControls)
target_link_libraries(${PROJECT_NAME} PUBLIC VfwControls)
target_link_libraries(${PROJECT_NAME} PUBLIC IspVfrInputControls)
if(ISP_AI_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC AIRuntimeControls)
endif()

# helpers
target_link_libraries(${PROJECT_NAME} PUBLIC drmHelper)
target_link_libraries(${PROJECT_NAME} PUBLIC MultiChannelAutoExposure)
target_link_libraries(${PROJECT_NAME} PUBLIC MultiChannelWhiteBalance)
target_link_libraries(${PROJECT_NAME} PUBLIC IspMixerHelper)
target_link_libraries(${PROJECT_NAME} PUBLIC lvglLogoHelper)
target_link_libraries(${PROJECT_NAME} PUBLIC ROIHelper)
target_link_libraries(${PROJECT_NAME} PUBLIC IspVfrInput)

# AI
if(ISP_AI_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC AI_ISP)
endif()

#Stitch
if(ISP_STITCH_BUILD)
    target_link_libraries(${PROJECT_NAME} PUBLIC StitchAdapter)
endif()

#endregion

install(TARGETS ${PROJECT_NAME} DESTINATION . COMPONENT deploy)

include(cmake/cpack.cmake OPTIONAL)
