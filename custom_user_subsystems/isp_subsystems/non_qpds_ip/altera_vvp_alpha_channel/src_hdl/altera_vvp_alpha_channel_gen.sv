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
 * Module: altera_vvp_alpha_channel_gen
 *
 * Description: Generate the Alpha Channel
 *
 * ##################################################################################
*/

`default_nettype none

module altera_vvp_alpha_channel_gen #(
  parameter NUMBER_OF_COLOR_PLANES                  =  3, // standard VVP name
  parameter PIXELS_IN_PARALLEL                      =  2, // standard VVP name
  parameter BPS                                     = 10, // standard VVP name
  parameter C_LUT_ADDR_BITS                         = 10,
  parameter C_LUT_DATA_BITS                         = 20
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

  // CPU Regs
  r_vid_control,
  r_lut_write_addr,
  r_lut_write_en,
  r_lut_write_data
);


  //  Constants  //
  localparam C_PADDED_BPS           = (BPS < 8) ? 8 : BPS;

  localparam C_AXIS_IN_PIXEL_BYTES  = ((NUMBER_OF_COLOR_PLANES*C_PADDED_BPS)+7)/8*8;
  localparam C_AXIS_IN_WIDTH        = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH       = C_AXIS_IN_WIDTH/8;

  localparam C_AXIS_OUT_PIXEL_BYTES = (((NUMBER_OF_COLOR_PLANES+1)*C_PADDED_BPS)+7)/8*8;
  localparam C_AXIS_OUT_WIDTH       = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_OUT_WIDTH      = C_AXIS_OUT_WIDTH/8;

  localparam C_PIX_COUNT_BITS       = 16;


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

  input logic [31:0]                    r_vid_control;
  input logic [C_LUT_ADDR_BITS-1:0]     r_lut_write_addr;
  input logic                           r_lut_write_en;
  input logic [C_LUT_DATA_BITS-1:0]     r_lut_write_data;



// alpha lut window is adjusted for PIP alignment
  logic [C_PIX_COUNT_BITS-1:0]            pip_count_lo;
  logic [C_PIX_COUNT_BITS-1:0]            pip_count_hi;

  assign pip_count_lo   = PIXELS_IN_PARALLEL == 1 ? r_vid_control[15:0]  : {1'b0 , r_vid_control[15:1]};
  assign pip_count_hi   = PIXELS_IN_PARALLEL == 1 ? r_vid_control[31:16] : {1'b0 , r_vid_control[31:17]};


  //-----------------------------------------------------------------
  // Count through PIP for each line. Reset LUT address at the start of
  // a line and only increment through the LUT when inside the LUT window.
  //-----------------------------------------------------------------
  logic [C_PIX_COUNT_BITS-1:0]  pip_count;
  logic                         tfirst;
  logic [C_LUT_ADDR_BITS:0]     lut_address;

  always_ff @(posedge main_clock) begin : a_line_pip_cnt
    if (main_reset) begin
      pip_count     <= {C_PIX_COUNT_BITS{1'b0}};
      tfirst        <= 1'b0;
      lut_address   <= {(C_LUT_ADDR_BITS+1){1'b0}};
    end else begin
      if (axi4s_vid_in_tvalid && axi4s_vid_in_tready) begin
        if (axi4s_vid_in_tlast) begin  // reset at end of line
          pip_count     <= {C_PIX_COUNT_BITS{1'b0}};
        end else begin
          pip_count     <= pip_count + 1'b1;
        end

        // generate a tfirst marker
        tfirst    <= axi4s_vid_in_tlast;

        // lut address reset at first pip of line
        if (axi4s_vid_in_tuser[0] || tfirst) begin
          lut_address   <= {(C_LUT_ADDR_BITS+1){1'b0}};
        // increment through lut when inside the window
        end else if (pip_count >= pip_count_lo && pip_count <= pip_count_hi) begin
          lut_address <= lut_address + PIXELS_IN_PARALLEL;
        end
      end
    end
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------------------
  // Alpha LUT
  //-----------------------------------------------------------------------------
  logic [C_LUT_DATA_BITS-1:0]             lut_data;

  altera_vvp_alpha_channel_lut #(
  .DATA_WIDTH(C_LUT_DATA_BITS),
  .DATA_DEPTH(1024),
  .RAMSTYLE("M20K"),
  .DELAY(0)
  ) u_alpha_channel_lut (
  .clk_rd(main_clock),
  .clk_wr(main_clock),
  .i_read_enable(1'b1),
  .i_read_addr(lut_address[C_LUT_ADDR_BITS:1]),
  .o_read_data(lut_data),
  .i_write_enable(r_lut_write_en),
  .i_write_addr(r_lut_write_addr),
  .i_write_data(r_lut_write_data)
  );

  logic [C_LUT_DATA_BITS-1:0]             lut_data_out;

  always_ff @(posedge main_clock) begin : a_lut_data
    if (axi4s_vid_in_tready) begin
      lut_data_out    <= lut_data;
    end
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Total delay of the compute pipeline
  //-----------------------------------------------------------------
  localparam PIPELINE_STAGES = 2;
  // Final stage
  localparam DELAY_DEPTH_FINAL_STAGE = PIPELINE_STAGES - 1;

  logic [PIPELINE_STAGES-1:0]                         valid_pipe;
  logic [PIPELINE_STAGES-1:0][C_TUSER_IN_WIDTH-1:0]   user_pipe;
  logic [PIPELINE_STAGES-1:0]                         last_pipe;
  logic [PIPELINE_STAGES-1:0][C_AXIS_IN_WIDTH-1:0]    data_pipe;

  always_ff @(posedge main_clock) begin
    if (main_reset) begin
      valid_pipe  <= {PIPELINE_STAGES{1'b0}};
    end else begin
      if (axi4s_vid_in_tready) begin
        valid_pipe[0] <= axi4s_vid_in_tvalid;
        for (int i = 1; i < PIPELINE_STAGES; i++) begin
          valid_pipe[i] <= valid_pipe[i-1];
        end
      end
    end
  end

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
  // Output control
  //-----------------------------------------------------------------
  logic                                             valid_out;
  logic                     [C_TUSER_OUT_WIDTH-1:0] user_out;
  logic                                             last_out;

  assign valid_out = valid_pipe[PIPELINE_STAGES-1];
  assign user_out  = { {(C_TUSER_OUT_WIDTH-C_TUSER_IN_WIDTH){1'b0}} , user_pipe[PIPELINE_STAGES-1] };
  assign last_out  = last_pipe[PIPELINE_STAGES-1];
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output data
  //-----------------------------------------------------------------
  localparam C_AXIS_IN_PACKED_BITS  = NUMBER_OF_COLOR_PLANES*BPS;
  localparam C_AXIS_OUT_PACKED_BITS = (NUMBER_OF_COLOR_PLANES+1)*BPS;

  logic                     [C_AXIS_OUT_WIDTH-1:0]  data_out;

  always_comb begin

    integer i;

    data_out = {C_AXIS_OUT_WIDTH{1'b0}};   // padding bits are zero

    if (PIXELS_IN_PARALLEL == 1) begin
      if (lut_address[0] == 1'b0) begin   // alternate from lut output hi and lo words in 1 PIP mode
        data_out[C_AXIS_OUT_PACKED_BITS-1:0]  = { lut_data_out[BPS-1:0] , data_pipe[DELAY_DEPTH_FINAL_STAGE][C_AXIS_IN_PACKED_BITS-1:0] };
      end else begin
        data_out[C_AXIS_OUT_PACKED_BITS-1:0]  = { lut_data_out[(2*BPS)-1:BPS] , data_pipe[DELAY_DEPTH_FINAL_STAGE][C_AXIS_IN_PACKED_BITS-1:0] };
      end
    end else begin
      data_out[C_AXIS_OUT_PACKED_BITS+C_AXIS_OUT_PIXEL_BYTES-1:C_AXIS_OUT_PIXEL_BYTES]  = { lut_data_out[(2*BPS)-1:BPS] , 
                                                        data_pipe[DELAY_DEPTH_FINAL_STAGE][C_AXIS_IN_PACKED_BITS+C_AXIS_IN_PIXEL_BYTES-1:C_AXIS_IN_PIXEL_BYTES] };
      data_out[C_AXIS_OUT_PACKED_BITS-1:0]  = { lut_data_out[BPS-1:0] , 
                                                        data_pipe[DELAY_DEPTH_FINAL_STAGE][C_AXIS_IN_PACKED_BITS-1:0] };
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
