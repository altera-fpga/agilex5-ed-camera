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

package require -exact qsys 18.1

############################################################################
# Module Properties
############################################################################
set_module_property DISPLAY_NAME                 "DLA Layout Transform"
set_module_property DESCRIPTION                  "DLA Layout Transform for converting 8b RGB raster to DLA Folded FP16"
set_module_property NAME                         intel_dla_lt
set_module_property VERSION                      1.0
set_module_property GROUP                        "Video and Vision Processing"
set_module_property ICON_PATH                    logo.jpg
set_module_property DATASHEET_URL                http://www.intel.com/content/www/us/en/products/programmable.html
set_module_property EDITABLE                     false
set_module_property AUTHOR                       "Intel Corporation"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property INTERNAL                     false
set_module_property ELABORATION_CALLBACK         elaboration_callback

############################################################################
# Module File Dependencies
############################################################################

add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL intel_dla_lt_core
add_fileset_file  src_hdl/intel_dla_lt_cpu.sv     SYSTEM_VERILOG  PATH   src_hdl/intel_dla_lt_cpu.sv
add_fileset_file  src_hdl/intel_dla_lt_gen.sv     SYSTEM_VERILOG  PATH   src_hdl/intel_dla_lt_gen.sv
add_fileset_file  src_hdl/intel_dla_lt_core.sv    SYSTEM_VERILOG  PATH   src_hdl/intel_dla_lt_core.sv   TOP_LEVEL_FILE
add_fileset_file  intel_dla_lt.sdc                SDC_ENTITY      PATH   intel_dla_lt.sdc {NO_SDC_PROMOTION}

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL intel_dla_lt_core
add_fileset_file  src_hdl/intel_dla_lt_cpu.sv     SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_cpu.sv
add_fileset_file  src_hdl/intel_dla_lt_gen.sv     SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_gen.sv
add_fileset_file  src_hdl/intel_dla_lt_core.sv    SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_core.sv   TOP_LEVEL_FILE

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL intel_dla_lt_core
add_fileset_file  src_hdl/intel_dla_lt_cpu.sv     SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_cpu.sv
add_fileset_file  src_hdl/intel_dla_lt_gen.sv     SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_gen.sv
add_fileset_file  src_hdl/intel_dla_lt_core.sv    SYSTEM_VERILOG PATH    src_hdl/intel_dla_lt_core.sv   TOP_LEVEL_FILE


############################################################################
# Module parameters
############################################################################

add_parameter                 C_OMNI_CAP_ENABLED          INTEGER               1
set_parameter_property        C_OMNI_CAP_ENABLED          VISIBLE               false
set_parameter_property        C_OMNI_CAP_ENABLED          ALLOWED_RANGES        0:1
set_parameter_property        C_OMNI_CAP_ENABLED          ENABLED               true
set_parameter_property        C_OMNI_CAP_ENABLED          HDL_PARAMETER         false

add_parameter                 DEVICE_FAMILY               STRING
set_parameter_property        DEVICE_FAMILY               VISIBLE               false
set_parameter_property        DEVICE_FAMILY               SYSTEM_INFO           {DEVICE_FAMILY}
set_parameter_property        DEVICE_FAMILY               AFFECTS_GENERATION    true
set_parameter_property        DEVICE_FAMILY               HDL_PARAMETER         true
set_parameter_property        DEVICE_FAMILY               ENABLED               true

add_parameter                 RUNTIME_CONTROL             INTEGER               1
set_parameter_property        RUNTIME_CONTROL             ALLOWED_RANGES        0:1
set_parameter_property        RUNTIME_CONTROL             DISPLAY_HINT          boolean
set_parameter_property        RUNTIME_CONTROL             HDL_PARAMETER         true
set_parameter_property        RUNTIME_CONTROL             DISPLAY_NAME          "CPU Interface"
set_parameter_property        RUNTIME_CONTROL             DESCRIPTION           "Enables the CPU Interface."
set_parameter_property        RUNTIME_CONTROL             AFFECTS_ELABORATION   true

