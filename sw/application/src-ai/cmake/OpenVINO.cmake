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

set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
MyOpenVINOHost
OVERRIDE_FIND_PACKAGE 
URL     https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu24_2025.4.0.20398.8fdad55727d_x86_64.tgz
URL_HASH SHA256=c57bc759a04bf316d66dc42d644433cf3bd590ad640933630a0616efb380a630
)
FetchContent_MakeAvailable(MyOpenVINOHost)
set(OPENVINO_SETUPVARS ${myopenvinohost_SOURCE_DIR}/setupvars.sh)
set(OPENVINO_OVC ${myopenvinohost_SOURCE_DIR}/python/openvino/tools/ovc/ovc.py)

FetchContent_Declare(
MyOpenVINO
OVERRIDE_FIND_PACKAGE
URL     https://storage.openvinotoolkit.org/repositories/openvino/packages/2025.4/linux/openvino_toolkit_ubuntu22_2025.4.0.20398.8fdad55727d_arm64.tgz
URL_HASH SHA256=d51ed6eb38933ad5e6424debe734568b758d0fd9b9df1b21098cfb3f20d20cd3
)

FetchContent_MakeAvailable(MyOpenVINO)
set(FETCHCONTENT_QUIET TRUE)
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/lib/aarch64/libopenvino.so FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/lib/aarch64/libopenvino_ir_frontend.so.2540 FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/lib/aarch64/libopenvino_arm_cpu_plugin.so FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/3rdparty/tbb/lib/libtbb.so FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/3rdparty/tbb/lib/libtbb.so.12 FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/3rdparty/tbb/lib/libtbbmalloc.so FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
FILE(INSTALL ${myopenvino_SOURCE_DIR}/runtime/3rdparty/tbb/lib/libtbbmalloc_proxy.so FOLLOW_SYMLINK_CHAIN USE_SOURCE_PERMISSIONS DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(OpenVINO_DIR ${myopenvino_SOURCE_DIR}/runtime/cmake)
find_package(OpenVINO
    CONFIG
)

set(TBB_DIR ${myopenvino_SOURCE_DIR}/runtime/3rdparty/tbb/lib/cmake/TBB)
find_package(TBB
    REQUIRED
)

install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libopenvino.so.2025.4.0 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libopenvino.so.2540 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libopenvino_ir_frontend.so.2025.4.0 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libopenvino_ir_frontend.so.2540 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbb.so.12.13 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbb.so.12 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbbmalloc.so.2.13 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbbmalloc.so.2 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbbmalloc_proxy.so.2.13 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libtbbmalloc_proxy.so.2 DESTINATION lib COMPONENT deploy)
install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libopenvino_arm_cpu_plugin.so DESTINATION lib COMPONENT deploy)
