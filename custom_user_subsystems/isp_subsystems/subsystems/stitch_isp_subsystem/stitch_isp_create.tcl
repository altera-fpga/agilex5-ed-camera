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
set_shell_parameter BLS_ADDR_OFFSET           "0x00009600"
set_shell_parameter CLIPPER_ADDR_OFFSET       "0x00008e00"
set_shell_parameter DPC_ADDR_OFFSET           "0x00008c00"
set_shell_parameter ANR_ADDR_OFFSET           "0x00010000"
set_shell_parameter BLC_ADDR_OFFSET           "0x00008a00"
set_shell_parameter VC_ADDR_OFFSET            "0x00040000"
set_shell_parameter SW_WBS_ADDR_OFFSET        "0x0000d000"
set_shell_parameter WBC_ADDR_OFFSET           "0x00009400"
set_shell_parameter WBS_ADDR_OFFSET           "0x0000c000"
set_shell_parameter DMS_ADDR_OFFSET           "0x00009200"
set_shell_parameter HS_ADDR_OFFSET            "0x0000e000"
set_shell_parameter CCM_ADDR_OFFSET           "0x00009a00"
set_shell_parameter ROI_ADDR_OFFSET           "0x0000a800"
set_shell_parameter HSCALER_ADDR_OFFSET       "0x00008000"
set_shell_parameter VSCALER_ADDR_OFFSET       "0x00008400"
set_shell_parameter WARP_ADDR_OFFSET          "0x00000000"
set_shell_parameter VFR_ADDR_OFFSET           "0x0000a000"
set_shell_parameter UPSCALER_ADDR_OFFSET      "0x0000a400"
set_shell_parameter ALPHA_CH_ADDR_OFFSET      "0x0000a600"

# General Video Controls
set_shell_parameter VID_CLK_FREQ              {297000000.0}
set_shell_parameter PIP                       {2}
set_shell_parameter EN_DEBUG                  {1}

set_shell_parameter INST_ID                   {0}
set_shell_parameter IO_ID                     {0}

# Warp Controls
set_shell_parameter WARP_SB                   {0}
set_shell_parameter WARP_MM                   {1}
set_shell_parameter WARP_CACHE                {512}

# Other Controls.
set_shell_parameter EMIF_AGENT                {}
set_shell_parameter EMIF_AGENT_CLK_FREQ       {200000000.0}
set_shell_parameter ALPHA_EN                  {0}
set_shell_parameter VFR_ALPHA_EN              {0}
set_shell_parameter SCALE_UP_ALPHA_EN         {0}
set_shell_parameter SMALL_VC                  {1}
set_shell_parameter SMALL_ANR                 {0}
set_shell_parameter BLS_EN                    {1}
set_shell_parameter CLIPPER_EN                {1}
set_shell_parameter EXT_SCALER_EN             {0}
set_shell_parameter H_SCALER_MAX_OUT          {2560}
set_shell_parameter V_SCALER_MAX_OUT          {1440}


# resolve interdependencies
proc derive_parameters {param_array} {
    upvar $param_array p_array

    # check if the emif agent has been configured
    set v_emif_agent          [get_shell_parameter EMIF_AGENT]

    if { ([llength ${v_emif_agent}] == 0) } {
        send_message ERROR "stitch_isp_create: EMIF Agent not specified"
    }
}

proc pre_creation_step {} {
    transfer_files
}

proc creation_step {} {
    create_stitch_isp_subsystem
}

proc post_creation_step {} {
    edit_top_level_qsys
    add_auto_connections
}

proc post_connection_step {} {
    modify_avmm_arbitration
}

proc transfer_files {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_script_path       [get_shell_parameter SUBSYSTEM_SOURCE_PATH]

    exec cp -rf ${v_script_path}/../../non_qpds_ip/altera_vvp_alpha_channel       ${v_project_path}/non_qpds_ip/user
    file_copy   ${v_script_path}/../../non_qpds_ip/altera_vvp_alpha_channel.ipx   ${v_project_path}/non_qpds_ip/user

    exec cp -rf ${v_script_path}/../../non_qpds_ip/altera_vvp_roi                 ${v_project_path}/non_qpds_ip/user
    file_copy   ${v_script_path}/../../non_qpds_ip/altera_vvp_roi.ipx             ${v_project_path}/non_qpds_ip/user
}