add_parameter                 C_CPU_OFFSET                INTEGER               0
set_parameter_property        C_CPU_OFFSET                ALLOWED_RANGES        0:62
set_parameter_property        C_CPU_OFFSET                DISPLAY_UNITS         {[0 - 62] Registers}
set_parameter_property        C_CPU_OFFSET                HDL_PARAMETER         true
set_parameter_property        C_CPU_OFFSET                DISPLAY_NAME          "CPU Register Base Address"
set_parameter_property        C_CPU_OFFSET                DESCRIPTION \
                                                                    "Base Address of internal CPU Registers"
set_parameter_property        C_CPU_OFFSET                AFFECTS_ELABORATION   false

add_parameter                 NUMBER_OF_COLOR_PLANES_IN   INTEGER               3
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   ALLOWED_RANGES        3
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   HDL_PARAMETER         true
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   DISPLAY_NAME          "Input Color Planes"
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   DESCRIPTION           "3 Color Planes on the Input."
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   AFFECTS_ELABORATION   true
set_parameter_property        NUMBER_OF_COLOR_PLANES_IN   ENABLED               false

add_parameter                 PIXELS_IN_PARALLEL_IN       INTEGER               1
set_parameter_property        PIXELS_IN_PARALLEL_IN       ALLOWED_RANGES        1
set_parameter_property        PIXELS_IN_PARALLEL_IN       HDL_PARAMETER         true
set_parameter_property        PIXELS_IN_PARALLEL_IN       DISPLAY_NAME          "Input Pixels processed in Parallel"
set_parameter_property        PIXELS_IN_PARALLEL_IN       DESCRIPTION \
                                                                      "1 Pixels processed in parallel on the Input."
set_parameter_property        PIXELS_IN_PARALLEL_IN       AFFECTS_ELABORATION   true
set_parameter_property        PIXELS_IN_PARALLEL_IN       ENABLED               false

add_parameter                 BPS_IN                      INTEGER               8
set_parameter_property        BPS_IN                      ALLOWED_RANGES        8
set_parameter_property        BPS_IN                      DISPLAY_UNITS         {[8] bits}
set_parameter_property        BPS_IN                      HDL_PARAMETER         true
set_parameter_property        BPS_IN                      DISPLAY_NAME          "Input Color Depth"
set_parameter_property        BPS_IN                      DESCRIPTION           "8 bits per Color on the Input."
set_parameter_property        BPS_IN                      AFFECTS_ELABORATION   true
set_parameter_property        BPS_IN                      ENABLED               false

add_parameter                 NUMBER_OF_COLOR_PLANES_OUT  INTEGER               4
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  ALLOWED_RANGES        4
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  HDL_PARAMETER         true
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  DISPLAY_NAME          "Output Color Planes"
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  DESCRIPTION           "4 Color Planes on the Output."
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  AFFECTS_ELABORATION   true
set_parameter_property        NUMBER_OF_COLOR_PLANES_OUT  ENABLED               false

add_parameter                 PIXELS_IN_PARALLEL_OUT      INTEGER               4
set_parameter_property        PIXELS_IN_PARALLEL_OUT      ALLOWED_RANGES        4
set_parameter_property        PIXELS_IN_PARALLEL_OUT      HDL_PARAMETER         true
set_parameter_property        PIXELS_IN_PARALLEL_OUT      DISPLAY_NAME          "Output Pixels processed in Parallel"
set_parameter_property        PIXELS_IN_PARALLEL_OUT      DESCRIPTION \
                                                                      "4 Pixels processed in parallel on the Output."
set_parameter_property        PIXELS_IN_PARALLEL_OUT      AFFECTS_ELABORATION   true
set_parameter_property        PIXELS_IN_PARALLEL_OUT      ENABLED               false

