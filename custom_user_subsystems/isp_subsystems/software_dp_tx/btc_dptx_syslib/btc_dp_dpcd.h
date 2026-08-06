// ********************************************************************************
//
// Copyright 2001-2025 Altera Corporation
// This software and the related documents are Altera copyrighted materials,
// and your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise,
// you may not use, modify, copy, publish, distribute, disclose or transmit
// this software or the related documents without Altera's prior written permission.
//
// This software and the related documents are provided as is, with no express or
// implied warranties, other than those that are expressly stated in the License.
//
// ********************************************************************************
// DisplayPort Sink/Source System Library DPCD location map
//
// Description:
// DPCD list
// - Updated for DP2.0
// ********************************************************************************

#ifndef _BTC_DP_DPCD_INCLUDE
#define _BTC_DP_DPCD_INCLUDE

#define DPCD_ADDR_DPCD_REV                        0x0000
#define DPCD_ADDR_MAX_LINK_RATE                   0x0001
#define DPCD_ADDR_MAX_LANE_COUNT                  0x0002
#define DPCD_ADDR_MAX_DOWNSPREAD                  0x0003
#define DPCD_ADDR_NORP                            0x0004
#define DPCD_ADDR_DOWNSTREAMPORT_PRESENT          0x0005
#define DPCD_ADDR_MAIN_LINK_CHANNEL_CODING        0x0006
#define DPCD_ADDR_DOWN_STREAM_PORT_COUNT          0x0007
#define DPCD_ADDR_RECEIVE_PORT0_CAP_0             0x0008
#define DPCD_ADDR_RECEIVE_PORT0_CAP_1             0x0009
#define DPCD_ADDR_RECEIVE_PORT1_CAP_0             0x000A
#define DPCD_ADDR_RECEIVE_PORT1_CAP_1             0x000B
#define DPCD_ADDR_I2C_SPEED                       0x000C
#define DPCD_ADDR_EDP_CONFIGURATION_CAP           0x000D
#define DPCD_ADDR_TRAINING_AUX_RD_INTERVAL        0x000E
#define DPCD_ADDR_ADAPTER_CAP                     0x000F
#define DPCD_ADDR_SUPPORTED_LINK_RATES            0x0010

#define DPCD_ADDR_FAUX_CAP                        0x0020
#define DPCD_ADDR_SINK_VIDEO_FALLBACK_FORMATS     0x0020
#define DPCD_ADDR_MST_CAP                         0x0021
#define DPCD_ADDR_N_OF_AUDIO_ENDPOINTS            0x0022

#define DPCD_ADDR_GUID                            0x0030

#define DPCD_ADDR_RX_GTC_VALUE7_0                 0x0054
#define DPCD_ADDR_RX_GTC_VALUE15_8                0x0055
#define DPCD_ADDR_RX_GTC_VALUE23_16               0x0056
#define DPCD_ADDR_RX_GTC_VALUE31_24               0x0057
#define DPCD_ADDR_RX_GTC_MSTR_REQ                 0x0058
#define DPCD_ADDR_RX_GTC_FREQ_LOCK_DONE           0x0059
#define DPCD_ADDR_RX_GTC_PHASE_SKEW_OFFSET7_0     0x005A
#define DPCD_ADDR_RX_GTC_PHASE_SKEW_OFFSET15_8    0x005B

#define DPCD_ADDR_DSC_SUPPORT                     0x0060
#define DPCD_ADDR_DSC_ALGORITHM_REVISION          0x0061
#define DPCD_ADDR_DSC_RC_BUFFER_BLOCK_SIZE        0x0062
#define DPCD_ADDR_DSC_RC_BUFFER_SIZE              0x0063
#define DPCD_ADDR_DSC_SLICE_CAPABILITIES_1        0x0064
#define DPCD_ADDR_DSC_LINE_BUFFER_DEPTH           0x0065
#define DPCD_ADDR_DSC_BLOCK_PREDICTION_SUPPORT    0x0066
#define DPCD_ADDR_DSC_DECODER_COLOR_FORMAT_CAPAB  0x0069
#define DPCD_ADDR_DSC_DECODER_COLOR_DEPTH_CAPAB   0x006A
#define DPCD_ADDR_PEAK_DSC_THROUGHPUT             0x006B
#define DPCD_ADDR_DSC_MAXIMUM_SLICE_WIDTH         0x006C
#define DPCD_ADDR_DSC_SLICE_CAPABILITIES_2        0x006D
#define DPCD_ADDR_BITS_PER_PIXEL_INCREMENT        0x006F

