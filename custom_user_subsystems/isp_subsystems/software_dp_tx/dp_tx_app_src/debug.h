/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// ********************************************************************************
// DisplayPort Core test code debug routines definitions
//
// Description:
//
// ********************************************************************************

void bitec_dp_dump_aux_debug_init(unsigned int base_addr_csr);
void bitec_dp_dump_aux_debug(unsigned int base_addr_csr, unsigned int base_addr_fifo,
                             unsigned int is_sink);
#if DP_SUPPORT_RX                             
void bitec_dp_dump_sink_msa(unsigned int base_addr);
void bitec_dp_dump_sink_config(unsigned int base_addr);
#endif /* DP_SUPPORT_RX */

#if DP_SUPPORT_TX
void bitec_dp_dump_source_msa(unsigned int base_addr);
void bitec_dp_dump_source_config(unsigned int base_addr);
#endif /* DP_SUPPORT_TX */

char* bitec_get_stdin();
