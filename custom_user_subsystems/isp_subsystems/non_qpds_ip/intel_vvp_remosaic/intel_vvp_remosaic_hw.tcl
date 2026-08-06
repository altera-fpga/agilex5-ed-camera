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
set_module_property DISPLAY_NAME                 "Remosaic"
set_module_property DESCRIPTION                  "Remosaic for converting RGB to Bayer 2x2"
set_module_property NAME                         intel_vvp_remosaic
set_module_property VERSION                      1.0
set_module_property GROUP                        "Video and Vision Processing"
set_module_property ICON_PATH                    logo.jpg
set_module_property DATASHEET_URL                http://www.intel.com/content/www/us/v_en/products/programmable.html
set_module_property EDITABLE                     false
set_module_property AUTHOR                       "Intel Corporation"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property INTERNAL                     false
set_module_property ELABORATION_CALLBACK         elaboration_callback

############################################################################
# Module File Dependencies
############################################################################

add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL intel_vvp_remosaic_core
add_fileset_file  src_hdl/intel_vvp_remosaic_cpu.sv     SYSTEM_VERILOG  PATH   src_hdl/intel_vvp_remosaic_cpu.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_gen.sv     SYSTEM_VERILOG  PATH   src_hdl/intel_vvp_remosaic_gen.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_core.sv    SYSTEM_VERILOG  PATH \
                                                              src_hdl/intel_vvp_remosaic_core.sv   TOP_LEVEL_FILE
add_fileset_file  intel_vvp_remosaic.sdc                SDC_ENTITY      PATH \
                                                              intel_vvp_remosaic.sdc {NO_SDC_PROMOTION}

add_fileset SIM_VERILOG SIM_VERILOG "" ""
set_fileset_property SIM_VERILOG TOP_LEVEL intel_vvp_remosaic_core
add_fileset_file  src_hdl/intel_vvp_remosaic_cpu.sv     SYSTEM_VERILOG PATH    src_hdl/intel_vvp_remosaic_cpu.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_gen.sv     SYSTEM_VERILOG PATH    src_hdl/intel_vvp_remosaic_gen.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_core.sv    SYSTEM_VERILOG PATH \
                                                              src_hdl/intel_vvp_remosaic_core.sv   TOP_LEVEL_FILE

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL intel_vvp_remosaic_core
add_fileset_file  src_hdl/intel_vvp_remosaic_cpu.sv     SYSTEM_VERILOG PATH    src_hdl/intel_vvp_remosaic_cpu.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_gen.sv     SYSTEM_VERILOG PATH    src_hdl/intel_vvp_remosaic_gen.sv
add_fileset_file  src_hdl/intel_vvp_remosaic_core.sv    SYSTEM_VERILOG PATH \
                                                              src_hdl/intel_vvp_remosaic_core.sv   TOP_LEVEL_FILE


############################################################################
# Module parameters
############################################################################

add_parameter             C_OMNI_CAP_ENABLED        INTEGER           1
set_parameter_property    C_OMNI_CAP_ENABLED        VISIBLE           false
set_parameter_property    C_OMNI_CAP_ENABLED        ALLOWED_RANGES    0:1
set_parameter_property    C_OMNI_CAP_ENABLED        ENABLED           true
set_parameter_property    C_OMNI_CAP_ENABLED        HDL_PARAMETER     false

add_parameter             DEVICE_FAMILY             STRING
set_parameter_property    DEVICE_FAMILY             VISIBLE             false
set_parameter_property    DEVICE_FAMILY             SYSTEM_INFO         {DEVICE_FAMILY}
set_parameter_property    DEVICE_FAMILY             AFFECTS_GENERATION  true
set_parameter_property    DEVICE_FAMILY             HDL_PARAMETER       true
set_parameter_property    DEVICE_FAMILY             ENABLED             true

add_parameter             RUNTIME_CONTROL           INTEGER             1
set_parameter_property    RUNTIME_CONTROL           ALLOWED_RANGES      0:1
set_parameter_property    RUNTIME_CONTROL           DISPLAY_HINT        boolean
set_parameter_property    RUNTIME_CONTROL           HDL_PARAMETER       true
set_parameter_property    RUNTIME_CONTROL           DISPLAY_NAME        "CPU Interface"
set_parameter_property    RUNTIME_CONTROL           DESCRIPTION         "Enables the CPU Interface."
set_parameter_property    RUNTIME_CONTROL           AFFECTS_ELABORATION true

