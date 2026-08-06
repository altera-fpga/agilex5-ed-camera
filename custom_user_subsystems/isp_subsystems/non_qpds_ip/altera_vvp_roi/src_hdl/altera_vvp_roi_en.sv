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
 * Module: altera_vvp_roi_en
 *
 * Description: Generate the Region Of Interest Enable
 *
 * ##################################################################################
*/

`default_nettype none

module altera_vvp_roi_en  #(
  parameter PIXELS_IN_PARALLEL = 1
) (
  // Video Clock and Reset
  main_clock,
  main_reset,

  // Input
  valid,
  ready,
  eol,
  sof,

  // CPU Regs
  h_start,
  h_end,
  v_start,
  v_end,

  // output
  enable
);

  //  Functions  //
  function integer vvp_clog2;
      input [31:0] value;
      integer i;
      begin
          vvp_clog2 = 32;
          for (i=31; i>0; i=i-1) begin
              if (2**i >= value) begin
                  vvp_clog2 = i;
              end
          end
      end
  endfunction

  //  Constants  //
  localparam MAX_FRAME_SIZE_BITS      = 16;


  //  Top Level Signals  //
  input  logic                               main_clock;
  input  logic                               main_reset;

  input  logic                               valid;
  input  logic                               ready;
  input  logic                               eol;
  input  logic                               sof;

  input  logic [MAX_FRAME_SIZE_BITS - 1 : 0] h_start;
  input  logic [MAX_FRAME_SIZE_BITS - 1 : 0] h_end;
  input  logic [MAX_FRAME_SIZE_BITS - 1 : 0] v_start;
  input  logic [MAX_FRAME_SIZE_BITS - 1 : 0] v_end;

  output logic [ PIXELS_IN_PARALLEL - 1 : 0] enable;


  localparam LOG2_PIP     = $clog2(PIXELS_IN_PARALLEL);
  localparam PIP_BITS     = vvp_clog2(PIXELS_IN_PARALLEL);

  logic [MAX_FRAME_SIZE_BITS - 1 : 0]   h_start_pip;
  logic [MAX_FRAME_SIZE_BITS - 1 : 0]   h_end_pip;

  logic [MAX_FRAME_SIZE_BITS - 1 : 0]   h_count;
  logic [MAX_FRAME_SIZE_BITS - 1 : 0]   v_count;
  logic                                 prime;
  logic                                 h_enable_start;
  logic                                 h_enable_end;
  logic                                 h_enable;
  logic                                 v_enable;

  logic [PIXELS_IN_PARALLEL-1:0]        h_enable_start_mask;
  logic [PIXELS_IN_PARALLEL-1:0]        h_enable_end_mask;
  logic [PIXELS_IN_PARALLEL-1:0]        h_enable_mask;


  // Generate pip aligned decodes
  assign h_start_pip = {{LOG2_PIP{1'b0}}, h_start[MAX_FRAME_SIZE_BITS-1:LOG2_PIP]};
  assign h_end_pip   = {{LOG2_PIP{1'b0}}, h_end[MAX_FRAME_SIZE_BITS-1:LOG2_PIP]};


  // count valid pixels, resetting each line and at SOF
  always_ff @( posedge main_clock ) begin
    if (main_reset) begin
      h_count           <= {MAX_FRAME_SIZE_BITS{1'b0}};
      v_count           <= {MAX_FRAME_SIZE_BITS{1'b0}};
      prime             <= 1'b0;
      enable            <= {PIXELS_IN_PARALLEL{1'b0}};
      h_enable_start    <= 1'b0;
      h_enable_end      <= 1'b0;
      h_enable          <= 1'b0;
      v_enable          <= 1'b0;

    end else begin
      // Generate Frame Counters
      if (valid && ready) begin
        if (sof) begin
          h_count   <= {MAX_FRAME_SIZE_BITS{1'b0}};
          v_count   <= {MAX_FRAME_SIZE_BITS{1'b0}};
          prime     <= 1'b0;
        end else begin
          if (eol) begin
            prime     <= 1'b1;
          end else begin
            prime     <= 1'b0;
          end
          if (prime) begin
            h_count   <= {MAX_FRAME_SIZE_BITS{1'b0}};
            v_count   <= v_count + 1'b1;
          end else begin
            h_count   <= h_count + 1'b1;
          end
        end
      end

      // Generate Output Enables
      if (ready) begin
        // H Enable start and end decodes for enable masking
        if (h_count == h_start_pip) begin
          h_enable_start    <= 1'b1;
        end else begin
          h_enable_start    <= 1'b0;
        end
        if (h_count == h_end_pip) begin
          h_enable_end      <= 1'b1;
        end else begin
          h_enable_end      <= 1'b0;
        end;

        // H Enable
        if (h_count == h_start_pip) begin
          h_enable  <= 1'b1;
        end else if (h_enable_end) begin
          h_enable  <= 1'b0;
        end

        // V Enable
        if (v_count >= v_start && v_count <= v_end) begin
          v_enable  <= 1'b1;
        end else begin
          v_enable  <= 1'b0;
        end

        // Output Enable
        for (int i = 0; i < PIXELS_IN_PARALLEL; i++) begin
          enable[i]  <= h_enable && h_enable_mask[i] && v_enable;
        end
      end

    end
  end


  // Generate Start and End position Enable Masks
  if (PIXELS_IN_PARALLEL > 1) begin
    genvar i;
    for (i = 0; i < PIXELS_IN_PARALLEL; i++) begin
      assign h_enable_start_mask[i] = (i >= h_start[PIP_BITS-1:0]) ? 1'b1 : 1'b0;
      assign h_enable_end_mask[i]   = (i <= h_end[PIP_BITS-1:0]) ? 1'b1 : 1'b0;
    end
  end else begin
    assign h_enable_start_mask = 1'b1;
    assign h_enable_end_mask = 1'b1;
  end

  // Select the Mask (default is all on)
  assign h_enable_mask = (h_enable_start) ? h_enable_start_mask : (h_enable_end) ? h_enable_end_mask : {PIXELS_IN_PARALLEL{1'b1}};

endmodule

`default_nettype wire
