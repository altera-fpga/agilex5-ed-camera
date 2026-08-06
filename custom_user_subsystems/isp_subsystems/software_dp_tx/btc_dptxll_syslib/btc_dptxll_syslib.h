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
// DisplayPort Source Link Layer System Library public definitions
//
// Description:
//
// ********************************************************************************

#ifndef _BTC_DPTXLL_SYSLIB_H_
#define _BTC_DPTXLL_SYSLIB_H_

#include <stdio.h>
#include "btc_dptx_syslib.h"

#ifndef BTC_DPTXLL_SYSLIB_VER
#define BTC_DPTXLL_SYSLIB_VER "6.3.3795"
#endif

typedef struct  // MST device port
{
  BTC_RAD RAD;              // Device RAD
  BTC_MST_DEVICE_PORT port; // Device port
  BYTE GUID[16];            // Device GUID
  unsigned int full_PBN;
  unsigned int available_PBN;
}BTC_MST_DEVPORT;

typedef struct // MST stream
{
  BYTE col_format : 3;        // 0 = RGB, 1 = YCbCr4:4:4, 2 = YCbCr4:2:2, 3 = YCbCr4:2:0, 4 = Y-Only, 5 = RAW
  BYTE col_bpc : 3;           // 0 = 6bpc, 1 = 8bpc, 2 = 10bpc, 3 = 12bpc, 4 = 16bpc, 5 = 7bpc, 6 = 14bpc
  BYTE col_range : 1;         // 0 = VESA, 1 = CEA
  BYTE col_colorimetry : 4;   // 0 = BT601-5, 1 = BT709-5, etc.
  BYTE col_usevsc : 1;        // 0 = use MISC0, 1 = use VSC SDP
  unsigned int pixel_rate;    // Pixel rate (kilopixels/s)
  unsigned int peak_BW;       // Peak stream bandwidth (kbytes/s)
  unsigned int PBN;           // Payload Bandwidth Number (units of 54/64 MBytes/sec)
  unsigned int PBN_alloc;     // Allocated Payload Bandwidth Number (units of 54/64 MBytes/sec)
  BYTE VCP_size;              // VCP size (0 - 63)
  unsigned int avg_tslots;    // Average timeslots per MTP (multiplied by 1000)
  unsigned int target_avg_tslots;        // Target average timeslots per MTP (multiplied by 1000)
  unsigned int max_target_avg_tslots;    // Max target average timeslots per MTP (multiplied by 1000)
}BTC_STREAM;

// MST CONNECTION_STATUS_NOTIFY callback type
typedef void (BTC_MST_CSN_CALLBACK) (BTC_MST_CONN_STAT_NOTIFY *csn_data);

//********** btc_dptxll_common.c *********//
int btc_dptxll_syslib_add_tx(BYTE tx_idx,unsigned int max_link_rate,unsigned int max_lane_count,unsigned int tx_num_of_sources,BYTE *edid_buf);
int btc_dptxll_syslib_init(void);
int btc_dptxll_syslib_set_max_link_rate(BYTE tx_idx,unsigned int max_link_rate);
unsigned int btc_dptxll_syslib_get_max_link_rate(BYTE tx_idx);
int btc_dptxll_syslib_set_max_lane_count(BYTE tx_idx,unsigned int max_lane_count);
unsigned int btc_dptxll_syslib_get_max_lane_count(BYTE tx_idx);
int btc_dptxll_syslib_monitor(void);
void btc_dptxll_sw_ver(BYTE *major,BYTE *minor, unsigned int *rev);
void btc_dptxll_mult_32x16(unsigned int in32, unsigned int in16, unsigned int *upper24, unsigned int *lower24);

//********** btc_dptxll_hpd.c *********//
int btc_dptxll_hpd_change(BYTE tx_idx,unsigned int asserted);
int btc_dptxll_hpd_irq(BYTE tx_idx);

//********** btc_dptxll_stream.c *********//
int btc_dptxll_stream_set_color_space(BYTE tx_idx,BYTE strm_idx,BYTE format,BYTE bpc,BYTE range,BYTE colorimetry,BYTE use_vsc_sdp);
int btc_dptxll_stream_set_pixel_rate(BYTE tx_idx,BYTE strm_idx,unsigned int pixel_rate_kpps);
int dp2p0_dptxll_stream_set_vfreq(BYTE tx_idx,BYTE strm_idx,unsigned int pixel_rate_kpps);
BYTE btc_dptxll_stream_calc_VCP_size(BYTE tx_idx,BYTE strm_idx);
int btc_dptxll_stream_allocate_req(BYTE tx_idx,BYTE strm_idx, BTC_MST_DEVPORT *dev_port);
int btc_dptxll_stream_allocate_rep(BYTE tx_idx);
int btc_dptxll_stream_delete_req(BYTE tx_idx,BYTE strm_idx, BTC_RAD *device_RAD, BYTE port_number);
int btc_dptxll_stream_delete_rep(BYTE tx_idx);
BTC_STREAM *btc_dptxll_stream_get(BYTE tx_idx,BYTE strm_idx);

//********** btc_dptxll_mst.c *********//
void dp2p0_wr_skip_mst_payload_messages(BYTE tx_idx, BYTE skip_allocate_payload);
int btc_dptxll_mst_get_device_ports(BYTE tx_idx,BTC_MST_DEVPORT **port_list,BYTE *num_of_ports);
int btc_dptxll_mst_edid_read_req(BYTE tx_idx,BTC_RAD *device_RAD,BYTE port_number);
int btc_dptxll_mst_edid_read_rep(BYTE tx_idx,BYTE **edid_data);
void btc_dptxll_mst_set_csn_callback(BYTE tx_idx,BTC_MST_CSN_CALLBACK *cback);
int btc_dptxll_mst_topology_discover(BYTE tx_idx,BTC_RAD *device_RAD);
int btc_dptxll_mst_cmp_ports(BTC_RAD *A_RAD,BYTE A_port_number,BTC_RAD *B_RAD,BYTE B_port_number);

#endif /* _BTC_DPTXLL_SYSLIB_H_ */

