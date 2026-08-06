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
 * Module: intel_vvp_exposure_fusion_gen
 *
 * Description: Generation of the Exposure Fusion
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_exposure_fusion_gen #(
  parameter NUMBER_OF_COLOR_PLANES  =  1, // standard VVP name
  parameter PIXELS_IN_PARALLEL      =  2, // standard VVP name
  parameter BPS_IN                  = 12, // standard VVP name
  parameter BPS_OUT                 = 16  // standard VVP name
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
  r_vid_output_mode,    // 00: exposure fusion, 01: short exposure, 10: reserved (for medium exposure), 11: long exposure
  r_vid_black_level,    // Black level
  r_vid_exposure_ratio, // Short / long exposure ratio
  r_vid_threshold       // Threshold value to start blending
);


  //  Constants  //
  localparam C_AXIS_NUM_IN_STREAMS = 2;
  localparam C_AXIS_IN_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*BPS_IN+7)/8*8;
  localparam C_AXIS_IN_WIDTH       = C_AXIS_NUM_IN_STREAMS*C_AXIS_IN_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_IN_WIDTH      = C_AXIS_IN_WIDTH/8;

  localparam C_AXIS_OUT_PIXEL_BYTES = (NUMBER_OF_COLOR_PLANES*BPS_OUT+7)/8*8;
  localparam C_AXIS_OUT_WIDTH       = C_AXIS_OUT_PIXEL_BYTES*PIXELS_IN_PARALLEL;
  localparam C_TUSER_OUT_WIDTH      = C_AXIS_OUT_WIDTH/8;

  localparam OUTPUT_MODE_WIDTH    = 2;
  localparam BPS_MAX              = 16;
  localparam EXPOSURE_RATIO_WIDTH = 17;

  localparam ALPHA_BITS = BPS_IN - 1;
  localparam logic [ALPHA_BITS-1:0] ALPHA_ONE = 1 << (ALPHA_BITS - 1);

  localparam BITS_PER_PIP = 16;

  //  Top Level Signals  //
  input  logic                            main_clock;
  input  logic                            main_reset;

  input  logic [     C_AXIS_IN_WIDTH-1:0] axi4s_vid_in_tdata;
  input  logic                            axi4s_vid_in_tlast;
  input  logic [    C_TUSER_IN_WIDTH-1:0] axi4s_vid_in_tuser;
  input  logic                            axi4s_vid_in_tvalid;
  output logic                            axi4s_vid_in_tready;

  output logic [    C_AXIS_OUT_WIDTH-1:0] axi4s_vid_out_tdata;
  output logic                            axi4s_vid_out_tlast;
  output logic [   C_TUSER_OUT_WIDTH-1:0] axi4s_vid_out_tuser;
  output logic                            axi4s_vid_out_tvalid;
  input  logic                            axi4s_vid_out_tready;

  input  logic [   OUTPUT_MODE_WIDTH-1:0] r_vid_output_mode;
  input  logic [             BPS_MAX-1:0] r_vid_black_level;
  input  logic [EXPOSURE_RATIO_WIDTH-1:0] r_vid_exposure_ratio;
  input  logic [             BPS_MAX-1:0] r_vid_threshold;


  // Subtract black level to make both streams linear. The result is kept signed since we do not want to disturb the
  // dark noise by clipping at zero this early in the ISP pipeline.
  logic [EXPOSURE_RATIO_WIDTH:0] exposure_ratio;

  logic [C_AXIS_IN_WIDTH/2-1:0] axi4s_vid_in_0_tdata;
  logic [C_AXIS_IN_WIDTH/2-1:0] axi4s_vid_in_1_tdata;

  logic signed [PIXELS_IN_PARALLEL-1:0][BPS_OUT:0] data_in_0;
  logic signed [PIXELS_IN_PARALLEL-1:0][BPS_OUT:0] data_in_1;

  assign axi4s_vid_in_0_tdata = axi4s_vid_in_tdata[C_AXIS_IN_WIDTH/2-1:0];
  assign axi4s_vid_in_1_tdata = axi4s_vid_in_tdata[C_AXIS_IN_WIDTH-1:C_AXIS_IN_WIDTH/2];

  for (genvar i = 0; i < PIXELS_IN_PARALLEL; ++i) begin : gen_black_level
    assign data_in_0[i] = ($signed({1'b0, axi4s_vid_in_0_tdata[BITS_PER_PIP*i+:BPS_IN]}) - $signed({1'b0, r_vid_black_level[BPS_IN-1:0]})) << (BPS_OUT - BPS_IN);
    assign data_in_1[i] = ($signed({1'b0, axi4s_vid_in_1_tdata[BITS_PER_PIP*i+:BPS_IN]}) - $signed({1'b0, r_vid_black_level[BPS_IN-1:0]})) << (BPS_OUT - BPS_IN);
  end

  // Buffer the input data
  localparam COMPUTE_BUFFER_WIDTH = 18;
  localparam DATA_BUFFER_DEPTH = PIXELS_IN_PARALLEL == 1 ? 2 : PIXELS_IN_PARALLEL;
  localparam DATA_IN_BUFFER_DELAY = 8;

  logic [DATA_BUFFER_DEPTH-1:0][COMPUTE_BUFFER_WIDTH-1:0] data_in_0_buff [DATA_IN_BUFFER_DELAY];
  logic [DATA_BUFFER_DEPTH-1:0][COMPUTE_BUFFER_WIDTH-1:0] data_in_1_buff [DATA_IN_BUFFER_DELAY];

  logic [COMPUTE_BUFFER_WIDTH+EXPOSURE_RATIO_WIDTH+1:0] scaled_data_0     [PIXELS_IN_PARALLEL];
  logic [COMPUTE_BUFFER_WIDTH+EXPOSURE_RATIO_WIDTH+1:0] scaled_data_0_reg [PIXELS_IN_PARALLEL];

  always_ff @(posedge main_clock) begin
    if (axi4s_vid_in_tready) begin
      // Pipeline stage #0
      exposure_ratio <= r_vid_output_mode == 2'b00 ? {1'b0, r_vid_exposure_ratio} : {1'b1, {EXPOSURE_RATIO_WIDTH{1'b0}}};
      if (PIXELS_IN_PARALLEL == 1) begin
        data_in_0_buff[0][0] <= {{(COMPUTE_BUFFER_WIDTH-BPS_OUT-1){data_in_0[0][BPS_OUT]}}, data_in_0[0]};
        data_in_0_buff[0][1] <= data_in_0_buff[0][0];
        data_in_1_buff[0][0] <= {{(COMPUTE_BUFFER_WIDTH-BPS_OUT-1){data_in_1[0][BPS_OUT]}}, data_in_1[0]};
        data_in_1_buff[0][1] <= data_in_1_buff[0][0];
      end else begin
        for (int i = 0; i < PIXELS_IN_PARALLEL; ++i) begin
          data_in_0_buff[0][i] <= {{(COMPUTE_BUFFER_WIDTH-BPS_OUT-1){data_in_0[i][BPS_OUT]}}, data_in_0[i]};
          data_in_1_buff[0][i] <= {{(COMPUTE_BUFFER_WIDTH-BPS_OUT-1){data_in_1[i][BPS_OUT]}}, data_in_1[i]};
        end
      end
      // Pipeline stage #1
      for (int i = 0; i < PIXELS_IN_PARALLEL; ++i) begin : gen_scaled_data
        scaled_data_0[i] <= $signed(data_in_0_buff[0][i]) * $signed({1'b0, exposure_ratio});
      end
      data_in_1_buff[1] <= r_vid_output_mode == 2'b11 ? '0 : data_in_1_buff[0];
      // Pipeline stage #2
      scaled_data_0_reg <= scaled_data_0;
      data_in_1_buff[2] <= data_in_1_buff[1];
      // Pipeline stage #3
      for (int i = 0; i < PIXELS_IN_PARALLEL; ++i) begin
        // Limitation: Slicing/shifting signed signal rounds to -inf instead of 0. We do not care since negative numbers will be negative noise component with no visible impact.
        data_in_0_buff[3][i] <= {{(COMPUTE_BUFFER_WIDTH-BPS_OUT-1){scaled_data_0_reg[i][EXPOSURE_RATIO_WIDTH+BPS_OUT+1]}}, scaled_data_0_reg[i][EXPOSURE_RATIO_WIDTH+:BPS_OUT+1]};
        data_in_1_buff[3][i] <= data_in_1_buff[2][i];
      end
      // Pipeline stage #4 - #6
      for (int i = 4; i < DATA_IN_BUFFER_DELAY; ++i) begin
        data_in_0_buff[i] <= data_in_0_buff[i-1];
        data_in_1_buff[i] <= data_in_1_buff[i-1];
      end
    end
  end

  // Exposure fusion algorithm
  logic [C_AXIS_OUT_WIDTH-1:0] data_out_im;
  logic [C_AXIS_OUT_WIDTH-1:0] data_out;

  localparam PIP_LOOP = PIXELS_IN_PARALLEL == 1 ? PIXELS_IN_PARALLEL : PIXELS_IN_PARALLEL - 1;

  for (genvar i = 0; i < PIP_LOOP; i+=2) begin : gen_compute
    logic [COMPUTE_BUFFER_WIDTH-1:0] data_max;
    logic [COMPUTE_BUFFER_WIDTH-1:0] alpha_extended;
    logic [          ALPHA_BITS-1:0] alpha;
    logic [          ALPHA_BITS-1:0] alpha_reg;
    logic [          ALPHA_BITS-1:0] one_minus_alpha;
    logic [          ALPHA_BITS-1:0] one_minus_alpha_reg;

    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH-1:0] data_blend_im_00;
    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH-1:0] data_blend_im_01;
    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH-1:0] data_blend_im_10;
    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH-1:0] data_blend_im_11;
    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH  :0] data_blend    [2];
    logic [ALPHA_BITS+COMPUTE_BUFFER_WIDTH  :0] data_blend_reg[2];

    logic signed [BPS_OUT:0] data_blend_bl [2];


    // Treat adjoint pixels as a pair.
    //  - If both of them are below the threshold use the long exposure (set alpha to 0).
    //  - If any one of them is over the threshold alpha blend long and short exposures.
    for (genvar j = 0; j < 2; ++j) begin : gen_add_bl
      // Limitation: Slicing/shifting signed signal rounds to -inf instead of 0. We do not care since negative numbers will be negative noise component with no visible impact.
      assign data_blend_bl[j] = $signed(data_blend_reg[j][ALPHA_BITS-1+:BPS_OUT+1]) + $signed({1'b0, r_vid_black_level[BPS_IN-1:0], {(BPS_OUT-BPS_IN){1'b0}}});
    end

    always_ff @(posedge main_clock) begin
      if (axi4s_vid_in_tready) begin
        // Pipeline stage #3
        data_max <= $signed(data_in_0_buff[3][i]) > $signed(data_in_0_buff[3][i+1]) ? data_in_0_buff[3][i] : data_in_0_buff[3][i+1];
        // Pipeline stage #4
        if ($signed(data_max) > $signed({1'b0, r_vid_threshold[BPS_MAX-1:0]})) begin
          alpha_extended <= $signed(data_max) - $signed({1'b0, r_vid_threshold[BPS_MAX-1:0]});
        end else begin
          alpha_extended <= '0;
        end
        // Pipeline stage #5
        if (r_vid_output_mode == 2'b11) begin // Bypass long exposure
          alpha           <= '0;
          one_minus_alpha <= ALPHA_ONE;
        end else if (r_vid_output_mode == 2'b10) begin // Bypass medium exposure
          // Not implemented, don't care
        end else if (r_vid_output_mode == 2'b01 || alpha_extended > ALPHA_ONE) begin
          alpha           <= ALPHA_ONE;
          one_minus_alpha <= '0;
        end else begin
          alpha           <=             alpha_extended[ALPHA_BITS-1:0];
          one_minus_alpha <= ALPHA_ONE - alpha_extended[ALPHA_BITS-1:0];
        end
        // Pipeline stage #6
        alpha_reg           <= alpha;
        one_minus_alpha_reg <= one_minus_alpha;
        // Pipeline stage #7
        data_blend_im_00 <= $signed({1'b0,           alpha_reg}) * $signed(data_in_1_buff[DATA_IN_BUFFER_DELAY-1][i  ]);
        data_blend_im_01 <= $signed({1'b0, one_minus_alpha_reg}) * $signed(data_in_0_buff[DATA_IN_BUFFER_DELAY-1][i  ]);
        data_blend_im_10 <= $signed({1'b0,           alpha_reg}) * $signed(data_in_1_buff[DATA_IN_BUFFER_DELAY-1][i+1]);
        data_blend_im_11 <= $signed({1'b0, one_minus_alpha_reg}) * $signed(data_in_0_buff[DATA_IN_BUFFER_DELAY-1][i+1]);
        // Pipeline stage #8
        data_blend[0] <= $signed(data_blend_im_00) + $signed(data_blend_im_01);
        data_blend[1] <= $signed(data_blend_im_10) + $signed(data_blend_im_11);
        // Pipeline stage #9
        data_blend_reg <= data_blend;
        // Pipeline stage #10
        data_out_im[(i)*BITS_PER_PIP+:BPS_OUT] <= data_blend_bl[0][BPS_OUT-1:0];
        if (PIXELS_IN_PARALLEL > 1) begin
          data_out_im[(i+1)*BITS_PER_PIP+:BPS_OUT] <= data_blend_bl[1][BPS_OUT-1:0];
        end
      end
    end
  end
  always_ff @(posedge main_clock) begin
    if (axi4s_vid_in_tready) begin
      // Pipeline stage #11
      data_out <= data_out_im;
    end
  end

  // Total delay of the compute pipeline
  localparam PIPELINE_STAGES = 13;

  logic [PIPELINE_STAGES-1:0]                       valid_pipe;
  logic [PIPELINE_STAGES-1:0][C_TUSER_IN_WIDTH-1:0] user_pipe;
  logic [PIPELINE_STAGES-1:0]                       last_pipe;

  logic                                             valid_out;
  logic                     [C_TUSER_OUT_WIDTH-1:0] user_out;
  logic                                             last_out;

  always_ff @(posedge main_clock) begin
    if (main_reset) begin
      valid_pipe <= {PIPELINE_STAGES             {1'b0}};
      user_pipe  <= {PIPELINE_STAGES*BITS_PER_PIP{1'b0}};
      last_pipe  <= {PIPELINE_STAGES             {1'b0}};
    end else begin
      if (axi4s_vid_in_tready) begin
        valid_pipe[0] <= axi4s_vid_in_tvalid;
        user_pipe [0] <= axi4s_vid_in_tuser;
        last_pipe [0] <= axi4s_vid_in_tlast;
        for (int i = 1; i < PIPELINE_STAGES; i++) begin
          valid_pipe[i] <= valid_pipe[i-1];
          user_pipe [i] <= user_pipe [i-1];
          last_pipe [i] <= last_pipe [i-1];
        end
      end
    end
  end
  assign last_out  = last_pipe [PIPELINE_STAGES-1];
  assign user_out  = user_pipe [PIPELINE_STAGES-1][C_TUSER_OUT_WIDTH-1:0];
  assign valid_out = valid_pipe[PIPELINE_STAGES-1];

  //-----------------------------------------------------------------
  // Output axis shim
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin : a_op_if
    logic [C_AXIS_OUT_WIDTH-1:0] tdata_reg;
    logic [C_TUSER_IN_WIDTH-1:0] tuser_reg;
    logic                        tlast_reg;

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

    if (main_reset) begin
      axi4s_vid_in_tready  <= 1'b1;
      axi4s_vid_out_tvalid <= 1'b0;
    end
  end

endmodule

`default_nettype wire
