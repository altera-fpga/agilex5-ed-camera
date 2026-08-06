###################################################################################
# Copyright (C) 2025 Altera Corporation
#
# This software and the related documents are Altera copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Altera's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the License.
###################################################################################

# create script specific parameters and default values

set_shell_parameter STAP_FILENAME     ""

set_shell_parameter HPS_INIT          {HPS FIRST}

set_shell_parameter FITTER_SEED       {1}
set_shell_parameter OPTIMIZATION_MODE {0}
set_shell_parameter SYNTH_WYSIWYG     {0}

# define the procedures used by the create .tcl script

proc pre_creation_step {} {
    transfer_files
    evaluate_terp
}

proc transfer_files {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_script_path       [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_stap_filename     [get_shell_parameter STAP_FILENAME]
    set v_hps_init          [get_shell_parameter HPS_INIT]

    file_copy   ${v_script_path}/isp_system.qsf.terp  ${v_project_path}/quartus/user/isp_system.qsf.terp
    file_copy   ${v_script_path}/isp_system.sdc.terp  ${v_project_path}/sdc/user/isp_system.sdc

    if {${v_stap_filename} != ""} {
        file_copy   ${v_script_path}/${v_stap_filename}      ${v_project_path}/quartus
    }

    # Copy supporting HPS boot file
    file mkdir  ${v_project_path}/scripts/ext

    if {${v_hps_init} == "AFTER INIT_DONE"} {
        # FPGA First bootloader
        file_copy   ${v_script_path}/../boot_files/u-boot-spl-dtb_ff.hex \
                                                ${v_project_path}/scripts/ext/u-boot-spl-dtb.hex
    } else {
        # HPS First bootloader
        file_copy   ${v_script_path}/../boot_files/u-boot-spl-dtb.hex \
                                                ${v_project_path}/scripts/ext/u-boot-spl-dtb.hex
    }

    file_copy   ${v_script_path}/isp_system_da_drc.dawf  ${v_project_path}/quartus/da_drc.dawf
}

proc evaluate_terp {} {
    set v_project_path  [get_shell_parameter PROJECT_PATH]
    set v_stap_filename [get_shell_parameter STAP_FILENAME]
    set v_seed          [get_shell_parameter FITTER_SEED]
    set v_optim_mode    [get_shell_parameter OPTIMIZATION_MODE]
    set v_wysiwyg_en    [get_shell_parameter SYNTH_WYSIWYG]

    if {${v_stap_filename} == ""} {
        set v_stap_enabled 0
    } else {
        set v_stap_enabled 1
    }

    evaluate_terp_file  ${v_project_path}/quartus/user/isp_system.qsf.terp \
                        [list ${v_stap_filename} ${v_stap_enabled} ${v_optim_mode} ${v_wysiwyg_en} ${v_seed}] 0 1
}
