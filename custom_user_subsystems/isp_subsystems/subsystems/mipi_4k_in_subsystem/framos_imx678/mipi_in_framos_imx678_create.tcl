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

set_shell_parameter AVMM_HOST                   {{AUTO X}}
set_shell_parameter AVMM_HOST_CLK_FREQ          {200000000.0}

set_shell_parameter OCS_ENTRY_BASE              ""

# IP AVMM Offset Addresses
set_shell_parameter DPHY_ADDR_OFFSET            "0x00000000"
set_shell_parameter PIO_0_ADDR_OFFSET           "0x00001000"
set_shell_parameter PIO_1_ADDR_OFFSET           "0x00001010"
set_shell_parameter MM_CCB_ADDR_OFFSET          "0x00002000"
set_shell_parameter SNOOP_0_ADDR_OFFSET         "0x00001a00"
set_shell_parameter EXP_FUSION_0_ADDR_OFFSET    "0x00001200"
set_shell_parameter PIP_CONV_0_ADDR_OFFSET      "0x00001600"
set_shell_parameter SNOOP_1_ADDR_OFFSET         "0x00001c00"
set_shell_parameter EXP_FUSION_1_ADDR_OFFSET    "0x00001400"
set_shell_parameter PIP_CONV_1_ADDR_OFFSET      "0x00001800"
set_shell_parameter CSI_0_ADDR_OFFSET           "0x00000000"
set_shell_parameter CSI_1_ADDR_OFFSET           "0x00001000"

# General Video Controls
set_shell_parameter PIP                         {2}
set_shell_parameter VID_OUT_RATE                "p60"
set_shell_parameter EN_DEBUG                    {1}

set_shell_parameter INST_ID                     {0}

set_shell_parameter MULTI_SENSOR                {1}
set_shell_parameter EXP_FUSION_EN               {0}


proc pre_creation_step {} {
    transfer_files
    evaluate_terp
}

proc creation_step {} {
    create_mipi_in_subsystem
}

proc post_creation_step {} {
    modify_manual_ocs_rom
    edit_top_level_qsys
    add_auto_connections
    edit_top_v_file
}


# resolve interdependencies
proc derive_parameters {param_array} {
    upvar $param_array p_array

    #set v_drv_clock_subsystem_name ""
    set_shell_parameter DRV_CLOCK_SUBSYSTEM_NAME ""
    set_shell_parameter DRV_OCS_SUBSYSTEM_NAME ""

    for {set id 0} {$id < $p_array(project,id)} {incr id} {
        if {$p_array($id,type) == "clock"} {
            set params $p_array($id,params)

            foreach v_pair ${params} {
                set v_name  [lindex ${v_pair} 0]
                set v_value [lindex ${v_pair} 1]
                set v_temp_array(${v_name}) ${v_value}
            }

            if {[info exists v_temp_array(INSTANCE_NAME)]} {
               set_shell_parameter DRV_CLOCK_SUBSYSTEM_NAME $v_temp_array(INSTANCE_NAME)
            }
        }

        # look for OCS subsystem
        if {$p_array($id,type) == "ocs"} {

            set params $p_array($id,params)

            foreach v_pair ${params} {
                set v_name  [lindex ${v_pair} 0]
                set v_value [lindex ${v_pair} 1]

                if {${v_name} == "INSTANCE_NAME"} {
                    set_shell_parameter DRV_OCS_SUBSYSTEM_NAME ${v_value}
                }
            }
        }

    }
}

