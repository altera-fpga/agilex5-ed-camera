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
// DisplayPort System Library public definitions
//
// Description:
//
// ********************************************************************************

#ifndef _BTC_DPTX_SYSLIB_H_
#define _BTC_DPTX_SYSLIB_H_

#include <stdio.h>
#include "btc_dp_types.h"
#include "btc_dp_dpcd.h"
#include "btc_dp_txregs.h"

#ifndef BTC_DPTX_SYSLIB_VER
#define BTC_DPTX_SYSLIB_VER "6.3.3795"
#endif

#define BTC_100US_TXTICKS     1 //number of timer time ticks equ to 100 us

// Enable / Disable IRQ on HPD events
#define BTC_DPTX_ENABLE_HPD_IRQ(tx_idx)     IOWR(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL,IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) | (1 << 31))
#define BTC_DPTX_DISABLE_HPD_IRQ(tx_idx)    IOWR(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL,IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) & ~(1 << 31))
#define BTC_DPTX_ISENABLED_HPD_IRQ(tx_idx)  ((IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) >> 31) & 0x01)

// Enable / Disable IRQ on AUX Replies from sink
#define BTC_DPTX_ENABLE_AUX_IRQ(tx_idx)     IOWR(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL,IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) | (1 << 30))
#define BTC_DPTX_DISABLE_AUX_IRQ(tx_idx)    IOWR(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL,IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) & ~(1 << 30))
#define BTC_DPTX_ISENABLED_AUX_IRQ(tx_idx)  ((IORD(btc_dptx_baseaddr(tx_idx),DPTX_REG_TX_CONTROL) >> 30) & 0x01)

#define BTC_STRM_VCP_ID(strm_idx)   (strm_idx+1) // Stream index to its VCP ID

#ifndef BTC_RAD_DEFINE
typedef struct  // MST Relative Address
{
  BYTE length; // Address (addr[]) length
  BYTE addr[15];
}BTC_RAD;
#define BTC_RAD_DEFINE
#endif

typedef struct
{
  BYTE data[16];
}BTC_MST_CURR_CAPAB_STRUCT;

typedef struct
{
  BYTE input_port : 1;
  BYTE peer_device_type : 3;
  BYTE port_number : 4;
  BYTE messaging_capability_status : 1;
  BYTE displayport_device_plug_status : 1;
  BYTE legacy_device_plug_status : 1;
  BYTE curr_capab_struct_idx : 3;
  BYTE padding : 2;
  BYTE DPCD_revision;
  BYTE peer_GUID[16];
  BYTE number_SDP_streams : 4;
  BYTE number_SDP_stream_sinks : 4;
  BTC_MST_CURR_CAPAB_STRUCT current_capabilities_structure[7];
}BTC_MST_DEVICE_PORT;

typedef struct
{
  BYTE GUID[16];
  BYTE number_of_ports;
  BTC_MST_DEVICE_PORT port[15];
}BTC_MST_DEVICE;

typedef struct
{
  BYTE port_number;
  unsigned int full_PBN;
  unsigned int available_PBN;
}BTC_MST_PATH_PBN;

typedef struct
{
  BYTE write_i2c_device_identifier;
  BYTE num_of_bytes_to_write;
  BYTE i2c_data_to_write[16];
  BYTE no_stop_bit;
  BYTE i2c_transaction_delay;
}BTC_MST_I2C_WR_TRANS;

typedef struct
{
  BYTE port_number;
  BYTE num_of_bytes_read;
  BYTE i2c_read_data[255];
}BTC_MST_I2C_RD_DATA;

typedef struct
{
  BYTE input_port : 1;
  BYTE peer_device_type : 3;
  BYTE port_number : 4;
  BYTE messaging_capability_status : 1;
  BYTE displayport_device_plug_status : 1;
  BYTE legacy_device_plug_status : 1;
  BYTE padding : 5;
  BYTE GUID[16];
}BTC_MST_CONN_STAT_NOTIFY;

