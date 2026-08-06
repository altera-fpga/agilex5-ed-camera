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
 * Module: intel_vvp_exposure_fusion_core
 *
 * Description: Exposure Fusion Core
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_exposure_fusion_core #(
  parameter C_CPU_OFFSET                =  0,

  parameter RUNTIME_CONTROL             =  1, // standard VVP name
  parameter NUMBER_OF_COLOR_PLANES      =  1, // standard VVP name
  parameter PIXELS_IN_PARALLEL          =  2, // standard VVP name
  parameter BPS_IN                      = 12, // standard VVP name
  parameter BPS_OUT                     = 16, // standard VVP name

  localparam OUTPUT_MODE_WIDTH    = 2,
  localparam BPS_MAX              = 16,
  localparam EXPOSURE_RATIO_WIDTH = 17,
  parameter logic [   OUTPUT_MODE_WIDTH-1:0] C_OUTPUT_MODE    = '0,
  parameter logic [             BPS_MAX-1:0] C_BLACK_LEVEL    = '0,
  parameter logic [EXPOSURE_RATIO_WIDTH-1:0] C_EXPOSURE_RATIO = '0,
  parameter logic [             BPS_MAX-1:0] C_THRESHOLD      = '0
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
  axi4s_vid_in_0_tdata,
  axi4s_vid_in_0_tlast,
  axi4s_vid_in_0_tuser,
  axi4s_vid_in_0_tvalid,
  axi4s_vid_in_0_tready,

  axi4s_vid_in_1_tdata,
  axi4s_vid_in_1_tlast,
  axi4s_vid_in_1_tuser,
  axi4s_vid_in_1_tvalid,
  axi4s_vid_in_1_tready,

  // AXI4S VVP Lite Out
  axi4s_vid_out_tdata,
  axi4s_vid_out_tlast,
  axi4s_vid_out_tuser,
  axi4s_vid_out_tvalid,
  axi4s_vid_out_tready
);

  //  Constants  //
  localparam C_AXIS_IN_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*BPS_IN+7)/8*8;
  localparam C_AXIS_IN_WIDTH       = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH      = C_AXIS_IN_WIDTH/8;

  localparam C_AXIS_OUT_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*BPS_OUT+7)/8*8;
  localparam C_AXIS_OUT_WIDTH       = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_OUT_WIDTH      = C_AXIS_OUT_WIDTH/8;


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

  input  logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_in_0_tdata;
  input  logic                          axi4s_vid_in_0_tlast;
  input  logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_in_0_tuser;
  input  logic                          axi4s_vid_in_0_tvalid;
  output logic                          axi4s_vid_in_0_tready;

  input  logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_in_1_tdata;
  input  logic                          axi4s_vid_in_1_tlast;
  input  logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_in_1_tuser;
  input  logic                          axi4s_vid_in_1_tvalid;
  output logic                          axi4s_vid_in_1_tready;

  output logic [C_AXIS_OUT_WIDTH-1:0]   axi4s_vid_out_tdata;
  output logic                          axi4s_vid_out_tlast;
  output logic [C_TUSER_OUT_WIDTH-1:0]  axi4s_vid_out_tuser;
  output logic                          axi4s_vid_out_tvalid;
  input  logic                          axi4s_vid_out_tready;


  //  Signals  //
  logic [   OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode;
  logic [             BPS_MAX-1:0] r_vid_black_level;
  logic [EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio;
  logic [             BPS_MAX-1:0] r_vid_threshold;

  logic [   2*C_AXIS_IN_WIDTH-1:0] axi4s_vid_in_buff_tdata; // double width
  logic                            axi4s_vid_in_buff_tlast;
  logic [    C_TUSER_IN_WIDTH-1:0] axi4s_vid_in_buff_tuser;
  logic                            axi4s_vid_in_buff_tvalid;
  logic                            axi4s_vid_in_buff_tready;

  logic [   2*C_AXIS_IN_WIDTH-1:0] axi4s_vid_in_buff_shim_tdata; // double width
  logic                            axi4s_vid_in_buff_shim_tlast;
  logic [    C_TUSER_IN_WIDTH-1:0] axi4s_vid_in_buff_shim_tuser;
  logic                            axi4s_vid_in_buff_shim_tvalid;
  logic                            axi4s_vid_in_buff_shim_tready;

  //  CPU  //
  intel_vvp_exposure_fusion_cpu #(
    .C_CPU_OFFSET     (C_CPU_OFFSET),
    .C_USE_CPU        (RUNTIME_CONTROL),
    .C_OUTPUT_MODE    (C_OUTPUT_MODE),
    .C_BLACK_LEVEL    (C_BLACK_LEVEL),
    .C_EXPOSURE_RATIO (C_EXPOSURE_RATIO),
    .C_THRESHOLD      (C_THRESHOLD)
  ) u_intel_vvp_exposure_fusion_cpu (
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

    .r_vid_output_mode      (r_vid_output_mode),
    .r_vid_black_level      (r_vid_black_level),
    .r_vid_exposure_ratio   (r_vid_exposure_ratio),
    .r_vid_threshold        (r_vid_threshold)
  );

  //  Line Buffer  //
  intel_vvp_exposure_fusion_buff #(
    .NUMBER_OF_COLOR_PLANES   (NUMBER_OF_COLOR_PLANES),
    .PIXELS_IN_PARALLEL       (PIXELS_IN_PARALLEL),
    .BPS                      (BPS_IN),
    .LINE_BUFFER_DEPTH        (2048)
  ) u_intel_vvp_exposure_fusion_buff (
    .main_clock             (main_clock),
    .main_reset             (main_reset),

    .axi4s_vid_in_0_tdata   (axi4s_vid_in_0_tdata),
    .axi4s_vid_in_0_tlast   (axi4s_vid_in_0_tlast),
    .axi4s_vid_in_0_tuser   (axi4s_vid_in_0_tuser),
    .axi4s_vid_in_0_tvalid  (axi4s_vid_in_0_tvalid),
    .axi4s_vid_in_0_tready  (axi4s_vid_in_0_tready),

    .axi4s_vid_in_1_tdata   (axi4s_vid_in_1_tdata),
    .axi4s_vid_in_1_tlast   (axi4s_vid_in_1_tlast),
    .axi4s_vid_in_1_tuser   (axi4s_vid_in_1_tuser),
    .axi4s_vid_in_1_tvalid  (axi4s_vid_in_1_tvalid),
    .axi4s_vid_in_1_tready  (axi4s_vid_in_1_tready),

    .axi4s_vid_out_tdata    (axi4s_vid_in_buff_tdata),
    .axi4s_vid_out_tlast    (axi4s_vid_in_buff_tlast),
    .axi4s_vid_out_tuser    (axi4s_vid_in_buff_tuser),
    .axi4s_vid_out_tvalid   (axi4s_vid_in_buff_tvalid),
    .axi4s_vid_out_tready   (axi4s_vid_in_buff_tready)
  );

  //  Intermediate Shim  //
  intel_vvp_exposure_fusion_shim #(
    .TDATA_BYTES  (2*C_TUSER_IN_WIDTH)
  ) u_intel_vvp_exposure_fusion_shim (
    .main_clock           (main_clock),
    .main_reset           (main_reset),

    .axi4s_vid_in_tdata   (axi4s_vid_in_buff_tdata),
    .axi4s_vid_in_tlast   (axi4s_vid_in_buff_tlast),
    .axi4s_vid_in_tuser   (axi4s_vid_in_buff_tuser),
    .axi4s_vid_in_tvalid  (axi4s_vid_in_buff_tvalid),
    .axi4s_vid_in_tready  (axi4s_vid_in_buff_tready),

    .axi4s_vid_out_tdata  (axi4s_vid_in_buff_shim_tdata),
    .axi4s_vid_out_tlast  (axi4s_vid_in_buff_shim_tlast),
    .axi4s_vid_out_tuser  (axi4s_vid_in_buff_shim_tuser),
    .axi4s_vid_out_tvalid (axi4s_vid_in_buff_shim_tvalid),
    .axi4s_vid_out_tready (axi4s_vid_in_buff_shim_tready)
  );

  //  Exposure Fusion Generator  //
  intel_vvp_exposure_fusion_gen #(
    .NUMBER_OF_COLOR_PLANES   (NUMBER_OF_COLOR_PLANES),
    .PIXELS_IN_PARALLEL       (PIXELS_IN_PARALLEL),
    .BPS_IN                   (BPS_IN),
    .BPS_OUT                  (BPS_OUT)
  ) u_intel_vvp_exposure_fusion_gen (
    .main_clock             (main_clock),
    .main_reset             (main_reset),

    .axi4s_vid_in_tdata     (axi4s_vid_in_buff_shim_tdata),
    .axi4s_vid_in_tlast     (axi4s_vid_in_buff_shim_tlast),
    .axi4s_vid_in_tuser     (axi4s_vid_in_buff_shim_tuser),
    .axi4s_vid_in_tvalid    (axi4s_vid_in_buff_shim_tvalid),
    .axi4s_vid_in_tready    (axi4s_vid_in_buff_shim_tready),

    .axi4s_vid_out_tdata    (axi4s_vid_out_tdata),
    .axi4s_vid_out_tlast    (axi4s_vid_out_tlast),
    .axi4s_vid_out_tuser    (axi4s_vid_out_tuser),
    .axi4s_vid_out_tvalid   (axi4s_vid_out_tvalid),
    .axi4s_vid_out_tready   (axi4s_vid_out_tready),

    .r_vid_output_mode      (r_vid_output_mode),
    .r_vid_black_level      (r_vid_black_level),
    .r_vid_exposure_ratio   (r_vid_exposure_ratio),
    .r_vid_threshold        (r_vid_threshold)
  );

endmodule

`default_nettype wire
