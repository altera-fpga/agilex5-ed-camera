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
 * Module: intel_vvp_exposure_fusion_shim
 *
 * Description: AXI shim for the Exposure Fusion
 *
 * ##################################################################################
*/

`default_nettype none

module intel_vvp_exposure_fusion_shim #(
  parameter TDATA_BYTES = 8
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
  axi4s_vid_out_tready
);

  //  Constants  //
  localparam C_TDATA_WIDTH = TDATA_BYTES * 8;
  localparam C_TUSER_WIDTH = TDATA_BYTES;

  //  Top Level Signals  //
  input  logic                         main_clock;
  input  logic                         main_reset;

  input  logic [    C_TDATA_WIDTH-1:0] axi4s_vid_in_tdata;
  input  logic                         axi4s_vid_in_tlast;
  input  logic [    C_TUSER_WIDTH-1:0] axi4s_vid_in_tuser;
  input  logic                         axi4s_vid_in_tvalid;
  output logic                         axi4s_vid_in_tready;

  output logic [    C_TDATA_WIDTH-1:0] axi4s_vid_out_tdata;
  output logic                         axi4s_vid_out_tlast;
  output logic [    C_TUSER_WIDTH-1:0] axi4s_vid_out_tuser;
  output logic                         axi4s_vid_out_tvalid;
  input  logic                         axi4s_vid_out_tready;

  //-----------------------------------------------------------------
  // axis shim
  //-----------------------------------------------------------------
  always_ff @(posedge main_clock) begin : a_op_if
    logic [C_TDATA_WIDTH-1:0] tdata_reg;
    logic [C_TUSER_WIDTH-1:0] tuser_reg;
    logic                     tlast_reg;

    axi4s_vid_in_tready  <= (axi4s_vid_out_tready || (axi4s_vid_in_tready  && (~axi4s_vid_out_tvalid || ~axi4s_vid_in_tvalid )));
    axi4s_vid_out_tvalid <= (axi4s_vid_in_tvalid  || (axi4s_vid_out_tvalid && (~axi4s_vid_in_tready  || ~axi4s_vid_out_tready)));

    // 1 Reg deep FIFO.
    if (axi4s_vid_in_tready) begin
      tdata_reg <= axi4s_vid_in_tdata;
      tlast_reg <= axi4s_vid_in_tlast;
      tuser_reg <= axi4s_vid_in_tuser;
    end

    // Select between FIFO or input data.
    if (axi4s_vid_out_tready || ~axi4s_vid_out_tvalid) begin
      axi4s_vid_out_tdata <= axi4s_vid_in_tdata;
      axi4s_vid_out_tlast <= axi4s_vid_in_tlast;
      axi4s_vid_out_tuser <= axi4s_vid_in_tuser;
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