#define DPCD_ADDR_FEC_CAPABILITY                  0x0090
#define DPCD_ADDR_FEC_CAPABILITY_0                0x0090
#define DPCD_ADDR_FEC_CAPABILITY_1                0x0091

#define DPCD_ADDR_PR_CAPABILITY_SUPPORTED         0x00B0
#define DPCD_ADDR_PR_CAPABILITY                   0x00B1
#define DPCD_ADDR_PR_SU_X_GRAN_CAPABILITY_0       0x00B2
#define DPCD_ADDR_PR_SU_X_GRAN_CAPABILITY_1       0x00B3
#define DPCD_ADDR_PR_SU_Y_GRAN_CAPABILITY         0x00B4

#define DPCD_ADDR_LINK_BW_SET                     0x0100
#define DPCD_ADDR_LANE_COUNT_SET                  0x0101
#define DPCD_ADDR_TRAINING_PATTERN_SET            0x0102
#define DPCD_ADDR_TRAINING_LANE0_SET              0x0103
#define DPCD_ADDR_TRAINING_LANE1_SET              0x0104
#define DPCD_ADDR_TRAINING_LANE2_SET              0x0105
#define DPCD_ADDR_TRAINING_LANE3_SET              0x0106
#define DPCD_ADDR_DOWNSPREAD_CTRL                 0x0107
#define DPCD_ADDR_MAIN_LINK_CHANNEL_CODING_SET    0x0108
#define DPCD_ADDR_I2C_SPEED1                      0x0109
#define DPCD_ADDR_EDP_CONFIGURATION_SET           0x010A
#define DPCD_ADDR_LINK_QUAL_LANE0_SET             0x010B
#define DPCD_ADDR_LINK_QUAL_LANE1_SET             0x010C
#define DPCD_ADDR_LINK_QUAL_LANE2_SET             0x010D
#define DPCD_ADDR_LINK_QUAL_LANE3_SET             0x010E
#define DPCD_ADDR_LINK_SQUARE_PATTERN_NUM         0x010F

#define DPCD_ADDR_CABLE_ATTRIBUTES_BY_DPTX        0x0110
#define DPCD_ADDR_MSTM_CTRL                       0x0111
#define DPCD_ADDR_AUDIO_DELAY0                    0x0112
#define DPCD_ADDR_AUDIO_DELAY1                    0x0113
#define DPCD_ADDR_AUDIO_DELAY2                    0x0114
#define DPCD_ADDR_TX_GTC_CAPABILITY               0x0115
#define DPCD_ADDR_TX_GTC_CAPABILITY               0x0115
#define DPCD_ADDR_UD_DP_PWR_NEEDED                0x0118
#define DPCD_ADDR_EXT_DPRX_SLEEP_WAKE_TO_GRANT    0x0119

#define DPCD_ADDR_FEC_CONFIGURATION               0x0120
#define DPCD_ADDR_SDP_ERROR_CONFIGURATION         0x0121

#define DPCD_ADDR_TX_GTC_VALUE7_0                 0x0154
#define DPCD_ADDR_TX_GTC_VALUE15_8                0x0155
#define DPCD_ADDR_TX_GTC_VALUE23_16               0x0156
#define DPCD_ADDR_TX_GTC_VALUE31_24               0x0157
#define DPCD_ADDR_RX_GTC_VALUE_PHASE_SKEW_EN      0x0158
#define DPCD_ADDR_TX_GTC_FREQ_LOCK_DONE           0x0159
#define DPCD_ADDR_TX_GTC_PHASE_SKEW_OFFSET7_0     0x015A
#define DPCD_ADDR_TX_GTC_PHASE_SKEW_OFFSET15_8    0x015B

#define DPCD_ADDR_DSC_ENABLE                      0x0160

#define DPCD_ADDR_ADAPTER_CTRL                    0x01A0
#define DPCD_ADDR_BRANCH_DEVICE_CTRL              0x01A1

#define DPCD_ADDR_PANEL_REPLAY_ENABLE_AND_CONFIG  0x01B0

