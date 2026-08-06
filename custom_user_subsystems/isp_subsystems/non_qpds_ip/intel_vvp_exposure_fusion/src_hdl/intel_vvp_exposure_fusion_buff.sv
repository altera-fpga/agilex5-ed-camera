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
 * Module: intel_vvp_exposure_fusion_buff
 *
 * Description: Creates a Buffer for the Exposure Fusion
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_exposure_fusion_buff #(
  parameter NUMBER_OF_COLOR_PLANES =    2, // standard VVP name
  parameter PIXELS_IN_PARALLEL     =    2, // standard VVP name
  parameter BPS                    =   12, // standard VVP name
  parameter LINE_BUFFER_DEPTH      = 2048
) (
  // Video Clock and Reset
  main_clock,
  main_reset,

  // AXI4S VVP Lite In
  axi4s_vid_in_0_tdata,
  axi4s_vid_in_0_tlast,
  axi4s_vid_in_0_tuser,
  axi4s_vid_in_0_tvalid,
  axi4s_vid_in_0_tready,

  // AXI4S VVP Lite In
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

  //  Helper functions  //
  // f(x) = ceil(log2(x))
  function automatic integer clog2;
    input [31:0] value;
    integer i;
    begin
      clog2 = 32;
      for (i=31; i>0; i=i-1) begin
        if (2**i >= value) begin
          clog2 = i;
        end
      end
    end
  endfunction

  //  Constants  //
  localparam C_AXIS_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*BPS+7)/8*8;
  localparam C_AXIS_WIDTH       = C_AXIS_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_WIDTH      = C_AXIS_WIDTH/8;

  // Line buffer stores 2 extra bits for TUSER[0] and TLAST bits corresponding to SOF and EOL signals
  localparam LINE_BUFFER_WIDTH = C_AXIS_WIDTH + 1 + 1;

  // localparam CNT_FLUSH_BITS = 5;

  //  Top Level Signals  //
  input  logic                         main_clock;
  input  logic                         main_reset;

  input  logic [     C_AXIS_WIDTH-1:0] axi4s_vid_in_0_tdata;
  input  logic                         axi4s_vid_in_0_tlast;
  input  logic [    C_TUSER_WIDTH-1:0] axi4s_vid_in_0_tuser;
  input  logic                         axi4s_vid_in_0_tvalid;
  output logic                         axi4s_vid_in_0_tready;

  input  logic [     C_AXIS_WIDTH-1:0] axi4s_vid_in_1_tdata;
  input  logic                         axi4s_vid_in_1_tlast;
  input  logic [    C_TUSER_WIDTH-1:0] axi4s_vid_in_1_tuser;
  input  logic                         axi4s_vid_in_1_tvalid;
  output logic                         axi4s_vid_in_1_tready;

  output logic [   2*C_AXIS_WIDTH-1:0] axi4s_vid_out_tdata;
  output logic                         axi4s_vid_out_tlast;
  output logic [  2*C_TUSER_WIDTH-1:0] axi4s_vid_out_tuser;
  output logic                         axi4s_vid_out_tvalid;
  input  logic                         axi4s_vid_out_tready;


  //  Signals  //
  typedef enum logic [2:0] {
    S_FLUSH,
    S_FILL,
    S_POST_FILL,
    S_2CH_RUN,
    S_2CH_EOL,
    S_2CH_BLANKING,
    S_RUN_1CH
  } ctrl_t;
  ctrl_t ctrl_state;

  logic vid_in_0_handshake;
  logic vid_in_1_handshake;
  logic vid_out_handshake;
  logic vid_in_0_sof;
  logic vid_in_1_sof;
  logic vid_in_0_eol;
  logic vid_in_1_eol;
  logic vid_in_1_buff_eol;

  logic vid_out_tvalid_1ch;
  logic vid_out_tvalid_2ch;

  logic [ C_AXIS_WIDTH-1:0] axi4s_vid_in_0_tdata_buff;
  logic [C_TUSER_WIDTH-1:0] axi4s_vid_in_0_tuser_buff;
  logic                     axi4s_vid_in_0_tlast_buff;
  logic [ C_AXIS_WIDTH-1:0] axi4s_vid_in_1_tdata_buff;
  logic [C_TUSER_WIDTH-1:0] axi4s_vid_in_1_tuser_buff;
  logic                     axi4s_vid_in_1_tlast_buff;
  logic                     axi4s_vid_in_1_tvalid_buff;
  logic                     axi4s_vid_in_1_tready_buff;

  logic [LINE_BUFFER_WIDTH-1:0]        line_buffer [LINE_BUFFER_DEPTH];
  logic [clog2(LINE_BUFFER_DEPTH)-1:0] lb_addr_wr;
  logic [clog2(LINE_BUFFER_DEPTH)-1:0] lb_addr_rd;
  logic [clog2(LINE_BUFFER_DEPTH)-1:0] lb_addr_wr_p1;
  logic [clog2(LINE_BUFFER_DEPTH)-1:0] lb_addr_rd_p1;
  logic                                lb_empty_next;
  logic                                lb_empty;
  logic                                lb_full;

  assign vid_in_0_handshake = axi4s_vid_in_0_tvalid && axi4s_vid_in_0_tready;
  assign vid_in_1_handshake = axi4s_vid_in_1_tvalid && axi4s_vid_in_1_tready;
  assign vid_out_handshake  = axi4s_vid_out_tvalid && axi4s_vid_out_tready;
  assign vid_in_0_sof       = vid_in_0_handshake && axi4s_vid_in_0_tuser[0];
  assign vid_in_1_sof       = vid_in_1_handshake && axi4s_vid_in_1_tuser[0];
  assign vid_in_0_eol       = vid_in_0_handshake && axi4s_vid_in_0_tlast;
  assign vid_in_1_eol       = vid_in_1_handshake && axi4s_vid_in_1_tlast;
  assign vid_in_1_buff_eol  = axi4s_vid_in_1_tvalid_buff && axi4s_vid_in_1_tready_buff && axi4s_vid_in_1_tlast_buff;

  assign lb_addr_wr_p1 = lb_addr_wr + 1'b1;
  assign lb_addr_rd_p1 = lb_addr_rd + 1'b1;
  assign lb_empty_next = ~lb_full && (lb_addr_wr == lb_addr_rd);

  always_ff @(posedge main_clock) begin : a_line_buffer
    if (ctrl_state == S_FLUSH) begin
      lb_addr_wr  <= '0;
      lb_addr_rd  <= '0;
      lb_empty    <= 1'b1;
      lb_full     <= 1'b0;
    end else begin
      if (vid_in_0_handshake) begin
        lb_addr_wr <= lb_addr_wr_p1;
        lb_empty   <= 1'b0;
      end else begin
        lb_empty <= lb_empty_next;
      end
      if (vid_out_handshake) begin
        lb_addr_rd <= lb_addr_rd_p1;
      end
      if (~vid_out_handshake && (vid_in_0_handshake) && (lb_addr_wr_p1 == lb_addr_rd)) begin
        lb_full  <= 1'b1;
        lb_empty <= 1'b0;
      end
    end
    // Line buffer
    line_buffer[lb_addr_wr] <= {axi4s_vid_in_0_tdata, axi4s_vid_in_0_tuser[0], axi4s_vid_in_0_tlast};
  end

  assign {axi4s_vid_in_0_tdata_buff, axi4s_vid_in_0_tuser_buff[0], axi4s_vid_in_0_tlast_buff} = line_buffer[lb_addr_rd];
  assign axi4s_vid_in_0_tuser_buff[C_TUSER_WIDTH-1:1] = '0;

  always_ff @(posedge main_clock) begin : a_op_if
    logic [ C_AXIS_WIDTH-1:0] tdata_reg;
    logic [C_TUSER_WIDTH-1:0] tuser_reg;
    logic                     tlast_reg;

    axi4s_vid_in_1_tready      <= (axi4s_vid_in_1_tready_buff || (axi4s_vid_in_1_tready      && (~axi4s_vid_in_1_tvalid_buff || ~axi4s_vid_in_1_tvalid     )));
    axi4s_vid_in_1_tvalid_buff <= (axi4s_vid_in_1_tvalid      || (axi4s_vid_in_1_tvalid_buff && (~axi4s_vid_in_1_tready      || ~axi4s_vid_in_1_tready_buff)));

    // 1 Reg deep FIFO.
    if (axi4s_vid_in_1_tready) begin
      tdata_reg <= axi4s_vid_in_1_tdata;
      tuser_reg <= axi4s_vid_in_1_tuser;
      tlast_reg <= axi4s_vid_in_1_tlast;
    end

    // Select between FIFO or input data.
    if (axi4s_vid_in_1_tready_buff || ~axi4s_vid_in_1_tvalid_buff) begin
      axi4s_vid_in_1_tdata_buff <= axi4s_vid_in_1_tdata;
      axi4s_vid_in_1_tuser_buff <= axi4s_vid_in_1_tuser;
      axi4s_vid_in_1_tlast_buff <= axi4s_vid_in_1_tlast;
      if (~axi4s_vid_in_1_tready) begin
        axi4s_vid_in_1_tdata_buff <= tdata_reg;
        axi4s_vid_in_1_tuser_buff <= tuser_reg;
        axi4s_vid_in_1_tlast_buff <= tlast_reg;
      end
    end

    if (main_reset) begin
      axi4s_vid_in_1_tready      <= 1'b1;
      axi4s_vid_in_1_tvalid_buff <= 1'b0;
    end
  end

  logic latest_tlast_ch;
  logic vid_ch_overrun;

  always_ff @(posedge main_clock) begin : a_vid_ch_ov
    if (ctrl_state == S_FLUSH) begin
      latest_tlast_ch <= 1'b1;
      vid_ch_overrun  <= 1'b0;
    end else begin
      if (vid_in_0_eol) begin
        latest_tlast_ch <= 1'b0;
        vid_ch_overrun  <= ~latest_tlast_ch;
      end
      if (vid_in_1_eol) begin
        latest_tlast_ch <= 1'b1;
        vid_ch_overrun  <= latest_tlast_ch;
      end
    end
  end

  always_ff @(posedge main_clock) begin : a_state_machine_states
    if (main_reset) begin
      ctrl_state <= S_FLUSH;
    end else begin
      case (ctrl_state)
        S_FLUSH : begin
          if (axi4s_vid_in_0_tvalid && axi4s_vid_in_0_tuser[0]) begin
            ctrl_state <= S_FILL;
          end
        end
        S_FILL : begin
          if (lb_full) begin
            ctrl_state <= S_FLUSH;
          end else if (vid_in_0_eol) begin
            ctrl_state <= S_POST_FILL;
          end
        end
        S_POST_FILL : begin
          if (lb_full) begin
            ctrl_state <= S_FLUSH;
          end else if (vid_in_1_sof) begin
            ctrl_state <= S_2CH_RUN;
          end else if (vid_in_0_handshake) begin
            ctrl_state <= S_RUN_1CH;
          end
        end
        S_2CH_RUN : begin
          if (lb_full || vid_ch_overrun) begin
            ctrl_state <= S_FLUSH;
          end else if (vid_in_1_buff_eol) begin
            ctrl_state <= S_2CH_EOL;
          end
        end
        S_2CH_EOL : begin
          if (lb_full || vid_ch_overrun) begin
            ctrl_state <= S_FLUSH;
          end else if (vid_out_handshake) begin
            ctrl_state <= S_2CH_BLANKING;
          end
        end
        S_2CH_BLANKING : begin
          if (lb_full || vid_ch_overrun) begin
            ctrl_state <= S_FLUSH;
          end else if (vid_in_1_handshake) begin
            ctrl_state <= S_2CH_RUN;
          end
        end
        S_RUN_1CH : begin
          if (lb_full || vid_in_1_handshake) begin
            ctrl_state <= S_FLUSH;
          end
        end
        default : begin
          ctrl_state <= S_FLUSH;
        end
      endcase
    end
  end


  assign vid_out_tvalid_1ch = ~lb_empty && ~lb_empty_next;
  assign vid_out_tvalid_2ch = vid_out_tvalid_1ch && axi4s_vid_in_1_tvalid_buff && axi4s_vid_in_1_tready_buff;

  always_comb begin : a_state_machine_outputs
      case (ctrl_state)
        S_FLUSH : begin
          axi4s_vid_in_0_tready      = axi4s_vid_in_0_tvalid && axi4s_vid_in_0_tuser[0] ? 1'b0 : axi4s_vid_in_0_tvalid;
          axi4s_vid_in_1_tready_buff = 1'b1;
          axi4s_vid_out_tvalid       = 1'b0;
        end
        S_FILL : begin
          axi4s_vid_in_0_tready      = 1'b1;
          axi4s_vid_in_1_tready_buff = 1'b0;
          axi4s_vid_out_tvalid       = 1'b0;
        end
        S_POST_FILL : begin
          axi4s_vid_in_0_tready      = 1'b1;
          axi4s_vid_in_1_tready_buff = 1'b1;
          axi4s_vid_out_tvalid       = 1'b0;
        end
        S_2CH_RUN : begin
          axi4s_vid_in_0_tready      = axi4s_vid_out_tready;
          axi4s_vid_in_1_tready_buff = axi4s_vid_out_tready;
          axi4s_vid_out_tvalid       = vid_out_tvalid_2ch;
        end
        S_2CH_EOL : begin
          axi4s_vid_in_0_tready      = axi4s_vid_out_tready;
          axi4s_vid_in_1_tready_buff = axi4s_vid_out_tready;
          axi4s_vid_out_tvalid       = vid_out_tvalid_2ch;
        end
        S_2CH_BLANKING : begin
          axi4s_vid_in_0_tready      = axi4s_vid_out_tready;
          axi4s_vid_in_1_tready_buff = axi4s_vid_out_tready;
          axi4s_vid_out_tvalid       = vid_out_tvalid_2ch;
        end
        S_RUN_1CH : begin
          axi4s_vid_in_0_tready      = axi4s_vid_out_tready;
          axi4s_vid_in_1_tready_buff = 1'b1;
          axi4s_vid_out_tvalid       = vid_out_tvalid_1ch;
        end
        default : begin
          axi4s_vid_in_0_tready      = 1'b1;
          axi4s_vid_in_1_tready_buff = 1'b1;
          axi4s_vid_out_tvalid       = 1'b0;
        end
      endcase
  end

  assign axi4s_vid_out_tdata = {axi4s_vid_in_1_tdata_buff, axi4s_vid_in_0_tdata_buff};
  assign axi4s_vid_out_tuser = axi4s_vid_in_0_tuser_buff;
  assign axi4s_vid_out_tlast = axi4s_vid_in_0_tlast_buff;

endmodule

`default_nettype wire