add_parameter                 BPS_OUT                     INTEGER               16
set_parameter_property        BPS_OUT                     ALLOWED_RANGES        16
set_parameter_property        BPS_OUT                     DISPLAY_UNITS         {"bits"}
set_parameter_property        BPS_OUT                     HDL_PARAMETER         true
set_parameter_property        BPS_OUT                     DISPLAY_NAME          "Output Color Depth"
set_parameter_property        BPS_OUT                     DESCRIPTION           "16 bits per Color on the Output."
set_parameter_property        BPS_OUT                     AFFECTS_ELABORATION   true
set_parameter_property        BPS_OUT                     ENABLED               false

add_parameter                 C_V_LINES                   INTEGER               318
set_parameter_property        C_V_LINES                   HDL_PARAMETER         true
set_parameter_property        C_V_LINES                   DISPLAY_NAME          "Vertical Lines - 2"
set_parameter_property        C_V_LINES                   DESCRIPTION \
                                                                        "Number of Lines of the Image, minus 2."
set_parameter_property        C_V_LINES                   AFFECTS_ELABORATION   false

add_parameter                 C_DC_FP16_PIX_VAL           STD_LOGIC_VECTOR      0x5800
set_parameter_property        C_DC_FP16_PIX_VAL           ALLOWED_RANGES        0:91128
set_parameter_property        C_DC_FP16_PIX_VAL           HDL_PARAMETER         true
set_parameter_property        C_DC_FP16_PIX_VAL           DISPLAY_NAME          "Don't Care FP16 Pixel Value"
set_parameter_property        C_DC_FP16_PIX_VAL           DESCRIPTION \
                                                    "1.5.10 (Sign.Exp.Man) IEEE-754 FP16 Don't Care Pixel Value."
set_parameter_property        C_DC_FP16_PIX_VAL           AFFECTS_ELABORATION   false

add_parameter                 C_DF_FP16_PIX_VAL           STD_LOGIC_VECTOR      0x0000
set_parameter_property        C_DF_FP16_PIX_VAL           ALLOWED_RANGES        0:91128
set_parameter_property        C_DF_FP16_PIX_VAL           HDL_PARAMETER         true
set_parameter_property        C_DF_FP16_PIX_VAL           DISPLAY_NAME          "Default FP16 Pixel Value"
set_parameter_property        C_DF_FP16_PIX_VAL           DESCRIPTION \
                                                        "1.5.10 (Sign.Exp.Man) IEEE-754 FP16 Default Pixel Value."
set_parameter_property        C_DF_FP16_PIX_VAL           AFFECTS_ELABORATION   false

add_parameter                 C_RB_SWAP                   INTEGER               0
set_parameter_property        C_RB_SWAP                   ALLOWED_RANGES        0:1
set_parameter_property        C_RB_SWAP                   DISPLAY_HINT          boolean
set_parameter_property        C_RB_SWAP                   HDL_PARAMETER         true
set_parameter_property        C_RB_SWAP                   DISPLAY_NAME          "Red Blue Channel Swap"
set_parameter_property        C_RB_SWAP                   DESCRIPTION           "Swaps the Red and Blue Channels."
set_parameter_property        C_RB_SWAP                   AFFECTS_ELABORATION   false



###################################################################################
# HELPER FUNCTIONS
###################################################################################

###################################################################################
#
# Create the Fake Generics (These dont exist in RTL) But can be probed by Qsys
#
###################################################################################
proc omni_add_fake_generic {v_gui_grp desc v_generic default lower higher } {

  add_display_item       ${v_gui_grp} ${v_generic} parameter
  add_parameter          ${v_generic} INTEGER ${lower}
  set_parameter_property ${v_generic} DEFAULT_VALUE ${default}
  set_parameter_property ${v_generic} DISPLAY_NAME ${desc}
  set_parameter_property ${v_generic} ALLOWED_RANGES ${lower}:${higher}
  set_parameter_property ${v_generic} ENABLED true
  set_parameter_property ${v_generic} UNITS None
  set_parameter_property ${v_generic} VISIBLE true
  set_parameter_property ${v_generic} HDL_PARAMETER false

}

