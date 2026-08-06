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
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
if(${CMAKE_VERSION} VERSION_GREATER "3.24.0")
    cmake_policy(SET CMP0135 NEW) 
endif()
if(${CMAKE_VERSION} VERSION_GREATER "3.31.0")
    cmake_policy(SET CMP0177 NEW) 
endif()

set(MACHINE agilex5_modular CACHE STRING "target machine")

project(VvpIspExampleDesign
    VERSION 0.0.1
    DESCRIPTION "Example design for the Video and Vision Processing (VVP) Image Signal Processing (ISP) library."
    HOMEPAGE_URL "https://github.com/intel-innersource/applications.fpga.reference-designs.projects.vvp-isp"
    LANGUAGES CXX C)

# poky toolchain sets some arguments which we need to control
set(CMAKE_CXX_COMPILER_ARG1 "-march=armv8.2-a+fp16" CACHE STRING "Arguments to CXX compiler" FORCE)
set(CMAKE_C_COMPILER_ARG1 "-march=armv8.2-a+fp16" CACHE STRING "Arguments to CXX compiler" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(OUTPUT_ROOT ${CMAKE_BINARY_DIR}) 
set(BIN_FOLDER "bin")

option(ISP_AI_BUILD "Build ISP + AI application" OFF)
option(ISP_STITCH_BUILD "Build ISP + Stitch application" OFF)
if(ISP_AI_BUILD AND ISP_STITCH_BUILD)
    message(FATAL_ERROR "ISP_AI_BUILD and ISP_STITCH_BUILD options are mutually exclusive.")
endif()

option(SUPPORT_DMA "Support DMA transfers" ON)

# Need to set this to ON so MSGDMA transactions use singleton correctly
set(SW_API_UTILS_MEM_SHARED_LIBRARY ON CACHE BOOL "Build SwApiUtils as a shared library" FORCE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(INSTALL_PREFIX ${OUTPUT_ROOT}/${BIN_FOLDER}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_ROOT}/${BIN_FOLDER}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_ROOT}/${BIN_FOLDER}/${CMAKE_BUILD_TYPE}/static_lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_ROOT}/${BIN_FOLDER}/${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_RPATH "./lib:.")
set(CMAKE_INSTALL_RPATH "./lib:.")

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(INSTALL_PREFIX ${CMAKE_BINARY_DIR})

# Board specific options
set(CAMERA_SOURCE_DIRS "")
set(CAMERA_LIBS "")
set(CAMERA_DEFINITIONS "")
set(AWB_PROFILE "")
include(cmake/boards/${MACHINE}.cmake OPTIONAL)

add_custom_target(${PROJECT_NAME} ALL)

# Copy asstes
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${AWB_PROFILE})
    set(AWB_PROFILE_FULL ${CMAKE_CURRENT_SOURCE_DIR}/${AWB_PROFILE})
else()
    set(AWB_PROFILE_FULL ${CMAKE_CURRENT_SOURCE_DIR}/yocto/meta-vvp-isp-demo/recipes-apps/vvp-isp-demo/files/${AWB_PROFILE})
endif()
install(FILES ${AWB_PROFILE_FULL} DESTINATION "." RENAME "awb_profile.json" COMPONENT deploy)

add_custom_command(OUTPUT ${INSTALL_PREFIX}/awb_profile.json
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_PREFIX}
    COMMAND ${CMAKE_COMMAND} -E copy ${AWB_PROFILE_FULL} ${INSTALL_PREFIX}/awb_profile.json
    DEPENDS ${AWB_PROFILE_FULL})

set(assets )
list(APPEND assets ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md)
list(APPEND assets ${CMAKE_CURRENT_SOURCE_DIR}/SECURITY.md)
list(APPEND all_files ${assets})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md DESTINATION "." COMPONENT YOCTO_SRC_DELIVERY)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/SECURITY.md DESTINATION "." COMPONENT YOCTO_SRC_DELIVERY)

set(assets_out ${INSTALL_PREFIX}/awb_profile.json)
foreach(asset ${assets})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${INSTALL_PREFIX} asset_out ${asset})
    get_filename_component(asset_out_dir ${asset_out} DIRECTORY)
    add_custom_command(OUTPUT ${asset_out}
                       COMMAND ${CMAKE_COMMAND} -E make_directory ${asset_out_dir}
                       COMMAND ${CMAKE_COMMAND} -E copy ${asset}
                                                        ${asset_out}
                       DEPENDS ${asset})
    list(APPEND assets_out ${asset_out})

    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "." asset_out ${asset})
    get_filename_component(asset_out_dir ${asset_out} DIRECTORY)
    install(FILES ${asset_out} DESTINATION ${asset_out_dir} COMPONENT deploy)
endforeach()
add_custom_target(${PROJECT_NAME}-copy-assets DEPENDS ${assets_out})

# dependencies
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-copy-assets)

