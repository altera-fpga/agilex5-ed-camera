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

# define the procedures used by the create_subsystems_qsys.tcl script

# ai ip architecture file (used to generate the ip)
set_shell_parameter ARCH_FILE                 ""

# ai ip csr hosts
set_shell_parameter AVMM_HOST                 {{AUTO X}}
set_shell_parameter AVMM_HOST_CLK_FREQ        {200000000.0}
set_shell_parameter AVMM_HOST_2               {{AUTO X}}
set_shell_parameter AVMM_HOST_2_CLK_FREQ      {100000000.0}

set_shell_parameter INST_ID                   {0}

set_shell_parameter IRQ_HOST                  ""
set_shell_parameter IRQ_PRIORITY              "X"

# ai ip ddr host/agent
set_shell_parameter EMIF_AGENT                {}
set_shell_parameter EMIF_AGENT_CLK_FREQ       {200000000.0}

# ai ip licensed? (1=Y, 2=N)
set_shell_parameter LICENSED                  {0}

# Subsystem Base Address
set_shell_parameter SS_BASE_ADDR              {0x00700000}

# Core controls
set_shell_parameter CORE_CLK_FREQ             {297000000.0}


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
                    set_shell_parameter DRV_OCS_SUBSYSTEM_NAME ${v_value}
                    break
                }
            }
        }
    }

    # check if the emif agent has been configured
    set v_emif_agent          [get_shell_parameter EMIF_AGENT]

    if {[llength ${v_emif_agent}] == 0} {
        send_message ERROR "ai_create: EMIF Agent not specified"
    }

}

proc pre_creation_step {} {
    transfer_files
    create_ai_ip_core
}

proc creation_step {} {
    create_ai_subsystem
}

proc post_creation_step {} {
    modify_manual_ocs_rom
    edit_top_level_qsys
    add_auto_connections
}


