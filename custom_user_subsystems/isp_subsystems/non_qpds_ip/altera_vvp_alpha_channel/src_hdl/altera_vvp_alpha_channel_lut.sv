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
 * Module: altera_vvp_alpha_channel_lut
 *
 * Description: Alpha Channel LUT
 *
 * ##################################################################################
*/

`default_nettype none

module altera_vvp_alpha_channel_lut #(
  int    DATA_WIDTH,
  int    DATA_DEPTH,
  int    ADDR_WIDTH = $clog2(DATA_DEPTH),
  string RAMSTYLE,
  int    DELAY
  ) (
  input wire                   clk_rd,
  input wire                   clk_wr,

  input wire                   i_read_enable,
  input wire [ADDR_WIDTH-1:0]  i_read_addr,
  output logic [DATA_WIDTH-1:0] o_read_data,

  input wire                   i_write_enable,
  input wire [ADDR_WIDTH-1:0]  i_write_addr,
  input wire [DATA_WIDTH-1:0]  i_write_data
  );
  // From the Verilog template. Slightly modified
  // Declare the RAM variable
	(* ramstyle = RAMSTYLE *) logic [DATA_WIDTH-1:0] ram[2**ADDR_WIDTH-1:0];

  case (DELAY)
    0 : begin
	    always_ff @(posedge clk_wr) begin
		    // Write
		    if (i_write_enable) ram[i_write_addr] <= i_write_data;
	    end
      assign o_read_data = ram[i_read_addr];
    end
    1 : begin
	    always_ff @ (posedge clk_wr) begin
		    // Write
		    if (i_write_enable) ram[i_write_addr] <= i_write_data;
	    end

      always_ff @ (posedge clk_rd) begin
        // Read
        o_read_data <= ram[i_read_addr];
      end
    end
    2 : begin
      always_ff @ (posedge clk_wr) begin
		    // Write
    		if (i_write_enable) ram[i_write_addr] <= i_write_data;
	    end
  
      logic [DATA_WIDTH-1:0] read_data;
      always_ff @ (posedge clk_rd) begin
        // Read
        read_data <= ram[i_read_addr];
        o_read_data <= read_data;
      end
    end
  endcase
endmodule