#region Pulling in general CMake scripts
include(cmake/CopyCompileCommands.cmake OPTIONAL)
include(cmake/GitVersionControlInfo.cmake OPTIONAL)
include(cmake/OutOfSourceBuilds.cmake OPTIONAL)
include(cmake/DefaultBuildTypeDebug.cmake OPTIONAL)
set(DOXYGEN_EXCLUDE_PATTERNS "*/build/*" "*.test.cpp")
include(cmake/DoxygenDocumentationGeneration.cmake OPTIONAL)
# Since LTO increases link-time, it is disabled by default.
include(cmake/LinkTimeInterproceduralOptimisation.cmake OPTIONAL)
include(cmake/Sanitizers.cmake OPTIONAL)
#endregion
include(cmake/${MACHINE}.cmake OPTIONAL)

#region Compiler and linker flags

# Applies compiler and linker settings required by SDL ST-411.
# See: https://altera-corp.atlassian.net/wiki/spaces/PSGPMO/pages/67179722/ST-411+-+Compiler+Flags+and+Code+signing

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -pipe -Wno-psabi -U_FORTIFY_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -pipe -Wno-psabi -U_FORTIFY_SOURCE")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O3 -flto -funroll-all-loops -fpeel-loops -ftracer -ftree-vectorize -pie -fPIE -fPIC -fstack-protector-all -fstack-protector-strong -D_FORTIFY_SOURCE=2 -pipe -Wformat -Wformat-security -Werror=format-security -fstack-clash-protection -feliminate-unused-debug-types -Wno-psabi -Wall")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O3 -flto -funroll-all-loops -fpeel-loops -ftracer -ftree-vectorize -pie -fPIE -fPIC -fstack-protector-all -fstack-protector-strong -D_FORTIFY_SOURCE=2 -pipe -Wformat -Wformat-security -Werror=format-security -fstack-clash-protection -feliminate-unused-debug-types -Wno-psabi -Wall")

# set RPATH not RUNPATH so it is inherited
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--disable-new-dtags")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--disable-new-dtags")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(common_compile_options "")
set(common_compile_definitions "")
set(common_link_options "")

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(common_link_options ${common_link_options} -pie)
    set(common_link_options ${common_link_options} -z,relro)
    set(common_link_options ${common_link_options} -z,now)
    set(common_link_options ${common_link_options} -z noexecstack)
    set(common_compile_options ${common_compile_options} -fstack-protector-strong)
    set(common_compile_options ${common_compile_options} -D_FORTIFY_SOURCE=2)
    set(common_compile_options ${common_compile_options} -O)
    set(common_compile_options ${common_compile_options} -Wformat -Wformat-security)
endif()

#endregion

option(STATIC_VIDEO_LIBRARY "Statically linked sw api library" ON)

set(WEBTOOLKIT_MINIFY_ASSETS OFF CACHE BOOL "Minify WebToolkit JS/CSS assets before packing" FORCE)
set(WEBTOOLKIT_PYTHON_PACKER ON CACHE BOOL "Use WebToolkit python FilePacker" FORCE)

if(MACHINE STREQUAL "agilex5_terasic_de25s" OR MACHINE STREQUAL "agilex5_terasic_de25_nano")
    set(VENDOR "agilex5_terasic")
else()
    set(VENDOR ${MACHINE})
endif()

option(ENABLE_TESTING "Enable tests" OFF)
if(${ENABLE_TESTING})
    add_subdirectory(test)
    message(TRACE "Enabled tests.")
endif()

# Application overlay source
set(APP_OVERLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/html/vvp-isp-overlay)

# Vendor overlay source (choose your preferred layout)
set(VENDOR_OVERLAY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/html/${VENDOR})

# Merged overlay staging dir in build tree
# Application toolkit will use this variable to know where to get
# Web UI overlay files
set(CLIENT_OVERLAY_DIR ${CMAKE_BINARY_DIR}/vvp-isp-overlay-merged)


# Optional: ask CMake to reconfigure when overlay files are added/removed
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${APP_OVERLAY_DIR}/*"
    "${VENDOR_OVERLAY_DIR}/*"
)

# Rebuild merged overlay at configure time
file(REMOVE_RECURSE ${CLIENT_OVERLAY_DIR})
file(MAKE_DIRECTORY ${CLIENT_OVERLAY_DIR})

# Layer 1: ISP overlay
if(EXISTS "${APP_OVERLAY_DIR}")
    file(COPY "${APP_OVERLAY_DIR}/" DESTINATION "${CLIENT_OVERLAY_DIR}")
endif()

# Layer 2: Vendor specific overlay (applied on top)
if(EXISTS "${VENDOR_OVERLAY_DIR}")
    file(COPY "${VENDOR_OVERLAY_DIR}/" DESTINATION "${CLIENT_OVERLAY_DIR}")
endif()

#region Subdirectories

# Application toolkit needs to be added first
add_subdirectory(application-toolkit)

# Application
add_subdirectory(src)

# HAPI libraries
add_subdirectory(hapi)

# HAPI UI libraries
add_subdirectory(hapi-ui)

# SW API libraries
add_subdirectory(sw-api)

if(ISP_AI_BUILD OR CPACK_ONLY)
    # AI
    add_subdirectory(src-ai)
endif()

#endregion

include(cmake/cpack.cmake OPTIONAL)

add_custom_target(
    install_deploy
    DEPENDS all
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR} --component deploy
)
