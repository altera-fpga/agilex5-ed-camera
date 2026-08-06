# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.14)

add_subdirectory(../build/bsp bsp)
add_subdirectory(btc_dptx_syslib/release_mst/btc_dptx_syslib btc_dptx_syslib)
add_subdirectory(btc_dptxll_syslib/release/btc_dptxll_syslib btc_dptxll_syslib)

include(../build/bsp/toolchain.cmake)

project(app)

enable_language(ASM)
enable_language(C)
enable_language(CXX)

add_executable(app.elf)

target_sources(app.elf
    PRIVATE
        dp_tx_app_src/dptx_app_defs.h
        dp_tx_app_src/tx_utils.c
        dp_tx_app_src/intel_axi2cv.c
        dp_tx_app_src/intel_fpga_i2c.h
        dp_tx_app_src/config.h
        dp_tx_app_src/board.c
        dp_tx_app_src/intel_vab_core.c
        dp_tx_app_src/dptx_formats.c
        dp_tx_app_src/aux_decoder.c
        dp_tx_app_src/debug.h
        dp_tx_app_src/intel_fpga_i2c.c
        dp_tx_app_src/board.h
        dp_tx_app_src/intel_vab_core_regs.h
        dp_tx_app_src/intel_vab_core_io.h
        dp_tx_app_src/aux_decoder.h
        dp_tx_app_src/main.c
        dp_tx_app_src/intel_vab_core.h
        dp_tx_app_src/dptx_formats.h
        dp_tx_app_src/tx_utils.h
        dp_tx_app_src/intel_axi2cv_regs.h
        dp_tx_app_src/intel_axi2cv.h
        dp_tx_app_src/debug.c
)

target_include_directories(app.elf
    PRIVATE
    PUBLIC
)

target_link_libraries(app.elf
    PRIVATE
        -T "${BspLinkerScript}" -nostdlib
        btc_dptx_syslib
        btc_dptxll_syslib
        "${ExtraArchiveLibraries}"
        -Wl,--start-group "${BspLibraryName}" -lc -lstdc++ -lgcc -lm -Wl,--end-group
)

# Create objdump from ELF.
set(objdump app.elf.objdump)
add_custom_command(
    OUTPUT "${objdump}"
    DEPENDS app.elf
    COMMAND "${ToolchainObjdump}" "${ToolchainObjdumpFlags}" app.elf >
            "${objdump}"
    COMMENT "Creating ${objdump}."
    VERBATIM
)
add_custom_target(create-objdump ALL DEPENDS "${objdump}")

# Report space free for stack + heap. Note that the file below is never created
# so the report is always output on build.
set(stack_report_file app.elf.stack_report)
add_custom_command(
    OUTPUT "${stack_report_file}"
    DEPENDS app.elf
    COMMAND niosv-stack-report -p "${ToolchainPrefix}" app.elf
    COMMENT "Reporting memory available for stack + heap in app.elf."
    VERBATIM
)
add_custom_target(niosv-stack-report ALL DEPENDS "${stack_report_file}")