###################################################################################
#
# Add the Offset Capability Info to a core.
#
###################################################################################
proc omni_add_capability { value version size {v_en 0} {v_st 0} } {

  add_display_item "" "Capability Info" GROUP tab
  add_display_item "Capability Info" "CapInfo" GROUP

  omni_add_fake_generic  "CapInfo" "Type"                      C_OMNI_CAP_TYPE ${value} 0 1024
  omni_add_fake_generic  "CapInfo" "Version "                  C_OMNI_CAP_VERSION ${version} 1 255
  omni_add_fake_generic  "CapInfo" "Size (32bit Words)"        C_OMNI_CAP_SIZE ${size} 0 1073741824
  omni_add_fake_generic  "CapInfo" "Associated ID"             C_OMNI_CAP_ID_ASSOCIATED 0 0 255
  omni_add_fake_generic  "CapInfo" "Component ID"              C_OMNI_CAP_ID_COMPONENT 0 0 255
  omni_add_fake_generic  "CapInfo" "IRQ Vector (255:disabled)" C_OMNI_CAP_IRQ 255 0 255
  omni_add_fake_generic  "CapInfo" "Tag"                       C_OMNI_CAP_TAG 0 0 255
  omni_add_fake_generic  "CapInfo" "IRQ Enable Exists"         C_OMNI_CAP_IRQ_ENABLE_EN ${v_en} 0 1
  omni_add_fake_generic  "CapInfo" "IRQ Enable Register"       C_OMNI_CAP_IRQ_ENABLE ${v_en} 0 32767
  if {${v_en}} {
    set_parameter_property C_OMNI_CAP_IRQ_ENABLE_EN    ENABLED false
    set_parameter_property C_OMNI_CAP_IRQ_ENABLE       ENABLED false
  }
  omni_add_fake_generic  "CapInfo" "IRQ Status Exists"         C_OMNI_CAP_IRQ_STATUS_EN ${v_st} 0 1
  omni_add_fake_generic  "CapInfo" "IRQ Status Register"       C_OMNI_CAP_IRQ_STATUS ${v_st} 0 32767
  if {${v_st}} {
    set_parameter_property C_OMNI_CAP_IRQ_STATUS_EN    ENABLED false
    set_parameter_property C_OMNI_CAP_IRQ_STATUS       ENABLED false
  }
  set_parameter_property C_OMNI_CAP_IRQ_ENABLE_EN DISPLAY_HINT boolean--
  set_parameter_property C_OMNI_CAP_IRQ_STATUS_EN DISPLAY_HINT boolean--

  set_parameter_property C_OMNI_CAP_TYPE    ENABLED false
  set_parameter_property C_OMNI_CAP_VERSION ENABLED false
  set_parameter_property C_OMNI_CAP_SIZE    ENABLED false

}

add_display_item        ""                      "Parameters"                GROUP tab

add_display_item        "Parameters"            RUNTIME_CONTROL             parameter
add_display_item        "Parameters"            C_CPU_OFFSET                parameter
add_display_item        "Parameters"            NUMBER_OF_COLOR_PLANES_IN   parameter
add_display_item        "Parameters"            PIXELS_IN_PARALLEL_IN       parameter
add_display_item        "Parameters"            BPS_IN                      parameter
add_display_item        "Parameters"            NUMBER_OF_COLOR_PLANES_OUT  parameter
add_display_item        "Parameters"            PIXELS_IN_PARALLEL_OUT      parameter
add_display_item        "Parameters"            BPS_OUT                     parameter
add_display_item        "Parameters"            C_V_LINES                   parameter
add_display_item        "Parameters"            C_DC_FP16_PIX_VAL           parameter
add_display_item        "Parameters"            C_DF_FP16_PIX_VAL           parameter
add_display_item        "Parameters"            C_RB_SWAP                   parameter


omni_add_capability 950 1 64 0 0


############################################################################
# Interfaces
############################################################################

#AVMM slave interface
add_interface             agent_clock             clock                 end
set_interface_property    agent_clock             clockRate             0
set_interface_property    agent_clock             enabled               true
add_interface_port        agent_clock             agent_clock           clk         Input  1

