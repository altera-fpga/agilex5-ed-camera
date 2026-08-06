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

add_subdirectory(../build/bsp bsp)

include(../build/bsp/toolchain.cmake)

project(app)

add_executable(app.elf)

# Header files
set(header_files )
list(APPEND header_files dla_registers.h)
list(APPEND header_files stream_controller.h)
list(APPEND header_files stream_controller_messages.h)
list(APPEND header_files stream_controller_userMessages.h)

# Source files
set(source_files )
list(APPEND source_files stream_controller.c)

source_group(include FILES ${header_files})
source_group(src FILES ${source_files})

set(all_files ${header_files} ${source_files})

target_sources(app.elf
    PRIVATE
        ${source_files}
    PUBLIC
)


target_include_directories(app.elf
    PRIVATE
        .
    PUBLIC
)

target_link_libraries(app.elf
    PRIVATE
        -T "${BspLinkerScript}" -nostdlib
        "${ExtraArchiveLibraries}"
        -Wl,--start-group "${BspLibraryName}" -lc -lstdc++ -lgcc -lm -Wl,--end-group
)
