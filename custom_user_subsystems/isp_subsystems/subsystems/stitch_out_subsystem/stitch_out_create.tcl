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
set_shell_parameter TPG_ADDR_OFFSET           "0x00000000"
set_shell_parameter MIXER_ADDR_OFFSET         "0x00000400"
set_shell_parameter 3D_LUT_ADDR_OFFSET        "0x00000800"
set_shell_parameter TMO_ADDR_OFFSET           "0x00000a00"
set_shell_parameter USM_ADDR_OFFSET           "0x00000c00"

# General Video Controls
set_shell_parameter VID_CLK_FREQ              {297000000.0}
set_shell_parameter PIP                       {2}
set_shell_parameter EN_DEBUG                  {1}

set_shell_parameter INST_ID                   {0}

# 3D LUT Controls.
set_shell_parameter SMALL_3DLUT               {1}
set_shell_parameter LUT_PIP_SHARING_EN        {0}
set_shell_parameter LUT_DOUBLE_BUFFERED_EN    {0}
# Enable to preload a default cube file
set_shell_parameter PRESET_FILE_3DLUT_EN      {0}
set_shell_parameter PRESET_FILE_3DLUT_0       {}
set_shell_parameter PRESET_FILE_3DLUT_1       {}

set_shell_parameter TMO_NIOS_CLK_FREQ         {148500000.0}


proc pre_creation_step {} {
    transfer_files
    evaluate_terp
}

proc creation_step {} {
    create_image_stitch_subsystem
}

proc post_creation_step {} {
    edit_top_level_qsys
    add_auto_connections
}


