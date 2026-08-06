# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.16)
if(${CMAKE_VERSION} VERSION_GREATER "3.24.0")
	cmake_policy(SET CMP0135 NEW) 
endif()
project(Yolov8nNetworks
    VERSION 0.0.1
    DESCRIPTION "YOLO v8 nano network compolation for Example design.")

set(ARCH_FILES_M2M)
list(APPEND ARCH_FILES_M2M ${CMAKE_CURRENT_SOURCE_DIR}/arch/generated_arch.arch)

if("${INSTALL_PREFIX}" STREQUAL "")
    set(INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/output)
endif()
if (NOT CPACK_ONLY)
    if("${OPENVINO_SETUPVARS}" STREQUAL "")
        include(cmake/OpenVINO.cmake)
    endif()
    if("${DLA_SETUPVARS}" STREQUAL "")
        include(cmake/CoreDLA.cmake)
    endif()
endif()

set(RESOLUTION_384_224 384 224)
set(RESOLUTION_416_256 416 256)
set(RESOLUTION_512_288 512 288)
set(RESOLUTION_576_352 576 352)
set(RESOLUTION_608_352 608 352)
set(RESOLUTION_640_384 640 384)

set(RESOLUTIONS)
#list(APPEND RESOLUTIONS RESOLUTION_384_224)
#list(APPEND RESOLUTIONS RESOLUTION_416_256)
#list(APPEND RESOLUTIONS RESOLUTION_512_288)
#list(APPEND RESOLUTIONS RESOLUTION_576_352)
#list(APPEND RESOLUTIONS RESOLUTION_608_352)
list(APPEND RESOLUTIONS RESOLUTION_640_384)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup_venv.sh.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/setup_venv.sh @ONLY FILE_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/convert_yolo_to_onnx.sh.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/convert_yolo_to_onnx.sh @ONLY FILE_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/onnx_to_openvino_ir.sh.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/onnx_to_openvino_ir.sh @ONLY FILE_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile.sh.in ${CMAKE_CURRENT_BINARY_DIR}/scripts/compile.sh @ONLY FILE_PERMISSIONS  OWNER_READ OWNER_WRITE OWNER_EXECUTE)
file(GLOB script_files ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*)

set(GENERATED_FILES)
set(OPTIMISE_FOR_NETWORK ${CMAKE_CURRENT_BINARY_DIR}/yolov8n_ir_640_384.xml)

if(DEFINED HTTP_PROXY)
    set(HTTPS_PROXY_STR https_proxy=${HTTP_PROXY})
else()
    set(HTTPS_PROXY_STR dummy=dummy)
endif()

set(all_files ${script_files})
foreach(ARCH_FILE_M2M ${ARCH_FILES_M2M})
    list(APPEND all_files ${ARCH_FILE_M2M})
endforeach()
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/ArchitectureOptimisation.md)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/CompileStepsWalkthrough.md)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/yolov8n_onnx.py)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLA.cmake)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoreDLAPatchCommand.sh)
list(APPEND all_files ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OpenVINO.cmake)
list(APPEND all_files )

set(cnn_delivery_files ${all_files})

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/python_env/bin/activate
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/scripts/setup_venv.sh
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/setup_venv.sh ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yolov8n.pt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yolov8n.pt ${CMAKE_CURRENT_BINARY_DIR}/yolov8n.pt COPYONLY)
    set(YOLO_DETECT True)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yolov8n-pose.pt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yolov8n-pose.pt ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose.pt COPYONLY)
    set(YOLO_POSE True)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yolo_cnn/yolov8n.pt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yolo_cnn/yolov8n.pt ${CMAKE_CURRENT_BINARY_DIR}/yolov8n.pt COPYONLY)
    set(YOLO_DETECT True)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/yolo_cnn/yolov8n-pose.pt)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/yolo_cnn/yolov8n-pose.pt ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose.pt COPYONLY)
    set(YOLO_POSE True)
endif()
include(cmake/ultralytics.cmake OPTIONAL)

set(CATEGORIES )
if(YOLO_DETECT)
    list(APPEND CATEGORIES ${INSTALL_PREFIX}/yolov8n_categories.txt)
    list(APPEND GENERATED_FILES ${INSTALL_PREFIX}/yolov8n_categories.txt)
    install(FILES ${INSTALL_PREFIX}/yolov8n_categories.txt DESTINATION "." COMPONENT deploy)
