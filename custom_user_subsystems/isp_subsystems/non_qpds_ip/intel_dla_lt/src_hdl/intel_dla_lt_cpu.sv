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
 * Module: intel_dla_lt_cpu
 *
 * Description: CPU interface for Layout Transform
 *
 * ##################################################################################
*/

`default_nettype none

module intel_dla_lt_cpu #(
  parameter C_CPU_OFFSET                            = 0,
  parameter C_USE_CPU                               = 1,
  parameter logic [15:0] C_V_LINES                  = 16'h00B2,
  parameter logic [15:0] C_DC_FP16_PIX_VAL          = {1'b0 , 5'b10110 , 10'b0},  // (Sign, Exponent, Mantissa) = 128
  parameter logic [15:0] C_DF_FP16_PIX_VAL          = {1'b0 , 5'b0 , 10'b0},      // (Sign, Exponent, Mantissa) = 0
  parameter logic        C_RB_SWAP                  = 1'b0        // 1 = swap Red and Blue
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
  r_vid_v_lines,
  r_resync_en,
  r_vid_dc_fp16_pix_val,
  r_vid_df_fp16_pix_val,
  r_rb_swap,

  // Debug
  dbg_counters,
  dbg_errs
);


  //  Constants  //
  localparam C_REG_VER                  = C_CPU_OFFSET + 0;
  localparam C_REG_CONTROL              = C_CPU_OFFSET + 1;
  localparam C_REG_PIX_VAL              = C_CPU_OFFSET + 2;
  localparam C_REG_FRAME_IN_STAT        = C_CPU_OFFSET + 3;
  localparam C_REG_FRAME_OUT_STAT       = C_CPU_OFFSET + 4;
  localparam C_REG_FRAME_ERR_STAT       = C_CPU_OFFSET + 5;
  localparam C_REG_FRAME_ERR_IN_STAT    = C_CPU_OFFSET + 6;
  localparam C_REG_FRAME_ERR_OUT_STAT   = C_CPU_OFFSET + 7;


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

  output  logic [15:0]                r_vid_v_lines;
  output  logic                       r_resync_en;
  output  logic [15:0]                r_vid_dc_fp16_pix_val;
  output  logic [15:0]                r_vid_df_fp16_pix_val;
  output  logic                       r_rb_swap;

  input   logic [63:0]                dbg_counters;
  input   logic [ 3:0]                dbg_errs;



  //  Signals  //
  logic [ 5:0]                r_av_address;
  logic                       r_av_read;
  logic                       r_av_write;
  logic [31:0]                r_av_writedata;
  logic [ 3:0]                r_av_byteenable;

  logic [31:0]                r_cpu_control;
  logic [31:0]                r_cpu_fp16_pix_val;

  logic [31:0]                r_vid_control_meta;
  logic [31:0]                r_vid_control_safe;
  logic [31:0]                r_vid_fp16_pix_val_meta;
  logic [31:0]                r_vid_fp16_pix_val_safe;

  logic [63:0]                dbg_counters_meta;
  logic [63:0]                dbg_counters_safe;
  logic [ 3:0]                dbg_errs_meta;
  logic [ 3:0]                dbg_errs_safe;
  logic [ 3:0]                dbg_errs_safe_d1;
  logic [ 3:0]                dbg_errs_safe_d2;

  logic [ 3:0]                clr_dbg_err;
  logic [63:0]                dbg_err_counter;
  logic [ 3:0]                dbg_err_en;


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
      clr_dbg_err         <= 4'b0;
      case (r_av_address)
        C_REG_VER           : begin
                                av_readdata   <= 32'hABBA_FEED;
                              end
        C_REG_CONTROL       : begin
                                nb_local_reg32[31:0]    = r_cpu_control;
                                av_readdata             <= nb_local_reg32;

                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_control           <= nb_local_reg32[31:0];
                              end
        C_REG_PIX_VAL       : begin
                                nb_local_reg32[31:0]    = r_cpu_fp16_pix_val;
                                av_readdata             <= nb_local_reg32;

                                t_cpu_write(r_av_write , r_av_byteenable , r_av_writedata , nb_local_reg32);
                                r_cpu_fp16_pix_val      <= nb_local_reg32[31:0];
                              end
        C_REG_FRAME_IN_STAT : begin
                                av_readdata     <= {dbg_counters_safe[16*1+15:16*1+0] , dbg_counters_safe[16*0+15:16*0+0]};
                              end
        C_REG_FRAME_OUT_STAT : begin
                                av_readdata     <= {dbg_counters_safe[16*3+15:16*3+0] , dbg_counters_safe[16*2+15:16*2+0]};
                              end
        C_REG_FRAME_ERR_STAT : begin
                                av_readdata     <= {27'b0 , dbg_err_en};
                              end
        C_REG_FRAME_ERR_IN_STAT : begin
                                av_readdata     <= {dbg_err_counter[16*1+15:16*1+0] , dbg_err_counter[16*0+15:16*0+0]};
                                clr_dbg_err[0]     <= 1'b1;
                                clr_dbg_err[1]     <= 1'b1;
                              end
        C_REG_FRAME_ERR_OUT_STAT : begin
                                av_readdata     <= {dbg_err_counter[16*3+15:16*3+0] , dbg_err_counter[16*2+15:16*2+0]};
                                clr_dbg_err[2]     <= 1'b1;
                                clr_dbg_err[3]     <= 1'b1;
                              end
        default             : begin // Address out of range
                                av_readdata     <= 32'h1234_ABCD;
                              end
      endcase
    end

    if (agent_reset) begin
      r_cpu_control             <= {16'b0 , C_V_LINES};
      r_cpu_fp16_pix_val        <= {C_DF_FP16_PIX_VAL, C_DC_FP16_PIX_VAL};

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

    r_vid_control_meta        <= r_cpu_control;
    r_vid_control_safe        <= r_vid_control_meta;

    r_vid_fp16_pix_val_meta    <= r_cpu_fp16_pix_val;
    r_vid_fp16_pix_val_safe    <= r_vid_fp16_pix_val_meta;

    if (main_reset) begin
      r_vid_control_meta            <= {16'b0 , C_V_LINES};
      r_vid_control_safe            <= {16'b0 , C_V_LINES};

      r_vid_fp16_pix_val_meta        <= {C_DF_FP16_PIX_VAL, C_DC_FP16_PIX_VAL};
      r_vid_fp16_pix_val_safe        <= {C_DF_FP16_PIX_VAL, C_DC_FP16_PIX_VAL};
    end

  end
  //-----------------------------------------------------------------


  // assign outputs
  assign r_vid_v_lines            = r_vid_control_safe[15:0];
  assign r_rb_swap                = r_vid_control_safe[24];
  assign r_resync_en              = r_vid_control_safe[31];

  assign r_vid_dc_fp16_pix_val    = r_vid_fp16_pix_val_safe[15:0];
  assign r_vid_df_fp16_pix_val    = r_vid_fp16_pix_val_safe[31:16];


  //-----------------------------------------------------------------
  // CDC CPU Interface
  //-----------------------------------------------------------------
  always_ff @(posedge agent_clock) begin : a_cdc_to_cpu_if

    dbg_counters_meta        <= dbg_counters;
    dbg_counters_safe        <= dbg_counters_meta;

    dbg_errs_meta     <= dbg_errs;
    dbg_errs_safe     <= dbg_errs_meta;
    dbg_errs_safe_d1  <= dbg_errs_safe;
    dbg_errs_safe_d2  <= dbg_errs_safe_d1;

    if (agent_reset) begin
      dbg_errs_meta            <= 4'b0;
      dbg_errs_safe            <= 4'b0;
      dbg_errs_safe_d1         <= 4'b0;
      dbg_errs_safe_d2         <= 4'b0;
    end

  end
  //-----------------------------------------------------------------

  //-----------------------------------------------------------------
  genvar error_indx;
  generate

    // Step through errors (4)
    for (error_indx = 0; error_indx < 4; error_indx++) begin

      always_ff @(posedge agent_clock) begin

        if (dbg_errs_safe_d1[error_indx] && ~dbg_errs_safe_d2[error_indx] && ~dbg_err_en[error_indx]) begin
          dbg_err_counter[error_indx*16+15:error_indx*16]   <= dbg_counters_safe[error_indx*16+15:error_indx*16];
        end

        if (clr_dbg_err[error_indx]) begin
          dbg_err_en[error_indx]  <= 1'b0;
        end else if (dbg_errs_safe_d1[error_indx] && ~dbg_errs_safe_d2[error_indx]) begin
          dbg_err_en[error_indx]  <= 1'b1;
        end

        if (agent_reset) begin
          dbg_err_en[error_indx]   <= 1'b0;
        end

      end

    end
  endgenerate
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
