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

set_shell_parameter AVMM_HOST                     {{AUTO X}}
set_shell_parameter AVMM_HOST_2                   {{AUTO X}}

# Subsystem Base Address
set_shell_parameter SS_BASE_ADDR                  "0x00600000"

# IP AVMM Offset Addresses
set_shell_parameter VFW_FRES_ADDR_OFFSET          "0x00000c00"
set_shell_parameter VFR_FRES_ADDR_OFFSET          "0x00001000"
set_shell_parameter SCALE_DOWN_ADDR_OFFSET        "0x00000000"
set_shell_parameter DLA_LT_ADDR_OFFSET            "0x00000a00"
set_shell_parameter VFW_LRES_ADDR_OFFSET          "0x00000200"
set_shell_parameter MSG_Q_ADDR_OFFSET             "0x00001800"
set_shell_parameter CPU_ISSUE_IRQ_ADDR_OFFSET     "0x00001400"
set_shell_parameter CPU2_TO_CPU_IRQ_ADDR_OFFSET   "0x00001410"

# Offsets
set_shell_parameter MSG_Q_BASE_ADDR               "0x00000000"
set_shell_parameter IRQ_FOR_HPS_BASE_ADDR         "0x00000800"
set_shell_parameter HPS_IRQ_GEN_BASE_ADDR         "0x00000810"

# General Video Controls
set_shell_parameter PIP                           {2}
set_shell_parameter VID_OUT_BPS                   {10}
set_shell_parameter EN_DEBUG                      {1}

set_shell_parameter INST_ID                       {0}

# AI Controls
set_shell_parameter EMIF_AGENT                    {}
set_shell_parameter EMIF_AGENT_CLK_FREQ           {200000000.0}

# AI Interrupts
set_shell_parameter FRES_VFW_IRQ_PRIORITY         "X"
set_shell_parameter FRES_VFW_IRQ_HOST             ""
set_shell_parameter FRES_VFR_IRQ_PRIORITY         "X"
set_shell_parameter FRES_VFR_IRQ_HOST             ""
set_shell_parameter LRES_VFW_IRQ_PRIORITY         "X"
set_shell_parameter LRES_VFW_IRQ_HOST             ""
set_shell_parameter MSG_NIOS_IRQ_PRIORITY         "X"
set_shell_parameter MSG_NIOS_IRQ_HOST             ""
set_shell_parameter MSG_HPS_IRQ_PRIORITY          "X"
set_shell_parameter MSG_HPS_IRQ_HOST              ""