add_interface             agent_reset             reset                 end
set_interface_property    agent_reset             associatedClock       agent_clock
set_interface_property    agent_reset             synchronousEdges      DEASSERT
set_interface_property    agent_reset             enabled               true
add_interface_port        agent_reset             agent_reset           reset       Input  1

add_interface             av_mm_control_agent     avalon      slave                 agent_clock
set_interface_property    av_mm_control_agent     addressAlignment                  DYNAMIC
set_interface_property    av_mm_control_agent     addressSpan                       64
set_interface_property    av_mm_control_agent     bridgesToMaster                   ""
set_interface_property    av_mm_control_agent     burstOnBurstBoundariesOnly        false
set_interface_property    av_mm_control_agent     holdTime                          0
set_interface_property    av_mm_control_agent     isMemoryDevice                    false
set_interface_property    av_mm_control_agent     isNonVolatileStorage              false
set_interface_property    av_mm_control_agent     linewrapBursts                    false
set_interface_property    av_mm_control_agent     minimumUninterruptedRunLength     1
set_interface_property    av_mm_control_agent     printableDevice                   false
set_interface_property    av_mm_control_agent     readWaitTime                      0
set_interface_property    av_mm_control_agent     setupTime                         0
set_interface_property    av_mm_control_agent     timingUnits                       Cycles
set_interface_property    av_mm_control_agent     writeWaitTime                     0
set_interface_property    av_mm_control_agent     readLatency                       0
set_interface_property    av_mm_control_agent     maximumPendingReadTransactions    1
set_interface_property    av_mm_control_agent     enabled                           true
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_byteenable        byteenable     Input    4
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_write             write          Input    1
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_writedata         writedata      Input    32
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_read              read           Input    1
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_readdata          readdata       Output   32
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_readdatavalid     readdatavalid  Output   1
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_address           address        Input    6
add_interface_port        av_mm_control_agent     av_mm_cpu_agent_waitrequest       waitrequest    Output   1

#Video interface
add_interface           main_clock          clock                   end
set_interface_property  main_clock          clockRate               0
set_interface_property  main_clock          ENABLED                 true
add_interface_port      main_clock          main_clock              clk     Input  1

add_interface           main_reset          reset                   end
set_interface_property  main_reset          associatedClock         main_clock
set_interface_property  main_reset          synchronousEdges        DEASSERT
set_interface_property  main_reset          ENABLED                 true
add_interface_port      main_reset          main_reset              reset   Input  1

add_interface           axi4s_vid_in        axi4stream              end
set_interface_property  axi4s_vid_in        associatedClock         main_clock
set_interface_property  axi4s_vid_in        associatedReset         main_reset
add_interface_port      axi4s_vid_in        axi4s_vid_in_tvalid     tvalid            Input   1
add_interface_port      axi4s_vid_in        axi4s_vid_in_tready     tready            Output  1
add_interface_port      axi4s_vid_in        axi4s_vid_in_tlast      tlast             Input   1

add_interface           fifo_reset          reset                   start
set_interface_property  fifo_reset          associatedClock         main_clock
set_interface_property  fifo_reset          synchronousEdges        DEASSERT
set_interface_property  fifo_reset          ENABLED                 true
add_interface_port      fifo_reset          fifo_reset              reset             Output  1

add_interface           axi4s_vid_fifo_out  axi4stream                start
set_interface_property  axi4s_vid_fifo_out  associatedClock           main_clock
set_interface_property  axi4s_vid_fifo_out  associatedReset           main_reset
add_interface_port      axi4s_vid_fifo_out  axi4s_vid_fifo_out_tvalid tvalid          Output  1
add_interface_port      axi4s_vid_fifo_out  axi4s_vid_fifo_out_tready tready          Input   1
add_interface_port      axi4s_vid_fifo_out  axi4s_vid_fifo_out_tlast  tlast           Output  1

