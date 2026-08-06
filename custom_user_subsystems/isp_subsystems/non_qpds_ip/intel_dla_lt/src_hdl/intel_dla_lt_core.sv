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
 * Module: intel_dla_lt_core
 *
 * Description: Layout Transform Core
 *
 * ##################################################################################
*/

`default_nettype none

module intel_dla_lt_core #(
  parameter DEVICE_FAMILY                           = "Arria 10", // -- Cyclone 10 GX -- Arria 10 -- Stratix 10 -- Agilex
  parameter C_CPU_OFFSET                            = 0,

  parameter RUNTIME_CONTROL                         = 1,  // standard VVP name

  parameter NUMBER_OF_COLOR_PLANES_IN               = 3,  // standard VVP name
  parameter PIXELS_IN_PARALLEL_IN                   = 1,  // standard VVP name
  parameter BPS_IN                                  = 8,  // standard VVP name

  parameter NUMBER_OF_COLOR_PLANES_OUT              = 4,  // standard VVP name
  parameter PIXELS_IN_PARALLEL_OUT                  = 4,  // standard VVP name
  parameter BPS_OUT                                 = 16, // standard VVP name

  parameter logic [15:0] C_V_LINES                  = 16'h00B2,       // real v lines minus 2
  parameter logic [15:0] C_DC_FP16_PIX_VAL          = {1'b0 , 5'b10110 , 10'b0},  // (Sign, Exponent, Mantissa) = 128
  parameter logic [15:0] C_DF_FP16_PIX_VAL          = {1'b0 , 5'b0 , 10'b0},      // (Sign, Exponent, Mantissa) = 0
  parameter logic        C_RB_SWAP                  = 1'b0        // 1 = swap Red and Blue
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

  // Data In
  axi4s_vid_in_tdata,
  axi4s_vid_in_tlast,
  axi4s_vid_in_tuser,
  axi4s_vid_in_tvalid,
  axi4s_vid_in_tready,

  // Line Store Control Out
  fifo_reset,

  // Line Store Data Out
  axi4s_vid_fifo_out_tdata,
  axi4s_vid_fifo_out_tlast,
  axi4s_vid_fifo_out_tuser,
  axi4s_vid_fifo_out_tvalid,
  axi4s_vid_fifo_out_tready,

  // Line Store Data In
  axi4s_vid_fifo_in_tdata,
  axi4s_vid_fifo_in_tlast,
  axi4s_vid_fifo_in_tuser,
  axi4s_vid_fifo_in_tvalid,
  axi4s_vid_fifo_in_tready,

  // Data Out
  axi4s_vid_out_tdata,
  axi4s_vid_out_tlast,
  axi4s_vid_out_tuser,
  axi4s_vid_out_tvalid,
  axi4s_vid_out_tready
);



  //  Constants  //
  localparam C_AXIS_IN_PIXEL_BYTES    = NUMBER_OF_COLOR_PLANES_IN * BPS_IN;
  localparam C_AXIS_IN_WIDTH          = C_AXIS_IN_PIXEL_BYTES * PIXELS_IN_PARALLEL_IN;
  localparam C_TUSER_IN_WIDTH         = C_AXIS_IN_WIDTH / 8;

  localparam C_AXIS_OUT_PIXEL_BYTES   = NUMBER_OF_COLOR_PLANES_OUT * BPS_OUT;
  localparam C_AXIS_OUT_WIDTH         = C_AXIS_OUT_PIXEL_BYTES * PIXELS_IN_PARALLEL_OUT;
  localparam C_TUSER_OUT_WIDTH        = C_AXIS_OUT_WIDTH / 8;


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

  output logic                          fifo_reset;

  output logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_fifo_out_tdata;
  output logic                          axi4s_vid_fifo_out_tlast;
  output logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_fifo_out_tuser;
  output logic                          axi4s_vid_fifo_out_tvalid;
  input  logic                          axi4s_vid_fifo_out_tready;

  input  logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_fifo_in_tdata;
  input  logic                          axi4s_vid_fifo_in_tlast;
  input  logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_fifo_in_tuser;
  input  logic                          axi4s_vid_fifo_in_tvalid;
  output logic                          axi4s_vid_fifo_in_tready;

  output logic [C_AXIS_OUT_WIDTH-1:0]   axi4s_vid_out_tdata;
  output logic                          axi4s_vid_out_tlast;
  output logic [C_TUSER_OUT_WIDTH-1:0]  axi4s_vid_out_tuser;
  output logic                          axi4s_vid_out_tvalid;
  input  logic                          axi4s_vid_out_tready;


  //  Signals  //
  logic [15:0]                r_vid_v_lines;
  logic                       r_resync_en;
  logic [15:0]                r_vid_dc_fp16_pix_val;
  logic [15:0]                r_vid_df_fp16_pix_val;
  logic                       r_rb_swap;
  logic [63:0]                dbg_counters;
  logic [ 3:0]                dbg_errs;

  assign fifo_reset = r_resync_en;


  //  CPU  //
  intel_dla_lt_cpu #(
      .C_CPU_OFFSET                 (C_CPU_OFFSET),
      .C_USE_CPU                    (RUNTIME_CONTROL),
      .C_V_LINES                    (C_V_LINES),
      .C_DC_FP16_PIX_VAL            (C_DC_FP16_PIX_VAL),
      .C_DF_FP16_PIX_VAL            (C_DF_FP16_PIX_VAL) )
  u_intel_dla_lt_cpu (
      .agent_clock                  (agent_clock),
      .agent_reset                  (agent_reset),
      .av_address                   (av_mm_cpu_agent_address),
      .av_read                      (av_mm_cpu_agent_read),
      .av_readdata                  (av_mm_cpu_agent_readdata),
      .av_readdatavalid             (av_mm_cpu_agent_readdatavalid),
      .av_waitrequest               (av_mm_cpu_agent_waitrequest),
      .av_write                     (av_mm_cpu_agent_write),
      .av_writedata                 (av_mm_cpu_agent_writedata),
      .av_byteenable                (av_mm_cpu_agent_byteenable),
      .main_clock                   (main_clock),
      .main_reset                   (main_reset),
      .r_vid_v_lines                (r_vid_v_lines),
      .r_resync_en                  (r_resync_en),
      .r_vid_dc_fp16_pix_val        (r_vid_dc_fp16_pix_val),
      .r_vid_df_fp16_pix_val        (r_vid_df_fp16_pix_val),
      .r_rb_swap                    (r_rb_swap),
      .dbg_counters                 (dbg_counters),
      .dbg_errs                     (dbg_errs) );


  //  Generator  //
  intel_dla_lt_gen #(
      .C_USE_CPU                    (RUNTIME_CONTROL),
      .NUMBER_OF_COLOR_PLANES_IN    (NUMBER_OF_COLOR_PLANES_IN),
      .PIXELS_IN_PARALLEL_IN        (PIXELS_IN_PARALLEL_IN),
      .BPS_IN                       (BPS_IN),
      .NUMBER_OF_COLOR_PLANES_OUT   (NUMBER_OF_COLOR_PLANES_OUT),
      .PIXELS_IN_PARALLEL_OUT       (PIXELS_IN_PARALLEL_OUT),
      .BPS_OUT                      (BPS_OUT),
      .C_V_LINES                    (C_V_LINES),
      .C_DC_FP16_PIX_VAL            (C_DC_FP16_PIX_VAL),
      .C_DF_FP16_PIX_VAL            (C_DF_FP16_PIX_VAL) )
  u_intel_dla_lt_gen (
      .main_clock                   (main_clock),
      .main_reset                   (main_reset),
      .axi4s_vid_in_tdata           (axi4s_vid_in_tdata),
      .axi4s_vid_in_tlast           (axi4s_vid_in_tlast),
      .axi4s_vid_in_tuser           (axi4s_vid_in_tuser),
      .axi4s_vid_in_tvalid          (axi4s_vid_in_tvalid),
      .axi4s_vid_in_tready          (axi4s_vid_in_tready),
      .axi4s_vid_fifo_out_tdata     (axi4s_vid_fifo_out_tdata),
      .axi4s_vid_fifo_out_tlast     (axi4s_vid_fifo_out_tlast),
      .axi4s_vid_fifo_out_tuser     (axi4s_vid_fifo_out_tuser),
      .axi4s_vid_fifo_out_tvalid    (axi4s_vid_fifo_out_tvalid),
      .axi4s_vid_fifo_out_tready    (axi4s_vid_fifo_out_tready),
      .axi4s_vid_fifo_in_tdata      (axi4s_vid_fifo_in_tdata),
      .axi4s_vid_fifo_in_tlast      (axi4s_vid_fifo_in_tlast),
      .axi4s_vid_fifo_in_tuser      (axi4s_vid_fifo_in_tuser),
      .axi4s_vid_fifo_in_tvalid     (axi4s_vid_fifo_in_tvalid),
      .axi4s_vid_fifo_in_tready     (axi4s_vid_fifo_in_tready),
      .axi4s_vid_out_tdata          (axi4s_vid_out_tdata),
      .axi4s_vid_out_tlast          (axi4s_vid_out_tlast),
      .axi4s_vid_out_tuser          (axi4s_vid_out_tuser),
      .axi4s_vid_out_tvalid         (axi4s_vid_out_tvalid),
      .axi4s_vid_out_tready         (axi4s_vid_out_tready),
      .r_vid_v_lines                (r_vid_v_lines),
      .r_resync_en                  (r_resync_en),
      .r_vid_dc_fp16_pix_val        (r_vid_dc_fp16_pix_val),
      .r_vid_df_fp16_pix_val        (r_vid_df_fp16_pix_val),
      .r_rb_swap                    (r_rb_swap),
      .dbg_counters                 (dbg_counters),
      .dbg_errs                     (dbg_errs) );

endmodule

`default_nettype wire
