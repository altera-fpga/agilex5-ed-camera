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
// DisplayPort Source System Library global (library private) definitions
//
// Description:
// These definitions are global to all library routines but private to this
// library.
// ********************************************************************************

#include "btc_cpu.h"
#include "btc_dptx_syslib.h"

#define BTC_TX_DISABLE_DP13  0  // Set to 1 to disable TX DP 1.3 capabilities

#define AUX_NATIVE_WR     0x80
#define AUX_NATIVE_RD     0x90
#define AUX_I2C_WR        0x00
#define AUX_I2C_RD        0x10
#define AUX_I2C_UPDATE    0x20
#define AUX_I2C_MOT       0x40

#define AUX_ACK           0x00
#define AUX_NACK          0x10
#define AUX_DEFER         0x20
#define AUX_I2C_ACK       0x00
#define AUX_I2C_NACK      0x40
#define AUX_I2C_DEFER     0x80

// TX instance type
typedef struct
{
  unsigned int base_addr;                 // TX base address
  unsigned int irq_id;                    // TX IRQ ID
  unsigned int irq_num;                   // TX IRQ number
  unsigned int aux_read_reply_timeout;    // read reply timeout
  BYTE num_lttprs;
  BYTE hpd : 1;                           // HPD status
} BTX_TXINSTANCE_TYPE;

// Sideband UP_REQ msg type
typedef struct
{
  BTC_RAD RAD;      // message RAD
  BYTE link_count_total : 4;
  BYTE msg_seq_no : 1;
  BYTE broadcast_msg : 1;
  BYTE path_msg : 1;
  BYTE valid : 1;   // 1 = msg complete and valid
  BYTE msg[256];
  BYTE ptr;         // index of next msg byte free to fill (= total message length)
} BTX_SBMSG_UPREQ_TYPE;

// Configuration data
#define BTC_MAX_NUM_OF_SOURCES         4 // Max number of Source cores supported

//********** btc_dptx_aux_ch.c *********//
int btc_dptx_aux_init(void);

//********** btc_dptx_mst.c *********//
void btc_dptx_mst_init(BYTE tx_idx);
void btc_dptx_mst_down_rep_decode(BYTE tx_idx,BYTE *msg,BYTE length);
void btc_dptx_mst_up_req_decode(BYTE tx_idx,BTX_SBMSG_UPREQ_TYPE *req);

//********** btc_dptx_sbmsg.c *********//
void btc_dptx_sbmsg_init(BYTE tx_idx);
void btc_dptx_sbmsg_monitor(void);
int btc_dptx_sbmsg_tx_request(BYTE tx_idx,BTC_RAD *RAD,BYTE *msg,BYTE length,BYTE broadcast,BYTE path);
int btc_dptx_sbmsg_tx_reply(BYTE tx_idx,BTC_RAD *RAD,BYTE *msg,BYTE length,BYTE broadcast,BYTE path);
void btc_dptx_sbmsg_down_rep_irq(BYTE tx_idx);
void btc_dptx_sbmsg_up_req_irq(BYTE tx_idx);
int btc_dptx_sbmsg_tx_req_busy(BYTE tx_idx);
int btc_dptx_sbmsg_tx_rep_busy(BYTE tx_idx);

//********** btc_dptx_common.c *********//
extern BTX_TXINSTANCE_TYPE _btx_inst[BTC_MAX_NUM_OF_SOURCES];

// TX instance I/O
#define BTX_IORD(idx,offset)      IORD(_btx_inst[idx].base_addr,offset)
#define BTX_IOWR(idx,offset,data) IOWR(_btx_inst[idx].base_addr,offset,data)

// HW Timers
#define BTC_TX_WAIT(tx_idx,delay_us) {  unsigned to = (delay_us/100)*BTC_100US_TXTICKS; \
                                        unsigned tstart = BTX_IORD(tx_idx,DPTX_REG_TIMESTAMP); \
                                        while(((BTX_IORD(tx_idx,DPTX_REG_TIMESTAMP) - tstart) & 0xFFFFFF) < to);  }

#define BTC_TX_TOUT(tx_idx,start,tout_us) (((BTX_IORD(tx_idx,DPTX_REG_TIMESTAMP) - start) & 0xFFFFFF) >= \
                                           ((tout_us/100)*BTC_100US_TXTICKS))