#define DPCD_ADDR_PAYLOAD_ALLOCATE_SET            0x01C0
#define DPCD_ADDR_PAYLOAD_ALLOCATE_START_TSLOT    0x01C1
#define DPCD_ADDR_PAYLOAD_ALLOCATE_TSLOT_COUNT    0x01C2

#define DPCD_ADDR_SINK_COUNT                      0x0200
#define DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR       0x0201
#define DPCD_ADDR_LANE0_1_STATUS                  0x0202
#define DPCD_ADDR_LANE2_3_STATUS                  0x0203
#define DPCD_ADDR_LANE_ALIGN_STATUS_UPDATED       0x0204
#define DPCD_ADDR_SINK_STATUS                     0x0205
#define DPCD_ADDR_ADJUST_REQUEST_LANE0_1          0x0206
#define DPCD_ADDR_ADJUST_REQUEST_LANE2_3          0x0207

#define DPCD_ADDR_DSC_STATUS                      0x020F

#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE0_LS     0x0210
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE0_MS     0x0211
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE1_LS     0x0212
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE1_MS     0x0213
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE2_LS     0x0214
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE2_MS     0x0215
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE3_LS     0x0216
#define DPCD_ADDR_SYMBOL_ERROR_COUNT_LANE3_MS     0x0217

#define DPCD_ADDR_TEST_REQUEST                    0x0218
#define DPCD_ADDR_TEST_LINK_RATE                  0x0219
#define DPCD_ADDR_TEST_LANE_COUNT                 0x0220
#define DPCD_ADDR_PHY_TEST_PATTERN                0x0248
#define DPCD_ADDR_TEST_80BIT_CUSTOM_PATTERN       0x0250
#define DPCD_ADDR_TEST_RESPONSE                   0x0260
#define DPCD_ADDR_TEST_EDID_CHECKSUM              0x0261

#define DPCD_ADDR_TEST_SINK                       0x0270

#define DPCD_ADDR_FEC_STATUS                      0x0280
#define DPCD_ADDR_FEC_ERROR_COUNT_LS              0x0281
#define DPCD_ADDR_FEC_ERROR_COUNT_MS              0x0282

#define DPCD_ADDR_PAYLOAD_TABLE_UPDATE_STATUS     0x02C0
#define DPCD_ADDR_VC_PAYLOAD_ID_SLOT_1            0x02C1
#define DPCD_ADDR_VC_PAYLOAD_ID_SLOT_63           0x02FF

#define DPCD_ADDR_SOURCE_IEEE_OUI_0               0x0300

#define DPCD_ADDR_SINK_IEEE_OUI_0                 0x0400

#define DPCD_SINK_RESERVED_1                      0x040C

#define DPCD_ADDR_BRANCH_IEEE_OUI_0               0x0500

#define DPCD_BRANCH_RESERVED_1                    0x050C

#define DPCD_ADDR_SET_POWER_STATE                 0x0600

#define DPCD_ADDR_EDP_DPCD_REV                    0x0700
#define DPCD_ADDR_EDP_DISPLAY_CONTROL             0x0720

#define DPCD_ADDR_DOWN_REQ                        0x1000
#define DPCD_ADDR_UP_REP                          0x1200
#define DPCD_ADDR_DOWN_REP                        0x1400
#define DPCD_ADDR_UP_REQ                          0x1600

#define DPCD_ADDR_SINK_COUNT_ESI                  0x2002
#define DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR_ESI0  0x2003
#define DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR_ESI1  0x2004
#define DPCD_ADDR_LINK_SERVICE_IRQ_VECTOR_ESI0    0x2005
#define DPCD_ADDR_LANE0_1_STATUS_ESI              0x200C
#define DPCD_ADDR_LANE2_3_STATUS_ESI              0x200D
#define DPCD_ADDR_LANE_ALIGN_STATUS_UPDATED_ESI   0x200E
#define DPCD_ADDR_SINK_STATUS_ESI                 0x200F

#define DPCD_ADDR_PANEL_REPLAY_ERROR_STATUS       0x2020
#define DPCD_ADDR_SUPR_EVENT_STATUS_INDICATOR     0x2021
#define DPCD_ADDR_SINK_DEVICE_PANEL_REPLAY_STATUS 0x2022
#define DPCD_ADDR_DEBUG_1_LAST_RECEIVED_VSC_SDP   0x2024