add_interface           axi4s_vid_fifo_in   axi4stream                end
set_interface_property  axi4s_vid_fifo_in   associatedClock           main_clock
set_interface_property  axi4s_vid_fifo_in   associatedReset           main_reset
add_interface_port      axi4s_vid_fifo_in   axi4s_vid_fifo_in_tvalid  tvalid          Input   1
add_interface_port      axi4s_vid_fifo_in   axi4s_vid_fifo_in_tready  tready          Output  1
add_interface_port      axi4s_vid_fifo_in   axi4s_vid_fifo_in_tlast   tlast           Input   1

add_interface           axi4s_vid_out       axi4stream              start
set_interface_property  axi4s_vid_out       associatedClock         main_clock
set_interface_property  axi4s_vid_out       associatedReset         main_reset
add_interface_port      axi4s_vid_out       axi4s_vid_out_tvalid    tvalid            Output  1
add_interface_port      axi4s_vid_out       axi4s_vid_out_tready    tready            Input   1
add_interface_port      axi4s_vid_out       axi4s_vid_out_tlast     tlast             Output  1


############################################################################
# Procedures
############################################################################

proc elaboration_callback {} {

  set v_num_color_planes_in    [ get_parameter_value   NUMBER_OF_COLOR_PLANES_IN ]
  set v_num_pip_in             [ get_parameter_value   PIXELS_IN_PARALLEL_IN ]
  set v_bits_per_color_in      [ get_parameter_value   BPS_IN ]

  set v_s_vid_axis_tdata_width  [expr ${v_num_color_planes_in} * ${v_bits_per_color_in} * ${v_num_pip_in}]
  set v_s_vid_axis_tuser_width  [expr ( ${v_num_color_planes_in} * ${v_bits_per_color_in} / 8 ) * ${v_num_pip_in}]

  set v_num_color_planes_out    [ get_parameter_value   NUMBER_OF_COLOR_PLANES_OUT ]
  set v_num_pip_out             [ get_parameter_value   PIXELS_IN_PARALLEL_OUT ]
  set v_bits_per_color_out      [ get_parameter_value   BPS_OUT ]

  set v_m_vid_axis_tdata_width  [expr ${v_num_color_planes_out} * ${v_bits_per_color_out} * ${v_num_pip_out}]
  set v_m_vid_axis_tuser_width  [expr ( ${v_num_color_planes_out} * ${v_bits_per_color_out} / 8 ) * ${v_num_pip_out}]

  add_interface_port      axi4s_vid_in        axi4s_vid_in_tuser        tuser   Input   ${v_s_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_in        axi4s_vid_in_tdata        tdata   Input   ${v_s_vid_axis_tdata_width}

  add_interface_port      axi4s_vid_fifo_out  axi4s_vid_fifo_out_tuser  tuser   Output  ${v_s_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_fifo_out  axi4s_vid_fifo_out_tdata  tdata   Output  ${v_s_vid_axis_tdata_width}

  add_interface_port      axi4s_vid_fifo_in   axi4s_vid_fifo_in_tuser   tuser   Input   ${v_s_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_fifo_in   axi4s_vid_fifo_in_tdata   tdata   Input   ${v_s_vid_axis_tdata_width}

  add_interface_port      axi4s_vid_out       axi4s_vid_out_tuser       tuser   Output  ${v_m_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_out       axi4s_vid_out_tdata       tdata   Output  ${v_m_vid_axis_tdata_width}

  set v_cpu_en              [ get_parameter_value   RUNTIME_CONTROL ]

  if { ${v_cpu_en} == 1 } {
    set_interface_property      agent_clock           ENABLED     true
    set_interface_property      agent_reset           ENABLED     true
    set_interface_property      av_mm_control_agent   ENABLED     true

    set_parameter_property      C_CPU_OFFSET          VISIBLE     true
  } else {
    set_interface_property      agent_clock           ENABLED     false
    set_interface_property      agent_reset           ENABLED     false
    set_interface_property      av_mm_control_agent   ENABLED     false

    set_parameter_property      C_CPU_OFFSET          VISIBLE     false
  }

}
