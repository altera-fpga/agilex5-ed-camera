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
 * Module: intel_vvp_remosaic_cpu
 *
 * Description: CPU interface for Remosaic
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_remosaic_cpu #(
  parameter C_CPU_OFFSET                            = 0,
  parameter C_USE_CPU                               = 1,
  parameter logic [7:0] C_CONV_MODE                 = 8'b00010110
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
  r_vid_conv_mode
);


  //  Constants  //
  localparam C_REG_VER          = C_CPU_OFFSET + 0;
  localparam C_REG_CONFIG       = C_CPU_OFFSET + 1;


  //  Top Level Signals  //
  input   logic                       agent_clock;
  input   logic                       agent_reset;

  input   logic [ 5:0]                av_address;
  input   logic                       av_read;
  output  logic [31:0]                av_readdata;
  output  logic                       av_readdatavalid;
  output  logic                       av_waitrequest;
  input   logic                       av_write;
  input   logic [31:0]                av_writedata;
  input   logic [ 3:0]                av_byteenable;

  input   logic                       main_clock;
  input   logic                       main_reset;

  output  logic [7:0]                 r_vid_conv_mode;



  //  Signals  //
  logic [ 5:0]                r_av_address;
  logic                       r_av_read;
  logic                       r_av_write;
  logic [31:0]                r_av_writedata;
  logic [ 3:0]                r_av_byteenable;

  logic [7:0]                 r_cpu_conv_mode;

  logic [7:0]                 r_vid_conv_mode_meta;
  logic [7:0]                 r_vid_conv_mode_safe;
  logic [7:0]                 r_vid_conv_mode_safe_d1;


  //  Tasks  //
  //-----------------------------------------------------------------
  task t_cpu_write;
    input         av_write;
    input [ 3:0]  av_byteenable;
    input [31:0]  av_writedata;
    inout [31:0]  av_new_writedata;

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
    r_av_address      <= av_address;
    r_av_read         <= av_read;
    r_av_write        <= av_write;
    r_av_writedata    <= av_writedata;
    r_av_byteenable   <= av_byteenable;
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Register CPU Interface
  always_ff @(posedge agent_clock) begin : a_make_cpu_if
    logic [31:0]    nb_local_reg32;

    av_waitrequest            <= 1'b0;
    if (C_USE_CPU) begin
      av_readdatavalid    <= r_av_read;
      av_readdata         <= 32'b0;
      nb_local_reg32      = 32'b0;
      case (r_av_address)
        C_REG_VER           : begin
                                av_readdata   <= 32'hBEEF_F00D;
                              end
        C_REG_CONFIG        : begin
                                nb_local_reg32[7:0]     = r_cpu_conv_mode;
                                av_readdata             <= nb_local_reg32;
                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_conv_mode         <= nb_local_reg32[7:0];
                              end
        default             : begin // Address out of range
                                av_readdata     <= 32'h1234_ABCD;
                              end
      endcase
    end

    if (agent_reset) begin
      r_cpu_conv_mode           <= C_CONV_MODE;

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

    r_vid_conv_mode_meta      <= r_cpu_conv_mode;
    r_vid_conv_mode_safe      <= r_vid_conv_mode_meta;
    r_vid_conv_mode_safe_d1   <= r_vid_conv_mode_safe;

    if (r_vid_conv_mode_safe == r_vid_conv_mode_safe_d1) begin
      r_vid_conv_mode   <= r_vid_conv_mode_safe_d1;
    end

    if (main_reset) begin
      r_vid_conv_mode   <= C_CONV_MODE;
    end

  end
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