proc transfer_files {} {
    set v_project_path            [get_shell_parameter PROJECT_PATH]
    set v_script_path             [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_multi_sensor            [get_shell_parameter MULTI_SENSOR]
    set v_exp_fusion_en           [get_shell_parameter EXP_FUSION_EN]

    if {${v_multi_sensor}} {
        file_copy   ${v_script_path}/mipi_in_dual_framos_imx678_ModKit.qsf.terp \
                                                      ${v_project_path}/quartus/user/mipi_in.qsf.terp
        file_copy   ${v_script_path}/mipi_in_dual_framos_imx678.sdc.terp \
                                                      ${v_project_path}/sdc/user/mipi_in.sdc
    } else {
        file_copy   ${v_script_path}/mipi_in_framos_0_imx678_ModKit.qsf.terp \
                                                      ${v_project_path}/quartus/user/mipi_in.qsf.terp
        file_copy   ${v_script_path}/mipi_in_framos_0_imx678.sdc.terp \
                                                      ${v_project_path}/sdc/user/mipi_in.sdc
    }

    if {${v_exp_fusion_en}} {
        exec cp -rf ${v_script_path}/../../../non_qpds_ip/intel_vvp_exposure_fusion \
                                                                ${v_project_path}/non_qpds_ip/user
        file_copy   ${v_script_path}/../../../non_qpds_ip/intel_vvp_exposure_fusion.ipx \
                                                                ${v_project_path}/non_qpds_ip/user
    }
}

proc evaluate_terp {} {
    set v_project_name  [get_shell_parameter PROJECT_NAME]
    set v_project_path  [get_shell_parameter PROJECT_PATH]

    evaluate_terp_file  ${v_project_path}/quartus/user/mipi_in.qsf.terp    [list ${v_project_name}] 0 1
}

proc create_mipi_in_subsystem {} {
    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_instance_name     [get_shell_parameter INSTANCE_NAME]

    # CSI2 Clocking Mode
    set v_continuous_clk      {1}

    # CSI2 Video pipeline
    set v_mipi_pip            {4}
    set v_mipi_bps            {16}

    # video pipeline
    set v_cppp                {1}
    set v_bps                 {12}
    set v_hdr_bps             {16}
    set v_pip                 [get_shell_parameter PIP]
    set v_vid_out_rate        [get_shell_parameter VID_OUT_RATE]

    # General
    set v_inst_id             [get_shell_parameter INST_ID]
    set v_enable_debug        [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready      {1}
    set v_multi_sensor        [get_shell_parameter MULTI_SENSOR]
    set v_exp_fusion_en       [get_shell_parameter EXP_FUSION_EN]

    set v_avmm_host_clk_freq  [get_shell_parameter AVMM_HOST_CLK_FREQ]


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  mipi_in_cpu_clk_bridge          altera_clock_bridge
    add_instance  mipi_in_cpu_rst_bridge          altera_reset_bridge
    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        add_instance  mipi_in_mipi_clk_bridge       altera_clock_bridge
        add_instance  mipi_in_mipi_rst_bridge       altera_reset_bridge
    }
    add_instance  mipi_in_vid_clk_bridge          altera_clock_bridge
    add_instance  mipi_in_vid_rst_bridge          altera_reset_bridge
    add_instance  mipi_in_mm_bridge               altera_avalon_mm_bridge
    add_instance  mipi_in_pio_0                   altera_avalon_pio
    add_instance  mipi_in_pio_1                   altera_avalon_pio
    add_instance  mipi_in_mm_ccb                  mm_ccb
    add_instance  mipi_in_mipi_dphy               mipi_dphy
    add_instance  mipi_in_mipi_csi2_0             intel_mipi_csi2
    add_instance  mipi_in_snoop_0_0               intel_vvp_snoop
    add_instance  mipi_in_proto_conv_0_0          intel_vvp_protocol_conv
    if {${v_exp_fusion_en}} {
        add_instance  mipi_in_proto_conv_0_1        intel_vvp_protocol_conv
        add_instance  mipi_in_exp_fusion_0          intel_vvp_exposure_fusion
    }
    add_instance  mipi_in_pip_conv_0              intel_vvp_pip_conv
    if {${v_multi_sensor}} {
        add_instance  mipi_in_mipi_csi2_1           intel_mipi_csi2
        add_instance  mipi_in_snoop_1_0             intel_vvp_snoop
        add_instance  mipi_in_proto_conv_1_0        intel_vvp_protocol_conv
        if {${v_exp_fusion_en}} {
            add_instance  mipi_in_proto_conv_1_1      intel_vvp_protocol_conv
            add_instance  mipi_in_exp_fusion_1        intel_vvp_exposure_fusion
        }
        add_instance  mipi_in_pip_conv_1            intel_vvp_pip_conv
    }


    ############################
    #### Set Parameters     ####
    ############################

    # mipi_in_cpu_clk_bridge
    set_instance_parameter_value    mipi_in_cpu_clk_bridge    EXPLICIT_CLOCK_RATE   ${v_avmm_host_clk_freq}
    set_instance_parameter_value    mipi_in_cpu_clk_bridge    NUM_CLOCK_OUTPUTS     {1}

    # mipi_in_cpu_rst_bridge
    set_instance_parameter_value    mipi_in_cpu_rst_bridge    ACTIVE_LOW_RESET      {0}
    set_instance_parameter_value    mipi_in_cpu_rst_bridge    NUM_RESET_OUTPUTS     {1}
    set_instance_parameter_value    mipi_in_cpu_rst_bridge    SYNCHRONOUS_EDGES     {deassert}
    set_instance_parameter_value    mipi_in_cpu_rst_bridge    SYNC_RESET            {0}
    set_instance_parameter_value    mipi_in_cpu_rst_bridge    USE_RESET_REQUEST     {0}

    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        # mipi_in_mipi_clk_bridge
        set_instance_parameter_value    mipi_in_mipi_clk_bridge     EXPLICIT_CLOCK_RATE     {297000000.0}
        set_instance_parameter_value    mipi_in_mipi_clk_bridge     NUM_CLOCK_OUTPUTS       {1}

        # mipi_in_mipi_rst_bridge
        set_instance_parameter_value    mipi_in_mipi_rst_bridge     ACTIVE_LOW_RESET        {0}
        set_instance_parameter_value    mipi_in_mipi_rst_bridge     NUM_RESET_OUTPUTS       {1}
        set_instance_parameter_value    mipi_in_mipi_rst_bridge     SYNCHRONOUS_EDGES       {deassert}
        set_instance_parameter_value    mipi_in_mipi_rst_bridge     SYNC_RESET              {0}
        set_instance_parameter_value    mipi_in_mipi_rst_bridge     USE_RESET_REQUEST       {0}
    }

    # mipi_in_vid_clk_bridge
    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        set_instance_parameter_value    mipi_in_vid_clk_bridge    EXPLICIT_CLOCK_RATE     {1485000000.0}
    } else {
        set_instance_parameter_value    mipi_in_vid_clk_bridge    EXPLICIT_CLOCK_RATE     {297000000.0}
    }
    set_instance_parameter_value    mipi_in_vid_clk_bridge    NUM_CLOCK_OUTPUTS       {1}

    # mipi_in_vid_rst_bridge
    set_instance_parameter_value    mipi_in_vid_rst_bridge    ACTIVE_LOW_RESET      {0}
    set_instance_parameter_value    mipi_in_vid_rst_bridge    NUM_RESET_OUTPUTS     {1}
    set_instance_parameter_value    mipi_in_vid_rst_bridge    SYNCHRONOUS_EDGES     {deassert}
    set_instance_parameter_value    mipi_in_vid_rst_bridge    SYNC_RESET            {0}
    set_instance_parameter_value    mipi_in_vid_rst_bridge    USE_RESET_REQUEST     {0}

    # mipi_in_mm_bridge
    set_instance_parameter_value    mipi_in_mm_bridge     ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value    mipi_in_mm_bridge     ADDRESS_WIDTH                 {14}
    set_instance_parameter_value    mipi_in_mm_bridge     DATA_WIDTH                    {32}
    set_instance_parameter_value    mipi_in_mm_bridge     LINEWRAPBURSTS                {0}
    set_instance_parameter_value    mipi_in_mm_bridge     M0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value    mipi_in_mm_bridge     MAX_BURST_SIZE                {1}
    set_instance_parameter_value    mipi_in_mm_bridge     MAX_PENDING_RESPONSES         {4}
    set_instance_parameter_value    mipi_in_mm_bridge     MAX_PENDING_WRITES            {0}
    set_instance_parameter_value    mipi_in_mm_bridge     PIPELINE_COMMAND              {1}
    set_instance_parameter_value    mipi_in_mm_bridge     PIPELINE_RESPONSE             {1}
    set_instance_parameter_value    mipi_in_mm_bridge     S0_WAITREQUEST_ALLOWANCE      {0}
    set_instance_parameter_value    mipi_in_mm_bridge     SYMBOL_WIDTH                  {8}
    set_instance_parameter_value    mipi_in_mm_bridge     SYNC_RESET                    {1}
    set_instance_parameter_value    mipi_in_mm_bridge     USE_AUTO_ADDRESS_WIDTH        {1}
    set_instance_parameter_value    mipi_in_mm_bridge     USE_RESPONSE                  {0}
    set_instance_parameter_value    mipi_in_mm_bridge     USE_WRITERESPONSE             {0}

    # mipi_in_pio_0 - configuration info
    set_instance_parameter_value    mipi_in_pio_0     bitClearingEdgeCapReg         {0}
    set_instance_parameter_value    mipi_in_pio_0     bitModifyingOutReg            {0}
    set_instance_parameter_value    mipi_in_pio_0     captureEdge                   {0}
    set_instance_parameter_value    mipi_in_pio_0     direction                     {InOut}
    set_instance_parameter_value    mipi_in_pio_0     edgeType                      {RISING}
    set_instance_parameter_value    mipi_in_pio_0     generateIRQ                   {0}
    set_instance_parameter_value    mipi_in_pio_0     irqType                       {LEVEL}
    set_instance_parameter_value    mipi_in_pio_0     resetValue                    {0.0}
    set_instance_parameter_value    mipi_in_pio_0     simDoTestBenchWiring          {0}
    set_instance_parameter_value    mipi_in_pio_0     simDrivenValue                {0.0}
    set_instance_parameter_value    mipi_in_pio_0     width                         {32}

    # mipi_in_pio_1 - timestamp
    set_instance_parameter_value    mipi_in_pio_1     bitClearingEdgeCapReg         {0}
    set_instance_parameter_value    mipi_in_pio_1     bitModifyingOutReg            {0}
    set_instance_parameter_value    mipi_in_pio_1     captureEdge                   {0}
    set_instance_parameter_value    mipi_in_pio_1     direction                     {Input}
    set_instance_parameter_value    mipi_in_pio_1     edgeType                      {RISING}
    set_instance_parameter_value    mipi_in_pio_1     generateIRQ                   {0}
    set_instance_parameter_value    mipi_in_pio_1     irqType                       {LEVEL}
    set_instance_parameter_value    mipi_in_pio_1     resetValue                    {0.0}
    set_instance_parameter_value    mipi_in_pio_1     simDoTestBenchWiring          {0}
    set_instance_parameter_value    mipi_in_pio_1     simDrivenValue                {0.0}
    set_instance_parameter_value    mipi_in_pio_1     width                         {32}

    # mipi_in_mm_ccb
    set_instance_parameter_value    mipi_in_mm_ccb    ADDRESS_UNITS                 {SYMBOLS}
    set_instance_parameter_value    mipi_in_mm_ccb    ADDRESS_WIDTH                 {14}
    set_instance_parameter_value    mipi_in_mm_ccb    COMMAND_FIFO_DEPTH            {4}
    set_instance_parameter_value    mipi_in_mm_ccb    DATA_WIDTH                    {32}
    set_instance_parameter_value    mipi_in_mm_ccb    MASTER_SYNC_DEPTH             {2}
    set_instance_parameter_value    mipi_in_mm_ccb    MAX_BURST_SIZE                {1}
    set_instance_parameter_value    mipi_in_mm_ccb    RESPONSE_FIFO_DEPTH           {4}
    set_instance_parameter_value    mipi_in_mm_ccb    SLAVE_SYNC_DEPTH              {2}
    set_instance_parameter_value    mipi_in_mm_ccb    SYMBOL_WIDTH                  {8}
    set_instance_parameter_value    mipi_in_mm_ccb    SYNC_RESET                    {1}
    set_instance_parameter_value    mipi_in_mm_ccb    USE_AUTO_ADDRESS_WIDTH        {1}

    # mipi_in_mipi_dphy
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_ALT_CAL_EN_0                        {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_ALT_CAL_LEN                         {65536}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_BIT_RATE_MBPS_RNG_0                 {1782.0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_BYTE_LOC_0                          {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_CONTINUOUS_CLK_0                    {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_CORE_CLK_DIV_0                      {8}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_DPHY_IP_ROLE_0                      {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_NUM_LANES_0                         {4}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_NUM_PLL                             {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PER_SKEW_CAL_EN_0                   {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PPI_WIDTH_USR_0                     {16}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PREAMBLE_EN_0                       {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_REF_CLK_FREQ_MHZ_0                  {100.0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_REF_CLK_FREQ_MHZ_1                  {20.0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_REF_CLK_IO_0                        {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_REF_CLK_IO_1                        {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_REF_CLK_IO_SHARE                    {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_AUTO_TYPE_0                      {2}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_BIT_RATE_MBPS_SEL_0              {64}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CAP_EQ_MODE_0                    {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_LOSS_DETECT_0                {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_LOSS_DETECT_0_AUTO_BOOL      {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_POST_0                       {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_SETTLE_0                     {7}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_SETTLE_0_AUTO_BOOL           {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_0_0           {48}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_1_0           {48}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_2_0           {48}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_3_0           {48}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_HS_SETTLE_0                      {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_HS_SETTLE_0_AUTO_BOOL            {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_INIT_0                           {12}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_INIT_0_AUTO_BOOL                 {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_PREP_TIME_TM_0                   {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TIMING_RW_0                      {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TM_CONTROL_RX_TM_EN_0            {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TM_CONTROL_RX_TM_LOOPBACK_MODE_0 {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RZQ_ID                              {1}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_SKEW_CAL_EN_0                       {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_SKEW_CAL_LEN                        {32768}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_SOURCE_PLL_0                        {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_TM_EN_0                             {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_TM_LOOPBACK_MODE_0                  {0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_VCO_FREQ_MHZ_0                      {600.0}
    set_instance_parameter_value    mipi_in_mipi_dphy     GUI_VCO_FREQ_MHZ_1                      {800.0}
    if {${v_multi_sensor}} {
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_ALT_CAL_EN_1                        {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_BIT_RATE_MBPS_RNG_1                 {1782.0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_BYTE_LOC_1                          {2}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_CONTINUOUS_CLK_1                    {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_CORE_CLK_DIV_1                      {8}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_DPHY_IP_ROLE_1                      {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_NUM_LANES_1                         {4}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PER_SKEW_CAL_EN_1                   {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PPI_WIDTH_USR_1                     {16}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_PREAMBLE_EN_1                       {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_AUTO_TYPE_1                      {2}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_BIT_RATE_MBPS_SEL_1              {64}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CAP_EQ_MODE_1                    {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_LOSS_DETECT_1                {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_LOSS_DETECT_1_AUTO_BOOL      {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_POST_1                       {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_SETTLE_1                     {7}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_CLK_SETTLE_1_AUTO_BOOL           {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_0_1           {48}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_1_1           {48}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_2_1           {48}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_DLANE_DESKEW_DELAY_3_1           {48}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_HS_SETTLE_1                      {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_HS_SETTLE_1_AUTO_BOOL            {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_INIT_1                           {12}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_INIT_1_AUTO_BOOL                 {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_PREP_TIME_TM_1                   {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TIMING_RW_1                      {1}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TM_CONTROL_RX_TM_EN_1            {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_RX_TM_CONTROL_RX_TM_LOOPBACK_MODE_1 {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_SKEW_CAL_EN_1                       {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_SOURCE_PLL_1                        {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_TM_EN_1                             {0}
        set_instance_parameter_value    mipi_in_mipi_dphy     GUI_TM_LOOPBACK_MODE_1                  {0}
    }

    # mipi_in_mipi_csi2_0
    if {${v_exp_fusion_en}} {
        set_instance_parameter_value    mipi_in_mipi_csi2_0     NUMBER_OF_VIDEO_STREAMING_INTERFACES    {2}
    } else {
        set_instance_parameter_value    mipi_in_mipi_csi2_0     NUMBER_OF_VIDEO_STREAMING_INTERFACES    {1}
    }
    set_instance_parameter_value    mipi_in_mipi_csi2_0     BUFFER_DEPTH                      {128}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     BITS_PER_LANE                     ${v_mipi_bps}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     DIRECTION                         {rx}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     ENABLE_ED_FILESET_SIM             {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     ENABLE_ED_FILESET_SYNTHESIS       {1}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     ENABLE_SCRAMBLING                 {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     LANE                              {4}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     PIXELS_IN_PARALLEL                ${v_mipi_pip}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SELECT_CUSTOM_DEVICE              {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SELECT_ED_FILESET                 {VERILOG}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SELECT_SUPPORTED_VARIANT          {1}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SELECT_TARGETED_BOARD             {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_LEGACY_YUV420_8B          {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW10                     {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW12                     {1}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW14                     {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW16                     {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW20                     {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW24                     {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW6                      {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW7                      {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RAW8                      {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RGB444                    {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RGB555                    {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RGB565                    {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RGB666                    {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_RGB888                    {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_YUV420_10B                {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_YUV420_8B                 {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_YUV422_10B                {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     SUPPORT_YUV422_8B                 {0}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     USE_CONTINUOUS_CLK                ${v_continuous_clk}
    set_instance_parameter_value    mipi_in_mipi_csi2_0     VIDEO_INTERFACE_MODE              {simple}

    # mipi_in_snoop_0_0
    set_instance_parameter_value    mipi_in_snoop_0_0     BPS                           ${v_bps}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_TYPE               {584}
    set_instance_parameter_value    mipi_in_snoop_0_0     C_OMNI_CAP_VERSION            {1}
    set_instance_parameter_value    mipi_in_snoop_0_0     EXTERNAL_MODE                 {0}
    set_instance_parameter_value    mipi_in_snoop_0_0     NUMBER_OF_COLOR_PLANES        ${v_cppp}
    set_instance_parameter_value    mipi_in_snoop_0_0     PIPELINE_READY                ${v_pipeline_ready}
    set_instance_parameter_value    mipi_in_snoop_0_0     PIXELS_IN_PARALLEL            ${v_mipi_pip}
    set_instance_parameter_value    mipi_in_snoop_0_0     SEPARATE_SLAVE_CLOCK          {1}
    set_instance_parameter_value    mipi_in_snoop_0_0     SLAVE_PROTOCOL                {Avalon}

    # mipi_in_proto_conv_0_0
    set_instance_parameter_value    mipi_in_proto_conv_0_0     BPS                         ${v_bps}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     CHROMA_SAMPLING             {444}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     CHROMA_SITING               {TOP_LEFT}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     CLIP_LONG_FIELDS            {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     COLOR_SPACE                 {RGB}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_TYPE             {573}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     ENABLE_TIMEOUT              {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     ENABLE_YCBCR_SWAP           {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     INPUT_MODE                  {INTERNAL}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     OUTPUT_MODE                 {EXTERNAL}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     PIXELS_IN_PARALLEL          ${v_mipi_pip}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     RUNTIME_CONTROL             {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     SEPARATE_SLAVE_CLOCK        {0}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     VIP_USER_SUPPORT            {DISCARD}
    set_instance_parameter_value    mipi_in_proto_conv_0_0     VVP_USER_SUPPORT            {NONE_ALLOWED}

    if {${v_exp_fusion_en}} {
        # mipi_in_proto_conv_0_1
        set_instance_parameter_value    mipi_in_proto_conv_0_1     BPS                          ${v_bps}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     CHROMA_SAMPLING              {444}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     CHROMA_SITING                {TOP_LEFT}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     CLIP_LONG_FIELDS             {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     COLOR_SPACE                  {RGB}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_ID_ASSOCIATED     {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_ID_COMPONENT      [expr ${v_inst_id} + 2]
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_IRQ               {255}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_IRQ_ENABLE        {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_IRQ_ENABLE_EN     {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_IRQ_STATUS        {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_IRQ_STATUS_EN     {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_TAG               {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_TYPE              {573}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     C_OMNI_CAP_VERSION           {1}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     ENABLE_DEBUG                 ${v_enable_debug}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     ENABLE_TIMEOUT               {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     ENABLE_YCBCR_SWAP            {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     INPUT_MODE                   {INTERNAL}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     NUMBER_OF_COLOR_PLANES       ${v_cppp}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     OUTPUT_MODE                  {EXTERNAL}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     PIPELINE_READY               ${v_pipeline_ready}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     PIXELS_IN_PARALLEL           ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     RUNTIME_CONTROL              {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     SEPARATE_SLAVE_CLOCK         {0}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     SLAVE_PROTOCOL               {Avalon}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     VIP_USER_SUPPORT             {DISCARD}
        set_instance_parameter_value    mipi_in_proto_conv_0_1     VVP_USER_SUPPORT             {NONE_ALLOWED}

        # mipi_in_exp_fusion_0
        set_instance_parameter_value    mipi_in_exp_fusion_0       BPS_IN                       ${v_bps}
        set_instance_parameter_value    mipi_in_exp_fusion_0       BPS_OUT                      ${v_hdr_bps}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_CPU_OFFSET                 {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_ID_ASSOCIATED     {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_ID_COMPONENT      ${v_inst_id}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_IRQ               {255}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_IRQ_ENABLE        {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_IRQ_ENABLE_EN     {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_IRQ_STATUS        {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_IRQ_STATUS_EN     {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_SIZE              {64}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_TAG               {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_TYPE              {597}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OMNI_CAP_VERSION           {1}
        set_instance_parameter_value    mipi_in_exp_fusion_0       NUMBER_OF_COLOR_PLANES       ${v_cppp}
        set_instance_parameter_value    mipi_in_exp_fusion_0       PIXELS_IN_PARALLEL           ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_exp_fusion_0       RUNTIME_CONTROL              {1}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_OUTPUT_MODE                {0}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_BLACK_LEVEL                {200}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_EXPOSURE_RATIO             {10802}
        set_instance_parameter_value    mipi_in_exp_fusion_0       C_THRESHOLD                  {4177}
    }

    # mipi_in_pip_conv_0
    if {${v_exp_fusion_en}} {
        set_instance_parameter_value    mipi_in_pip_conv_0      BPS             ${v_hdr_bps}
    } else {
        set_instance_parameter_value    mipi_in_pip_conv_0      BPS             ${v_bps}
    }
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_ID_ASSOCIATED      {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_ID_COMPONENT       ${v_inst_id}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_IRQ                {255}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_IRQ_ENABLE         {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_IRQ_ENABLE_EN      {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_IRQ_STATUS         {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_IRQ_STATUS_EN      {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_TAG                {0}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_TYPE               {569}
    set_instance_parameter_value    mipi_in_pip_conv_0      C_OMNI_CAP_VERSION            {1}
    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        set_instance_parameter_value    mipi_in_pip_conv_0      DUAL_CLOCK      {1}
    } else {
        set_instance_parameter_value    mipi_in_pip_conv_0      DUAL_CLOCK      {0}
    }
    set_instance_parameter_value    mipi_in_pip_conv_0      FIFO_DEPTH                  {2048}
    set_instance_parameter_value    mipi_in_pip_conv_0      ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    mipi_in_pip_conv_0      EXTERNAL_MODE               {1}
    set_instance_parameter_value    mipi_in_pip_conv_0      NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    mipi_in_pip_conv_0      PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    mipi_in_pip_conv_0      PIXELS_IN_PARALLEL_IN       ${v_mipi_pip}
    set_instance_parameter_value    mipi_in_pip_conv_0      PIXELS_IN_PARALLEL_OUT      ${v_pip}
    set_instance_parameter_value    mipi_in_pip_conv_0      SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    mipi_in_pip_conv_0      SLAVE_PROTOCOL              {Avalon}

    if {${v_multi_sensor}} {
        # mipi_in_mipi_csi2_1
        if {${v_exp_fusion_en}} {
            set_instance_parameter_value    mipi_in_mipi_csi2_1   NUMBER_OF_VIDEO_STREAMING_INTERFACES  {2}
        } else {
            set_instance_parameter_value    mipi_in_mipi_csi2_1   NUMBER_OF_VIDEO_STREAMING_INTERFACES  {1}
        }
        set_instance_parameter_value    mipi_in_mipi_csi2_1     BUFFER_DEPTH                    {128}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     BITS_PER_LANE                   ${v_mipi_bps}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     DIRECTION                       {rx}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     ENABLE_ED_FILESET_SIM           {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     ENABLE_ED_FILESET_SYNTHESIS     {1}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     ENABLE_SCRAMBLING               {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     LANE                            {4}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     PIXELS_IN_PARALLEL              ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SELECT_CUSTOM_DEVICE            {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SELECT_ED_FILESET               {VERILOG}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SELECT_SUPPORTED_VARIANT        {1}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SELECT_TARGETED_BOARD           {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_LEGACY_YUV420_8B        {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW10                   {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW12                   {1}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW14                   {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW16                   {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW20                   {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW24                   {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW6                    {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW7                    {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RAW8                    {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RGB444                  {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RGB555                  {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RGB565                  {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RGB666                  {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_RGB888                  {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_YUV420_10B              {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_YUV420_8B               {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_YUV422_10B              {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     SUPPORT_YUV422_8B               {0}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     USE_CONTINUOUS_CLK              ${v_continuous_clk}
        set_instance_parameter_value    mipi_in_mipi_csi2_1     VIDEO_INTERFACE_MODE            {simple}

        # mipi_in_snoop_1_0
        set_instance_parameter_value    mipi_in_snoop_1_0       BPS                             ${v_bps}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_ID_ASSOCIATED        {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_ID_COMPONENT         [expr ${v_inst_id} + 1]
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_IRQ                  {255}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_IRQ_ENABLE           {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_IRQ_ENABLE_EN        {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_IRQ_STATUS           {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_IRQ_STATUS_EN        {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_TAG                  {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_TYPE                 {584}
        set_instance_parameter_value    mipi_in_snoop_1_0       C_OMNI_CAP_VERSION              {1}
        set_instance_parameter_value    mipi_in_snoop_1_0       EXTERNAL_MODE                   {0}
        set_instance_parameter_value    mipi_in_snoop_1_0       NUMBER_OF_COLOR_PLANES          ${v_cppp}
        set_instance_parameter_value    mipi_in_snoop_1_0       PIPELINE_READY                  ${v_pipeline_ready}
        set_instance_parameter_value    mipi_in_snoop_1_0       PIXELS_IN_PARALLEL              ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_snoop_1_0       SEPARATE_SLAVE_CLOCK            {1}
        set_instance_parameter_value    mipi_in_snoop_1_0       SLAVE_PROTOCOL                  {Avalon}

        # mipi_in_proto_conv_1_0
        set_instance_parameter_value    mipi_in_proto_conv_1_0      BPS                         ${v_bps}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      CHROMA_SAMPLING             {444}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      CHROMA_SITING               {TOP_LEFT}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      CLIP_LONG_FIELDS            {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      COLOR_SPACE                 {RGB}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_ID_COMPONENT     [expr ${v_inst_id} + 1]
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_TYPE             {573}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      ENABLE_TIMEOUT              {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      ENABLE_YCBCR_SWAP           {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      INPUT_MODE                  {INTERNAL}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      NUMBER_OF_COLOR_PLANES      ${v_cppp}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      OUTPUT_MODE                 {EXTERNAL}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      PIPELINE_READY              ${v_pipeline_ready}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      PIXELS_IN_PARALLEL          ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      RUNTIME_CONTROL             {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      SEPARATE_SLAVE_CLOCK        {0}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      SLAVE_PROTOCOL              {Avalon}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      VIP_USER_SUPPORT            {DISCARD}
        set_instance_parameter_value    mipi_in_proto_conv_1_0      VVP_USER_SUPPORT            {NONE_ALLOWED}

        if {${v_exp_fusion_en}} {
            # mipi_in_proto_conv_1_1
            set_instance_parameter_value    mipi_in_proto_conv_1_1    BPS                         ${v_bps}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    CHROMA_SAMPLING             {444}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    CHROMA_SITING               {TOP_LEFT}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    CLIP_LONG_FIELDS            {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    COLOR_SPACE                 {RGB}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_ID_ASSOCIATED    {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_ID_COMPONENT     [expr ${v_inst_id} + 3]
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_IRQ              {255}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_IRQ_ENABLE       {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_IRQ_ENABLE_EN    {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_IRQ_STATUS       {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_IRQ_STATUS_EN    {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_TAG              {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_TYPE             {573}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    C_OMNI_CAP_VERSION          {1}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    ENABLE_DEBUG                ${v_enable_debug}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    ENABLE_TIMEOUT              {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    ENABLE_YCBCR_SWAP           {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    INPUT_MODE                  {INTERNAL}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    NUMBER_OF_COLOR_PLANES      ${v_cppp}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    OUTPUT_MODE                 {EXTERNAL}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    PIPELINE_READY              ${v_pipeline_ready}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    PIXELS_IN_PARALLEL          ${v_mipi_pip}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    RUNTIME_CONTROL             {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    SEPARATE_SLAVE_CLOCK        {0}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    SLAVE_PROTOCOL              {Avalon}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    VIP_USER_SUPPORT            {DISCARD}
            set_instance_parameter_value    mipi_in_proto_conv_1_1    VVP_USER_SUPPORT            {NONE_ALLOWED}

            # mipi_in_exp_fusion_1
            set_instance_parameter_value    mipi_in_exp_fusion_1      BPS_IN                        ${v_bps}
            set_instance_parameter_value    mipi_in_exp_fusion_1      BPS_OUT                       ${v_hdr_bps}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_CPU_OFFSET                  {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_ID_ASSOCIATED      {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_IRQ                {255}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_IRQ_ENABLE         {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_IRQ_ENABLE_EN      {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_IRQ_STATUS         {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_IRQ_STATUS_EN      {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_SIZE               {64}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_TAG                {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_TYPE               {597}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OMNI_CAP_VERSION            {1}
            set_instance_parameter_value    mipi_in_exp_fusion_1      NUMBER_OF_COLOR_PLANES        ${v_cppp}
            set_instance_parameter_value    mipi_in_exp_fusion_1      PIXELS_IN_PARALLEL            ${v_mipi_pip}
            set_instance_parameter_value    mipi_in_exp_fusion_1      RUNTIME_CONTROL               {1}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_OUTPUT_MODE                 {0}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_BLACK_LEVEL                 {200}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_EXPOSURE_RATIO              {10802}
            set_instance_parameter_value    mipi_in_exp_fusion_1      C_THRESHOLD                   {4177}
        }

        # mipi_in_pip_conv_1
        if {${v_exp_fusion_en}} {
            set_instance_parameter_value    mipi_in_pip_conv_1      BPS           ${v_hdr_bps}
        } else {
            set_instance_parameter_value    mipi_in_pip_conv_1      BPS           ${v_bps}
        }
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_ID_ASSOCIATED      {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_ID_COMPONENT       [expr ${v_inst_id} + 1]
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_IRQ                {255}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_IRQ_ENABLE         {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_IRQ_ENABLE_EN      {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_IRQ_STATUS         {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_IRQ_STATUS_EN      {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_TAG                {0}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_TYPE               {569}
        set_instance_parameter_value    mipi_in_pip_conv_1      C_OMNI_CAP_VERSION            {1}
        if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
              set_instance_parameter_value    mipi_in_pip_conv_1      DUAL_CLOCK                  {1}
        } else {
              set_instance_parameter_value    mipi_in_pip_conv_1      DUAL_CLOCK                  {0}
        }
        set_instance_parameter_value    mipi_in_pip_conv_1      FIFO_DEPTH                    {2048}
        set_instance_parameter_value    mipi_in_pip_conv_1      ENABLE_DEBUG                  ${v_enable_debug}
        set_instance_parameter_value    mipi_in_pip_conv_1      EXTERNAL_MODE                 {1}
        set_instance_parameter_value    mipi_in_pip_conv_1      NUMBER_OF_COLOR_PLANES        ${v_cppp}
        set_instance_parameter_value    mipi_in_pip_conv_1      PIPELINE_READY                ${v_pipeline_ready}
        set_instance_parameter_value    mipi_in_pip_conv_1      PIXELS_IN_PARALLEL_IN         ${v_mipi_pip}
        set_instance_parameter_value    mipi_in_pip_conv_1      PIXELS_IN_PARALLEL_OUT        ${v_pip}
        set_instance_parameter_value    mipi_in_pip_conv_1      SEPARATE_SLAVE_CLOCK          {1}
        set_instance_parameter_value    mipi_in_pip_conv_1      SLAVE_PROTOCOL                {Avalon}
    }


    ############################
    #### Create Connections ####
    ############################

    # mipi_in_cpu_clk_bridge
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_cpu_rst_bridge.clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_mm_bridge.clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_mm_ccb.s0_clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_mipi_dphy.reg_clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_pio_0.clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_pio_1.clk
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_snoop_0_0.agent_clock
    if {${v_exp_fusion_en}} {
        add_connection         mipi_in_cpu_clk_bridge.out_clk         mipi_in_exp_fusion_0.agent_clock
    }
    add_connection         mipi_in_cpu_clk_bridge.out_clk           mipi_in_pip_conv_0.agent_clock
    if {${v_multi_sensor}} {
        add_connection         mipi_in_cpu_clk_bridge.out_clk         mipi_in_snoop_1_0.agent_clock
        if {${v_exp_fusion_en}} {
            add_connection         mipi_in_cpu_clk_bridge.out_clk       mipi_in_exp_fusion_1.agent_clock
        }
        add_connection         mipi_in_cpu_clk_bridge.out_clk         mipi_in_pip_conv_1.agent_clock
    }

    # mipi_in_cpu_rst_bridge
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_mm_bridge.reset
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_mm_ccb.s0_reset
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_mipi_dphy.reg_srst
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_mipi_dphy.arst
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_pio_0.reset
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_pio_1.reset
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_snoop_0_0.agent_reset
    if {${v_exp_fusion_en}} {
        add_connection         mipi_in_cpu_rst_bridge.out_reset       mipi_in_exp_fusion_0.agent_reset
    }
    add_connection         mipi_in_cpu_rst_bridge.out_reset         mipi_in_pip_conv_0.agent_reset
    if {${v_multi_sensor}} {
        add_connection         mipi_in_cpu_rst_bridge.out_reset       mipi_in_snoop_1_0.agent_reset
        if {${v_exp_fusion_en}} {
            add_connection         mipi_in_cpu_rst_bridge.out_reset     mipi_in_exp_fusion_1.agent_reset
        }
        add_connection         mipi_in_cpu_rst_bridge.out_reset       mipi_in_pip_conv_1.agent_reset
    }

    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        # mipi_in_mipi_clk_bridge
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_mipi_rst_bridge.clk
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_mm_ccb.m0_clk
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_mipi_csi2_0.axi4s_clk
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_snoop_0_0.main_clock
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_proto_conv_0_0.main_clock
        if {${v_exp_fusion_en}} {
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_proto_conv_0_1.main_clock
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_exp_fusion_0.main_clock
        }
        add_connection          mipi_in_mipi_clk_bridge.out_clk       mipi_in_pip_conv_0.in_clock
        if {${v_multi_sensor}} {
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_mipi_csi2_1.axi4s_clk
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_snoop_1_0.main_clock
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_proto_conv_1_0.main_clock
            if {${v_exp_fusion_en}} {
                add_connection      mipi_in_mipi_clk_bridge.out_clk       mipi_in_proto_conv_1_1.main_clock
                add_connection      mipi_in_mipi_clk_bridge.out_clk       mipi_in_exp_fusion_1.main_clock
            }
            add_connection        mipi_in_mipi_clk_bridge.out_clk       mipi_in_pip_conv_1.in_clock
        }

        # mipi_in_mipi_rst_bridge
        add_connection          mipi_in_mipi_rst_bridge.out_reset       mipi_in_mm_ccb.m0_reset
        add_connection          mipi_in_mipi_rst_bridge.out_reset       mipi_in_mipi_csi2_0.axi4s_rst
        add_connection          mipi_in_mipi_rst_bridge.out_reset       mipi_in_snoop_0_0.main_reset
        add_connection          mipi_in_mipi_rst_bridge.out_reset       mipi_in_proto_conv_0_0.main_reset
        if {${v_exp_fusion_en}} {
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_proto_conv_0_1.main_reset
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_exp_fusion_0.main_reset
        }
        add_connection          mipi_in_mipi_rst_bridge.out_reset       mipi_in_pip_conv_0.in_reset
        if {${v_multi_sensor}} {
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_mipi_csi2_1.axi4s_rst
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_snoop_1_0.main_reset
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_proto_conv_1_0.main_reset
            if {${v_exp_fusion_en}} {
                add_connection      mipi_in_mipi_rst_bridge.out_reset       mipi_in_proto_conv_1_1.main_reset
                add_connection      mipi_in_mipi_rst_bridge.out_reset       mipi_in_exp_fusion_1.main_reset
            }
            add_connection        mipi_in_mipi_rst_bridge.out_reset       mipi_in_pip_conv_1.in_reset
        }

        # mipi_in_vid_clk_bridge
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_vid_rst_bridge.clk
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_pip_conv_0.out_clock
        if {${v_multi_sensor}} {
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_pip_conv_1.out_clock
        }

        # mipi_in_vid_rst_bridge
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_mm_ccb.m0_reset
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_pip_conv_0.out_reset
        if {${v_multi_sensor}} {
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_pip_conv_1.out_reset
        }

    } else {
        # mipi_in_vid_clk_bridge
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_vid_rst_bridge.clk
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_mm_ccb.m0_clk
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_mipi_csi2_0.axi4s_clk
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_snoop_0_0.main_clock
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_proto_conv_0_0.main_clock
        if {${v_exp_fusion_en}} {
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_proto_conv_0_1.main_clock
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_exp_fusion_0.main_clock
        }
        add_connection          mipi_in_vid_clk_bridge.out_clk          mipi_in_pip_conv_0.main_clock
        if {${v_multi_sensor}} {
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_mipi_csi2_1.axi4s_clk
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_snoop_1_0.main_clock
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_proto_conv_1_0.main_clock
            if {${v_exp_fusion_en}} {
                add_connection      mipi_in_vid_clk_bridge.out_clk          mipi_in_proto_conv_1_1.main_clock
                add_connection      mipi_in_vid_clk_bridge.out_clk          mipi_in_exp_fusion_1.main_clock
            }
            add_connection        mipi_in_vid_clk_bridge.out_clk          mipi_in_pip_conv_1.main_clock
        }

        # mipi_in_vid_rst_bridge
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_mm_ccb.m0_reset
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_mipi_csi2_0.axi4s_rst
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_snoop_0_0.main_reset
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_proto_conv_0_0.main_reset
        if {${v_exp_fusion_en}} {
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_proto_conv_0_1.main_reset
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_exp_fusion_0.main_reset
        }
        add_connection          mipi_in_vid_rst_bridge.out_reset        mipi_in_pip_conv_0.main_reset
        if {${v_multi_sensor}} {
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_mipi_csi2_1.axi4s_rst
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_snoop_1_0.main_reset
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_proto_conv_1_0.main_reset
            if {${v_exp_fusion_en}} {
                add_connection      mipi_in_vid_rst_bridge.out_reset        mipi_in_proto_conv_1_1.main_reset
                add_connection      mipi_in_vid_rst_bridge.out_reset        mipi_in_exp_fusion_1.main_reset
            }
            add_connection        mipi_in_vid_rst_bridge.out_reset        mipi_in_pip_conv_1.main_reset
        }
    }

    # mipi_in_mm_bridge
    add_connection         mipi_in_mm_bridge.m0           mipi_in_mm_ccb.s0
    add_connection         mipi_in_mm_bridge.m0           mipi_in_mipi_dphy.axi_lite
    add_connection         mipi_in_mm_bridge.m0           mipi_in_pio_0.s1
    add_connection         mipi_in_mm_bridge.m0           mipi_in_pio_1.s1
    add_connection         mipi_in_mm_bridge.m0           mipi_in_snoop_0_0.av_mm_control_agent
    if {${v_exp_fusion_en}} {
        add_connection       mipi_in_mm_bridge.m0           mipi_in_exp_fusion_0.av_mm_control_agent
    }
    add_connection         mipi_in_mm_bridge.m0           mipi_in_pip_conv_0.av_mm_control_agent
    if {${v_multi_sensor}} {
        add_connection       mipi_in_mm_bridge.m0           mipi_in_snoop_1_0.av_mm_control_agent
        if {${v_exp_fusion_en}} {
            add_connection     mipi_in_mm_bridge.m0           mipi_in_exp_fusion_1.av_mm_control_agent
        }
        add_connection       mipi_in_mm_bridge.m0           mipi_in_pip_conv_1.av_mm_control_agent
    }

    # mipi_in_mm_ccb
    add_connection         mipi_in_mm_ccb.m0        mipi_in_mipi_csi2_0.avalon_mm_control_interface
    if {${v_multi_sensor}} {
        add_connection       mipi_in_mm_ccb.m0        mipi_in_mipi_csi2_1.avalon_mm_control_interface
    }

    # mipi_in_mipi_dphy
    add_connection      mipi_in_mipi_dphy.LINK0_D0_ppi_rx_hs_clk       mipi_in_mipi_csi2_0.d0_ppi_hs_clk
    add_connection      mipi_in_mipi_dphy.LINK0_D0_ppi_rx_hs_srst      mipi_in_mipi_csi2_0.d0_ppi_rx_hs_srst
    add_connection      mipi_in_mipi_dphy.LINK0_D0_ppi_ctrl            mipi_in_mipi_csi2_0.d0_ppi_ctrl

    add_connection      mipi_in_mipi_dphy.LINK0_D1_ppi_rx_hs_clk       mipi_in_mipi_csi2_0.d1_ppi_hs_clk
    add_connection      mipi_in_mipi_dphy.LINK0_D1_ppi_rx_hs_srst      mipi_in_mipi_csi2_0.d1_ppi_rx_hs_srst
    add_connection      mipi_in_mipi_dphy.LINK0_D1_ppi_ctrl            mipi_in_mipi_csi2_0.d1_ppi_ctrl

    add_connection      mipi_in_mipi_dphy.LINK0_D2_ppi_rx_hs_clk       mipi_in_mipi_csi2_0.d2_ppi_hs_clk
    add_connection      mipi_in_mipi_dphy.LINK0_D2_ppi_rx_hs_srst      mipi_in_mipi_csi2_0.d2_ppi_rx_hs_srst
    add_connection      mipi_in_mipi_dphy.LINK0_D2_ppi_ctrl            mipi_in_mipi_csi2_0.d2_ppi_ctrl

    add_connection      mipi_in_mipi_dphy.LINK0_D3_ppi_rx_hs_clk       mipi_in_mipi_csi2_0.d3_ppi_hs_clk
    add_connection      mipi_in_mipi_dphy.LINK0_D3_ppi_rx_hs_srst      mipi_in_mipi_csi2_0.d3_ppi_rx_hs_srst
    add_connection      mipi_in_mipi_dphy.LINK0_D3_ppi_ctrl            mipi_in_mipi_csi2_0.d3_ppi_ctrl

    add_connection      mipi_in_mipi_dphy.LINK0_CK_ppi_rx_hs_clk       mipi_in_mipi_csi2_0.ck_ppi_hs_clk
    add_connection      mipi_in_mipi_dphy.LINK0_CK_ppi_rx_hs_srst      mipi_in_mipi_csi2_0.ck_ppi_rx_hs_srst
    add_connection      mipi_in_mipi_dphy.LINK0_CK_ppi_ctrl            mipi_in_mipi_csi2_0.ck_ppi_ctrl

    if {${v_multi_sensor}} {
        add_connection    mipi_in_mipi_dphy.LINK1_D0_ppi_rx_hs_clk        mipi_in_mipi_csi2_1.d0_ppi_hs_clk
        add_connection    mipi_in_mipi_dphy.LINK1_D0_ppi_rx_hs_srst       mipi_in_mipi_csi2_1.d0_ppi_rx_hs_srst
        add_connection    mipi_in_mipi_dphy.LINK1_D0_ppi_ctrl             mipi_in_mipi_csi2_1.d0_ppi_ctrl

        add_connection    mipi_in_mipi_dphy.LINK1_D1_ppi_rx_hs_clk        mipi_in_mipi_csi2_1.d1_ppi_hs_clk
        add_connection    mipi_in_mipi_dphy.LINK1_D1_ppi_rx_hs_srst       mipi_in_mipi_csi2_1.d1_ppi_rx_hs_srst
        add_connection    mipi_in_mipi_dphy.LINK1_D1_ppi_ctrl             mipi_in_mipi_csi2_1.d1_ppi_ctrl

        add_connection    mipi_in_mipi_dphy.LINK1_D2_ppi_rx_hs_clk        mipi_in_mipi_csi2_1.d2_ppi_hs_clk
        add_connection    mipi_in_mipi_dphy.LINK1_D2_ppi_rx_hs_srst       mipi_in_mipi_csi2_1.d2_ppi_rx_hs_srst
        add_connection    mipi_in_mipi_dphy.LINK1_D2_ppi_ctrl             mipi_in_mipi_csi2_1.d2_ppi_ctrl

        add_connection    mipi_in_mipi_dphy.LINK1_D3_ppi_rx_hs_clk        mipi_in_mipi_csi2_1.d3_ppi_hs_clk
        add_connection    mipi_in_mipi_dphy.LINK1_D3_ppi_rx_hs_srst       mipi_in_mipi_csi2_1.d3_ppi_rx_hs_srst
        add_connection    mipi_in_mipi_dphy.LINK1_D3_ppi_ctrl             mipi_in_mipi_csi2_1.d3_ppi_ctrl

        add_connection    mipi_in_mipi_dphy.LINK1_CK_ppi_rx_hs_clk        mipi_in_mipi_csi2_1.ck_ppi_hs_clk
        add_connection    mipi_in_mipi_dphy.LINK1_CK_ppi_rx_hs_srst       mipi_in_mipi_csi2_1.ck_ppi_rx_hs_srst
        add_connection    mipi_in_mipi_dphy.LINK1_CK_ppi_ctrl             mipi_in_mipi_csi2_1.ck_ppi_ctrl
    }

    # mipi_in_mipi_csi2_0
    add_connection      mipi_in_mipi_csi2_0.d0_ppi_hs            mipi_in_mipi_dphy.LINK0_D0_ppi_rx_hs
    add_connection      mipi_in_mipi_csi2_0.d0_ppi_lp            mipi_in_mipi_dphy.LINK0_D0_ppi_rx_lp
    add_connection      mipi_in_mipi_csi2_0.i_d0_ppi_rx_err      mipi_in_mipi_dphy.LINK0_D0_ppi_rx_err

    add_connection      mipi_in_mipi_csi2_0.d1_ppi_hs            mipi_in_mipi_dphy.LINK0_D1_ppi_rx_hs
    add_connection      mipi_in_mipi_csi2_0.d1_ppi_lp            mipi_in_mipi_dphy.LINK0_D1_ppi_rx_lp
    add_connection      mipi_in_mipi_csi2_0.i_d1_ppi_rx_err      mipi_in_mipi_dphy.LINK0_D1_ppi_rx_err

    add_connection      mipi_in_mipi_csi2_0.d2_ppi_hs            mipi_in_mipi_dphy.LINK0_D2_ppi_rx_hs
    add_connection      mipi_in_mipi_csi2_0.d2_ppi_lp            mipi_in_mipi_dphy.LINK0_D2_ppi_rx_lp
    add_connection      mipi_in_mipi_csi2_0.i_d2_ppi_rx_err      mipi_in_mipi_dphy.LINK0_D2_ppi_rx_err

    add_connection      mipi_in_mipi_csi2_0.d3_ppi_hs            mipi_in_mipi_dphy.LINK0_D3_ppi_rx_hs
    add_connection      mipi_in_mipi_csi2_0.d3_ppi_lp            mipi_in_mipi_dphy.LINK0_D3_ppi_rx_lp
    add_connection      mipi_in_mipi_csi2_0.i_d3_ppi_rx_err      mipi_in_mipi_dphy.LINK0_D3_ppi_rx_err

    add_connection      mipi_in_mipi_csi2_0.ck_ppi_hs            mipi_in_mipi_dphy.LINK0_CK_ppi_rx_hs
    add_connection      mipi_in_mipi_csi2_0.ck_ppi_lp            mipi_in_mipi_dphy.LINK0_CK_ppi_rx_lp
    add_connection      mipi_in_mipi_csi2_0.i_ck_ppi_rx_err      mipi_in_mipi_dphy.LINK0_CK_ppi_rx_err

    add_connection      mipi_in_mipi_csi2_0.video_streaming_interface_0       mipi_in_snoop_0_0.axi4s_vid_in
    if {${v_exp_fusion_en}} {
        add_connection    mipi_in_mipi_csi2_0.video_streaming_interface_1       mipi_in_proto_conv_0_1.axi4s_vid_in
    }

    # mipi_in_snoop_0_0
    add_connection         mipi_in_snoop_0_0.axi4s_vid_out           mipi_in_proto_conv_0_0.axi4s_vid_in

    # mipi_in_proto_conv_0_0
    if {${v_exp_fusion_en}} {
        add_connection         mipi_in_proto_conv_0_0.axi4s_vid_out       mipi_in_exp_fusion_0.axi4s_vid_in_0
        # mipi_in_exp_fusion_0
        add_connection         mipi_in_exp_fusion_0.axi4s_vid_out         mipi_in_pip_conv_0.axi4s_vid_in
    } else {
        add_connection         mipi_in_proto_conv_0_0.axi4s_vid_out       mipi_in_pip_conv_0.axi4s_vid_in
    }

    # mipi_in_proto_conv_0_1
    if {${v_exp_fusion_en}} {
        add_connection         mipi_in_proto_conv_0_1.axi4s_vid_out      mipi_in_exp_fusion_0.axi4s_vid_in_1
    }

    if {${v_multi_sensor}} {
        # mipi_in_mipi_csi2_1
        add_connection         mipi_in_mipi_csi2_1.d0_ppi_hs            mipi_in_mipi_dphy.LINK1_D0_ppi_rx_hs
        add_connection         mipi_in_mipi_csi2_1.d0_ppi_lp            mipi_in_mipi_dphy.LINK1_D0_ppi_rx_lp
        add_connection         mipi_in_mipi_csi2_1.i_d0_ppi_rx_err      mipi_in_mipi_dphy.LINK1_D0_ppi_rx_err

        add_connection         mipi_in_mipi_csi2_1.d1_ppi_hs            mipi_in_mipi_dphy.LINK1_D1_ppi_rx_hs
        add_connection         mipi_in_mipi_csi2_1.d1_ppi_lp            mipi_in_mipi_dphy.LINK1_D1_ppi_rx_lp
        add_connection         mipi_in_mipi_csi2_1.i_d1_ppi_rx_err      mipi_in_mipi_dphy.LINK1_D1_ppi_rx_err

        add_connection         mipi_in_mipi_csi2_1.d2_ppi_hs            mipi_in_mipi_dphy.LINK1_D2_ppi_rx_hs
        add_connection         mipi_in_mipi_csi2_1.d2_ppi_lp            mipi_in_mipi_dphy.LINK1_D2_ppi_rx_lp
        add_connection         mipi_in_mipi_csi2_1.i_d2_ppi_rx_err      mipi_in_mipi_dphy.LINK1_D2_ppi_rx_err

        add_connection         mipi_in_mipi_csi2_1.d3_ppi_hs            mipi_in_mipi_dphy.LINK1_D3_ppi_rx_hs
        add_connection         mipi_in_mipi_csi2_1.d3_ppi_lp            mipi_in_mipi_dphy.LINK1_D3_ppi_rx_lp
        add_connection         mipi_in_mipi_csi2_1.i_d3_ppi_rx_err      mipi_in_mipi_dphy.LINK1_D3_ppi_rx_err

        add_connection         mipi_in_mipi_csi2_1.ck_ppi_hs            mipi_in_mipi_dphy.LINK1_CK_ppi_rx_hs
        add_connection         mipi_in_mipi_csi2_1.ck_ppi_lp            mipi_in_mipi_dphy.LINK1_CK_ppi_rx_lp
        add_connection         mipi_in_mipi_csi2_1.i_ck_ppi_rx_err      mipi_in_mipi_dphy.LINK1_CK_ppi_rx_err

        add_connection      mipi_in_mipi_csi2_1.video_streaming_interface_0    mipi_in_snoop_1_0.axi4s_vid_in
        if {${v_exp_fusion_en}} {
            add_connection    mipi_in_mipi_csi2_1.video_streaming_interface_1    mipi_in_proto_conv_1_1.axi4s_vid_in
        }

        # mipi_in_snoop_1_0
        add_connection         mipi_in_snoop_1_0.axi4s_vid_out           mipi_in_proto_conv_1_0.axi4s_vid_in

        # mipi_in_proto_conv_1_0
        if {${v_exp_fusion_en}} {
            add_connection         mipi_in_proto_conv_1_0.axi4s_vid_out      mipi_in_exp_fusion_1.axi4s_vid_in_0
            # mipi_in_exp_fusion_1
            add_connection         mipi_in_exp_fusion_1.axi4s_vid_out        mipi_in_pip_conv_1.axi4s_vid_in
        } else {
            add_connection         mipi_in_proto_conv_1_0.axi4s_vid_out      mipi_in_pip_conv_1.axi4s_vid_in
        }

        # mipi_in_proto_conv_1_1
        if {${v_exp_fusion_en}} {
            add_connection      mipi_in_proto_conv_1_1.axi4s_vid_out     mipi_in_exp_fusion_1.axi4s_vid_in_1
        }
    }


    ##########################
    ##### Create Exports #####
    ##########################

    # mipi_in_cpu_clk_bridge
    add_interface           cpu_clk_in    clock       sink
    set_interface_property  cpu_clk_in    EXPORT_OF   mipi_in_cpu_clk_bridge.in_clk

    # mipi_in_cpu_rst_bridge
    add_interface           cpu_rst_in    reset       sink
    set_interface_property  cpu_rst_in    EXPORT_OF   mipi_in_cpu_rst_bridge.in_reset

    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        # isp_in_mipi_clk_bridge
        set_interface_property  vid_mipi_clk_in    EXPORT_OF   isp_in_mipi_clk_bridge.in_clk

        # isp_in_mipi_rst_bridge
        set_interface_property  vid_mipi_rst_in    EXPORT_OF   isp_in_mipi_rst_bridge.in_reset
    }

    # mipi_in_vid_clk_bridge
    add_interface           vid_clk_in    clock       sink
    set_interface_property  vid_clk_in    EXPORT_OF   mipi_in_vid_clk_bridge.in_clk

    # mipi_in_vid_rst_bridge
    add_interface           vid_rst_in    reset       sink
    set_interface_property  vid_rst_in    EXPORT_OF   mipi_in_vid_rst_bridge.in_reset

    # mipi_in_mm_bridge
    add_interface           mm_ctrl_in    avalon      slave
    set_interface_property  mm_ctrl_in    EXPORT_OF   mipi_in_mm_bridge.s0

    # mipi_in_mipi_dphy
    add_interface           mipi_dphy_rzq                   conduit     end
    set_interface_property  mipi_dphy_rzq                   EXPORT_OF   mipi_in_mipi_dphy.rzq

    add_interface           mipi_dphy_ref_clk_0             clock       sink
    set_interface_property  mipi_dphy_ref_clk_0             EXPORT_OF   mipi_in_mipi_dphy.ref_clk_0

    add_interface           mipi_dphy_LINK0_dphy_io         conduit     end
    set_interface_property  mipi_dphy_LINK0_dphy_io         EXPORT_OF   mipi_in_mipi_dphy.LINK0_dphy_io

    if {${v_multi_sensor}} {
        add_interface           mipi_dphy_LINK1_dphy_io         conduit     end
        set_interface_property  mipi_dphy_LINK1_dphy_io         EXPORT_OF   mipi_in_mipi_dphy.LINK1_dphy_io
    }

    # mipi_in_pip_conv_0
    add_interface           mipi_csi2_0_m_vid_axis          axi4stream  manager
    set_interface_property  mipi_csi2_0_m_vid_axis          EXPORT_OF   mipi_in_pip_conv_0.axi4s_vid_out

    if {${v_multi_sensor}} {
        # mipi_in_pip_conv_1
        add_interface           mipi_csi2_1_m_vid_axis          axi4stream  manager
        set_interface_property  mipi_csi2_1_m_vid_axis          EXPORT_OF   mipi_in_pip_conv_1.axi4s_vid_out
    }

    # mipi_pio_0
    add_interface           mipi_pio_0         conduit     end
    set_interface_property  mipi_pio_0         EXPORT_OF   mipi_in_pio_0.external_connection

    # mipi_pio_1
    add_interface           mipi_pio_1         conduit     end
    set_interface_property  mipi_pio_1         EXPORT_OF   mipi_in_pio_1.external_connection


    #################################
    ##### Assign Base Addresses #####
    #################################

    set v_dphy_addr_offset            [get_shell_parameter DPHY_ADDR_OFFSET]
    set v_pio_0_addr_offset           [get_shell_parameter PIO_0_ADDR_OFFSET]
    set v_pio_1_addr_offset           [get_shell_parameter PIO_1_ADDR_OFFSET]
    set v_mm_ccb_addr_offset          [get_shell_parameter MM_CCB_ADDR_OFFSET]
    set v_snoop_0_addr_offset         [get_shell_parameter SNOOP_0_ADDR_OFFSET]
    set v_exp_fusion_0_addr_offset    [get_shell_parameter EXP_FUSION_0_ADDR_OFFSET]
    set v_pip_conv_0_addr_offset      [get_shell_parameter PIP_CONV_0_ADDR_OFFSET]
    set v_snoop_1_addr_offset         [get_shell_parameter SNOOP_1_ADDR_OFFSET]
    set v_exp_fusion_1_addr_offset    [get_shell_parameter EXP_FUSION_1_ADDR_OFFSET]
    set v_pip_conv_1_addr_offset      [get_shell_parameter PIP_CONV_1_ADDR_OFFSET]
    set v_csi_0_addr_offset           [get_shell_parameter CSI_0_ADDR_OFFSET]
    set v_csi_1_addr_offset           [get_shell_parameter CSI_1_ADDR_OFFSET]

    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_mipi_dphy.axi_lite \
                                                                  baseAddress ${v_dphy_addr_offset}
    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_pio_0.s1 \
                                                                  baseAddress ${v_pio_0_addr_offset}
    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_pio_1.s1 \
                                                                  baseAddress ${v_pio_1_addr_offset}
    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_mm_ccb.s0 \
                                                                  baseAddress ${v_mm_ccb_addr_offset}
    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_snoop_0_0.av_mm_control_agent \
                                                                  baseAddress ${v_snoop_0_addr_offset}
    if {${v_exp_fusion_en}} {
        set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_exp_fusion_0.av_mm_control_agent \
                                                                  baseAddress ${v_exp_fusion_0_addr_offset}
    }
    set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_pip_conv_0.av_mm_control_agent \
                                                                  baseAddress ${v_pip_conv_0_addr_offset}
    if {${v_multi_sensor}} {
        set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_snoop_1_0.av_mm_control_agent \
                                                                  baseAddress ${v_snoop_1_addr_offset}
        if {${v_exp_fusion_en}} {
            set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_exp_fusion_1.av_mm_control_agent \
                                                                  baseAddress ${v_exp_fusion_1_addr_offset}
        }
        set_connection_parameter_value mipi_in_mm_bridge.m0/mipi_in_pip_conv_1.av_mm_control_agent \
                                                                  baseAddress ${v_pip_conv_1_addr_offset}
    }

    set_connection_parameter_value mipi_in_mm_ccb.m0/mipi_in_mipi_csi2_0.avalon_mm_control_interface \
                                                                  baseAddress ${v_csi_0_addr_offset}
    if {${v_multi_sensor}} {
        set_connection_parameter_value mipi_in_mm_ccb.m0/mipi_in_mipi_csi2_1.avalon_mm_control_interface \
                                                                  baseAddress ${v_csi_1_addr_offset}
    }

    lock_avalon_base_address  mipi_in_mipi_dphy.axi_lite
    lock_avalon_base_address  mipi_in_pio_0.s1
    lock_avalon_base_address  mipi_in_pio_1.s1
    lock_avalon_base_address  mipi_in_mm_ccb.s0
    lock_avalon_base_address  mipi_in_snoop_0_0.av_mm_control_agent
    if {${v_exp_fusion_en}} {
        lock_avalon_base_address  mipi_in_exp_fusion_0.av_mm_control_agent
    }
    lock_avalon_base_address  mipi_in_pip_conv_0.av_mm_control_agent
    if {${v_multi_sensor}} {
        lock_avalon_base_address  mipi_in_snoop_1_0.av_mm_control_agent
        if {${v_exp_fusion_en}} {
            lock_avalon_base_address  mipi_in_exp_fusion_1.av_mm_control_agent
        }
        lock_avalon_base_address  mipi_in_pip_conv_1.av_mm_control_agent
    }

    lock_avalon_base_address  mipi_in_mipi_csi2_0.avalon_mm_control_interface
    if {${v_multi_sensor}} {
        lock_avalon_base_address  mipi_in_mipi_csi2_1.avalon_mm_control_interface
    }


    #############################
    ##### Sync / Validation #####
    #############################

    sync_sysinfo_parameters
    save_system
}

proc edit_top_level_qsys {} {
    set v_project_name            [get_shell_parameter PROJECT_NAME]
    set v_project_path            [get_shell_parameter PROJECT_PATH]
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]
    set v_multi_sensor            [get_shell_parameter MULTI_SENSOR]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name} ${v_instance_name}

    add_interface           mipi_dphy_rzq         conduit     end
    set_interface_property  mipi_dphy_rzq         EXPORT_OF   ${v_instance_name}.mipi_dphy_rzq

    add_interface           mipi_dphy_ref_clk_0       clock       sink
    set_interface_property  mipi_dphy_ref_clk_0       EXPORT_OF   ${v_instance_name}.mipi_dphy_ref_clk_0

    add_interface           mipi_dphy_LINK0_dphy_io     conduit     end
    set_interface_property  mipi_dphy_LINK0_dphy_io     EXPORT_OF   ${v_instance_name}.mipi_dphy_LINK0_dphy_io

    if {${v_multi_sensor}} {
        add_interface           mipi_dphy_LINK1_dphy_io   conduit     end
        set_interface_property  mipi_dphy_LINK1_dphy_io   EXPORT_OF   ${v_instance_name}.mipi_dphy_LINK1_dphy_io
    }

    add_interface           mipi_pio_0      conduit     end
    set_interface_property  mipi_pio_0      EXPORT_OF   ${v_instance_name}.mipi_pio_0

    add_interface           mipi_pio_1      conduit     end
    set_interface_property  mipi_pio_1      EXPORT_OF   ${v_instance_name}.mipi_pio_1

    sync_sysinfo_parameters
    save_system
}

proc add_auto_connections {} {
    set v_instance_name         [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host_clk_freq    [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_avmm_host             [get_shell_parameter AVMM_HOST]
    set v_multi_sensor          [get_shell_parameter MULTI_SENSOR]
    set v_vid_out_rate          [get_shell_parameter VID_OUT_RATE]
    set v_pip                   [get_shell_parameter PIP]

    add_auto_connection   ${v_instance_name}    cpu_clk_in        ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name}    cpu_rst_in        ${v_avmm_host_clk_freq}

    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
        add_auto_connection   ${v_instance_name}    vid_clk_in        297000000
        add_auto_connection   ${v_instance_name}    vid_rst_in        297000000
    } else {
        add_auto_connection   ${v_instance_name}    vid_clk_in        148500000
        add_auto_connection   ${v_instance_name}    vid_rst_in        148500000
    }

    if {(${v_vid_out_rate} == "p30") && (${v_pip} == 2)} {
        add_auto_connection   ${v_instance_name}    vid_mipi_clk_in   297000000
        add_auto_connection   ${v_instance_name}    vid_mipi_rst_in   297000000
    }

    # to isp in
    add_auto_connection   ${v_instance_name}    mipi_csi2_0_m_vid_axis      mipi_csi2_0_vid_axis
    if {${v_multi_sensor}} {
        add_auto_connection   ${v_instance_name}    mipi_csi2_1_m_vid_axis      mipi_csi2_1_vid_axis
    }

    # HPS to mm bridge
    add_avmm_connections  mm_ctrl_in    ${v_avmm_host}
}

proc edit_top_v_file {} {
    set v_drv_clock_subsystem_name  [get_shell_parameter DRV_CLOCK_SUBSYSTEM_NAME]
    set v_vid_out_rate              [get_shell_parameter VID_OUT_RATE]
    set v_board_name                [get_shell_parameter DEVKIT]
    set v_multi_sensor              [get_shell_parameter MULTI_SENSOR]
    set v_exp_fusion_en             [get_shell_parameter EXP_FUSION_EN]
    set v_time_seconds              [clock seconds]

    add_top_port_list input   ""          "mipi_dphy_rzq"
    add_top_port_list input   ""          "mipi_dphy_ref_clk"
    add_top_port_list input   "\[3:0\]"   "LINK0_dphy_io_dphy_link_d_p"
    add_top_port_list input   "\[3:0\]"   "LINK0_dphy_io_dphy_link_d_n"
    add_top_port_list input   ""          "LINK0_dphy_io_dphy_link_c_p"
    add_top_port_list input   ""          "LINK0_dphy_io_dphy_link_c_n"
    if {${v_multi_sensor}} {
        add_top_port_list input   "\[3:0\]"   "LINK1_dphy_io_dphy_link_d_p"
        add_top_port_list input   "\[3:0\]"   "LINK1_dphy_io_dphy_link_d_n"
        add_top_port_list input   ""          "LINK1_dphy_io_dphy_link_c_p"
        add_top_port_list input   ""          "LINK1_dphy_io_dphy_link_c_n"
    }
    add_top_port_list inout   ""          "cam_i2c_scl"
    add_top_port_list inout   ""          "cam_i2c_sda"
    add_top_port_list input   ""          "cam_mipi0_fstrobe"
    if {${v_multi_sensor}} {
        add_top_port_list input   ""          "cam_mipi1_fstrobe"
    }
    add_top_port_list input   ""          "cam_mipi_xvs"
    add_top_port_list input   ""          "cam_mipi_xhs"
    add_top_port_list input   ""          "cam_xtrig_xvs"
    add_top_port_list inout   ""          "max10_i2c_scl"
    add_top_port_list inout   ""          "max10_i2c_sda"

    add_declaration_list wire   "\[31:0\]"  "mipi_pio_0_out"
    add_declaration_list wire   "\[31:0\]"  "mipi_pio_0_in"
    add_declaration_list wire   "\[31:0\]"  "mipi_pio_1_in"

    add_declaration_list reg   "\[2:0\]"    "cam_i2c_scl_oe_reg"
    add_declaration_list reg   "\[2:0\]"    "cam_i2c_sda_oe_reg"
    add_declaration_list reg   "\[2:0\]"    "max10_i2c_scl_oe_reg"
    add_declaration_list reg   "\[2:0\]"    "max10_i2c_sda_oe_reg"

    add_qsys_inst_exports_list    mipi_dphy_rzq_rzq                     mipi_dphy_rzq
    add_qsys_inst_exports_list    mipi_dphy_ref_clk_0_clk               mipi_dphy_ref_clk
    add_qsys_inst_exports_list    mipi_dphy_LINK0_dphy_io_dphy_link_dp  LINK0_dphy_io_dphy_link_d_p
    add_qsys_inst_exports_list    mipi_dphy_LINK0_dphy_io_dphy_link_dn  LINK0_dphy_io_dphy_link_d_n
    add_qsys_inst_exports_list    mipi_dphy_LINK0_dphy_io_dphy_link_cp  LINK0_dphy_io_dphy_link_c_p
    add_qsys_inst_exports_list    mipi_dphy_LINK0_dphy_io_dphy_link_cn  LINK0_dphy_io_dphy_link_c_n
    if {${v_multi_sensor}} {
        add_qsys_inst_exports_list    mipi_dphy_LINK1_dphy_io_dphy_link_dp  LINK1_dphy_io_dphy_link_d_p
        add_qsys_inst_exports_list    mipi_dphy_LINK1_dphy_io_dphy_link_dn  LINK1_dphy_io_dphy_link_d_n
        add_qsys_inst_exports_list    mipi_dphy_LINK1_dphy_io_dphy_link_cp  LINK1_dphy_io_dphy_link_c_p
        add_qsys_inst_exports_list    mipi_dphy_LINK1_dphy_io_dphy_link_cn  LINK1_dphy_io_dphy_link_c_n
    }
    add_qsys_inst_exports_list    mipi_pio_0_in_port        mipi_pio_0_in
    add_qsys_inst_exports_list    mipi_pio_0_out_port       mipi_pio_0_out
    add_qsys_inst_exports_list    mipi_pio_1_export         mipi_pio_1_in

    add_assignments_list "hps_i2c1_scl_in"      "cam_i2c_scl"
    add_assignments_list "cam_i2c_scl"          "cam_i2c_scl_oe_reg\[2\] ? 1'b0 : 1'bz"
    add_assignments_list "hps_i2c1_sda_in"      "cam_i2c_sda"
    add_assignments_list "cam_i2c_sda"          "cam_i2c_sda_oe_reg\[2\] ? 1'b0 : 1'bz"
    add_assignments_list "hps_i2c0_scl_in"      "max10_i2c_scl"
    add_assignments_list "max10_i2c_scl"        "max10_i2c_scl_oe_reg\[2\] ? 1'b0 : 1'bz"
    add_assignments_list "hps_i2c0_sda_in"      "max10_i2c_sda"
    add_assignments_list "max10_i2c_sda"        "max10_i2c_sda_oe_reg\[2\] ? 1'b0 : 1'bz"
    add_assignments_list "mipi_pio_0_in\[31:12\]"    "20'b0"
    if {${v_multi_sensor}} {
        add_assignments_list "mipi_pio_0_in\[11:8\]"     "4'b0001"
    } else {
        add_assignments_list "mipi_pio_0_in\[11:8\]"     "4'b0000"
    }
    if {${v_board_name} == "AGX_5E_Si_Devkit"} {
        add_assignments_list "mipi_pio_0_in\[7:4\]"     "4'b0000"
    } elseif {${v_board_name} == "AGX_5E_Modular_Devkit"} {
        add_assignments_list "mipi_pio_0_in\[7:4\]"     "4'b0001"
    } else {
        add_assignments_list "mipi_pio_0_in\[7:4\]"     "4'b1111"
    }
    add_assignments_list "mipi_pio_0_in\[3\]"       "1'b0"
    if {${v_exp_fusion_en}} {
        add_assignments_list "mipi_pio_0_in\[2\]"       "1'b1"
    } else {
        add_assignments_list "mipi_pio_0_in\[2\]"       "1'b0"
    }
    add_assignments_list "mipi_pio_0_in\[1\]"       "multiple_isp_pipes"
    if {${v_vid_out_rate} == "p60"} {
        add_assignments_list "mipi_pio_0_in\[0\]"     "1'b1"
    } else {
        add_assignments_list "mipi_pio_0_in\[0\]"     "1'b0"
    }
    add_assignments_list "mipi_pio_1_in\[31:0\]"   "32'd${v_time_seconds}"

    set generic_code {}

    lappend generic_code "always @ (posedge ${v_drv_clock_subsystem_name}_ref_clk) begin"
    lappend generic_code "    cam_i2c_scl_oe_reg <= {cam_i2c_scl_oe_reg\[1:0\], hps_i2c1_scl_oe};"
    lappend generic_code "    cam_i2c_sda_oe_reg <= {cam_i2c_sda_oe_reg\[1:0\], hps_i2c1_sda_oe};"
    lappend generic_code ""
    lappend generic_code "    max10_i2c_scl_oe_reg <= {max10_i2c_scl_oe_reg\[1:0\], hps_i2c0_scl_oe};"
    lappend generic_code "    max10_i2c_sda_oe_reg <= {max10_i2c_sda_oe_reg\[1:0\], hps_i2c0_sda_oe};"
    lappend generic_code "end"

    add_code_insert_list $generic_code
}

proc modify_manual_ocs_rom {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_drv_ocs_subsystem_name    [get_shell_parameter DRV_OCS_SUBSYSTEM_NAME]

    if {${v_drv_ocs_subsystem_name} != ""} {

        set v_ocs_entry_base              [get_shell_parameter OCS_ENTRY_BASE]
        set v_pio_0_addr_offset           [get_shell_parameter PIO_0_ADDR_OFFSET]
        set v_pio_1_addr_offset           [get_shell_parameter PIO_1_ADDR_OFFSET]
        set v_inst_id                     [get_shell_parameter INST_ID]

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

      set_component_parameter_value  C_NUM_CAPS [expr ${v_num_caps} + 2]

      set v_index ${v_num_caps}

      # manual pio - Camera Control/Capabilities Reg
      set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} + ${v_pio_0_addr_offset}]
      set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
      set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
      set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
      set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT ${v_inst_id}

      set v_index [incr v_index]

      # manual pio - Timestamp Reg
      set_component_parameter_value      C_CAP${v_index}_BASE         [expr ${v_ocs_entry_base} + ${v_pio_1_addr_offset}]
      set_component_parameter_value      C_CAP${v_index}_SIZE         {16}
      set_component_parameter_value      C_CAP${v_index}_TYPE         {528}
      set_component_parameter_value      C_CAP${v_index}_VERSION      {1}
      set_component_parameter_value      C_CAP${v_index}_ID_COMPONENT [expr ${v_inst_id} + 1]

      save_component

      sync_sysinfo_parameters
      save_system
    }
}
