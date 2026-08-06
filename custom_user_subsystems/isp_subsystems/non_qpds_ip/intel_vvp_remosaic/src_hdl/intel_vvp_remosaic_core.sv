/* ##################################################################################
 * Copyright (C) 2025 Altera Corporation
 *
 * This software and the related documents are Altera copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this software
 * or the related documents without Altera's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the License.
 * ##################################################################################

 * ##################################################################################
 *
 * Module: intel_vvp_remosaic_core
 *
 * Description: Remosaic Core
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_remosaic_core #(
  parameter DEVICE_FAMILY                           = "Arria 10", // -- Cyclone 10 GX -- Arria 10 -- Stratix 10 -- Agilex
  parameter C_CPU_OFFSET                            = 0,
  parameter RUNTIME_CONTROL                         =  1, // standard VVP name
  parameter NUMBER_OF_COLOR_PLANES                  =  3, // standard VVP name
  parameter PIXELS_IN_PARALLEL                      =  2, // standard VVP name
  parameter BPS                                     = 10, // standard VVP name
  parameter logic [7:0] C_CONV_MODE                 = 8'b00010110
) (
  // CPU Clock and Reset
  agent_clock,
  agent_reset,

  // CPU Avalon Interface
  av_mm_cpu_agent_address,
  av_mm_cpu_agent_read,
  av_mm_cpu_agent_readdata,
  av_mm_cpu_agent_readdatavalid,
  av_mm_cpu_agent_waitrequest,
  av_mm_cpu_agent_write,
  av_mm_cpu_agent_writedata,
  av_mm_cpu_agent_byteenable,

  // Video Clock and Reset
  main_clock,
  main_reset,

  // AXI4S VVP Lite In
  axi4s_vid_in_tdata,
  axi4s_vid_in_tlast,
  axi4s_vid_in_tuser,
  axi4s_vid_in_tvalid,
  axi4s_vid_in_tready,

  // AXI4S VVP Lite Out
  axi4s_vid_out_tdata,
  axi4s_vid_out_tlast,
  axi4s_vid_out_tuser,
  axi4s_vid_out_tvalid,
  axi4s_vid_out_tready
);


  //  Constants  //
  localparam C_PADDED_BPS       = (BPS < 8) ? 8 : BPS;

  localparam C_AXIS_IN_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*C_PADDED_BPS+7)/8*8;
  localparam C_AXIS_IN_WIDTH       = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH      = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL/8;

  localparam C_AXIS_OUT_PIXEL_BYTES = (C_PADDED_BPS+7)/8*8;
  localparam C_AXIS_OUT_WIDTH       = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_OUT_WIDTH      = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL/8;


  //  Top Level Signals  //
  input  logic                          agent_clock;
  input  logic                          agent_reset;

  input  logic [ 5:0]                   av_mm_cpu_agent_address;
  input  logic                          av_mm_cpu_agent_read;
  output logic [31:0]                   av_mm_cpu_agent_readdata;
  output logic                          av_mm_cpu_agent_readdatavalid;
  output logic                          av_mm_cpu_agent_waitrequest;
  input  logic                          av_mm_cpu_agent_write;
  input  logic [31:0]                   av_mm_cpu_agent_writedata;
  input  logic [ 3:0]                   av_mm_cpu_agent_byteenable;

  input  logic                          main_clock;
  input  logic                          main_reset;

  input  logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_in_tdata;
  input  logic                          axi4s_vid_in_tlast;
  input  logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_in_tuser;
  input  logic                          axi4s_vid_in_tvalid;
  output logic                          axi4s_vid_in_tready;

  output logic [C_AXIS_OUT_WIDTH-1:0]   axi4s_vid_out_tdata;
  output logic                          axi4s_vid_out_tlast;
  output logic [C_TUSER_OUT_WIDTH-1:0]  axi4s_vid_out_tuser;
  output logic                          axi4s_vid_out_tvalid;
  input  logic                          axi4s_vid_out_tready;


  //  Signals  //
  logic [7:0]         r_vid_conv_mode;


  //  CPU  //
  intel_vvp_remosaic_cpu #(
      .C_CPU_OFFSET     (C_CPU_OFFSET),
      .C_USE_CPU        (RUNTIME_CONTROL),
      .C_CONV_MODE      (C_CONV_MODE) )
  u_intel_vvp_remosaic_cpu (
      .agent_clock            (agent_clock),
      .agent_reset            (agent_reset),
      .av_address             (av_mm_cpu_agent_address),
      .av_read                (av_mm_cpu_agent_read),
      .av_readdata            (av_mm_cpu_agent_readdata),
      .av_readdatavalid       (av_mm_cpu_agent_readdatavalid),
      .av_waitrequest         (av_mm_cpu_agent_waitrequest),
      .av_write               (av_mm_cpu_agent_write),
      .av_writedata           (av_mm_cpu_agent_writedata),
      .av_byteenable          (av_mm_cpu_agent_byteenable),
      .main_clock             (main_clock),
      .main_reset             (main_reset),
      .r_vid_conv_mode        (r_vid_conv_mode) );


  //  Remosaic Generator  //
  intel_vvp_remosaic_gen #(
      .C_USE_CPU                (RUNTIME_CONTROL),
      .NUMBER_OF_COLOR_PLANES   (NUMBER_OF_COLOR_PLANES),
      .PIXELS_IN_PARALLEL       (PIXELS_IN_PARALLEL),
      .BPS                      (BPS),
      .C_CONV_MODE              (C_CONV_MODE) )
  u_intel_vvp_remosaic_gen (
      .main_clock             (main_clock),
      .main_reset             (main_reset),
      .axi4s_vid_in_tdata     (axi4s_vid_in_tdata),
      .axi4s_vid_in_tlast     (axi4s_vid_in_tlast),
      .axi4s_vid_in_tuser     (axi4s_vid_in_tuser),
      .axi4s_vid_in_tvalid    (axi4s_vid_in_tvalid),
      .axi4s_vid_in_tready    (axi4s_vid_in_tready),
      .axi4s_vid_out_tdata    (axi4s_vid_out_tdata),
      .axi4s_vid_out_tlast    (axi4s_vid_out_tlast),
      .axi4s_vid_out_tuser    (axi4s_vid_out_tuser),
      .axi4s_vid_out_tvalid   (axi4s_vid_out_tvalid),
      .axi4s_vid_out_tready   (axi4s_vid_out_tready),
      .r_vid_conv_mode        (r_vid_conv_mode) );

endmodule

`default_nettype wire
