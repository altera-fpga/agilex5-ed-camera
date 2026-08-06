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

set_shell_parameter AVMM_HOST                       {{AUTO X}}
set_shell_parameter AVMM_HOST_CLK_FREQ              {200000000.0}

set_shell_parameter OCS_ENTRY_BASE                  ""

# IP AVMM Offset Addresses
set_shell_parameter TPG_ADDR_OFFSET                 "0x00000000"
set_shell_parameter SCALER_UP_ADDR_OFFSET           "0x00000200"
set_shell_parameter MIXER_ADDR_OFFSET               "0x00000400"
set_shell_parameter PIO_BOARD_ADDR_OFFSET           "0x00000800"
set_shell_parameter PIO_STATUS_ADDR_OFFSET          "0x00000810"
set_shell_parameter PIO_SUPP_FORMATS_ADDR_OFFSET    "0x00000820"
set_shell_parameter PIO_CURR_FORMAT_ADDR_OFFSET     "0x00000830"
set_shell_parameter PIO_FORMAT_OVRRIDE_ADDR_OFFSET  "0x00000840"
set_shell_parameter PROTO_CONV_ADDR_OFFSET          "0x00000a00"
set_shell_parameter VFR_ADDR_OFFSET                 "0x00000c00"
set_shell_parameter PIP_CONV_ADDR_OFFSET            "0x00001000"
set_shell_parameter VFW_ADDR_OFFSET                 "0x00001200"
set_shell_parameter LOGO_VFR_ADDR_OFFSET            "0x00001800"
set_shell_parameter DRM_ADAPTER_ADDR_OFFSET         "0x00001c00"
set_shell_parameter 1D_LUT_ADDR_OFFSET              "0x00002000"

# General Video Controls
set_shell_parameter PIP                             {2}
set_shell_parameter VID_OUT_RATE                    "p60"
set_shell_parameter VID_OUT_BPS                     {10}
set_shell_parameter VID_OUT_PIP                     {4}
set_shell_parameter EN_DEBUG                        {1}
set_shell_parameter PIP_CONV_FIFO_DEPTH             {512}

# Overlay
set_shell_parameter HPS_DMA_ENABLE                  {0}
set_shell_parameter HPS_DMA_NUM_OF_CHANNELS         {2}
set_shell_parameter OVERLAY_USE_MEM_COPY            {0}
set_shell_parameter LOGO_USE_MEM_COPY               {0}
set_shell_parameter LOGO_SOFT_IP_EN                 {0}
set_shell_parameter OVERLAY_DRM_IP_EN               {0}
set_shell_parameter OVERLAY_ARGB2222                {0}
set_shell_parameter OVERLAY_IRQ_PRIORITY            "X"
set_shell_parameter OVERLAY_IRQ_HOST                ""
set_shell_parameter LOGO_IRQ_PRIORITY               "X"
set_shell_parameter LOGO_IRQ_HOST                   ""

set_shell_parameter INST_ID                         {0}

set_shell_parameter OUTPUT_SNAPSHOT_EN              {0}
set_shell_parameter EMIF_AGENT                      {}
set_shell_parameter EMIF_AGENT_CLK_FREQ             {200000000.0}


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

    # overlay validation checks
    set v_hps_dma_enable            [get_shell_parameter HPS_DMA_ENABLE]
    set v_hps_dma_num_of_channels   [get_shell_parameter HPS_DMA_NUM_OF_CHANNELS]
    set v_overlay_use_mem_copy      [get_shell_parameter OVERLAY_USE_MEM_COPY]
    set v_logo_use_mem_copy         [get_shell_parameter LOGO_USE_MEM_COPY]
    set v_logo_soft_ip_en           [get_shell_parameter LOGO_SOFT_IP_EN]
    set v_overlay_drm_ip_en         [get_shell_parameter OVERLAY_DRM_IP_EN]

    # dma from hps ddr, if not, then hps mem copy to fabric ddr
    # 1 (logo) or 2 channels (logo and overlay). logo supports 4k ARGB8888. Overlay supports 1080p, an Upscaler, and
    # different ARGB formats. DRM format can be DRM Adapter IP or fixed format ARGB8888 (uses pixel adapter to get to
    # 10b) or ARGB2222 (uses hard coded logic to get to10b). Logo can changed out for the soft IP variant.
    if {${v_hps_dma_enable}} {
        if {${v_hps_dma_num_of_channels} == 0} {
            send_message ERROR "vid_out_create: DMA Enabled but number of channels not specified"
        }
        set_shell_parameter DRV_HPS_DMA_NUM_OF_CHANNELS   ${v_hps_dma_num_of_channels}
        set_shell_parameter DRV_OVERLAY_USE_MEM_COPY      {0}
        set_shell_parameter DRV_LOGO_USE_MEM_COPY         {0}
        set_shell_parameter DRV_LOGO_SOFT_IP_EN           {0}
        if {${v_hps_dma_num_of_channels} > 0} {
            set_shell_parameter DRV_LOGO_EN       {1}
        } else {
            set_shell_parameter DRV_LOGO_EN       {0}
        }
        if {${v_hps_dma_num_of_channels} > 1} {
            set_shell_parameter DRV_OVERLAY_EN          {1}
            set_shell_parameter DRV_OVERLAY_DRM_IP_EN   ${v_overlay_drm_ip_en}
        } else {
            set_shell_parameter DRV_OVERLAY_EN          {0}
            set_shell_parameter DRV_OVERLAY_DRM_IP_EN   {0}
        }
    } else {
        set_shell_parameter DRV_HPS_DMA_NUM_OF_CHANNELS   {0}
        set_shell_parameter DRV_LOGO_USE_MEM_COPY         ${v_logo_use_mem_copy}
        if {${v_logo_use_mem_copy}} {
            set_shell_parameter DRV_LOGO_EN           {1}
            set_shell_parameter DRV_LOGO_SOFT_IP_EN   {0}
        } else {
            set_shell_parameter DRV_LOGO_EN           {0}
            set_shell_parameter DRV_LOGO_SOFT_IP_EN   ${v_logo_soft_ip_en}
        }
        set_shell_parameter DRV_OVERLAY_USE_MEM_COPY      ${v_overlay_use_mem_copy}
        if {${v_overlay_use_mem_copy}} {
            set_shell_parameter DRV_OVERLAY_EN    {1}
            set_shell_parameter DRV_OVERLAY_DRM_IP_EN   ${v_overlay_drm_ip_en}
        } else {
            set_shell_parameter DRV_OVERLAY_EN    {0}
            set_shell_parameter DRV_OVERLAY_DRM_IP_EN   {0}
        }
    }

    # check if the emif agent has been configured
    set v_output_snapshot_en      [get_shell_parameter OUTPUT_SNAPSHOT_EN]
    set v_emif_agent              [get_shell_parameter EMIF_AGENT]

    if {(${v_output_snapshot_en} || ${v_overlay_use_mem_copy} || ${v_logo_use_mem_copy}) \
                                                        && [llength ${v_emif_agent}] == 0} {
        send_message ERROR "vid_fr4k_out_create: EMIF Agent not specified"
    }
  }

proc pre_creation_step {} {
    transfer_files
}

proc creation_step {} {
    create_vid_out_subsystem
}

proc post_creation_step {} {
    modify_manual_ocs_rom
    edit_top_level_qsys
    add_auto_connections
    edit_top_v_file
}


