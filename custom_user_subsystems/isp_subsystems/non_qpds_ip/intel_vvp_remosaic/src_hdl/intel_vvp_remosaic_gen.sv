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
 * Module: intel_vvp_remosaic_gen
 *
 * Description: Generate the Remosaic
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_remosaic_gen #(
  parameter C_USE_CPU                               = 1,

  parameter NUMBER_OF_COLOR_PLANES                  =  3, // standard VVP name
  parameter PIXELS_IN_PARALLEL                      =  2, // standard VVP name
  parameter BPS                                     = 10, // standard VVP name

  parameter logic [7:0] C_CONV_MODE                 = 8'b00010110   //BGGR - 2b per 2x2 mapping 00=blue, 01=green, 10=red
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

  // Video Control Regs
  r_vid_conv_mode
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

  input logic [7:0]                     r_vid_conv_mode;


  //  Signals  //
  logic                         pix_even_oddn;
  logic                         line_even_oddn;
  logic [7:0]                   rr_vid_conv_mode;

  logic                         pixel_00;
  logic [7:0]                   cpu_conv_mode;
  logic [7:0]                   conv_mode;

  logic [1:0]                   color_sel;
  logic [C_AXIS_OUT_WIDTH-1:0]  axi4s_vid_out_tdata_int;

  logic                         axi4s_vid_in_tready_raw;
  logic                         axi4s_vid_out_tvalid_raw;
  logic                         axi4s_vid_in_tready_dup;
  logic                         axi4s_vid_out_tvalid_dup;
  logic [C_AXIS_OUT_WIDTH-1:0]  tdata_reg;
  logic                         tlast_reg;
  logic                         tuser_reg;


  //-----------------------------------------------------------------
  // Pixel and Line Counters
  // Used to sync the mapping pattern to a known pixel every line/frame
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin : a_pix_line_cnt

    if (axi4s_vid_in_tvalid && axi4s_vid_in_tready) begin

      if (axi4s_vid_in_tlast) begin  // reset at end of line
        pix_even_oddn   <= 1'b0;
      end else if (PIXELS_IN_PARALLEL == 1) begin // PiP=1 special case - alternate pixels per clock cycle
        pix_even_oddn   <= !pix_even_oddn;
      end

      if (axi4s_vid_in_tuser[0]) begin    // decode is 1 cycle late - marks sof rather than eof! (async decode must also be used for mapping)
        line_even_oddn    <= 1'b0;
        rr_vid_conv_mode  <= r_vid_conv_mode;   // latch CPU value
      end else if (axi4s_vid_in_tlast) begin  // alternate at end of each line
        line_even_oddn    <= !line_even_oddn;
      end

    end

    if (main_reset) begin
      pix_even_oddn             <= 1'b0;
      line_even_oddn            <= 1'b0;
      rr_vid_conv_mode          <= C_CONV_MODE;
    end

  end
  //-----------------------------------------------------------------

  // decode start of frame in same cycle as asserted (async decode to use in pixel 0,0 position)
  assign pixel_00 = (axi4s_vid_in_tvalid && axi4s_vid_in_tready && axi4s_vid_in_tuser[0]) ? 1'b1 : 1'b0;

  // Assign CPU conversion mode to either the CPU reg at pixel 0,0 async decode or registered version thereafter. Ensures value is sync'd to pixel 0,0
  assign cpu_conv_mode = (axi4s_vid_in_tvalid && axi4s_vid_in_tready && axi4s_vid_in_tuser[0]) ? r_vid_conv_mode : rr_vid_conv_mode;
  // Assign conversion mode to either CPU reg (sync'd to pixel 0,0) or Hardcoded if no CPU
  assign conv_mode = C_USE_CPU ? cpu_conv_mode : C_CONV_MODE;


  //-----------------------------------------------------------------
  // Select Colour channel from input pixel based on selected mapping
  // Colour channel mapping changes depending on PiP and odd/even lines.
  //-----------------------------------------------------------------
  always_comb begin

    integer i;

    axi4s_vid_out_tdata_int = {C_AXIS_OUT_WIDTH{1'b0}};   // padding bits are zero

    for (i = 0; i < PIXELS_IN_PARALLEL; i++) begin

      if ((((i+2) % 2) == 1'b1) || pix_even_oddn) begin   // Even PiP mappings (PiP=2+) and special case (PiP=1 alternate clock cycles) - for Odd lines, use Odd line Even pixel mapping
        color_sel = conv_mode[5:4];
        if (line_even_oddn && ~pixel_00)   color_sel = conv_mode[1:0];   // for Even lines, use Even line Even pixel mapping
      end else begin                                      // Odd PiP mappings - for Odd lines, use Odd line Odd pixel mapping
        color_sel = conv_mode[7:6];
        if (line_even_oddn && ~pixel_00)   color_sel = conv_mode[3:2];   // for Even lines, use Even line Odd pixel mapping
      end

      case (color_sel)
        2'b01:    if (NUMBER_OF_COLOR_PLANES > 1) begin
                    axi4s_vid_out_tdata_int[(i*C_AXIS_OUT_PIXEL_BYTES)+:C_PADDED_BPS]    = axi4s_vid_in_tdata[(i*C_AXIS_IN_PIXEL_BYTES)+(1*C_PADDED_BPS)+:C_PADDED_BPS];  // Green
                  end
        2'b10:    if (NUMBER_OF_COLOR_PLANES > 2) begin
                    axi4s_vid_out_tdata_int[(i*C_AXIS_OUT_PIXEL_BYTES)+:C_PADDED_BPS]    = axi4s_vid_in_tdata[(i*C_AXIS_IN_PIXEL_BYTES)+(2*C_PADDED_BPS)+:C_PADDED_BPS];  // Red
                  end
        default:  axi4s_vid_out_tdata_int[(i*C_AXIS_OUT_PIXEL_BYTES)+:C_PADDED_BPS]    = axi4s_vid_in_tdata[(i*C_AXIS_IN_PIXEL_BYTES)+(0*C_PADDED_BPS)+:C_PADDED_BPS];  // Blue
      endcase

    end

  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output axis shim
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin : a_op_if

    axi4s_vid_in_tready   <= (axi4s_vid_out_tready || (axi4s_vid_in_tready && (~axi4s_vid_out_tvalid || ~axi4s_vid_in_tvalid)));
    axi4s_vid_out_tvalid  <= (axi4s_vid_in_tvalid || (axi4s_vid_out_tvalid && (~axi4s_vid_in_tready || ~axi4s_vid_out_tready)));

    // 1 Reg deep FIFO.
    if (axi4s_vid_in_tready) begin
      tdata_reg   <= axi4s_vid_out_tdata_int;
      tlast_reg   <= axi4s_vid_in_tlast;
      tuser_reg   <= axi4s_vid_in_tuser[0];
    end

    // Select between FIFO or input data.
    if (axi4s_vid_out_tready || ~axi4s_vid_out_tvalid) begin
      axi4s_vid_out_tdata     <= axi4s_vid_out_tdata_int;
      axi4s_vid_out_tlast     <= axi4s_vid_in_tlast;
      axi4s_vid_out_tuser[0]  <= axi4s_vid_in_tuser[0];
      if (~axi4s_vid_in_tready) begin
        axi4s_vid_out_tdata     <= tdata_reg;
        axi4s_vid_out_tlast     <= tlast_reg;
        axi4s_vid_out_tuser[0]  <= tuser_reg;
      end
    end

    if (main_reset) begin
      axi4s_vid_in_tready   <= 1'b1;
      axi4s_vid_out_tvalid  <= 1'b0;

      axi4s_vid_out_tlast       <= 1'b0;
      axi4s_vid_out_tuser       <= {C_TUSER_OUT_WIDTH{1'b0}};
      axi4s_vid_out_tdata       <= {C_AXIS_OUT_WIDTH{1'b0}};
    end

  end
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
