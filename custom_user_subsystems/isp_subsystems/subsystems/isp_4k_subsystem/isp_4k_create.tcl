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

set_shell_parameter AVMM_HOST                 {{AUTO X}}
set_shell_parameter AVMM_HOST_CLK_FREQ        {200000000.0}

    # IP AVMM Offset Addresses
set_shell_parameter WARP_ADDR_OFFSET          "0x00000000"
set_shell_parameter 3D_LUT_HDR_2_ADDR_OFFSET  "0x00008200"
set_shell_parameter BLC_ADDR_OFFSET           "0x00008a00"
set_shell_parameter CLIPPER_ADDR_OFFSET       "0x00008e00"
set_shell_parameter DPC_ADDR_OFFSET           "0x00008c00"
set_shell_parameter TMO_ADDR_OFFSET           "0x00009000"
set_shell_parameter DMS_ADDR_OFFSET           "0x00009200"
set_shell_parameter WBC_ADDR_OFFSET           "0x00009400"
set_shell_parameter BLS_ADDR_OFFSET           "0x00009600"
set_shell_parameter USM_ADDR_OFFSET           "0x00009800"
set_shell_parameter CCM_ADDR_OFFSET           "0x00009c00"
set_shell_parameter 3D_LUT_HDR_1_ADDR_OFFSET  "0x00009a00"
set_shell_parameter WARP_THROTTLE_ADDR_OFFSET "0x0000a400"
set_shell_parameter HS_ADDR_OFFSET            "0x0000b000"
set_shell_parameter WBS_ADDR_OFFSET           "0x0000c000"
set_shell_parameter SW_WBS_ADDR_OFFSET        "0x0000d000"
set_shell_parameter VFW_ADDR_OFFSET           "0x0000d400"
set_shell_parameter ROI_ADDR_OFFSET           "0x0000d800"
set_shell_parameter CPM_ADDR_OFFSET           "0x0000da00"
set_shell_parameter ANR_ADDR_OFFSET           "0x00010000"
set_shell_parameter 1D_LUT_ADDR_OFFSET        "0x00020000"
set_shell_parameter VC_ADDR_OFFSET            "0x00040000"

# General Video Controls
set_shell_parameter VID_CLK_FREQ              {297000000.0}
set_shell_parameter PIP                       {2}
set_shell_parameter EN_DEBUG                  {1}

set_shell_parameter INST_ID                   {0}

# HDR Controls
set_shell_parameter EXP_FUSION_EN             {0}
set_shell_parameter HDR_EN                    {0}

# Warp Controls
set_shell_parameter WARP_EN                   {1}
set_shell_parameter WARP_SB                   {0}
set_shell_parameter WARP_MM                   {1}
set_shell_parameter WARP_CACHE                {512}

# AI Controls
set_shell_parameter AI_EN                     {0}

# 3D LUT Controls.
set_shell_parameter 3DLUT_EN                  {1}
set_shell_parameter SMALL_3DLUT               {0}
set_shell_parameter LUT_PIP_SHARING_EN        {0}
set_shell_parameter LUT_DOUBLE_BUFFERED_EN    {0}
# Enable to preload a default cube file
set_shell_parameter PRESET_FILE_3DLUT_EN      {0}
# For HDR this would be a sRGB to HLG function
set_shell_parameter PRESET_FILE_3DLUT_0_0     {}
set_shell_parameter PRESET_FILE_3DLUT_0_1     {}
# For HDR this would be a HLG to BT709 function
set_shell_parameter PRESET_FILE_3DLUT_1_0     {}
set_shell_parameter PRESET_FILE_3DLUT_1_1     {}

# Other Controls.
set_shell_parameter ASYNC_CLK                 {0}
set_shell_parameter EMIF_AGENT                {}
set_shell_parameter EMIF_AGENT_CLK_FREQ       {200000000.0}
set_shell_parameter RAW_SNAPSHOT_EN           {0}
set_shell_parameter EMIF_AGENT_2              {}
set_shell_parameter EMIF_AGENT_2_CLK_FREQ     {200000000.0}
set_shell_parameter SMALL_VC                  {1}
set_shell_parameter SMALL_ANR                 {0}
set_shell_parameter BLS_EN                    {1}
set_shell_parameter CLIPPER_EN                {1}

set_shell_parameter TMO_NIOS_CLK_FREQ         {148500000.0}


# resolve interdependencies
proc derive_parameters {param_array} {
    upvar $param_array p_array

    # check if the emif agent has been configured
    set v_warp_en             [get_shell_parameter WARP_EN]
    set v_emif_agent          [get_shell_parameter EMIF_AGENT]

    if {${v_warp_en} && [llength ${v_emif_agent}] == 0} {
        send_message ERROR "isp_4k_create: Warp EMIF Agent not specified"
    }

    set v_raw_snapshot_en     [get_shell_parameter RAW_SNAPSHOT_EN]
    set v_emif_agent_2        [get_shell_parameter EMIF_AGENT_2]

    if {${v_raw_snapshot_en} && [llength ${v_emif_agent_2}] == 0} {
        send_message ERROR "isp_4k_create: Snapshot EMIF Agent 2 not specified"
    }
  }

proc pre_creation_step {} {
    transfer_files
    evaluate_terp
}

proc creation_step {} {
    create_isp_subsystem
}

proc post_creation_step {} {
    edit_top_level_qsys
    add_auto_connections
}

proc post_connection_step {} {
    set v_warp_en               [get_shell_parameter WARP_EN]

    if {${v_warp_en}} {
        modify_avmm_arbitration
    }
}


proc transfer_files {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_script_path               [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_lut_double_buffered_en    [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en     [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_0_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_0_0]
    set v_3d_lut_0_1_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_0_1]
    set v_3d_lut_1_0_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_1_0]
    set v_3d_lut_1_1_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_1_1]
    set v_ai_en                     [get_shell_parameter AI_EN]

    # 3D LUT Init Files
    if {${v_3d_lut_preset_file_en}} {
        file_copy ${v_script_path}/${v_3d_lut_0_0_preset_file} \
                                              ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_0_preset_file}
        file_copy ${v_script_path}/${v_3d_lut_1_0_preset_file} \
                                              ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_0_preset_file}
        if {${v_lut_double_buffered_en}} {
            file_copy ${v_script_path}/${v_3d_lut_0_1_preset_file} \
                                                  ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_1_preset_file}
            file_copy ${v_script_path}/${v_3d_lut_1_1_preset_file} \
                                                  ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_1_preset_file}
        }
        file_copy ${v_script_path}/3D_LUT_preset_script.tcl.terp \
                                              ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp
        file_copy ${v_script_path}/3D_LUT_subsystem.qsf.terp \
                                              ${v_project_path}/quartus/user/${v_instance_name}.qsf.terp
    }

    # Copy ROI
    exec cp -rf ${v_script_path}/../../non_qpds_ip/altera_vvp_roi                 ${v_project_path}/non_qpds_ip/user
    file_copy   ${v_script_path}/../../non_qpds_ip/altera_vvp_roi.ipx             ${v_project_path}/non_qpds_ip/user
}


proc evaluate_terp {} {
    set v_project_name              [get_shell_parameter PROJECT_NAME]
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_lut_double_buffered_en    [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en     [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_0_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_0_0]
    set v_3d_lut_0_1_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_0_1]
    set v_3d_lut_1_0_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_1_0]
    set v_3d_lut_1_1_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_1_1]

    # 3D LUT Init Files
    if {${v_3d_lut_preset_file_en}} {
        evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp \
            [list ${v_project_name} ${v_instance_name} ${v_3d_lut_0_0_preset_file} ${v_3d_lut_1_0_preset_file}] 0 1
        if {${v_lut_double_buffered_en}} {
            evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp \
            [list ${v_project_name} ${v_instance_name} ${v_3d_lut_0_1_preset_file} ${v_3d_lut_1_1_preset_file}] 0 1
        }
        evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}.qsf.terp \
            [list ${v_instance_name}] 0 1
    }
}