// HDCP Encoder state
typedef enum
{
  BTC_HDCPTX_STATE_NON_AUTH = 0,                            // Non authenticated
  BTC_HDCPTX_STATE_HDCP1_CAPAB,                             // HDCP1 capability check
  BTC_HDCPTX_STATE_HDCP1_AN,                                // HDCP1 send An
  BTC_HDCPTX_STATE_HDCP1_EXCG_KSV,                          // HDCP1 exchange KSVs
  BTC_HDCPTX_STATE_HDCP1_CHECK_R0,                          // HDCP1 check R0
  BTC_HDCPTX_STATE_HDCP1_WAIT_READY,                        // HDCP1 wait for READY
  BTC_HDCPTX_STATE_HDCP1_RD_KSV_LIST,                       // HDCP1 read KSV list
  BTC_HDCPTX_STATE_HDCP1_AUTH,                              // HDCP1 authenticated
  BTC_HDCPTX_STATE_HDCP2_AKE_INIT,                          // HDCP2 AKE_Init
  BTC_HDCPTX_STATE_HDCP2_AKE_SEND_CERT,                     // HDCP2 AKE_Send_Cert
  BTC_HDCPTX_STATE_HDCP2_AKE_NO_STORED_KM,                  // HDCP2 AKE_No_Stored_km
  BTC_HDCPTX_STATE_HDCP2_AKE_STORED_KM,                     // HDCP2 AKE_Stored_km
  BTC_HDCPTX_STATE_HDCP2_AKE_SEND_H_PRIME,                  // HDCP2 AKE_Send_H_prime
  BTC_HDCPTX_STATE_HDCP2_AKE_SEND_PAIRING_INFO,             // HDCP2 AKE_Send_Pairing_Info
  BTC_HDCPTX_STATE_HDCP2_LC_INIT,                           // HDCP2 LC_Init
  BTC_HDCPTX_STATE_HDCP2_LC_SEND_L_PRIME,                   // HDCP2 LC_Send_L_prime
  BTC_HDCPTX_STATE_HDCP2_SKE_SEND_EKS,                      // HDCP2 SKE_Send_Eks
  BTC_HDCPTX_STATE_HDCP2_REPEATERAUTH_SEND_RECEIVERID_LIST, // HDCP2 RepeaterAuth_Send_ReceiverID_List
  BTC_HDCPTX_STATE_HDCP2_REPEATERAUTH_SEND_ACK,             // HDCP2 RepeaterAuth_Send_Ack
  BTC_HDCPTX_STATE_HDCP2_REPEATERAUTH_STREAM_READY,         // HDCP2 RepeaterAuth_Stream_Ready
  BTC_HDCPTX_STATE_HDCP2_REPEATERAUTH_STREAM_MANAGE,        // HDCP2 RepeaterAuth_Stream_Manage
  BTC_HDCPTX_STATE_HDCP2_AUTH                               // HDCP2 authenticated
}BTC_HDCPTX_STATE;

// HDCP Encoder status
// rxstatus:
//  bit4 = LINK_INTEGRITY_FAILURE
//  bit3 = REAUTH_REQ
//  bit2 = PAIRING_AVAILABLE
//  bit1 = H'_AVAILABLE
//  bit0 = READY
typedef struct
{
  BTC_HDCPTX_STATE state;
  BYTE rxstatus;              // Bstatus / RxStatus flags
  unsigned int rxcaps;        // Bcaps / RxCaps
  unsigned int rxinfo;        // Binfo / RxInfo
  BYTE receiver_id[5];        // Receiver_ID of the last connected Sink
  BYTE m_mprime_mismatch : 1; // 1 = M to M' comparison mismatch
}BTC_HDCPTX_STAT;

//********** btc_dptx_common.c *********//
int btc_dptx_syslib_add_tx(BYTE tx_idx,
                           unsigned int tx_base_addr,
                           unsigned int tx_irq_id,
                           unsigned int tx_irq_num);
int btc_dptx_syslib_init(void);
int btc_dptx_syslib_monitor(void);
unsigned int btc_dptx_baseaddr(BYTE tx_idx);
void btc_dptx_sw_ver(BYTE *major,BYTE *minor, unsigned int *rev);
void btc_dptx_rtl_ver(BYTE *major,BYTE *minor, unsigned int *rev);

//********** btc_dptx_aux_ch.c *********//
int btc_dptx_aux_write(BYTE tx_idx,unsigned int address,BYTE size,BYTE *data);
int btc_dptx_aux_read(BYTE tx_idx,unsigned int address,BYTE size,BYTE *data);
int btc_dptx_aux_i2c_write(BYTE tx_idx,BYTE address,BYTE size,BYTE *data,BYTE mot);
int btc_dptx_aux_i2c_read(BYTE tx_idx,BYTE address,BYTE size,BYTE *data,BYTE mot);
void btc_dptx_aux_set_read_reply_timeout(BYTE tx_idx,unsigned int new_aux_read_reply_timeout);

//********** btc_dptx_utils.c *********//
int btc_dptx_set_color_space(BYTE tx_idx,BYTE format,BYTE bpc,BYTE range,BYTE colorimetry,BYTE use_vsc_sdp);
int btc_dptx_video_enable(BYTE tx_idx,BYTE enabled);
int btc_dptx_edid_read(BYTE tx_idx,BYTE *data);
int btc_dptx_edid_block_read(BYTE tx_idx,BYTE block,BYTE *data);

