/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// ********************************************************************************
// DisplayPort Core test code configuration
//
// Description:
//
// ********************************************************************************

#define BITEC_RX_AUX_DEBUG          0 // Set to 1 to enable AUX CH traffic monitoring
#define BITEC_TX_AUX_DEBUG          0 // Set to 1 to enable AUX CH traffic monitoring
#define BITEC_STATUS_DEBUG          1 // Set to 1 to enable MSA and link status monitoring

// RX Capabilities
#define DP_SUPPORT_RX               0 // Set to 1 if DP support RX
#define DP_SUPPORT_RX_HDCP          0
#define DP_RX_BPS                   1 // Video bit depth
#define BITEC_RX_GPUMODE            1 // Set to 1 to enable Sink GPU-mode
#define BITEC_RX_CAPAB_MST          0 // Set to 1 to enable MST support
#define BITEC_RX_FAST_LT_SUPPORT    0 // Set to 1 to enable Fast Link Training support
#define BITEC_RX_LQA_SUPPORT        0 // Set to 1 to enable Link Quality Analysis support
#define BITEC_RX_SDP_SUPPORT        0
#define BITEC_EDID_800X600_AUDIO    0 // Set to 1 to use an EDID with max resolution 800 x 600
#define BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_MST 0
// TX Capabilities
#define DP_SUPPORT_TX               1 // Set to 1 if DP support TX
#define DP_SUPPORT_TX_HDCP          0
#define DP_TX_BPS                   2 // Video bit depth
#define BITEC_TX_CAPAB_MST          0 // Set to 1 to enable MST support
#define TX_VIDEO_IM_ENABLE          0 // Set to 1 to enable TX Video IM interface
// EDID PassThru from Sink to Source
#define DP_SUPPORT_EDID_PASSTHRU    0 // Set to 1 to enable EDID passthru from Sink to Source.
                                      // Else DP Sink will use default EDID.
                                      // Only Support EDID passthru when both Tx and Rx is supported
#define BITEC_DP_CARD_REV           100 // Value is the FMC revision. Rev. 4 - 8: no Retimer, Rev 9 - 10 : Paradetech Retimer, Rev.11+: Megachip Retimer.
#define SELECTED_BOARD              3 // Value from HW.Tcl that determines target board per FPGA family
#define MST_RX_STREAMS              1 // RX MST number of streams
#define MST_TX_STREAMS              1 // TX MST number of streams
#define PSG_8K_EDID                 0 // set to 1 if Sink support 8K
#define DP_SUPPORT_HDCP_KEY_MANAGE  0

#define DP_SUPPORT_AXI              1 // set to 1 to enable Video over AXI bridge


#define DP_SUPPORT_RX_FEC           0
#define DP_SUPPORT_RX_DSC           0
#define DP_RX_FEC_ERRINJ_EN         0
#define DP_SUPPORT_TX_DSC           0
#define DP_SUPPORT_TX_FEC           0