add_parameter             C_CPU_OFFSET              INTEGER             0
set_parameter_property    C_CPU_OFFSET              ALLOWED_RANGES      0:62
set_parameter_property    C_CPU_OFFSET              DISPLAY_UNITS       {[0 - 62] Registers}
set_parameter_property    C_CPU_OFFSET              HDL_PARAMETER       true
set_parameter_property    C_CPU_OFFSET              DISPLAY_NAME        "CPU Register Base Address"
set_parameter_property    C_CPU_OFFSET              DESCRIPTION         "Base Address of internal CPU Registers"
set_parameter_property    C_CPU_OFFSET              AFFECTS_ELABORATION false

add_parameter             NUMBER_OF_COLOR_PLANES    INTEGER             3
set_parameter_property    NUMBER_OF_COLOR_PLANES    ALLOWED_RANGES      {"1" "2" "3"}
set_parameter_property    NUMBER_OF_COLOR_PLANES    HDL_PARAMETER       true
set_parameter_property    NUMBER_OF_COLOR_PLANES    DISPLAY_NAME        "Colour Planes"
set_parameter_property    NUMBER_OF_COLOR_PLANES    DESCRIPTION         "1 to 3 Colour Planes."
set_parameter_property    NUMBER_OF_COLOR_PLANES    AFFECTS_ELABORATION true

add_parameter             PIXELS_IN_PARALLEL        INTEGER             2
set_parameter_property    PIXELS_IN_PARALLEL        ALLOWED_RANGES      {"1" "2" "4" "8"}
set_parameter_property    PIXELS_IN_PARALLEL        HDL_PARAMETER       true
set_parameter_property    PIXELS_IN_PARALLEL        DISPLAY_NAME        "Pixels processed in Parallel"
set_parameter_property    PIXELS_IN_PARALLEL        DESCRIPTION         "1, 2, 4, or 8 Pixels processed in parallel."
set_parameter_property    PIXELS_IN_PARALLEL        AFFECTS_ELABORATION true

add_parameter             BPS                       INTEGER             10
set_parameter_property    BPS                       ALLOWED_RANGES      8:16
set_parameter_property    BPS                       DISPLAY_UNITS       {[8 - 16] bits}
set_parameter_property    BPS                       HDL_PARAMETER       true
set_parameter_property    BPS                       DISPLAY_NAME        "Colour Depth"
set_parameter_property    BPS                       DESCRIPTION         "8 to 16 bits."
set_parameter_property    BPS                       AFFECTS_ELABORATION true

add_parameter             C_CONV_MODE               INTEGER             22
set_parameter_property    C_CONV_MODE               ALLOWED_RANGES \
                                                {"22: BGGR" "148: RGGB" "66: GBBR" "129:RBBG" "41:BRRG" "104:GRRB"}
