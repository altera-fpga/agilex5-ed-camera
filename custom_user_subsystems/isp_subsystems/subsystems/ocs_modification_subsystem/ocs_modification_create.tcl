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

proc post_creation_step {} {
    modify_manual_ocs_rom
}

# resolve interdependencies
proc derive_parameters {param_array} {
    upvar $param_array p_array

    set_shell_parameter DRV_OCS_SUBSYSTEM_NAME ""

    # look for OCS subsystem
    for {set id 0} {$id < $p_array(project,id)} {incr id} {
        if {$p_array($id,type) == "ocs"} {

            set params $p_array($id,params)

            foreach v_pair ${params} {
                set v_name  [lindex ${v_pair} 0]
                set v_value [lindex ${v_pair} 1]

                if {${v_name} == "INSTANCE_NAME"} {
                    set_shell_parameter   DRV_OCS_SUBSYSTEM_NAME  ${v_value}
                    break
                }
            }
        }
    }
}

proc modify_manual_ocs_rom {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_drv_ocs_subsystem_name    [get_shell_parameter DRV_OCS_SUBSYSTEM_NAME]

    if {${v_drv_ocs_subsystem_name} != ""} {
        load_system ${v_project_path}/rtl/shell/${v_drv_ocs_subsystem_name}.qsys

        load_component  hps_intel_offset_capability_manual

        set v_num_caps    [get_component_parameter_value    C_NUM_CAPS]

        # check if the capability contains a dummy entry (required for ip instantiation)
        if {${v_num_caps} == 1} {
            set v_cap0_type   [get_component_parameter_value    C_CAP0_TYPE]

            if {${v_cap0_type} == 0} {
                set v_num_caps    0
            }
        }

        set_component_parameter_value   C_NUM_CAPS  [expr ${v_num_caps} + 1]

        set v_index   ${v_num_caps}

        # manual address span extender - DMA HPS to FPGA
        set_component_parameter_value      C_CAP${v_index}_BASE         {0x04800a00}
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {530}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT {0}

        save_component

        sync_sysinfo_parameters
        save_system
    }
}
