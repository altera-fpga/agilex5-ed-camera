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
 * Module: intel_vvp_exposure_fusion_cpu
 *
 * Description: CPU Interface for the Exposure Fusion
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_exposure_fusion_cpu #(
  parameter C_CPU_OFFSET    = 0,
  parameter C_USE_CPU       = 1,

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
  av_address,
  av_read,
  av_readdata,
  av_readdatavalid,
  av_waitrequest,
  av_write,
  av_writedata,
  av_byteenable,

  // Video Clock and Reset
  main_clock,
  main_reset,

  // Video Control Regs
  r_vid_output_mode,
  r_vid_black_level,
  r_vid_exposure_ratio,
  r_vid_threshold
);


  //  Constants  //
  localparam C_REG_VER            = C_CPU_OFFSET + 0;
  localparam C_REG_CONFIG         = C_CPU_OFFSET + 1;
  localparam C_REG_BLACK_LEVEL    = C_CPU_OFFSET + 2;
  localparam C_REG_EXPOSURE_RATIO = C_CPU_OFFSET + 3;
  localparam C_REG_THRESHOLD      = C_CPU_OFFSET + 4;

  //  Top Level Signals  //
  input   logic                            agent_clock;
  input   logic                            agent_reset;

  input   logic [                     5:0] av_address;
  input   logic                            av_read;
  output  logic [                    31:0] av_readdata;
  output  logic                            av_readdatavalid;
  output  logic                            av_waitrequest;
  input   logic                            av_write;
  input   logic [                    31:0] av_writedata;
  input   logic [                     3:0] av_byteenable;

  input   logic                            main_clock;
  input   logic                            main_reset;

  output  logic [   OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode;
  output  logic [             BPS_MAX-1:0] r_vid_black_level;
  output  logic [EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio;
  output  logic [             BPS_MAX-1:0] r_vid_threshold;


  //  Signals  //
  logic [ 5:0] r_av_address;
  logic        r_av_read;
  logic        r_av_write;
  logic [31:0] r_av_writedata;
  logic [ 3:0] r_av_byteenable;

  logic [    OUTPUT_MODE_WIDTH-1:0] r_cpu_output_mode;
  logic [    OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode_meta;
  logic [    OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode_safe;
  logic [    OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode_safe_d1;

  logic [              BPS_MAX-1:0] r_cpu_black_level;
  logic [              BPS_MAX-1:0] r_vid_black_level_meta;
  logic [              BPS_MAX-1:0] r_vid_black_level_safe;
  logic [              BPS_MAX-1:0] r_vid_black_level_safe_d1;

  logic [ EXPOSURE_RATIO_WIDTH-1:0] r_cpu_exposure_ratio;
  logic [ EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio_meta;
  logic [ EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio_safe;
  logic [ EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio_safe_d1;

  logic [              BPS_MAX-1:0] r_cpu_threshold;
  logic [              BPS_MAX-1:0] r_vid_threshold_meta;
  logic [              BPS_MAX-1:0] r_vid_threshold_safe;
  logic [              BPS_MAX-1:0] r_vid_threshold_safe_d1;

  //  Tasks  //
  //-----------------------------------------------------------------
  task t_cpu_write;
    input        av_write;
    input [ 3:0] av_byteenable;
    input [31:0] av_writedata;
    inout [31:0] av_new_writedata;

    if (av_write) begin
      if (av_byteenable[0])
        av_new_writedata[ 7: 0] = av_writedata[ 7: 0];
      if (av_byteenable[1])
        av_new_writedata[15: 8] = av_writedata[15: 8];
      if (av_byteenable[2])
        av_new_writedata[23:16] = av_writedata[23:16];
      if (av_byteenable[3])
        av_new_writedata[31:24] = av_writedata[31:24];
    end
  endtask
//-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Register CPU Interface
  always_ff @(posedge agent_clock) begin : a_reg_cpu_if
    r_av_address    <= av_address;
    r_av_read       <= av_read;
    r_av_write      <= av_write;
    r_av_writedata  <= av_writedata;
    r_av_byteenable <= av_byteenable;
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Register CPU Interface
  always_ff @(posedge agent_clock) begin : a_make_cpu_if
    logic [31:0]    nb_local_reg32;

    av_waitrequest <= 1'b0;
    if (C_USE_CPU) begin
      av_readdatavalid <= r_av_read;
      av_readdata    <= 32'b0;
      nb_local_reg32 = 32'b0;
      case (r_av_address)
        C_REG_VER           : begin
                                av_readdata   <= 32'hBEEF_F00D;
                              end
        C_REG_CONFIG        : begin
                                nb_local_reg32[OUTPUT_MODE_WIDTH-1:0] = r_cpu_output_mode;
                                av_readdata <= nb_local_reg32;
                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_output_mode <= nb_local_reg32[OUTPUT_MODE_WIDTH-1:0];
                              end
        C_REG_BLACK_LEVEL   : begin
                                nb_local_reg32[BPS_MAX-1:0] = r_cpu_black_level;
                                av_readdata <= nb_local_reg32;
                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_black_level <= nb_local_reg32[BPS_MAX-1:0];
                              end
        C_REG_EXPOSURE_RATIO: begin
                                nb_local_reg32[EXPOSURE_RATIO_WIDTH-1:0] = r_cpu_exposure_ratio;
                                av_readdata <= nb_local_reg32;
                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_exposure_ratio <= nb_local_reg32[EXPOSURE_RATIO_WIDTH-1:0];
                              end
        C_REG_THRESHOLD     : begin
                                nb_local_reg32[BPS_MAX-1:0] = r_cpu_threshold;
                                av_readdata <= nb_local_reg32;
                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_threshold <= nb_local_reg32[BPS_MAX-1:0];
                              end
        default             : begin // Address out of range
                                av_readdata <= 32'h1234_ABCD;
                              end
      endcase
    end

    if (agent_reset) begin
      r_cpu_output_mode         <= C_OUTPUT_MODE;
      r_cpu_black_level         <= C_BLACK_LEVEL;
      r_cpu_exposure_ratio      <= C_EXPOSURE_RATIO;
      r_cpu_threshold           <= C_THRESHOLD;

      // cpu sigs
      av_waitrequest            <= 1'b1;
      av_readdata               <= 32'b0;
      av_readdatavalid          <= 1'b0;
    end

  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // CDC CPU Interface
  always_ff @(posedge main_clock) begin : a_cdc_cpu_if

    r_vid_output_mode_meta    <= r_cpu_output_mode;
    r_vid_output_mode_safe    <= r_vid_output_mode_meta;
    r_vid_output_mode_safe_d1 <= r_vid_output_mode_safe;
    if (r_vid_output_mode_safe == r_vid_output_mode_safe_d1) begin
      r_vid_output_mode <= r_vid_output_mode_safe_d1;
    end

    r_vid_black_level_meta    <= r_cpu_black_level;
    r_vid_black_level_safe    <= r_vid_black_level_meta;
    r_vid_black_level_safe_d1 <= r_vid_black_level_safe;
    if (r_vid_black_level_safe == r_vid_black_level_safe_d1) begin
      r_vid_black_level <= r_vid_black_level_safe_d1;
    end

    r_vid_exposure_ratio_meta    <= r_cpu_exposure_ratio;
    r_vid_exposure_ratio_safe    <= r_vid_exposure_ratio_meta;
    r_vid_exposure_ratio_safe_d1 <= r_vid_exposure_ratio_safe;
    if (r_vid_exposure_ratio_safe == r_vid_exposure_ratio_safe_d1) begin
      r_vid_exposure_ratio <= r_vid_exposure_ratio_safe_d1;
    end

    r_vid_threshold_meta    <= r_cpu_threshold;
    r_vid_threshold_safe    <= r_vid_threshold_meta;
    r_vid_threshold_safe_d1 <= r_vid_threshold_safe;
    if (r_vid_threshold_safe == r_vid_threshold_safe_d1) begin
      r_vid_threshold <= r_vid_threshold_safe_d1;
    end

    if (main_reset) begin
      r_vid_output_mode    <= C_OUTPUT_MODE;
      r_vid_black_level    <= C_BLACK_LEVEL;
      r_vid_exposure_ratio <= C_EXPOSURE_RATIO;
      r_vid_threshold      <= C_THRESHOLD;
    end

  end
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
