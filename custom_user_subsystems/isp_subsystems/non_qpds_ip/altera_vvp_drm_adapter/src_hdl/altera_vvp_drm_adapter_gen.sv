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
 * Module: altera_vvp_drm_adapter_gen
 *
 * Description: Generate the VVP DRM Adapter
 *
 * ##################################################################################
*/

`default_nettype none

module altera_vvp_drm_adapter_gen #(
  parameter NUMBER_OF_COLOR_PLANES                  =  4, // standard VVP name - only supports 4
  parameter PIXELS_IN_PARALLEL                      =  1, // standard VVP name - only support 1 or 2
  parameter BPS_IN                                  =  8, // standard VVP name - only supports 8
  parameter BPS_OUT                                 = 10  // standard VVP name - only supports 10
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
  r_vid_control
);


  //  Constants  //
  localparam C_PADDED_BPS_IN        = (BPS_IN < 8) ? 8 : BPS_IN;
  localparam C_AXIS_IN_PIXEL_BYTES  = ((NUMBER_OF_COLOR_PLANES*C_PADDED_BPS_IN)+7)/8*8;
  localparam C_AXIS_IN_WIDTH        = C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH       = C_AXIS_IN_WIDTH/8;

  localparam C_PADDED_BPS_OUT       = (BPS_OUT < 8) ? 8 : BPS_OUT;
  localparam C_AXIS_OUT_PIXEL_BYTES = ((NUMBER_OF_COLOR_PLANES*C_PADDED_BPS_OUT)+7)/8*8;
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

  input logic [31:0]                    r_vid_control;


  //  Local Parameters  //
  // drm mode
  localparam logic [1:0] ARGB8888   = 2'b00;
  localparam logic [1:0] ARGB4444   = 2'b01;
  localparam logic [1:0] ARGB2222   = 2'b10;
  localparam logic [1:0] UNUSED     = 2'b11;

  // packing modes
  localparam logic [1:0] PIXEL1       = 2'b00;
  localparam logic [1:0] PIXEL2       = 2'b01;
  localparam logic [1:0] PIXEL3       = 2'b10;
  localparam logic [1:0] PIXEL4       = 2'b11;

  //  Signals  //
  logic [1:0]         drm_mode;
  logic [1:0]         packed_pixel_count;
  logic [1:0]         packed_pixel_count_d1;

  logic [PIXELS_IN_PARALLEL-1:0][3:0][NUMBER_OF_COLOR_PLANES-1:0][BPS_OUT-1:0]    data_out_4pix;
  logic [PIXELS_IN_PARALLEL-1:0][1:0][NUMBER_OF_COLOR_PLANES-1:0][BPS_OUT-1:0]    data_out_2pix;
  logic [PIXELS_IN_PARALLEL-1:0][0:0][NUMBER_OF_COLOR_PLANES-1:0][BPS_OUT-1:0]    data_out_1pix;

  logic [PIXELS_IN_PARALLEL-1:0][C_AXIS_OUT_PIXEL_BYTES-1:0]        data_out;
  logic                                                             last_out;
  logic                                                             user_out;
  logic                                                             axi4s_vid_in_tready_int;

  // set drm mode from cpu register
  assign drm_mode   = r_vid_control[1:0];


  //-----------------------------------------------------------------
  // If input data represents more than 1 pixel, then unpack according to the CPU register.
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin : a_pix_cnt
    if (main_reset) begin
      packed_pixel_count    <= 2'b00;

    end else begin
      if (axi4s_vid_in_tvalid && axi4s_vid_in_tready_int) begin
        if (packed_pixel_count != 2'b00) begin
          packed_pixel_count      <= packed_pixel_count - 1'b1;  // decrement
        end else begin
          case (drm_mode)
            ARGB4444 : begin
              packed_pixel_count    <= 2'b01;
            end
            ARGB2222 : begin
              packed_pixel_count    <= 2'b11;
            end
            default : begin   // ARGB8888
              packed_pixel_count    <= 2'b00;
            end
          endcase
        end
        packed_pixel_count_d1   <= packed_pixel_count;  // used to determine unpacking completed
      end

    end
  end
  //-----------------------------------------------------------------

  // Must gate the tready until all pixels have been unpacked
  assign axi4s_vid_in_tready = (drm_mode == ARGB8888 || (packed_pixel_count == 2'b00 && packed_pixel_count_d1 != 2'b00)) ? axi4s_vid_in_tready_int : 1'b0;


  //-----------------------------------------------------------------
  // Output data
  // create all the valid options and map to the output data.
  //-----------------------------------------------------------------
  always_comb begin
    // Mapping 4 x 2b input samples to 4 x 10b output samples
    // 1 per color plane and 1 set per pip
    data_out_4pix[0][0][0]  = { axi4s_vid_in_tdata[1:0] , {8{axi4s_vid_in_tdata[0]}} };
    data_out_4pix[0][0][1]  = { axi4s_vid_in_tdata[3:2] , {8{axi4s_vid_in_tdata[2]}} };
    data_out_4pix[0][0][2]  = { axi4s_vid_in_tdata[5:4] , {8{axi4s_vid_in_tdata[4]}} };
    data_out_4pix[0][0][3]  = { axi4s_vid_in_tdata[7:6] , {8{axi4s_vid_in_tdata[6]}} };
    data_out_4pix[0][1][0]  = { axi4s_vid_in_tdata[9:8] , {8{axi4s_vid_in_tdata[8]}} };
    data_out_4pix[0][1][1]  = { axi4s_vid_in_tdata[11:10] , {8{axi4s_vid_in_tdata[10]}} };
    data_out_4pix[0][1][2]  = { axi4s_vid_in_tdata[13:12] , {8{axi4s_vid_in_tdata[12]}} };
    data_out_4pix[0][1][3]  = { axi4s_vid_in_tdata[15:14] , {8{axi4s_vid_in_tdata[14]}} };
    data_out_4pix[0][2][0]  = { axi4s_vid_in_tdata[17:16] , {8{axi4s_vid_in_tdata[16]}} };
    data_out_4pix[0][2][1]  = { axi4s_vid_in_tdata[19:18] , {8{axi4s_vid_in_tdata[18]}} };
    data_out_4pix[0][2][2]  = { axi4s_vid_in_tdata[21:20] , {8{axi4s_vid_in_tdata[20]}} };
    data_out_4pix[0][2][3]  = { axi4s_vid_in_tdata[23:22] , {8{axi4s_vid_in_tdata[22]}} };
    data_out_4pix[0][3][0]  = { axi4s_vid_in_tdata[25:24] , {8{axi4s_vid_in_tdata[24]}} };
    data_out_4pix[0][3][1]  = { axi4s_vid_in_tdata[27:26] , {8{axi4s_vid_in_tdata[26]}} };
    data_out_4pix[0][3][2]  = { axi4s_vid_in_tdata[29:28] , {8{axi4s_vid_in_tdata[28]}} };
    data_out_4pix[0][3][3]  = { axi4s_vid_in_tdata[31:30] , {8{axi4s_vid_in_tdata[30]}} };
    if (PIXELS_IN_PARALLEL > 1) begin
      data_out_4pix[1][0][0]  = { axi4s_vid_in_tdata[32+1:32+0] , {8{axi4s_vid_in_tdata[32+0]}} };
      data_out_4pix[1][0][1]  = { axi4s_vid_in_tdata[32+3:32+2] , {8{axi4s_vid_in_tdata[32+2]}} };
      data_out_4pix[1][0][2]  = { axi4s_vid_in_tdata[32+5:32+4] , {8{axi4s_vid_in_tdata[32+4]}} };
      data_out_4pix[1][0][3]  = { axi4s_vid_in_tdata[32+7:32+6] , {8{axi4s_vid_in_tdata[32+6]}} };
      data_out_4pix[1][1][0]  = { axi4s_vid_in_tdata[32+9:32+8] , {8{axi4s_vid_in_tdata[32+8]}} };
      data_out_4pix[1][1][1]  = { axi4s_vid_in_tdata[32+11:32+10] , {8{axi4s_vid_in_tdata[32+10]}} };
      data_out_4pix[1][1][2]  = { axi4s_vid_in_tdata[32+13:32+12] , {8{axi4s_vid_in_tdata[32+12]}} };
      data_out_4pix[1][1][3]  = { axi4s_vid_in_tdata[32+15:32+14] , {8{axi4s_vid_in_tdata[32+14]}} };
      data_out_4pix[1][2][0]  = { axi4s_vid_in_tdata[32+17:32+16] , {8{axi4s_vid_in_tdata[32+16]}} };
      data_out_4pix[1][2][1]  = { axi4s_vid_in_tdata[32+19:32+18] , {8{axi4s_vid_in_tdata[32+18]}} };
      data_out_4pix[1][2][2]  = { axi4s_vid_in_tdata[32+21:32+20] , {8{axi4s_vid_in_tdata[32+20]}} };
      data_out_4pix[1][2][3]  = { axi4s_vid_in_tdata[32+23:32+22] , {8{axi4s_vid_in_tdata[32+22]}} };
      data_out_4pix[1][3][0]  = { axi4s_vid_in_tdata[32+25:32+24] , {8{axi4s_vid_in_tdata[32+24]}} };
      data_out_4pix[1][3][1]  = { axi4s_vid_in_tdata[32+27:32+26] , {8{axi4s_vid_in_tdata[32+26]}} };
      data_out_4pix[1][3][2]  = { axi4s_vid_in_tdata[32+29:32+28] , {8{axi4s_vid_in_tdata[32+28]}} };
      data_out_4pix[1][3][3]  = { axi4s_vid_in_tdata[32+31:32+30] , {8{axi4s_vid_in_tdata[32+30]}} };
    end

    // Mapping 2 x 4b input samples to 2 x 10b output samples
    // 1 per color plane and 1 set per pip
    data_out_2pix[0][0][0]  = { axi4s_vid_in_tdata[3:0] , {6{axi4s_vid_in_tdata[0]}} };
    data_out_2pix[0][0][1]  = { axi4s_vid_in_tdata[7:4] , {6{axi4s_vid_in_tdata[4]}} };
    data_out_2pix[0][0][2]  = { axi4s_vid_in_tdata[11:8] , {6{axi4s_vid_in_tdata[8]}} };
    data_out_2pix[0][0][3]  = { axi4s_vid_in_tdata[15:12] , {6{axi4s_vid_in_tdata[12]}} };
    data_out_2pix[0][1][0]  = { axi4s_vid_in_tdata[19:16] , {6{axi4s_vid_in_tdata[16]}} };
    data_out_2pix[0][1][1]  = { axi4s_vid_in_tdata[23:20] , {6{axi4s_vid_in_tdata[20]}} };
    data_out_2pix[0][1][2]  = { axi4s_vid_in_tdata[27:24] , {6{axi4s_vid_in_tdata[24]}} };
    data_out_2pix[0][1][3]  = { axi4s_vid_in_tdata[31:28] , {6{axi4s_vid_in_tdata[28]}} };
    if (PIXELS_IN_PARALLEL > 1) begin
      data_out_2pix[1][0][0]  = { axi4s_vid_in_tdata[32+3:32+0] , {6{axi4s_vid_in_tdata[32+0]}} };
      data_out_2pix[1][0][1]  = { axi4s_vid_in_tdata[32+7:32+4] , {6{axi4s_vid_in_tdata[32+4]}} };
      data_out_2pix[1][0][2]  = { axi4s_vid_in_tdata[32+11:32+8] , {6{axi4s_vid_in_tdata[32+8]}} };
      data_out_2pix[1][0][3]  = { axi4s_vid_in_tdata[32+15:32+12] , {6{axi4s_vid_in_tdata[32+12]}} };
      data_out_2pix[1][1][0]  = { axi4s_vid_in_tdata[32+19:32+16] , {6{axi4s_vid_in_tdata[32+16]}} };
      data_out_2pix[1][1][1]  = { axi4s_vid_in_tdata[32+23:32+20] , {6{axi4s_vid_in_tdata[32+20]}} };
      data_out_2pix[1][1][2]  = { axi4s_vid_in_tdata[32+27:32+24] , {6{axi4s_vid_in_tdata[32+24]}} };
      data_out_2pix[1][1][3]  = { axi4s_vid_in_tdata[32+31:32+28] , {6{axi4s_vid_in_tdata[32+28]}} };
    end

    // Mapping 1 x 8b input sample to 1 x 10b output sample
    // 1 per color plane and 1 set per pip
    data_out_1pix[0][0][0]  = { axi4s_vid_in_tdata[7:0] , {2{axi4s_vid_in_tdata[0]}} };
    data_out_1pix[0][0][1]  = { axi4s_vid_in_tdata[15:8] , {2{axi4s_vid_in_tdata[8]}} };
    data_out_1pix[0][0][2]  = { axi4s_vid_in_tdata[23:16] , {2{axi4s_vid_in_tdata[16]}} };
    data_out_1pix[0][0][3]  = { axi4s_vid_in_tdata[31:24] , {2{axi4s_vid_in_tdata[24]}} };
    if (PIXELS_IN_PARALLEL > 1) begin
      data_out_1pix[1][0][0]  = { axi4s_vid_in_tdata[32+7:32+0] , {2{axi4s_vid_in_tdata[32+0]}} };
      data_out_1pix[1][0][1]  = { axi4s_vid_in_tdata[32+15:32+8] , {2{axi4s_vid_in_tdata[32+8]}} };
      data_out_1pix[1][0][2]  = { axi4s_vid_in_tdata[32+23:32+16] , {2{axi4s_vid_in_tdata[32+16]}} };
      data_out_1pix[1][0][3]  = { axi4s_vid_in_tdata[32+31:32+24] , {2{axi4s_vid_in_tdata[32+24]}} };
    end

    // select data out
    case (drm_mode)
      ARGB4444 : begin
        case (packed_pixel_count)
          PIXEL2 : begin
            data_out[0] = {data_out_2pix[0][0][3] , data_out_2pix[0][0][2] , data_out_2pix[0][0][1] , data_out_2pix[0][0][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_2pix[1][0][3] , data_out_2pix[1][0][2] , data_out_2pix[1][0][1] , data_out_2pix[1][0][0]};
            end
            last_out = 1'b0;
            user_out = axi4s_vid_in_tuser;
          end
          default : begin   // PIXEL1
            data_out[0] = {data_out_2pix[0][1][3] , data_out_2pix[0][1][2] , data_out_2pix[0][1][1] , data_out_2pix[0][1][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_2pix[1][1][3] , data_out_2pix[1][1][2] , data_out_2pix[1][1][1] , data_out_2pix[1][1][0]};
            end
            last_out = axi4s_vid_in_tlast;
            user_out = 1'b0;
          end
        endcase
      end
      ARGB2222 : begin
        case (packed_pixel_count)
          PIXEL4 : begin
            data_out[0] = {data_out_4pix[0][0][3] , data_out_4pix[0][0][2] , data_out_4pix[0][0][1] , data_out_4pix[0][0][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_4pix[1][0][3] , data_out_4pix[1][0][2] , data_out_4pix[1][0][1] , data_out_4pix[1][0][0]};
            end
            last_out = 1'b0;
            user_out = axi4s_vid_in_tuser;
          end
          PIXEL3 : begin
            data_out[0] = {data_out_4pix[0][1][3] , data_out_4pix[0][1][2] , data_out_4pix[0][1][1] , data_out_4pix[0][1][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_4pix[1][1][3] , data_out_4pix[1][1][2] , data_out_4pix[1][1][1] , data_out_4pix[1][1][0]};
            end
            last_out = 1'b0;
            user_out = 1'b0;
          end
          PIXEL2 : begin
            data_out[0] = {data_out_4pix[0][2][3] , data_out_4pix[0][2][2] , data_out_4pix[0][2][1] , data_out_4pix[0][2][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_4pix[1][2][3] , data_out_4pix[1][2][2] , data_out_4pix[1][2][1] , data_out_4pix[1][2][0]};
            end
            last_out = 1'b0;
            user_out = 1'b0;
          end
          default : begin   // PIXEL1
            data_out[0] = {data_out_4pix[0][3][3] , data_out_4pix[0][3][2] , data_out_4pix[0][3][1] , data_out_4pix[0][3][0]};
            if (PIXELS_IN_PARALLEL > 1) begin
              data_out[1] = {data_out_4pix[1][3][3] , data_out_4pix[1][3][2] , data_out_4pix[1][3][1] , data_out_4pix[1][3][0]};
            end
            last_out = axi4s_vid_in_tlast;
            user_out = 1'b0;
          end
        endcase
      end
      default : begin   // ARGB8888 PIXEL1
        data_out[0] = {data_out_1pix[0][0][3] , data_out_1pix[0][0][2] , data_out_1pix[0][0][1] , data_out_1pix[0][0][0]};
        if (PIXELS_IN_PARALLEL > 1) begin
          data_out[1] = {data_out_1pix[1][0][3] , data_out_1pix[1][0][2] , data_out_1pix[1][0][1] , data_out_1pix[1][0][0]};
        end
        last_out = axi4s_vid_in_tlast;
        user_out = axi4s_vid_in_tuser;
      end
    endcase

  end
  //-----------------------------------------------------------------


  //-----------------------------------------------------------------
  // Output axis shim
  //-----------------------------------------------------------------
    logic [C_AXIS_OUT_WIDTH-1:0]  tdata_reg;
    logic                         tlast_reg;
    logic                         tuser_reg;

  always_ff @(posedge main_clock) begin : a_op_if
    if (main_reset) begin
      axi4s_vid_in_tready_int   <= 1'b1;
      axi4s_vid_out_tvalid      <= 1'b0;

    end else begin
      axi4s_vid_in_tready_int   <= (axi4s_vid_out_tready || (axi4s_vid_in_tready_int && (~axi4s_vid_out_tvalid || ~axi4s_vid_in_tvalid)));
      axi4s_vid_out_tvalid      <= (axi4s_vid_in_tvalid || (axi4s_vid_out_tvalid && (~axi4s_vid_in_tready_int || ~axi4s_vid_out_tready)));

      // 1 Reg deep FIFO.
      if (axi4s_vid_in_tready_int) begin
        if (PIXELS_IN_PARALLEL > 1) begin
          tdata_reg <= { data_out[1] , data_out[0] };
        end else begin
          tdata_reg <= data_out[0];
        end
        tlast_reg <= last_out;
        tuser_reg <= user_out;
      end

      // Select between FIFO or input data.
      if (axi4s_vid_out_tready || ~axi4s_vid_out_tvalid) begin
        if (PIXELS_IN_PARALLEL > 1) begin
          axi4s_vid_out_tdata <= { data_out[1] , data_out[0] };
        end else begin
          axi4s_vid_out_tdata <= data_out[0];
        end
        axi4s_vid_out_tlast <= last_out;
        axi4s_vid_out_tuser <= {{(C_TUSER_OUT_WIDTH-1){1'b0}} , user_out};
        if (~axi4s_vid_in_tready_int) begin
          axi4s_vid_out_tdata <= tdata_reg;
          axi4s_vid_out_tlast <= tlast_reg;
          axi4s_vid_out_tuser <= {{(C_TUSER_OUT_WIDTH-1){1'b0}} , tuser_reg};
        end
      end
    end
  end
  //-----------------------------------------------------------------

endmodule

`default_nettype wire