# resolve interdependencies
proc derive_parameters {param_array} {
    upvar $param_array p_array

    set_shell_parameter DRV_OCS_SUBSYSTEM_NAME ""

    # look for OCS subsystem
    for {set id 0} {$id < $p_array(project,id)} {incr id} {
        if {$p_array($id,type) == "ocs"} {
            set params $p_array($id,params)

            foreach v_pair $params {
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
        send_message ERROR "isp_4k_ai_create: EMIF Agent not specified"
    }

}

proc pre_creation_step {} {
    transfer_files
}

proc creation_step {} {
    create_isp_ai_subsystem
}

proc post_creation_step {} {
    modify_manual_ocs_rom
    edit_top_level_qsys
    add_auto_connections
}

proc transfer_files {} {
    set v_project_path                  [get_shell_parameter PROJECT_PATH]
    set v_script_path                   [get_shell_parameter SUBSYSTEM_SOURCE_PATH]

    exec cp -rf ${v_script_path}/../../non_qpds_ip/intel_dla_lt         ${v_project_path}/non_qpds_ip/user
    file_copy   ${v_script_path}/../../non_qpds_ip/intel_dla_lt.ipx     ${v_project_path}/non_qpds_ip/user

    file_copy   ${v_script_path}/isp_4k_ai.sdc.terp                     ${v_project_path}/sdc/user/isp_ai.sdc
}

proc create_isp_ai_subsystem {} {
    set v_project_path        [get_shell_parameter PROJECT_PATH]
    set v_instance_name       [get_shell_parameter INSTANCE_NAME]

    # Video Pipeline
    set v_cppp                {3}
    set v_vid_out_bps         [get_shell_parameter VID_OUT_BPS]
    set v_pip                 [get_shell_parameter PIP]

    # AI Video In Pipeline (fixed config)
    set v_ai_bps              {8}
    set v_ai_cppp             {3}
    set v_ai_pip              {1}
    set v_ai_lt_bps           {16}
    set v_ai_lt_cppp          {4}
    set v_ai_lt_pip           {4}

    set v_emif_agent_clk_freq     [get_shell_parameter EMIF_AGENT_CLK_FREQ]

    # General
    set v_inst_id           [get_shell_parameter INST_ID]
    set v_enable_debug      [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready    {1}

    # AI max input size
    # downscale is 6:1 ratio from 4k to 640x360 for 16:9 widescreen input
    set v_coredla_width_max   {640}
    set v_coredla_height_max  {360}

    # LT output is half height and half width, with 1 extra pixel word and 1 extra line pair to account for padding
    set v_lt_coredla_width_max    [expr ((${v_coredla_width_max} /2) + 1)]
    set v_lt_coredla_height_max   [expr ((${v_coredla_height_max} /2) + 1)]

    # AI default input size
    # downscale is 12:1 ratio from 4k to 320x180 for 16:9 widescreen input
    # SW to program changes for 640x360 mode
    set v_coredla_width_def       [expr (${v_coredla_width_max} /2)]
    set v_coredla_height_def      [expr (${v_coredla_height_max} /2)]

    set v_coredla_scale_ratio     [expr {3840 / ${v_coredla_width_def}}]

    set v_dla_lt_v_lines          [expr ${v_coredla_height_def} -2]


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  isp_ai_cpu_clk_bridge         altera_clock_bridge
    add_instance  isp_ai_cpu_rst_bridge         altera_reset_bridge
    add_instance  isp_ai_vid_clk_bridge         altera_clock_bridge
    add_instance  isp_ai_vid_rst_bridge         altera_reset_bridge
    add_instance  isp_ai_emif_clk_bridge        altera_clock_bridge
    add_instance  isp_ai_emif_rst_bridge        altera_reset_bridge
    add_instance  isp_ai_mm_bridge              altera_avalon_mm_bridge
    add_instance  isp_ai_axi4s_bcast            intel_vvp_axi4s_broadcaster
    add_instance  isp_ai_vfw_fres               intel_vvp_vfw
    add_instance  isp_ai_se_vfw_fres            altera_address_span_extender
    add_instance  isp_ai_vfr_fres               intel_vvp_vfr
    add_instance  isp_ai_se_vfr_fres            altera_address_span_extender
    add_instance  isp_ai_pix_adapt              intel_vvp_pixel_adapter
    add_instance  isp_ai_scaler_down            intel_vvp_scaler
    add_instance  isp_ai_dla_lt                 intel_dla_lt
    add_instance  isp_ai_fifo_dla_lt            intel_vvp_fifo
    add_instance  isp_ai_vfw_lres               intel_vvp_vfw
    add_instance  isp_ai_se_vfw_lres            altera_address_span_extender
    add_instance  isp_ai_irq_cdc_0              altera_irq_clock_crosser
    add_instance  isp_ai_irq_cdc_1              altera_irq_clock_crosser
    add_instance  isp_ai_irq_cdc_2              altera_irq_clock_crosser

    # Message Queue
    add_instance  isp_ai_cpu2_clk_bridge        altera_clock_bridge
    add_instance  isp_ai_cpu2_rst_bridge        altera_reset_bridge
    add_instance  isp_ai_cpu2_mm_bridge         altera_avalon_mm_bridge
    add_instance  isp_ai_cpu2_cc_bridge         mm_ccb
    add_instance  isp_ai_msg_q                  intel_onchip_memory
    add_instance  isp_ai_cpu_issue_irq          altera_avalon_pio
    add_instance  isp_ai_cpu_irq_for_cpu2       altera_avalon_pio
    add_instance  isp_ai_cpu2_issue_irq         altera_avalon_pio
    add_instance  isp_ai_cpu2_irq_for_cpu       altera_avalon_pio


    ############################
    #### Set Parameters     ####
    ############################

    # isp_ai_cpu_clk_bridge
    set_instance_parameter_value      isp_ai_cpu_clk_bridge       EXPLICIT_CLOCK_RATE           {100000000.0}
    set_instance_parameter_value      isp_ai_cpu_clk_bridge       NUM_CLOCK_OUTPUTS             {1}

    # isp_ai_cpu_rst_bridge
    set_instance_parameter_value      isp_ai_cpu_rst_bridge       ACTIVE_LOW_RESET              {0}
    set_instance_parameter_value      isp_ai_cpu_rst_bridge       NUM_RESET_OUTPUTS             {1}
    set_instance_parameter_value      isp_ai_cpu_rst_bridge       SYNCHRONOUS_EDGES             {deassert}
    set_instance_parameter_value      isp_ai_cpu_rst_bridge       SYNC_RESET                    {0}
    set_instance_parameter_value      isp_ai_cpu_rst_bridge       USE_RESET_REQUEST             {0}

    # isp_ai_mm_bridge
    set_instance_parameter_value      isp_ai_mm_bridge            ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value      isp_ai_mm_bridge            ADDRESS_WIDTH                 {0}
    set_instance_parameter_value      isp_ai_mm_bridge            DATA_WIDTH                    {32}
    set_instance_parameter_value      isp_ai_mm_bridge            LINEWRAPBURSTS                {0}
    set_instance_parameter_value      isp_ai_mm_bridge            M0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_ai_mm_bridge            MAX_BURST_SIZE                {1}
    set_instance_parameter_value      isp_ai_mm_bridge            MAX_PENDING_RESPONSES         {4}
    set_instance_parameter_value      isp_ai_mm_bridge            MAX_PENDING_WRITES            {0}
    set_instance_parameter_value      isp_ai_mm_bridge            PIPELINE_COMMAND              {1}
    set_instance_parameter_value      isp_ai_mm_bridge            PIPELINE_RESPONSE             {1}
    set_instance_parameter_value      isp_ai_mm_bridge            S0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_ai_mm_bridge            SYMBOL_WIDTH                  {8}
    set_instance_parameter_value      isp_ai_mm_bridge            SYNC_RESET                    {0}
    set_instance_parameter_value      isp_ai_mm_bridge            USE_AUTO_ADDRESS_WIDTH        {1}
    set_instance_parameter_value      isp_ai_mm_bridge            USE_RESPONSE                  {0}
    set_instance_parameter_value      isp_ai_mm_bridge            USE_WRITERESPONSE             {0}

    # isp_ai_vid_clk_bridge
    set_instance_parameter_value      isp_ai_vid_clk_bridge       EXPLICIT_CLOCK_RATE     {297000000.0}
    set_instance_parameter_value      isp_ai_vid_clk_bridge       NUM_CLOCK_OUTPUTS       {1}

    # isp_ai_vid_rst_bridge
    set_instance_parameter_value      isp_ai_vid_rst_bridge       ACTIVE_LOW_RESET        {0}
    set_instance_parameter_value      isp_ai_vid_rst_bridge       NUM_RESET_OUTPUTS       {1}
    set_instance_parameter_value      isp_ai_vid_rst_bridge       SYNCHRONOUS_EDGES       {deassert}
    set_instance_parameter_value      isp_ai_vid_rst_bridge       SYNC_RESET              {0}
    set_instance_parameter_value      isp_ai_vid_rst_bridge       USE_RESET_REQUEST       {0}

    # isp_ai_emif_clk_bridge
    set_instance_parameter_value      isp_ai_emif_clk_bridge      EXPLICIT_CLOCK_RATE     ${v_emif_agent_clk_freq}
    set_instance_parameter_value      isp_ai_emif_clk_bridge      NUM_CLOCK_OUTPUTS       {1}

    # isp_ai_emif_rst_bridge
    set_instance_parameter_value      isp_ai_emif_rst_bridge      ACTIVE_LOW_RESET        {1}
    set_instance_parameter_value      isp_ai_emif_rst_bridge      NUM_RESET_OUTPUTS       {1}
    set_instance_parameter_value      isp_ai_emif_rst_bridge      SYNCHRONOUS_EDGES       {deassert}
    set_instance_parameter_value      isp_ai_emif_rst_bridge      SYNC_RESET              {0}
    set_instance_parameter_value      isp_ai_emif_rst_bridge      USE_RESET_REQUEST       {0}

    # isp_ai_axi4s_bcast
    set_instance_parameter_value      isp_ai_axi4s_bcast      BPS                           ${v_vid_out_bps}
    set_instance_parameter_value      isp_ai_axi4s_bcast      GLOBAL_STALL                  {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      IN_TREADY                     {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUTPUTS                       {2}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_0_FIFO                    {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_0_FIFO_DEPTH              {1024}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_0_TREADY                  {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_1_FIFO                    {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_1_FIFO_DEPTH              {1024}
    set_instance_parameter_value      isp_ai_axi4s_bcast      OUT_1_TREADY                  {1}
    set_instance_parameter_value      isp_ai_axi4s_bcast      PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_ai_axi4s_bcast      VVP_INTF_TYPE                 {Lite}

    # isp_ai_vfw_fres
    set_instance_parameter_value      isp_ai_vfw_fres         BPS                           ${v_vid_out_bps}
    set_instance_parameter_value      isp_ai_vfw_fres         CLOCKS_ARE_SEPARATE           {1}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_TYPE               {585}
    set_instance_parameter_value      isp_ai_vfw_fres         C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ai_vfw_fres         ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ai_vfw_fres         EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ai_vfw_fres         MAX_HEIGHT                    {2160}
    set_instance_parameter_value      isp_ai_vfw_fres         MAX_WIDTH                     {3840}
    set_instance_parameter_value      isp_ai_vfw_fres         NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_ai_vfw_fres         PACKING                       {PERFECT}
    set_instance_parameter_value      isp_ai_vfw_fres         PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_ai_vfw_fres         P_AV_MM_ADDR_WIDTH            {32}
    set_instance_parameter_value      isp_ai_vfw_fres         P_AV_MM_DATA_WIDTH            {256}
    set_instance_parameter_value      isp_ai_vfw_fres         SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_ai_vfw_fres         SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_ai_vfw_fres         WRITE_BURST_TARGET            {32}
    set_instance_parameter_value      isp_ai_vfw_fres         WRITE_FIFO_DEPTH              {512}

    # isp_ai_se_vfw_fres
    set_instance_parameter_value      isp_ai_se_vfw_fres      BURSTCOUNT_WIDTH              {6}
    set_instance_parameter_value      isp_ai_se_vfw_fres      DATA_WIDTH                    {256}
    set_instance_parameter_value      isp_ai_se_vfw_fres      ENABLE_SLAVE_PORT             {0}
    set_instance_parameter_value      isp_ai_se_vfw_fres      MASTER_ADDRESS_DEF            {0}
    set_instance_parameter_value      isp_ai_se_vfw_fres      MASTER_ADDRESS_WIDTH          {33}
    set_instance_parameter_value      isp_ai_se_vfw_fres      MAX_PENDING_READS             {8}
    set_instance_parameter_value      isp_ai_se_vfw_fres      SLAVE_ADDRESS_WIDTH           {26}
    set_instance_parameter_value      isp_ai_se_vfw_fres      SUB_WINDOW_COUNT              {1}
    set_instance_parameter_value      isp_ai_se_vfw_fres      SYNC_RESET                    {0}

    # isp_ai_vfr_fres
    set_instance_parameter_value      isp_ai_vfr_fres         BPS                           ${v_vid_out_bps}
    set_instance_parameter_value      isp_ai_vfr_fres         CLOCKS_ARE_SEPARATE           {1}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_TYPE               {586}
    set_instance_parameter_value      isp_ai_vfr_fres         C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ai_vfr_fres         ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ai_vfr_fres         EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ai_vfr_fres         MAX_BUFFER_SETS               {2}
    set_instance_parameter_value      isp_ai_vfr_fres         MAX_HEIGHT                    {2160}
    set_instance_parameter_value      isp_ai_vfr_fres         MAX_WIDTH                     {3840}
    set_instance_parameter_value      isp_ai_vfr_fres         NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_ai_vfr_fres         PACKING                       {PERFECT}
    set_instance_parameter_value      isp_ai_vfr_fres         PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_ai_vfr_fres         P_AV_MM_ADDR_WIDTH            {32}
    set_instance_parameter_value      isp_ai_vfr_fres         P_AV_MM_DATA_WIDTH            {256}
    set_instance_parameter_value      isp_ai_vfr_fres         READ_BURST_TARGET             {32}
    set_instance_parameter_value      isp_ai_vfr_fres         READ_FIFO_DEPTH               {512}
    set_instance_parameter_value      isp_ai_vfr_fres         SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_ai_vfr_fres         SLAVE_PROTOCOL                {Avalon}

    # isp_ai_se_vfr_fres
    set_instance_parameter_value      isp_ai_se_vfr_fres      BURSTCOUNT_WIDTH              {6}
    set_instance_parameter_value      isp_ai_se_vfr_fres      DATA_WIDTH                    {256}
    set_instance_parameter_value      isp_ai_se_vfr_fres      ENABLE_SLAVE_PORT             {0}
    set_instance_parameter_value      isp_ai_se_vfr_fres      MASTER_ADDRESS_DEF            {0}
    set_instance_parameter_value      isp_ai_se_vfr_fres      MASTER_ADDRESS_WIDTH          {33}
    set_instance_parameter_value      isp_ai_se_vfr_fres      MAX_PENDING_READS             {8}
    set_instance_parameter_value      isp_ai_se_vfr_fres      SLAVE_ADDRESS_WIDTH           {26}
    set_instance_parameter_value      isp_ai_se_vfr_fres      SUB_WINDOW_COUNT              {1}
    set_instance_parameter_value      isp_ai_se_vfr_fres      SYNC_RESET                    {0}

    # isp_ai_pix_adapt
    set_instance_parameter_value      isp_ai_pix_adapt        BPS_IN                        ${v_vid_out_bps}
    set_instance_parameter_value      isp_ai_pix_adapt        BPS_OUT                       ${v_ai_bps}
    set_instance_parameter_value      isp_ai_pix_adapt        ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ai_pix_adapt        EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ai_pix_adapt        NUMBER_OF_COLOR_PLANES        ${v_ai_cppp}
    set_instance_parameter_value      isp_ai_pix_adapt        PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_ai_pix_adapt        PIXELS_IN_PARALLEL            ${v_ai_pip}
    set_instance_parameter_value      isp_ai_pix_adapt        P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_ai_pix_adapt        P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_ai_pix_adapt        RUNTIME_CONTROL               {0}
    set_instance_parameter_value      isp_ai_pix_adapt        SEPARATE_SLAVE_CLOCK          {0}
    set_instance_parameter_value      isp_ai_pix_adapt        SLAVE_PROTOCOL                {Avalon}

    # isp_ai_scaler_down
    set_instance_parameter_value      isp_ai_scaler_down      ALGORITHM                     {POLYPHASE}
    set_instance_parameter_value      isp_ai_scaler_down      BPS                           ${v_ai_bps}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_TYPE               {564}
    set_instance_parameter_value      isp_ai_scaler_down      C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ai_scaler_down      EDGE_MIRROR                   {REPLICATE}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_420                    {0}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_420_MIRROR             {0}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_422                    {0}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_444                    {1}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_H                      {1}
    set_instance_parameter_value      isp_ai_scaler_down      ENABLE_V                      {1}
    set_instance_parameter_value      isp_ai_scaler_down      EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ai_scaler_down      HALF_RATE_420                 {0}
    set_instance_parameter_value      isp_ai_scaler_down      H_BANKS                       {1}
    set_instance_parameter_value      isp_ai_scaler_down      H_COEFF_FRAC_BITS             {8}
    set_instance_parameter_value      isp_ai_scaler_down      H_COEFF_FUNCTION              {LANCZOS_1}
    set_instance_parameter_value      isp_ai_scaler_down      H_COEFF_INT_BITS              {1}
    set_instance_parameter_value      isp_ai_scaler_down      H_COEFF_SIGNED                {1}
    set_instance_parameter_value      isp_ai_scaler_down      H_INIT_FILE \
                                                                      {<enter file name (including full path)>}
    set_instance_parameter_value      isp_ai_scaler_down      H_PARTIAL_SCALING             {0}
    set_instance_parameter_value      isp_ai_scaler_down      H_PHASES                      {16}
    set_instance_parameter_value      isp_ai_scaler_down      H_TAPS                        ${v_coredla_scale_ratio}
    set_instance_parameter_value      isp_ai_scaler_down      MAX_IN_WIDTH                  {3840}
    set_instance_parameter_value      isp_ai_scaler_down      MAX_OUT_WIDTH                 ${v_coredla_width_max}
    set_instance_parameter_value      isp_ai_scaler_down      MEM_INIT                      {1}
    set_instance_parameter_value      isp_ai_scaler_down      NO_BLANKING                   {0}
    set_instance_parameter_value      isp_ai_scaler_down      NUMBER_OF_COLOR_PLANES        ${v_ai_cppp}
    set_instance_parameter_value      isp_ai_scaler_down      OUTPUT_HEIGHT                 ${v_coredla_height_def}
    set_instance_parameter_value      isp_ai_scaler_down      PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_ai_scaler_down      PIXELS_IN_PARALLEL            ${v_ai_pip}
    set_instance_parameter_value      isp_ai_scaler_down      P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_ai_scaler_down      P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_ai_scaler_down      RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_ai_scaler_down      RUNTIME_LOAD                  {0}
    set_instance_parameter_value      isp_ai_scaler_down      SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_ai_scaler_down      SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_ai_scaler_down      V_BANKS                       {1}
    set_instance_parameter_value      isp_ai_scaler_down      V_COEFF_FRAC_BITS             {8}
    set_instance_parameter_value      isp_ai_scaler_down      V_COEFF_FUNCTION              {LANCZOS_1}
    set_instance_parameter_value      isp_ai_scaler_down      V_COEFF_INT_BITS              {1}
    set_instance_parameter_value      isp_ai_scaler_down      V_COEFF_SIGNED                {1}
    set_instance_parameter_value      isp_ai_scaler_down      V_INIT_FILE \
                                                                      {<enter file name (including full path)>}
    set_instance_parameter_value      isp_ai_scaler_down      V_PARTIAL_SCALING             {0}
    set_instance_parameter_value      isp_ai_scaler_down      V_PHASES                      {16}
    set_instance_parameter_value      isp_ai_scaler_down      V_PRES_FRAC_BITS              {0}
    set_instance_parameter_value      isp_ai_scaler_down      V_TAPS                        ${v_coredla_scale_ratio}

    # isp_ai_dla_lt
    set_instance_parameter_value      isp_ai_dla_lt           BPS_IN                        ${v_ai_bps}
    set_instance_parameter_value      isp_ai_dla_lt           C_V_LINES                     ${v_dla_lt_v_lines}
    set_instance_parameter_value      isp_ai_dla_lt           C_DC_FP16_PIX_VAL             {0x5800}
    set_instance_parameter_value      isp_ai_dla_lt           C_DF_FP16_PIX_VAL             {0x0000}
    set_instance_parameter_value      isp_ai_dla_lt           C_CPU_OFFSET                  {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_SIZE               {64}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_TYPE               {950}
    set_instance_parameter_value      isp_ai_dla_lt           C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ai_dla_lt           NUMBER_OF_COLOR_PLANES_IN     ${v_ai_cppp}
    set_instance_parameter_value      isp_ai_dla_lt           PIXELS_IN_PARALLEL_IN         ${v_ai_pip}
    set_instance_parameter_value      isp_ai_dla_lt           RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_ai_dla_lt           BPS_OUT                       ${v_ai_lt_bps}
    set_instance_parameter_value      isp_ai_dla_lt           NUMBER_OF_COLOR_PLANES_OUT    ${v_ai_lt_cppp}
    set_instance_parameter_value      isp_ai_dla_lt           PIXELS_IN_PARALLEL_OUT        ${v_ai_lt_pip}
    set_instance_parameter_value      isp_ai_dla_lt           C_RB_SWAP                     {0}

    # isp_ai_fifo_dla_lt
    set_instance_parameter_value      isp_ai_fifo_dla_lt      BPS                           ${v_ai_bps}
    set_instance_parameter_value      isp_ai_fifo_dla_lt      DUAL_CLOCK                    {0}
    set_instance_parameter_value      isp_ai_fifo_dla_lt      FIFO_DEPTH                    {1024}
    set_instance_parameter_value      isp_ai_fifo_dla_lt      NUMBER_OF_COLOR_PLANES        ${v_ai_cppp}
    set_instance_parameter_value      isp_ai_fifo_dla_lt      PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_ai_fifo_dla_lt      PIXELS_IN_PARALLEL            ${v_ai_pip}

    # isp_ai_vfw_lres
    set_instance_parameter_value      isp_ai_vfw_lres         BPS                           ${v_ai_lt_bps}
    set_instance_parameter_value      isp_ai_vfw_lres         CLOCKS_ARE_SEPARATE           {1}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_TYPE               {585}
    set_instance_parameter_value      isp_ai_vfw_lres         C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ai_vfw_lres         ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ai_vfw_lres         EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ai_vfw_lres         MAX_HEIGHT                    ${v_lt_coredla_height_max}
    set_instance_parameter_value      isp_ai_vfw_lres         MAX_WIDTH                     ${v_lt_coredla_width_max}
    set_instance_parameter_value      isp_ai_vfw_lres         NUMBER_OF_COLOR_PLANES        ${v_ai_lt_cppp}
    set_instance_parameter_value      isp_ai_vfw_lres         PACKING                       {PERFECT}
    set_instance_parameter_value      isp_ai_vfw_lres         PIXELS_IN_PARALLEL            ${v_ai_lt_pip}
    set_instance_parameter_value      isp_ai_vfw_lres         P_AV_MM_ADDR_WIDTH            {32}
    set_instance_parameter_value      isp_ai_vfw_lres         P_AV_MM_DATA_WIDTH            {256}
    set_instance_parameter_value      isp_ai_vfw_lres         SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_ai_vfw_lres         SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_ai_vfw_lres         WRITE_BURST_TARGET            {8}
    set_instance_parameter_value      isp_ai_vfw_lres         WRITE_FIFO_DEPTH              {512}

    # isp_ai_se_vfw_lres
    set_instance_parameter_value      isp_ai_se_vfw_lres      BURSTCOUNT_WIDTH              {6}
    set_instance_parameter_value      isp_ai_se_vfw_lres      DATA_WIDTH                    {256}
    set_instance_parameter_value      isp_ai_se_vfw_lres      ENABLE_SLAVE_PORT             {0}
    set_instance_parameter_value      isp_ai_se_vfw_lres      MASTER_ADDRESS_DEF            {0}
    set_instance_parameter_value      isp_ai_se_vfw_lres      MASTER_ADDRESS_WIDTH          {33}
    set_instance_parameter_value      isp_ai_se_vfw_lres      MAX_PENDING_READS             {8}
    set_instance_parameter_value      isp_ai_se_vfw_lres      SLAVE_ADDRESS_WIDTH           {26}
    set_instance_parameter_value      isp_ai_se_vfw_lres      SUB_WINDOW_COUNT              {1}
    set_instance_parameter_value      isp_ai_se_vfw_lres      SYNC_RESET                    {0}

    # isp_ai_irq_cdc_0
    set_instance_parameter_value      isp_ai_irq_cdc_0        IRQ_WIDTH                     {1}
    set_instance_parameter_value      isp_ai_irq_cdc_0        SYNC_RESET                    {0}
    # isp_ai_irq_cdc_1
    set_instance_parameter_value      isp_ai_irq_cdc_1        IRQ_WIDTH                     {1}
    set_instance_parameter_value      isp_ai_irq_cdc_1        SYNC_RESET                    {0}
    # isp_ai_irq_cdc_2
    set_instance_parameter_value      isp_ai_irq_cdc_2        IRQ_WIDTH                     {1}
    set_instance_parameter_value      isp_ai_irq_cdc_2        SYNC_RESET                    {0}

    # isp_ai_cpu2_clk_bridge
    set_instance_parameter_value      isp_ai_cpu2_clk_bridge  EXPLICIT_CLOCK_RATE           {200000000.0}
    set_instance_parameter_value      isp_ai_cpu2_clk_bridge  NUM_CLOCK_OUTPUTS             {1}

    # isp_ai_cpu2_rst_bridge
    set_instance_parameter_value      isp_ai_cpu2_rst_bridge  ACTIVE_LOW_RESET              {0}
    set_instance_parameter_value      isp_ai_cpu2_rst_bridge  NUM_RESET_OUTPUTS             {1}
    set_instance_parameter_value      isp_ai_cpu2_rst_bridge  SYNCHRONOUS_EDGES             {deassert}
    set_instance_parameter_value      isp_ai_cpu2_rst_bridge  SYNC_RESET                    {0}
    set_instance_parameter_value      isp_ai_cpu2_rst_bridge  USE_RESET_REQUEST             {0}

    # isp_ai_cpu2_mm_bridge
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   ADDRESS_WIDTH                 {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   DATA_WIDTH                    {32}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   LINEWRAPBURSTS                {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   M0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   MAX_BURST_SIZE                {1}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   MAX_PENDING_RESPONSES         {4}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   MAX_PENDING_WRITES            {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   PIPELINE_COMMAND              {1}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   PIPELINE_RESPONSE             {1}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   S0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   SYMBOL_WIDTH                  {8}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   SYNC_RESET                    {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   USE_AUTO_ADDRESS_WIDTH        {1}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   USE_RESPONSE                  {0}
    set_instance_parameter_value      isp_ai_cpu2_mm_bridge   USE_WRITERESPONSE             {0}

    # isp_ai_cpu2_cc_bridge
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   ADDRESS_WIDTH                 {10}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   COMMAND_FIFO_DEPTH            {4}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   DATA_WIDTH                    {32}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   MASTER_SYNC_DEPTH             {2}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   MAX_BURST_SIZE                {1}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   RESPONSE_FIFO_DEPTH           {4}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   SLAVE_SYNC_DEPTH              {2}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   SYMBOL_WIDTH                  {8}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   SYNC_RESET                    {0}
    set_instance_parameter_value      isp_ai_cpu2_cc_bridge   USE_AUTO_ADDRESS_WIDTH        {1}

    # isp_ai_msg_q
    set_instance_parameter_value      isp_ai_msg_q      AXI_interface                       {1}
    set_instance_parameter_value      isp_ai_msg_q      allowInSystemMemoryContentEditor    {0}
    set_instance_parameter_value      isp_ai_msg_q      blockType                           {M20K}
    set_instance_parameter_value      isp_ai_msg_q      clockEnable                         {0}
    set_instance_parameter_value      isp_ai_msg_q      copyInitFile                        {0}
    set_instance_parameter_value      isp_ai_msg_q      dataWidth                           {32}
    set_instance_parameter_value      isp_ai_msg_q      dataWidth2                          {32}
    set_instance_parameter_value      isp_ai_msg_q      dualPort                            {1}
    set_instance_parameter_value      isp_ai_msg_q      ecc_check                           {0}
    set_instance_parameter_value      isp_ai_msg_q      ecc_encoder_bypass                  {0}
    set_instance_parameter_value      isp_ai_msg_q      ecc_pipeline_reg                    {0}
    set_instance_parameter_value      isp_ai_msg_q      enPRInitMode                        {0}
    set_instance_parameter_value      isp_ai_msg_q      enableDiffWidth                     {0}
    set_instance_parameter_value      isp_ai_msg_q      gui_debugaccess                     {0}
    set_instance_parameter_value      isp_ai_msg_q      idWidth                             {1}
    set_instance_parameter_value      isp_ai_msg_q      initMemContent                      {0}
    set_instance_parameter_value      isp_ai_msg_q      initializationFileName              {onchip_mem.hex}
    set_instance_parameter_value      isp_ai_msg_q      instanceID                          {NONE}
    set_instance_parameter_value      isp_ai_msg_q      interfaceType                       {0}
    set_instance_parameter_value      isp_ai_msg_q      lvl1OutputRegA                      {0}
    set_instance_parameter_value      isp_ai_msg_q      lvl1OutputRegB                      {0}
    set_instance_parameter_value      isp_ai_msg_q      lvl2OutputRegA                      {0}
    set_instance_parameter_value      isp_ai_msg_q      lvl2OutputRegB                      {0}
    set_instance_parameter_value      isp_ai_msg_q      memorySize                          {2048.0}
    set_instance_parameter_value      isp_ai_msg_q      poison_enable                       {0}
    set_instance_parameter_value      isp_ai_msg_q      readDuringWriteMode_Mixed           {DONT_CARE}
    set_instance_parameter_value      isp_ai_msg_q      resetrequest_enabled                {1}
    set_instance_parameter_value      isp_ai_msg_q      singleClockOperation                {0}
    set_instance_parameter_value      isp_ai_msg_q      tightly_coupled_ecc                 {0}
    set_instance_parameter_value      isp_ai_msg_q      useNonDefaultInitFile               {0}
    set_instance_parameter_value      isp_ai_msg_q      writable                            {1}

    # isp_ai_cpu_issue_irq
    set_instance_parameter_value      isp_ai_cpu_issue_irq      bitClearingEdgeCapReg       {0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      bitModifyingOutReg          {0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      captureEdge                 {0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      direction                   {Output}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      edgeType                    {RISING}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      generateIRQ                 {0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      irqType                     {LEVEL}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      resetValue                  {0.0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      simDoTestBenchWiring        {0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      simDrivenValue              {0.0}
    set_instance_parameter_value      isp_ai_cpu_issue_irq      width                       {1}

    # isp_ai_cpu_irq_for_cpu2
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     bitClearingEdgeCapReg     {1}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     bitModifyingOutReg        {0}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     captureEdge               {1}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     direction                 {Input}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     edgeType                  {RISING}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     generateIRQ               {1}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     irqType                   {EDGE}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     resetValue                {0.0}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     simDoTestBenchWiring      {0}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     simDrivenValue            {0.0}
    set_instance_parameter_value      isp_ai_cpu_irq_for_cpu2     width                     {1}

    # isp_ai_cpu2_issue_irq
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       bitClearingEdgeCapReg     {0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       bitModifyingOutReg        {0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       captureEdge               {0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       direction                 {Output}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       edgeType                  {RISING}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       generateIRQ               {0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       irqType                   {LEVEL}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       resetValue                {0.0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       simDoTestBenchWiring      {0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       simDrivenValue            {0.0}
    set_instance_parameter_value      isp_ai_cpu2_issue_irq       width                     {1}

    # isp_ai_cpu2_irq_for_cpu
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     bitClearingEdgeCapReg     {1}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     bitModifyingOutReg        {0}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     captureEdge               {1}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     direction                 {Input}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     edgeType                  {RISING}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     generateIRQ               {1}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     irqType                   {EDGE}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     resetValue                {0.0}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     simDoTestBenchWiring      {0}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     simDrivenValue            {0.0}
    set_instance_parameter_value      isp_ai_cpu2_irq_for_cpu     width                     {1}


    ############################
    #### Create Connections ####
    ############################

    # isp_ai_cpu_clk_bridge
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_cpu_rst_bridge.clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_mm_bridge.clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_vfw_fres.control_clock
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_vfr_fres.control_clock
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_scaler_down.agent_clock
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_dla_lt.agent_clock
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_vfw_lres.control_clock
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_irq_cdc_0.sender_clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_irq_cdc_1.sender_clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_irq_cdc_2.sender_clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_cpu2_cc_bridge.m0_clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_msg_q.clk1
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_cpu_issue_irq.clk
    add_connection      isp_ai_cpu_clk_bridge.out_clk           isp_ai_cpu2_irq_for_cpu.clk

    # isp_ai_cpu_rst_bridge
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_mm_bridge.reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_vfw_fres.control_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_vfr_fres.control_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_scaler_down.agent_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_dla_lt.agent_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_vfw_lres.control_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_irq_cdc_0.sender_clk_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_irq_cdc_1.sender_clk_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_irq_cdc_2.sender_clk_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_cpu2_cc_bridge.m0_reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_msg_q.reset1
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_cpu_issue_irq.reset
    add_connection      isp_ai_cpu_rst_bridge.out_reset         isp_ai_cpu2_irq_for_cpu.reset

    # isp_ai_vid_clk_bridge
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_vid_rst_bridge.clk
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_axi4s_bcast.vid_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_vfw_fres.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_vfr_fres.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_pix_adapt.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_scaler_down.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_dla_lt.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_fifo_dla_lt.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_vfw_lres.main_clock
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_irq_cdc_0.receiver_clk
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_irq_cdc_1.receiver_clk
    add_connection      isp_ai_vid_clk_bridge.out_clk           isp_ai_irq_cdc_2.receiver_clk

    # isp_ai_vid_rst_bridge
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_axi4s_bcast.vid_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_vfw_fres.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_vfr_fres.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_pix_adapt.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_scaler_down.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_dla_lt.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_fifo_dla_lt.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_vfw_lres.main_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_irq_cdc_0.receiver_clk_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_irq_cdc_1.receiver_clk_reset
    add_connection      isp_ai_vid_rst_bridge.out_reset         isp_ai_irq_cdc_2.receiver_clk_reset

    # isp_ai_emif_clk_bridge
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_emif_rst_bridge.clk
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_vfw_fres.mem_clock
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_se_vfw_fres.clock
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_vfr_fres.mem_clock
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_se_vfr_fres.clock
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_vfw_lres.mem_clock
    add_connection      isp_ai_emif_clk_bridge.out_clk          isp_ai_se_vfw_lres.clock

    # isp_ai_emif_rst_bridge
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_vfw_fres.mem_reset
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_se_vfw_fres.reset
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_vfr_fres.mem_reset
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_se_vfr_fres.reset
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_vfw_lres.mem_reset
    add_connection      isp_ai_emif_rst_bridge.out_reset        isp_ai_se_vfw_lres.reset

    # isp_ai_mm_bridge
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_vfw_fres.av_mm_control_agent
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_vfr_fres.av_mm_control_agent
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_scaler_down.av_mm_control_agent
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_dla_lt.av_mm_control_agent
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_vfw_lres.av_mm_control_agent
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_msg_q.s1
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_cpu_issue_irq.s1
    add_connection      isp_ai_mm_bridge.m0                     isp_ai_cpu2_irq_for_cpu.s1

    # isp_ai_axi4s_bcast
    add_connection          isp_ai_axi4s_bcast.axi4s_vid_out_1      isp_ai_pix_adapt.axi4s_vid_in

    # isp_ai_axi4s_bcast
    add_connection          isp_ai_axi4s_bcast.axi4s_vid_out_0      isp_ai_vfw_fres.axi4s_vid_in

    # isp_ai_vfw_fres
    add_connection          isp_ai_vfw_fres.av_mm_mem_write_host    isp_ai_se_vfw_fres.windowed_slave

    # isp_ai_vfr_fres
    add_connection          isp_ai_vfr_fres.av_mm_mem_read_host     isp_ai_se_vfr_fres.windowed_slave

    # isp_ai_pix_adapt
    add_connection          isp_ai_pix_adapt.axi4s_vid_out          isp_ai_scaler_down.axi4s_vid_in

    # isp_ai_scaler_down
    add_connection          isp_ai_scaler_down.axi4s_vid_out        isp_ai_dla_lt.axi4s_vid_in

    # isp_ai_dla_lt
    add_connection          isp_ai_dla_lt.axi4s_vid_out             isp_ai_vfw_lres.axi4s_vid_in
    add_connection          isp_ai_dla_lt.fifo_reset                isp_ai_fifo_dla_lt.main_reset
    add_connection          isp_ai_dla_lt.axi4s_vid_fifo_out        isp_ai_fifo_dla_lt.axi4s_vid_in

    # isp_ai_fifo_dla_lt
    add_connection          isp_ai_fifo_dla_lt.axi4s_vid_out        isp_ai_dla_lt.axi4s_vid_fifo_in

    # isp_ai_vfw_lres
    add_connection          isp_ai_vfw_lres.av_mm_mem_write_host    isp_ai_se_vfw_lres.windowed_slave

    # isp_ai_irq_cdc
    add_connection          isp_ai_irq_cdc_0.receiver               isp_ai_vfw_fres.frame_writer_int
    add_connection          isp_ai_irq_cdc_1.receiver               isp_ai_vfr_fres.frame_reader_int
    add_connection          isp_ai_irq_cdc_2.receiver               isp_ai_vfw_lres.frame_writer_int

    # message queue
    # isp_ai_cpu2_clk_bridge
    add_connection          isp_ai_cpu2_clk_bridge.out_clk          isp_ai_cpu2_rst_bridge.clk
    add_connection          isp_ai_cpu2_clk_bridge.out_clk          isp_ai_cpu2_mm_bridge.clk
    add_connection          isp_ai_cpu2_clk_bridge.out_clk          isp_ai_cpu2_cc_bridge.s0_clk
    add_connection          isp_ai_cpu2_clk_bridge.out_clk          isp_ai_cpu_irq_for_cpu2.clk
    add_connection          isp_ai_cpu2_clk_bridge.out_clk          isp_ai_cpu2_issue_irq.clk

    # isp_ai_cpu2_rst_bridge
    add_connection          isp_ai_cpu2_rst_bridge.out_reset        isp_ai_cpu2_mm_bridge.reset
    add_connection          isp_ai_cpu2_rst_bridge.out_reset        isp_ai_cpu2_cc_bridge.s0_reset
    add_connection          isp_ai_cpu2_rst_bridge.out_reset        isp_ai_cpu_irq_for_cpu2.reset
    add_connection          isp_ai_cpu2_rst_bridge.out_reset        isp_ai_cpu2_issue_irq.reset

    # isp_ai_cpu2_mm_bridge
    add_connection          isp_ai_cpu2_mm_bridge.m0                isp_ai_cpu2_cc_bridge.s0
    add_connection          isp_ai_cpu2_mm_bridge.m0                isp_ai_cpu_irq_for_cpu2.s1
    add_connection          isp_ai_cpu2_mm_bridge.m0                isp_ai_cpu2_issue_irq.s1

    # isp_ai_cpu2_cc_bridge
    add_connection          isp_ai_cpu2_cc_bridge.m0                isp_ai_msg_q.s2

    # isp_ai_cpu_issue_irq
    add_connection          isp_ai_cpu_issue_irq.external_connection  isp_ai_cpu_irq_for_cpu2.external_connection

    # isp_ai_cpu2_issue_irq
    add_connection          isp_ai_cpu2_issue_irq.external_connection isp_ai_cpu2_irq_for_cpu.external_connection

    ##########################
    ##### Create Exports #####
    ##########################

    # isp_ai_cpu_clk_bridge
    add_interface           cpu_clk_in              clock       sink
    set_interface_property  cpu_clk_in              EXPORT_OF   isp_ai_cpu_clk_bridge.in_clk

    # isp_ai_cpu_rst_bridge
    add_interface           cpu_rst_in              reset       sink
    set_interface_property  cpu_rst_in              EXPORT_OF   isp_ai_cpu_rst_bridge.in_reset

    # isp_ai_vid_clk_bridge
    set_interface_property  vid_clk_in              EXPORT_OF   isp_ai_vid_clk_bridge.in_clk

    # isp_ai_vid_rst_bridge
    set_interface_property  vid_rst_in              EXPORT_OF   isp_ai_vid_rst_bridge.in_reset

    # isp_ai_emif_clk_bridge
    set_interface_property  emif_clk_in             EXPORT_OF   isp_ai_emif_clk_bridge.in_clk

    # isp_ai_emif_rst_bridge
    set_interface_property  emif_rst_in             EXPORT_OF   isp_ai_emif_rst_bridge.in_reset

    # isp_ai_mm_bridge
    add_interface           mm_ctrl_in              avalon      slave
    set_interface_property  mm_ctrl_in              EXPORT_OF   isp_ai_mm_bridge.s0

    # isp_ai_axi4s_bcast
    add_interface           isp_out_s_vid_axis      axi4stream  subordinate
    set_interface_property  isp_out_s_vid_axis      EXPORT_OF   isp_ai_axi4s_bcast.axi4s_vid_in

    # isp_ai_vfw_fres
    # isp_ai_irq_cdc
    set_interface_property    fres_vfw_int          EXPORT_OF   isp_ai_irq_cdc_0.sender
    set_interface_property    fres_vfr_int          EXPORT_OF   isp_ai_irq_cdc_1.sender
    set_interface_property    lres_vfw_int          EXPORT_OF   isp_ai_irq_cdc_2.sender

    # isp_ai_se_vfw_fres
    set_interface_property  av_mm_host_fres_vfw     EXPORT_OF   isp_ai_se_vfw_fres.expanded_master

    # isp_ai_vfr_fres
    add_interface           ai_out_m_vid_axis       axi4stream  manager
    set_interface_property  ai_out_m_vid_axis       EXPORT_OF   isp_ai_vfr_fres.axi4s_vid_out

    # isp_ai_se_vfr_fres
    set_interface_property  av_mm_host_fres_vfr     EXPORT_OF   isp_ai_se_vfr_fres.expanded_master

    # isp_ai_se_vfw_lres
    set_interface_property  av_mm_host_lres_vfw     EXPORT_OF   isp_ai_se_vfw_lres.expanded_master

    # Message Queue
    # isp_ai_cpu2_clk_bridge
    add_interface           cpu2_clk_in             clock       sink
    set_interface_property  cpu2_clk_in             EXPORT_OF   isp_ai_cpu2_clk_bridge.in_clk

    # isp_ai_cpu2_rst_bridge
    add_interface           cpu2_rst_in             reset       sink
    set_interface_property  cpu2_rst_in             EXPORT_OF   isp_ai_cpu2_rst_bridge.in_reset

    # isp_ai_cpu2_mm_bridge
    add_interface           cpu2_mm_ctrl_in         avalon      slave
    set_interface_property  cpu2_mm_ctrl_in         EXPORT_OF   isp_ai_cpu2_mm_bridge.s0

    # isp_ai_cpu_irq_for_cpu2
    set_interface_property  cpu2_int                EXPORT_OF   isp_ai_cpu_irq_for_cpu2.irq

    # isp_ai_cpu2_irq_for_cpu
    set_interface_property  cpu_int                 EXPORT_OF   isp_ai_cpu2_irq_for_cpu.irq


    #################################
    ##### Assign Base Addresses #####
    #################################

    set v_vfw_fres_addr_offset          [get_shell_parameter VFW_FRES_ADDR_OFFSET]
    set v_vfr_fres_addr_offset          [get_shell_parameter VFR_FRES_ADDR_OFFSET]
    set v_scale_down_addr_offset        [get_shell_parameter SCALE_DOWN_ADDR_OFFSET]
    set v_dla_lt_addr_offset            [get_shell_parameter DLA_LT_ADDR_OFFSET]
    set v_vfw_lres_addr_offset          [get_shell_parameter VFW_LRES_ADDR_OFFSET]
    set v_msg_q_addr_offset             [get_shell_parameter MSG_Q_ADDR_OFFSET]
    set v_cpu_issue_irq_addr_offset     [get_shell_parameter CPU_ISSUE_IRQ_ADDR_OFFSET]
    set v_cpu2_to_cpu_irq_addr_offset   [get_shell_parameter CPU2_TO_CPU_IRQ_ADDR_OFFSET]

    set v_onchipmem_ai_message_queue_base_address         [get_shell_parameter MSG_Q_BASE_ADDR]
    set v_pio_cpu_irq_for_cpu2_base_address               [get_shell_parameter IRQ_FOR_HPS_BASE_ADDR]
    set v_pio_cpu2_issue_irq_base_address                 [get_shell_parameter HPS_IRQ_GEN_BASE_ADDR]

    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_vfw_fres.av_mm_control_agent \
                                                                baseAddress ${v_vfw_fres_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_vfr_fres.av_mm_control_agent \
                                                                baseAddress ${v_vfr_fres_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_scaler_down.av_mm_control_agent \
                                                                baseAddress ${v_scale_down_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_dla_lt.av_mm_control_agent \
                                                                baseAddress ${v_dla_lt_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_vfw_lres.av_mm_control_agent \
                                                                baseAddress ${v_vfw_lres_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_msg_q.s1 \
                                                                baseAddress ${v_msg_q_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_cpu_issue_irq.s1 \
                                                                baseAddress ${v_cpu_issue_irq_addr_offset}
    set_connection_parameter_value isp_ai_mm_bridge.m0/isp_ai_cpu2_irq_for_cpu.s1 \
                                                                baseAddress ${v_cpu2_to_cpu_irq_addr_offset}

    set_connection_parameter_value isp_ai_cpu2_cc_bridge.m0/isp_ai_msg_q.s2 \
                                                        baseAddress ${v_onchipmem_ai_message_queue_base_address}

    set_connection_parameter_value isp_ai_cpu2_mm_bridge.m0/isp_ai_cpu2_cc_bridge.s0 \
                                                        baseAddress ${v_onchipmem_ai_message_queue_base_address}
    set_connection_parameter_value isp_ai_cpu2_mm_bridge.m0/isp_ai_cpu_irq_for_cpu2.s1 \
                                                        baseAddress ${v_pio_cpu_irq_for_cpu2_base_address}
    set_connection_parameter_value isp_ai_cpu2_mm_bridge.m0/isp_ai_cpu2_issue_irq.s1 \
                                                        baseAddress ${v_pio_cpu2_issue_irq_base_address}

    lock_avalon_base_address  isp_ai_vfw_fres.av_mm_control_agent
    lock_avalon_base_address  isp_ai_vfr_fres.av_mm_control_agent
    lock_avalon_base_address  isp_ai_scaler_down.av_mm_control_agent
    lock_avalon_base_address  isp_ai_dla_lt.av_mm_control_agent
    lock_avalon_base_address  isp_ai_vfw_lres.av_mm_control_agent
    lock_avalon_base_address  isp_ai_msg_q.s1
    lock_avalon_base_address  isp_ai_cpu_issue_irq.s1
    lock_avalon_base_address  isp_ai_cpu2_irq_for_cpu.s1

    lock_avalon_base_address  isp_ai_msg_q.s2

    lock_avalon_base_address  isp_ai_cpu2_cc_bridge.s0
    lock_avalon_base_address  isp_ai_cpu_irq_for_cpu2.s1
    lock_avalon_base_address  isp_ai_cpu2_issue_irq.s1

    #############################
    ##### Sync / Validation #####
    #############################

    sync_sysinfo_parameters
    save_system
}

proc modify_manual_ocs_rom {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_drv_ocs_subsystem_name    [get_shell_parameter DRV_OCS_SUBSYSTEM_NAME]
    set v_inst_id                   [get_shell_parameter INST_ID]

    if {${v_drv_ocs_subsystem_name} != ""} {
      load_system ${v_project_path}/rtl/shell/${v_drv_ocs_subsystem_name}.qsys

      load_component hps_intel_offset_capability_manual

      set v_num_caps  [get_component_parameter_value C_NUM_CAPS]

      # check if the capability contains a dummy entry (required for ip instantiation)
      if {${v_num_caps} == 1} {
        set v_cap0_type   [get_component_parameter_value C_CAP0_TYPE]

        if {${v_cap0_type} == 0} {
          set v_num_caps 0
        }
      }

      set_component_parameter_value  C_NUM_CAPS [expr ${v_num_caps} + 3]

      set v_onchipmem_ai_message_queue_address \
                [expr [get_shell_parameter SS_BASE_ADDR] + [get_shell_parameter MSG_Q_BASE_ADDR]]
      set v_pio_cpu_irq_for_cpu2_address \
                [expr [get_shell_parameter SS_BASE_ADDR] + [get_shell_parameter IRQ_FOR_HPS_BASE_ADDR]]
      set v_pio_cpu2_issue_irq_address \
                [expr [get_shell_parameter SS_BASE_ADDR] + [get_shell_parameter HPS_IRQ_GEN_BASE_ADDR]]

      set v_index ${v_num_caps}

      # manual On-Chip Memory II - message queue
      set_component_parameter_value   C_CAP${v_index}_BASE          ${v_onchipmem_ai_message_queue_address}
      set_component_parameter_value   C_CAP${v_index}_SIZE          {512}
      set_component_parameter_value   C_CAP${v_index}_TYPE          {1004}
      set_component_parameter_value   C_CAP${v_index}_VERSION       {1}
      set_component_parameter_value   C_CAP${v_index}_ID_COMPONENT  ${v_inst_id}

      set v_index [incr v_index]

      # manual pio - Generate an output for IRQ generation
      set_component_parameter_value   C_CAP${v_index}_BASE          ${v_pio_cpu_irq_for_cpu2_address}
      set_component_parameter_value   C_CAP${v_index}_SIZE          {16}
      set_component_parameter_value   C_CAP${v_index}_TYPE          {528}
      set_component_parameter_value   C_CAP${v_index}_VERSION       {1}
      set_component_parameter_value   C_CAP${v_index}_ID_COMPONENT  ${v_inst_id}

      set v_index [incr v_index]

      # manual pio - Generate an input capture for IRQ generation
      set_component_parameter_value   C_CAP${v_index}_BASE          ${v_pio_cpu2_issue_irq_address}
      set_component_parameter_value   C_CAP${v_index}_SIZE          {16}
      set_component_parameter_value   C_CAP${v_index}_TYPE          {528}
      set_component_parameter_value   C_CAP${v_index}_VERSION       {1}
      set_component_parameter_value   C_CAP${v_index}_ID_COMPONENT  [expr ${v_inst_id} + 1]

      save_component

      sync_sysinfo_parameters
      save_system
    }
}

proc edit_top_level_qsys {} {
    set v_project_name        [get_shell_parameter PROJECT_NAME]
    set v_project_path        [get_shell_parameter PROJECT_PATH]
    set v_instance_name       [get_shell_parameter INSTANCE_NAME]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name}   ${v_instance_name}

    sync_sysinfo_parameters
    save_system
}

proc add_auto_connections {} {
    set v_instance_name               [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host                   [get_shell_parameter AVMM_HOST]
    set v_avmm_host_2                 [get_shell_parameter AVMM_HOST_2]
    set v_fres_vfw_irq_priority       [get_shell_parameter FRES_VFW_IRQ_PRIORITY]
    set v_fres_vfw_irq_host           [get_shell_parameter FRES_VFW_IRQ_HOST]
    set v_fres_vfr_irq_priority       [get_shell_parameter FRES_VFR_IRQ_PRIORITY]
    set v_fres_vfr_irq_host           [get_shell_parameter FRES_VFR_IRQ_HOST]
    set v_lres_vfw_irq_priority       [get_shell_parameter LRES_VFW_IRQ_PRIORITY]
    set v_lres_vfw_irq_host           [get_shell_parameter LRES_VFW_IRQ_HOST]
    set v_msg_nios_irq_priority       [get_shell_parameter MSG_NIOS_IRQ_PRIORITY]
    set v_msg_nios_irq_host           [get_shell_parameter MSG_NIOS_IRQ_HOST]
    set v_msg_hps_irq_priority        [get_shell_parameter MSG_HPS_IRQ_PRIORITY]
    set v_msg_hps_irq_host            [get_shell_parameter MSG_HPS_IRQ_HOST]

    set v_emif_agent          [get_shell_parameter EMIF_AGENT]

    add_auto_connection   ${v_instance_name}    cpu_clk_in        100000000
    add_auto_connection   ${v_instance_name}    cpu_rst_in        100000000

    add_auto_connection   ${v_instance_name}    cpu2_clk_in       200000000
    add_auto_connection   ${v_instance_name}    cpu2_rst_in       200000000

    add_auto_connection   ${v_instance_name}    vid_clk_in        297000000
    add_auto_connection   ${v_instance_name}    vid_rst_in        297000000

    add_auto_connection   ${v_instance_name}    emif_clk_in       emif_user_clk
    add_auto_connection   ${v_instance_name}    emif_rst_in       emif_user_rst

    # frame readers/writers to DDR4
    add_auto_connection   ${v_instance_name}    av_mm_host_fres_vfw       ${v_emif_agent}_user_data
    add_auto_connection   ${v_instance_name}    av_mm_host_fres_vfr       ${v_emif_agent}_user_data

    add_auto_connection   ${v_instance_name}    av_mm_host_lres_vfw       ${v_emif_agent}_user_data

    # from isp
    add_auto_connection   ${v_instance_name}    isp_out_s_vid_axis        isp_out_vid_axis

    # to vid out
    add_auto_connection   ${v_instance_name}    ai_out_m_vid_axis         full_isp_out_vid_axis

    # NIOS to mm bridge
    add_avmm_connections  mm_ctrl_in          ${v_avmm_host}

    # HPS to mm bridge
    add_avmm_connections  cpu2_mm_ctrl_in     ${v_avmm_host_2}

    # Interrupts
    if {(${v_fres_vfw_irq_host} != "NONE") && (${v_fres_vfw_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "fres_vfw_int" \
                                          ${v_fres_vfw_irq_priority}  ${v_fres_vfw_irq_host}_irq
    }
    if {(${v_fres_vfr_irq_host} != "NONE") && (${v_fres_vfr_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "fres_vfr_int" \
                                          ${v_fres_vfr_irq_priority}  ${v_fres_vfr_irq_host}_irq
    }
    if {(${v_lres_vfw_irq_host} != "NONE") && (${v_lres_vfw_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "lres_vfw_int" \
                                          ${v_lres_vfw_irq_priority}  ${v_lres_vfw_irq_host}_irq
    }
    if {(${v_msg_nios_irq_host} != "NONE") && (${v_msg_nios_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "cpu_int" \
                                        ${v_msg_nios_irq_priority}  ${v_msg_nios_irq_host}_irq
    }
    if {(${v_msg_hps_irq_host} != "NONE") && (${v_msg_hps_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "cpu2_int" \
                                        ${v_msg_hps_irq_priority}  ${v_msg_hps_irq_host}_irq
    }
}