set_parameter_property    C_CONV_MODE               HDL_PARAMETER       true
set_parameter_property    C_CONV_MODE               DISPLAY_NAME        "Bayer Filter Mosaic Pattern"
set_parameter_property    C_CONV_MODE               DESCRIPTION         "BGGR, RGGB, etc Bayer Filter Pattern."
set_parameter_property    C_CONV_MODE               AFFECTS_ELABORATION false


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
proc omni_add_capability { value version size {v_en 0 } { v_st 0} } {

  add_display_item "" "Capability Info" GROUP tab
  add_display_item "Capability Info" "CapInfo" GROUP

  omni_add_fake_generic  "CapInfo" "Type"                      C_OMNI_CAP_TYPE ${value} 0 1024
  omni_add_fake_generic  "CapInfo" "Version "                  C_OMNI_CAP_VERSION ${version} 1 255
  omni_add_fake_generic  "CapInfo" "Size (32bit Words)"        C_OMNI_CAP_SIZE ${size} 0 1073741824
  omni_add_fake_generic  "CapInfo" "Associated ID"             C_OMNI_CAP_ID_ASSOCIATED 0 0 255
  omni_add_fake_generic  "CapInfo" "Component ID"              C_OMNI_CAP_ID_COMPONENT 0 0 255
  omni_add_fake_generic  "CapInfo" "IRQ Vector (255:disabled)" C_OMNI_CAP_IRQ 255 0 255
  omni_add_fake_generic  "CapInfo" "Tag"                       C_OMNI_CAP_TAG 0 0 255
  if {${v_en}} {
    omni_add_fake_generic  "CapInfo" "IRQ Enable Exists"         C_OMNI_CAP_IRQ_ENABLE_EN 1 0 1
  } else {
    omni_add_fake_generic  "CapInfo" "IRQ Enable Exists"         C_OMNI_CAP_IRQ_ENABLE_EN 0 0 1
  }
  omni_add_fake_generic  "CapInfo" "IRQ Enable Register"       C_OMNI_CAP_IRQ_ENABLE ${v_en} 0 32767
  if {${v_en}} {
    set_parameter_property C_OMNI_CAP_IRQ_ENABLE_EN    ENABLED false
    set_parameter_property C_OMNI_CAP_IRQ_ENABLE       ENABLED false
  }


  if {${v_st}} {
    omni_add_fake_generic  "CapInfo" "IRQ Status Exists"         C_OMNI_CAP_IRQ_STATUS_EN 1 0 1
  } else {
    omni_add_fake_generic  "CapInfo" "IRQ Status Exists"         C_OMNI_CAP_IRQ_STATUS_EN 0 0 1
  }
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

add_display_item        ""                      "Parameters"            GROUP tab

add_display_item        "Parameters"            RUNTIME_CONTROL         parameter
add_display_item        "Parameters"            C_CPU_OFFSET            parameter
add_display_item        "Parameters"            NUMBER_OF_COLOR_PLANES  parameter
add_display_item        "Parameters"            PIXELS_IN_PARALLEL      parameter
add_display_item        "Parameters"            BPS                     parameter
add_display_item        "Parameters"            C_CONV_MODE             parameter


omni_add_capability 581 1 64 0 0


############################################################################
# Interfaces
############################################################################

#AVMM slave interface
add_interface             agent_clock             clock                 end
set_interface_property    agent_clock             clockRate             0
set_interface_property    agent_clock             enabled               true
add_interface_port        agent_clock             agent_clock           clk        Input  1

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

add_interface           axi4s_vid_in          axi4stream              end
set_interface_property  axi4s_vid_in          associatedClock         main_clock
set_interface_property  axi4s_vid_in          associatedReset         main_reset
add_interface_port      axi4s_vid_in          axi4s_vid_in_tvalid     tvalid            Input   1
add_interface_port      axi4s_vid_in          axi4s_vid_in_tready     tready            Output  1
add_interface_port      axi4s_vid_in          axi4s_vid_in_tlast      tlast             Input   1

add_interface           axi4s_vid_out          axi4stream              start
set_interface_property  axi4s_vid_out          associatedClock         main_clock
set_interface_property  axi4s_vid_out          associatedReset         main_reset
add_interface_port      axi4s_vid_out          axi4s_vid_out_tvalid    tvalid            Output  1
add_interface_port      axi4s_vid_out          axi4s_vid_out_tready    tready            Input   1
add_interface_port      axi4s_vid_out          axi4s_vid_out_tlast     tlast             Output  1



############################################################################
# Procedures
############################################################################

proc elaboration_callback {} {

  set v_num_color_planes    [ get_parameter_value   NUMBER_OF_COLOR_PLANES ]
  set v_num_pip             [ get_parameter_value   PIXELS_IN_PARALLEL ]
  set v_bits_per_color      [ get_parameter_value   BPS ]

  set v_s_vid_axis_tdata_width  [expr ( ( ${v_num_color_planes} * ${v_bits_per_color} +7)/8*8) * ${v_num_pip}]
  set v_s_vid_axis_tuser_width  [expr ( ( ${v_num_color_planes} * ${v_bits_per_color} +7)/8) * ${v_num_pip}]

  set v_m_vid_axis_tdata_width  [expr ( ( ${v_bits_per_color} +7)/8*8) * ${v_num_pip}]
  set v_m_vid_axis_tuser_width  [expr ( ( ${v_bits_per_color} +7)/8) * ${v_num_pip}]

  add_interface_port      axi4s_vid_in      axi4s_vid_in_tuser      tuser     Input   ${v_s_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_in      axi4s_vid_in_tdata      tdata     Input   ${v_s_vid_axis_tdata_width}

  add_interface_port      axi4s_vid_out     axi4s_vid_out_tuser     tuser    Output  ${v_m_vid_axis_tuser_width}
  add_interface_port      axi4s_vid_out     axi4s_vid_out_tdata     tdata    Output  ${v_m_vid_axis_tdata_width}

  set v_cpu_en              [ get_parameter_value   RUNTIME_CONTROL ]

  if { ${v_cpu_en} == 1} {
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