proc transfer_files {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_instance_name     [get_shell_parameter INSTANCE_NAME]
    set v_script_path       [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_licensed          [get_shell_parameter LICENSED]

    file_copy   ${v_script_path}/ai.qsf.terp  ${v_project_path}/quartus/user/${v_instance_name}.qsf

    if {${v_licensed} == 0} {
        file_copy   ${v_script_path}/non_qpds_ip/.  ${v_project_path}/non_qpds_ip/user/altera_ai_ip/verilog
    }
}


proc create_ai_ip_core {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_arch_file         [get_shell_parameter ARCH_FILE]
    set v_licensed          [get_shell_parameter LICENSED]

    # form the command to execute (with variable substitution)
    set     v_cmd "dla_create_ip --flow create-ip "
    append  v_cmd "--arch \"${v_arch_file}\" "
    append  v_cmd "--ip-dir ${v_project_path}/ai_ip_gen "
    if {${v_licensed}} {
        append  v_cmd "--licensed"
    } else {
        append  v_cmd "--unlicensed"
    }

    set v_result [catch {exec sh -c "${v_cmd}"} v_result_text]

    if {${v_result} != 0} {
        return -code ${v_result} ${v_result_text}
    }

    file_copy   ${v_project_path}/ai_ip_gen/.   ${v_project_path}/non_qpds_ip/user

    # tidy up
    exec rm -rf ${v_project_path}/ai_ip_gen
}


proc create_ai_subsystem {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_inst_id                   [get_shell_parameter INST_ID]
    set v_emif_agent_clk_freq       [get_shell_parameter EMIF_AGENT_CLK_FREQ]
    set v_arch_file                 [get_shell_parameter ARCH_FILE]
    set v_avmm_host_clk_freq        [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_avmm_host_2_clk_freq      [get_shell_parameter AVMM_HOST_2_CLK_FREQ]
    set v_core_clk_freq             [get_shell_parameter CORE_CLK_FREQ]

    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  ai_core_clk       altera_clock_bridge
    add_instance  ai_core_rst       altera_reset_bridge
    add_instance  ai_hps_clk        altera_clock_bridge
    add_instance  ai_hps_rst        altera_reset_bridge
    add_instance  ai_hps_mm         mm_ccb
    add_instance  ai_nios_clk       altera_clock_bridge
    add_instance  ai_nios_rst       altera_reset_bridge
    add_instance  ai_nios_mm        mm_ccb
    add_instance  ai_ddr_clk        altera_clock_bridge
    add_instance  ai_ddr_rst        altera_reset_bridge
    add_instance  ai_ddr_se         altera_address_span_extender
    add_instance  ai_mm_bridge      altera_avalon_mm_bridge
    add_instance  ai_ip_core        altera_ai_ip


    ############################
    #### Set Parameters     ####
    ############################

    # ai_core_clk
    set_instance_parameter_value  ai_core_clk       EXPLICIT_CLOCK_RATE         ${v_core_clk_freq}
    set_instance_parameter_value  ai_core_clk       NUM_CLOCK_OUTPUTS           {1}

    # ai_core_rst
    set_instance_parameter_value  ai_core_rst       ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value  ai_core_rst       SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value  ai_core_rst       NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value  ai_core_rst       USE_RESET_REQUEST           {0}
    set_instance_parameter_value  ai_core_rst       SYNC_RESET                  {0}

    # ai_hps_clk
    set_instance_parameter_value  ai_hps_clk        EXPLICIT_CLOCK_RATE         ${v_avmm_host_clk_freq}
    set_instance_parameter_value  ai_hps_clk        NUM_CLOCK_OUTPUTS           {1}

    # ai_hps_rst
    set_instance_parameter_value  ai_hps_rst        ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value  ai_hps_rst        SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value  ai_hps_rst        NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value  ai_hps_rst        USE_RESET_REQUEST           {0}
    set_instance_parameter_value  ai_hps_rst        SYNC_RESET                  {0}

    # ai_nios_clk
    set_instance_parameter_value  ai_nios_clk       EXPLICIT_CLOCK_RATE         ${v_avmm_host_2_clk_freq}
    set_instance_parameter_value  ai_nios_clk       NUM_CLOCK_OUTPUTS           {1}

    # ai_nios_rst
    set_instance_parameter_value  ai_nios_rst       ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value  ai_nios_rst       SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value  ai_nios_rst       NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value  ai_nios_rst       USE_RESET_REQUEST           {0}
    set_instance_parameter_value  ai_nios_rst       SYNC_RESET                  {0}

    # ai_ddr_clk
    set_instance_parameter_value  ai_ddr_clk        EXPLICIT_CLOCK_RATE         ${v_emif_agent_clk_freq}
    set_instance_parameter_value  ai_ddr_clk        NUM_CLOCK_OUTPUTS           {1}

    # ai_ddr_rst
    set_instance_parameter_value  ai_ddr_rst        ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value  ai_ddr_rst        SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value  ai_ddr_rst        NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value  ai_ddr_rst        USE_RESET_REQUEST           {0}
    set_instance_parameter_value  ai_ddr_rst        SYNC_RESET                  {0}

    # ai_hps_mm
    set_instance_parameter_value  ai_hps_mm         ADDRESS_UNITS               {SYMBOLS}
    set_instance_parameter_value  ai_hps_mm         ADDRESS_WIDTH               {16}
    set_instance_parameter_value  ai_hps_mm         COMMAND_FIFO_DEPTH          {4}
    set_instance_parameter_value  ai_hps_mm         DATA_WIDTH                  {32}
    set_instance_parameter_value  ai_hps_mm         MASTER_SYNC_DEPTH           {2}
    set_instance_parameter_value  ai_hps_mm         MAX_BURST_SIZE              {1}
    set_instance_parameter_value  ai_hps_mm         RESPONSE_FIFO_DEPTH         {4}
    set_instance_parameter_value  ai_hps_mm         SLAVE_SYNC_DEPTH            {2}
    set_instance_parameter_value  ai_hps_mm         SYMBOL_WIDTH                {8}
    set_instance_parameter_value  ai_hps_mm         SYNC_RESET                  {0}
    set_instance_parameter_value  ai_hps_mm         USE_AUTO_ADDRESS_WIDTH      {1}

    # ai_nios_mm
    set_instance_parameter_value  ai_nios_mm        ADDRESS_UNITS               {SYMBOLS}
    set_instance_parameter_value  ai_nios_mm        ADDRESS_WIDTH               {16}
    set_instance_parameter_value  ai_nios_mm        COMMAND_FIFO_DEPTH          {4}
    set_instance_parameter_value  ai_nios_mm        DATA_WIDTH                  {32}
    set_instance_parameter_value  ai_nios_mm        MASTER_SYNC_DEPTH           {2}
    set_instance_parameter_value  ai_nios_mm        MAX_BURST_SIZE              {1}
    set_instance_parameter_value  ai_nios_mm        RESPONSE_FIFO_DEPTH         {4}
    set_instance_parameter_value  ai_nios_mm        SLAVE_SYNC_DEPTH            {2}
    set_instance_parameter_value  ai_nios_mm        SYMBOL_WIDTH                {8}
    set_instance_parameter_value  ai_nios_mm        SYNC_RESET                  {0}
    set_instance_parameter_value  ai_nios_mm        USE_AUTO_ADDRESS_WIDTH      {1}

    # ai_ddr_se
    set_instance_parameter_value  ai_ddr_se         BURSTCOUNT_WIDTH            {6}
    set_instance_parameter_value  ai_ddr_se         DATA_WIDTH                  {256}
    set_instance_parameter_value  ai_ddr_se         ENABLE_SLAVE_PORT           {0}
    set_instance_parameter_value  ai_ddr_se         MASTER_ADDRESS_DEF          {0}
    set_instance_parameter_value  ai_ddr_se         MASTER_ADDRESS_WIDTH        {33}
    set_instance_parameter_value  ai_ddr_se         MAX_PENDING_READS           {8}
    set_instance_parameter_value  ai_ddr_se         SLAVE_ADDRESS_WIDTH         {26}
    set_instance_parameter_value  ai_ddr_se         SUB_WINDOW_COUNT            {1}
    set_instance_parameter_value  ai_ddr_se         SYNC_RESET                  {0}

    # ai_mm_bridge
    set_instance_parameter_value    ai_mm_bridge    ADDRESS_UNITS             {SYMBOLS}
    set_instance_parameter_value    ai_mm_bridge    ADDRESS_WIDTH             {31}
    set_instance_parameter_value    ai_mm_bridge    DATA_WIDTH                {256}
    set_instance_parameter_value    ai_mm_bridge    LINEWRAPBURSTS            {0}
    set_instance_parameter_value    ai_mm_bridge    M0_WAITREQUEST_ALLOWANCE  {0}
    set_instance_parameter_value    ai_mm_bridge    MAX_BURST_SIZE            {32}
    set_instance_parameter_value    ai_mm_bridge    MAX_PENDING_RESPONSES     {64}
    set_instance_parameter_value    ai_mm_bridge    MAX_PENDING_WRITES        {64}
    set_instance_parameter_value    ai_mm_bridge    PIPELINE_COMMAND          {1}
    set_instance_parameter_value    ai_mm_bridge    PIPELINE_RESPONSE         {1}
    set_instance_parameter_value    ai_mm_bridge    S0_WAITREQUEST_ALLOWANCE  {0}
    set_instance_parameter_value    ai_mm_bridge    SYMBOL_WIDTH              {8}
    set_instance_parameter_value    ai_mm_bridge    SYNC_RESET                {0}
    set_instance_parameter_value    ai_mm_bridge    USE_AUTO_ADDRESS_WIDTH    {0}
    set_instance_parameter_value    ai_mm_bridge    USE_RESPONSE              {1}
    set_instance_parameter_value    ai_mm_bridge    USE_WRITERESPONSE         {1}

    # ai_ip_core
    set_instance_parameter_value  ai_ip_core        ARCH_OPTION                "generated_arch_AGX5"
    set_instance_parameter_value  ai_ip_core        C_DDR_AXI_ADDR_WIDTH        {31}
    set_instance_parameter_value  ai_ip_core        C_DDR_AXI_DATA_WIDTH        {256}
    set_instance_parameter_value  ai_ip_core        C_DDR_AXI_READ_ID_WIDTH     {2}
    set_instance_parameter_value  ai_ip_core        C_DDR_AXI_WRITE_ID_WIDTH    {2}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_SIZE             {2048}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_TYPE             {949}
    set_instance_parameter_value  ai_ip_core        C_OMNI_CAP_VERSION          {1}


    save_system
    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Create Connections ####
    ############################

    # ai_core_clk
    add_connection  ai_core_clk.out_clk         ai_ip_core.dla_clk

    # ai_core_rst
    add_connection  ai_core_rst.out_reset       ai_ip_core.dla_resetn

    # ai_hps_clk
    add_connection  ai_hps_clk.out_clk          ai_core_rst.clk
    add_connection  ai_hps_clk.out_clk          ai_hps_rst.clk
    add_connection  ai_hps_clk.out_clk          ai_hps_mm.s0_clk
    add_connection  ai_hps_clk.out_clk          ai_ip_core.irq_clk

    # ai_hps_rst
    add_connection  ai_hps_rst.out_reset        ai_hps_mm.s0_reset

    # ai_nios_clk
    add_connection  ai_nios_clk.out_clk         ai_nios_rst.clk
    add_connection  ai_nios_clk.out_clk         ai_nios_mm.s0_clk

    # ai_nios_rst
    add_connection  ai_nios_rst.out_reset       ai_nios_mm.s0_reset

    # ai_ddr_clk
    add_connection  ai_ddr_clk.out_clk          ai_ddr_rst.clk
    add_connection  ai_ddr_clk.out_clk          ai_ddr_se.clock
    add_connection  ai_ddr_clk.out_clk          ai_ip_core.ddr_clk
    add_connection  ai_ddr_clk.out_clk          ai_mm_bridge.clk
    add_connection  ai_ddr_clk.out_clk          ai_hps_mm.m0_clk
    add_connection  ai_ddr_clk.out_clk          ai_nios_mm.m0_clk

    # ai_ddr_rst
    add_connection  ai_ddr_rst.out_reset        ai_ddr_se.reset
    add_connection  ai_ddr_rst.out_reset        ai_mm_bridge.reset
    add_connection  ai_ddr_rst.out_reset        ai_hps_mm.m0_reset
    add_connection  ai_ddr_rst.out_reset        ai_nios_mm.m0_reset

    # ai_hps_mm
    add_connection  ai_hps_mm.m0                ai_ip_core.csr_axi

    # ai_nios_mm
    add_connection  ai_nios_mm.m0               ai_ip_core.csr_axi

    # ai_ip_core
    add_connection  ai_ip_core.ddr_axi          ai_mm_bridge.s0

    # ai_mm_bridge
    add_connection  ai_mm_bridge.m0             ai_ddr_se.windowed_slave


    ##########################
    ##### Create Exports #####
    ##########################

    # ai_core_clk
    add_interface          i_ai_clk           clock      sink
    set_interface_property i_ai_clk           export_of  ai_core_clk.in_clk

    # ai_core_rst
    add_interface          i_ai_rst           reset      sink
    set_interface_property i_ai_rst           export_of  ai_core_rst.in_reset

    # ai_hps_clk
    add_interface          i_csr_clk          clock      sink
    set_interface_property i_csr_clk          export_of  ai_hps_clk.in_clk

    # ai_hps_rst
    add_interface          i_csr_rst          reset      sink
    set_interface_property i_csr_rst          export_of  ai_hps_rst.in_reset

    # ai_nios_clk
    add_interface          i_csr_clk_2        clock      sink
    set_interface_property i_csr_clk_2        export_of  ai_nios_clk.in_clk

    # ai_nios_rst
    add_interface          i_csr_rst_2        reset      sink
    set_interface_property i_csr_rst_2        export_of  ai_nios_rst.in_reset

    # ai_ddr_clk
    add_interface          i_ddr_clk          clock      sink
    set_interface_property i_ddr_clk          export_of  ai_ddr_clk.in_clk

    # ai_ddr_rst
    add_interface          i_ddr_rst          reset      sink
    set_interface_property i_ddr_rst          export_of  ai_ddr_rst.in_reset

    # ai_hps_mm
    add_interface          i_csr_mm_agent     avalon     agent
    set_interface_property i_csr_mm_agent     export_of  ai_hps_mm.s0

    # ai_nios_mm
    add_interface          i_csr_mm_agent_2   avalon     agent
    set_interface_property i_csr_mm_agent_2   export_of  ai_nios_mm.s0

    # ai_ddr_se
    add_interface          o_ddr_mm_host      avalon     host
    set_interface_property o_ddr_mm_host      export_of  ai_ddr_se.expanded_master

    # ai_ip_core
    add_interface          o_ai_irq           interrupt  sender
    set_interface_property o_ai_irq           export_of  ai_ip_core.irq_level


    sync_sysinfo_parameters
    save_system
}


proc edit_top_level_qsys {} {
    set v_project_name  [get_shell_parameter PROJECT_NAME]
    set v_project_path  [get_shell_parameter PROJECT_PATH]
    set v_instance_name [get_shell_parameter INSTANCE_NAME]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name} ${v_instance_name}

    sync_sysinfo_parameters
    save_system
}


proc add_auto_connections {} {
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]
    set v_core_clk_freq           [get_shell_parameter CORE_CLK_FREQ]
    set v_avmm_host               [get_shell_parameter AVMM_HOST]
    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_avmm_host_2             [get_shell_parameter AVMM_HOST_2]
    set v_avmm_host_2_clk_freq    [get_shell_parameter AVMM_HOST_2_CLK_FREQ]
    set v_irq_priority            [get_shell_parameter IRQ_PRIORITY]
    set v_irq_host                [get_shell_parameter IRQ_HOST]
    set v_emif_agent              [get_shell_parameter EMIF_AGENT]

    add_auto_connection   ${v_instance_name}  i_ai_clk        ${v_core_clk_freq}
    add_auto_connection   ${v_instance_name}  i_ai_rst        ${v_core_clk_freq}

    add_auto_connection   ${v_instance_name}  i_csr_clk       ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name}  i_csr_rst       ${v_avmm_host_clk_freq}

    add_auto_connection   ${v_instance_name}  i_csr_clk_2     ${v_avmm_host_2_clk_freq}
    add_auto_connection   ${v_instance_name}  i_csr_rst_2     ${v_avmm_host_2_clk_freq}

    add_irq_connection    ${v_instance_name}  o_ai_irq   ${v_irq_priority}   ${v_irq_host}_irq

    add_avmm_connections  i_csr_mm_agent      ${v_avmm_host}

    add_avmm_connections  i_csr_mm_agent_2    ${v_avmm_host_2}

    add_auto_connection   ${v_instance_name}  i_ddr_clk       ${v_emif_agent}_user_clk
    add_auto_connection   ${v_instance_name}  i_ddr_rst       ${v_emif_agent}_user_rst
    add_auto_connection   ${v_instance_name}  o_ddr_mm_host   ${v_emif_agent}_user_data
}


