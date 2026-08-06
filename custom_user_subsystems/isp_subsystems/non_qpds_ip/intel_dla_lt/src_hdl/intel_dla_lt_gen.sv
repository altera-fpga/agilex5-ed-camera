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
 * Module: intel_dla_lt_gen
 *
 * Description: Generates the Layout Transform
 *
 * The AI IP requires the input image to be padded by 1 black pixel before and after
 * the real image pixels. Furthermore, the input image is required to have 1 line of
 * mid-gray pixels before and after the real image. For flexibility, both the black
 * and mid-gray pixel colors can be modified where the black value is the DF (Default)
 * value, and the mid-gray value is the DC (Don't Care) value.
 *
 * The Layout transform (LT) packs a 2x2 block of pixels (as received in image raster
 * scan format i.e. pixels arriving in the order from left to right, top to bottom)
 * into a single output word. This is a technique called folding and increases the
 * performance through the first convolution layer. The pixel values themselves are
 * also converted from uint 8 bit values to FP11 values (padded to 16bits).
 *
 * A line store is required so that the pixels from 2 video lines can be packed
 * together. Pixel and Line counters, along with tLast (end of line (EOL) marker) and
 * tUser (Start Of Frame (SOF) marker) would nominally be enough to control the LT
 * pipeline (including the line store control). However, the added complication of
 * both pixel and line padding requires extra *_oddn_even and line_mode control
 * signals. This is simply due to the fact that the real input image pixels should
 * not be stalled when generating the extra padding pixels and lines. We can take
 * advantage of the 2x2 packing to negate the effect of adding the extra padding
 * pixels and lines by assuming the first pixel and line are treated as the second
 * pixel and line, where the first pixel and line of padding can be generated on the
 * fly. The same technique can apply to the last pixel and last line which can be
 * treated as the penultimate pixel and line so that the last padding pixel and line
 * can be generated on the fly.
 *
 * The LT packs 4 input pixels into an output word. However, since each pixel is 3
 * color planes, there is some wastage in the output word (equivalent to 1 color
 * plane for each of the 4 pixels).
 * ##################################################################################
*/

`default_nettype none

module intel_dla_lt_gen #(
  parameter C_USE_CPU                               = 1,

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
  // Video Clock and Reset
  main_clock,
  main_reset,

  // Data In
  axi4s_vid_in_tdata,
  axi4s_vid_in_tlast,
  axi4s_vid_in_tuser,
  axi4s_vid_in_tvalid,
  axi4s_vid_in_tready,

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
  axi4s_vid_out_tready,

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


  // Parameters
  localparam C_AXIS_IN_PIXEL_BYTES    = NUMBER_OF_COLOR_PLANES_IN * BPS_IN;
  localparam C_AXIS_IN_WIDTH          = C_AXIS_IN_PIXEL_BYTES * PIXELS_IN_PARALLEL_IN;
  localparam C_TUSER_IN_WIDTH         = C_AXIS_IN_WIDTH / 8;

  localparam C_AXIS_OUT_PIXEL_BYTES   = NUMBER_OF_COLOR_PLANES_OUT * BPS_OUT;
  localparam C_AXIS_OUT_WIDTH         = C_AXIS_OUT_PIXEL_BYTES * PIXELS_IN_PARALLEL_OUT;
  localparam C_TUSER_OUT_WIDTH        = C_AXIS_OUT_WIDTH / 8;


  // Top Level Signals
  input  logic                          main_clock;
  input  logic                          main_reset;

  input  logic [C_AXIS_IN_WIDTH-1:0]    axi4s_vid_in_tdata;
  input  logic                          axi4s_vid_in_tlast;
  input  logic [C_TUSER_IN_WIDTH-1:0]   axi4s_vid_in_tuser;
  input  logic                          axi4s_vid_in_tvalid;
  output logic                          axi4s_vid_in_tready;

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

  input  logic [15:0]                   r_vid_v_lines;
  input  logic                          r_resync_en;
  input  logic [15:0]                   r_vid_dc_fp16_pix_val;
  input  logic [15:0]                   r_vid_df_fp16_pix_val;
  input  logic                          r_rb_swap;

  output logic [63:0]                   dbg_counters;
  output logic [ 3:0]                   dbg_errs;


  // Local Params
  localparam logic [1:0] PIX_DEFAULT  = 2'b00;
  localparam logic [1:0] PIX_DONTCARE = 2'b01;
  localparam logic [1:0] PIX_LIVE     = 2'b10;
  localparam logic [1:0] PIX_STORE    = 2'b11;

  // Local Signals
  logic [15:0]    vid_v_lines;
  logic [15:0]    vid_dc_fp16_pix_val;
  logic [15:0]    vid_df_fp16_pix_val;
  logic           rb_swap;

  logic           data_out_ready;
  logic           line_oddn_even;



  // Assign the dont care and default pixel values
  assign vid_v_lines         = C_USE_CPU ? r_vid_v_lines         : C_V_LINES;
  assign vid_dc_fp16_pix_val = C_USE_CPU ? r_vid_dc_fp16_pix_val : C_DC_FP16_PIX_VAL;
  assign vid_df_fp16_pix_val = C_USE_CPU ? r_vid_df_fp16_pix_val : C_DF_FP16_PIX_VAL;
  assign rb_swap             = C_USE_CPU ? r_rb_swap             : C_RB_SWAP;


  // The FIFO should be sized to never deassert tready
  assign data_out_ready = (axi4s_vid_fifo_out_tready && axi4s_vid_out_tready);

  assign axi4s_vid_in_tready  = data_out_ready;


  //-----------------------------------------------------------------
  // Pixel and Line Counters
  //
  // Used to sync the mapping pattern to a known pixel every line/frame.
  //
  // Note that Lines are numbered 0, 1, 2, etc and therefore even lines
  // are deemed lines 0, 2, 4, while odd are 1, 3, 5, etc.
  //-----------------------------------------------------------------
  logic           enable;
  logic [15:0]    line_counter;
  logic [1:0]     line_mode;
  logic           pix_oddn_even;
  logic           pixel_first;

  always_ff @(posedge main_clock) begin

    if (r_resync_en) begin
      enable  <= 1'b0;
    // resync to SOF if resync en activated
    end else if (axi4s_vid_in_tvalid && axi4s_vid_in_tuser[0]) begin
      enable  <= 1'b1;
    end

    if (~enable) begin
      line_counter    <= 16'b0;
      line_mode       <= 2'b00;
      pix_oddn_even   <= 1'b0;
      pixel_first     <= 1'b0;

    end else if (data_out_ready && axi4s_vid_in_tvalid) begin

      // line counter
      if (axi4s_vid_in_tuser[0]) begin
        line_counter    <= 16'b0;
        line_mode       <= 2'b00;
      end else if (axi4s_vid_in_tlast) begin
        line_counter    <= line_counter + 1'b1;
        if (line_counter == vid_v_lines) begin          //y_res val-1 so 160-1 = 159
          line_mode       <= 2'b10;
        end else begin
          line_mode       <= 2'b01;
        end
      end

      // pixel counter
      if (axi4s_vid_in_tuser[0]) begin
        pix_oddn_even   <= 1'b1;
      end else if (axi4s_vid_in_tlast) begin
        pix_oddn_even   <= 1'b0;
      end else begin
        pix_oddn_even   <= ~pix_oddn_even;
      end

      pixel_first <= axi4s_vid_in_tlast;
    end

    if (main_reset) begin
      enable          <= 1'b0;
      line_counter    <= 16'b0;
      line_mode       <= 2'b00;
      pix_oddn_even   <= 1'b0;
      pixel_first     <= 1'b0;
    end

  end

  assign line_oddn_even = line_counter[0];
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Line Store
  //
  // Store only the even numbered lines (lines 1, 3, 5 etc when starting from line 0).
  // The FIFO should be sized to never stall
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin

    axi4s_vid_fifo_out_tvalid   <= 1'b0;
    if (data_out_ready) begin
        if (line_mode == 2'b01 && line_oddn_even) begin
          axi4s_vid_fifo_out_tvalid   <= axi4s_vid_in_tvalid;
        end
        axi4s_vid_fifo_out_tdata    <= axi4s_vid_in_tdata;
        axi4s_vid_fifo_out_tlast    <= axi4s_vid_in_tlast;
        axi4s_vid_fifo_out_tuser    <= axi4s_vid_in_tuser;
    end

  end

  // make sure all bad data is drained from the line store during the first and last lines. Always ensures we get back into sync
  assign axi4s_vid_fifo_in_tready = (line_mode == 2'b00 || line_mode == 2'b10) ? 1'b1 : (line_mode == 2'b01 && ~line_oddn_even) ? (data_out_ready && axi4s_vid_in_tvalid) : 1'b0;
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output Pixel Control
  //
  // Select Pixel value and latch depending on position within the
  // mapping pattern.
  //-----------------------------------------------------------------
  logic [3:0]         pix_val_en;
  logic [3:0][1:0]    pix_val_sel;

  always_ff @(posedge main_clock) begin

    if (data_out_ready) begin
      // default
      pix_val_en[0]   <= axi4s_vid_in_tvalid;
      pix_val_en[1]   <= axi4s_vid_in_tvalid;
      pix_val_en[2]   <= axi4s_vid_in_tvalid;
      pix_val_en[3]   <= axi4s_vid_in_tvalid;
      pix_val_sel[0]  <= PIX_STORE;
      pix_val_sel[1]  <= PIX_STORE;
      pix_val_sel[2]  <= PIX_LIVE;
      pix_val_sel[3]  <= PIX_LIVE;
      if (axi4s_vid_in_tuser[0] || line_mode == 2'b00) begin  // first line
        // default overrides
        pix_val_sel[0]  <= PIX_DONTCARE;
        pix_val_sel[1]  <= PIX_DONTCARE;
        if (axi4s_vid_in_tuser[0]) begin  // first pixel
          pix_val_sel[0]  <= PIX_DEFAULT;
          pix_val_sel[2]  <= PIX_DEFAULT;
        end else if (axi4s_vid_in_tlast) begin  // last pixel
          pix_val_sel[1]  <= PIX_DEFAULT;
          pix_val_sel[3]  <= PIX_DEFAULT;
        end else begin    // subsequent pixels
          pix_val_en[2]   <= axi4s_vid_in_tvalid && pix_oddn_even;
          pix_val_en[3]   <= axi4s_vid_in_tvalid && ~pix_oddn_even;
        end
      end else if (line_mode == 2'b10) begin    // last line
        // default overrides
        pix_val_sel[0]  <= PIX_LIVE;
        pix_val_sel[1]  <= PIX_LIVE;
        pix_val_sel[2]  <= PIX_DONTCARE;
        pix_val_sel[3]  <= PIX_DONTCARE;
        if (pixel_first) begin  // first pixel
          pix_val_sel[0]  <= PIX_DEFAULT;
          pix_val_sel[2]  <= PIX_DEFAULT;
        end else if (axi4s_vid_in_tlast) begin  // last pixel
          pix_val_sel[1]  <= PIX_DEFAULT;
          pix_val_sel[3]  <= PIX_DEFAULT;
        end else begin    // subsequent pixels
          pix_val_en[0]   <= axi4s_vid_in_tvalid && pix_oddn_even;
          pix_val_en[1]   <= axi4s_vid_in_tvalid && ~pix_oddn_even;
        end
      end else begin  // subsequent lines
        if (pixel_first) begin  // first pixel
          pix_val_sel[0]  <= PIX_DEFAULT;
          pix_val_sel[2]  <= PIX_DEFAULT;
        end else if (axi4s_vid_in_tlast) begin  // last pixel
          pix_val_sel[1]  <= PIX_DEFAULT;
          pix_val_sel[3]  <= PIX_DEFAULT;
        end else begin    // subsequent pixels
          pix_val_en[0]   <= axi4s_vid_in_tvalid && pix_oddn_even;
          pix_val_en[1]   <= axi4s_vid_in_tvalid && ~pix_oddn_even;
          pix_val_en[2]   <= axi4s_vid_in_tvalid && pix_oddn_even;
          pix_val_en[3]   <= axi4s_vid_in_tvalid && ~pix_oddn_even;
        end
      end
    end

  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output Gen Control
  //
  //-----------------------------------------------------------------
  logic     vid_out_tvalid;
  logic     vid_out_tuser;
  logic     vid_out_tlast;

  always_ff @(posedge main_clock) begin


    if (~enable) begin
      vid_out_tvalid  <= 1'b0;

    end else if (data_out_ready) begin

      // Valid
      vid_out_tvalid  <= 1'b0;
      if (  axi4s_vid_in_tuser[0] || line_mode == 2'b00 || line_mode == 2'b10 ||
            (line_mode == 2'b01 && ~line_oddn_even && axi4s_vid_fifo_in_tvalid) ) begin
        vid_out_tvalid  <= axi4s_vid_in_tvalid && (~pix_oddn_even || axi4s_vid_in_tlast);
      end

    end

    if (data_out_ready) begin

      // tuser
      vid_out_tuser   <= axi4s_vid_in_tuser[0];

      // tlast
      vid_out_tlast   <= 1'b0;
      if (line_mode == 2'b00 || line_mode == 2'b10 || (line_mode == 2'b01 && ~line_oddn_even)) begin
        vid_out_tlast   <= axi4s_vid_in_tlast;
      end

    end
  end
  //-----------------------------------------------------------------------------


  //-----------------------------------------------------------------------------
  // Data In Conversion
  //
  // uint8 to fp16 conversion. RGB pixel Data from live and line store.
  // Each colour component is 8bits and requires conversion to IEEE 754 Half Float (FP16).
  //-----------------------------------------------------------------------------
  logic [47:0]    vid_fifo_in;
  logic [47:0]    vid_in;

  genvar coli_indx;
  generate

    // uint8 to fp16 - step through colour planes in (RGB)
    for (coli_indx = 0; coli_indx < 3; coli_indx++) begin

      always_ff @(posedge main_clock) begin
        if (data_out_ready) begin

          if (axi4s_vid_fifo_in_tdata[coli_indx*8+7]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10110 , axi4s_vid_fifo_in_tdata[coli_indx*8+6:coli_indx*8+0] , 3'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+6]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10101 , axi4s_vid_fifo_in_tdata[coli_indx*8+5:coli_indx*8+0] , 4'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+5]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10100 , axi4s_vid_fifo_in_tdata[coli_indx*8+4:coli_indx*8+0] , 5'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+4]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10011 , axi4s_vid_fifo_in_tdata[coli_indx*8+3:coli_indx*8+0] , 6'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+3]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10010 , axi4s_vid_fifo_in_tdata[coli_indx*8+2:coli_indx*8+0] , 7'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+2]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10001 , axi4s_vid_fifo_in_tdata[coli_indx*8+1:coli_indx*8+0] , 8'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+1]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10000 , axi4s_vid_fifo_in_tdata[coli_indx*8+0] , 9'b0};
          end else if (axi4s_vid_fifo_in_tdata[coli_indx*8+0]) begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b01111 , 10'b0};
          end else begin
            vid_fifo_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b0 , 10'b0};
          end

          if (axi4s_vid_in_tdata[coli_indx*8+7]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10110 , axi4s_vid_in_tdata[coli_indx*8+6:coli_indx*8+0] , 3'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+6]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10101 , axi4s_vid_in_tdata[coli_indx*8+5:coli_indx*8+0] , 4'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+5]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10100 , axi4s_vid_in_tdata[coli_indx*8+4:coli_indx*8+0] , 5'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+4]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10011 , axi4s_vid_in_tdata[coli_indx*8+3:coli_indx*8+0] , 6'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+3]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10010 , axi4s_vid_in_tdata[coli_indx*8+2:coli_indx*8+0] , 7'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+2]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10001 , axi4s_vid_in_tdata[coli_indx*8+1:coli_indx*8+0] , 8'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+1]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b10000 , axi4s_vid_in_tdata[coli_indx*8+0] , 9'b0};
          end else if (axi4s_vid_in_tdata[coli_indx*8+0]) begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b01111 , 10'b0};
          end else begin
            vid_in[coli_indx*16+15:coli_indx*16+0]   <= {1'b0 , 5'b0 , 10'b0};
          end

        end
      end

    end
  endgenerate
  //-----------------------------------------------------------------------------


  //-----------------------------------------------------------------
  // Data In Control
  //-----------------------------------------------------------------
  logic     vid_out_d1_tvalid;
  logic     vid_out_d1_tuser;
  logic     vid_out_d1_tlast;

  always_ff @(posedge main_clock) begin

    if (~enable) begin
      vid_out_d1_tvalid  <= 1'b0;

    end else if (data_out_ready) begin

      vid_out_d1_tvalid  <= vid_out_tvalid;

    end

    if (data_out_ready) begin

      vid_out_d1_tuser   <= vid_out_tuser;
      vid_out_d1_tlast   <= vid_out_tlast;

    end
  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output Data
  //-----------------------------------------------------------------
  localparam C_AXIS_PRE_OUT_PIXEL_BYTES   = NUMBER_OF_COLOR_PLANES_IN * BPS_OUT;
  localparam C_AXIS_PRE_OUT_WIDTH         = C_AXIS_OUT_PIXEL_BYTES * PIXELS_IN_PARALLEL_OUT;

  logic [C_AXIS_PRE_OUT_WIDTH-1:0]     vid_out_data;

  genvar pixo_indx;
  generate

    // Step through pixels out (4)
    for (pixo_indx = 0; pixo_indx < 4; pixo_indx++) begin

      always_ff @(posedge main_clock) begin
        if (data_out_ready) begin

          if (pix_val_en[pixo_indx]) begin
            case (pix_val_sel[pixo_indx])
              PIX_DEFAULT : begin
                vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_df_fp16_pix_val;   // red
                vid_out_data[pixo_indx*16+79:pixo_indx*16+64]    <= vid_df_fp16_pix_val;   // green
                vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_df_fp16_pix_val;   // blue
              end
              PIX_DONTCARE : begin
                vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_dc_fp16_pix_val;   // red
                vid_out_data[pixo_indx*16+79:pixo_indx*16+64]    <= vid_dc_fp16_pix_val;   // green
                vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_dc_fp16_pix_val;   // blue
              end
              PIX_STORE : begin
                if (rb_swap) begin
                  vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_fifo_in[15:0];    // blue
                end else begin
                  vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_fifo_in[47:32];   // red
                end
                vid_out_data[pixo_indx*16+79:pixo_indx*16+64]    <= vid_fifo_in[31:16];   // green
                if (rb_swap) begin
                  vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_fifo_in[47:32];   // red
                end else begin
                  vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_fifo_in[15:0];    // blue
                end
              end
              default : begin   // live input
                if (rb_swap) begin
                  vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_in[15:0];     // blue
                end else begin
                  vid_out_data[pixo_indx*16+15:pixo_indx*16+0]     <= vid_in[47:32];    // red
                end
                vid_out_data[pixo_indx*16+79:pixo_indx*16+64]    <= vid_in[31:16];    // green
                if (rb_swap) begin
                  vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_in[47:32];    // red
                end else begin
                  vid_out_data[pixo_indx*16+143:pixo_indx*16+128]  <= vid_in[15:0];     // blue
                end
              end
            endcase
          end
        end
      end

    end
  endgenerate
  //-----------------------------------------------------------------------------

  // output
  localparam C_AXIS_OUT_PAD_WIDTH = C_AXIS_OUT_WIDTH - C_AXIS_PRE_OUT_WIDTH;

  assign axi4s_vid_out_tdata  = {{C_AXIS_OUT_PAD_WIDTH-1{1'b0}} , vid_out_data};   // pad unused upper bits
  assign axi4s_vid_out_tlast  = vid_out_d1_tlast;
  assign axi4s_vid_out_tuser  = {{C_TUSER_OUT_WIDTH-1{1'b0}} , vid_out_d1_tuser}; // pad unused upper bits
  assign axi4s_vid_out_tvalid = vid_out_d1_tvalid;


  //-----------------------------------------------------------------
  // Debug
  //-----------------------------------------------------------------
  logic [15:0]    line_count_in_dbg;
  logic [15:0]    line_count_in_dbg_store;
  logic [ 2:0]    sof_in_seen;
  logic           input_line_err;
  logic [15:0]    pix_count_in_dbg;
  logic [15:0]    pix_count_in_dbg_store;
  logic           input_pix_err;

  logic [15:0]    line_count_out_dbg;
  logic [15:0]    line_count_out_dbg_store;
  logic [ 2:0]    sof_out_seen;
  logic           output_line_err;
  logic [15:0]    pix_count_out_dbg;
  logic [15:0]    pix_count_out_dbg_store;
  logic           output_pix_err;

  always_ff @(posedge main_clock) begin

    // Input
    if (~enable) begin
      line_count_in_dbg     <= 16'b0;
      sof_in_seen           <= 3'b0;
      input_line_err        <= 1'b0;
      pix_count_in_dbg      <= 16'b1;
      input_pix_err         <= 1'b0;

    end else if (data_out_ready && axi4s_vid_in_tvalid) begin

      // line counter
      if (axi4s_vid_in_tuser[0]) begin
        line_count_in_dbg        <= 16'b0;
        line_count_in_dbg_store  <= line_count_in_dbg;

        sof_in_seen       <= {sof_in_seen[1:0] , 1'b1};
        input_line_err    <= 1'b0;
        if (sof_in_seen == 3'b111 && line_count_in_dbg != line_count_in_dbg_store) begin
          input_line_err    <= 1'b1;
        end
      end else if (axi4s_vid_in_tlast) begin
        line_count_in_dbg    <= line_count_in_dbg + 1'b1;
      end

      // pixel counter
      if (axi4s_vid_in_tlast) begin
        pix_count_in_dbg         <= 16'b1;
        pix_count_in_dbg_store   <= pix_count_in_dbg;

        input_pix_err    <= 1'b0;
        if (sof_in_seen == 3'b111  && pix_count_in_dbg != pix_count_in_dbg_store) begin
          input_pix_err    <= 1'b1;
        end
      end else begin
        pix_count_in_dbg   <= pix_count_in_dbg + 1'b1;
      end

    end


    // Output
    if (~enable) begin
      line_count_out_dbg     <= 16'b0;
      sof_out_seen           <= 3'b0;
      output_line_err        <= 1'b0;
      pix_count_out_dbg      <= 16'b1;
      output_pix_err         <= 1'b0;

    end else if (data_out_ready) begin

      // Valid
      if (  (  axi4s_vid_in_tuser[0] || line_mode == 2'b00 || line_mode == 2'b10 ||
            (line_mode == 2'b01 && ~line_oddn_even && axi4s_vid_fifo_in_tvalid) ) &&
            (axi4s_vid_in_tvalid && (~pix_oddn_even || axi4s_vid_in_tlast)) ) begin

        // line counter
        if (axi4s_vid_in_tuser[0]) begin
          line_count_out_dbg        <= 16'b0;
          line_count_out_dbg_store  <= line_count_out_dbg;

          sof_out_seen       <= {sof_out_seen[1:0] , 1'b1};
          output_line_err    <= 1'b0;
          if (sof_out_seen == 3'b111 && line_count_out_dbg != line_count_out_dbg_store) begin
            output_line_err    <= 1'b1;
          end
        end else if (axi4s_vid_in_tlast && (line_mode == 2'b00 || line_mode == 2'b10 || (line_mode == 2'b01 && ~line_oddn_even))) begin
          line_count_out_dbg    <= line_count_out_dbg + 1'b1;
        end

        // pixel counter
          if (axi4s_vid_in_tlast && (line_mode == 2'b00 || line_mode == 2'b10 || (line_mode == 2'b01 && ~line_oddn_even))) begin
            pix_count_out_dbg         <= 16'b1;
            pix_count_out_dbg_store   <= pix_count_out_dbg;

            output_pix_err    <= 1'b0;
            if (sof_out_seen == 3'b111  && pix_count_out_dbg != pix_count_out_dbg_store) begin
              output_pix_err    <= 1'b1;
            end
        end else begin
          pix_count_out_dbg   <= pix_count_out_dbg + 1'b1;
        end

      end

    end

    if (main_reset) begin
      line_count_in_dbg     <= 16'b0;
      sof_in_seen           <= 3'b0;
      input_line_err        <= 1'b0;
      pix_count_in_dbg      <= 16'b1;
      input_pix_err         <= 1'b0;

      line_count_out_dbg     <= 16'b0;
      sof_out_seen           <= 3'b0;
      output_line_err        <= 1'b0;
      pix_count_out_dbg      <= 16'b1;
      output_pix_err         <= 1'b0;
    end

  end

  assign dbg_counters = {pix_count_out_dbg_store , line_count_out_dbg_store , pix_count_in_dbg_store , line_count_in_dbg_store};
  assign dbg_errs = {output_pix_err , output_line_err , input_pix_err , input_line_err};
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