proc transfer_files {} {
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_script_path               [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_lut_double_buffered_en    [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en     [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_preset_file      [get_shell_parameter PRESET_FILE_3DLUT_0]
    set v_3d_lut_1_preset_file      [get_shell_parameter PRESET_FILE_3DLUT_1]

    # 3D LUT Init Files
    if {${v_3d_lut_preset_file_en}} {
        file_copy ${v_script_path}/${v_3d_lut_0_preset_file} \
                                              ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_preset_file}
        if {${v_lut_double_buffered_en}} {
            file_copy ${v_script_path}/${v_3d_lut_1_preset_file} \
                                              ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_preset_file}
        }
        file_copy ${v_script_path}/3D_LUT_preset_script.tcl.terp \
                                              ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp
        file_copy ${v_script_path}/3D_LUT_subsystem.qsf.terp \
                                              ${v_project_path}/quartus/user/${v_instance_name}.qsf.terp
    }
}


proc evaluate_terp {} {
    set v_project_name              [get_shell_parameter PROJECT_NAME]
    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_lut_double_buffered_en    [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en     [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_preset_file      [get_shell_parameter PRESET_FILE_3DLUT_0]
    set v_3d_lut_1_preset_file      [get_shell_parameter PRESET_FILE_3DLUT_1]

    # 3D LUT Init Files
    if {${v_3d_lut_preset_file_en}} {
        if {${v_lut_double_buffered_en}} {
            evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp \
                [list ${v_project_name} ${v_instance_name} ${v_3d_lut_0_preset_file} ${v_3d_lut_1_preset_file}] 0 1
        } else {
            evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}_script.tcl.terp \
                [list ${v_project_name} ${v_instance_name} ${v_3d_lut_0_preset_file}] 0 1
        }
        evaluate_terp_file  ${v_project_path}/quartus/user/${v_instance_name}.qsf.terp \
            [list ${v_instance_name}] 0 1
    }
}


proc create_image_stitch_subsystem {} {
    set v_project_path            [get_shell_parameter PROJECT_PATH]
    set v_instance_name           [get_shell_parameter INSTANCE_NAME]

    # video Pipeline
    set v_vid_clk_freq            [get_shell_parameter VID_CLK_FREQ]
    set v_cppp                    {3}
    set v_bps                     {10}
    set v_pip                     [get_shell_parameter PIP]

    # 3D LUT
    set v_small_3d_lut            [get_shell_parameter SMALL_3DLUT]
    set v_lut_pip_sharing_en      [get_shell_parameter LUT_PIP_SHARING_EN]
    set v_lut_double_buffered_en  [get_shell_parameter LUT_DOUBLE_BUFFERED_EN]
    set v_3d_lut_preset_file_en   [get_shell_parameter PRESET_FILE_3DLUT_EN]
    set v_3d_lut_0_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_0]
    set v_3d_lut_1_preset_file    [get_shell_parameter PRESET_FILE_3DLUT_1]

    # General
    set v_inst_id                 [get_shell_parameter INST_ID]
    set v_enable_debug            [get_shell_parameter EN_DEBUG]
    set v_pipeline_ready          {1}

    set v_avmm_host_clk_freq      [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_tmo_nios_clk_freq       [get_shell_parameter TMO_NIOS_CLK_FREQ]


    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys


    ############################
    #### Add Instances      ####
    ############################

    add_instance  stitch_cpu_clk          altera_clock_bridge
    add_instance  stitch_cpu_rst          altera_reset_bridge
    add_instance  stitch_mm_bridge        altera_avalon_mm_bridge
    add_instance  stitch_vid_clk          altera_clock_bridge
    add_instance  stitch_vid_rst          altera_reset_bridge
    add_instance  stitch_niosv_clk        altera_clock_bridge
    add_instance  stitch_niosv_rst        altera_reset_bridge
    add_instance  stitch_tpg              intel_vvp_tpg
    add_instance  stitch_mixer            intel_vvp_mixer
    add_instance  stitch_3d_lut           intel_vvp_3d_lut
    add_instance  stitch_tmo              intel_vvp_tmo
    add_instance  stitch_usm              intel_vvp_usm


    ############################
    #### Set Parameters     ####
    ############################

    # stitch_cpu_clk
    set_instance_parameter_value    stitch_cpu_clk      EXPLICIT_CLOCK_RATE         ${v_avmm_host_clk_freq}
    set_instance_parameter_value    stitch_cpu_clk      NUM_CLOCK_OUTPUTS           {1}

    # stitch_cpu_rst
    set_instance_parameter_value    stitch_cpu_rst      ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value    stitch_cpu_rst      NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    stitch_cpu_rst      SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    stitch_cpu_rst      SYNC_RESET                  {0}
    set_instance_parameter_value    stitch_cpu_rst      USE_RESET_REQUEST           {0}

    # stitch_mm_bridge
    set_instance_parameter_value    stitch_mm_bridge    ADDRESS_UNITS               {SYMBOLS}
    set_instance_parameter_value    stitch_mm_bridge    ADDRESS_WIDTH               {0}
    set_instance_parameter_value    stitch_mm_bridge    DATA_WIDTH                  {32}
    set_instance_parameter_value    stitch_mm_bridge    LINEWRAPBURSTS              {0}
    set_instance_parameter_value    stitch_mm_bridge    M0_WAITREQUEST_ALLOWANCE    {0}
    set_instance_parameter_value    stitch_mm_bridge    MAX_BURST_SIZE              {1}
    set_instance_parameter_value    stitch_mm_bridge    MAX_PENDING_RESPONSES       {4}
    set_instance_parameter_value    stitch_mm_bridge    MAX_PENDING_WRITES          {0}
    set_instance_parameter_value    stitch_mm_bridge    PIPELINE_COMMAND            {1}
    set_instance_parameter_value    stitch_mm_bridge    PIPELINE_RESPONSE           {1}
    set_instance_parameter_value    stitch_mm_bridge    S0_WAITREQUEST_ALLOWANCE    {0}
    set_instance_parameter_value    stitch_mm_bridge    SYMBOL_WIDTH                {8}
    set_instance_parameter_value    stitch_mm_bridge    SYNC_RESET                  {0}
    set_instance_parameter_value    stitch_mm_bridge    USE_AUTO_ADDRESS_WIDTH      {1}
    set_instance_parameter_value    stitch_mm_bridge    USE_RESPONSE                {0}
    set_instance_parameter_value    stitch_mm_bridge    USE_WRITERESPONSE           {0}

    # stitch_vid_clk
    set_instance_parameter_value    stitch_vid_clk      EXPLICIT_CLOCK_RATE         ${v_vid_clk_freq}
    set_instance_parameter_value    stitch_vid_clk      NUM_CLOCK_OUTPUTS           {1}

    # stitch_vid_rst
    set_instance_parameter_value    stitch_vid_rst      ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value    stitch_vid_rst      NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    stitch_vid_rst      SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    stitch_vid_rst      SYNC_RESET                  {0}
    set_instance_parameter_value    stitch_vid_rst      USE_RESET_REQUEST           {0}

    # stitch_niosv_clk
    set_instance_parameter_value    stitch_niosv_clk    EXPLICIT_CLOCK_RATE         ${v_tmo_nios_clk_freq}
    set_instance_parameter_value    stitch_niosv_clk    NUM_CLOCK_OUTPUTS           {1}

    # stitch_niosv_rst
    set_instance_parameter_value    stitch_niosv_rst    ACTIVE_LOW_RESET            {0}
    set_instance_parameter_value    stitch_niosv_rst    NUM_RESET_OUTPUTS           {1}
    set_instance_parameter_value    stitch_niosv_rst    SYNCHRONOUS_EDGES           {deassert}
    set_instance_parameter_value    stitch_niosv_rst    SYNC_RESET                  {0}
    set_instance_parameter_value    stitch_niosv_rst    USE_RESET_REQUEST           {0}

    # stitch_tpg
    set_instance_parameter_value    stitch_tpg          BINARY_DISPLAY_MODE         {Seconds}
    set_instance_parameter_value    stitch_tpg          BPS                         ${v_bps}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_0            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_1            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_2            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_3            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_4            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_5            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_6            {0}
    set_instance_parameter_value    stitch_tpg          CORE_COL_SPACE_7            {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_0              {1}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_1              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_2              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_3              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_4              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_5              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_6              {0}
    set_instance_parameter_value    stitch_tpg          CORE_PATTERN_7              {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_TYPE             {566}
    set_instance_parameter_value    stitch_tpg          C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    stitch_tpg          ENABLE_CTRL_IN              {0}
    set_instance_parameter_value    stitch_tpg          ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    stitch_tpg          EXTERNAL_MODE               {1}
    set_instance_parameter_value    stitch_tpg          FIXED_BARS_MODE             {0}
    set_instance_parameter_value    stitch_tpg          FIXED_B_BACKGROUND          {0}
    set_instance_parameter_value    stitch_tpg          FIXED_B_CB                  {16}
    set_instance_parameter_value    stitch_tpg          FIXED_B_FONT                {255}
    set_instance_parameter_value    stitch_tpg          FIXED_FINE_FACTOR           {256}
    set_instance_parameter_value    stitch_tpg          FIXED_FPS                   {60}
    set_instance_parameter_value    stitch_tpg          FIXED_G_BACKGROUND          {0}
    set_instance_parameter_value    stitch_tpg          FIXED_G_FONT                {255}
    set_instance_parameter_value    stitch_tpg          FIXED_G_Y                   {16}
    set_instance_parameter_value    stitch_tpg          FIXED_HEIGHT                {16384}
    set_instance_parameter_value    stitch_tpg          FIXED_INTERLACE             {0}
    set_instance_parameter_value    stitch_tpg          FIXED_LOCATION_X            {0}
    set_instance_parameter_value    stitch_tpg          FIXED_LOCATION_Y            {0}
    set_instance_parameter_value    stitch_tpg          FIXED_POWER_FACTOR          {16}
    set_instance_parameter_value    stitch_tpg          FIXED_R_BACKGROUND          {0}
    set_instance_parameter_value    stitch_tpg          FIXED_R_CR                  {16}
    set_instance_parameter_value    stitch_tpg          FIXED_R_FONT                {255}
    set_instance_parameter_value    stitch_tpg          FIXED_SCALE_FACTOR          {1}
    set_instance_parameter_value    stitch_tpg          FIXED_WIDTH                 {16384}
    set_instance_parameter_value    stitch_tpg          NUM_CORES                   {1}
    set_instance_parameter_value    stitch_tpg          OUTPUT_FORMAT               {4.4.4}
    set_instance_parameter_value    stitch_tpg          PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    stitch_tpg          PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    stitch_tpg          RUNTIME_CONTROL             {1}
    set_instance_parameter_value    stitch_tpg          SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    stitch_tpg          SLAVE_PROTOCOL              {Avalon}

    # stitch_mixer
    set_instance_parameter_value    stitch_mixer        NUM_LAYERS                  {3}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_1             {0}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_2             {2}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_3             {0}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_4             {0}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_5             {0}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_6             {0}
    set_instance_parameter_value    stitch_mixer        BLENDING_MODE_7             {0}
    set_instance_parameter_value    stitch_mixer        BPS                         ${v_bps}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_TYPE             {563}
    set_instance_parameter_value    stitch_mixer        C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    stitch_mixer        DO_ROUNDING                 {0}
    set_instance_parameter_value    stitch_mixer        ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    stitch_mixer        EXPORT_PROBES               {0}
    set_instance_parameter_value    stitch_mixer        EXTERNAL_MODE               {1}
    set_instance_parameter_value    stitch_mixer        NUMBER_OF_COLOR_PLANES      ${v_cppp}
    set_instance_parameter_value    stitch_mixer        PIPELINE_LEVEL              {1}
    set_instance_parameter_value    stitch_mixer        PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    stitch_mixer        P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    stitch_mixer        P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_1        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_2        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_3        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_4        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_5        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_6        {1}
    set_instance_parameter_value    stitch_mixer        RESTRICTED_OFFSETS_7        {1}
    set_instance_parameter_value    stitch_mixer        RUNTIME_CONTROL             {1}
    set_instance_parameter_value    stitch_mixer        SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    stitch_mixer        SLAVE_PROTOCOL              {Avalon}

    # stitch_3d_lut
    set_instance_parameter_value    stitch_3d_lut       BPS_IN                      ${v_bps}
    set_instance_parameter_value    stitch_3d_lut       BPS_OUT                     ${v_bps}
    set_instance_parameter_value    stitch_3d_lut       BYPASS_ALPHA                {1023}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_TYPE             {357}
    set_instance_parameter_value    stitch_3d_lut       C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    stitch_3d_lut       EXTERNAL_MODE               {1}
    set_instance_parameter_value    stitch_3d_lut       LUT_ALPHA                   {0}
    set_instance_parameter_value    stitch_3d_lut       LUT_CPU_READABLE            {0}
    set_instance_parameter_value    stitch_3d_lut       LUT_DEPTH                   ${v_bps}
    if {(${v_pip} > 1) && (${v_lut_pip_sharing_en} )} {
        set_instance_parameter_value    stitch_3d_lut       LUT_PIP_SHARING             {1}
    } else {
        set_instance_parameter_value    stitch_3d_lut       LUT_PIP_SHARING             {0}
    }
    if {${v_small_3d_lut}} {
        set_instance_parameter_value    stitch_3d_lut       LUT_DIMENSION               {17}
    } else {
        set_instance_parameter_value    stitch_3d_lut       LUT_DIMENSION               {33}
    }
    if {${v_lut_double_buffered_en}} {
        set_instance_parameter_value    stitch_3d_lut       LUT_DOUBLE_BUFFERED         {1}
    } else {
        set_instance_parameter_value    stitch_3d_lut       LUT_DOUBLE_BUFFERED         {0}
    }
    if {${v_3d_lut_preset_file_en}} {
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_0                  {1}
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_FILENAME_0 \
                                                    ${v_project_path}/non_qpds_ip/user/${v_3d_lut_0_preset_file}
        if {${v_lut_double_buffered_en}} {
            set_instance_parameter_value    stitch_3d_lut       LUT_INIT_1                  {1}
            set_instance_parameter_value    stitch_3d_lut       LUT_INIT_FILENAME_1 \
                                                    ${v_project_path}/non_qpds_ip/user/${v_3d_lut_1_preset_file}
        }
    } else {
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_0                  {0}
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_FILENAME_0         {}
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_1                  {0}
        set_instance_parameter_value    stitch_3d_lut       LUT_INIT_FILENAME_1         {}
    }
    set_instance_parameter_value    stitch_3d_lut       LUT_INIT_TYPE_0             {normalized}
    set_instance_parameter_value    stitch_3d_lut       LUT_INIT_TYPE_1             {normalized}
    set_instance_parameter_value    stitch_3d_lut       PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    stitch_3d_lut       RESET_ENABLED               {0}
    set_instance_parameter_value    stitch_3d_lut       RUNTIME_CONTROL             {1}
    set_instance_parameter_value    stitch_3d_lut       SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    stitch_3d_lut       SLAVE_PROTOCOL              {Avalon}

    # stitch_tmo
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_TYPE             {355}
    set_instance_parameter_value    stitch_tmo          C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    stitch_tmo          P_BPS                       ${v_bps}
    set_instance_parameter_value    stitch_tmo          P_NUMBER_OF_COLOR_PLANES    ${v_cppp}
    set_instance_parameter_value    stitch_tmo          P_PIXELS_IN_PARALLEL        ${v_pip}

    # stitch_usm
    set_instance_parameter_value    stitch_usm          BPS                         ${v_bps}
    set_instance_parameter_value    stitch_usm          COEFFS_FRACTION_BITS        {8}
    set_instance_parameter_value    stitch_usm          COEFFS_INTEGER_BITS         {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_ID_COMPONENT     ${v_inst_id}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_TYPE             {380}
    set_instance_parameter_value    stitch_usm          C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value    stitch_usm          ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value    stitch_usm          EXTERNAL_MODE               {1}
    set_instance_parameter_value    stitch_usm          H_TAPS                      {5}
    set_instance_parameter_value    stitch_usm          MAX_HEIGHT                  {4096}
    set_instance_parameter_value    stitch_usm          MAX_WIDTH                   {8192}
    set_instance_parameter_value    stitch_usm          NO_BLANKING                 {1}
    set_instance_parameter_value    stitch_usm          PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value    stitch_usm          PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value    stitch_usm          P_CORE_CTRL_ID              {0}
    set_instance_parameter_value    stitch_usm          P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value    stitch_usm          RUNTIME_CONTROL             {1}
    set_instance_parameter_value    stitch_usm          SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value    stitch_usm          SIGNED_COEFFS               {0}
    set_instance_parameter_value    stitch_usm          SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value    stitch_usm          V_TAPS                      {5}


    ############################
    #### Create Connections ####
    ############################

    # stitch_cpu_clk
    add_connection    stitch_cpu_clk.out_clk          stitch_cpu_rst.clk
    add_connection    stitch_cpu_clk.out_clk          stitch_mm_bridge.clk
    add_connection    stitch_cpu_clk.out_clk          stitch_tpg.agent_clock
    add_connection    stitch_cpu_clk.out_clk          stitch_mixer.agent_clock
    add_connection    stitch_cpu_clk.out_clk          stitch_3d_lut.cpu_clock
    add_connection    stitch_cpu_clk.out_clk          stitch_tmo.external_cpu_clock
    add_connection    stitch_cpu_clk.out_clk          stitch_usm.agent_clock

    # stitch_cpu_rst
    add_connection    stitch_cpu_rst.out_reset        stitch_mm_bridge.reset
    add_connection    stitch_cpu_rst.out_reset        stitch_tpg.agent_reset
    add_connection    stitch_cpu_rst.out_reset        stitch_mixer.agent_reset
    add_connection    stitch_cpu_rst.out_reset        stitch_3d_lut.cpu_reset
    add_connection    stitch_cpu_rst.out_reset        stitch_tmo.external_cpu_reset
    add_connection    stitch_cpu_rst.out_reset        stitch_usm.agent_reset

    # stitch_mm_bridge
    add_connection    stitch_mm_bridge.m0             stitch_tpg.av_mm_control_agent
    add_connection    stitch_mm_bridge.m0             stitch_mixer.av_mm_control_agent
    add_connection    stitch_mm_bridge.m0             stitch_3d_lut.av_mm_cpu_agent
    add_connection    stitch_mm_bridge.m0             stitch_tmo.av_mm_cpu_agent
    add_connection    stitch_mm_bridge.m0             stitch_usm.av_mm_control_agent

    # stitch_vid_clk
    add_connection    stitch_vid_clk.out_clk          stitch_vid_rst.clk
    add_connection    stitch_vid_clk.out_clk          stitch_tpg.main_clock
    add_connection    stitch_vid_clk.out_clk          stitch_mixer.main_clock
    add_connection    stitch_vid_clk.out_clk          stitch_3d_lut.vid_clock
    add_connection    stitch_vid_clk.out_clk          stitch_tmo.vid_clock
    add_connection    stitch_vid_clk.out_clk          stitch_usm.main_clock

    # stitch_vid_rst
    add_connection    stitch_vid_rst.out_reset        stitch_tpg.main_reset
    add_connection    stitch_vid_rst.out_reset        stitch_mixer.main_reset
    add_connection    stitch_vid_rst.out_reset        stitch_3d_lut.vid_reset
    add_connection    stitch_vid_rst.out_reset        stitch_tmo.vid_reset
    add_connection    stitch_vid_rst.out_reset        stitch_usm.main_reset

    # stitch_niosv_clk
    add_connection    stitch_niosv_clk.out_clk        stitch_niosv_rst.clk
    add_connection    stitch_niosv_clk.out_clk        stitch_tmo.internal_cpu_clock

    # stitch_niosv_rst
    add_connection    stitch_niosv_rst.out_reset      stitch_tmo.internal_cpu_reset

    # stitch_tpg
    add_connection    stitch_tpg.axi4s_vid_out        stitch_mixer.axi4s_vid_0_in

    # stitch_mixer
    add_connection    stitch_mixer.axi4s_vid_out      stitch_3d_lut.axi4s_vid_in

    # stitch_3d_lut
    add_connection    stitch_3d_lut.axi4s_vid_out     stitch_tmo.axi4s_vid_in

    # stitch_tmo
    add_connection    stitch_tmo.axi4s_vid_out        stitch_usm.axi4s_vid_in


    ##########################
    ##### Create Exports #####
    ##########################

    # stitch_cpu_clk
    add_interface           cpu_clk_in    clock       sink
    set_interface_property  cpu_clk_in    EXPORT_OF   stitch_cpu_clk.in_clk

    # stitch_cpu_rst
    add_interface           cpu_rst_in    reset       sink
    set_interface_property  cpu_rst_in    EXPORT_OF   stitch_cpu_rst.in_reset

    # stitch_mm_bridge
    add_interface           mm_ctrl_in    avalon      slave
    set_interface_property  mm_ctrl_in    EXPORT_OF   stitch_mm_bridge.s0

    # stitch_vid_clk
    set_interface_property  vid_clk_in    EXPORT_OF   stitch_vid_clk.in_clk

    # stitch_vid_rst
    set_interface_property  vid_rst_in    EXPORT_OF   stitch_vid_rst.in_reset

    # stitch_niosv_clk
    set_interface_property  niosv_clk_in  EXPORT_OF   stitch_niosv_clk.in_clk

    # stitch_niosv_rst
    set_interface_property  niosv_rst_in  EXPORT_OF   stitch_niosv_rst.in_reset

    # stitch_mixer - RGBA channel must overlay RGB channel
    add_interface           switch_in_s0_vid_axis    axi4stream  subordinate
    set_interface_property  switch_in_s0_vid_axis    EXPORT_OF   stitch_mixer.axi4s_vid_1_in

    add_interface           switch_in_s1_vid_axis    axi4stream  subordinate
    set_interface_property  switch_in_s1_vid_axis    EXPORT_OF   stitch_mixer.axi4s_vid_2_in

    # stitch_usm
    add_interface           switch_out_m_vid_axis    axi4stream  manager
    set_interface_property  switch_out_m_vid_axis    EXPORT_OF   stitch_usm.axi4s_vid_out


    #################################
    ##### Assign Base Addresses #####
    #################################
    set v_tpg_addr_offset                 [get_shell_parameter TPG_ADDR_OFFSET]
    set v_mixer_addr_offset               [get_shell_parameter MIXER_ADDR_OFFSET]
    set v_3d_lut_addr_offset              [get_shell_parameter 3D_LUT_ADDR_OFFSET]
    set v_tmo_addr_offset                 [get_shell_parameter TMO_ADDR_OFFSET]
    set v_usm_addr_offset                 [get_shell_parameter USM_ADDR_OFFSET]

    set_connection_parameter_value stitch_mm_bridge.m0/stitch_tpg.av_mm_control_agent \
                                                                baseAddress ${v_tpg_addr_offset}
    set_connection_parameter_value stitch_mm_bridge.m0/stitch_mixer.av_mm_control_agent \
                                                                          baseAddress ${v_mixer_addr_offset}
    set_connection_parameter_value stitch_mm_bridge.m0/stitch_3d_lut.av_mm_cpu_agent \
                                                                          baseAddress ${v_3d_lut_addr_offset}
    set_connection_parameter_value stitch_mm_bridge.m0/stitch_tmo.av_mm_cpu_agent \
                                                                          baseAddress ${v_tmo_addr_offset}
    set_connection_parameter_value stitch_mm_bridge.m0/stitch_usm.av_mm_control_agent \
                                                                          baseAddress ${v_usm_addr_offset}

    lock_avalon_base_address  stitch_tpg.av_mm_control_agent
    lock_avalon_base_address  stitch_mixer.av_mm_control_agent
    lock_avalon_base_address  stitch_3d_lut.av_mm_cpu_agent
    lock_avalon_base_address  stitch_tmo.av_mm_cpu_agent
    lock_avalon_base_address  stitch_usm.av_mm_control_agent


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
    set v_avmm_host             [get_shell_parameter AVMM_HOST]
    set v_avmm_host_clk_freq    [get_shell_parameter AVMM_HOST_CLK_FREQ]
    set v_tmo_nios_clk_freq     [get_shell_parameter TMO_NIOS_CLK_FREQ]
    set v_vid_clk_freq          [get_shell_parameter VID_CLK_FREQ]

    add_auto_connection   ${v_instance_name}  cpu_clk_in          ${v_avmm_host_clk_freq}
    add_auto_connection   ${v_instance_name}  cpu_rst_in          ${v_avmm_host_clk_freq}

    add_auto_connection   ${v_instance_name}  vid_clk_in          ${v_vid_clk_freq}
    add_auto_connection   ${v_instance_name}  vid_rst_in          ${v_vid_clk_freq}

    add_auto_connection   ${v_instance_name}  niosv_clk_in        ${v_tmo_nios_clk_freq}
    add_auto_connection   ${v_instance_name}  niosv_rst_in        ${v_tmo_nios_clk_freq}

    # from isp
    add_auto_connection   ${v_instance_name}  switch_in_s0_vid_axis   isp_out_0_vid_axis
    add_auto_connection   ${v_instance_name}  switch_in_s1_vid_axis   isp_out_1_vid_axis

    # to vid out
    add_auto_connection   ${v_instance_name}  switch_out_m_vid_axis   full_isp_out_vid_axis

    # HPS to mm bridge
    add_avmm_connections  mm_ctrl_in      ${v_avmm_host}
}