#define DPCD_ADDR_EXTENDED_CAPAB_FIELD            0x2200
#define DPCD_ADDR_EXT_DPCD_REV                    0x2200
#define DPCD_ADDR_EXT_MAX_LINK_RATE               0x2201
#define DPCD_ADDR_EXT_MAX_LANE_COUNT              0x2202
#define DPCD_ADDR_EXT_MAX_DOWNSPREAD              0x2203
#define DPCD_ADDR_EXT_NORP                        0x2204
#define DPCD_ADDR_EXT_DOWNSTREAMPORT_PRESENT      0x2205
#define DPCD_ADDR_EXT_MAIN_LINK_CHANNEL_CODING    0x2206
#define DPCD_ADDR_EXT_DOWN_STREAM_PORT_COUNT      0x2207
#define DPCD_ADDR_EXT_RECEIVE_PORT0_CAP_0         0x2208
#define DPCD_ADDR_EXT_RECEIVE_PORT0_CAP_1         0x2209
#define DPCD_ADDR_EXT_RECEIVE_PORT1_CAP_0         0x220A
#define DPCD_ADDR_EXT_RECEIVE_PORT1_CAP_1         0x220B
#define DPCD_ADDR_EXT_I2C_SPEED                   0x220C
#define DPCD_ADDR_EXT_EDP_CONFIGURATION_CAP       0x220D
#define DPCD_ADDR_EXT_TRAINING_AUX_RD_INTERVAL    0x220E
#define DPCD_ADDR_EXT_ADAPTER_CAP                 0x220F
#define DPCD_ADDR_DPRX_FEATURE_ENUMERATION_LIST   0x2210
#define DPCD_ADDR_EXT_ADAPTIVE_SYNC_CAPABILITY    0x2214
#define DPCD_ADDR_EXT_LINK_RATE                   0x2215
#define DPCD_ADDR_EXT_128B132B_TRAINING_AUX_RD_INTERVAL   0x2216
#define DPCD_ADDR_CABLE_ATTRIBUTES_BY_DPRX        0x2217

#define DPCD_ADDR_TEST_264BIT_CUSTOM_PATTERN      0x2230
#define DPCD_ADDR_CONTINUOUS_264BIT_CAP           0x2251
#define DPCD_ADDR_CONTINUOUS_264BIT_CTRL          0x2252

#define DPCD_ADDR_HDCP1_BKSV                      0x68000
#define DPCD_ADDR_HDCP1_R0                        0x68005
#define DPCD_ADDR_HDCP1_AKSV                      0x68007
#define DPCD_ADDR_HDCP1_AN                        0x6800C
#define DPCD_ADDR_HDCP1_V                         0x68014
#define DPCD_ADDR_HDCP1_BCAPS                     0x68028
#define DPCD_ADDR_HDCP1_BSTATUS                   0x68029
#define DPCD_ADDR_HDCP1_BINFO                     0x6802A
#define DPCD_ADDR_HDCP1_KSVFIFO                   0x6802C
#define DPCD_ADDR_HDCP1_AINFO                     0x6803B
#define DPCD_ADDR_HDCP1_RSVD                      0x6803C

#define DPCD_ADDR_HDCP2_RTX                       0x69000
#define DPCD_ADDR_HDCP2_TXCAPS                    0x69008
#define DPCD_ADDR_HDCP2_CERTRX                    0x6900B
#define DPCD_ADDR_HDCP2_RRX                       0x69215
#define DPCD_ADDR_HDCP2_RXCAPS                    0x6921D
#define DPCD_ADDR_HDCP2_EKPUB_KM                  0x69220
#define DPCD_ADDR_HDCP2_EKH_KM_WR                 0x692A0
#define DPCD_ADDR_HDCP2_M                         0x692B0
#define DPCD_ADDR_HDCP2_HPRIME                    0x692C0
#define DPCD_ADDR_HDCP2_EKH_KM_RD                 0x692E0
#define DPCD_ADDR_HDCP2_RN                        0x692F0
#define DPCD_ADDR_HDCP2_LPRIME                    0x692F8
#define DPCD_ADDR_HDCP2_EDKEY_KS                  0x69318
#define DPCD_ADDR_HDCP2_RIV                       0x69328
#define DPCD_ADDR_HDCP2_RXINFO                    0x69330
#define DPCD_ADDR_HDCP2_SEQ_NUM_V                 0x69332
#define DPCD_ADDR_HDCP2_VPRIME                    0x69335
#define DPCD_ADDR_HDCP2_RECEIVER_ID_LIST          0x69345
#define DPCD_ADDR_HDCP2_V                         0x693E0
#define DPCD_ADDR_HDCP2_SEQ_NUM_M                 0x693F0
#define DPCD_ADDR_HDCP2_K                         0x693F3
#define DPCD_ADDR_HDCP2_STREAMID_TYPE             0x693F5
#define DPCD_ADDR_HDCP2_MPRIME                    0x69473
#define DPCD_ADDR_HDCP2_RXSTATUS                  0x69493

