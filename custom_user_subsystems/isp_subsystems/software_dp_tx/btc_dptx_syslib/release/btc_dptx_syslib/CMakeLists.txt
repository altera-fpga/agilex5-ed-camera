# Copyright 2025 Altera Corporation.
#
# This software and the related documents are Altera copyrighted materials,
# and your use of them is governed by the express license under which they
# were provided to you ("License"). Unless the License provides otherwise,
# you may not use, modify, copy, publish, distribute, disclose or transmit
# this software or the related documents without Altera's prior written
# permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

project(btc_dptx_syslib)

# Add the library as an import instead of compiling from source
add_library(${PROJECT_NAME} STATIC IMPORTED GLOBAL)


# Set the path to the precompiled library file
set_target_properties(${PROJECT_NAME}  PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib${PROJECT_NAME}.a")

# Specify the include directories for the library
target_include_directories(${PROJECT_NAME}  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/../.."
)