proc create_isp_subsystem {} {
    set v_project_path            [get_shell_parameter PROJECT_PATH]
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]

    # Video Pipeline
    set v_vid_clk_freq            [get_shell_parameter VID_CLK_FREQ]
    set v_cppp                    {3}
    set v_pip                     [get_shell_parameter PIP]

    # EMIFs
    set v_emif_agent_clk_freq     [get_shell_parameter EMIF_AGENT_CLK_FREQ]
    set v_emif_agent_2_clk_freq   [get_shell_parameter EMIF_AGENT_2_CLK_FREQ]

    # Warp
    set v_warp_en                 [get_shell_parameter WARP_EN]
    if {${v_pip} > 1} {
        set v_num_warp_engines        {2}
    } else {
        set v_num_warp_engines        {1}
    }
    set v_warp_debug              [get_shell_parameter EN_DEBUG]
    set v_warp_single_bounce      [get_shell_parameter WARP_SB]
    set v_warp_mipmap_enable      [get_shell_parameter WARP_MM]
    set v_warp_cache_blocks       [get_shell_parameter WARP_CACHE]

    # HDR
    set v_exp_fusion_en           [get_shell_parameter EXP_FUSION_EN]
    set v_hdr_en                  [get_shell_parameter HDR_EN]

    # ISP Pipeline
    set v_isp_cppp                {1}
    if {${v_exp_fusion_en}} {
        set v_isp_bps                 {16}
        set v_isp_bps_cut             {14}
    } else {
        set v_isp_bps                 {12}
        set v_isp_bps_cut             {12}
    }
    set v_tmo_bps                 {12}
    set v_usm_bps                 {10}
    set v_small_vc                [get_shell_parameter SMALL_VC]
    set v_small_anr               [get_shell_parameter SMALL_ANR]
    set v_bls_en                  [get_shell_parameter BLS_EN]
    set v_clipper_en              [get_shell_parameter CLIPPER_EN]

    # 3D LUT
    if {${v_hdr_en}} {
        set v_3d_lut_en               {1}
    } else {
        set v_3d_lut_en               [get_shell_parameter 3DLUT_EN]
    }
    set v_small_3d_lut            [get_shell_parameter SMALL_3DLUT]
    set v_lut_pip_sharing_en      [get_shell_parameter LUT_PIP_SHARING_EN]
    set v_lut_double_buffered_en  [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en   [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_0_preset_file  [get_shell_parameter PRESET_FILE_3DLUT_0_0]
    set v_3d_lut_0_1_preset_file  [get_shell_parameter PRESET_FILE_3DLUT_0_1]
    set v_3d_lut_1_0_preset_file  [get_shell_parameter PRESET_FILE_3DLUT_1_0]
    set v_3d_lut_1_1_preset_file  [get_shell_parameter PRESET_FILE_3DLUT_1_1]

    # AI Pipeline
    set v_ai_en                   [get_shell_parameter AI_EN]

    # Others
    set v_raw_snapshot_en         [get_shell_parameter RAW_SNAPSHOT_EN]

    # General
    set v_inst_id                 [get_shell_parameter INST_ID]
    set v_enable_debug            [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready          {1}

    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_tmo_nios_clk_freq       [get_shell_parameter TMO_NIOS_CLK_FREQ]

    # Switch Mode - Crash when ASYNC_IP_SW = 1, else Sync Switch on SOF with SYNC_IP_SW = 1, else on EOL
    set v_wbs_async_ip_sw         {0}
    set v_wbs_sync_ip_sw          {1}


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  isp_cpu_clk               altera_clock_bridge
    add_instance  isp_cpu_rst               altera_reset_bridge
    add_instance  isp_mm_bridge             altera_avalon_mm_bridge
    add_instance  isp_vid_clk               altera_clock_bridge
    add_instance  isp_vid_rst               altera_reset_bridge
    add_instance  isp_niosv_clk             altera_clock_bridge
    add_instance  isp_niosv_rst             altera_reset_bridge
    if {${v_warp_en}} {
        add_instance  isp_emif_clk              altera_clock_bridge
        add_instance  isp_emif_rst              altera_reset_bridge
    }
    if {${v_raw_snapshot_en}} {
        add_instance  isp_emif_2_clk            altera_clock_bridge
        add_instance  isp_emif_2_rst            altera_reset_bridge
        add_instance  isp_axi4s_bcast           intel_vvp_axi4s_broadcaster
        add_instance  isp_cpm                   intel_vvp_cpm
        add_instance  isp_vfw                   intel_vvp_vfw
        add_instance  isp_se_vfw                altera_address_span_extender
    }
    if {${v_bls_en}} {
        add_instance  isp_bls                   intel_vvp_bls
    }
    if {${v_clipper_en}} {
        add_instance  isp_clipper               intel_vvp_clipper
    }
    add_instance  isp_dpc                   intel_vvp_dpc
    add_instance  isp_anr                   intel_vvp_anr
    add_instance  isp_blc                   intel_vvp_blc
    add_instance  isp_vc                    intel_vvp_vc
    add_instance  isp_switch_wbs            intel_vvp_switch
    add_instance  isp_wbc                   intel_vvp_wbc
    add_instance  isp_wbs                   intel_vvp_wbs
    add_instance  isp_dms                   intel_vvp_demosaic
    add_instance  isp_hs                    intel_vvp_hs
    add_instance  isp_ccm                   intel_vvp_csc
    if {${v_hdr_en}} {
        add_instance  isp_1d_lut_hdr          intel_vvp_1d_lut
        add_instance  isp_3d_lut_hdr_1        intel_vvp_3d_lut
    }
    if {${v_3d_lut_en}} {
        add_instance  isp_3d_lut_hdr_2          intel_vvp_3d_lut
    }
    add_instance  isp_tmo                   intel_vvp_tmo
    add_instance  isp_pix_adapt_tmo         intel_vvp_pixel_adapter
    add_instance  isp_usm                   intel_vvp_usm
    add_instance  isp_roi                   altera_vvp_roi
    if {${v_warp_en}} {
        add_instance  isp_warp                intel_vvp_warp
        add_instance  isp_se_warp             altera_address_span_extender
        if {${v_ai_en}} {
            add_instance  isp_warp_throttle   altera_vvp_throttle
        }
    }


    ############################
    #### Set Parameters     ####
    ############################

    # isp_cpu_clk
    set_instance_parameter_value      isp_cpu_clk    EXPLICIT_CLOCK_RATE       ${v_avmm_host_clk_freq}
    set_instance_parameter_value      isp_cpu_clk    NUM_CLOCK_OUTPUTS         {1}

    # isp_cpu_rst
    set_instance_parameter_value      isp_cpu_rst    ACTIVE_LOW_RESET          {0}
    set_instance_parameter_value      isp_cpu_rst    NUM_RESET_OUTPUTS         {1}
    set_instance_parameter_value      isp_cpu_rst    SYNCHRONOUS_EDGES         {deassert}
    set_instance_parameter_value      isp_cpu_rst    SYNC_RESET                {0}
    set_instance_parameter_value      isp_cpu_rst    USE_RESET_REQUEST         {0}

    # isp_mm_bridge
    set_instance_parameter_value      isp_mm_bridge       ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value      isp_mm_bridge       ADDRESS_WIDTH                 {0}
    set_instance_parameter_value      isp_mm_bridge       DATA_WIDTH                    {32}
    set_instance_parameter_value      isp_mm_bridge       LINEWRAPBURSTS                {0}
    set_instance_parameter_value      isp_mm_bridge       M0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_mm_bridge       MAX_BURST_SIZE                {1}
    set_instance_parameter_value      isp_mm_bridge       MAX_PENDING_RESPONSES         {4}
    set_instance_parameter_value      isp_mm_bridge       MAX_PENDING_WRITES            {0}
    set_instance_parameter_value      isp_mm_bridge       PIPELINE_COMMAND              {1}
    set_instance_parameter_value      isp_mm_bridge       PIPELINE_RESPONSE             {1}
    set_instance_parameter_value      isp_mm_bridge       S0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      isp_mm_bridge       SYMBOL_WIDTH                  {8}
    set_instance_parameter_value      isp_mm_bridge       SYNC_RESET                    {0}
    set_instance_parameter_value      isp_mm_bridge       USE_AUTO_ADDRESS_WIDTH        {1}
    set_instance_parameter_value      isp_mm_bridge       USE_RESPONSE                  {0}
    set_instance_parameter_value      isp_mm_bridge       USE_WRITERESPONSE             {0}

    # isp_vid_clk
    set_instance_parameter_value      isp_vid_clk      EXPLICIT_CLOCK_RATE     ${v_vid_clk_freq}
    set_instance_parameter_value      isp_vid_clk      NUM_CLOCK_OUTPUTS       {1}

    # isp_vid_rst
    set_instance_parameter_value      isp_vid_rst      ACTIVE_LOW_RESET        {0}
    set_instance_parameter_value      isp_vid_rst      NUM_RESET_OUTPUTS       {1}
    set_instance_parameter_value      isp_vid_rst      SYNCHRONOUS_EDGES       {deassert}
    set_instance_parameter_value      isp_vid_rst      SYNC_RESET              {0}
    set_instance_parameter_value      isp_vid_rst      USE_RESET_REQUEST       {0}

    # isp_niosv_clk
    set_instance_parameter_value      isp_niosv_clk      EXPLICIT_CLOCK_RATE     ${v_tmo_nios_clk_freq}
    set_instance_parameter_value      isp_niosv_clk      NUM_CLOCK_OUTPUTS       {1}

    # isp_niosv_rst
    set_instance_parameter_value      isp_niosv_rst      ACTIVE_LOW_RESET        {0}
    set_instance_parameter_value      isp_niosv_rst      NUM_RESET_OUTPUTS       {1}
    set_instance_parameter_value      isp_niosv_rst      SYNCHRONOUS_EDGES       {deassert}
    set_instance_parameter_value      isp_niosv_rst      SYNC_RESET              {0}
    set_instance_parameter_value      isp_niosv_rst      USE_RESET_REQUEST       {0}

    if {${v_warp_en}} {
        # isp_emif_clk
        set_instance_parameter_value      isp_emif_clk     EXPLICIT_CLOCK_RATE       ${v_emif_agent_clk_freq}
        set_instance_parameter_value      isp_emif_clk     NUM_CLOCK_OUTPUTS         {1}

        # isp_emif_rst
        set_instance_parameter_value      isp_emif_rst     ACTIVE_LOW_RESET          {1}
        set_instance_parameter_value      isp_emif_rst     NUM_RESET_OUTPUTS         {1}
        set_instance_parameter_value      isp_emif_rst     SYNCHRONOUS_EDGES         {deassert}
        set_instance_parameter_value      isp_emif_rst     SYNC_RESET                {0}
        set_instance_parameter_value      isp_emif_rst     USE_RESET_REQUEST         {0}
    }

    if {${v_raw_snapshot_en}} {
        # isp_emif_2_clk
        set_instance_parameter_value      isp_emif_2_clk     EXPLICIT_CLOCK_RATE       ${v_emif_agent_2_clk_freq}
        set_instance_parameter_value      isp_emif_2_clk     NUM_CLOCK_OUTPUTS         {1}

        # isp_emif_2_rst
        set_instance_parameter_value      isp_emif_2_rst     ACTIVE_LOW_RESET          {1}
        set_instance_parameter_value      isp_emif_2_rst     NUM_RESET_OUTPUTS         {1}
        set_instance_parameter_value      isp_emif_2_rst     SYNCHRONOUS_EDGES         {deassert}
        set_instance_parameter_value      isp_emif_2_rst     SYNC_RESET                {0}
        set_instance_parameter_value      isp_emif_2_rst     USE_RESET_REQUEST         {0}

        # isp_axi4s_bcast
        set_instance_parameter_value      isp_axi4s_bcast      BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_axi4s_bcast      GLOBAL_STALL                  {1}
        set_instance_parameter_value      isp_axi4s_bcast      IN_TREADY                     {1}
        set_instance_parameter_value      isp_axi4s_bcast      NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
        set_instance_parameter_value      isp_axi4s_bcast      OUTPUTS                       {2}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_0_FIFO                    {1}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_0_FIFO_DEPTH              {1024}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_0_TREADY                  {1}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_1_FIFO                    {1}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_1_FIFO_DEPTH              {1024}
        set_instance_parameter_value      isp_axi4s_bcast      OUT_1_TREADY                  {1}
        set_instance_parameter_value      isp_axi4s_bcast      PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_axi4s_bcast      VVP_INTF_TYPE                 {Lite}

        # isp_cpm
        set_instance_parameter_value      isp_cpm       BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_TYPE               {571}
        set_instance_parameter_value      isp_cpm       C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_cpm       ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_cpm       EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_cpm       IP0_KEEP_COLOR_PLANE_0        {1}
        set_instance_parameter_value      isp_cpm       IP0_KEEP_COLOR_PLANE_1        {1}
        set_instance_parameter_value      isp_cpm       IP0_KEEP_COLOR_PLANE_2        {1}
        set_instance_parameter_value      isp_cpm       IP0_KEEP_COLOR_PLANE_3        {1}
        set_instance_parameter_value      isp_cpm       IP1_KEEP_COLOR_PLANE_0        {1}
        set_instance_parameter_value      isp_cpm       IP1_KEEP_COLOR_PLANE_1        {1}
        set_instance_parameter_value      isp_cpm       IP1_KEEP_COLOR_PLANE_2        {1}
        set_instance_parameter_value      isp_cpm       IP1_KEEP_COLOR_PLANE_3        {1}
        set_instance_parameter_value      isp_cpm       KEEP_IP1_AUX_PACKETS          {1}
        set_instance_parameter_value      isp_cpm       KEEP_OP1_AUX_PACKETS          {1}
        set_instance_parameter_value      isp_cpm       MAPPING_COLOR_PLANE_0         {0}
        set_instance_parameter_value      isp_cpm       MAPPING_COLOR_PLANE_1         {0}
        set_instance_parameter_value      isp_cpm       MAPPING_COLOR_PLANE_2         {0}
        set_instance_parameter_value      isp_cpm       MAPPING_COLOR_PLANE_3         {0}
        set_instance_parameter_value      isp_cpm       MODE                          {REARRANGE}
        set_instance_parameter_value      isp_cpm       NUMBER_OF_COLOR_PLANES_IN0    ${v_isp_cppp}
        set_instance_parameter_value      isp_cpm       NUMBER_OF_COLOR_PLANES_IN1    {2}
        set_instance_parameter_value      isp_cpm       NUMBER_OF_COLOR_PLANES_OUT0   ${v_cppp}
        set_instance_parameter_value      isp_cpm       NUMBER_OF_COLOR_PLANES_OUT1   {2}
        set_instance_parameter_value      isp_cpm       OP0_KEEP_COLOR_PLANE_0        {1}
        set_instance_parameter_value      isp_cpm       OP0_KEEP_COLOR_PLANE_1        {1}
        set_instance_parameter_value      isp_cpm       OP0_KEEP_COLOR_PLANE_2        {1}
        set_instance_parameter_value      isp_cpm       OP0_KEEP_COLOR_PLANE_3        {1}
        set_instance_parameter_value      isp_cpm       OP1_KEEP_COLOR_PLANE_0        {1}
        set_instance_parameter_value      isp_cpm       OP1_KEEP_COLOR_PLANE_1        {1}
        set_instance_parameter_value      isp_cpm       OP1_KEEP_COLOR_PLANE_2        {1}
        set_instance_parameter_value      isp_cpm       OP1_KEEP_COLOR_PLANE_3        {1}
        set_instance_parameter_value      isp_cpm       PADDING_COLOR_PLANE_0         {0}
        set_instance_parameter_value      isp_cpm       PADDING_COLOR_PLANE_1         {0}
        set_instance_parameter_value      isp_cpm       PADDING_COLOR_PLANE_2         {0}
        set_instance_parameter_value      isp_cpm       PADDING_COLOR_PLANE_3         {0}
        set_instance_parameter_value      isp_cpm       PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_cpm       P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      isp_cpm       P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      isp_cpm       RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_cpm       SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_cpm       SLAVE_PROTOCOL                {Avalon}

        # isp_vfw
        set_instance_parameter_value      isp_vfw         BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_vfw         CLOCKS_ARE_SEPARATE           {1}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_TYPE               {585}
        set_instance_parameter_value      isp_vfw         C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_vfw         ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_vfw         EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_vfw         MAX_HEIGHT                    {4096}
        set_instance_parameter_value      isp_vfw         MAX_WIDTH                     {4096}
        set_instance_parameter_value      isp_vfw         NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      isp_vfw         PACKING                       {PERFECT}
        set_instance_parameter_value      isp_vfw         PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_vfw         P_AV_MM_ADDR_WIDTH            {32}
        set_instance_parameter_value      isp_vfw         P_AV_MM_DATA_WIDTH            {256}
        set_instance_parameter_value      isp_vfw         SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_vfw         SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_vfw         WRITE_BURST_TARGET            {64}
        set_instance_parameter_value      isp_vfw         WRITE_FIFO_DEPTH              {1024}

        # isp_se_vfw
        set_instance_parameter_value      isp_se_vfw      BURSTCOUNT_WIDTH              {7}
        set_instance_parameter_value      isp_se_vfw      DATA_WIDTH                    {256}
        set_instance_parameter_value      isp_se_vfw      ENABLE_SLAVE_PORT             {0}
        set_instance_parameter_value      isp_se_vfw      MASTER_ADDRESS_DEF            {0}
        set_instance_parameter_value      isp_se_vfw      MASTER_ADDRESS_WIDTH          {33}
        set_instance_parameter_value      isp_se_vfw      MAX_PENDING_READS             {8}
        set_instance_parameter_value      isp_se_vfw      SLAVE_ADDRESS_WIDTH           {26}
        set_instance_parameter_value      isp_se_vfw      SUB_WINDOW_COUNT              {1}
        set_instance_parameter_value      isp_se_vfw      SYNC_RESET                    {0}
    }

    if {${v_bls_en}} {
        # isp_bls
        set_instance_parameter_value      isp_bls       AV_MAX_PENDING_READS          {8}
        set_instance_parameter_value      isp_bls       BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_TYPE               {372}
        set_instance_parameter_value      isp_bls       C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_bls       DUPLICATE_AND_BYPASS          {1}
        set_instance_parameter_value      isp_bls       ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_bls       ENABLE_EXT_DATA_RW            {0}
        set_instance_parameter_value      isp_bls       EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_bls       H_TAPS                        {1}
        set_instance_parameter_value      isp_bls       MAX_HEIGHT                    {4096}
        set_instance_parameter_value      isp_bls       MAX_WIDTH                     {4096}
        set_instance_parameter_value      isp_bls       NO_BLANKING                   {1}
        set_instance_parameter_value      isp_bls       NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
        set_instance_parameter_value      isp_bls       NUM_EXT_DATA_REGS             {0}
        set_instance_parameter_value      isp_bls       PIPELINE_DATA_MM              {0}
        set_instance_parameter_value      isp_bls       PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value      isp_bls       PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_bls       P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      isp_bls       P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      isp_bls       RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_bls       SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_bls       SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_bls       V_TAPS                        {1}
    }

    if {${v_clipper_en}} {
        # isp_clipper
        set_instance_parameter_value      isp_clipper       BOTTOM_OFFSET                 {0}
        set_instance_parameter_value      isp_clipper       BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_clipper       CLIPPING_METHOD               {OFFSETS}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_TYPE               {557}
        set_instance_parameter_value      isp_clipper       C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_clipper       ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_clipper       EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_clipper       LEFT_OFFSET                   {0}
        set_instance_parameter_value      isp_clipper       NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
        set_instance_parameter_value      isp_clipper       PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_clipper       RECTANGLE_HEIGHT              {2160}
        set_instance_parameter_value      isp_clipper       RECTANGLE_WIDTH               {3840}
        set_instance_parameter_value      isp_clipper       RIGHT_OFFSET                  {0}
        set_instance_parameter_value      isp_clipper       RUNTIME_CONTROL               {0}
        set_instance_parameter_value      isp_clipper       SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_clipper       SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_clipper       TOP_OFFSET                    {0}
    }

    # isp_dpc
    set_instance_parameter_value      isp_dpc       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_dpc       BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_dpc       BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_TYPE               {373}
    set_instance_parameter_value      isp_dpc       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_dpc       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_dpc       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_dpc       ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_dpc       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_dpc       H_TAPS                        {5}
    set_instance_parameter_value      isp_dpc       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_dpc       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_dpc       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_dpc       NUMBER_OF_COLOR_PLANES_IN     ${v_isp_cppp}
    set_instance_parameter_value      isp_dpc       NUMBER_OF_COLOR_PLANES_OUT    ${v_isp_cppp}
    set_instance_parameter_value      isp_dpc       NUM_EXT_DATA_REGS             {0}
    set_instance_parameter_value      isp_dpc       PIPELINE_DATA_MM              {0}
    set_instance_parameter_value      isp_dpc       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_dpc       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_dpc       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_dpc       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_dpc       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_dpc       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_dpc       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_dpc       V_TAPS                        {5}

    # isp_anr
    set_instance_parameter_value      isp_anr       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_anr       BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_anr       BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_TYPE               {374}
    set_instance_parameter_value      isp_anr       C_OMNI_CAP_VERSION            {2}
    set_instance_parameter_value      isp_anr       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_anr       CFA_ENABLE                    {1}
    set_instance_parameter_value      isp_anr       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_anr       ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_anr       EXTERNAL_MODE                 {1}
    if {${v_small_anr}} {
        set_instance_parameter_value      isp_anr       H_TAPS                        {9}
        set_instance_parameter_value      isp_anr       V_TAPS                        {9}
    } else {
        set_instance_parameter_value      isp_anr       H_TAPS                        {17}
        set_instance_parameter_value      isp_anr       V_TAPS                        {17}
    }
    set_instance_parameter_value      isp_anr       MAX_HEIGHT                    {65536}
    set_instance_parameter_value      isp_anr       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_anr       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_anr       NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
    set_instance_parameter_value      isp_anr       NUM_EXT_DATA_REGS             {2048}
    set_instance_parameter_value      isp_anr       PIPELINE_DATA_MM              {0}
    set_instance_parameter_value      isp_anr       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_anr       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_anr       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_anr       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_anr       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_anr       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_anr       SLAVE_PROTOCOL                {Avalon}

    # isp_blc
    set_instance_parameter_value      isp_blc       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_blc       BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_blc       BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_TYPE               {375}
    set_instance_parameter_value      isp_blc       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_blc       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_blc       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_blc       ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_blc       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_blc       H_TAPS                        {1}
    set_instance_parameter_value      isp_blc       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_blc       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_blc       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_blc       NUMBER_OF_COLOR_PLANES_IN     ${v_isp_cppp}
    set_instance_parameter_value      isp_blc       NUMBER_OF_COLOR_PLANES_OUT    ${v_isp_cppp}
    set_instance_parameter_value      isp_blc       NUM_EXT_DATA_REGS             {0}
    set_instance_parameter_value      isp_blc       PIPELINE_DATA_MM              {0}
    set_instance_parameter_value      isp_blc       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_blc       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_blc       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_blc       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_blc       REFLECT_AROUND_ZERO           {1}
    set_instance_parameter_value      isp_blc       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_blc       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_blc       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_blc       V_TAPS                        {1}

    # isp_vc
    set_instance_parameter_value      isp_vc        AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_vc        BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_vc        BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_vc        CFA_ENABLE                    {1}
    set_instance_parameter_value      isp_vc        CFA_NUM_OF_COLOR_PLANES       ${v_isp_cppp}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_TYPE               {376}
    set_instance_parameter_value      isp_vc        C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_vc        DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_vc        ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_vc        ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_vc        EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_vc        H_TAPS                        {1}
    if {${v_small_vc}} {
        set_instance_parameter_value      isp_vc      MAX_GAIN_MESH_POINTS          {1024}
        set_instance_parameter_value      isp_vc      PER_COLOR_GAIN_ENABLE         {0}
    } else {
        set_instance_parameter_value      isp_vc      MAX_GAIN_MESH_POINTS          {4096}
        set_instance_parameter_value      isp_vc      PER_COLOR_GAIN_ENABLE         {1}
    }
    set_instance_parameter_value      isp_vc        MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_vc        MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_vc        NO_BLANKING                   {1}
    set_instance_parameter_value      isp_vc        NUM_EXT_DATA_REGS             {32768}
    set_instance_parameter_value      isp_vc        PIPELINE_DATA_MM              {1}
    set_instance_parameter_value      isp_vc        PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_vc        PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_vc        P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_vc        P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_vc        RGB_NUM_OF_COLOR_PLANES       ${v_isp_cppp}
    set_instance_parameter_value      isp_vc        RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_vc        SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_vc        SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_vc        V_TAPS                        {1}

    # isp_switch_wbs
    set_instance_parameter_value      isp_switch_wbs      BPS                           ${v_isp_bps}
    set_instance_parameter_value      isp_switch_wbs      CRASH_SWITCH                  ${v_wbs_async_ip_sw}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_TYPE               {565}
    set_instance_parameter_value      isp_switch_wbs      C_OMNI_CAP_VERSION            {2}
    set_instance_parameter_value      isp_switch_wbs      ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_switch_wbs      EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_switch_wbs      NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
    set_instance_parameter_value      isp_switch_wbs      NUM_INPUTS                    {3}
    set_instance_parameter_value      isp_switch_wbs      NUM_OUTPUTS                   {3}
    set_instance_parameter_value      isp_switch_wbs      PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_switch_wbs      SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_switch_wbs      SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_switch_wbs      UNINTERRUPTED_INPUTS          ${v_wbs_sync_ip_sw}
    set_instance_parameter_value      isp_switch_wbs      USE_OP_RESP                   {0}
    set_instance_parameter_value      isp_switch_wbs      USE_TREADIES                  {1}
    set_instance_parameter_value      isp_switch_wbs      VVP_INTF_TYPE                 {VVP_LITE}

    # isp_wbc
    set_instance_parameter_value      isp_wbc       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_wbc       BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_wbc       BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_TYPE               {378}
    set_instance_parameter_value      isp_wbc       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_wbc       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_wbc       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_wbc       ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_wbc       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_wbc       H_TAPS                        {1}
    set_instance_parameter_value      isp_wbc       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_wbc       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_wbc       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_wbc       NUMBER_OF_COLOR_PLANES_IN     ${v_isp_cppp}
    set_instance_parameter_value      isp_wbc       NUMBER_OF_COLOR_PLANES_OUT    ${v_isp_cppp}
    set_instance_parameter_value      isp_wbc       NUM_EXT_DATA_REGS             {0}
    set_instance_parameter_value      isp_wbc       PIPELINE_DATA_MM              {0}
    set_instance_parameter_value      isp_wbc       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_wbc       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_wbc       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_wbc       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_wbc       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_wbc       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_wbc       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_wbc       V_TAPS                        {1}

    # isp_wbs
    set_instance_parameter_value      isp_wbs       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_wbs       BPS                           ${v_isp_bps}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_TYPE               {377}
    set_instance_parameter_value      isp_wbs       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_wbs       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_wbs       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_wbs       ENABLE_EXT_DATA_RW            {1}
    set_instance_parameter_value      isp_wbs       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_wbs       H_TAPS                        {1}
    set_instance_parameter_value      isp_wbs       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_wbs       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_wbs       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_wbs       NUMBER_OF_COLOR_PLANES        ${v_isp_cppp}
    set_instance_parameter_value      isp_wbs       NUM_EXT_DATA_REGS             {512}
    set_instance_parameter_value      isp_wbs       PIPELINE_DATA_MM              {1}
    set_instance_parameter_value      isp_wbs       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_wbs       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_wbs       PRECISION_BITS                {8}
    set_instance_parameter_value      isp_wbs       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_wbs       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_wbs       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_wbs       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_wbs       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_wbs       V_TAPS                        {2}

    # isp_dms
    set_instance_parameter_value      isp_dms       AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_dms       BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_dms       BPS_OUT                       ${v_isp_bps}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_TYPE               {582}
    set_instance_parameter_value      isp_dms       C_OMNI_CAP_VERSION            {2}
    set_instance_parameter_value      isp_dms       DUPLICATE_AND_BYPASS          {0}
    set_instance_parameter_value      isp_dms       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_dms       ENABLE_EXT_DATA_RW            {0}
    set_instance_parameter_value      isp_dms       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_dms       H_TAPS                        {5}
    set_instance_parameter_value      isp_dms       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_dms       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_dms       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_dms       NUMBER_OF_COLOR_PLANES_IN     ${v_isp_cppp}
    set_instance_parameter_value      isp_dms       NUMBER_OF_COLOR_PLANES_OUT    ${v_cppp}
    set_instance_parameter_value      isp_dms       NUM_EXT_DATA_REGS             {0}
    set_instance_parameter_value      isp_dms       PIPELINE_DATA_MM              {0}
    set_instance_parameter_value      isp_dms       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_dms       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_dms       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_dms       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_dms       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_dms       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_dms       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_dms       V_TAPS                        {5}

    # isp_hs
    set_instance_parameter_value      isp_hs        AV_MAX_PENDING_READS          {8}
    set_instance_parameter_value      isp_hs        BPS                           ${v_isp_bps}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_TYPE               {379}
    set_instance_parameter_value      isp_hs        C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_hs        DUPLICATE_AND_BYPASS          {1}
    set_instance_parameter_value      isp_hs        ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_hs        ENABLE_EXT_DATA_RW            {1}
    set_instance_parameter_value      isp_hs        EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_hs        H_TAPS                        {1}
    set_instance_parameter_value      isp_hs        MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_hs        MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_hs        NO_BLANKING                   {1}
    set_instance_parameter_value      isp_hs        NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_hs        NUM_HIST_BINS                 {256}
    set_instance_parameter_value      isp_hs        PIPELINE_DATA_MM              {1}
    set_instance_parameter_value      isp_hs        PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_hs        PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_hs        P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_hs        P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_hs        RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_hs        SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_hs        SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_hs        V_TAPS                        {1}

    # isp_ccm
    set_instance_parameter_value      isp_ccm       BPS_IN                        ${v_isp_bps}
    if {${v_hdr_en}} {
        set_instance_parameter_value      isp_ccm       BPS_OUT                       ${v_isp_bps}
    } elseif {${v_3d_lut_en}} {
        set_instance_parameter_value      isp_ccm       BPS_OUT                       ${v_isp_bps_cut}
    } else {
        set_instance_parameter_value      isp_ccm       BPS_OUT                       ${v_tmo_bps}
    }
    set_instance_parameter_value      isp_ccm       COEFFICIENT_INT_BITS          {8}
    set_instance_parameter_value      isp_ccm       COEFFICIENT_SIGNED            {1}
    set_instance_parameter_value      isp_ccm       COEF_SUM_FRACTION_BITS        {8}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_TYPE               {559}
    set_instance_parameter_value      isp_ccm       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_ccm       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_ccm       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_ccm       MOVE_BINARY_POINT_RIGHT       {0}
    set_instance_parameter_value      isp_ccm       OUTPUT_COLORSPACE             {0}
    set_instance_parameter_value      isp_ccm       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_ccm       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_ccm       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_ccm       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_ccm       REMOVE_FRACTION_METHOD        {1}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_A0                 {1.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_A1                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_A2                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_B0                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_B1                 {1.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_B2                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_C0                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_C1                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_C2                 {1.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_S0                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_S1                 {0.0}
    set_instance_parameter_value      isp_ccm       REQ_FCOEFF_S2                 {0.0}
    set_instance_parameter_value      isp_ccm       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_ccm       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_ccm       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_ccm       SUMMAND_INT_BITS              {10}
    set_instance_parameter_value      isp_ccm       SUMMAND_SIGNED                {1}

    if {${v_hdr_en}} {
        # isp_1d_lut_hdr
        set_instance_parameter_value      isp_1d_lut_hdr        AV_MAX_PENDING_READS          {8}
        set_instance_parameter_value      isp_1d_lut_hdr        BITS_LUT                      {12}
        set_instance_parameter_value      isp_1d_lut_hdr        BITS_STEP                     {2}
        set_instance_parameter_value      isp_1d_lut_hdr        BPS_IN                        ${v_isp_bps}
        set_instance_parameter_value      isp_1d_lut_hdr        BPS_OUT                       ${v_isp_bps_cut}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_TYPE               {381}
        set_instance_parameter_value      isp_1d_lut_hdr        C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_1d_lut_hdr        DUPLICATE_AND_BYPASS          {0}
        set_instance_parameter_value      isp_1d_lut_hdr        ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_1d_lut_hdr        ENABLE_EXT_DATA_RW            {0}
        set_instance_parameter_value      isp_1d_lut_hdr        EQUIDISTANT                   {0}
        set_instance_parameter_value      isp_1d_lut_hdr        EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_1d_lut_hdr        H_TAPS                        {1}
        set_instance_parameter_value      isp_1d_lut_hdr        MAX_HEIGHT                    {4096}
        set_instance_parameter_value      isp_1d_lut_hdr        MAX_WIDTH                     {4096}
        set_instance_parameter_value      isp_1d_lut_hdr        NO_BLANKING                   {1}
        set_instance_parameter_value      isp_1d_lut_hdr        NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      isp_1d_lut_hdr        PIPELINE_DATA_MM              {0}
        set_instance_parameter_value      isp_1d_lut_hdr        PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value      isp_1d_lut_hdr        PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_1d_lut_hdr        P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      isp_1d_lut_hdr        P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      isp_1d_lut_hdr        REVERSE_LUT                   {0}
        set_instance_parameter_value      isp_1d_lut_hdr        RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_1d_lut_hdr        SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_1d_lut_hdr        SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_1d_lut_hdr        V_TAPS                        {1}

        # isp_3d_lut_hdr_1
        set_instance_parameter_value      isp_3d_lut_hdr_1        BPS_IN                        ${v_isp_bps_cut}
        set_instance_parameter_value      isp_3d_lut_hdr_1        BPS_OUT                       ${v_isp_bps_cut}
        set_instance_parameter_value      isp_3d_lut_hdr_1        BYPASS_ALPHA                  {1023}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_TYPE               {357}
        set_instance_parameter_value      isp_3d_lut_hdr_1        C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_3d_lut_hdr_1        EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_3d_lut_hdr_1        LUT_ALPHA                     {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        LUT_CPU_READABLE              {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        LUT_DEPTH                     ${v_isp_bps_cut}
        if {(${v_pip} > 1) && (${v_lut_pip_sharing_en} )} {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_PIP_SHARING               {1}
        } else {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_PIP_SHARING               {0}
        }
        if {${v_small_3d_lut}} {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_DIMENSION                 {17}
        } else {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_DIMENSION                 {33}
        }
        if {${v_lut_double_buffered_en}} {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_DOUBLE_BUFFERED           {1}
        } else {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_DOUBLE_BUFFERED           {0}
        }
        if {${v_3d_lut_preset_file_en}} {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_0                    {1}
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_FILENAME_0 \
                                                      ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_0_preset_file}
            if {${v_lut_double_buffered_en}} {
                set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_1                    {1}
                set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_FILENAME_1 \
                                                      ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_1_preset_file}
            }
        } else {
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_0                    {0}
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_FILENAME_0           {}
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_1                    {0}
            set_instance_parameter_value      isp_3d_lut_hdr_1      LUT_INIT_FILENAME_1           {}
        }
        set_instance_parameter_value      isp_3d_lut_hdr_1        LUT_INIT_TYPE_0               {normalized}
        set_instance_parameter_value      isp_3d_lut_hdr_1        LUT_INIT_TYPE_1               {normalized}
        set_instance_parameter_value      isp_3d_lut_hdr_1        PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_3d_lut_hdr_1        RESET_ENABLED                 {0}
        set_instance_parameter_value      isp_3d_lut_hdr_1        RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_3d_lut_hdr_1        SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_3d_lut_hdr_1        SLAVE_PROTOCOL                {Avalon}
    }

    if {${v_3d_lut_en}} {
        # isp_3d_lut_hdr_2
        set_instance_parameter_value      isp_3d_lut_hdr_2        BPS_IN                        ${v_isp_bps_cut}
        set_instance_parameter_value      isp_3d_lut_hdr_2        BPS_OUT                       ${v_tmo_bps}
        set_instance_parameter_value      isp_3d_lut_hdr_2        BYPASS_ALPHA                  {1023}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_TYPE               {357}
        set_instance_parameter_value      isp_3d_lut_hdr_2        C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_3d_lut_hdr_2        EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_3d_lut_hdr_2        LUT_ALPHA                     {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        LUT_CPU_READABLE              {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        LUT_DEPTH                     ${v_isp_bps_cut}
            if {(${v_pip} > 1) && (${v_lut_pip_sharing_en} )} {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_PIP_SHARING               {1}
            } else {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_PIP_SHARING               {0}
            }
            if {${v_small_3d_lut}} {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_DIMENSION                 {17}
            } else {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_DIMENSION                 {33}
            }
            if {${v_lut_double_buffered_en}} {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_DOUBLE_BUFFERED           {1}
            } else {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_DOUBLE_BUFFERED           {0}
            }
        if {${v_3d_lut_preset_file_en}} {
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_0                    {1}
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_FILENAME_0 \
                                                        ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_0_preset_file}
            if {${v_lut_double_buffered_en}} {
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_1                    {1}
                set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_FILENAME_1 \
                                                        ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_1_preset_file}
            }
        } else {
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_0                    {0}
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_FILENAME_0           {}
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_1                    {0}
            set_instance_parameter_value      isp_3d_lut_hdr_2      LUT_INIT_FILENAME_1           {}
        }
        set_instance_parameter_value      isp_3d_lut_hdr_2        LUT_INIT_TYPE_0               {normalized}
        set_instance_parameter_value      isp_3d_lut_hdr_2        LUT_INIT_TYPE_1               {normalized}
        set_instance_parameter_value      isp_3d_lut_hdr_2        PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_3d_lut_hdr_2        RESET_ENABLED                 {0}
        set_instance_parameter_value      isp_3d_lut_hdr_2        RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_3d_lut_hdr_2        SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_3d_lut_hdr_2        SLAVE_PROTOCOL                {Avalon}
    }

    # isp_tmo
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_TYPE               {355}
    set_instance_parameter_value      isp_tmo         C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_tmo         P_BPS                         ${v_tmo_bps}
    set_instance_parameter_value      isp_tmo         P_NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value      isp_tmo         P_PIXELS_IN_PARALLEL          ${v_pip}

    # isp_pix_adapt_tmo
    set_instance_parameter_value      isp_pix_adapt_tmo     BPS_IN                        ${v_tmo_bps}
    set_instance_parameter_value      isp_pix_adapt_tmo     BPS_OUT                       ${v_usm_bps}
    set_instance_parameter_value      isp_pix_adapt_tmo     ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_pix_adapt_tmo     EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_pix_adapt_tmo     NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_pix_adapt_tmo     PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_pix_adapt_tmo     PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_pix_adapt_tmo     P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_pix_adapt_tmo     P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_pix_adapt_tmo     RUNTIME_CONTROL               {0}
    set_instance_parameter_value      isp_pix_adapt_tmo     SEPARATE_SLAVE_CLOCK          {0}
    set_instance_parameter_value      isp_pix_adapt_tmo     SLAVE_PROTOCOL                {Avalon}

    # isp_usm
    set_instance_parameter_value      isp_usm       BPS                           ${v_usm_bps}
    set_instance_parameter_value      isp_usm       COEFFS_FRACTION_BITS          {8}
    set_instance_parameter_value      isp_usm       COEFFS_INTEGER_BITS           {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_TYPE               {380}
    set_instance_parameter_value      isp_usm       C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_usm       ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_usm       EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_usm       H_TAPS                        {5}
    set_instance_parameter_value      isp_usm       MAX_HEIGHT                    {4096}
    set_instance_parameter_value      isp_usm       MAX_WIDTH                     {4096}
    set_instance_parameter_value      isp_usm       NO_BLANKING                   {1}
    set_instance_parameter_value      isp_usm       PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_usm       PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_usm       P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_usm       P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_usm       RUNTIME_CONTROL               {1}
    set_instance_parameter_value      isp_usm       SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value      isp_usm       SIGNED_COEFFS                 {0}
    set_instance_parameter_value      isp_usm       SLAVE_PROTOCOL                {Avalon}
    set_instance_parameter_value      isp_usm       V_TAPS                        {5}

    # isp_roi
    set_instance_parameter_value      isp_roi        BPS                           ${v_usm_bps}
    set_instance_parameter_value      isp_roi        C_CPU_OFFSET                  {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_SIZE               {64}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_TYPE               {600}
    set_instance_parameter_value      isp_roi        C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value      isp_roi        NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_roi        PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_roi        RUNTIME_CONTROL               {1}

    if {${v_warp_en}} {
        # isp_warp
        set_instance_parameter_value      isp_warp        BPS                           ${v_usm_bps}
        set_instance_parameter_value      isp_warp        CACHE_BLOCKS                  ${v_warp_cache_blocks}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_TYPE               {367}
        set_instance_parameter_value      isp_warp        C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_warp        DEBUG_ENABLE                  ${v_warp_debug}
        set_instance_parameter_value      isp_warp        EASY_WARP                     {0}
        set_instance_parameter_value      isp_warp        EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_warp        MAX_INPUT_WIDTH               {3840}
        set_instance_parameter_value      isp_warp        MAX_OUTPUT_WIDTH              {3840}
        set_instance_parameter_value      isp_warp        MEMORY_MAP                    {2}
        set_instance_parameter_value      isp_warp        MIPMAP_ENABLE                 ${v_warp_mipmap_enable}
        set_instance_parameter_value      isp_warp        NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      isp_warp        NUM_ENGINES                   ${v_num_warp_engines}
        set_instance_parameter_value      isp_warp        PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_warp        SINGLE_BOUNCE                 ${v_warp_single_bounce}
        set_instance_parameter_value      isp_warp        EXT_MEM_DATA_WIDTH            256

        # isp_se_warp
        set_instance_parameter_value      isp_se_warp         BURSTCOUNT_WIDTH              {7}
        set_instance_parameter_value      isp_se_warp         DATA_WIDTH                    {256}
        set_instance_parameter_value      isp_se_warp         ENABLE_SLAVE_PORT             {0}
        set_instance_parameter_value      isp_se_warp         MASTER_ADDRESS_DEF            {0}
        set_instance_parameter_value      isp_se_warp         MASTER_ADDRESS_WIDTH          {33}
        set_instance_parameter_value      isp_se_warp         MAX_PENDING_READS             {64}
        set_instance_parameter_value      isp_se_warp         SLAVE_ADDRESS_WIDTH           {26}
        set_instance_parameter_value      isp_se_warp         SUB_WINDOW_COUNT              {1}
        set_instance_parameter_value      isp_se_warp         SYNC_RESET                    {0}

        if {${v_ai_en}} {
            # isp_warp_throttle
            set_instance_parameter_value    isp_warp_throttle     BPS                         ${v_usm_bps}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_ID_ASSOCIATED    {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_IRQ              {255}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_IRQ_ENABLE       {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_IRQ_ENABLE_EN    {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_IRQ_STATUS       {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_IRQ_STATUS_EN    {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_SIZE             {128}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_TAG              {0}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_TYPE             {598}
            set_instance_parameter_value    isp_warp_throttle     C_OMNI_CAP_VERSION          {1}
            set_instance_parameter_value    isp_warp_throttle     ENABLE_DEBUG                ${v_enable_debug}
            set_instance_parameter_value    isp_warp_throttle     EXTERNAL_MODE               {1}
            set_instance_parameter_value    isp_warp_throttle     NUMBER_OF_COLOR_PLANES      ${v_cppp}
            set_instance_parameter_value    isp_warp_throttle     PIPELINE_READY              ${v_pipeline_ready}
            set_instance_parameter_value    isp_warp_throttle     PIXELS_IN_PARALLEL          ${v_pip}
            set_instance_parameter_value    isp_warp_throttle     SEPARATE_SLAVE_CLOCK        {1}
            set_instance_parameter_value    isp_warp_throttle     SLAVE_PROTOCOL              {Avalon}
        }
    }


    ############################
    #### Create Connections ####
    ############################

    # isp_cpu_clk
    add_connection        isp_cpu_clk.out_clk       isp_cpu_rst.clk
    add_connection        isp_cpu_clk.out_clk       isp_mm_bridge.clk
    if {${v_raw_snapshot_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_cpm.agent_clock
        add_connection        isp_cpu_clk.out_clk       isp_vfw.control_clock
    }
    if {${v_bls_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_bls.agent_clock
    }
    if {${v_clipper_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_clipper.agent_clock
    }
    add_connection        isp_cpu_clk.out_clk       isp_dpc.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_anr.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_blc.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_vc.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_switch_wbs.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_wbc.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_wbs.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_dms.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_hs.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_ccm.agent_clock
    if {${v_hdr_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_1d_lut_hdr.agent_clock
        add_connection        isp_cpu_clk.out_clk       isp_3d_lut_hdr_1.cpu_clock
    }
    if {${v_3d_lut_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_3d_lut_hdr_2.cpu_clock
    }
    add_connection        isp_cpu_clk.out_clk       isp_tmo.external_cpu_clock
    add_connection        isp_cpu_clk.out_clk       isp_usm.agent_clock
    add_connection        isp_cpu_clk.out_clk       isp_roi.agent_clock
    if {${v_warp_en}} {
        add_connection        isp_cpu_clk.out_clk       isp_warp.av_mm_control_agent_clock
        if {${v_ai_en}} {
            add_connection        isp_cpu_clk.out_clk       isp_warp_throttle.agent_clock
        }
    }

    # isp_cpu_rst
    add_connection        isp_cpu_rst.out_reset         isp_mm_bridge.reset
    if {${v_raw_snapshot_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_cpm.agent_reset
        add_connection        isp_cpu_rst.out_reset         isp_vfw.control_reset
    }
    if {${v_bls_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_bls.agent_reset
    }
    if {${v_clipper_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_clipper.agent_reset
    }
    add_connection        isp_cpu_rst.out_reset         isp_dpc.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_anr.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_blc.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_vc.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_switch_wbs.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_wbc.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_wbs.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_dms.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_hs.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_ccm.agent_reset
    if {${v_hdr_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_1d_lut_hdr.agent_reset
        add_connection        isp_cpu_rst.out_reset         isp_3d_lut_hdr_1.cpu_reset
    }
    if {${v_3d_lut_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_3d_lut_hdr_2.cpu_reset
    }
    add_connection        isp_cpu_rst.out_reset         isp_tmo.external_cpu_reset
    add_connection        isp_cpu_rst.out_reset         isp_usm.agent_reset
    add_connection        isp_cpu_rst.out_reset         isp_roi.agent_reset
    if {${v_warp_en}} {
        add_connection        isp_cpu_rst.out_reset         isp_warp.av_mm_control_agent_reset
        if {${v_ai_en}} {
            add_connection        isp_cpu_rst.out_reset         isp_warp_throttle.agent_reset
        }
    }

    # isp_mm_bridge
    if {${v_raw_snapshot_en}} {
        add_connection          isp_mm_bridge.m0            isp_cpm.av_mm_control_agent
        add_connection          isp_mm_bridge.m0            isp_vfw.av_mm_control_agent
    }
    if {${v_bls_en}} {
        add_connection          isp_mm_bridge.m0            isp_bls.av_mm_control_agent
    }
    if {${v_clipper_en}} {
        add_connection          isp_mm_bridge.m0            isp_clipper.av_mm_control_agent
    }
    add_connection          isp_mm_bridge.m0            isp_dpc.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_anr.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_blc.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_vc.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_switch_wbs.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_wbc.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_wbs.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_dms.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_hs.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_ccm.av_mm_control_agent
    if {${v_hdr_en}} {
        add_connection          isp_mm_bridge.m0            isp_1d_lut_hdr.av_mm_control_agent
        add_connection          isp_mm_bridge.m0            isp_3d_lut_hdr_1.av_mm_cpu_agent
    }
    if {${v_3d_lut_en}} {
        add_connection          isp_mm_bridge.m0            isp_3d_lut_hdr_2.av_mm_cpu_agent
    }
    add_connection          isp_mm_bridge.m0            isp_tmo.av_mm_cpu_agent
    add_connection          isp_mm_bridge.m0            isp_usm.av_mm_control_agent
    add_connection          isp_mm_bridge.m0            isp_roi.av_mm_control_agent
    if {${v_warp_en}} {
        add_connection          isp_mm_bridge.m0            isp_warp.av_mm_control_agent
        if {${v_ai_en}} {
            add_connection          isp_mm_bridge.m0            isp_warp_throttle.av_mm_control_agent
        }
    }

    # isp_vid_clk
    add_connection        isp_vid_clk.out_clk           isp_vid_rst.clk
    if {${v_raw_snapshot_en}} {
        add_connection        isp_vid_clk.out_clk           isp_axi4s_bcast.vid_clock
        add_connection        isp_vid_clk.out_clk           isp_cpm.main_clock
        add_connection        isp_vid_clk.out_clk           isp_vfw.main_clock
    }
    if {${v_bls_en}} {
        add_connection        isp_vid_clk.out_clk           isp_bls.main_clock
    }
    if {${v_clipper_en}} {
        add_connection        isp_vid_clk.out_clk           isp_clipper.main_clock
    }
    add_connection        isp_vid_clk.out_clk           isp_dpc.main_clock
    add_connection        isp_vid_clk.out_clk           isp_anr.main_clock
    add_connection        isp_vid_clk.out_clk           isp_blc.main_clock
    add_connection        isp_vid_clk.out_clk           isp_vc.main_clock
    add_connection        isp_vid_clk.out_clk           isp_switch_wbs.main_clock
    add_connection        isp_vid_clk.out_clk           isp_wbc.main_clock
    add_connection        isp_vid_clk.out_clk           isp_wbs.main_clock
    add_connection        isp_vid_clk.out_clk           isp_dms.main_clock
    add_connection        isp_vid_clk.out_clk           isp_hs.main_clock
    add_connection        isp_vid_clk.out_clk           isp_ccm.main_clock
    if {${v_hdr_en}} {
        add_connection        isp_vid_clk.out_clk           isp_1d_lut_hdr.main_clock
        add_connection        isp_vid_clk.out_clk           isp_3d_lut_hdr_1.vid_clock
    }
    if {${v_3d_lut_en}} {
        add_connection        isp_vid_clk.out_clk           isp_3d_lut_hdr_2.vid_clock
    }
    add_connection        isp_vid_clk.out_clk           isp_tmo.vid_clock
    add_connection        isp_vid_clk.out_clk           isp_pix_adapt_tmo.main_clock
    add_connection        isp_vid_clk.out_clk           isp_usm.main_clock
    add_connection        isp_vid_clk.out_clk           isp_roi.main_clock
    if {${v_warp_en}} {
        add_connection        isp_vid_clk.out_clk           isp_warp.core_clock
        add_connection        isp_vid_clk.out_clk           isp_warp.axi4s_vid_in_0_clock
        add_connection        isp_vid_clk.out_clk           isp_warp.axi4s_vid_out_0_clock
        if {${v_ai_en}} {
            add_connection        isp_vid_clk.out_clk           isp_warp_throttle.main_clock
        }
    }

    # isp_vid_rst
    if {${v_raw_snapshot_en}} {
        add_connection        isp_vid_rst.out_reset         isp_axi4s_bcast.vid_reset
        add_connection        isp_vid_rst.out_reset         isp_cpm.main_reset
        add_connection        isp_vid_rst.out_reset         isp_vfw.main_reset
    }
    if {${v_bls_en}} {
        add_connection        isp_vid_rst.out_reset         isp_bls.main_reset
    }
    if {${v_clipper_en}} {
        add_connection        isp_vid_rst.out_reset         isp_clipper.main_reset
    }
    add_connection        isp_vid_rst.out_reset         isp_dpc.main_reset
    add_connection        isp_vid_rst.out_reset         isp_anr.main_reset
    add_connection        isp_vid_rst.out_reset         isp_blc.main_reset
    add_connection        isp_vid_rst.out_reset         isp_vc.main_reset
    add_connection        isp_vid_rst.out_reset         isp_switch_wbs.main_reset
    add_connection        isp_vid_rst.out_reset         isp_wbc.main_reset
    add_connection        isp_vid_rst.out_reset         isp_wbs.main_reset
    add_connection        isp_vid_rst.out_reset         isp_dms.main_reset
    add_connection        isp_vid_rst.out_reset         isp_hs.main_reset
    add_connection        isp_vid_rst.out_reset         isp_ccm.main_reset
    if {${v_hdr_en}} {
        add_connection        isp_vid_rst.out_reset         isp_1d_lut_hdr.main_reset
        add_connection        isp_vid_rst.out_reset         isp_3d_lut_hdr_1.vid_reset
    }
    if {${v_3d_lut_en}} {
        add_connection        isp_vid_rst.out_reset         isp_3d_lut_hdr_2.vid_reset
    }
    add_connection        isp_vid_rst.out_reset         isp_tmo.vid_reset
    add_connection        isp_vid_rst.out_reset         isp_pix_adapt_tmo.main_reset
    add_connection        isp_vid_rst.out_reset         isp_usm.main_reset
    add_connection        isp_vid_rst.out_reset         isp_roi.main_reset
    if {${v_warp_en}} {
        add_connection        isp_vid_rst.out_reset         isp_warp.core_reset
        add_connection        isp_vid_rst.out_reset         isp_warp.axi4s_vid_in_0_reset
        add_connection        isp_vid_rst.out_reset         isp_warp.axi4s_vid_out_0_reset
        if {${v_ai_en}} {
            add_connection        isp_vid_rst.out_reset          isp_warp_throttle.main_reset
        }
    }

    # isp_niosv_clk
    add_connection         isp_niosv_clk.out_clk         isp_niosv_rst.clk
    add_connection         isp_niosv_clk.out_clk         isp_tmo.internal_cpu_clock

    # isp_niosv_rst
    add_connection         isp_niosv_rst.out_reset       isp_tmo.internal_cpu_reset

    if {${v_warp_en}} {
        # isp_emif_clk
        add_connection         isp_emif_clk.out_clk          isp_emif_rst.clk
        add_connection         isp_emif_clk.out_clk          isp_warp.av_mm_memory_host_clock
        add_connection         isp_emif_clk.out_clk          isp_se_warp.clock

        # isp_emif_rst
        add_connection         isp_emif_rst.out_reset        isp_warp.av_mm_memory_host_reset
        add_connection         isp_emif_rst.out_reset        isp_se_warp.reset
    }

    if {${v_clipper_en}} {
        if {${v_bls_en}} {
            # isp_bls
            add_connection      isp_bls.axi4s_vid_out         isp_clipper.axi4s_vid_in
        }

        if {${v_raw_snapshot_en}} {
            # isp_clipper
            add_connection      isp_clipper.axi4s_vid_out     isp_axi4s_bcast.axi4s_vid_in
        } else {
            # isp_clipper
            add_connection      isp_clipper.axi4s_vid_out     isp_dpc.axi4s_vid_in
        }
    } elseif {${v_bls_en}} {
        if {${v_raw_snapshot_en}} {
            # isp_bls
            add_connection      isp_bls.axi4s_vid_out         isp_axi4s_bcast.axi4s_vid_in
        } else {
            # isp_bls
            add_connection      isp_bls.axi4s_vid_out         isp_dpc.axi4s_vid_in
        }
    }

    if {${v_raw_snapshot_en}} {
        # isp_emif_2_clk
        add_connection         isp_emif_2_clk.out_clk          isp_emif_2_rst.clk
        add_connection         isp_emif_2_clk.out_clk          isp_vfw.mem_clock
        add_connection         isp_emif_2_clk.out_clk          isp_se_vfw.clock

        # isp_emif_2_rst
        add_connection         isp_emif_2_rst.out_reset        isp_vfw.mem_reset
        add_connection         isp_emif_2_rst.out_reset        isp_se_vfw.reset

        # isp_axi4s_bcast
        add_connection         isp_axi4s_bcast.axi4s_vid_out_0      isp_dpc.axi4s_vid_in
        add_connection         isp_axi4s_bcast.axi4s_vid_out_1      isp_cpm.axi4s_vid_in

        # isp_cpm
        add_connection         isp_cpm.axi4s_vid_out          isp_vfw.axi4s_vid_in

        # isp_vfw
        add_connection         isp_vfw.av_mm_mem_write_host         isp_se_vfw.windowed_slave
    }

    # isp_dpc
    add_connection         isp_dpc.axi4s_vid_out          isp_anr.axi4s_vid_in

    # isp_anr
    add_connection         isp_anr.axi4s_vid_out          isp_blc.axi4s_vid_in

    # isp_blc
    add_connection         isp_blc.axi4s_vid_out          isp_vc.axi4s_vid_in

    # isp_vc
    add_connection         isp_vc.axi4s_vid_out           isp_switch_wbs.axi4s_vid_in_0

    # isp_switch_wbs
    add_connection         isp_switch_wbs.axi4s_vid_out_0         isp_wbc.axi4s_vid_in
    add_connection         isp_switch_wbs.axi4s_vid_out_1         isp_wbs.axi4s_vid_in
    add_connection         isp_switch_wbs.axi4s_vid_out_2         isp_dms.axi4s_vid_in

    # isp_wbc
    add_connection         isp_wbc.axi4s_vid_out          isp_switch_wbs.axi4s_vid_in_1

    # isp_wbs
    add_connection         isp_wbs.axi4s_vid_out          isp_switch_wbs.axi4s_vid_in_2

    # isp_dms
    add_connection         isp_dms.axi4s_vid_out          isp_hs.axi4s_vid_in

    # isp_hs
    add_connection         isp_hs.axi4s_vid_out           isp_ccm.axi4s_vid_in

    if {${v_hdr_en}} {
        # isp_ccm
        add_connection         isp_ccm.axi4s_vid_out              isp_1d_lut_hdr.axi4s_vid_in

        # isp_1d_lut_hdr
        add_connection         isp_1d_lut_hdr.axi4s_vid_out       isp_3d_lut_hdr_1.axi4s_vid_in

        # isp_3d_lut_hdr_1
        add_connection         isp_3d_lut_hdr_1.axi4s_vid_out     isp_3d_lut_hdr_2.axi4s_vid_in

        # isp_3d_lut_hdr_2
        add_connection         isp_3d_lut_hdr_2.axi4s_vid_out     isp_tmo.axi4s_vid_in
    } elseif {${v_3d_lut_en}} {
        # isp_ccm
        add_connection         isp_ccm.axi4s_vid_out              isp_3d_lut_hdr_2.axi4s_vid_in

        # isp_3d_lut_hdr_2
        add_connection         isp_3d_lut_hdr_2.axi4s_vid_out     isp_tmo.axi4s_vid_in
    } else {
        # isp_ccm
        add_connection         isp_ccm.axi4s_vid_out              isp_tmo.axi4s_vid_in
    }

    # isp_tmo
    add_connection         isp_tmo.axi4s_vid_out                isp_pix_adapt_tmo.axi4s_vid_in

    # isp_pix_adapt_tmo
    add_connection         isp_pix_adapt_tmo.axi4s_vid_out      isp_usm.axi4s_vid_in

    # isp_usm
    add_connection         isp_usm.axi4s_vid_out                isp_roi.axi4s_vid_in

    if {${v_warp_en}} {
        # isp_roi
        add_connection         isp_roi.axi4s_vid_out                isp_warp.axi4s_vid_in_0
        # isp_warp
        if {${v_ai_en}} {
            add_connection        isp_warp.axi4s_vid_out_0              isp_warp_throttle.axi4s_vid_in
        }

        # isp_warp
        add_connection         isp_warp.av_mm_memory_host           isp_se_warp.windowed_slave
    }


    ##########################
    ##### Create Exports #####
    ##########################

    # isp_cpu_clk
    add_interface           cpu_clk_in    clock       sink
    set_interface_property  cpu_clk_in    EXPORT_OF   isp_cpu_clk.in_clk

    # isp_cpu_rst
    add_interface           cpu_rst_in    reset       sink
    set_interface_property  cpu_rst_in    EXPORT_OF   isp_cpu_rst.in_reset

    # isp_mm_bridge
    add_interface           mm_ctrl_in    avalon      slave
    set_interface_property  mm_ctrl_in    EXPORT_OF   isp_mm_bridge.s0

    # isp_vid_clk
    set_interface_property  vid_clk_in    EXPORT_OF   isp_vid_clk.in_clk

    # isp_vid_rst
    set_interface_property  vid_rst_in    EXPORT_OF   isp_vid_rst.in_reset

    # isp_niosv_clk
    set_interface_property  niosv_clk_in  EXPORT_OF   isp_niosv_clk.in_clk

    # isp_niosv_rst
    set_interface_property  niosv_rst_in  EXPORT_OF   isp_niosv_rst.in_reset

    if {${v_warp_en}} {
        # isp_emif_clk
        set_interface_property  emif_clk_in   EXPORT_OF   isp_emif_clk.in_clk

        # isp_emif_rst
        set_interface_property  emif_rst_in   EXPORT_OF   isp_emif_rst.in_reset
    }

    add_interface           isp_in_s_vid_axis    axi4stream  subordinate
    if {${v_bls_en}} {
        # isp_bls
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_bls.axi4s_vid_in
    } elseif {${v_clipper_en}} {
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_clipper.axi4s_vid_in
    } elseif {${v_raw_snapshot_en}} {
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_axi4s_bcast.axi4s_vid_in
    } else {
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_dpc.axi4s_vid_in
    }

    if {${v_warp_en}} {
        # isp_se_warp
        set_interface_property  av_mm_host_warp    EXPORT_OF   isp_se_warp.expanded_master
        set_interface_property  warp_int           EXPORT_OF   isp_warp.interrupt

        if {${v_ai_en}} {
            # isp_warp_throttle
            add_interface           isp_out_m_vid_axis        axi4stream  manager
            set_interface_property  isp_out_m_vid_axis        EXPORT_OF   isp_warp_throttle.axi4s_vid_out
        } else {
            # isp_warp
            add_interface           full_isp_out_m_vid_axis        axi4stream  manager
            set_interface_property  full_isp_out_m_vid_axis        EXPORT_OF   isp_warp.axi4s_vid_out_0
        }
    } elseif {${v_ai_en}} {
        # isp_roi
        add_interface           isp_out_m_vid_axis        axi4stream  manager
        set_interface_property  isp_out_m_vid_axis        EXPORT_OF   isp_roi.axi4s_vid_out
    }

    if {${v_raw_snapshot_en}} {
        # isp_emif_2_clk
        set_interface_property  emif_2_clk_in   EXPORT_OF   isp_emif_2_clk.in_clk

        # isp_emif_2_rst
        set_interface_property  emif_2_rst_in   EXPORT_OF   isp_emif_2_rst.in_reset

        # isp_vfw
        set_interface_property  vfw_int    EXPORT_OF   isp_vfw.frame_writer_int

        # isp_se_vfw
        set_interface_property  av_mm_host_vfw    EXPORT_OF   isp_se_vfw.expanded_master
    }


    #################################
    ##### Assign Base Addresses #####
    #################################
    set v_cpm_addr_offset                 [get_shell_parameter CPM_ADDR_OFFSET]
    set v_vfw_addr_offset                 [get_shell_parameter VFW_ADDR_OFFSET]
    set v_bls_addr_offset                 [get_shell_parameter BLS_ADDR_OFFSET]
    set v_clipper_addr_offset             [get_shell_parameter CLIPPER_ADDR_OFFSET]
    set v_dpc_addr_offset                 [get_shell_parameter DPC_ADDR_OFFSET]
    set v_anr_addr_offset                 [get_shell_parameter ANR_ADDR_OFFSET]
    set v_blc_addr_offset                 [get_shell_parameter BLC_ADDR_OFFSET]
    set v_vc_addr_offset                  [get_shell_parameter VC_ADDR_OFFSET]
    set v_sw_wbs_addr_offset              [get_shell_parameter SW_WBS_ADDR_OFFSET]
    set v_wbc_addr_offset                 [get_shell_parameter WBC_ADDR_OFFSET]
    set v_wbs_addr_offset                 [get_shell_parameter WBS_ADDR_OFFSET]
    set v_dms_addr_offset                 [get_shell_parameter DMS_ADDR_OFFSET]
    set v_hs_addr_offset                  [get_shell_parameter HS_ADDR_OFFSET]
    set v_ccm_addr_offset                 [get_shell_parameter CCM_ADDR_OFFSET]
    set v_1d_lut_addr_offset              [get_shell_parameter 1D_LUT_ADDR_OFFSET]
    set v_3d_lut_hdr_1_addr_offset        [get_shell_parameter 3D_LUT_HDR_1_ADDR_OFFSET]
    set v_3d_lut_hdr_2_addr_offset        [get_shell_parameter 3D_LUT_HDR_2_ADDR_OFFSET]
    set v_tmo_addr_offset                 [get_shell_parameter TMO_ADDR_OFFSET]
    set v_usm_addr_offset                 [get_shell_parameter USM_ADDR_OFFSET]
    set v_roi_addr_offset                 [get_shell_parameter ROI_ADDR_OFFSET]
    set v_warp_addr_offset                [get_shell_parameter WARP_ADDR_OFFSET]
    set v_warp_throttle_addr_offset       [get_shell_parameter WARP_THROTTLE_ADDR_OFFSET]

    if {${v_raw_snapshot_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_cpm.av_mm_control_agent \
                                                                  baseAddress ${v_cpm_addr_offset}
        set_connection_parameter_value isp_mm_bridge.m0/isp_vfw.av_mm_control_agent \
                                                                  baseAddress ${v_vfw_addr_offset}
    }
    if {${v_bls_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_bls.av_mm_control_agent \
                                                                  baseAddress ${v_bls_addr_offset}
    }
    if {${v_clipper_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_clipper.av_mm_control_agent \
                                                                  baseAddress ${v_clipper_addr_offset}
    }
    set_connection_parameter_value isp_mm_bridge.m0/isp_dpc.av_mm_control_agent \
                                                                  baseAddress ${v_dpc_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_anr.av_mm_control_agent \
                                                                  baseAddress ${v_anr_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_blc.av_mm_control_agent \
                                                                  baseAddress ${v_blc_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_vc.av_mm_control_agent \
                                                                  baseAddress ${v_vc_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_switch_wbs.av_mm_control_agent \
                                                                  baseAddress ${v_sw_wbs_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_wbc.av_mm_control_agent \
                                                                  baseAddress ${v_wbc_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_wbs.av_mm_control_agent \
                                                                  baseAddress ${v_wbs_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_dms.av_mm_control_agent \
                                                                  baseAddress ${v_dms_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_hs.av_mm_control_agent \
                                                                  baseAddress ${v_hs_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_ccm.av_mm_control_agent \
                                                                  baseAddress ${v_ccm_addr_offset}
    if {${v_hdr_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_1d_lut_hdr.av_mm_control_agent \
                                                                  baseAddress ${v_1d_lut_addr_offset}
        set_connection_parameter_value isp_mm_bridge.m0/isp_3d_lut_hdr_1.av_mm_cpu_agent \
                                                                  baseAddress ${v_3d_lut_hdr_1_addr_offset}
    }
    if {${v_3d_lut_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_3d_lut_hdr_2.av_mm_cpu_agent \
                                                                  baseAddress ${v_3d_lut_hdr_2_addr_offset}
    }
    set_connection_parameter_value isp_mm_bridge.m0/isp_tmo.av_mm_cpu_agent \
                                                                  baseAddress ${v_tmo_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_usm.av_mm_control_agent \
                                                                  baseAddress ${v_usm_addr_offset}
    set_connection_parameter_value isp_mm_bridge.m0/isp_roi.av_mm_control_agent \
                                                                  baseAddress ${v_roi_addr_offset}
    if {${v_warp_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_warp.av_mm_control_agent \
                                                                  baseAddress ${v_warp_addr_offset}
        if {${v_ai_en}} {
            set_connection_parameter_value isp_mm_bridge.m0/isp_warp_throttle.av_mm_control_agent \
                                                                  baseAddress ${v_warp_throttle_addr_offset}
        }
    }

    if {${v_raw_snapshot_en}} {
        lock_avalon_base_address  isp_vfw.av_mm_control_agent
        lock_avalon_base_address  isp_cpm.av_mm_control_agent
    }
    if {${v_bls_en}} {
        lock_avalon_base_address  isp_bls.av_mm_control_agent
    }
    if {${v_clipper_en}} {
        lock_avalon_base_address  isp_clipper.av_mm_control_agent
    }
    lock_avalon_base_address  isp_dpc.av_mm_control_agent
    lock_avalon_base_address  isp_anr.av_mm_control_agent
    lock_avalon_base_address  isp_blc.av_mm_control_agent
    lock_avalon_base_address  isp_vc.av_mm_control_agent
    lock_avalon_base_address  isp_switch_wbs.av_mm_control_agent
    lock_avalon_base_address  isp_wbc.av_mm_control_agent
    lock_avalon_base_address  isp_wbs.av_mm_control_agent
    lock_avalon_base_address  isp_dms.av_mm_control_agent
    lock_avalon_base_address  isp_hs.av_mm_control_agent
    lock_avalon_base_address  isp_ccm.av_mm_control_agent
    if {${v_hdr_en}} {
        lock_avalon_base_address  isp_1d_lut_hdr.av_mm_control_agent
        lock_avalon_base_address  isp_3d_lut_hdr_1.av_mm_cpu_agent
    }
    if {${v_3d_lut_en}} {
        lock_avalon_base_address  isp_3d_lut_hdr_2.av_mm_cpu_agent
    }
    lock_avalon_base_address  isp_tmo.av_mm_cpu_agent
    lock_avalon_base_address  isp_usm.av_mm_control_agent
    lock_avalon_base_address  isp_roi.av_mm_control_agent
    if {${v_warp_en}} {
        lock_avalon_base_address  isp_warp.av_mm_control_agent
        if {${v_ai_en}} {
            lock_avalon_base_address  isp_warp_throttle.av_mm_control_agent
        }

        set_connection_parameter_value isp_warp.av_mm_memory_host/isp_se_warp.windowed_slave \
                                            qsys_mm.burstAdapterImplementation {PER_BURST_TYPE_CONVERTER}
        set_connection_parameter_value isp_warp.av_mm_memory_host/isp_se_warp.windowed_slave \
                                            qsys_mm.widthAdapterImplementation {OPTIMIZED_CONVERTER}
        set_domain_assignment isp_warp.av_mm_memory_host qsys_mm.burstAdapterImplementation PER_BURST_TYPE_CONVERTER
        set_domain_assignment isp_warp.av_mm_memory_host qsys_mm.widthAdapterImplementation OPTIMIZED_CONVERTER
    }


    #############################
    ##### Sync / Validation #####
    #############################

    sync_sysinfo_parameters
    save_system
}

proc  edit_top_level_qsys {} {
    set v_project_name      [get_shell_parameter PROJECT_NAME]
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_instance_name     [get_shell_parameter INSTANCE_NAME]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name} ${v_instance_name}

    sync_sysinfo_parameters
    save_system
}


proc add_auto_connections {} {
    set v_instance_name         [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host_clk_freq    [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_vid_clk_freq          [get_shell_parameter VID_CLK_FREQ]
    set v_tmo_nios_clk_freq     [get_shell_parameter TMO_NIOS_CLK_FREQ]
    set v_async_clk             [get_shell_parameter ASYNC_CLK]
    set v_warp_en               [get_shell_parameter WARP_EN]
    set v_emif_agent            [get_shell_parameter EMIF_AGENT]
    set v_ai_en                 [get_shell_parameter AI_EN]
    set v_raw_snapshot_en       [get_shell_parameter RAW_SNAPSHOT_EN]
    set v_emif_agent_2          [get_shell_parameter EMIF_AGENT_2]
    set v_avmm_host             [get_shell_parameter AVMM_HOST]

    add_auto_connection   ${v_instance_name} cpu_clk_in       ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name} cpu_rst_in       ${v_avmm_host_clk_freq}

    add_auto_connection   ${v_instance_name} vid_clk_in       ${v_vid_clk_freq}
    add_auto_connection   ${v_instance_name} vid_rst_in       ${v_vid_clk_freq}

    add_auto_connection   ${v_instance_name} niosv_clk_in     ${v_tmo_nios_clk_freq}
    add_auto_connection   ${v_instance_name} niosv_rst_in     ${v_tmo_nios_clk_freq}

    # DDR
    if {${v_warp_en}} {
        if {${v_async_clk}} {
            add_auto_connection   ${v_instance_name} emif_clk_in      [expr ${v_async_clk} * 1000000]
            add_auto_connection   ${v_instance_name} emif_rst_in      [expr ${v_async_clk} * 1000000]
        } else {
            add_auto_connection   ${v_instance_name} emif_clk_in      ${v_emif_agent}_user_clk
            add_auto_connection   ${v_instance_name} emif_rst_in      ${v_emif_agent}_user_rst
        }

        add_auto_connection   ${v_instance_name} av_mm_host_warp  ${v_emif_agent}_user_data
    }

    if {${v_raw_snapshot_en}} {
        add_auto_connection   ${v_instance_name} emif_2_clk_in    ${v_emif_agent_2}_user_clk
        add_auto_connection   ${v_instance_name} emif_2_rst_in    ${v_emif_agent_2}_user_rst
        add_auto_connection   ${v_instance_name} av_mm_host_vfw   ${v_emif_agent_2}_user_data
    }

    # from isp in
    add_auto_connection   ${v_instance_name} isp_in_s_vid_axis          isp_in_0_vid_axis

    if {${v_ai_en}} {
        # to ai
        add_auto_connection   ${v_instance_name} isp_out_m_vid_axis         isp_out_vid_axis
    } else {
        # to vid out
        add_auto_connection   ${v_instance_name} full_isp_out_m_vid_axis    full_isp_out_vid_axis
    }

    # HPS to mm bridge
    add_avmm_connections  mm_ctrl_in      ${v_avmm_host}
}


proc modify_avmm_arbitration {} {
    set v_project_name  [get_shell_parameter PROJECT_NAME]
    set v_project_path  [get_shell_parameter PROJECT_PATH]
    set v_instance_name [get_shell_parameter INSTANCE_NAME]

    # Give warp priority in to FPGA EMIF to avoid video glitches at output
    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    set v_warp_avmm_conn [get_connections ${v_instance_name}.av_mm_host_warp]
    set v_num_conns      [llength ${v_warp_avmm_conn}]

    if {${v_num_conns} > 0} {
        set_connection_parameter_value ${v_warp_avmm_conn} arbitrationPriority {16}
    }

    sync_sysinfo_parameters
    save_system
}