//********** btc_dptx_lt.c *********//
void btc_dptx_link_discovery(BYTE tx_idx);
int btc_dptx_link_training(BYTE tx_idx,unsigned int link_rate, unsigned int lane_count);
int btc_dptx_link_training_fec(BYTE tx_idx,unsigned int link_rate, unsigned int lane_count, unsigned int fec);
int btc_dptx_fast_link_training(BYTE tx_idx,
                                unsigned int link_rate, unsigned int lane_count,
                                unsigned int volt_swing, unsigned int pre_emph,
                                unsigned int new_cfg);
void btc_dptx_hpd_change(BYTE tx_idx,unsigned int asserted);
int btc_dptx_is_link_up(BYTE tx_idx);
unsigned int btc_dptx_link_bw(BYTE tx_idx);

//********** btc_dptx_ta.c *********//
int btc_dptx_test_autom(BYTE tx_idx);

//********** btc_dptx_mst.c *********//
int btc_dptx_mst_down_rep_irq(BYTE tx_idx);
int btc_dptx_mst_up_req_irq(BYTE tx_idx);
int btc_dptx_mst_link_address_req(BYTE tx_idx,BTC_RAD *RAD);
int btc_dptx_mst_enum_path_req(BYTE tx_idx,BTC_RAD *RAD,BYTE port_number);
int btc_dptx_mst_remote_i2c_rd_req(BYTE tx_idx,
                                   BTC_RAD *RAD,
                                   BYTE port_number,
                                   BYTE num_of_wr_trans,
                                   BTC_MST_I2C_WR_TRANS *wr_trans,
                                   BYTE rd_i2c_dev_id,
                                   BYTE num_of_rd_bytes);
int btc_dptx_mst_remote_i2c_wr_req(BYTE tx_idx,
                                   BTC_RAD *RAD,
                                   BYTE port_number,
                                   BTC_MST_I2C_WR_TRANS *wr_trans);
int btc_dptx_mst_clear_payload_table_req(BYTE tx_idx,BTC_RAD *RAD);
int btc_dptx_mst_get_msg_transact_ver_req(BYTE tx_idx,BTC_RAD *RAD,BYTE port_number);
int btc_dptx_mst_remote_dpcd_wr_req(BYTE tx_idx,BTC_RAD *RAD,BYTE port_number,unsigned int addr,BYTE length,BYTE *data);
int btc_dptx_mst_link_address_rep(BYTE tx_idx,BTC_MST_DEVICE *device,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_enum_path_rep(BYTE tx_idx,BTC_MST_PATH_PBN *path_pbn,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_remote_i2c_rd_rep(BYTE tx_idx,BTC_MST_I2C_RD_DATA *data,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_remote_i2c_wr_rep(BYTE tx_idx,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_clear_payload_table_rep(BYTE tx_idx,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_remote_dpcd_wr_rep(BYTE tx_idx,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_allocate_payload_req(BYTE tx_idx,BTC_RAD *RAD,BYTE port_number,BYTE num_sdp_streams,BYTE *sdp_stream_sinks,BYTE vcp_id,unsigned int pbn);
int btc_dptx_mst_allocate_payload_rep(BYTE tx_idx,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
int btc_dptx_mst_get_msg_transact_ver_rep(BYTE tx_idx,BYTE *version,BYTE *GUID,BYTE *reas_for_nak,BYTE *nak_data);
BTC_MST_CONN_STAT_NOTIFY *btc_dptx_mst_conn_stat_notify_req(BYTE tx_idx);
int btc_dptx_mst_vcpid_set(BYTE tx_idx,BYTE strm_idx,BYTE vcpid);
int btc_dptx_mst_vcptab_addvc(BYTE tx_idx,BYTE vc_size, BYTE vc_id, unsigned int *chan_coding);
int btc_dptx_mst_vcptab_delvc(BYTE tx_idx,BYTE vc_id);
void btc_dptx_mst_vcptab_clear(BYTE tx_idx);
void btc_dptx_mst_vcptab_update(BYTE tx_idx);
int btc_dptx_mst_hdcp_encryption_enable(BYTE tx_idx,BYTE strm_idx,BYTE enabled);
void btc_dptx_mst_enable(BYTE tx_idx,BYTE enabled);
int btc_dptx_mst_tavgts_set(BYTE tx_idx,BYTE strm_idx, BYTE value);
int btc_dptx_mst_set_color_space(BYTE tx_idx,BYTE strm_idx,BYTE format,BYTE bpc,BYTE range,BYTE colorimetry,BYTE use_vsc_sdp);
int btc_dptx_mst_set_stream_en(BYTE tx_idx, BYTE strm_idx);

#endif /* _BTC_DPTX_SYSLIB_H_ */

