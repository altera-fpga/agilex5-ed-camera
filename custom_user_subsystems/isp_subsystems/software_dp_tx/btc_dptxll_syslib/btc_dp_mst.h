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
// DisplayPort Sink/Source System Library MST related definitions
//
// Description:
//
// ********************************************************************************

#ifndef _BTC_DP_MST_INCLUDE
#define _BTC_DP_MST_INCLUDE

// Request names and identifiers
#define BTC_SBRQ_GET_MESSAGE_TRANSACTION_VERSION  0x00
#define BTC_SBRQ_LINK_ADDRESS                     0x01
#define BTC_SBRQ_CONNECTION_STATUS_NOTIFY         0x02
#define BTC_SBRQ_ENUM_PATH_RESOURCES              0x10
#define BTC_SBRQ_ALLOCATE_PAYLOAD                 0x11
#define BTC_SBRQ_QUERY_PAYLOAD                    0x12
#define BTC_SBRQ_RESOURCE_STATUS_NOTIFY           0x13
#define BTC_SBRQ_CLEAR_PAYLOAD_ID_TABLE           0x14
#define BTC_SBRQ_REMOTE_DPCD_READ                 0x20
#define BTC_SBRQ_REMOTE_DPCD_WRITE                0x21
#define BTC_SBRQ_REMOTE_I2C_READ                  0x22
#define BTC_SBRQ_REMOTE_I2C_WRITE                 0x23
#define BTC_SBRQ_POWER_UP_PHY                     0x24
#define BTC_SBRQ_POWER_DOWN_PHY                   0x25
#define BTC_SBRQ_SINK_EVENT_NOTIFY                0x30
#define BTC_SBRQ_QUERY_STREAM_ENC_STATUS          0x38

// Peer device types
#define BTC_PEER_DEV_NONE               0x00
#define BTC_PEER_DEV_SOURCE             0x01
#define BTC_PEER_DEV_BRANCH             0x02
#define BTC_PEER_DEV_SST_SINK           0x03
#define BTC_PEER_DEV_DP_TO_LEGACY       0x04
#define BTC_PEER_DEV_DP_TO_WIRELESS     0x05
#define BTC_PEER_DEV_WIRELESS_TO_DP     0x06

// Reasons for NAK
#define BTC_RFNAK_WRITE_FAILURE               0x01
#define BTC_RFNAK_INVALID_RAD                 0x02
#define BTC_RFNAK_CRC_FAILURE                 0x03
#define BTC_RFNAK_BAD_PARAM                   0x04
#define BTC_RFNAK_DEFER                       0x05
#define BTC_RFNAK_LINK_FAILURE                0x06
#define BTC_RFNAK_NO_RESOURCES                0x07
#define BTC_RFNAK_DPCD_FAIL                   0x08
#define BTC_RFNAK_I2C_NAK                     0x09
#define BTC_RFNAK_ALLOCATE_FAIL               0x0A

#endif // #ifndef _BTC_DP_MST_INCLUDE
