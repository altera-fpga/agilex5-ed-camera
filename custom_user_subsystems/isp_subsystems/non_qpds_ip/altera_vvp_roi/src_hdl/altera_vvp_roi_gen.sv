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
 * Module: altera_vvp_roi_gen
 *
 * Description: Generate the Region Of Interest
 *
 * ##################################################################################
*/

`default_nettype none

module altera_vvp_roi_gen #(
  parameter NUMBER_OF_COLOR_PLANES                  =  3, // standard VVP name
  parameter PIXELS_IN_PARALLEL                      =  2, // standard VVP name
  parameter BPS                                     = 10  // standard VVP name
) (
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
  axi4s_vid_out_tready,

  roi_enable,

  // CPU Regs
  r_vid_control
);


  //  Constants  //
  localparam C_PADDED_BPS           = (BPS < 8) ? 8 : BPS;

  localparam C_AXIS_IN_PIXEL_BYTES  = ((NUMBER_OF_COLOR_PLANES*C_PADDED_BPS)+7)/8*8;
  localparam C_AXIS_IN_WIDTH        = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH       = C_AXIS_IN_WIDTH/8;

  localparam C_AXIS_OUT_PIXEL_BYTES = ((NUMBER_OF_COLOR_PLANES*C_PADDED_BPS)+7)/8*8;
  localparam C_AXIS_OUT_WIDTH       = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_OUT_WIDTH      = C_AXIS_OUT_WIDTH/8;


  //  Top Level Signals  //
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

  input logic [PIXELS_IN_PARALLEL-1:0]  roi_enable;

  input logic [31:0]                    r_vid_control;


  //-----------------------------------------------------------------
  // Total delay of the compute pipeline
  //-----------------------------------------------------------------
  localparam PIPELINE_STAGES = 3;

  logic [PIPELINE_STAGES-1:0]                           valid_pipe;
  logic [PIPELINE_STAGES-1:0][C_TUSER_IN_WIDTH-1:0]     user_pipe;
  logic [PIPELINE_STAGES-1:0]                           last_pipe;
  logic [PIPELINE_STAGES-1:0][C_AXIS_IN_WIDTH-1:0]      data_pipe;

  // pipelines that require reset
  always_ff @(posedge main_clock) begin
    if (main_reset) begin
      valid_pipe    <= {PIPELINE_STAGES{1'b0}};
    end else begin
      if (axi4s_vid_in_tready) begin
        valid_pipe[0]   <= axi4s_vid_in_tvalid;
        for (int i = 1; i < PIPELINE_STAGES; i++) begin
          valid_pipe[i]   <= valid_pipe[i-1];
        end
      end
    end
  end

  // pipelines that require no reset
  always_ff @(posedge main_clock) begin
    if (axi4s_vid_in_tready) begin
      user_pipe[0]  <= axi4s_vid_in_tuser;
      last_pipe[0]  <= axi4s_vid_in_tlast;
      data_pipe[0]  <= axi4s_vid_in_tdata;
      for (int i = 1; i < PIPELINE_STAGES; i++) begin
        user_pipe[i]  <= user_pipe[i-1];
        last_pipe[i]  <= last_pipe[i-1];
        data_pipe[i]  <= data_pipe[i-1];
      end
    end
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Pipeline Output
  //-----------------------------------------------------------------
  logic                                             valid_out;
  logic                     [C_TUSER_OUT_WIDTH-1:0] user_out;
  logic                                             last_out;
  logic                     [C_AXIS_IN_WIDTH-1:0]   data_pipe_out;

  assign valid_out      = valid_pipe[PIPELINE_STAGES-1];
  assign user_out       = { {(C_TUSER_OUT_WIDTH-C_TUSER_IN_WIDTH){1'b0}} , user_pipe[PIPELINE_STAGES-1] };
  assign last_out       = last_pipe[PIPELINE_STAGES-1];
  assign data_pipe_out  = data_pipe[PIPELINE_STAGES-1];
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // CPU Enable - align control to start of frames
  //-----------------------------------------------------------------
  logic             enable;

  always_ff @(posedge main_clock) begin
    if (main_reset) begin
      enable    <= 1'b0;
    end else begin
      if (axi4s_vid_in_tready && valid_out && user_out) begin
        enable    <= r_vid_control[0];
      end
    end
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output data
  //-----------------------------------------------------------------
  logic [1:0]       mode;

  assign mode     = r_vid_control[3:2];

  logic [C_AXIS_OUT_WIDTH-1:0]   data_out;

  always_comb begin

    data_out = {C_AXIS_OUT_WIDTH{1'b0}};   // padding bits are zero

          if (enable == 1'b0 || roi_enable[0]) begin
              data_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)]  = data_pipe_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)];
              data_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)]  = data_pipe_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)];
              data_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)]  = data_pipe_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)];
          end else begin
            case (mode)
              2'b01:  begin
                        data_out[(1*C_PADDED_BPS)-1:0*C_PADDED_BPS]  = {2'b0 , data_pipe_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)+2] };
                        data_out[(2*C_PADDED_BPS)-1:1*C_PADDED_BPS]  = {2'b0 , data_pipe_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)+2] };
                        data_out[(3*C_PADDED_BPS)-1:2*C_PADDED_BPS]  = {2'b0 , data_pipe_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)+2] };
                      end
              2'b10:  begin
                        data_out[(1*C_PADDED_BPS)-1:0*C_PADDED_BPS]  = {3'b0 , data_pipe_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)+3] };
                        data_out[(2*C_PADDED_BPS)-1:1*C_PADDED_BPS]  = {3'b0 , data_pipe_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)+3] };
                        data_out[(3*C_PADDED_BPS)-1:2*C_PADDED_BPS]  = {3'b0 , data_pipe_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)+3] };
                      end
              2'b11:  begin
                        data_out[(1*C_PADDED_BPS)-1:0*C_PADDED_BPS]  = {4'b0 , data_pipe_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)+4] };
                        data_out[(2*C_PADDED_BPS)-1:1*C_PADDED_BPS]  = {4'b0 , data_pipe_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)+4] };
                        data_out[(3*C_PADDED_BPS)-1:2*C_PADDED_BPS]  = {4'b0 , data_pipe_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)+4] };
                      end
              default:begin
                        data_out[(1*C_PADDED_BPS)-1:0*C_PADDED_BPS]  = {1'b0 , data_pipe_out[(1*C_PADDED_BPS)-1:(0*C_PADDED_BPS)+1] };
                        data_out[(2*C_PADDED_BPS)-1:1*C_PADDED_BPS]  = {1'b0 , data_pipe_out[(2*C_PADDED_BPS)-1:(1*C_PADDED_BPS)+1] };
                        data_out[(3*C_PADDED_BPS)-1:2*C_PADDED_BPS]  = {1'b0 , data_pipe_out[(3*C_PADDED_BPS)-1:(2*C_PADDED_BPS)+1] };
                      end
            endcase
          end

          if (PIXELS_IN_PARALLEL > 1) begin
            if (enable == 1'b0 || roi_enable[1]) begin
              data_out[C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(0*C_PADDED_BPS)]  = data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(0*C_PADDED_BPS)];
              data_out[C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)]  = data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)];
              data_out[C_AXIS_OUT_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)]  = data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)];
            end else begin
              case (mode)
                2'b01:  begin
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(0*C_PADDED_BPS)]  = {2'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(0*C_PADDED_BPS)+2] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)]  = {2'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)+2] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)]  = {2'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)+2] };
                        end
                2'b10:  begin
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(0*C_PADDED_BPS)]  = {3'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(0*C_PADDED_BPS)+3] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)]  = {3'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)+3] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)]  = {3'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)+3] };
                        end
                2'b11:  begin
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(0*C_PADDED_BPS)]  = {4'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(0*C_PADDED_BPS)+4] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)]  = {4'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)+4] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)]  = {4'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)+4] };
                        end
                default:begin
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(0*C_PADDED_BPS)]  = {1'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(0*C_PADDED_BPS)+1] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(1*C_PADDED_BPS)]  = {1'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(1*C_PADDED_BPS)+1] };
                          data_out[C_AXIS_OUT_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_OUT_PIXEL_BYTES+(2*C_PADDED_BPS)]  = {1'b0 , data_pipe_out[C_AXIS_IN_PIXEL_BYTES+(3*C_PADDED_BPS)-1:C_AXIS_IN_PIXEL_BYTES+(2*C_PADDED_BPS)+1] };
                        end
              endcase
            end
          end
  end

  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output axis shim
  //-----------------------------------------------------------------
    logic [C_AXIS_OUT_WIDTH-1:0]  tdata_reg;
    logic [C_TUSER_OUT_WIDTH-1:0] tuser_reg;
    logic                         tlast_reg;

  always_ff @(posedge main_clock) begin : a_op_if
    if (main_reset) begin
      axi4s_vid_in_tready  <= 1'b1;
      axi4s_vid_out_tvalid <= 1'b0;
    end else begin
      axi4s_vid_in_tready  <= (axi4s_vid_out_tready || (axi4s_vid_in_tready  && (~axi4s_vid_out_tvalid || ~valid_out           )));
      axi4s_vid_out_tvalid <= (valid_out            || (axi4s_vid_out_tvalid && (~axi4s_vid_in_tready  || ~axi4s_vid_out_tready)));

      // 1 Reg deep FIFO.
      if (axi4s_vid_in_tready) begin
        tdata_reg <= data_out;
        tlast_reg <= last_out;
        tuser_reg <= user_out;
      end

      // Select between FIFO or input data.
      if (axi4s_vid_out_tready || ~axi4s_vid_out_tvalid) begin
        axi4s_vid_out_tdata <= data_out;
        axi4s_vid_out_tlast <= last_out;
        axi4s_vid_out_tuser <= user_out;
        if (~axi4s_vid_in_tready) begin
          axi4s_vid_out_tdata <= tdata_reg;
          axi4s_vid_out_tlast <= tlast_reg;
          axi4s_vid_out_tuser <= tuser_reg;
        end
      end
    end
  end
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