#define DPCD_ADDR_LTTPR_CAP_AND_ID                      0xF0000
#define DPCD_ADDR_PHY_REPEATER_8B10B_MAX_LINK_RATE      0xF0001
#define DPCD_ADDR_PHY_REPEATER_COUNT                    0xF0002
#define DPCD_ADDR_PHY_REPEATER_MODE                     0xF0003
#define DPCD_ADDR_PHY_REPEATER_MAX_LANE_COUNT           0xF0004
#define DPCD_ADDR_PHY_REPEATER_MAINLINK_CHANNEL_CODING  0xF0006
#define DPCD_ADDR_PHY_REPEATER_128B132B_DP_RATES        0xF0007
#define DPCD_ADDR_TOTAL_LTTPR_CNT                       0xF000A

#define DPCD_ADDR_TRAINING_PATTERN_SET_PHY_REP1         0xF0010

//------------ Locations not yet handled follow -----------------//

#define DPCD_ADDR_DWN_STRM_PORT0_CAP              0x0080

#define DPCD_ADDR_TRAINING_SCORE_LANE0            0x0208
#define DPCD_ADDR_TRAINING_SCORE_LANE1            0x0209
#define DPCD_ADDR_TRAINING_SCORE_LANE2            0x020A
#define DPCD_ADDR_TRAINING_SCORE_LANE3            0x020B
#define DPCD_ADDR_TEST_PATTERN                    0x0221
#define DPCD_ADDR_TEST_H_TOTAL_LSB                0x0222
#define DPCD_ADDR_TEST_H_TOTAL_MSB                0x0223
#define DPCD_ADDR_TEST_V_TOTAL_LSB                0x0224
#define DPCD_ADDR_TEST_V_TOTAL_MSB                0x0225
#define DPCD_ADDR_TEST_H_START_LSB                0x0226
#define DPCD_ADDR_TEST_H_START_MSB                0x0227
#define DPCD_ADDR_TEST_V_START_LSB                0x0228
#define DPCD_ADDR_TEST_V_START_MSB                0x0229
#define DPCD_ADDR_TEST_HSYNC_LSB                  0x022A
#define DPCD_ADDR_TEST_HSYNC_MSB                  0x022B
#define DPCD_ADDR_TEST_VSYNC_LSB                  0x022C
#define DPCD_ADDR_TEST_VSYNC_MSB                  0x022D
#define DPCD_ADDR_TEST_H_WIDTH_LSB                0x022E
#define DPCD_ADDR_TEST_H_WIDTH_MSB                0x022F
#define DPCD_ADDR_TEST_V_HEIGHT_LSB               0x0230
#define DPCD_ADDR_TEST_V_HEIGHT_MSB               0x0231
#define DPCD_ADDR_TEST_MISC_LSB                   0x0232
#define DPCD_ADDR_TEST_MISC_MSB                   0x0233
#define DPCD_ADDR_TEST_REFRESH_RATE_NUMERATOR     0x0234
#define DPCD_ADDR_TEST_CRC_R_CR_LSB               0x0240
#define DPCD_ADDR_TEST_CRC_R_CR_MSB               0x0241
#define DPCD_ADDR_TEST_CRC_G_Y_LSB                0x0242
#define DPCD_ADDR_TEST_CRC_G_Y_MSB                0x0243
#define DPCD_ADDR_TEST_CRC_B_CB_LSB               0x0244
#define DPCD_ADDR_TEST_CRC_B_CB_MSB               0x0245
#define DPCD_ADDR_TEST_SINK_MISC                  0x0246

#endif //#ifndef _BTC_DP_DPCD_INCLUDE