proc transfer_files {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_script_path               [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_drv_logo_soft_ip_en       [get_shell_parameter DRV_LOGO_SOFT_IP_EN]
    set v_drv_overlay_drm_ip_en     [get_shell_parameter DRV_OVERLAY_DRM_IP_EN]

    if {${v_drv_logo_soft_ip_en}} {
        # Copy Icon
        exec cp -rf ${v_script_path}/../../non_qpds_ip/intel_vvp_icon \
                                                      ${v_project_path}/non_qpds_ip/user/intel_vvp_icon
        file_copy   ${v_script_path}/../../non_qpds_ip/intel_vvp_icon.ipx \
                                                      ${v_project_path}/non_qpds_ip/user
    }

    if {${v_drv_overlay_drm_ip_en}} {
        # Copy DRM Adapter
        exec cp -rf ${v_script_path}/../../non_qpds_ip/altera_vvp_drm_adapter \
                                                      ${v_project_path}/non_qpds_ip/user/altera_vvp_drm_adapter
        file_copy   ${v_script_path}/../../non_qpds_ip/altera_vvp_drm_adapter.ipx \
                                                      ${v_project_path}/non_qpds_ip/user
    }
}


proc create_vid_out_subsystem {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_instance_name     [get_shell_parameter INSTANCE_NAME]

    # Vid Pipeline
    set v_cppp              {3}
    set v_bps               {10}
    set v_pip               [get_shell_parameter PIP]
    set v_vid_out_rate      [get_shell_parameter VID_OUT_RATE]
    set v_vid_out_bps       [get_shell_parameter VID_OUT_BPS]
    set v_board_name        [get_shell_parameter DEVKIT]

    if {${v_board_name} == "AGX_5E_MACNICA_Sulfur_Devkit"} {
        set v_vid_out_if_pip    {2}
    } else {
        set v_vid_out_if_pip    [get_shell_parameter VID_OUT_PIP]
    }

    # overlay video Pipelines
    set v_drv_hps_dma_num_of_channels   [get_shell_parameter DRV_HPS_DMA_NUM_OF_CHANNELS]
    set v_drv_logo_soft_ip_en           [get_shell_parameter DRV_LOGO_SOFT_IP_EN]

    # Overlay
    set v_overlay_en              [get_shell_parameter DRV_OVERLAY_EN]
    set v_drv_overlay_drm_ip_en   [get_shell_parameter DRV_OVERLAY_DRM_IP_EN]

    # Only if no DRM Adapter is hardcoded 2222 mode supported
    if {${v_drv_overlay_drm_ip_en}} {
        set v_overlay_argb2222        {0}
    } else {
        set v_overlay_argb2222        [get_shell_parameter OVERLAY_ARGB2222]
    }
    set v_overlay_bps             {8}
    set v_drv_overlay_use_mem_copy    [get_shell_parameter DRV_OVERLAY_USE_MEM_COPY]
    if {${v_drv_overlay_use_mem_copy}} {
        set v_overlay_burst_length    {8}
    } else {
        set v_overlay_burst_length    {2}
    }

    set v_overlay_max_h           {1080}
    set v_overlay_max_w           {1920}
    if {${v_overlay_argb2222}} {
        set v_overlay_cppp            {1}
    } else {
        set v_overlay_cppp            {4}
    }

    # Logo Overlay
    set v_logo_en                     [get_shell_parameter DRV_LOGO_EN]
    set v_drv_logo_use_mem_copy       [get_shell_parameter DRV_LOGO_USE_MEM_COPY]
    if {${v_drv_logo_use_mem_copy}} {
        set v_logo_burst_length    {8}
    } else {
        set v_logo_burst_length    {2}
    }
    set v_logo_bps                {8}
    set v_logo_cppp               {4}
    set v_logo_max_h              {2160}
    set v_logo_max_w              {3840}

    set v_output_snapshot_en      [get_shell_parameter OUTPUT_SNAPSHOT_EN]
    set v_emif_agent_clk_freq     [get_shell_parameter EMIF_AGENT_CLK_FREQ]

    set v_mixer_channels          [expr 2 + ${v_drv_hps_dma_num_of_channels} + ${v_drv_overlay_use_mem_copy} + \
                                                        ${v_drv_logo_use_mem_copy} + ${v_drv_logo_soft_ip_en}]

    # General
    set v_inst_id                 [get_shell_parameter INST_ID]
    set v_enable_debug            [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready          {1}

    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]

    set v_pip_conv_fifo_depth     [get_shell_parameter PIP_CONV_FIFO_DEPTH]

    # If DP and 25.1, then we are using Multi-rate DP
    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
        set_shell_parameter DUAL_CLOCK_EN {0}
    } else {
        set_shell_parameter DUAL_CLOCK_EN {1}
    }
    set v_dual_clock_en   [get_shell_parameter DUAL_CLOCK_EN]


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  vid_out_cpu_clk_bridge        altera_clock_bridge
    add_instance  vid_out_cpu_rst_bridge        altera_reset_bridge
    add_instance  vid_out_mm_bridge             altera_avalon_mm_bridge
    add_instance  vid_out_vid_clk_bridge        altera_clock_bridge
    add_instance  vid_out_vid_rst_bridge        altera_reset_bridge
    add_instance  vid_out_tpg                   intel_vvp_tpg
    if {${v_drv_overlay_use_mem_copy}} {
        add_instance  vid_out_se_vfr            altera_address_span_extender
    }
    if {${v_overlay_en}} {
        add_instance  vid_out_vfr               intel_vvp_vfr
        add_instance  vid_out_scaler_up         intel_vvp_scaler
        if {${v_drv_overlay_drm_ip_en}} {
            add_instance  vid_out_drm_adapter       altera_vvp_drm_adapter
        } elseif {${v_overlay_argb2222} == 0} {
            # pixel adapter used for 8b to 10b conversion
            add_instance  vid_out_pixel_adapter     intel_vvp_pixel_adapter
        }
    }
    if {${v_drv_logo_use_mem_copy}} {
        add_instance  vid_out_se_logo_vfr      altera_address_span_extender
    }
    if {${v_logo_en}} {
        add_instance  vid_out_logo_vfr          intel_vvp_vfr
        add_instance  vid_out_logo_pix_adapt    intel_vvp_pixel_adapter
    }
    if {${v_drv_logo_soft_ip_en}} {
        add_instance  vid_out_icon              intel_vvp_icon
    }
    add_instance  vid_out_mixer                 intel_vvp_mixer
    add_instance  vid_out_1d_lut                intel_vvp_1d_lut

    if {${v_output_snapshot_en} || ${v_drv_overlay_use_mem_copy} || ${v_drv_logo_use_mem_copy}} {
        add_instance  vid_out_emif_clk_bridge       altera_clock_bridge
        add_instance  vid_out_emif_rst_bridge       altera_reset_bridge
    }
    if {${v_output_snapshot_en}} {
        add_instance  vid_out_axi4s_bcast           intel_vvp_axi4s_broadcaster
        add_instance  vid_out_vfw                   intel_vvp_vfw
        add_instance  vid_out_se_vfw                altera_address_span_extender
    }
    if {${v_dual_clock_en} == 1} {
      add_instance  vid_out_if_clk_bridge       altera_clock_bridge
      add_instance  vid_out_if_rst_bridge       altera_reset_bridge
      if {${v_pip} != ${v_vid_out_if_pip}} {
          add_instance  vid_out_pip_conv              intel_vvp_pip_conv
      } else {
          add_instance  vid_out_fifo                  intel_vvp_fifo
      }
    } elseif {${v_pip} != ${v_vid_out_if_pip}} {
        add_instance  vid_out_pip_conv              intel_vvp_pip_conv
    }
    add_instance  vid_out_proto_conv            intel_vvp_protocol_conv
    add_instance  vid_out_pio_board             altera_avalon_pio
    add_instance  vid_out_pio_status            altera_avalon_pio
    add_instance  vid_out_pio_supportd_fmats    altera_avalon_pio
    add_instance  vid_out_pio_curr_format       altera_avalon_pio
    add_instance  vid_out_pio_format_ovrride    altera_avalon_pio


    ############################
    #### Set Parameters     ####
    ############################

    # vid_out_cpu_clk_bridge
    set_instance_parameter_value      vid_out_cpu_clk_bridge     EXPLICIT_CLOCK_RATE       ${v_avmm_host_clk_freq}
    set_instance_parameter_value      vid_out_cpu_clk_bridge     NUM_CLOCK_OUTPUTS         {1}

    # vid_out_cpu_rst_bridge
    set_instance_parameter_value      vid_out_cpu_rst_bridge     ACTIVE_LOW_RESET              {0}
    set_instance_parameter_value      vid_out_cpu_rst_bridge     NUM_RESET_OUTPUTS             {1}
    set_instance_parameter_value      vid_out_cpu_rst_bridge     SYNCHRONOUS_EDGES             {deassert}
    set_instance_parameter_value      vid_out_cpu_rst_bridge     SYNC_RESET                    {0}
    set_instance_parameter_value      vid_out_cpu_rst_bridge     USE_RESET_REQUEST             {0}

    # vid_out_mm_bridge
    set_instance_parameter_value      vid_out_mm_bridge        ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value      vid_out_mm_bridge        ADDRESS_WIDTH                 {0}
    set_instance_parameter_value      vid_out_mm_bridge        DATA_WIDTH                    {32}
    set_instance_parameter_value      vid_out_mm_bridge        LINEWRAPBURSTS                {0}
    set_instance_parameter_value      vid_out_mm_bridge        M0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      vid_out_mm_bridge        MAX_BURST_SIZE                {1}
    set_instance_parameter_value      vid_out_mm_bridge        MAX_PENDING_RESPONSES         {4}
    set_instance_parameter_value      vid_out_mm_bridge        MAX_PENDING_WRITES            {0}
    set_instance_parameter_value      vid_out_mm_bridge        PIPELINE_COMMAND              {1}
    set_instance_parameter_value      vid_out_mm_bridge        PIPELINE_RESPONSE             {1}
    set_instance_parameter_value      vid_out_mm_bridge        S0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value      vid_out_mm_bridge        SYMBOL_WIDTH                  {8}
    set_instance_parameter_value      vid_out_mm_bridge        SYNC_RESET                    {0}
    set_instance_parameter_value      vid_out_mm_bridge        USE_AUTO_ADDRESS_WIDTH        {1}
    set_instance_parameter_value      vid_out_mm_bridge        USE_RESPONSE                  {0}
    set_instance_parameter_value      vid_out_mm_bridge        USE_WRITERESPONSE             {0}

    # vid_out_vid_clk_bridge
    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
      set_instance_parameter_value      vid_out_vid_clk_bridge     EXPLICIT_CLOCK_RATE     {297000000.0}
    } else {
      set_instance_parameter_value      vid_out_vid_clk_bridge     EXPLICIT_CLOCK_RATE     {148500000.0}
    }
    set_instance_parameter_value      vid_out_vid_clk_bridge       NUM_CLOCK_OUTPUTS       {1}

    # vid_out_vid_rst_bridge
    set_instance_parameter_value      vid_out_vid_rst_bridge     ACTIVE_LOW_RESET        {0}
    set_instance_parameter_value      vid_out_vid_rst_bridge     NUM_RESET_OUTPUTS       {1}
    set_instance_parameter_value      vid_out_vid_rst_bridge     SYNCHRONOUS_EDGES       {deassert}
    set_instance_parameter_value      vid_out_vid_rst_bridge     SYNC_RESET              {0}
    set_instance_parameter_value      vid_out_vid_rst_bridge     USE_RESET_REQUEST       {0}

    # vid_out_tpg
    set_instance_parameter_value    vid_out_tpg          BINARY_DISPLAY_MODE         {Seconds}
    set_instance_parameter_value    vid_out_tpg          BPS                         ${v_bps}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_0            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_1            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_2            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_3            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_4            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_5            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_6            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_COL_SPACE_7            {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_0              {1}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_1              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_2              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_3              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_4              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_5              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_6              {0}
    set_instance_parameter_value    vid_out_tpg          CORE_PATTERN_7              {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_TYPE             {566}
    set_instance_parameter_value    vid_out_tpg          C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    vid_out_tpg          ENABLE_CTRL_IN              {0}
    set_instance_parameter_value    vid_out_tpg          ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    vid_out_tpg          EXTERNAL_MODE               {1}
    set_instance_parameter_value    vid_out_tpg          FIXED_BARS_MODE             {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_B_BACKGROUND          {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_B_CB                  {16}
    set_instance_parameter_value    vid_out_tpg          FIXED_B_FONT                {255}
    set_instance_parameter_value    vid_out_tpg          FIXED_FINE_FACTOR           {256}
    set_instance_parameter_value    vid_out_tpg          FIXED_FPS                   {60}
    set_instance_parameter_value    vid_out_tpg          FIXED_G_BACKGROUND          {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_G_FONT                {255}
    set_instance_parameter_value    vid_out_tpg          FIXED_G_Y                   {16}
    set_instance_parameter_value    vid_out_tpg          FIXED_HEIGHT                {16384}
    set_instance_parameter_value    vid_out_tpg          FIXED_INTERLACE             {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_LOCATION_X            {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_LOCATION_Y            {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_POWER_FACTOR          {16}
    set_instance_parameter_value    vid_out_tpg          FIXED_R_BACKGROUND          {0}
    set_instance_parameter_value    vid_out_tpg          FIXED_R_CR                  {16}
    set_instance_parameter_value    vid_out_tpg          FIXED_R_FONT                {255}
    set_instance_parameter_value    vid_out_tpg          FIXED_SCALE_FACTOR          {1}
    set_instance_parameter_value    vid_out_tpg          FIXED_WIDTH                 {16384}
    set_instance_parameter_value    vid_out_tpg          NUM_CORES                   {2}
    set_instance_parameter_value    vid_out_tpg          OUTPUT_FORMAT               {4.4.4}
    set_instance_parameter_value    vid_out_tpg          PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    vid_out_tpg          PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    vid_out_tpg          RUNTIME_CONTROL             {1}
    set_instance_parameter_value    vid_out_tpg          SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    vid_out_tpg          SLAVE_PROTOCOL              {Avalon}

    if {${v_drv_overlay_use_mem_copy}} {
        # vid_out_se_vfr
        set_instance_parameter_value      vid_out_se_vfr      BURSTCOUNT_WIDTH              {6}
        set_instance_parameter_value      vid_out_se_vfr      DATA_WIDTH                    {256}
        set_instance_parameter_value      vid_out_se_vfr      ENABLE_SLAVE_PORT             {0}
        set_instance_parameter_value      vid_out_se_vfr      MASTER_ADDRESS_DEF            {0}
        set_instance_parameter_value      vid_out_se_vfr      MASTER_ADDRESS_WIDTH          {33}
        set_instance_parameter_value      vid_out_se_vfr      MAX_PENDING_READS             {8}
        set_instance_parameter_value      vid_out_se_vfr      SLAVE_ADDRESS_WIDTH           {26}
        set_instance_parameter_value      vid_out_se_vfr      SUB_WINDOW_COUNT              {1}
        set_instance_parameter_value      vid_out_se_vfr      SYNC_RESET                    {0}
    }

    if {${v_overlay_en}} {
        # vid_out_vfr
        set_instance_parameter_value      vid_out_vfr     BPS                           ${v_overlay_bps}
        set_instance_parameter_value      vid_out_vfr     CLOCKS_ARE_SEPARATE           {1}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_TYPE               {586}
        set_instance_parameter_value      vid_out_vfr     C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      vid_out_vfr     ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      vid_out_vfr     EXTERNAL_MODE                 {1}
        set_instance_parameter_value      vid_out_vfr     MAX_BUFFER_SETS               {2}
        set_instance_parameter_value      vid_out_vfr     MAX_HEIGHT                    ${v_overlay_max_h}
        set_instance_parameter_value      vid_out_vfr     MAX_WIDTH                     ${v_overlay_max_w}
        set_instance_parameter_value      vid_out_vfr     NUMBER_OF_COLOR_PLANES        ${v_overlay_cppp}
        set_instance_parameter_value      vid_out_vfr     PACKING                       {PERFECT}
        set_instance_parameter_value      vid_out_vfr     PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      vid_out_vfr     P_AV_MM_ADDR_WIDTH            {32}
        set_instance_parameter_value      vid_out_vfr     P_AV_MM_DATA_WIDTH            {256}
        set_instance_parameter_value      vid_out_vfr     READ_BURST_TARGET             ${v_overlay_burst_length}
        set_instance_parameter_value      vid_out_vfr     READ_FIFO_DEPTH               {512}
        set_instance_parameter_value      vid_out_vfr     SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      vid_out_vfr     SLAVE_PROTOCOL                {Avalon}

        # vid_out_scaler_up
        set_instance_parameter_value      vid_out_scaler_up   ALGORITHM                     {NEAREST_NEIGHBOUR}
        set_instance_parameter_value      vid_out_scaler_up   BPS                           ${v_overlay_bps}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_TYPE               {564}
        set_instance_parameter_value      vid_out_scaler_up   C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      vid_out_scaler_up   EDGE_MIRROR                   {REPLICATE}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_420                    {0}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_420_MIRROR             {0}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_422                    {0}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_444                    {1}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_H                      {1}
        set_instance_parameter_value      vid_out_scaler_up   ENABLE_V                      {1}
        set_instance_parameter_value      vid_out_scaler_up   EXTERNAL_MODE                 {1}
        set_instance_parameter_value      vid_out_scaler_up   HALF_RATE_420                 {0}
        set_instance_parameter_value      vid_out_scaler_up   H_BANKS                       {1}
        set_instance_parameter_value      vid_out_scaler_up   H_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      vid_out_scaler_up   H_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      vid_out_scaler_up   H_COEFF_INT_BITS              {1}
        set_instance_parameter_value      vid_out_scaler_up   H_COEFF_SIGNED                {1}
        set_instance_parameter_value      vid_out_scaler_up   H_INIT_FILE \
                                                                        {<enter file name (including full path)>}
        set_instance_parameter_value      vid_out_scaler_up   H_PARTIAL_SCALING             {0}
        set_instance_parameter_value      vid_out_scaler_up   H_PHASES                      {4}
        set_instance_parameter_value      vid_out_scaler_up   H_TAPS                        {4}
        set_instance_parameter_value      vid_out_scaler_up   MAX_IN_WIDTH                  ${v_overlay_max_w}
        set_instance_parameter_value      vid_out_scaler_up   MAX_OUT_WIDTH                 {3840}
        set_instance_parameter_value      vid_out_scaler_up   MEM_INIT                      {1}
        set_instance_parameter_value      vid_out_scaler_up   NO_BLANKING                   {0}
        set_instance_parameter_value      vid_out_scaler_up   NUMBER_OF_COLOR_PLANES        ${v_overlay_cppp}
        set_instance_parameter_value      vid_out_scaler_up   OUTPUT_HEIGHT                 {2160}
        set_instance_parameter_value      vid_out_scaler_up   PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value      vid_out_scaler_up   PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      vid_out_scaler_up   P_CORE_CTRL_ID                {0}
        set_instance_parameter_value      vid_out_scaler_up   P_UPDATE_CMD_SUPPORTED        {0}
        set_instance_parameter_value      vid_out_scaler_up   RUNTIME_CONTROL               {1}
        set_instance_parameter_value      vid_out_scaler_up   RUNTIME_LOAD                  {0}
        set_instance_parameter_value      vid_out_scaler_up   SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      vid_out_scaler_up   SLAVE_PROTOCOL                {Avalon}
        set_instance_parameter_value      vid_out_scaler_up   V_BANKS                       {1}
        set_instance_parameter_value      vid_out_scaler_up   V_COEFF_FRAC_BITS             {8}
        set_instance_parameter_value      vid_out_scaler_up   V_COEFF_FUNCTION              {LANCZOS_2}
        set_instance_parameter_value      vid_out_scaler_up   V_COEFF_INT_BITS              {1}
        set_instance_parameter_value      vid_out_scaler_up   V_COEFF_SIGNED                {1}
        set_instance_parameter_value      vid_out_scaler_up   V_INIT_FILE \
                                                                        {<enter file name (including full path)>}
        set_instance_parameter_value      vid_out_scaler_up   V_PARTIAL_SCALING             {0}
        set_instance_parameter_value      vid_out_scaler_up   V_PHASES                      {4}
        set_instance_parameter_value      vid_out_scaler_up   V_PRES_FRAC_BITS              {0}
        set_instance_parameter_value      vid_out_scaler_up   V_TAPS                        {4}

        if {${v_drv_overlay_drm_ip_en}} {
            # vid_out_drm_adapter
            set_instance_parameter_value      vid_out_drm_adapter   BPS_IN                        ${v_overlay_bps}
            set_instance_parameter_value      vid_out_drm_adapter   BPS_OUT                       ${v_bps}
            set_instance_parameter_value      vid_out_drm_adapter   C_CPU_OFFSET                  {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_ID_ASSOCIATED      {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_IRQ                {255}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_IRQ_ENABLE         {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_IRQ_ENABLE_EN      {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_IRQ_STATUS         {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_IRQ_STATUS_EN      {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_SIZE               {64}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_TAG                {0}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_TYPE               {601}
            set_instance_parameter_value      vid_out_drm_adapter   C_OMNI_CAP_VERSION            {1}
            set_instance_parameter_value      vid_out_drm_adapter   NUMBER_OF_COLOR_PLANES        ${v_overlay_cppp}
            set_instance_parameter_value      vid_out_drm_adapter   PIXELS_IN_PARALLEL            ${v_pip}
            set_instance_parameter_value      vid_out_drm_adapter   RUNTIME_CONTROL               {1}
        } elseif {${v_overlay_argb2222} == 0} {
            # vid_out_pixel_adapter
            set_instance_parameter_value      vid_out_pixel_adapter   A_DITHER_BITS           {0}
            set_instance_parameter_value      vid_out_pixel_adapter   BPS_IN                  ${v_overlay_bps}
            set_instance_parameter_value      vid_out_pixel_adapter   BPS_OUT                 ${v_bps}
            set_instance_parameter_value      vid_out_pixel_adapter   B_DITHER_BITS           {0}
            set_instance_parameter_value      vid_out_pixel_adapter   ENABLE_DEBUG            {0}
            set_instance_parameter_value      vid_out_pixel_adapter   ENABLE_DITHER           {0}
            set_instance_parameter_value      vid_out_pixel_adapter   EXTERNAL_MODE           {1}
            set_instance_parameter_value      vid_out_pixel_adapter   FIXED_RAND_SEED         {1}
            set_instance_parameter_value      vid_out_pixel_adapter   G_DITHER_BITS           {0}
            set_instance_parameter_value      vid_out_pixel_adapter   MIRROR_TOP              {0}
            set_instance_parameter_value      vid_out_pixel_adapter   NOISE_OPERATION         {0}
            set_instance_parameter_value      vid_out_pixel_adapter   NUMBER_OF_COLOR_PLANES  ${v_overlay_cppp}
            set_instance_parameter_value      vid_out_pixel_adapter   OVERWRITE_MASK          {0}
            set_instance_parameter_value      vid_out_pixel_adapter   PIPELINE_READY          ${v_pipeline_ready}
            set_instance_parameter_value      vid_out_pixel_adapter   PIXELS_IN_PARALLEL      ${v_pip}
            set_instance_parameter_value      vid_out_pixel_adapter   P_CORE_CTRL_ID          {0}
            set_instance_parameter_value      vid_out_pixel_adapter   P_UPDATE_CMD_SUPPORTED  {0}
            set_instance_parameter_value      vid_out_pixel_adapter   RUNTIME_CONTROL         {0}
            set_instance_parameter_value      vid_out_pixel_adapter   R_DITHER_BITS           {0}
            set_instance_parameter_value      vid_out_pixel_adapter   SEPARATE_SLAVE_CLOCK    {0}
            set_instance_parameter_value      vid_out_pixel_adapter   SLAVE_PROTOCOL          {Avalon}
        }
    }

    if {${v_drv_logo_use_mem_copy}} {
        # vid_out_se_logo_vfr
        set_instance_parameter_value      vid_out_se_logo_vfr      BURSTCOUNT_WIDTH              {6}
        set_instance_parameter_value      vid_out_se_logo_vfr      DATA_WIDTH                    {256}
        set_instance_parameter_value      vid_out_se_logo_vfr      ENABLE_SLAVE_PORT             {0}
        set_instance_parameter_value      vid_out_se_logo_vfr      MASTER_ADDRESS_DEF            {0}
        set_instance_parameter_value      vid_out_se_logo_vfr      MASTER_ADDRESS_WIDTH          {33}
        set_instance_parameter_value      vid_out_se_logo_vfr      MAX_PENDING_READS             {8}
        set_instance_parameter_value      vid_out_se_logo_vfr      SLAVE_ADDRESS_WIDTH           {26}
        set_instance_parameter_value      vid_out_se_logo_vfr      SUB_WINDOW_COUNT              {1}
        set_instance_parameter_value      vid_out_se_logo_vfr      SYNC_RESET                    {0}
    }

    if {${v_logo_en}} {
        # vid_out_logo_vfr
        set_instance_parameter_value    vid_out_logo_vfr     BPS                        ${v_logo_bps}
        set_instance_parameter_value    vid_out_logo_vfr     CLOCKS_ARE_SEPARATE        {1}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_ID_ASSOCIATED   {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_ID_COMPONENT    ${v_inst_id}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_IRQ             {255}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_IRQ_ENABLE      {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_IRQ_ENABLE_EN   {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_IRQ_STATUS      {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_IRQ_STATUS_EN   {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_TAG             {0}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_TYPE            {586}
        set_instance_parameter_value    vid_out_logo_vfr     C_OMNI_CAP_VERSION         {1}
        set_instance_parameter_value    vid_out_logo_vfr     ENABLE_DEBUG               ${v_enable_debug}
        set_instance_parameter_value    vid_out_logo_vfr     EXTERNAL_MODE              {1}
        set_instance_parameter_value    vid_out_logo_vfr     MAX_BUFFER_SETS            {2}
        set_instance_parameter_value    vid_out_logo_vfr     MAX_HEIGHT                 ${v_logo_max_h}
        set_instance_parameter_value    vid_out_logo_vfr     MAX_WIDTH                  ${v_logo_max_w}
        set_instance_parameter_value    vid_out_logo_vfr     NUMBER_OF_COLOR_PLANES     ${v_logo_cppp}
        set_instance_parameter_value    vid_out_logo_vfr     PACKING                    {PERFECT}
        set_instance_parameter_value    vid_out_logo_vfr     PIXELS_IN_PARALLEL         ${v_pip}
        set_instance_parameter_value    vid_out_logo_vfr     P_AV_MM_ADDR_WIDTH         {32}
        set_instance_parameter_value    vid_out_logo_vfr     P_AV_MM_DATA_WIDTH         {256}
        set_instance_parameter_value    vid_out_logo_vfr     READ_BURST_TARGET          ${v_logo_burst_length}
        set_instance_parameter_value    vid_out_logo_vfr     READ_FIFO_DEPTH            {512}
        set_instance_parameter_value    vid_out_logo_vfr     SEPARATE_SLAVE_CLOCK       {1}
        set_instance_parameter_value    vid_out_logo_vfr     SLAVE_PROTOCOL             {Avalon}

        # vid_out_logo_pix_adapt
        set_instance_parameter_value    vid_out_logo_pix_adapt   A_DITHER_BITS           {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   BPS_IN                  ${v_logo_bps}
        set_instance_parameter_value    vid_out_logo_pix_adapt   BPS_OUT                 ${v_bps}
        set_instance_parameter_value    vid_out_logo_pix_adapt   B_DITHER_BITS           {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   ENABLE_DEBUG            {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   ENABLE_DITHER           {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   EXTERNAL_MODE           {1}
        set_instance_parameter_value    vid_out_logo_pix_adapt   FIXED_RAND_SEED         {1}
        set_instance_parameter_value    vid_out_logo_pix_adapt   G_DITHER_BITS           {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   MIRROR_TOP              {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   NOISE_OPERATION         {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   NUMBER_OF_COLOR_PLANES  ${v_logo_cppp}
        set_instance_parameter_value    vid_out_logo_pix_adapt   OVERWRITE_MASK          {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   PIPELINE_READY          ${v_pipeline_ready}
        set_instance_parameter_value    vid_out_logo_pix_adapt   PIXELS_IN_PARALLEL      ${v_pip}
        set_instance_parameter_value    vid_out_logo_pix_adapt   P_CORE_CTRL_ID          {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   P_UPDATE_CMD_SUPPORTED  {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   RUNTIME_CONTROL         {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   R_DITHER_BITS           {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   SEPARATE_SLAVE_CLOCK    {0}
        set_instance_parameter_value    vid_out_logo_pix_adapt   SLAVE_PROTOCOL          {Avalon}
    }

    if {${v_drv_logo_soft_ip_en}} {
        # vid_out_icon
        set_instance_parameter_value    vid_out_icon         BPS                         ${v_bps}
        set_instance_parameter_value    vid_out_icon         EXTERNAL_MODE               {1}
        set_instance_parameter_value    vid_out_icon         PIPELINE_READY              ${v_pipeline_ready}
        set_instance_parameter_value    vid_out_icon         PIXELS_IN_PARALLEL          ${v_pip}
    }

    # vid_out_mixer
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_1             {0}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_2             {3}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_3             {3}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_4             {3}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_5             {3}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_6             {3}
    set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_7             {3}
    set_instance_parameter_value    vid_out_mixer        BPS                         ${v_bps}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_TYPE             {563}
    set_instance_parameter_value    vid_out_mixer        C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    vid_out_mixer        DO_ROUNDING                 {0}
    set_instance_parameter_value    vid_out_mixer        ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    vid_out_mixer        EXPORT_PROBES               {0}
    set_instance_parameter_value    vid_out_mixer        EXTERNAL_MODE               {1}
    set_instance_parameter_value    vid_out_mixer        NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    vid_out_mixer        NUM_LAYERS                  ${v_mixer_channels}
    # logo soft ip does not have alpha.
    if {${v_mixer_channels} > 3} {
        if {${v_drv_logo_soft_ip_en}} {
            set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_3             {1}
        }
    } elseif {${v_mixer_channels} > 2} {
        if {${v_drv_logo_soft_ip_en}} {
            set_instance_parameter_value    vid_out_mixer        BLENDING_MODE_2             {1}
        }
    }
    set_instance_parameter_value    vid_out_mixer        PIPELINE_LEVEL              {1}
    set_instance_parameter_value    vid_out_mixer        PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    vid_out_mixer        P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    vid_out_mixer        P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_1        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_2        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_3        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_4        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_5        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_6        {1}
    set_instance_parameter_value    vid_out_mixer        RESTRICTED_OFFSETS_7        {1}
    set_instance_parameter_value    vid_out_mixer        RUNTIME_CONTROL             {1}
    set_instance_parameter_value    vid_out_mixer        SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    vid_out_mixer        SLAVE_PROTOCOL              {Avalon}

    # vid_out_1d_lut
    set_instance_parameter_value    vid_out_1d_lut       AV_MAX_PENDING_READS        {8}
    set_instance_parameter_value    vid_out_1d_lut       BITS_LUT                    {9}
    set_instance_parameter_value    vid_out_1d_lut       BITS_STEP                   {2}
    set_instance_parameter_value    vid_out_1d_lut       BPS_IN                      ${v_bps}
    set_instance_parameter_value    vid_out_1d_lut       BPS_OUT                     ${v_bps}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_TYPE             {381}
    set_instance_parameter_value    vid_out_1d_lut       C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    vid_out_1d_lut       DUPLICATE_AND_BYPASS        {0}
    set_instance_parameter_value    vid_out_1d_lut       ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    vid_out_1d_lut       ENABLE_EXT_DATA_RW          {0}
    set_instance_parameter_value    vid_out_1d_lut       EQUIDISTANT                 {0}
    set_instance_parameter_value    vid_out_1d_lut       EXTERNAL_MODE               {1}
    set_instance_parameter_value    vid_out_1d_lut       H_TAPS                      {1}
    set_instance_parameter_value    vid_out_1d_lut       MAX_HEIGHT                  {4096}
    set_instance_parameter_value    vid_out_1d_lut       MAX_WIDTH                   {8192}
    set_instance_parameter_value    vid_out_1d_lut       NO_BLANKING                 {1}
    set_instance_parameter_value    vid_out_1d_lut       NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    vid_out_1d_lut       PIPELINE_DATA_MM            {0}
    set_instance_parameter_value    vid_out_1d_lut       PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    vid_out_1d_lut       PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    vid_out_1d_lut       P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    vid_out_1d_lut       P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    vid_out_1d_lut       REVERSE_LUT                 {0}
    set_instance_parameter_value    vid_out_1d_lut       RUNTIME_CONTROL             {1}
    set_instance_parameter_value    vid_out_1d_lut       SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    vid_out_1d_lut       SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    vid_out_1d_lut       V_TAPS                      {1}

    if {${v_output_snapshot_en} || ${v_drv_overlay_use_mem_copy} || ${v_drv_logo_use_mem_copy}} {
        # vid_out_emif_clk_bridge
        set_instance_parameter_value  vid_out_emif_clk_bridge     EXPLICIT_CLOCK_RATE       ${v_emif_agent_clk_freq}
        set_instance_parameter_value  vid_out_emif_clk_bridge     NUM_CLOCK_OUTPUTS         {1}

        # vid_out_emif_rst_bridge
        set_instance_parameter_value  vid_out_emif_rst_bridge     ACTIVE_LOW_RESET          {1}
        set_instance_parameter_value  vid_out_emif_rst_bridge     NUM_RESET_OUTPUTS         {1}
        set_instance_parameter_value  vid_out_emif_rst_bridge     SYNCHRONOUS_EDGES         {deassert}
        set_instance_parameter_value  vid_out_emif_rst_bridge     SYNC_RESET                {0}
        set_instance_parameter_value  vid_out_emif_rst_bridge     USE_RESET_REQUEST         {0}
    }

    if {${v_output_snapshot_en}} {
        # vid_out_axi4s_bcast
        set_instance_parameter_value      vid_out_axi4s_bcast      BPS                           ${v_bps}
        set_instance_parameter_value      vid_out_axi4s_bcast      GLOBAL_STALL                  {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      IN_TREADY                     {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUTPUTS                       {2}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_0_FIFO                    {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_0_FIFO_DEPTH              {1024}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_0_TREADY                  {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_1_FIFO                    {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_1_FIFO_DEPTH              {1024}
        set_instance_parameter_value      vid_out_axi4s_bcast      OUT_1_TREADY                  {1}
        set_instance_parameter_value      vid_out_axi4s_bcast      PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      vid_out_axi4s_bcast      VVP_INTF_TYPE                 {Lite}

        # vid_out_vfw
        set_instance_parameter_value      vid_out_vfw         BPS                           ${v_bps}
        set_instance_parameter_value      vid_out_vfw         CLOCKS_ARE_SEPARATE           {1}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_TYPE               {585}
        set_instance_parameter_value      vid_out_vfw         C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value      vid_out_vfw         ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value      vid_out_vfw         EXTERNAL_MODE                 {1}
        set_instance_parameter_value      vid_out_vfw         MAX_HEIGHT                    {4096}
        set_instance_parameter_value      vid_out_vfw         MAX_WIDTH                     {4096}
        set_instance_parameter_value      vid_out_vfw         NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value      vid_out_vfw         PACKING                       {PERFECT}
        set_instance_parameter_value      vid_out_vfw         PIXELS_IN_PARALLEL            ${v_pip}
        set_instance_parameter_value      vid_out_vfw         P_AV_MM_ADDR_WIDTH            {32}
        set_instance_parameter_value      vid_out_vfw         P_AV_MM_DATA_WIDTH            {256}
        set_instance_parameter_value      vid_out_vfw         SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value      vid_out_vfw         SLAVE_PROTOCOL                {Avalon}
        # p60 or p30 1PIP is faster clock
        if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
            set_instance_parameter_value      vid_out_vfw         WRITE_BURST_TARGET            {64}
            set_instance_parameter_value      vid_out_vfw         WRITE_FIFO_DEPTH              {1024}
        } else {
            set_instance_parameter_value      vid_out_vfw         WRITE_BURST_TARGET            {32}
            set_instance_parameter_value      vid_out_vfw         WRITE_FIFO_DEPTH              {64}
        }

        # vid_out_se_vfw
        # p60 or p30 1PIP is faster clock
        if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
            set_instance_parameter_value      vid_out_se_vfw      BURSTCOUNT_WIDTH              {7}
        } else {
            set_instance_parameter_value      vid_out_se_vfw      BURSTCOUNT_WIDTH              {6}
        }
        set_instance_parameter_value      vid_out_se_vfw      DATA_WIDTH                    {256}
        set_instance_parameter_value      vid_out_se_vfw      ENABLE_SLAVE_PORT             {0}
        set_instance_parameter_value      vid_out_se_vfw      MASTER_ADDRESS_DEF            {0}
        set_instance_parameter_value      vid_out_se_vfw      MASTER_ADDRESS_WIDTH          {33}
        set_instance_parameter_value      vid_out_se_vfw      MAX_PENDING_READS             {8}
        set_instance_parameter_value      vid_out_se_vfw      SLAVE_ADDRESS_WIDTH           {26}
        set_instance_parameter_value      vid_out_se_vfw      SUB_WINDOW_COUNT              {1}
        set_instance_parameter_value      vid_out_se_vfw      SYNC_RESET                    {0}
    }

    if {${v_dual_clock_en} == 1} {
        # vid_out_if_clk_bridge
        set_instance_parameter_value    vid_out_if_clk_bridge    EXPLICIT_CLOCK_RATE   {297000000.0}
        set_instance_parameter_value    vid_out_if_clk_bridge    NUM_CLOCK_OUTPUTS     {1}

        # vid_out_if_rst_bridge
        set_instance_parameter_value    vid_out_if_rst_bridge      ACTIVE_LOW_RESET        {0}
        set_instance_parameter_value    vid_out_if_rst_bridge      NUM_RESET_OUTPUTS       {1}
        set_instance_parameter_value    vid_out_if_rst_bridge      SYNCHRONOUS_EDGES       {deassert}
        set_instance_parameter_value    vid_out_if_rst_bridge      SYNC_RESET              {0}
        set_instance_parameter_value    vid_out_if_rst_bridge      USE_RESET_REQUEST       {0}

        if {${v_pip} != ${v_vid_out_if_pip}} {
            # vid_out_pip_conv
            set_instance_parameter_value    vid_out_pip_conv     BPS                           ${v_vid_out_bps}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_ID_ASSOCIATED      {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ                {255}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_ENABLE         {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_ENABLE_EN      {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_STATUS         {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_STATUS_EN      {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_TAG                {0}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_TYPE               {569}
            set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_VERSION            {1}
            set_instance_parameter_value    vid_out_pip_conv     DUAL_CLOCK                    {1}
            set_instance_parameter_value    vid_out_pip_conv     FIFO_DEPTH                    ${v_pip_conv_fifo_depth}
            set_instance_parameter_value    vid_out_pip_conv     ENABLE_DEBUG                  ${v_enable_debug}
            set_instance_parameter_value    vid_out_pip_conv     EXTERNAL_MODE                 {1}
            set_instance_parameter_value    vid_out_pip_conv     NUMBER_OF_COLOR_PLANES        ${v_cppp}
            set_instance_parameter_value    vid_out_pip_conv     PIPELINE_READY                ${v_pipeline_ready}
            set_instance_parameter_value    vid_out_pip_conv     PIXELS_IN_PARALLEL_IN         ${v_pip}
            set_instance_parameter_value    vid_out_pip_conv     PIXELS_IN_PARALLEL_OUT        ${v_vid_out_if_pip}
            set_instance_parameter_value    vid_out_pip_conv     SEPARATE_SLAVE_CLOCK          {1}
            set_instance_parameter_value    vid_out_pip_conv     SLAVE_PROTOCOL                {Avalon}
        } else {
            # vid_out_fifo
            set_instance_parameter_value    vid_out_fifo         BPS                           ${v_vid_out_bps}
            set_instance_parameter_value    vid_out_fifo         DUAL_CLOCK                    {1}
            set_instance_parameter_value    vid_out_fifo         FIFO_DEPTH                    ${v_pip_conv_fifo_depth}
            set_instance_parameter_value    vid_out_fifo         NUMBER_OF_COLOR_PLANES        ${v_cppp}
            set_instance_parameter_value    vid_out_fifo         PIPELINE_READY                ${v_pipeline_ready}
            set_instance_parameter_value    vid_out_fifo         PIXELS_IN_PARALLEL            ${v_vid_out_if_pip}
        }

    } elseif {${v_pip} != ${v_vid_out_if_pip}} {
        # vid_out_pip_conv
        set_instance_parameter_value    vid_out_pip_conv     BPS                           ${v_vid_out_bps}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_TYPE               {569}
        set_instance_parameter_value    vid_out_pip_conv     C_OMNI_CAP_VERSION            {1}
        set_instance_parameter_value    vid_out_pip_conv     DUAL_CLOCK                    {0}
        set_instance_parameter_value    vid_out_pip_conv     FIFO_DEPTH                    ${v_pip_conv_fifo_depth}
        set_instance_parameter_value    vid_out_pip_conv     ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value    vid_out_pip_conv     EXTERNAL_MODE                 {1}
        set_instance_parameter_value    vid_out_pip_conv     NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value    vid_out_pip_conv     PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value    vid_out_pip_conv     PIXELS_IN_PARALLEL_IN         ${v_pip}
        set_instance_parameter_value    vid_out_pip_conv     PIXELS_IN_PARALLEL_OUT        ${v_vid_out_if_pip}
        set_instance_parameter_value    vid_out_pip_conv     SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value    vid_out_pip_conv     SLAVE_PROTOCOL                {Avalon}
    }

    # vid_out_proto_conv
    set_instance_parameter_value    vid_out_proto_conv     BPS                         ${v_vid_out_bps}
    set_instance_parameter_value    vid_out_proto_conv     CHROMA_SAMPLING             {444}
    set_instance_parameter_value    vid_out_proto_conv     CHROMA_SITING               {TOP_LEFT}
    set_instance_parameter_value    vid_out_proto_conv     CLIP_LONG_FIELDS            {0}
    set_instance_parameter_value    vid_out_proto_conv     COLOR_SPACE                 {RGB}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_TYPE             {573}
    set_instance_parameter_value    vid_out_proto_conv     C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    vid_out_proto_conv     ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    vid_out_proto_conv     ENABLE_TIMEOUT              {0}
    set_instance_parameter_value    vid_out_proto_conv     ENABLE_YCBCR_SWAP           {0}
    set_instance_parameter_value    vid_out_proto_conv     INPUT_MODE                  {EXTERNAL}
    set_instance_parameter_value    vid_out_proto_conv     NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    vid_out_proto_conv     OUTPUT_MODE                 {INTERNAL}
    set_instance_parameter_value    vid_out_proto_conv     PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    vid_out_proto_conv     PIXELS_IN_PARALLEL          ${v_vid_out_if_pip}
    set_instance_parameter_value    vid_out_proto_conv     RUNTIME_CONTROL             {1}
    set_instance_parameter_value    vid_out_proto_conv     SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    vid_out_proto_conv     SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    vid_out_proto_conv     VIP_USER_SUPPORT            {DISCARD}
    set_instance_parameter_value    vid_out_proto_conv     VVP_USER_SUPPORT            {NONE_ALLOWED}

    # vid_out_pio_board
    set_instance_parameter_value  vid_out_pio_board             bitClearingEdgeCapReg         {0}
    set_instance_parameter_value  vid_out_pio_board             bitModifyingOutReg            {0}
    set_instance_parameter_value  vid_out_pio_board             captureEdge                   {0}
    set_instance_parameter_value  vid_out_pio_board             direction                     {Input}
    set_instance_parameter_value  vid_out_pio_board             edgeType                      {RISING}
    set_instance_parameter_value  vid_out_pio_board             generateIRQ                   {0}
    set_instance_parameter_value  vid_out_pio_board             irqType                       {LEVEL}
    set_instance_parameter_value  vid_out_pio_board             resetValue                    {0.0}
    set_instance_parameter_value  vid_out_pio_board             simDoTestBenchWiring          {0}
    set_instance_parameter_value  vid_out_pio_board             simDrivenValue                {0.0}
    set_instance_parameter_value  vid_out_pio_board             width                         {32}

    # vid_out_pio_status
    set_instance_parameter_value  vid_out_pio_status            bitClearingEdgeCapReg         {0}
    set_instance_parameter_value  vid_out_pio_status            bitModifyingOutReg            {0}
    set_instance_parameter_value  vid_out_pio_status            captureEdge                   {0}
    set_instance_parameter_value  vid_out_pio_status            direction                     {Input}
    set_instance_parameter_value  vid_out_pio_status            edgeType                      {RISING}
    set_instance_parameter_value  vid_out_pio_status            generateIRQ                   {0}
    set_instance_parameter_value  vid_out_pio_status            irqType                       {LEVEL}
    set_instance_parameter_value  vid_out_pio_status            resetValue                    {0.0}
    set_instance_parameter_value  vid_out_pio_status            simDoTestBenchWiring          {0}
    set_instance_parameter_value  vid_out_pio_status            simDrivenValue                {0.0}
    set_instance_parameter_value  vid_out_pio_status            width                         {32}

    # vid_out_pio_supportd_fmats
    set_instance_parameter_value  vid_out_pio_supportd_fmats    bitClearingEdgeCapReg         {0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    bitModifyingOutReg            {0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    captureEdge                   {0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    direction                     {Input}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    edgeType                      {RISING}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    generateIRQ                   {0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    irqType                       {LEVEL}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    resetValue                    {0.0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    simDoTestBenchWiring          {0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    simDrivenValue                {0.0}
    set_instance_parameter_value  vid_out_pio_supportd_fmats    width                         {32}

    # vid_out_pio_curr_format
    set_instance_parameter_value  vid_out_pio_curr_format       bitClearingEdgeCapReg         {0}
    set_instance_parameter_value  vid_out_pio_curr_format       bitModifyingOutReg            {0}
    set_instance_parameter_value  vid_out_pio_curr_format       captureEdge                   {0}
    set_instance_parameter_value  vid_out_pio_curr_format       direction                     {Input}
    set_instance_parameter_value  vid_out_pio_curr_format       edgeType                      {RISING}
    set_instance_parameter_value  vid_out_pio_curr_format       generateIRQ                   {0}
    set_instance_parameter_value  vid_out_pio_curr_format       irqType                       {LEVEL}
    set_instance_parameter_value  vid_out_pio_curr_format       resetValue                    {0.0}
    set_instance_parameter_value  vid_out_pio_curr_format       simDoTestBenchWiring          {0}
    set_instance_parameter_value  vid_out_pio_curr_format       simDrivenValue                {0.0}
    set_instance_parameter_value  vid_out_pio_curr_format       width                         {32}

    # vid_out_pio_format_ovrride
    set_instance_parameter_value  vid_out_pio_format_ovrride    bitClearingEdgeCapReg         {0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    bitModifyingOutReg            {0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    captureEdge                   {0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    direction                     {Output}
    set_instance_parameter_value  vid_out_pio_format_ovrride    edgeType                      {RISING}
    set_instance_parameter_value  vid_out_pio_format_ovrride    generateIRQ                   {0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    irqType                       {LEVEL}
    set_instance_parameter_value  vid_out_pio_format_ovrride    resetValue                    {0.0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    simDoTestBenchWiring          {0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    simDrivenValue                {0.0}
    set_instance_parameter_value  vid_out_pio_format_ovrride    width                         {32}


    ############################
    #### Create Connections ####
    ############################

    # vid_out_cpu_clk_bridge
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_cpu_rst_bridge.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_mm_bridge.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_tpg.agent_clock
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_mixer.agent_clock
    if {${v_overlay_en} && (${v_drv_overlay_use_mem_copy} == 0)} {
        add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_vfr.mem_clock
    }
    if {${v_overlay_en}} {
        add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_vfr.control_clock
        add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_scaler_up.agent_clock
        if {${v_drv_overlay_drm_ip_en}} {
            add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_drm_adapter.agent_clock
        }
    }
    if {${v_logo_en} && (${v_drv_logo_use_mem_copy} == 0)} {
        add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_logo_vfr.mem_clock
    }
    if {${v_logo_en}} {
        add_connection  vid_out_cpu_clk_bridge.out_clk        vid_out_logo_vfr.control_clock
    }
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_1d_lut.agent_clock
    if {${v_output_snapshot_en}} {
        add_connection         vid_out_cpu_clk_bridge.out_clk         vid_out_vfw.control_clock
    }
    if {${v_pip} != ${v_vid_out_if_pip}} {
        add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pip_conv.agent_clock
    }
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_proto_conv.agent_clock
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pio_board.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pio_status.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pio_supportd_fmats.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pio_curr_format.clk
    add_connection          vid_out_cpu_clk_bridge.out_clk         vid_out_pio_format_ovrride.clk

    # vid_out_cpu_rst_bridge
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_mm_bridge.reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_tpg.agent_reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_mixer.agent_reset
    if {${v_overlay_en} && (${v_drv_overlay_use_mem_copy} == 0)} {
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_vfr.mem_reset
    }
    if {${v_overlay_en}} {
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_vfr.control_reset
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_scaler_up.agent_reset
        if {${v_drv_overlay_drm_ip_en}} {
            add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_drm_adapter.agent_reset
        }
    }
    if {${v_logo_en} && (${v_drv_logo_use_mem_copy} == 0)} {
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_logo_vfr.mem_reset
    }
    if {${v_logo_en}} {
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_logo_vfr.control_reset
    }
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_1d_lut.agent_reset
    if {${v_output_snapshot_en}} {
        add_connection      vid_out_cpu_rst_bridge.out_reset         vid_out_vfw.control_reset
    }
    if {${v_pip} != ${v_vid_out_if_pip}} {
        add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pip_conv.agent_reset
    }
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_proto_conv.agent_reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pio_board.reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pio_status.reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pio_supportd_fmats.reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pio_curr_format.reset
    add_connection          vid_out_cpu_rst_bridge.out_reset       vid_out_pio_format_ovrride.reset

    # vid_out_mm_bridge
    add_connection          vid_out_mm_bridge.m0             vid_out_tpg.av_mm_control_agent
    add_connection          vid_out_mm_bridge.m0             vid_out_mixer.av_mm_control_agent
    if {${v_overlay_en}} {
        add_connection          vid_out_mm_bridge.m0             vid_out_vfr.av_mm_control_agent
        add_connection          vid_out_mm_bridge.m0             vid_out_scaler_up.av_mm_control_agent
        if {${v_drv_overlay_drm_ip_en}} {
            add_connection          vid_out_mm_bridge.m0             vid_out_drm_adapter.av_mm_control_agent
        }
    }
    if {${v_logo_en}} {
        add_connection          vid_out_mm_bridge.m0             vid_out_logo_vfr.av_mm_control_agent
    }
    add_connection          vid_out_mm_bridge.m0            vid_out_1d_lut.av_mm_control_agent
    if {${v_output_snapshot_en}} {
        add_connection          vid_out_mm_bridge.m0            vid_out_vfw.av_mm_control_agent
    }
    if {${v_pip} != ${v_vid_out_if_pip}} {
        add_connection          vid_out_mm_bridge.m0            vid_out_pip_conv.av_mm_control_agent
    }
    add_connection          vid_out_mm_bridge.m0            vid_out_proto_conv.av_mm_control_agent
    add_connection          vid_out_mm_bridge.m0            vid_out_pio_board.s1
    add_connection          vid_out_mm_bridge.m0            vid_out_pio_status.s1
    add_connection          vid_out_mm_bridge.m0            vid_out_pio_supportd_fmats.s1
    add_connection          vid_out_mm_bridge.m0            vid_out_pio_curr_format.s1
    add_connection          vid_out_mm_bridge.m0            vid_out_pio_format_ovrride.s1

    # vid_out_vid_clk_bridge
    add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_vid_rst_bridge.clk
    add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_tpg.main_clock
    if {${v_overlay_en}} {
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_vfr.main_clock
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_scaler_up.main_clock
        if {${v_drv_overlay_drm_ip_en}} {
            add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_drm_adapter.main_clock
        } elseif {${v_overlay_argb2222} == 0} {
            add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_pixel_adapter.main_clock
        }
    }
    if {${v_logo_en}} {
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_logo_vfr.main_clock
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_logo_pix_adapt.main_clock
    }
    if {${v_drv_logo_soft_ip_en}} {
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_icon.main_clock
    }
    add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_mixer.main_clock
    add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_1d_lut.main_clock
    if {${v_output_snapshot_en}} {
        add_connection         vid_out_vid_clk_bridge.out_clk           vid_out_axi4s_bcast.vid_clock
        add_connection         vid_out_vid_clk_bridge.out_clk           vid_out_vfw.main_clock
    }
    if {${v_dual_clock_en} == 1} {
        if {${v_pip} != ${v_vid_out_if_pip}} {
            add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_pip_conv.in_clock
        } else {
            add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_fifo.in_clock
        }
    } elseif {${v_pip} != ${v_vid_out_if_pip}} {
        add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_pip_conv.main_clock
    }
    add_connection          vid_out_vid_clk_bridge.out_clk         vid_out_proto_conv.main_clock

    # vid_out_vid_rst_bridge
    add_connection    vid_out_vid_rst_bridge.out_reset        vid_out_tpg.main_reset
    if {${v_overlay_en}} {
        add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_vfr.main_reset
        add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_scaler_up.main_reset
        if {${v_drv_overlay_drm_ip_en}} {
            add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_drm_adapter.main_reset
        } elseif {${v_overlay_argb2222} == 0} {
            add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_pixel_adapter.main_reset
        }
    }
    if {${v_logo_en}} {
        add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_logo_vfr.main_reset
        add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_logo_pix_adapt.main_reset
    }
    if {${v_drv_logo_soft_ip_en}} {
        add_connection          vid_out_vid_rst_bridge.out_reset      vid_out_icon.main_reset
    }
    add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_mixer.main_reset
    add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_1d_lut.main_reset
    if {${v_output_snapshot_en}} {
        add_connection         vid_out_vid_rst_bridge.out_reset         vid_out_axi4s_bcast.vid_reset
        add_connection         vid_out_vid_rst_bridge.out_reset         vid_out_vfw.main_reset
    }
    if {${v_dual_clock_en} == 1} {
        if {${v_pip} != ${v_vid_out_if_pip}} {
            add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_pip_conv.in_reset
        } else {
            add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_fifo.in_reset
        }
    } elseif {${v_pip} != ${v_vid_out_if_pip}} {
        add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_pip_conv.main_reset
    }
    add_connection          vid_out_vid_rst_bridge.out_reset       vid_out_proto_conv.main_reset

    if {${v_dual_clock_en} == 1} {
        add_connection          vid_out_if_clk_bridge.out_clk          vid_out_if_rst_bridge.clk
        if {${v_pip} != ${v_vid_out_if_pip}} {
            # vid_out_if_clk_bridge
            add_connection          vid_out_if_clk_bridge.out_clk          vid_out_pip_conv.out_clock

            # vid_out_if_rst_bridge
            add_connection          vid_out_if_rst_bridge.out_reset        vid_out_pip_conv.out_reset
        } else {
            # vid_out_if_clk_bridge
            add_connection          vid_out_if_clk_bridge.out_clk          vid_out_fifo.out_clock

            # vid_out_if_rst_bridge
            add_connection          vid_out_if_rst_bridge.out_reset        vid_out_fifo.out_reset
        }
    }

    # vid_out_emif_clk_bridge
    if {${v_output_snapshot_en} || ${v_drv_overlay_use_mem_copy} || ${v_drv_logo_use_mem_copy}} {
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_emif_rst_bridge.clk
    }
    if {${v_output_snapshot_en}} {
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_vfw.mem_clock
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_se_vfw.clock
    }
    if {${v_drv_overlay_use_mem_copy}} {
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_se_vfr.clock
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_vfr.mem_clock
    }
    if {${v_drv_logo_use_mem_copy}} {
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_se_logo_vfr.clock
        add_connection         vid_out_emif_clk_bridge.out_clk          vid_out_logo_vfr.mem_clock
    }

    # vid_out_emif_rst_bridge
    if {${v_output_snapshot_en}} {
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_vfw.mem_reset
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_se_vfw.reset
    }
    if {${v_drv_overlay_use_mem_copy}} {
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_se_vfr.reset
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_vfr.mem_reset
    }
    if {${v_drv_logo_use_mem_copy}} {
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_se_logo_vfr.reset
        add_connection         vid_out_emif_rst_bridge.out_reset        vid_out_logo_vfr.mem_reset
    }

    # vid_out_tpg
    add_connection    vid_out_tpg.axi4s_vid_out        vid_out_mixer.axi4s_vid_0_in


    if {${v_drv_overlay_use_mem_copy}} {
        # vid_out_se_vfr
        add_connection          vid_out_vfr.av_mm_mem_read_host     vid_out_se_vfr.windowed_slave
    }

    if {${v_drv_logo_use_mem_copy}} {
        # vid_out_se_logo_vfr
        add_connection          vid_out_logo_vfr.av_mm_mem_read_host     vid_out_se_logo_vfr.windowed_slave
    }

    if {${v_overlay_en}} {
        # vid_out_vfr
        add_connection          vid_out_vfr.axi4s_vid_out           vid_out_scaler_up.axi4s_vid_in

        if {${v_drv_overlay_drm_ip_en}} {
            add_connection          vid_out_scaler_up.axi4s_vid_out       vid_out_drm_adapter.axi4s_vid_in

            # vid_out_drm_adapter
            add_connection          vid_out_drm_adapter.axi4s_vid_out                vid_out_mixer.axi4s_vid_2_in
        } elseif {${v_overlay_argb2222} == 0} {
            # vid_out_scaler_up
            add_connection          vid_out_scaler_up.axi4s_vid_out      vid_out_pixel_adapter.axi4s_vid_in

            # vid_out_pixel_adapter
            add_connection          vid_out_pixel_adapter.axi4s_vid_out      vid_out_mixer.axi4s_vid_2_in
        }

        if {${v_logo_en}} {
            # vid_out_logo_vfr
            add_connection          vid_out_logo_vfr.axi4s_vid_out           vid_out_logo_pix_adapt.axi4s_vid_in

            # vid_out_logo_pix_adapt
            add_connection          vid_out_logo_pix_adapt.axi4s_vid_out      vid_out_mixer.axi4s_vid_3_in

            if {${v_drv_logo_soft_ip_en}} {
                # vid_out_icon
                add_connection    vid_out_icon.axi4s_vid_out       vid_out_mixer.axi4s_vid_4_in
            }

        } elseif {${v_drv_logo_soft_ip_en}} {
            # vid_out_icon
            add_connection    vid_out_icon.axi4s_vid_out       vid_out_mixer.axi4s_vid_3_in
        }

    } elseif {${v_logo_en}} {
            # vid_out_logo_vfr
            add_connection          vid_out_logo_vfr.axi4s_vid_out           vid_out_logo_pix_adapt.axi4s_vid_in

            # vid_out_logo_pix_adapt
            add_connection          vid_out_logo_pix_adapt.axi4s_vid_out      vid_out_mixer.axi4s_vid_2_in

            if {${v_drv_logo_soft_ip_en}} {
                # vid_out_icon
                add_connection    vid_out_icon.axi4s_vid_out       vid_out_mixer.axi4s_vid_3_in
            }

    } elseif {${v_drv_logo_soft_ip_en}} {
        # vid_out_icon
        add_connection    vid_out_icon.axi4s_vid_out       vid_out_mixer.axi4s_vid_2_in
    }

    # vid_out_mixer
    add_connection    vid_out_mixer.axi4s_vid_out      vid_out_1d_lut.axi4s_vid_in

    if {${v_pip} != ${v_vid_out_if_pip}} {
        if {${v_output_snapshot_en}} {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_axi4s_bcast.axi4s_vid_in
            # vid_out_axi4s_bcast
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_0      vid_out_pip_conv.axi4s_vid_in
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_1      vid_out_vfw.axi4s_vid_in

            # vid_out_vfw
            add_connection         vid_out_vfw.av_mm_mem_write_host             vid_out_se_vfw.windowed_slave
        } else {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_pip_conv.axi4s_vid_in
        }
        # vid_out_pip_conv
        add_connection          vid_out_pip_conv.axi4s_vid_out          vid_out_proto_conv.axi4s_vid_in
    } elseif {${v_dual_clock_en}} {
        if {${v_output_snapshot_en}} {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_axi4s_bcast.axi4s_vid_in
            # vid_out_axi4s_bcast
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_0      vid_out_fifo.axi4s_vid_in
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_1      vid_out_vfw.axi4s_vid_in

            # vid_out_vfw
            add_connection         vid_out_vfw.av_mm_mem_write_host             vid_out_se_vfw.windowed_slave
        } else {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_fifo.axi4s_vid_in
        }
        # vid_out_fifo
        add_connection          vid_out_fifo.axi4s_vid_out              vid_out_proto_conv.axi4s_vid_in
    } else {
        if {${v_output_snapshot_en}} {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_axi4s_bcast.axi4s_vid_in
            # vid_out_axi4s_bcast
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_0      vid_out_proto_conv.axi4s_vid_in
            add_connection          vid_out_axi4s_bcast.axi4s_vid_out_1      vid_out_vfw.axi4s_vid_in

            # vid_out_vfw
            add_connection         vid_out_vfw.av_mm_mem_write_host             vid_out_se_vfw.windowed_slave
        } else {
            # vid_out_1d_lut
            add_connection          vid_out_1d_lut.axi4s_vid_out            vid_out_proto_conv.axi4s_vid_in
        }
    }

    ##########################
    ##### Create Exports #####
    ##########################

    # vid_out_cpu_clk_bridge
    add_interface           cpu_clk_in_clk        clock         sink
    set_interface_property  cpu_clk_in_clk        export_of     vid_out_cpu_clk_bridge.in_clk

    # vid_out_cpu_rst_bridge
    add_interface           cpu_reset_in_reset    reset         sink
    set_interface_property  cpu_reset_in_reset    export_of     vid_out_cpu_rst_bridge.in_reset

    # vid_out_mm_bridge
    add_interface           mm_ctrl_in            avalon        slave
    set_interface_property  mm_ctrl_in            export_of     vid_out_mm_bridge.s0

    # vid_out_vid_clk_bridge
    add_interface           vid_clk_in            clock         sink
    set_interface_property  vid_clk_in            export_of     vid_out_vid_clk_bridge.in_clk

    # vid_out_vid_rst_bridge
    add_interface           vid_rst_in            reset         sink
    set_interface_property  vid_rst_in            export_of     vid_out_vid_rst_bridge.in_reset

    if {${v_dual_clock_en} == 1} {
        # vid_out_if_clk_bridge
        add_interface           vid_out_if_clk_in     clock         sink
        set_interface_property  vid_out_if_clk_in     export_of     vid_out_if_clk_bridge.in_clk

        # vid_out_if_rst_bridge
        add_interface           vid_out_if_rst_in     reset         sink
        set_interface_property  vid_out_if_rst_in     export_of     vid_out_if_rst_bridge.in_reset
    }

    # vid_out_mixer
    add_interface           vid_in     axi4stream  subordinate
    set_interface_property  vid_in     EXPORT_OF   vid_out_mixer.axi4s_vid_1_in

    if {${v_overlay_en}} {
        if {${v_drv_overlay_use_mem_copy}} {
            # vid_out_se_vfr
            set_interface_property  av_mm_host_vfr  EXPORT_OF   vid_out_se_vfr.expanded_master
        } else {
            # vid_out_vfr
            add_interface           vid_out_overlay_av_mm_rd_host   avalon      host
            set_interface_property  vid_out_overlay_av_mm_rd_host   EXPORT_OF   vid_out_vfr.av_mm_mem_read_host
        }
        # vid_out_vfr
        set_interface_property  vid_out_overlay_vfr_int         EXPORT_OF   vid_out_vfr.frame_reader_int

        if {(${v_drv_overlay_drm_ip_en} == 0) && ${v_overlay_argb2222}} {
            # vid_out_scaler_up
            add_interface           vid_out_m_vid_axis    avalon      axi4stream  manager
            set_interface_property  vid_out_m_vid_axis    EXPORT_OF   vid_out_scaler_up.axi4s_vid_out

            # vid_out_mixer
            add_interface           vid_out_s_vid_axis     axi4stream  subordinate
            set_interface_property  vid_out_s_vid_axis     EXPORT_OF   vid_out_mixer.axi4s_vid_2_in
        }
    }

    if {${v_logo_en}} {
        if {${v_drv_logo_use_mem_copy}} {
            # vid_out_se_logo_vfr
            set_interface_property  av_mm_host_logo_vfr  EXPORT_OF   vid_out_se_logo_vfr.expanded_master
        } else {
            # vid_out_logo_vfr
            add_interface           vid_out_logo_av_mm_rd_host   avalon      host
            set_interface_property  vid_out_logo_av_mm_rd_host   EXPORT_OF   vid_out_logo_vfr.av_mm_mem_read_host
        }

        set_interface_property  vid_out_logo_vfr_int         EXPORT_OF   vid_out_logo_vfr.frame_reader_int
    }

    if {${v_output_snapshot_en} || ${v_drv_overlay_use_mem_copy} || ${v_drv_logo_use_mem_copy}} {
        # vid_out_emif_clk_bridge
        set_interface_property  emif_clk_in   EXPORT_OF   vid_out_emif_clk_bridge.in_clk

        # vid_out_emif_rst_bridge
        set_interface_property  emif_rst_in   EXPORT_OF   vid_out_emif_rst_bridge.in_reset
    }
    if {${v_output_snapshot_en}} {
        # vid_out_vfw
        set_interface_property  vfw_int    EXPORT_OF   vid_out_vfw.frame_writer_int

        # vid_out_se_vfw
        set_interface_property  av_mm_host_vfw    EXPORT_OF   vid_out_se_vfw.expanded_master
    }

    # vid_out_proto_conv
    add_interface           vid_out               axi4stream    manager
    set_interface_property  vid_out               export_of     vid_out_proto_conv.axi4s_vid_out

    # vid_out_pio_board
    add_interface             vid_out_board_pio           conduit     end
    set_interface_property    vid_out_board_pio           export_of   vid_out_pio_board.external_connection

    # vid_out_pio_status
    add_interface             vid_out_status_pio          conduit     end
    set_interface_property    vid_out_status_pio          export_of   vid_out_pio_status.external_connection

    # vid_out_pio_supportd_fmats
    add_interface           vid_out_supportd_fmats_pio    conduit     end
    set_interface_property  vid_out_supportd_fmats_pio    export_of   vid_out_pio_supportd_fmats.external_connection

    # vid_out_pio_curr_format
    add_interface             vid_out_curr_format_pio     conduit     end
    set_interface_property    vid_out_curr_format_pio     export_of   vid_out_pio_curr_format.external_connection

    # vid_out_pio_format_ovrride
    add_interface             vid_out_format_ovrride_pio  conduit     end
    set_interface_property    vid_out_format_ovrride_pio  export_of   vid_out_pio_format_ovrride.external_connection


    #################################
    ##### Assign Base Addresses #####
    #################################

    set v_tpg_addr_offset                 [get_shell_parameter TPG_ADDR_OFFSET]
    set v_mixer_addr_offset               [get_shell_parameter MIXER_ADDR_OFFSET]
    set v_vfr_addr_offset                 [get_shell_parameter VFR_ADDR_OFFSET]
    set v_logo_vfr_addr_offset            [get_shell_parameter LOGO_VFR_ADDR_OFFSET]
    set v_scaler_up_addr_offset           [get_shell_parameter SCALER_UP_ADDR_OFFSET]
    set v_1d_lut_addr_offset              [get_shell_parameter 1D_LUT_ADDR_OFFSET]
    set v_vfw_addr_offset                 [get_shell_parameter VFW_ADDR_OFFSET]
    set v_pip_conv_addr_offset            [get_shell_parameter PIP_CONV_ADDR_OFFSET]
    set v_proto_conv_addr_offset          [get_shell_parameter PROTO_CONV_ADDR_OFFSET]
    set v_pio_board_addr_offset           [get_shell_parameter PIO_BOARD_ADDR_OFFSET]
    set v_pio_status_addr_offset          [get_shell_parameter PIO_STATUS_ADDR_OFFSET]
    set v_pio_supp_formats_addr_offset    [get_shell_parameter PIO_SUPP_FORMATS_ADDR_OFFSET]
    set v_pio_curr_format_addr_offset     [get_shell_parameter PIO_CURR_FORMAT_ADDR_OFFSET]
    set v_pio_format_ovrride_addr_offset  [get_shell_parameter PIO_FORMAT_OVRRIDE_ADDR_OFFSET]
    set v_drm_adapter_addr_offset         [get_shell_parameter DRM_ADAPTER_ADDR_OFFSET]

    set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_tpg.av_mm_control_agent \
                                                                baseAddress ${v_tpg_addr_offset}
    set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_mixer.av_mm_control_agent \
                                                                baseAddress ${v_mixer_addr_offset}
    if {${v_overlay_en}} {
        set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_vfr.av_mm_control_agent \
                                                                baseAddress ${v_vfr_addr_offset}
        set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_scaler_up.av_mm_control_agent \
                                                                baseAddress ${v_scaler_up_addr_offset}
        if {${v_drv_overlay_drm_ip_en}} {
            set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_drm_adapter.av_mm_control_agent \
                                                                baseAddress ${v_drm_adapter_addr_offset}
        }
    }
    if {${v_logo_en}} {
        set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_logo_vfr.av_mm_control_agent \
                                                                baseAddress ${v_logo_vfr_addr_offset}
    }
    set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_1d_lut.av_mm_control_agent \
                                                                baseAddress ${v_1d_lut_addr_offset}
    if {${v_output_snapshot_en}} {
        set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_vfw.av_mm_control_agent \
                                                                baseAddress ${v_vfw_addr_offset}
    }

    if {${v_pip} != ${v_vid_out_if_pip}} {
        set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_pip_conv.av_mm_control_agent \
                                                                baseAddress ${v_pip_conv_addr_offset}
    }
    set_connection_parameter_value vid_out_mm_bridge.m0/vid_out_proto_conv.av_mm_control_agent \
                                                                baseAddress ${v_proto_conv_addr_offset}
    set_connection_parameter_value  vid_out_mm_bridge.m0/vid_out_pio_board.s1 \
                                                                baseAddress ${v_pio_board_addr_offset}
    set_connection_parameter_value  vid_out_mm_bridge.m0/vid_out_pio_status.s1 \
                                                                baseAddress ${v_pio_status_addr_offset}
    set_connection_parameter_value  vid_out_mm_bridge.m0/vid_out_pio_supportd_fmats.s1 \
                                                                baseAddress ${v_pio_supp_formats_addr_offset}
    set_connection_parameter_value  vid_out_mm_bridge.m0/vid_out_pio_curr_format.s1 \
                                                                baseAddress ${v_pio_curr_format_addr_offset}
    set_connection_parameter_value  vid_out_mm_bridge.m0/vid_out_pio_format_ovrride.s1 \
                                                                baseAddress ${v_pio_format_ovrride_addr_offset}

    lock_avalon_base_address  vid_out_tpg.av_mm_control_agent
    lock_avalon_base_address  vid_out_mixer.av_mm_control_agent
    if {${v_overlay_en}} {
        lock_avalon_base_address  vid_out_vfr.av_mm_control_agent
        lock_avalon_base_address  vid_out_scaler_up.av_mm_control_agent
        if {${v_drv_overlay_drm_ip_en}} {
            lock_avalon_base_address  vid_out_drm_adapter.av_mm_control_agent
        }
    }
    if {${v_logo_en}} {
        lock_avalon_base_address  vid_out_logo_vfr.av_mm_control_agent
    }
    lock_avalon_base_address  vid_out_1d_lut.av_mm_control_agent
    if {${v_output_snapshot_en}} {
        lock_avalon_base_address  vid_out_vfw.av_mm_control_agent
    }
    if {${v_pip} != ${v_vid_out_if_pip}} {
        lock_avalon_base_address  vid_out_pip_conv.av_mm_control_agent
    }
    lock_avalon_base_address  vid_out_proto_conv.av_mm_control_agent
    lock_avalon_base_address  vid_out_pio_board.s1
    lock_avalon_base_address  vid_out_pio_status.s1
    lock_avalon_base_address  vid_out_pio_supportd_fmats.s1
    lock_avalon_base_address  vid_out_pio_curr_format.s1
    lock_avalon_base_address  vid_out_pio_format_ovrride.s1


    #############################
    ##### Sync / Validation #####
    #############################

    sync_sysinfo_parameters
    save_system
}

proc edit_top_level_qsys {} {
    set v_project_name          [get_shell_parameter PROJECT_NAME]
    set v_project_path          [get_shell_parameter PROJECT_PATH]
    set v_instance_name         [get_shell_parameter INSTANCE_NAME]
    set v_overlay_en            [get_shell_parameter DRV_OVERLAY_EN]
    set v_overlay_argb2222      [get_shell_parameter OVERLAY_ARGB2222]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name}   ${v_instance_name}

    if {${v_overlay_en} && ${v_overlay_argb2222}} {
        # vid_out_scaler_up
        add_interface           vid_out_m_vid_axis     axi4stream  manager
        set_interface_property  vid_out_m_vid_axis     EXPORT_OF   ${v_instance_name}.vid_out_m_vid_axis

        # vid_out_mixer
        add_interface           vid_out_s_vid_axis     axi4stream  subordinate
        set_interface_property  vid_out_s_vid_axis     EXPORT_OF   ${v_instance_name}.vid_out_s_vid_axis
    }

    # vid_out_pio_board
    add_interface             "${v_instance_name}_vid_out_board_pio"         conduit     end
    set_interface_property    "${v_instance_name}_vid_out_board_pio" \
                              export_of   ${v_instance_name}.vid_out_board_pio

    # vid_out_pio_status
    add_interface             "${v_instance_name}_vid_out_status_pio"         conduit     end
    set_interface_property    "${v_instance_name}_vid_out_status_pio" \
                              export_of   ${v_instance_name}.vid_out_status_pio

    # vid_out_pio_supportd_fmats
    add_interface             "${v_instance_name}_vid_out_supportd_fmats_pio"     conduit     end
    set_interface_property    "${v_instance_name}_vid_out_supportd_fmats_pio" \
                              export_of   ${v_instance_name}.vid_out_supportd_fmats_pio

    # vid_out_pio_curr_format
    add_interface             "${v_instance_name}_vid_out_curr_format_pio"        conduit     end
    set_interface_property    "${v_instance_name}_vid_out_curr_format_pio" \
                              export_of   ${v_instance_name}.vid_out_curr_format_pio

    # vid_out_pio_format_ovrride
    add_interface             "${v_instance_name}_vid_out_format_ovrride_pio"     conduit     end
    set_interface_property    "${v_instance_name}_vid_out_format_ovrride_pio" \
                              export_of   ${v_instance_name}.vid_out_format_ovrride_pio

    sync_sysinfo_parameters
    save_system
}

proc add_auto_connections {} {
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host                 [get_shell_parameter AVMM_HOST]
    set v_avmm_host_clk_freq        [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_vid_out_rate              [get_shell_parameter VID_OUT_RATE]
    set v_pip                       [get_shell_parameter PIP]
    set v_dual_clock_en             [get_shell_parameter DUAL_CLOCK_EN]
    set v_overlay_en                [get_shell_parameter DRV_OVERLAY_EN]
    set v_overlay_irq_priority      [get_shell_parameter OVERLAY_IRQ_PRIORITY]
    set v_overlay_irq_host          [get_shell_parameter OVERLAY_IRQ_HOST]
    set v_drv_overlay_use_mem_copy  [get_shell_parameter DRV_OVERLAY_USE_MEM_COPY]
    set v_logo_en                   [get_shell_parameter DRV_LOGO_EN]
    set v_logo_irq_priority         [get_shell_parameter LOGO_IRQ_PRIORITY]
    set v_logo_irq_host             [get_shell_parameter LOGO_IRQ_HOST]
    set v_drv_logo_use_mem_copy     [get_shell_parameter DRV_LOGO_USE_MEM_COPY]
    set v_output_snapshot_en        [get_shell_parameter OUTPUT_SNAPSHOT_EN]
    set v_emif_agent                [get_shell_parameter EMIF_AGENT]

    add_auto_connection   ${v_instance_name}    cpu_clk_in_clk          ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name}    cpu_reset_in_reset      ${v_avmm_host_clk_freq}

    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
        add_auto_connection   ${v_instance_name}    vid_clk_in          297000000
        add_auto_connection   ${v_instance_name}    vid_rst_in          297000000
    } else {
        add_auto_connection   ${v_instance_name}    vid_clk_in          148500000
        add_auto_connection   ${v_instance_name}    vid_rst_in          148500000
    }

    if {${v_dual_clock_en} == 1} {
        add_auto_connection   ${v_instance_name}    vid_out_if_clk_in   297000000
        add_auto_connection   ${v_instance_name}    vid_out_if_rst_in   297000000
    }

    # Vid In
    add_auto_connection   ${v_instance_name}    vid_in      full_isp_out_vid_axis

    # vid_out_vfr
    if {${v_overlay_en}} {
        if {(${v_overlay_irq_host} != "NONE") && (${v_overlay_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "vid_out_overlay_vfr_int" \
                                    ${v_overlay_irq_priority}  ${v_overlay_irq_host}_irq
        }
    }
    # vid_out_logo_vfr
    if {${v_logo_en}} {
        if {(${v_logo_irq_host} != "NONE") && (${v_logo_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "vid_out_logo_vfr_int" \
                                    ${v_logo_irq_priority}  ${v_logo_irq_host}_irq
        }
    }

    if {${v_logo_en} && (${v_drv_logo_use_mem_copy} == 0)} {
            add_auto_connection  ${v_instance_name}   vid_out_logo_av_mm_rd_host      hps_dma_avmm_s0

            if {${v_overlay_en} && (${v_drv_overlay_use_mem_copy} == 0)} {
                add_auto_connection  ${v_instance_name}   vid_out_overlay_av_mm_rd_host      hps_dma_avmm_s1
            }
    } elseif {${v_overlay_en} && (${v_drv_overlay_use_mem_copy} == 0)} {
        add_auto_connection  ${v_instance_name}   vid_out_overlay_av_mm_rd_host      hps_dma_avmm_s0
    }

    # Pip Converter Out
    add_auto_connection   ${v_instance_name}    vid_out     vid_out_if_vid_in

    # HPS to mm bridge
    add_avmm_connections  mm_ctrl_in      ${v_avmm_host}

    if {${v_output_snapshot_en} || ${v_drv_overlay_use_mem_copy} || ${v_drv_logo_use_mem_copy}} {
        add_auto_connection   ${v_instance_name} emif_clk_in      ${v_emif_agent}_user_clk
        add_auto_connection   ${v_instance_name} emif_rst_in      ${v_emif_agent}_user_rst
    }
    if {${v_output_snapshot_en}} {
        add_auto_connection   ${v_instance_name} av_mm_host_vfw   ${v_emif_agent}_user_data
    }
    if {${v_drv_overlay_use_mem_copy}} {
        add_auto_connection   ${v_instance_name} av_mm_host_vfr   ${v_emif_agent}_user_data
    }
    if {${v_drv_logo_use_mem_copy}} {
        add_auto_connection   ${v_instance_name} av_mm_host_logo_vfr   ${v_emif_agent}_user_data
    }

}


proc edit_top_v_file {} {
    set v_instance_name       [get_shell_parameter INSTANCE_NAME]
    set v_overlay_en          [get_shell_parameter DRV_OVERLAY_EN]
    set v_overlay_argb2222    [get_shell_parameter OVERLAY_ARGB2222]
    set v_pip                 [get_shell_parameter PIP]

    if {${v_overlay_en} && ${v_overlay_argb2222}} {
        add_declaration_list wire   ""          "vid_out_m_vid_axis_tready"
        add_declaration_list wire   ""          "vid_out_m_vid_axis_tvalid"
        if {${v_pip} == 1} {
            add_declaration_list wire   "\[1:0\]"   "vid_out_m_vid_axis_tuser"
            add_declaration_list wire   "\[15:0\]"  "vid_out_m_vid_axis_tdata"
        } else {
            add_declaration_list wire   "\[3:0\]"   "vid_out_m_vid_axis_tuser"
            add_declaration_list wire   "\[31:0\]"  "vid_out_m_vid_axis_tdata"
        }
        add_declaration_list wire   ""          "vid_out_m_vid_axis_tlast"

        add_declaration_list wire   ""          "vid_out_s_vid_axis_tready"
        add_declaration_list wire   ""          "vid_out_s_vid_axis_tvalid"
        if {${v_pip} == 1} {
            add_declaration_list wire   "\[4:0\]"   "vid_out_s_vid_axis_tuser"
            add_declaration_list wire   "\[39:0\]"  "vid_out_s_vid_axis_tdata"
        } else {
            add_declaration_list wire   "\[9:0\]"   "vid_out_s_vid_axis_tuser"
            add_declaration_list wire   "\[79:0\]"  "vid_out_s_vid_axis_tdata"
        }
        add_declaration_list wire   ""          "vid_out_s_vid_axis_tlast"

        add_qsys_inst_exports_list    vid_out_m_vid_axis_tready        vid_out_m_vid_axis_tready
        add_qsys_inst_exports_list    vid_out_m_vid_axis_tvalid        vid_out_m_vid_axis_tvalid
        add_qsys_inst_exports_list    vid_out_m_vid_axis_tuser         vid_out_m_vid_axis_tuser
        add_qsys_inst_exports_list    vid_out_m_vid_axis_tdata         vid_out_m_vid_axis_tdata
        add_qsys_inst_exports_list    vid_out_m_vid_axis_tlast         vid_out_m_vid_axis_tlast

        add_qsys_inst_exports_list    vid_out_s_vid_axis_tready        vid_out_s_vid_axis_tready
        add_qsys_inst_exports_list    vid_out_s_vid_axis_tvalid        vid_out_s_vid_axis_tvalid
        add_qsys_inst_exports_list    vid_out_s_vid_axis_tuser         vid_out_s_vid_axis_tuser
        add_qsys_inst_exports_list    vid_out_s_vid_axis_tdata         vid_out_s_vid_axis_tdata
        add_qsys_inst_exports_list    vid_out_s_vid_axis_tlast         vid_out_s_vid_axis_tlast

        set generic_code {}
        lappend generic_code  "assign vid_out_m_vid_axis_tready         = vid_out_s_vid_axis_tready;"
        lappend generic_code  "assign vid_out_s_vid_axis_tvalid         = vid_out_m_vid_axis_tvalid;"
        lappend generic_code  "// convert Overlay ARGB2222 to ARGB10101010"
        lappend generic_code  "// decode each 2 bit value to represent 0, 33, 66, or 100% 10 bit value"
        lappend generic_code  "assign vid_out_s_vid_axis_tdata\[9:0\]     = (vid_out_m_vid_axis_tdata\[1:0\] == 0) ? 10'h0 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[1:0\] == 1) ? 10'h155 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[1:0\] == 2) ? 10'h2aa : 10'h3ff;"
        lappend generic_code  "assign vid_out_s_vid_axis_tdata\[19:10\]   = (vid_out_m_vid_axis_tdata\[3:2\] == 0) ? 10'h0 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[3:2\] == 1) ? 10'h155 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[3:2\] == 2) ? 10'h2aa : 10'h3ff;"
        lappend generic_code  "assign vid_out_s_vid_axis_tdata\[29:20\]   = (vid_out_m_vid_axis_tdata\[5:4\] == 0) ? 10'h0 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[5:4\] == 1) ? 10'h155 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[5:4\] == 2) ? 10'h2aa : 10'h3ff;"
        lappend generic_code  "assign vid_out_s_vid_axis_tdata\[39:30\]   = (vid_out_m_vid_axis_tdata\[7:6\] == 0) ? 10'h0 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[7:6\] == 1) ? 10'h155 : "
        lappend generic_code  "                   (vid_out_m_vid_axis_tdata\[7:6\] == 2) ? 10'h2aa : 10'h3ff;"
        if {${v_pip} > 1} {
            lappend generic_code  "assign vid_out_s_vid_axis_tdata\[49:40\]   = (vid_out_m_vid_axis_tdata\[17:16\] == 0) ? 10'h0 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[17:16\] == 1) ? 10'h155 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[17:16\] == 2) ? 10'h2aa : 10'h3ff;"
            lappend generic_code  "assign vid_out_s_vid_axis_tdata\[59:50\]   = (vid_out_m_vid_axis_tdata\[19:18\] == 0) ? 10'h0 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[19:18\] == 1) ? 10'h155 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[19:18\] == 2) ? 10'h2aa : 10'h3ff;"
            lappend generic_code  "assign vid_out_s_vid_axis_tdata\[69:60\]   = (vid_out_m_vid_axis_tdata\[21:20\] == 0) ? 10'h0 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[21:20\] == 1) ? 10'h155 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[21:20\] == 2) ? 10'h2aa : 10'h3ff;"
            lappend generic_code  "assign vid_out_s_vid_axis_tdata\[79:70\]   = (vid_out_m_vid_axis_tdata\[23:22\] == 0) ? 10'h0 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[23:22\] == 1) ? 10'h155 : "
            lappend generic_code  "                                          (vid_out_m_vid_axis_tdata\[23:22\] == 2) ? 10'h2aa : 10'h3ff;"
        }
        lappend generic_code  "assign vid_out_s_vid_axis_tlast          = vid_out_m_vid_axis_tlast;"
        if {${v_pip} == 1} {
            lappend generic_code  "assign vid_out_s_vid_axis_tuser          = {4'b0 , vid_out_m_vid_axis_tuser\[0\]};"
        } else {
            lappend generic_code  "assign vid_out_s_vid_axis_tuser          = {9'b0 , vid_out_m_vid_axis_tuser\[0\]};"
        }

        add_code_insert_list $generic_code

    }

    # PIO
    add_declaration_list  reg "\[31:0\]"  vid_out_pio_board_i
    add_declaration_list  reg "\[31:0\]"  vid_out_status_i
    add_declaration_list  reg "\[31:0\]"  vid_out_supportd_fmats_i
    add_declaration_list  reg "\[31:0\]"  vid_out_curr_format_i
    add_declaration_list  reg "\[31:0\]"  vid_out_format_ovrride_o

    # PIO
    add_qsys_inst_exports_list  ${v_instance_name}_vid_out_board_pio_export             vid_out_pio_board_i
    add_qsys_inst_exports_list  ${v_instance_name}_vid_out_status_pio_export            vid_out_status_i
    add_qsys_inst_exports_list  ${v_instance_name}_vid_out_supportd_fmats_pio_export    vid_out_supportd_fmats_i
    add_qsys_inst_exports_list  ${v_instance_name}_vid_out_curr_format_pio_export       vid_out_curr_format_i
    add_qsys_inst_exports_list  ${v_instance_name}_vid_out_format_ovrride_pio_export    vid_out_format_ovrride_o
}

proc modify_manual_ocs_rom {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_drv_ocs_subsystem_name    [get_shell_parameter DRV_OCS_SUBSYSTEM_NAME]
    set v_hps_dma_enable            [get_shell_parameter HPS_DMA_ENABLE]
    set v_hps_dma_num_of_channels   [get_shell_parameter HPS_DMA_NUM_OF_CHANNELS]

    if {${v_drv_ocs_subsystem_name} != ""} {

        set v_ocs_entry_base                  [get_shell_parameter OCS_ENTRY_BASE]
        set v_pio_board_addr_offset           [get_shell_parameter PIO_BOARD_ADDR_OFFSET]
        set v_pio_status_addr_offset          [get_shell_parameter PIO_STATUS_ADDR_OFFSET]
        set v_pio_supp_formats_addr_offset    [get_shell_parameter PIO_SUPP_FORMATS_ADDR_OFFSET]
        set v_pio_curr_format_addr_offset     [get_shell_parameter PIO_CURR_FORMAT_ADDR_OFFSET]
        set v_pio_format_ovrride_addr_offset  [get_shell_parameter PIO_FORMAT_OVRRIDE_ADDR_OFFSET]
        set v_inst_id                         [get_shell_parameter INST_ID]

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

        if {${v_hps_dma_enable}} {
            set_component_parameter_value  C_NUM_CAPS [expr ${v_num_caps} + 5 + ${v_hps_dma_num_of_channels}]
        } else {
            set_component_parameter_value  C_NUM_CAPS [expr ${v_num_caps} + 5]
        }

        set v_index ${v_num_caps}

        # DP Multi-rate control

        # dp pio - pio_board
        set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} \
                                                                            + ${v_pio_board_addr_offset}]
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT ${v_inst_id}

        set v_index [incr v_index]

        # dp pio - pio_act_dim
        set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} \
                                                                            + ${v_pio_status_addr_offset}]
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT [expr ${v_inst_id} + 1]

        set v_index [incr v_index]

        # dp pio - pio_fr_dim
        set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} \
                                                                            + ${v_pio_supp_formats_addr_offset}]
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT [expr ${v_inst_id} + 2]

        set v_index [incr v_index]

        # dp pio - pio_new_act_dim
        set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} \
                                                                            + ${v_pio_curr_format_addr_offset}]
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT [expr ${v_inst_id} + 3]

        set v_index [incr v_index]

        # dp pio - pio_status
        set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} \
                                                                            + ${v_pio_format_ovrride_addr_offset}]
        set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
        set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
        set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
        set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT [expr ${v_inst_id} + 4]

        set v_index [incr v_index]

        if {${v_hps_dma_enable}} {
            set addr_base     "0x00800a00"

            for {set i 0} {${i} <= [expr ${v_hps_dma_num_of_channels} - 1]} {incr i} {
                set addr_offset   [expr (${i} * 64)]

                # manual address span extender - DMA HPS to FPGA
                set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${addr_base} + ${addr_offset}]
                set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
                set_component_parameter_value      C_CAP${v_index}_TYPE         {530}
                set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
                set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT ${i}

                set v_index [incr v_index]
            }

        }

        save_component

        sync_sysinfo_parameters
        save_system
    }
}