proc create_stitch_isp_subsystem {} {
    set v_project_path            [get_shell_parameter PROJECT_PATH]
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]

    # video Pipeline
    set v_vid_clk_freq            [get_shell_parameter VID_CLK_FREQ]
    set v_cppp                    {3}
    set v_bps                     {10}
    set v_pip                     [get_shell_parameter PIP]
    set v_alpha_cp                {1}
    set v_cppp_with_alpha         [expr ${v_cppp} + ${v_alpha_cp}]

    # EMIF
    set v_emif_agent_clk_freq     [get_shell_parameter EMIF_AGENT_CLK_FREQ]

    # Warp
    if {${v_pip} > 1} {
        set v_num_warp_engines        {2}
    } else {
        set v_num_warp_engines        {1}
    }
    set v_warp_debug              [get_shell_parameter EN_DEBUG]
    set v_warp_single_bounce      [get_shell_parameter WARP_SB]
    set v_warp_mipmap_enable      [get_shell_parameter WARP_MM]
    set v_warp_cache_blocks       [get_shell_parameter WARP_CACHE]

    # ISP Pipeline
    set v_isp_cppp                {1}
    set v_isp_bps                 {12}

    # Others
    set v_small_vc                [get_shell_parameter SMALL_VC]
    set v_small_anr               [get_shell_parameter SMALL_ANR]
    set v_bls_en                  [get_shell_parameter BLS_EN]
    set v_clipper_en              [get_shell_parameter CLIPPER_EN]
    set v_alpha_en                [get_shell_parameter ALPHA_EN]
    if {${v_alpha_en}} {
        set v_vfr_alpha_en            [get_shell_parameter VFR_ALPHA_EN]
        set v_scale_up_alpha_en       [get_shell_parameter SCALE_UP_ALPHA_EN]
    } else {
        set v_vfr_alpha_en            {0}
        set v_scale_up_alpha_en       {0}
    }

    set v_ext_scaler_en           [get_shell_parameter EXT_SCALER_EN]
    set v_h_scaler_max_out        [get_shell_parameter H_SCALER_MAX_OUT]
    set v_v_scaler_max_out        [get_shell_parameter V_SCALER_MAX_OUT]

    # General
    set v_inst_id                 [get_shell_parameter INST_ID]
    set v_enable_debug            [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready          {1}

    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]

    # Switch Mode - Crash when ASYNC_IP_SW = 1, else Sync Switch on SOF with SYNC_IP_SW = 1, else on EOL
    set v_wbs_async_ip_sw         {0}
    set v_wbs_sync_ip_sw          {1}


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  isp_cpu_clk           altera_clock_bridge
    add_instance  isp_cpu_rst           altera_reset_bridge
    add_instance  isp_mm_bridge         altera_avalon_mm_bridge
    add_instance  isp_vid_clk           altera_clock_bridge
    add_instance  isp_vid_rst           altera_reset_bridge
    add_instance  isp_emif_clk          altera_clock_bridge
    add_instance  isp_emif_rst          altera_reset_bridge
    if {${v_bls_en}} {
        add_instance  isp_bls               intel_vvp_bls
    }
    if {${v_clipper_en}} {
        add_instance  isp_clipper           intel_vvp_clipper
    }
    add_instance  isp_dpc               intel_vvp_dpc
    add_instance  isp_anr               intel_vvp_anr
    add_instance  isp_blc               intel_vvp_blc
    add_instance  isp_vc                intel_vvp_vc
    add_instance  isp_switch_wbs        intel_vvp_switch
    add_instance  isp_wbc               intel_vvp_wbc
    add_instance  isp_wbs               intel_vvp_wbs
    add_instance  isp_dms               intel_vvp_demosaic
    add_instance  isp_hs                intel_vvp_hs
    add_instance  isp_ccm               intel_vvp_csc
    add_instance  isp_roi               altera_vvp_roi
    if {${v_ext_scaler_en}} {
        add_instance  isp_h_scaler_down     intel_vvp_scaler
        add_instance  isp_v_scaler_down     intel_vvp_scaler
    }
    add_instance  isp_pix_adapt         intel_vvp_pixel_adapter
    add_instance  isp_warp              intel_vvp_warp
    add_instance  isp_se_warp           altera_address_span_extender
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_instance  isp_alpha_vfr         intel_vvp_vfr
            add_instance  isp_alpha_se_vfr      altera_address_span_extender
            if {${v_scale_up_alpha_en}} {
                add_instance  isp_alpha_up_scaler   intel_vvp_scaler
            }
            add_instance  isp_alpha_cpm         intel_vvp_cpm
        } else {
            add_instance  isp_alpha_channel     altera_vvp_alpha_channel
        }
    }


    ############################
    #### Set Parameters     ####
    ############################

    # isp_cpu_clk
    set_instance_parameter_value    isp_cpu_clk       EXPLICIT_CLOCK_RATE         ${v_avmm_host_clk_freq}
    set_instance_parameter_value    isp_cpu_clk       NUM_CLOCK_OUTPUTS           {1}

    # isp_cpu_rst
    set_instance_parameter_value    isp_cpu_rst       ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value    isp_cpu_rst       NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    isp_cpu_rst       SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    isp_cpu_rst       SYNC_RESET                  {0}
    set_instance_parameter_value    isp_cpu_rst       USE_RESET_REQUEST           {0}

    # isp_mm_bridge
    set_instance_parameter_value    isp_mm_bridge     ADDRESS_UNITS               {SYMBOLS}
    set_instance_parameter_value    isp_mm_bridge     ADDRESS_WIDTH               {0}
    set_instance_parameter_value    isp_mm_bridge     DATA_WIDTH                  {32}
    set_instance_parameter_value    isp_mm_bridge     LINEWRAPBURSTS              {0}
    set_instance_parameter_value    isp_mm_bridge     M0_WAITREQUEST_ALLOWANCE    {0}
    set_instance_parameter_value    isp_mm_bridge     MAX_BURST_SIZE              {1}
    set_instance_parameter_value    isp_mm_bridge     MAX_PENDING_RESPONSES       {4}
    set_instance_parameter_value    isp_mm_bridge     MAX_PENDING_WRITES          {0}
    set_instance_parameter_value    isp_mm_bridge     PIPELINE_COMMAND            {1}
    set_instance_parameter_value    isp_mm_bridge     PIPELINE_RESPONSE           {1}
    set_instance_parameter_value    isp_mm_bridge     S0_WAITREQUEST_ALLOWANCE    {0}
    set_instance_parameter_value    isp_mm_bridge     SYMBOL_WIDTH                {8}
    set_instance_parameter_value    isp_mm_bridge     SYNC_RESET                  {0}
    set_instance_parameter_value    isp_mm_bridge     USE_AUTO_ADDRESS_WIDTH      {1}
    set_instance_parameter_value    isp_mm_bridge     USE_RESPONSE                {0}
    set_instance_parameter_value    isp_mm_bridge     USE_WRITERESPONSE           {0}

    # isp_vid_clk
    set_instance_parameter_value    isp_vid_clk       EXPLICIT_CLOCK_RATE         ${v_vid_clk_freq}
    set_instance_parameter_value    isp_vid_clk       NUM_CLOCK_OUTPUTS           {1}

    # isp_vid_rst
    set_instance_parameter_value    isp_vid_rst       ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value    isp_vid_rst       NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    isp_vid_rst       SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    isp_vid_rst       SYNC_RESET                  {0}
    set_instance_parameter_value    isp_vid_rst       USE_RESET_REQUEST           {0}

    # isp_emif_clk
    set_instance_parameter_value    isp_emif_clk      EXPLICIT_CLOCK_RATE         ${v_emif_agent_clk_freq}
    set_instance_parameter_value    isp_emif_clk      NUM_CLOCK_OUTPUTS           {1}

    # isp_emif_rst
    set_instance_parameter_value    isp_emif_rst      ACTIVE_LOW_RESET            {1}
    set_instance_parameter_value    isp_emif_rst      NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    isp_emif_rst      SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    isp_emif_rst      SYNC_RESET                  {0}
    set_instance_parameter_value    isp_emif_rst      USE_RESET_REQUEST           {0}

    if {${v_bls_en}} {
        # isp_bls
        set_instance_parameter_value    isp_bls           AV_MAX_PENDING_READS        {8}
        set_instance_parameter_value    isp_bls           BPS                         ${v_isp_bps}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_TYPE             {372}
        set_instance_parameter_value    isp_bls           C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value    isp_bls           DUPLICATE_AND_BYPASS        {1}
        set_instance_parameter_value    isp_bls           ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value    isp_bls           ENABLE_EXT_DATA_RW          {0}
        set_instance_parameter_value    isp_bls           EXTERNAL_MODE               {1}
        set_instance_parameter_value    isp_bls           H_TAPS                      {1}
        set_instance_parameter_value    isp_bls           MAX_HEIGHT                  {4096}
        set_instance_parameter_value    isp_bls           MAX_WIDTH                   {4096}
        set_instance_parameter_value    isp_bls           NO_BLANKING                 {1}
        set_instance_parameter_value    isp_bls           NUMBER_OF_COLOR_PLANES      ${v_isp_cppp}
        set_instance_parameter_value    isp_bls           NUM_EXT_DATA_REGS           {0}
        set_instance_parameter_value    isp_bls           PIPELINE_DATA_MM            {0}
        set_instance_parameter_value    isp_bls           PIPELINE_READY              ${v_pipeline_ready}
        set_instance_parameter_value    isp_bls           PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value    isp_bls           P_CORE_CTRL_ID              {0}
        set_instance_parameter_value    isp_bls           P_UPDATE_CMD_SUPPORTED      {0}
        set_instance_parameter_value    isp_bls           RUNTIME_CONTROL             {1}
        set_instance_parameter_value    isp_bls           SEPARATE_SLAVE_CLOCK        {1}
        set_instance_parameter_value    isp_bls           SLAVE_PROTOCOL              {Avalon}
        set_instance_parameter_value    isp_bls           V_TAPS                      {1}
    }

    if {${v_clipper_en}} {
        # isp_clipper
        set_instance_parameter_value    isp_clipper       BOTTOM_OFFSET               {0}
        set_instance_parameter_value    isp_clipper       BPS                         ${v_isp_bps}
        set_instance_parameter_value    isp_clipper       CLIPPING_METHOD             {OFFSETS}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_TYPE             {557}
        set_instance_parameter_value    isp_clipper       C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value    isp_clipper       ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value    isp_clipper       EXTERNAL_MODE               {1}
        set_instance_parameter_value    isp_clipper       LEFT_OFFSET                 {0}
        set_instance_parameter_value    isp_clipper       NUMBER_OF_COLOR_PLANES      ${v_isp_cppp}
        set_instance_parameter_value    isp_clipper       PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value    isp_clipper       RECTANGLE_HEIGHT            {2160}
        set_instance_parameter_value    isp_clipper       RECTANGLE_WIDTH             {3840}
        set_instance_parameter_value    isp_clipper       RIGHT_OFFSET                {0}
        set_instance_parameter_value    isp_clipper       RUNTIME_CONTROL             {0}
        set_instance_parameter_value    isp_clipper       SEPARATE_SLAVE_CLOCK        {1}
        set_instance_parameter_value    isp_clipper       SLAVE_PROTOCOL              {Avalon}
        set_instance_parameter_value    isp_clipper       TOP_OFFSET                  {0}
    }

    # isp_dpc
    set_instance_parameter_value    isp_dpc           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_dpc           BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_dpc           BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_TYPE             {373}
    set_instance_parameter_value    isp_dpc           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_dpc           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_dpc           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_dpc           ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_dpc           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_dpc           H_TAPS                      {5}
    set_instance_parameter_value    isp_dpc           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_dpc           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_dpc           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_dpc           NUMBER_OF_COLOR_PLANES_IN   ${v_isp_cppp}
    set_instance_parameter_value    isp_dpc           NUMBER_OF_COLOR_PLANES_OUT  ${v_isp_cppp}
    set_instance_parameter_value    isp_dpc           NUM_EXT_DATA_REGS           {0}
    set_instance_parameter_value    isp_dpc           PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    isp_dpc           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_dpc           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_dpc           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_dpc           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_dpc           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_dpc           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_dpc           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_dpc           V_TAPS                      {5}

    # isp_anr
    set_instance_parameter_value    isp_anr           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_anr           BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_anr           BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_TYPE             {374}
    set_instance_parameter_value    isp_anr           C_OMNI_CAP_VERSION          {2}
    set_instance_parameter_value    isp_anr           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_anr           CFA_ENABLE                  {1}
    set_instance_parameter_value    isp_anr           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_anr           ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_anr           EXTERNAL_MODE               {1}
    if {${v_small_anr}} {
        set_instance_parameter_value    isp_anr           H_TAPS                      {9}
        set_instance_parameter_value    isp_anr           V_TAPS                      {9}
    } else {
        set_instance_parameter_value    isp_anr           H_TAPS                      {17}
        set_instance_parameter_value    isp_anr           V_TAPS                      {17}
    }
    set_instance_parameter_value    isp_anr           MAX_HEIGHT                  {65536}
    set_instance_parameter_value    isp_anr           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_anr           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_anr           NUMBER_OF_COLOR_PLANES      ${v_isp_cppp}
    set_instance_parameter_value    isp_anr           NUM_EXT_DATA_REGS           {2048}
    set_instance_parameter_value    isp_anr           PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    isp_anr           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_anr           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_anr           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_anr           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_anr           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_anr           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_anr           SLAVE_PROTOCOL              {Avalon}

    # isp_blc
    set_instance_parameter_value    isp_blc           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_blc           BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_blc           BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_TYPE             {375}
    set_instance_parameter_value    isp_blc           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_blc           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_blc           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_blc           ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_blc           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_blc           H_TAPS                      {1}
    set_instance_parameter_value    isp_blc           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_blc           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_blc           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_blc           NUMBER_OF_COLOR_PLANES_IN   ${v_isp_cppp}
    set_instance_parameter_value    isp_blc           NUMBER_OF_COLOR_PLANES_OUT  ${v_isp_cppp}
    set_instance_parameter_value    isp_blc           NUM_EXT_DATA_REGS           {0}
    set_instance_parameter_value    isp_blc           PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    isp_blc           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_blc           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_blc           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_blc           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_blc           REFLECT_AROUND_ZERO         {1}
    set_instance_parameter_value    isp_blc           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_blc           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_blc           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_blc           V_TAPS                      {1}

    # isp_vc
    set_instance_parameter_value    isp_vc            AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_vc            BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_vc            BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_vc            CFA_ENABLE                  {1}
    set_instance_parameter_value    isp_vc            CFA_NUM_OF_COLOR_PLANES     ${v_isp_cppp}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_TYPE             {376}
    set_instance_parameter_value    isp_vc            C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_vc            DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_vc            ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_vc            ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_vc            EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_vc            H_TAPS                      {1}
    if {${v_small_vc}} {
        set_instance_parameter_value    isp_vc            MAX_GAIN_MESH_POINTS        {1024}
        set_instance_parameter_value    isp_vc            PER_COLOR_GAIN_ENABLE       {0}
    } else {
        set_instance_parameter_value    isp_vc            MAX_GAIN_MESH_POINTS        {4096}
        set_instance_parameter_value    isp_vc            PER_COLOR_GAIN_ENABLE       {1}
    }
    set_instance_parameter_value    isp_vc            MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_vc            MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_vc            NO_BLANKING                 {1}
    set_instance_parameter_value    isp_vc            NUM_EXT_DATA_REGS           {32768}
    set_instance_parameter_value    isp_vc            PIPELINE_DATA_MM            {1}
    set_instance_parameter_value    isp_vc            PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_vc            PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_vc            P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_vc            P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_vc            RGB_NUM_OF_COLOR_PLANES     ${v_isp_cppp}
    set_instance_parameter_value    isp_vc            RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_vc            SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_vc            SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_vc            V_TAPS                      {1}

    # isp_switch_wbs
    set_instance_parameter_value    isp_switch_wbs    BPS                         ${v_isp_bps}
    set_instance_parameter_value    isp_switch_wbs    CRASH_SWITCH                ${v_wbs_async_ip_sw}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_TYPE             {565}
    set_instance_parameter_value    isp_switch_wbs    C_OMNI_CAP_VERSION          {2}
    set_instance_parameter_value    isp_switch_wbs    ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_switch_wbs    EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_switch_wbs    NUMBER_OF_COLOR_PLANES      ${v_isp_cppp}
    set_instance_parameter_value    isp_switch_wbs    NUM_INPUTS                  {3}
    set_instance_parameter_value    isp_switch_wbs    NUM_OUTPUTS                 {3}
    set_instance_parameter_value    isp_switch_wbs    PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_switch_wbs    SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_switch_wbs    SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_switch_wbs    UNINTERRUPTED_INPUTS        ${v_wbs_sync_ip_sw}
    set_instance_parameter_value    isp_switch_wbs    USE_OP_RESP                 {0}
    set_instance_parameter_value    isp_switch_wbs    USE_TREADIES                {1}
    set_instance_parameter_value    isp_switch_wbs    VVP_INTF_TYPE               {VVP_LITE}

    # isp_wbc
    set_instance_parameter_value    isp_wbc           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_wbc           BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_wbc           BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_TYPE             {378}
    set_instance_parameter_value    isp_wbc           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_wbc           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_wbc           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_wbc           ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_wbc           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_wbc           H_TAPS                      {1}
    set_instance_parameter_value    isp_wbc           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_wbc           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_wbc           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_wbc           NUMBER_OF_COLOR_PLANES_IN   ${v_isp_cppp}
    set_instance_parameter_value    isp_wbc           NUMBER_OF_COLOR_PLANES_OUT  ${v_isp_cppp}
    set_instance_parameter_value    isp_wbc           NUM_EXT_DATA_REGS           {0}
    set_instance_parameter_value    isp_wbc           PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    isp_wbc           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_wbc           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_wbc           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_wbc           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_wbc           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_wbc           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_wbc           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_wbc           V_TAPS                      {1}

    # isp_wbs
    set_instance_parameter_value    isp_wbs           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_wbs           BPS                         ${v_isp_bps}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_TYPE             {377}
    set_instance_parameter_value    isp_wbs           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_wbs           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_wbs           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_wbs           ENABLE_EXT_DATA_RW          {1}
    set_instance_parameter_value    isp_wbs           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_wbs           H_TAPS                      {1}
    set_instance_parameter_value    isp_wbs           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_wbs           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_wbs           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_wbs           NUMBER_OF_COLOR_PLANES      ${v_isp_cppp}
    set_instance_parameter_value    isp_wbs           NUM_EXT_DATA_REGS           {512}
    set_instance_parameter_value    isp_wbs           PIPELINE_DATA_MM            {1}
    set_instance_parameter_value    isp_wbs           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_wbs           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_wbs           PRECISION_BITS              {8}
    set_instance_parameter_value    isp_wbs           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_wbs           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_wbs           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_wbs           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_wbs           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_wbs           V_TAPS                      {2}

    # isp_dms
    set_instance_parameter_value    isp_dms           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_dms           BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_dms           BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_TYPE             {582}
    set_instance_parameter_value    isp_dms           C_OMNI_CAP_VERSION          {2}
    set_instance_parameter_value    isp_dms           DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    isp_dms           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_dms           ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    isp_dms           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_dms           H_TAPS                      {5}
    set_instance_parameter_value    isp_dms           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_dms           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_dms           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_dms           NUMBER_OF_COLOR_PLANES_IN   ${v_isp_cppp}
    set_instance_parameter_value    isp_dms           NUMBER_OF_COLOR_PLANES_OUT  ${v_cppp}
    set_instance_parameter_value    isp_dms           NUM_EXT_DATA_REGS           {0}
    set_instance_parameter_value    isp_dms           PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    isp_dms           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_dms           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_dms           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_dms           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_dms           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_dms           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_dms           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_dms           V_TAPS                      {5}

    # isp_hs
    set_instance_parameter_value    isp_hs           AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    isp_hs           BPS                         ${v_isp_bps}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_TYPE             {379}
    set_instance_parameter_value    isp_hs           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_hs           DUPLICATE_AND_BYPASS        {1}
    set_instance_parameter_value    isp_hs           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_hs           ENABLE_EXT_DATA_RW          {1}
    set_instance_parameter_value    isp_hs           EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_hs           H_TAPS                      {1}
    set_instance_parameter_value    isp_hs           MAX_HEIGHT                  {4096}
    set_instance_parameter_value    isp_hs           MAX_WIDTH                   {4096}
    set_instance_parameter_value    isp_hs           NO_BLANKING                 {1}
    set_instance_parameter_value    isp_hs           NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    isp_hs           NUM_HIST_BINS               {256}
    set_instance_parameter_value    isp_hs           PIPELINE_DATA_MM            {1}
    set_instance_parameter_value    isp_hs           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_hs           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_hs           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_hs           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_hs           RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_hs           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_hs           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_hs           V_TAPS                      {1}

    # isp_ccm
    set_instance_parameter_value    isp_ccm          BPS_IN                      ${v_isp_bps}
    set_instance_parameter_value    isp_ccm          BPS_OUT                     ${v_isp_bps}
    set_instance_parameter_value    isp_ccm          COEFFICIENT_INT_BITS        {8}
    set_instance_parameter_value    isp_ccm          COEFFICIENT_SIGNED          {1}
    set_instance_parameter_value    isp_ccm          COEF_SUM_FRACTION_BITS      {8}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_TYPE             {559}
    set_instance_parameter_value    isp_ccm          C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    isp_ccm          ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    isp_ccm          EXTERNAL_MODE               {1}
    set_instance_parameter_value    isp_ccm          MOVE_BINARY_POINT_RIGHT     {0}
    set_instance_parameter_value    isp_ccm          OUTPUT_COLORSPACE           {0}
    set_instance_parameter_value    isp_ccm          PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    isp_ccm          PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    isp_ccm          P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    isp_ccm          P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    isp_ccm          REMOVE_FRACTION_METHOD      {1}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_A0               {1.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_A1               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_A2               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_B0               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_B1               {1.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_B2               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_C0               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_C1               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_C2               {1.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_S0               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_S1               {0.0}
    set_instance_parameter_value    isp_ccm          REQ_FCOEFF_S2               {0.0}
    set_instance_parameter_value    isp_ccm          RUNTIME_CONTROL             {1}
    set_instance_parameter_value    isp_ccm          SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    isp_ccm          SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    isp_ccm          SUMMAND_INT_BITS            {10}
    set_instance_parameter_value    isp_ccm          SUMMAND_SIGNED              {1}

    # isp_roi
    set_instance_parameter_value      isp_roi        BPS                           ${v_isp_bps}
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

    if {${v_ext_scaler_en}} {
        # isp_ai_h_scaler_down
        set_instance_parameter_value      isp_h_scaler_down    ALGORITHM                     {POLYPHASE}
        set_instance_parameter_value      isp_h_scaler_down    BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_TYPE               {564}
        set_instance_parameter_value      isp_h_scaler_down    C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_h_scaler_down    EDGE_MIRROR                   {REPLICATE}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_420                    {0}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_420_MIRROR             {0}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_422                    {0}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_444                    {1}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_H                      {1}
        set_instance_parameter_value      isp_h_scaler_down    ENABLE_V                      {0}
        set_instance_parameter_value      isp_h_scaler_down    EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_h_scaler_down    HALF_RATE_420                 {0}
        set_instance_parameter_value      isp_h_scaler_down    H_BANKS                       {1}
        set_instance_parameter_value      isp_h_scaler_down    H_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      isp_h_scaler_down    H_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      isp_h_scaler_down    H_COEFF_INT_BITS              {1}
        set_instance_parameter_value      isp_h_scaler_down    H_COEFF_SIGNED                {1}
        set_instance_parameter_value      isp_h_scaler_down    H_INIT_FILE \
                                                                          {<enter file name (including full path)>}
        set_instance_parameter_value      isp_h_scaler_down    H_PARTIAL_SCALING             {0}
        set_instance_parameter_value      isp_h_scaler_down    H_PHASES                      {16}
        set_instance_parameter_value      isp_h_scaler_down    H_TAPS                        {12}
        set_instance_parameter_value      isp_h_scaler_down    MAX_IN_WIDTH                  {3840}
        set_instance_parameter_value      isp_h_scaler_down    MAX_OUT_WIDTH                 ${v_h_scaler_max_out}
        set_instance_parameter_value      isp_h_scaler_down    MEM_INIT                      {1}
        set_instance_parameter_value      isp_h_scaler_down    NO_BLANKING                   {0}
        set_instance_parameter_value      isp_h_scaler_down    NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      isp_h_scaler_down    OUTPUT_HEIGHT                 ${v_v_scaler_max_out}
        set_instance_parameter_value      isp_h_scaler_down    PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value      isp_h_scaler_down    PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_h_scaler_down    P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      isp_h_scaler_down    P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      isp_h_scaler_down    RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_h_scaler_down    RUNTIME_LOAD                  {1}
        set_instance_parameter_value      isp_h_scaler_down    SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_h_scaler_down    SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_h_scaler_down    V_BANKS                       {1}
        set_instance_parameter_value      isp_h_scaler_down    V_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      isp_h_scaler_down    V_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      isp_h_scaler_down    V_COEFF_INT_BITS              {1}
        set_instance_parameter_value      isp_h_scaler_down    V_COEFF_SIGNED                {1}
        set_instance_parameter_value      isp_h_scaler_down    V_INIT_FILE \
                                                                          {<enter file name (including full path)>}
        set_instance_parameter_value      isp_h_scaler_down    V_PARTIAL_SCALING             {0}
        set_instance_parameter_value      isp_h_scaler_down    V_PHASES                      {16}
        set_instance_parameter_value      isp_h_scaler_down    V_PRES_FRAC_BITS              {0}
        set_instance_parameter_value      isp_h_scaler_down    V_TAPS                        {12}

        # isp_ai_v_scaler_down
        set_instance_parameter_value      isp_v_scaler_down    ALGORITHM                     {POLYPHASE}
        set_instance_parameter_value      isp_v_scaler_down    BPS                           ${v_isp_bps}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_TYPE               {564}
        set_instance_parameter_value      isp_v_scaler_down    C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      isp_v_scaler_down    EDGE_MIRROR                   {REPLICATE}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_420                    {0}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_420_MIRROR             {0}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_422                    {0}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_444                    {1}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_H                      {0}
        set_instance_parameter_value      isp_v_scaler_down    ENABLE_V                      {1}
        set_instance_parameter_value      isp_v_scaler_down    EXTERNAL_MODE                 {1}
        set_instance_parameter_value      isp_v_scaler_down    HALF_RATE_420                 {0}
        set_instance_parameter_value      isp_v_scaler_down    H_BANKS                       {1}
        set_instance_parameter_value      isp_v_scaler_down    H_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      isp_v_scaler_down    H_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      isp_v_scaler_down    H_COEFF_INT_BITS              {1}
        set_instance_parameter_value      isp_v_scaler_down    H_COEFF_SIGNED                {1}
        set_instance_parameter_value      isp_v_scaler_down    H_INIT_FILE \
                                                                          {<enter file name (including full path)>}
        set_instance_parameter_value      isp_v_scaler_down    H_PARTIAL_SCALING             {0}
        set_instance_parameter_value      isp_v_scaler_down    H_PHASES                      {16}
        set_instance_parameter_value      isp_v_scaler_down    H_TAPS                        {12}
        set_instance_parameter_value      isp_v_scaler_down    MAX_IN_WIDTH                  ${v_h_scaler_max_out}
        set_instance_parameter_value      isp_v_scaler_down    MAX_OUT_WIDTH                 ${v_h_scaler_max_out}
        set_instance_parameter_value      isp_v_scaler_down    MEM_INIT                      {1}
        set_instance_parameter_value      isp_v_scaler_down    NO_BLANKING                   {0}
        set_instance_parameter_value      isp_v_scaler_down    NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      isp_v_scaler_down    OUTPUT_HEIGHT                 ${v_v_scaler_max_out}
        set_instance_parameter_value      isp_v_scaler_down    PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value      isp_v_scaler_down    PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      isp_v_scaler_down    P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      isp_v_scaler_down    P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      isp_v_scaler_down    RUNTIME_CONTROL               {1}
        set_instance_parameter_value      isp_v_scaler_down    RUNTIME_LOAD                  {1}
        set_instance_parameter_value      isp_v_scaler_down    SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      isp_v_scaler_down    SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      isp_v_scaler_down    V_BANKS                       {1}
        set_instance_parameter_value      isp_v_scaler_down    V_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      isp_v_scaler_down    V_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      isp_v_scaler_down    V_COEFF_INT_BITS              {1}
        set_instance_parameter_value      isp_v_scaler_down    V_COEFF_SIGNED                {1}
        set_instance_parameter_value      isp_v_scaler_down    V_INIT_FILE \
                                                                          {<enter file name (including full path)>}
        set_instance_parameter_value      isp_v_scaler_down    V_PARTIAL_SCALING             {0}
        set_instance_parameter_value      isp_v_scaler_down    V_PHASES                      {16}
        set_instance_parameter_value      isp_v_scaler_down    V_PRES_FRAC_BITS              {0}
        set_instance_parameter_value      isp_v_scaler_down    V_TAPS                        {12}
    }

    # isp_pix_adapt
    set_instance_parameter_value      isp_pix_adapt     BPS_IN                        ${v_isp_bps}
    set_instance_parameter_value      isp_pix_adapt     BPS_OUT                       ${v_bps}
    set_instance_parameter_value      isp_pix_adapt     ENABLE_DEBUG                  ${v_enable_debug}
    set_instance_parameter_value      isp_pix_adapt     EXTERNAL_MODE                 {1}
    set_instance_parameter_value      isp_pix_adapt     NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value      isp_pix_adapt     PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value      isp_pix_adapt     PIXELS_IN_PARALLEL            ${v_pip}
    set_instance_parameter_value      isp_pix_adapt     P_CORE_CTRL_ID                {0}
    set_instance_parameter_value      isp_pix_adapt     P_UPDATE_CMD_SUPPORTED        {0}
    set_instance_parameter_value      isp_pix_adapt     RUNTIME_CONTROL               {0}
    set_instance_parameter_value      isp_pix_adapt     SEPARATE_SLAVE_CLOCK          {0}
    set_instance_parameter_value      isp_pix_adapt     SLAVE_PROTOCOL                {Avalon}

    # isp_warp
    set_instance_parameter_value    isp_warp            BPS                       ${v_bps}
    set_instance_parameter_value    isp_warp            CACHE_BLOCKS              ${v_warp_cache_blocks}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_ID_ASSOCIATED  {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_ID_COMPONENT   ${v_inst_id}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_IRQ            {255}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_IRQ_ENABLE     {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_IRQ_ENABLE_EN  {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_IRQ_STATUS     {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_IRQ_STATUS_EN  {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_TAG            {0}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_TYPE           {367}
    set_instance_parameter_value    isp_warp            C_OMNI_CAP_VERSION        {1}
    set_instance_parameter_value    isp_warp            DEBUG_ENABLE              ${v_warp_debug}
    set_instance_parameter_value    isp_warp            EASY_WARP                 {0}
    set_instance_parameter_value    isp_warp            EXTERNAL_MODE             {1}
    set_instance_parameter_value    isp_warp            MAX_INPUT_WIDTH           {3840}
    set_instance_parameter_value    isp_warp            MAX_OUTPUT_WIDTH          {3840}
    set_instance_parameter_value    isp_warp            MEMORY_MAP                {2}
    set_instance_parameter_value    isp_warp            MIPMAP_ENABLE             ${v_warp_mipmap_enable}
    set_instance_parameter_value    isp_warp            NUMBER_OF_COLOR_PLANES    ${v_cppp}
    set_instance_parameter_value    isp_warp            NUM_ENGINES               ${v_num_warp_engines}
    set_instance_parameter_value    isp_warp            PIXELS_IN_PARALLEL        ${v_pip}
    set_instance_parameter_value    isp_warp            SINGLE_BOUNCE             ${v_warp_single_bounce}
    set_instance_parameter_value    isp_warp            EXT_MEM_DATA_WIDTH        256

    # isp_se_warp
    set_instance_parameter_value    isp_se_warp         BURSTCOUNT_WIDTH          {7}
    set_instance_parameter_value    isp_se_warp         DATA_WIDTH                {256}
    set_instance_parameter_value    isp_se_warp         ENABLE_SLAVE_PORT         {0}
    set_instance_parameter_value    isp_se_warp         MASTER_ADDRESS_DEF        {0}
    set_instance_parameter_value    isp_se_warp         MASTER_ADDRESS_WIDTH      {33}
    set_instance_parameter_value    isp_se_warp         MAX_PENDING_READS         {64}
    set_instance_parameter_value    isp_se_warp         SLAVE_ADDRESS_WIDTH       {26}
    set_instance_parameter_value    isp_se_warp         SUB_WINDOW_COUNT          {1}
    set_instance_parameter_value    isp_se_warp         SYNC_RESET                {0}

    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            # isp_alpha_vfr
            set_instance_parameter_value    isp_alpha_vfr       BPS                       ${v_bps}
            set_instance_parameter_value    isp_alpha_vfr       CLOCKS_ARE_SEPARATE       {1}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_ID_ASSOCIATED  {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_ID_COMPONENT   ${v_inst_id}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_IRQ            {255}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_IRQ_ENABLE     {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_IRQ_ENABLE_EN  {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_IRQ_STATUS     {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_IRQ_STATUS_EN  {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_TAG            {0}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_TYPE           {586}
            set_instance_parameter_value    isp_alpha_vfr       C_OMNI_CAP_VERSION        {1}
            set_instance_parameter_value    isp_alpha_vfr       ENABLE_DEBUG              ${v_enable_debug}
            set_instance_parameter_value    isp_alpha_vfr       EXTERNAL_MODE             {1}
            set_instance_parameter_value    isp_alpha_vfr       MAX_BUFFER_SETS           {2}
            set_instance_parameter_value    isp_alpha_vfr       MAX_HEIGHT                {2200}
            set_instance_parameter_value    isp_alpha_vfr       MAX_WIDTH                 {4400}
            set_instance_parameter_value    isp_alpha_vfr       NUMBER_OF_COLOR_PLANES    ${v_alpha_cp}
            set_instance_parameter_value    isp_alpha_vfr       PACKING                   {PERFECT}
            set_instance_parameter_value    isp_alpha_vfr       PIXELS_IN_PARALLEL        ${v_pip}
            set_instance_parameter_value    isp_alpha_vfr       P_AV_MM_ADDR_WIDTH        {32}
            set_instance_parameter_value    isp_alpha_vfr       P_AV_MM_DATA_WIDTH        {256}
            set_instance_parameter_value    isp_alpha_vfr       SEPARATE_SLAVE_CLOCK      {1}
            set_instance_parameter_value    isp_alpha_vfr       SLAVE_PROTOCOL            {Avalon}
            set_instance_parameter_value    isp_alpha_vfr       READ_BURST_TARGET         {8}
            set_instance_parameter_value    isp_alpha_vfr       READ_FIFO_DEPTH           {512}

            # isp_alpha_se_vfr
            set_instance_parameter_value    isp_alpha_se_vfr    BURSTCOUNT_WIDTH          {6}
            set_instance_parameter_value    isp_alpha_se_vfr    DATA_WIDTH                {256}
            set_instance_parameter_value    isp_alpha_se_vfr    ENABLE_SLAVE_PORT         {0}
            set_instance_parameter_value    isp_alpha_se_vfr    MASTER_ADDRESS_DEF        {0}
            set_instance_parameter_value    isp_alpha_se_vfr    MASTER_ADDRESS_WIDTH      {33}
            set_instance_parameter_value    isp_alpha_se_vfr    MAX_PENDING_READS         {8}
            set_instance_parameter_value    isp_alpha_se_vfr    SLAVE_ADDRESS_WIDTH       {26}
            set_instance_parameter_value    isp_alpha_se_vfr    SUB_WINDOW_COUNT          {1}
            set_instance_parameter_value    isp_alpha_se_vfr    SYNC_RESET                {0}

            if {${v_scale_up_alpha_en}} {
                # isp_alpha_up_scaler
                set_instance_parameter_value    isp_alpha_up_scaler   ALGORITHM                 {NEAREST_NEIGHBOUR}
                set_instance_parameter_value    isp_alpha_up_scaler   BPS                       ${v_bps}
                set_instance_parameter_value    isp_alpha_up_scaler   NUMBER_OF_COLOR_PLANES    ${v_alpha_cp}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_ID_ASSOCIATED  {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_ID_COMPONENT   [expr ${v_inst_id} + 2]
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_IRQ            {255}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_IRQ_ENABLE     {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_IRQ_ENABLE_EN  {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_IRQ_STATUS     {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_IRQ_STATUS_EN  {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_TAG            {0}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_TYPE           {564}
                set_instance_parameter_value    isp_alpha_up_scaler   C_OMNI_CAP_VERSION        {1}
                set_instance_parameter_value    isp_alpha_up_scaler   EDGE_MIRROR               {REPLICATE}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_420                {0}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_420_MIRROR         {0}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_422                {0}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_444                {1}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_DEBUG              ${v_enable_debug}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_H                  {1}
                set_instance_parameter_value    isp_alpha_up_scaler   ENABLE_V                  {1}
                set_instance_parameter_value    isp_alpha_up_scaler   EXTERNAL_MODE             {1}
                set_instance_parameter_value    isp_alpha_up_scaler   HALF_RATE_420             {0}
                set_instance_parameter_value    isp_alpha_up_scaler   H_BANKS                   {1}
                set_instance_parameter_value    isp_alpha_up_scaler   H_COEFF_FRAC_BITS         {8}
                set_instance_parameter_value    isp_alpha_up_scaler   H_COEFF_FUNCTION          {LANCZOS_2}
                set_instance_parameter_value    isp_alpha_up_scaler   H_COEFF_INT_BITS          {1}
                set_instance_parameter_value    isp_alpha_up_scaler   H_COEFF_SIGNED            {1}
                set_instance_parameter_value    isp_alpha_up_scaler   H_INIT_FILE \
                                                                      {<enter file name (including full path)>}
                set_instance_parameter_value    isp_alpha_up_scaler   H_PARTIAL_SCALING         {0}
                set_instance_parameter_value    isp_alpha_up_scaler   H_PHASES                  {4}
                set_instance_parameter_value    isp_alpha_up_scaler   H_TAPS                    {4}
                set_instance_parameter_value    isp_alpha_up_scaler   MAX_IN_WIDTH              {1920}
                set_instance_parameter_value    isp_alpha_up_scaler   MAX_OUT_WIDTH             {3840}
                set_instance_parameter_value    isp_alpha_up_scaler   MEM_INIT                  {1}
                set_instance_parameter_value    isp_alpha_up_scaler   NO_BLANKING               {0}
                set_instance_parameter_value    isp_alpha_up_scaler   OUTPUT_HEIGHT             {2160}
                set_instance_parameter_value    isp_alpha_up_scaler   PIPELINE_READY            ${v_pipeline_ready}
                set_instance_parameter_value    isp_alpha_up_scaler   PIXELS_IN_PARALLEL        ${v_pip}
                set_instance_parameter_value    isp_alpha_up_scaler   P_CORE_CTRL_ID            {0}
                set_instance_parameter_value    isp_alpha_up_scaler   P_UPDATE_CMD_SUPPORTED    {0}
                set_instance_parameter_value    isp_alpha_up_scaler   RUNTIME_CONTROL           {1}
                set_instance_parameter_value    isp_alpha_up_scaler   RUNTIME_LOAD              {0}
                set_instance_parameter_value    isp_alpha_up_scaler   SEPARATE_SLAVE_CLOCK      {1}
                set_instance_parameter_value    isp_alpha_up_scaler   SLAVE_PROTOCOL            {Avalon}
                set_instance_parameter_value    isp_alpha_up_scaler   V_BANKS                   {1}
                set_instance_parameter_value    isp_alpha_up_scaler   V_COEFF_FRAC_BITS         {8}
                set_instance_parameter_value    isp_alpha_up_scaler   V_COEFF_FUNCTION          {LANCZOS_2}
                set_instance_parameter_value    isp_alpha_up_scaler   V_COEFF_INT_BITS          {1}
                set_instance_parameter_value    isp_alpha_up_scaler   V_COEFF_SIGNED            {1}
                set_instance_parameter_value    isp_alpha_up_scaler   V_INIT_FILE \
                                                                      {<enter file name (including full path)>}
                set_instance_parameter_value    isp_alpha_up_scaler   V_PARTIAL_SCALING         {0}
                set_instance_parameter_value    isp_alpha_up_scaler   V_PHASES                  {4}
                set_instance_parameter_value    isp_alpha_up_scaler   V_PRES_FRAC_BITS          {0}
                set_instance_parameter_value    isp_alpha_up_scaler   V_TAPS                    {4}
            }
            # isp_alpha_cpm
            set_instance_parameter_value    isp_alpha_cpm       BPS                           ${v_bps}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_ID_ASSOCIATED      {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_IRQ                {255}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_IRQ_ENABLE         {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_IRQ_ENABLE_EN      {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_IRQ_STATUS         {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_IRQ_STATUS_EN      {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_TAG                {0}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_TYPE               {571}
            set_instance_parameter_value    isp_alpha_cpm       C_OMNI_CAP_VERSION            {1}
            set_instance_parameter_value    isp_alpha_cpm       ENABLE_DEBUG                  ${v_enable_debug}
            set_instance_parameter_value    isp_alpha_cpm       EXTERNAL_MODE                 {1}
            set_instance_parameter_value    isp_alpha_cpm       IP0_KEEP_COLOR_PLANE_0        {1}
            set_instance_parameter_value    isp_alpha_cpm       IP0_KEEP_COLOR_PLANE_1        {1}
            set_instance_parameter_value    isp_alpha_cpm       IP0_KEEP_COLOR_PLANE_2        {1}
            set_instance_parameter_value    isp_alpha_cpm       IP0_KEEP_COLOR_PLANE_3        {0}
            set_instance_parameter_value    isp_alpha_cpm       IP1_KEEP_COLOR_PLANE_0        {1}
            set_instance_parameter_value    isp_alpha_cpm       IP1_KEEP_COLOR_PLANE_1        {0}
            set_instance_parameter_value    isp_alpha_cpm       IP1_KEEP_COLOR_PLANE_2        {0}
            set_instance_parameter_value    isp_alpha_cpm       IP1_KEEP_COLOR_PLANE_3        {0}
            set_instance_parameter_value    isp_alpha_cpm       KEEP_IP1_AUX_PACKETS          {1}
            set_instance_parameter_value    isp_alpha_cpm       KEEP_OP1_AUX_PACKETS          {1}
            set_instance_parameter_value    isp_alpha_cpm       MAPPING_COLOR_PLANE_0         {0}
            set_instance_parameter_value    isp_alpha_cpm       MAPPING_COLOR_PLANE_1         {0}
            set_instance_parameter_value    isp_alpha_cpm       MAPPING_COLOR_PLANE_2         {0}
            set_instance_parameter_value    isp_alpha_cpm       MAPPING_COLOR_PLANE_3         {0}
            set_instance_parameter_value    isp_alpha_cpm       MODE                          {MERGE}
            set_instance_parameter_value    isp_alpha_cpm       NUMBER_OF_COLOR_PLANES_IN0    ${v_cppp}
            set_instance_parameter_value    isp_alpha_cpm       NUMBER_OF_COLOR_PLANES_IN1    ${v_alpha_cp}
            set_instance_parameter_value    isp_alpha_cpm       NUMBER_OF_COLOR_PLANES_OUT0   ${v_cppp_with_alpha}
            set_instance_parameter_value    isp_alpha_cpm       NUMBER_OF_COLOR_PLANES_OUT1   {2}
            set_instance_parameter_value    isp_alpha_cpm       OP0_KEEP_COLOR_PLANE_0        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP0_KEEP_COLOR_PLANE_1        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP0_KEEP_COLOR_PLANE_2        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP0_KEEP_COLOR_PLANE_3        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP1_KEEP_COLOR_PLANE_0        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP1_KEEP_COLOR_PLANE_1        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP1_KEEP_COLOR_PLANE_2        {1}
            set_instance_parameter_value    isp_alpha_cpm       OP1_KEEP_COLOR_PLANE_3        {1}
            set_instance_parameter_value    isp_alpha_cpm       PADDING_COLOR_PLANE_0         {0}
            set_instance_parameter_value    isp_alpha_cpm       PADDING_COLOR_PLANE_1         {0}
            set_instance_parameter_value    isp_alpha_cpm       PADDING_COLOR_PLANE_2         {0}
            set_instance_parameter_value    isp_alpha_cpm       PADDING_COLOR_PLANE_3         {0}
            set_instance_parameter_value    isp_alpha_cpm       PIXELS_IN_PARALLEL            ${v_pip}
            set_instance_parameter_value    isp_alpha_cpm       P_CORE_CTRL_ID                {0}
            set_instance_parameter_value    isp_alpha_cpm       P_UPDATE_CMD_SUPPORTED        {0}
            set_instance_parameter_value    isp_alpha_cpm       RUNTIME_CONTROL               {0}
            set_instance_parameter_value    isp_alpha_cpm       SEPARATE_SLAVE_CLOCK          {0}
            set_instance_parameter_value    isp_alpha_cpm       SLAVE_PROTOCOL                {Avalon}
        } else {
            # isp_alpha_channel
            set_instance_parameter_value      isp_alpha_channel        BPS                           ${v_bps}
            set_instance_parameter_value      isp_alpha_channel        C_CPU_OFFSET                  {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_ID_ASSOCIATED      {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_IRQ                {255}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_IRQ_ENABLE         {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_IRQ_ENABLE_EN      {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_IRQ_STATUS         {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_IRQ_STATUS_EN      {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_SIZE               {64}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_TAG                {0}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_TYPE               {599}
            set_instance_parameter_value      isp_alpha_channel        C_OMNI_CAP_VERSION            {1}
            set_instance_parameter_value      isp_alpha_channel        NUMBER_OF_COLOR_PLANES        ${v_cppp}
            set_instance_parameter_value      isp_alpha_channel        PIXELS_IN_PARALLEL            ${v_pip}
            set_instance_parameter_value      isp_alpha_channel        RUNTIME_CONTROL               {1}
        }
    }


    ############################
    #### Create Connections ####
    ############################

    # isp_cpu_clk
    add_connection      isp_cpu_clk.out_clk        isp_cpu_rst.clk
    add_connection      isp_cpu_clk.out_clk        isp_mm_bridge.clk
    if {${v_bls_en}} {
        add_connection      isp_cpu_clk.out_clk        isp_bls.agent_clock
    }
    if {${v_clipper_en}} {
        add_connection      isp_cpu_clk.out_clk        isp_clipper.agent_clock
    }
    add_connection      isp_cpu_clk.out_clk        isp_dpc.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_anr.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_blc.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_vc.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_switch_wbs.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_wbc.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_wbs.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_dms.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_hs.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_ccm.agent_clock
    add_connection      isp_cpu_clk.out_clk        isp_roi.agent_clock
    if {${v_ext_scaler_en}} {
        add_connection      isp_cpu_clk.out_clk         isp_h_scaler_down.agent_clock
        add_connection      isp_cpu_clk.out_clk         isp_v_scaler_down.agent_clock
    }
    add_connection      isp_cpu_clk.out_clk        isp_warp.av_mm_control_agent_clock
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_cpu_clk.out_clk        isp_alpha_vfr.control_clock
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_cpu_clk.out_clk        isp_alpha_up_scaler.agent_clock
            }
        } else {
            add_connection          isp_cpu_clk.out_clk     isp_alpha_channel.agent_clock
        }
    }

    # isp_cpu_rst
    add_connection      isp_cpu_rst.out_reset      isp_mm_bridge.reset
    if {${v_bls_en}} {
        add_connection      isp_cpu_rst.out_reset      isp_bls.agent_reset
    }
    if {${v_clipper_en}} {
        add_connection      isp_cpu_rst.out_reset      isp_clipper.agent_reset
    }
    add_connection      isp_cpu_rst.out_reset      isp_dpc.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_anr.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_blc.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_vc.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_switch_wbs.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_wbc.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_wbs.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_dms.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_hs.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_ccm.agent_reset
    add_connection      isp_cpu_rst.out_reset      isp_roi.agent_reset
    if {${v_ext_scaler_en}} {
        add_connection      isp_cpu_rst.out_reset     isp_h_scaler_down.agent_reset
        add_connection      isp_cpu_rst.out_reset     isp_v_scaler_down.agent_reset
    }
    add_connection      isp_cpu_rst.out_reset      isp_warp.av_mm_control_agent_reset
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_cpu_rst.out_reset      isp_alpha_vfr.control_reset
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_cpu_rst.out_reset      isp_alpha_up_scaler.agent_reset
            }
        } else {
            add_connection          isp_cpu_rst.out_reset      isp_alpha_channel.agent_reset
        }
    }

    # isp_mm_bridge
    if {${v_bls_en}} {
        add_connection      isp_mm_bridge.m0                  isp_bls.av_mm_control_agent
    }
    if {${v_clipper_en}} {
        add_connection      isp_mm_bridge.m0                  isp_clipper.av_mm_control_agent
    }
    add_connection      isp_mm_bridge.m0                  isp_dpc.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_anr.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_blc.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_vc.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_switch_wbs.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_wbc.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_wbs.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_dms.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_hs.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_ccm.av_mm_control_agent
    add_connection      isp_mm_bridge.m0                  isp_roi.av_mm_control_agent
    if {${v_ext_scaler_en}} {
        add_connection      isp_mm_bridge.m0                  isp_h_scaler_down.av_mm_control_agent
        add_connection      isp_mm_bridge.m0                  isp_v_scaler_down.av_mm_control_agent
    }
    add_connection      isp_mm_bridge.m0                  isp_warp.av_mm_control_agent
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_mm_bridge.m0                  isp_alpha_vfr.av_mm_control_agent
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_mm_bridge.m0                  isp_alpha_up_scaler.av_mm_control_agent
            }
        } else {
            add_connection          isp_mm_bridge.m0                  isp_alpha_channel.av_mm_control_agent
        }
    }

    # isp_vid_clk
    add_connection      isp_vid_clk.out_clk        isp_vid_rst.clk
    if {${v_bls_en}} {
        add_connection      isp_vid_clk.out_clk        isp_bls.main_clock
    }
    if {${v_clipper_en}} {
        add_connection      isp_vid_clk.out_clk        isp_clipper.main_clock
    }
    add_connection      isp_vid_clk.out_clk        isp_dpc.main_clock
    add_connection      isp_vid_clk.out_clk        isp_anr.main_clock
    add_connection      isp_vid_clk.out_clk        isp_blc.main_clock
    add_connection      isp_vid_clk.out_clk        isp_vc.main_clock
    add_connection      isp_vid_clk.out_clk        isp_switch_wbs.main_clock
    add_connection      isp_vid_clk.out_clk        isp_wbc.main_clock
    add_connection      isp_vid_clk.out_clk        isp_wbs.main_clock
    add_connection      isp_vid_clk.out_clk        isp_dms.main_clock
    add_connection      isp_vid_clk.out_clk        isp_hs.main_clock
    add_connection      isp_vid_clk.out_clk        isp_ccm.main_clock
    add_connection      isp_vid_clk.out_clk        isp_pix_adapt.main_clock
    add_connection      isp_vid_clk.out_clk        isp_roi.main_clock
    if {${v_ext_scaler_en}} {
        add_connection      isp_vid_clk.out_clk         isp_h_scaler_down.main_clock
        add_connection      isp_vid_clk.out_clk         isp_v_scaler_down.main_clock
    }
    add_connection      isp_vid_clk.out_clk        isp_warp.core_clock
    add_connection      isp_vid_clk.out_clk        isp_warp.axi4s_vid_in_0_clock
    add_connection      isp_vid_clk.out_clk        isp_warp.axi4s_vid_out_0_clock
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_vid_clk.out_clk        isp_alpha_vfr.main_clock
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_vid_clk.out_clk        isp_alpha_up_scaler.main_clock
            }
            add_connection      isp_vid_clk.out_clk        isp_alpha_cpm.main_clock
        } else {
            add_connection      isp_vid_clk.out_clk         isp_alpha_channel.main_clock
        }
    }

    # isp_vid_rst
    if {${v_bls_en}} {
        add_connection      isp_vid_rst.out_reset      isp_bls.main_reset
    }
    if {${v_clipper_en}} {
        add_connection      isp_vid_rst.out_reset      isp_clipper.main_reset
    }
    add_connection      isp_vid_rst.out_reset      isp_dpc.main_reset
    add_connection      isp_vid_rst.out_reset      isp_anr.main_reset
    add_connection      isp_vid_rst.out_reset      isp_blc.main_reset
    add_connection      isp_vid_rst.out_reset      isp_vc.main_reset
    add_connection      isp_vid_rst.out_reset      isp_switch_wbs.main_reset
    add_connection      isp_vid_rst.out_reset      isp_wbc.main_reset
    add_connection      isp_vid_rst.out_reset      isp_wbs.main_reset
    add_connection      isp_vid_rst.out_reset      isp_dms.main_reset
    add_connection      isp_vid_rst.out_reset      isp_hs.main_reset
    add_connection      isp_vid_rst.out_reset      isp_ccm.main_reset
    add_connection      isp_vid_rst.out_reset      isp_pix_adapt.main_reset
    add_connection      isp_vid_rst.out_reset      isp_roi.main_reset
    if {${v_ext_scaler_en}} {
        add_connection      isp_vid_rst.out_reset         isp_h_scaler_down.main_reset
        add_connection      isp_vid_rst.out_reset         isp_v_scaler_down.main_reset
    }
    add_connection      isp_vid_rst.out_reset      isp_warp.core_reset
    add_connection      isp_vid_rst.out_reset      isp_warp.axi4s_vid_in_0_reset
    add_connection      isp_vid_rst.out_reset      isp_warp.axi4s_vid_out_0_reset
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_vid_rst.out_reset      isp_alpha_vfr.main_reset
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_vid_rst.out_reset      isp_alpha_up_scaler.main_reset
            }
            add_connection      isp_vid_rst.out_reset      isp_alpha_cpm.main_reset
        } else {
            add_connection      isp_vid_rst.out_reset         isp_alpha_channel.main_reset
        }
    }

    # isp_emif_clk
    add_connection      isp_emif_clk.out_clk       isp_emif_rst.clk
    add_connection      isp_emif_clk.out_clk       isp_warp.av_mm_memory_host_clock
    add_connection      isp_emif_clk.out_clk       isp_se_warp.clock
    if {${v_alpha_en} && ${v_vfr_alpha_en}} {
        add_connection      isp_emif_clk.out_clk       isp_alpha_vfr.mem_clock
        add_connection      isp_emif_clk.out_clk       isp_alpha_se_vfr.clock
    }

    # isp_emif_rst
    add_connection      isp_emif_rst.out_reset     isp_warp.av_mm_memory_host_reset
    add_connection      isp_emif_rst.out_reset     isp_se_warp.reset
    if {${v_alpha_en} && ${v_vfr_alpha_en}} {
        add_connection      isp_emif_rst.out_reset     isp_alpha_vfr.mem_reset
        add_connection      isp_emif_rst.out_reset     isp_alpha_se_vfr.reset
    }

    if {${v_clipper_en}} {
        if {${v_bls_en}} {
            # isp_bls
            add_connection      isp_bls.axi4s_vid_out         isp_clipper.axi4s_vid_in
        }

        # isp_clipper
        add_connection      isp_clipper.axi4s_vid_out     isp_dpc.axi4s_vid_in
    } elseif {${v_bls_en}} {
        # isp_bls
        add_connection      isp_bls.axi4s_vid_out         isp_dpc.axi4s_vid_in
    }

    # isp_dpc
    add_connection      isp_dpc.axi4s_vid_out         isp_anr.axi4s_vid_in

    # isp_anr
    add_connection      isp_anr.axi4s_vid_out         isp_blc.axi4s_vid_in

    # isp_blc
    add_connection      isp_blc.axi4s_vid_out         isp_vc.axi4s_vid_in

    # isp_vc
    add_connection      isp_vc.axi4s_vid_out          isp_switch_wbs.axi4s_vid_in_0

    # isp_switch_wbs
    add_connection      isp_switch_wbs.axi4s_vid_out_0        isp_wbc.axi4s_vid_in
    add_connection      isp_switch_wbs.axi4s_vid_out_1        isp_wbs.axi4s_vid_in
    add_connection      isp_switch_wbs.axi4s_vid_out_2        isp_dms.axi4s_vid_in

    # isp_wbc
    add_connection      isp_wbc.axi4s_vid_out         isp_switch_wbs.axi4s_vid_in_1

    # isp_wbs
    add_connection      isp_wbs.axi4s_vid_out         isp_switch_wbs.axi4s_vid_in_2

    # isp_dms
    add_connection      isp_dms.axi4s_vid_out         isp_hs.axi4s_vid_in

    # isp_hs
    add_connection    isp_hs.axi4s_vid_out         isp_ccm.axi4s_vid_in

    # isp_ccm
    add_connection    isp_ccm.axi4s_vid_out        isp_roi.axi4s_vid_in

    if {${v_ext_scaler_en}} {
        # isp_roi
        add_connection    isp_roi.axi4s_vid_out         isp_h_scaler_down.axi4s_vid_in

        # isp_h_scaler_down
        add_connection          isp_h_scaler_down.axi4s_vid_out      isp_v_scaler_down.axi4s_vid_in

        # isp_v_scaler_down
        add_connection          isp_v_scaler_down.axi4s_vid_out      isp_pix_adapt.axi4s_vid_in

        # isp_pix_adapt
        add_connection    isp_pix_adapt.axi4s_vid_out      isp_warp.axi4s_vid_in_0
    } else {
        # isp_roi
        add_connection    isp_roi.axi4s_vid_out         isp_pix_adapt.axi4s_vid_in

        # isp_pix_adapt
        add_connection    isp_pix_adapt.axi4s_vid_out      isp_warp.axi4s_vid_in_0
    }

    # isp_warp
    add_connection      isp_warp.av_mm_memory_host    isp_se_warp.windowed_slave

    # isp_alpha_vfr
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            add_connection      isp_alpha_vfr.av_mm_mem_read_host       isp_alpha_se_vfr.windowed_slave
            if {${v_scale_up_alpha_en}} {
                add_connection      isp_alpha_vfr.axi4s_vid_out             isp_alpha_up_scaler.axi4s_vid_in

                # isp_alpha_up_scaler
                add_connection      isp_alpha_up_scaler.axi4s_vid_out       isp_alpha_cpm.axi4s_vid_in_1
            } else {
                add_connection      isp_alpha_vfr.axi4s_vid_out             isp_alpha_cpm.axi4s_vid_in_1
            }
            # isp_warp
            add_connection      isp_warp.axi4s_vid_out_0            isp_alpha_cpm.axi4s_vid_in_0
        } else {
            # isp_warp
            add_connection      isp_warp.axi4s_vid_out_0            isp_alpha_channel.axi4s_vid_in
        }
    }


    ##########################
    ##### Create Exports #####
    ##########################

    # isp_cpu_clk
    add_interface           cpu_clk_in    clock       sink
    set_interface_property  cpu_clk_in    EXPORT_OF   isp_cpu_clk.in_clk

    # cpu_rst_bridge
    add_interface           cpu_rst_in    reset       sink
    set_interface_property  cpu_rst_in    EXPORT_OF   isp_cpu_rst.in_reset

    # isp_mm_bridge
    add_interface           mm_ctrl_in    avalon      slave
    set_interface_property  mm_ctrl_in    EXPORT_OF   isp_mm_bridge.s0

    # isp_vid_clk
    set_interface_property  vid_clk_in    EXPORT_OF   isp_vid_clk.in_clk

    # isp_vid_rst
    set_interface_property  vid_rst_in    EXPORT_OF   isp_vid_rst.in_reset

    # isp_emif_clk
    set_interface_property  emif_clk_in   EXPORT_OF   isp_emif_clk.in_clk

    # isp_emif_rst
    set_interface_property  emif_rst_in   EXPORT_OF   isp_emif_rst.in_reset

    add_interface           isp_in_s_vid_axis    axi4stream  subordinate
    if {${v_bls_en}} {
        # isp_bls
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_bls.axi4s_vid_in
    } elseif {${v_clipper_en}} {
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_clipper.axi4s_vid_in
    } else {
        set_interface_property  isp_in_s_vid_axis    EXPORT_OF   isp_dpc.axi4s_vid_in
    }

    # isp_se_warp
    set_interface_property  av_mm_host_warp    EXPORT_OF   isp_se_warp.expanded_master
    set_interface_property  warp_int           EXPORT_OF   isp_warp.interrupt

    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            # isp_alpha_se_vfr
            set_interface_property  av_mm_host_vfr      EXPORT_OF   isp_alpha_se_vfr.expanded_master
            set_interface_property  vfr_int             EXPORT_OF   isp_alpha_vfr.frame_reader_int

            # isp_alpha_cpm
            add_interface           isp_out_s_vid_axis    axi4stream  subordinate
            set_interface_property  isp_out_s_vid_axis    EXPORT_OF   isp_alpha_cpm.axi4s_vid_out
        } else {
            # isp_alpha_channel
            add_interface           isp_out_s_vid_axis    axi4stream  subordinate
            set_interface_property  isp_out_s_vid_axis    EXPORT_OF   isp_alpha_channel.axi4s_vid_out
        }
    } else {
        # isp warp
        add_interface           isp_out_s_vid_axis    axi4stream  subordinate
        set_interface_property  isp_out_s_vid_axis    EXPORT_OF   isp_warp.axi4s_vid_out_0
    }


    #################################
    ##### Assign Base Addresses #####
    #################################
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
    set v_roi_addr_offset                 [get_shell_parameter ROI_ADDR_OFFSET]
    set v_hscaler_addr_offset             [get_shell_parameter HSCALER_ADDR_OFFSET]
    set v_vscaler_addr_offset             [get_shell_parameter VSCALER_ADDR_OFFSET]
    set v_warp_addr_offset                [get_shell_parameter WARP_ADDR_OFFSET]
    set v_vfr_addr_offset                 [get_shell_parameter VFR_ADDR_OFFSET]
    set v_upscaler_addr_offset            [get_shell_parameter UPSCALER_ADDR_OFFSET]
    set v_alpha_ch_addr_offset            [get_shell_parameter ALPHA_CH_ADDR_OFFSET]

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
    set_connection_parameter_value isp_mm_bridge.m0/isp_roi.av_mm_control_agent \
                                                                          baseAddress ${v_roi_addr_offset}
    if {${v_ext_scaler_en}} {
        set_connection_parameter_value isp_mm_bridge.m0/isp_h_scaler_down.av_mm_control_agent \
                                                                          baseAddress ${v_hscaler_addr_offset}
        set_connection_parameter_value isp_mm_bridge.m0/isp_v_scaler_down.av_mm_control_agent \
                                                                          baseAddress ${v_vscaler_addr_offset}
    }
    set_connection_parameter_value isp_mm_bridge.m0/isp_warp.av_mm_control_agent \
                                                                          baseAddress ${v_warp_addr_offset}
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            set_connection_parameter_value isp_mm_bridge.m0/isp_alpha_vfr.av_mm_control_agent \
                                                                          baseAddress ${v_vfr_addr_offset}
            if {${v_scale_up_alpha_en}} {
                set_connection_parameter_value isp_mm_bridge.m0/isp_alpha_up_scaler.av_mm_control_agent \
                                                                          baseAddress ${v_upscaler_addr_offset}
            }
        } else {
            set_connection_parameter_value isp_mm_bridge.m0/isp_alpha_channel.av_mm_control_agent \
                                                                          baseAddress ${v_alpha_ch_addr_offset}
        }
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
    lock_avalon_base_address  isp_roi.av_mm_control_agent
    if {${v_ext_scaler_en}} {
        lock_avalon_base_address  isp_h_scaler_down.av_mm_control_agent
        lock_avalon_base_address  isp_v_scaler_down.av_mm_control_agent
    }
    lock_avalon_base_address  isp_warp.av_mm_control_agent
    if {${v_alpha_en}} {
        if {${v_vfr_alpha_en}} {
            lock_avalon_base_address  isp_alpha_vfr.av_mm_control_agent
            if {${v_scale_up_alpha_en}} {
                lock_avalon_base_address  isp_alpha_up_scaler.av_mm_control_agent
            }
        } else {
            lock_avalon_base_address  isp_alpha_channel.av_mm_control_agent
        }
    }


    # Performance boost
    set_connection_parameter_value isp_warp.av_mm_memory_host/isp_se_warp.windowed_slave \
                                            qsys_mm.burstAdapterImplementation {PER_BURST_TYPE_CONVERTER}
    set_connection_parameter_value isp_warp.av_mm_memory_host/isp_se_warp.windowed_slave \
                                            qsys_mm.widthAdapterImplementation {OPTIMIZED_CONVERTER}
    set_domain_assignment isp_warp.av_mm_memory_host qsys_mm.burstAdapterImplementation PER_BURST_TYPE_CONVERTER
    set_domain_assignment isp_warp.av_mm_memory_host qsys_mm.widthAdapterImplementation OPTIMIZED_CONVERTER


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
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_vid_clk_freq            [get_shell_parameter VID_CLK_FREQ]
    set v_emif_agent              [get_shell_parameter EMIF_AGENT]
    set v_alpha_en                [get_shell_parameter ALPHA_EN]
    set v_vfr_alpha_en            [get_shell_parameter VFR_ALPHA_EN]
    set v_io_id                   [get_shell_parameter IO_ID]
    set v_avmm_host               [get_shell_parameter AVMM_HOST]

    add_auto_connection   ${v_instance_name}  cpu_clk_in          ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name}  cpu_rst_in          ${v_avmm_host_clk_freq}

    add_auto_connection   ${v_instance_name}  vid_clk_in          ${v_vid_clk_freq}
    add_auto_connection   ${v_instance_name}  vid_rst_in          ${v_vid_clk_freq}

    # DDR
    add_auto_connection   ${v_instance_name}  emif_clk_in         ${v_emif_agent}_user_clk
    add_auto_connection   ${v_instance_name}  emif_rst_in         ${v_emif_agent}_user_rst
    if {${v_alpha_en} && ${v_vfr_alpha_en}} {
        add_auto_connection   ${v_instance_name}  av_mm_host_vfr      ${v_emif_agent}_user_data
    }
    add_auto_connection   ${v_instance_name}  av_mm_host_warp     ${v_emif_agent}_user_data

    # from isp in
    add_auto_connection   ${v_instance_name}  isp_in_s_vid_axis   isp_in_${v_io_id}_vid_axis

    # to vid out
    add_auto_connection   ${v_instance_name}  isp_out_s_vid_axis  isp_out_${v_io_id}_vid_axis

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