endif()
if(YOLO_POSE)
    list(APPEND CATEGORIES ${INSTALL_PREFIX}/yolov8n-pose_categories.txt)
    list(APPEND GENERATED_FILES ${INSTALL_PREFIX}/yolov8n-pose_categories.txt)
    install(FILES ${INSTALL_PREFIX}/yolov8n-pose_categories.txt DESTINATION "." COMPONENT deploy)
endif()

foreach(RESOLUTION_NAME ${RESOLUTIONS})
    set(RESOLUTION )
    list(APPEND RESOLUTION ${${RESOLUTION_NAME}})

    list(JOIN RESOLUTION "_" RESOLUTION_STR )

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/yolov8n_short_${RESOLUTION_STR}.onnx ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose_short_${RESOLUTION_STR}.onnx ${CATEGORIES} 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/scripts/convert_yolo_to_onnx.sh ${RESOLUTION}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/convert_yolo_to_onnx.sh ${CMAKE_CURRENT_BINARY_DIR}/python_env/bin/activate ${CMAKE_CURRENT_SOURCE_DIR}/yolov8n_onnx.py ${CMAKE_CURRENT_BINARY_DIR}/yolov8n.pt ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose.pt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        USES_TERMINAL
    )
    set(CATEGORIES )

    if(YOLO_DETECT)
        add_custom_command(
            OUTPUT yolov8n_ir_${RESOLUTION_STR}.xml
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/scripts/onnx_to_openvino_ir.sh "yolov8n" "${RESOLUTION_STR}"
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/onnx_to_openvino_ir.sh ${CMAKE_CURRENT_BINARY_DIR}/yolov8n_short_${RESOLUTION_STR}.onnx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        foreach(ARCH_FILE_M2M ${ARCH_FILES_M2M})
            get_filename_component(ARCH ${ARCH_FILE_M2M} NAME)
            add_custom_command(
                OUTPUT ${INSTALL_PREFIX}/${ARCH}/yolov8n_dla_m2m_compiled_${RESOLUTION_STR}.bin
                COMMAND bash -c "${CMAKE_CURRENT_BINARY_DIR}/scripts/compile.sh ${ARCH_FILE_M2M} \"yolov8n\" \"${RESOLUTION_STR}\""
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/compile.sh ${CMAKE_CURRENT_BINARY_DIR}/yolov8n_ir_${RESOLUTION_STR}.xml ${ARCH_FILE_M2M}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
            list(APPEND GENERATED_FILES ${INSTALL_PREFIX}/${ARCH}/yolov8n_dla_m2m_compiled_${RESOLUTION_STR}.bin)
            install(FILES ${INSTALL_PREFIX}/${ARCH}/yolov8n_dla_m2m_compiled_${RESOLUTION_STR}.bin DESTINATION ${ARCH} COMPONENT deploy)
        endforeach()
    endif()

    if(YOLO_POSE)
        add_custom_command(
            OUTPUT yolov8n-pose_ir_${RESOLUTION_STR}.xml
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/scripts/onnx_to_openvino_ir.sh "yolov8n-pose" "${RESOLUTION_STR}"
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/onnx_to_openvino_ir.sh ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose_short_${RESOLUTION_STR}.onnx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        foreach(ARCH_FILE_M2M ${ARCH_FILES_M2M})
            get_filename_component(ARCH ${ARCH_FILE_M2M} NAME)
            add_custom_command(
                OUTPUT ${INSTALL_PREFIX}/${ARCH}/yolov8n-pose_dla_m2m_compiled_${RESOLUTION_STR}.bin
                COMMAND bash -c "${CMAKE_CURRENT_BINARY_DIR}/scripts/compile.sh ${ARCH_FILE_M2M} \"yolov8n-pose\" \"${RESOLUTION_STR}\""
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/scripts/compile.sh ${CMAKE_CURRENT_BINARY_DIR}/yolov8n-pose_ir_${RESOLUTION_STR}.xml ${ARCH_FILE_M2M}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )
            list(APPEND GENERATED_FILES ${INSTALL_PREFIX}/${ARCH}/yolov8n-pose_dla_m2m_compiled_${RESOLUTION_STR}.bin)
            install(FILES ${INSTALL_PREFIX}/${ARCH}/yolov8n-pose_dla_m2m_compiled_${RESOLUTION_STR}.bin DESTINATION ${ARCH} COMPONENT deploy)
        endforeach()
    endif()

endforeach()

add_custom_target(yolov8n ALL
    DEPENDS ${GENERATED_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack.cmake OPTIONAL)