proc modify_manual_ocs_rom {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_drv_ocs_subsystem_name    [get_shell_parameter DRV_OCS_SUBSYSTEM_NAME]
    set v_inst_id                   [get_shell_parameter INST_ID]

    if {${v_drv_ocs_subsystem_name} != ""} {

      load_system ${v_project_path}/rtl/shell/${v_drv_ocs_subsystem_name}.qsys
      load_component hps_intel_offset_capability_manual

      set v_num_caps [get_component_parameter_value C_NUM_CAPS]

          # check if the capability contains a dummy entry (required for ip instantiation)
          if {${v_num_caps} == 1} {
              set v_cap0_type [get_component_parameter_value  C_CAP0_TYPE]

              if {${v_cap0_type} == 0} {
                  set v_num_caps 0
              }
          }

      set_component_parameter_value  C_NUM_CAPS [expr ${v_num_caps} + 1]

      set v_ai_ip_base_addr     [get_shell_parameter SS_BASE_ADDR]

          set v_index ${v_num_caps}

      # ai ip
      set_component_parameter_value  C_CAP${v_index}_BASE           ${v_ai_ip_base_addr}
      set_component_parameter_value  C_CAP${v_index}_SIZE           {512}
      set_component_parameter_value  C_CAP${v_index}_TYPE           {949}
      set_component_parameter_value  C_CAP${v_index}_VERSION        {1}
      set_component_parameter_value  C_CAP${v_index}_ID_COMPONENT   ${v_inst_id}

      save_component

      sync_sysinfo_parameters
      save_system
    }
}
