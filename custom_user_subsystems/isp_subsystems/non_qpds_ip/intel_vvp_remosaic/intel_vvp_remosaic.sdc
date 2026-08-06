###################################################################################
# Copyright (C) 2025 Altera Corporation
#
# This software and the related documents are Altera copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Altera's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the License.
###################################################################################

proc apply_cdc {from_list to_list {is_bus 1}} {
  if {$is_bus} {
    set_max_skew  -from $from_list -to $to_list -get_skew_value_from_clock_period src_clock_period -skew_value_multiplier 0.8
  }
  set_max_delay -from $from_list -to $to_list 100
  set_min_delay -from $from_list -to $to_list -100
  if { ![string equal "quartus_syn" $::TimeQuestInfo(nameofexecutable)] } { #avoid set_net_delay being executed during synthesis
    set_net_delay -from $from_list -to $to_list -max -get_value_from_clock_period dst_clock_period -value_multiplier 0.8
  }
}

proc apply_all_cdc {} {

  set inst [get_current_instance]

  # CPU Control Registers
  # agent_clk -> main_clk
  apply_cdc  [get_keepers -nowarn "$inst|u_intel_vvp_remosaic_cpu|r_cpu_conv_mode[*]"] [get_keepers -nowarn "$inst|u_intel_vvp_remosaic_cpu|r_vid_conv_mode_meta[*]"] 1

}

apply_all_cdc
