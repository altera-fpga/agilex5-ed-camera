/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "btc_dptx_syslib.h"
#include "btc_dptxll_syslib.h"
#include "config.h"
#include "tx_utils.h"
#include "sys/alt_irq.h"
#include "sys/alt_timestamp.h"
#include "sys/msw_interrupt.h"
#if DP_SUPPORT_TX_HDCP
#include "hdcp.h"
#endif

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DGB_PRINTF printf
#else
#define DGB_PRINTF(format, args...) ((void)0)
#endif

#if BITEC_TX_CAPAB_MST

// MST peer device types
#define BTC_PEER_DEV_NONE 0x00
#define BTC_PEER_DEV_SOURCE 0x01
#define BTC_PEER_DEV_BRANCH 0x02
#define BTC_PEER_DEV_SST_SINK 0x03
#define BTC_PEER_DEV_DP_TO_LEGACY 0x04
#define BTC_PEER_DEV_DP_TO_WIRELESS 0x05
#define BTC_PEER_DEV_WIRELESS_TO_DP 0x06

// MST status variables
BTC_PC_STATE pc_fsm = PC_FSM_IDLE;       // PC fsm state
BTC_MST_DEVPORT* dev_ports;              // Device ports of connected sink
BTC_MST_DEVICE_PORT* aPort;
int port0_idx, port1_idx, port2_idx, port3_idx;
BYTE num_of_ports;
BYTE* edid;
BYTE tavg_ts;
int pc_fsm_force_hpd0, pc_fsm_force_hpd1;

#if DEBUG_PRINT_ENABLED
BTC_PC_STATE pc_fsm_prev = PC_FSM_IDLE;  // Previous PC fsm state for debug
#endif

#endif

// If new Sink detected
int new_rx = 0;

// Dashboard bitfields
#define DASH_BPP18_MASK 0x0
#define DASH_BPP24_MASK 0x1
#define DASH_BPP30_MASK 0x2
#define DASH_BPP36_MASK 0x3
#define DASH_BPP48_MASK 0x4

BYTE tx_edid_data[128 * 4];  // TX copy of Sink EDID
unsigned int T100uS = 0;

void bitec_dptx_hpd_isr(void* context);
void bitec_dptx_hpd_irq();
int btc_dptxll_hpd_irq_hdcp(BYTE tx_idx);
void bitec_csn_callback(BTC_MST_CONN_STAT_NOTIFY* csn_data);
unsigned int bitec_dptx_test_autom();

// Get the core capabilities (defined in QSYS and ported to system.h)
#if DP_SUPPORT_HDCP_KEY_MANAGE
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_LANE_COUNT
#else
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LANE_COUNT
#endif

//******************************************************
// Initialize the TX
//******************************************************
void bitec_dptx_init()
{
    unsigned int tx_link_rate = TX_MAX_LINK_RATE;
    unsigned int tx_lane_count = TX_MAX_LANE_COUNT;

    // to avoid unused warning
    (void)tx_link_rate;
    (void)tx_lane_count;

#if BITEC_TX_CAPAB_MST
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
    btc_dptxll_syslib_add_tx(0, tx_link_rate, tx_lane_count,
                             DP_TX_DP_SOURCE_TX_MGMT_BITEC_CFG_TX_MAX_NUM_OF_STREAMS, tx_edid_data);
#else
    btc_dptxll_syslib_add_tx(0, tx_link_rate, tx_lane_count,
                             DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_NUM_OF_STREAMS, tx_edid_data);
#endif
    btc_dptxll_syslib_init();
    btc_dptxll_mst_set_csn_callback(0, bitec_csn_callback);

    for (unsigned int k = 0; k < MST_TX_STREAMS; ++k)
    {
        btc_dptxll_stream_set_color_space(0, k, 0, 1, 0, 0, 0);  // Set Stream k video color space
    }

    pc_fsm_force_hpd0 = 0;
    pc_fsm_force_hpd1 = 0;
#endif

    // Register the interrupt handler
#if (DP_SUPPORT_HDCP_KEY_MANAGE)
    alt_ic_isr_register(DP_TX_DP_SOURCE_TX_MGMT_IRQ_INTERRUPT_CONTROLLER_ID,
                        DP_TX_DP_SOURCE_TX_MGMT_IRQ, bitec_dptx_hpd_isr, NULL, 0x0);
#else
    alt_ic_isr_register(DP_TX_DP_SOURCE_IRQ_INTERRUPT_CONTROLLER_ID, DP_TX_DP_SOURCE_IRQ,
                        bitec_dptx_hpd_isr, NULL, 0x0);
#endif
}

//******************************************************
// Perform Link Training
//******************************************************
void bitec_dptx_linktrain_parameterized(unsigned int link_rate, unsigned int lane_count, unsigned int bpc)
{
    btc_dptx_edid_read(0, tx_edid_data);               // Read the sink EDID
    btc_dptx_set_color_space(0, 0, bpc, 0, 0, 0);      // Set TX video color space
 #if (DP_SUPPORT_TX_FEC)
    btc_dptx_link_training_fec(0, link_rate, lane_count,1);  // Enable FEC then do link training 
#else
    btc_dptx_link_training(0, link_rate, lane_count);        // Do link training without FEC
#endif
}

void bitec_dptx_linktrain()
{
    // Link train with default values
    bitec_dptx_linktrain_parameterized(TX_MAX_LINK_RATE, TX_MAX_LANE_COUNT, DP_TX_BPS);
}


//******************************************************
// HPD activity service routine
//******************************************************
void bitec_dptx_hpd_isr(void* context)
{
    unsigned int status_reg;
    unsigned int link_rate;
    unsigned int lane_count;
    unsigned int bpc;

    // Disable TX Core HPD interrupts
    BTC_DPTX_DISABLE_HPD_IRQ(0);

    // Allows for nested interrupts using the Enhanced Interrupt API
    // but without requiring the External Interrupt Controller (EIC)
    // and the Vectored Interrupt Controller (VIC)
    alt_niosv_enable_msw_interrupt();

    link_rate = TX_MAX_LINK_RATE;
    lane_count = TX_MAX_LANE_COUNT;
    bpc = DP_TX_BPS;

    status_reg = IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS);
    IOWR(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS, 0);  // Writing DPTX_REG_TX_STATUS clears IRQ
    if (status_reg & 0x1)
    {
        // Long HPD
        if (status_reg & 0x4)
        {
            DGB_PRINTF("bitec_dptx_hpd_isr(): long HPD %x\n", status_reg);
            // HPD is at '1'

#if DP_SUPPORT_TX_HDCP
            hdcp_unauth();
#endif
#if BITEC_TX_CAPAB_MST
            pc_fsm_force_hpd1 = 1;
#else
            // Check Automated test request
            btc_dptx_hpd_change(0, 1);
            bitec_dptx_test_autom();              // Only TEST_EDID_READ uses HPD rising edge
            btc_dptx_edid_read(0, tx_edid_data);  // Read the sink EDID

            // If new Sink is detected.
            new_rx = 1;

            btc_dptx_set_color_space(0, 0, bpc, 0, 0, 0);      // Set TX video color space
#if (DP_SUPPORT_TX_FEC)
           usleep(50000); 
           btc_dptx_link_training_fec(0, link_rate, lane_count,1);  // Enable FEC then do link training 
#else
            btc_dptx_link_training(0, link_rate, lane_count);        // Do link training without FEC
#endif
#endif
        }
        else
        {
            // HPD is at '0'
            DGB_PRINTF("bitec_dptx_hpd_isr(): HPD 0 %x\n", status_reg);
#if BITEC_TX_CAPAB_MST
            pc_fsm_force_hpd0 = 1;
#else
            // Send the idle pattern
            btc_dptx_hpd_change(0, 0);
            btc_dptx_video_enable(0, 0);
#endif
        }
    }
    else
    {
        // HPD short pulse (IRQ)
        DGB_PRINTF("bitec_dptx_hpd_isr(): HPD pulse %x\n", status_reg);
#if BITEC_TX_CAPAB_MST
#if DP_SUPPORT_TX_HDCP
        btc_dptxll_hpd_irq_hdcp(0);
#else
        btc_dptxll_hpd_irq(0);
#endif
#else
        bitec_dptx_hpd_irq();
#endif
    }

    // Prevent nested interrupts
    alt_niosv_disable_msw_interrupt();

    // Enable TX Core HPD interrupts
    BTC_DPTX_ENABLE_HPD_IRQ(0);
}

//******************************************************
// HPD IRQ (short pulse) handler
//******************************************************
void bitec_dptx_hpd_irq()
{
    BYTE data[8];
    unsigned int status_ok, lane_count, link_rate;
    BYTE edid_data[128 * 4];

    // Check for Test Automation requests
    if (bitec_dptx_test_autom())
        return;

    btc_dptx_aux_read(0, DPCD_ADDR_LINK_BW_SET, 2, data);
    link_rate = data[0] & 0xFF;
    lane_count = data[1] & 0x1F;

    btc_dptx_aux_read(0, DPCD_ADDR_SINK_COUNT, 6, data);  // Read link status

    // Check CP_IRQ
    if (data[1] & 0x04)
    {
        // HDCP CP_IRQ
#if DP_SUPPORT_TX_HDCP
        hdcp_cp_irq(0);
#else
        BYTE data = 0x04;
        btc_dptx_aux_write(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, &data);  // Reset CP_IRQ
#endif
        return;
    }

    // Check Downstream port status change
    if (data[4] & (1 << 6))
        btc_dptx_edid_read(0, edid_data);  // Read the sink EDID

    // Check link status
    status_ok = data[4] & 0x01;  // Get inter-lane align
    switch (lane_count)
    {
    case 1:
        status_ok &= ((data[2] & 0x07) == 0x07);
        break;
    case 2:
        status_ok &= ((data[2] & 0x77) == 0x77);
        break;
    case 4:
        status_ok &= ((data[2] & 0x77) == 0x77) & ((data[3] & 0x77) == 0x77);
        break;
    default:
        break;
    }
    if (!status_ok)
    {
#if DP_SUPPORT_TX_HDCP
        hdcp_cp_irq(0);
#endif
#if (DP_SUPPORT_TX_FEC)
        btc_dptx_link_training_fec(0, link_rate, lane_count,1);  // Enable FEC then do link training 
#else
        btc_dptx_link_training(0, link_rate, lane_count);        // Do link training without FEC
#endif
    }
}

//******************************************************
// Test Automation handler
//
// Return: 1 = Test Automation performed, 0 = no action
//******************************************************
unsigned int bitec_dptx_test_autom()
{
    BYTE data[2];

    // Check Automated test request
    btc_dptx_aux_read(0, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, data);
    btc_dptx_aux_read(0, DPCD_ADDR_TEST_REQUEST, 1, data + 1);

    if ((data[0] & 0x02) && (data[1] != 0x00))
    {
        btc_dptx_test_autom(0);
        return 1;
    }

    return 0;
}

#if BITEC_TX_CAPAB_MST
//******************************************************
// Simulates a PC using the TX MST link
// Pick up one of the connected sink
// devices and try to output video to it
//
// Must be invoked periodically
//******************************************************
void bitec_dptx_pc()
{
    unsigned int i;
    BYTE vcp_size;
    BYTE chan_coding;
    unsigned int msa_reg;
    BYTE msa_bpc;
    BYTE msa_color;
    BYTE msa_color_range;
    BYTE msa_color_colorimetry;
    BYTE msa_color_sdp;
    unsigned long long pixel_rate;

    int return_val = 0;
    BYTE link_rate;

    // Debug code
#if DEBUG_PRINT_ENABLED
    const char* pc_fsm_states[] = {"PC_FSM_IDLE",
                                   "PC_FSM_HPD_0",
                                   "PC_FSM_HPD_1",
                                   "PC_FSM_START",
                                   "PC_FSM_GET_PORTS",
                                   "PC_FSM_FIND_STREAM_1",
                                   "PC_FSM_FIND_STREAM_2",
                                   "PC_FSM_FIND_STREAM_3",
                                   "PC_FSM_RDEDID_0",
                                   "PC_FSM_RDEDID_1",
                                   "PC_FSM_RDEDID_2",
                                   "PC_FSM_RDEDID_3",
                                   "PC_FSM_ALLOCATE_STREAM_0",
                                   "PC_FSM_WAIT_ALLOCATED_0",
                                   "PC_FSM_ALLOCATE_STREAM_1",
                                   "PC_FSM_WAIT_ALLOCATED_1",
                                   "PC_FSM_ALLOCATE_STREAM_2",
                                   "PC_FSM_WAIT_ALLOCATED_2",
                                   "PC_FSM_ALLOCATE_STREAM_3",
                                   "PC_FSM_WAIT_ALLOCATED_3",
                                   "PC_FSM_MST_DATA",
                                   "PC_FSM_MST_ON",
                                   "PC_FSM_NOOUT"};

    if (pc_fsm != pc_fsm_prev)
    {
        DGB_PRINTF("pc_fsm(): %s \n", pc_fsm_states[pc_fsm]);
    }
    pc_fsm_prev = pc_fsm;
#endif

    switch (pc_fsm)
    {
    case PC_FSM_IDLE:  // No sink detected
        break;

    case PC_FSM_HPD_0:  // HPD set to 0
        btc_dptxll_hpd_change(0, 0);
        pc_fsm = PC_FSM_IDLE;
        break;

    case PC_FSM_HPD_1:  // HPD set to 1
        btc_dptxll_hpd_change(0, 1);
        for (i = 0; i < MST_TX_STREAMS; ++i)
        {
            btc_dptxll_stream_set_color_space(0, i, 0, 1, 0, 0, 0);  // Set Stream i video color space
        }
        pc_fsm = PC_FSM_START;
        break;

    case PC_FSM_START:  // A new sink got connected
        BYTE data;
        port0_idx = 255;
        port1_idx = 255;
        port2_idx = 255;
        port3_idx = 255;
        btc_dptx_aux_read(0, DPCD_ADDR_TRAINING_AUX_RD_INTERVAL, 1, &data);
        if (data & 0x80)
            btc_dptx_aux_read(0, DPCD_ADDR_EXTENDED_CAPAB_FIELD, 1,
                              &data);  // Use DP 1.3 capabilities
        else
            btc_dptx_aux_read(0, DPCD_ADDR_DPCD_REV, 1, &data);
        if (data >= 0x12)
        {
            // DPCD 1.2 or higher, go through the vcp allocation if MST framing is on at 128b/132b rates OR both the sink and this source support MST at 8b/10b rates)
            // BITEC_TX_CAPAB_MST doesn't guarantee the Tx can do MST framing at 8b10b rates, (MST_TX_STREAMS > 1) does.
            btc_dptx_aux_read(0, DPCD_ADDR_MST_CAP, 1, &data);
            chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
            if (((data & 1) && (MST_TX_STREAMS > 1)) || (chan_coding & 0x2))
                pc_fsm = PC_FSM_GET_PORTS;  // MST_CAP = 1
            else
                pc_fsm = PC_FSM_NOOUT;  // MST_CAP = 0: use SST output
        }
        else
            pc_fsm = PC_FSM_NOOUT;  // MST_CAP = 0: use SST output
        break;

    case PC_FSM_GET_PORTS:  // Determine DP device attached
        i = btc_dptxll_mst_get_device_ports(0, &dev_ports, &num_of_ports);
        if (i == 1) 
        {
            DGB_PRINTF("btc_dptxll_mst_get_device_ports returned FAIL\n");
            pc_fsm = PC_FSM_NOOUT;
        }
        else if (i == 0)
        {
           if (num_of_ports == 0)
           {
               // Bypass, no valid reply from sink
               num_of_ports = 1;
               dev_ports[0].port.displayport_device_plug_status = 1;
               dev_ports[0].port.peer_device_type = BTC_PEER_DEV_SST_SINK;
               pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
           }
           else
           {
                DGB_PRINTF("btc_dptxll_mst_get_device_ports returned SUCCESS. Number of ports: %x \n",
                           num_of_ports);
                // Topology discovery ready: search for a Stream 0 sink device
                // This iterates through ports that we aren't interested in
                for (i = 0; i < num_of_ports; i++)
                {
                    aPort = &dev_ports[i].port;
                    DGB_PRINTF("Input Port: %x \n", aPort->input_port);
                    DGB_PRINTF("Port Connected: %x \n", aPort->displayport_device_plug_status);
                    DGB_PRINTF("Peer Device Type: %x \n", aPort->peer_device_type);
                    DGB_PRINTF("Messaging Capability: %x \n", aPort->messaging_capability_status);

                    if (aPort->input_port == 0 &&                      // != Input Ports
                        aPort->displayport_device_plug_status == 1 &&  // Ports connected
                        (aPort->peer_device_type == BTC_PEER_DEV_SST_SINK &&
                         aPort->messaging_capability_status == 0))
                        break;  // Break out of loop only once all the above is cleared
                }

                // If the for loop above still resulted in i < num_of_ports
                if (i < num_of_ports)
                {
                    DGB_PRINTF("i (%d) still less than num_of_ports (%d) \n", i, num_of_ports);
                    // A suitable device port was found
                    if (dev_ports[i].available_PBN > 0)
                    {
                        DGB_PRINTF("available_PBN is : %d \n", dev_ports[i].available_PBN);
                        port0_idx = i;
                        btc_dptxll_mst_edid_read_req(0, &dev_ports[port0_idx].RAD,
                                                     dev_ports[port0_idx].port.port_number);
                    }
                    else
                    {
                        port0_idx = 255;
                    }
                }

                if (port0_idx != 255)
                {
                    pc_fsm = PC_FSM_RDEDID_0;
                }
                else
                {
                    // Don't bail to FSM_NOOUT, just get one stream going
                    // pc_fsm = PC_FSM_NOOUT;
                    pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
                }
            }
        }
        break;
    case PC_FSM_FIND_STREAM_1:  // Find a port for Stream 1
        for (i = port0_idx + 1; i < num_of_ports; i++)
        {
            aPort = &dev_ports[i].port;
            if (aPort->input_port == 0 && aPort->displayport_device_plug_status == 1 &&
                (aPort->peer_device_type == BTC_PEER_DEV_SST_SINK &&
                 aPort->messaging_capability_status == 0))
                break;
        }

        if (i < num_of_ports)
        {
            // A suitable device port was found
            if (dev_ports[i].available_PBN > 0)
            {
                port1_idx = i;
                btc_dptxll_mst_edid_read_req(0, &dev_ports[port1_idx].RAD,
                                             dev_ports[port1_idx].port.port_number);
            }
            else
            {
                port1_idx = 255;
            }
        }

        if (port1_idx != 255)
        {
            pc_fsm = PC_FSM_RDEDID_1;
        }
        else
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
        }
        break;

    case PC_FSM_FIND_STREAM_2:  // Find a port for Stream 2
        for (i = port0_idx + 2; i < num_of_ports; i++)
        {
            aPort = &dev_ports[i].port;
            if (aPort->input_port == 0 && aPort->displayport_device_plug_status == 1 &&
                (aPort->peer_device_type == BTC_PEER_DEV_SST_SINK &&
                 aPort->messaging_capability_status == 0))
                break;
        }
        if (i < num_of_ports)
        {
            if (dev_ports[i].available_PBN > 0)
            {
                port2_idx = i;
                btc_dptxll_mst_edid_read_req(0, &dev_ports[port2_idx].RAD,
                                             dev_ports[port2_idx].port.port_number);
            }
            else
            {
                port2_idx = 255;
            }
        }
        if (port2_idx != 255)
        {
            pc_fsm = PC_FSM_RDEDID_2;
        }
        else
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
        }
        break;

    case PC_FSM_FIND_STREAM_3:  // Find a port for Stream 3
        for (i = port0_idx + 3; i < num_of_ports; i++)
        {
            aPort = &dev_ports[i].port;
            if (aPort->input_port == 0 && aPort->displayport_device_plug_status == 1 &&
                (aPort->peer_device_type == BTC_PEER_DEV_SST_SINK &&
                 aPort->messaging_capability_status == 0))
                break;
        }
        if (i < num_of_ports)
        {
            if (dev_ports[i].available_PBN > 0)
            {
                port3_idx = i;
                btc_dptxll_mst_edid_read_req(0, &dev_ports[port3_idx].RAD,
                                             dev_ports[port3_idx].port.port_number);
            }
            else
            {
                port3_idx = 255;
            }
        }
        if (port3_idx != 255)
        {
            pc_fsm = PC_FSM_RDEDID_3;
        }
        else
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
        }
        break;

    case PC_FSM_RDEDID_0:  // Wait for EDID Stream 0 read complete
        i = btc_dptxll_mst_edid_read_rep(0, &edid);
        if (i == 0)
        {
            if (MST_TX_STREAMS > 1)
            {
                pc_fsm = PC_FSM_FIND_STREAM_1;
            }
            else
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;

    case PC_FSM_RDEDID_1:  // Wait for EDID Stream 1 read complete
        i = btc_dptxll_mst_edid_read_rep(0, &edid);
        if (i == 0)
        {
            if (MST_TX_STREAMS > 2)
            {
                pc_fsm = PC_FSM_FIND_STREAM_2;
            }
            else
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;

    case PC_FSM_RDEDID_2:  // Wait for EDID Stream 2 read complete
        i = btc_dptxll_mst_edid_read_rep(0, &edid);
        if (i == 0)
        {
            if (MST_TX_STREAMS > 3)
            {
                pc_fsm = PC_FSM_FIND_STREAM_3;
            }
            else
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;

    case PC_FSM_RDEDID_3:  // Wait for EDID Stream 3 read complete
        i = btc_dptxll_mst_edid_read_rep(0, &edid);
        if (i == 0)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_0;
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_NOOUT;
        }
        break;

    case PC_FSM_ALLOCATE_STREAM_0:  // Allocate Stream 0 to port0_idx
        if (!btc_dptx_is_link_up(0))
        {
            pc_fsm = PC_FSM_NOOUT;
            break;
        }
        chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
#if DP_SUPPORT_TX && (DP_SUPPORT_RX == 0)
        // Set VFREQ for TX-only design in DP2.0 so the following MVID/NVID queries are valid
        if ((chan_coding & 0x2) == 0x2)
            dp2p0_dptxll_stream_set_vfreq(0, 0, 594000/MST_TX_STREAMS); // Enough for 1080p60, as used in Tx-only example design but not mote than a 10Gbps 1-lane link
#endif
        pixel_rate = 0ULL;
        if ((chan_coding & 0x2) == 0x2)
        {
            msa_reg = IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_COLOUR);
            msa_bpc = msa_reg & 0x7;
            msa_color = (msa_reg >> 4) & 0xf;
            msa_color_range = (msa_reg >> 12) & 0x01;
            msa_color_colorimetry = (msa_reg >> 8) & 0x0f;
            msa_color_sdp = (msa_reg >> 13) & 0x01;
            btc_dptxll_stream_set_color_space(0, 0, msa_color, msa_bpc, msa_color_range, msa_color_colorimetry, msa_color_sdp);
            
            pixel_rate = IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_NVID) & 0x00FFFFFF;
            pixel_rate = pixel_rate << 24;
            pixel_rate |= IORD(btc_dptx_baseaddr(0), DPTX0_REG_MSA_MVID) & 0x00FFFFFF;
            DGB_PRINTF("Vfreq  : %d Hz\n", pixel_rate);
            pixel_rate = pixel_rate / 1000;
        }

        if (pixel_rate == 0)
        {
            pixel_rate = 594000 / MST_TX_STREAMS;
        }
        btc_dptxll_stream_set_pixel_rate(0, 0, pixel_rate);

        vcp_size = btc_dptxll_stream_calc_VCP_size(0, 0);
        if (!vcp_size || vcp_size > 63)
        {
            DGB_PRINTF("Could not allocate Stream 0, invalid vcp_size=%d\n", vcp_size);
            pc_fsm = PC_FSM_NOOUT;
        }
        else
        {
            btc_dptxll_stream_allocate_req(0, 0, &dev_ports[port0_idx]);
            pc_fsm = PC_FSM_WAIT_ALLOCATED_0;
        }
        break;

    case PC_FSM_WAIT_ALLOCATED_0:  // Wait for Stream 0 allocation
        i = btc_dptxll_stream_allocate_rep(0);
        if (i == 0)
        {
            if ((MST_TX_STREAMS > 1) && (port1_idx != 255))
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_1;
            }
            else
            {
                pc_fsm = PC_FSM_MST_ON;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_0;  // Retry
        }
        break;

    case PC_FSM_ALLOCATE_STREAM_1:  // Allocate Stream 1 to port1_idx
        if (!btc_dptx_is_link_up(0))
        {
            pc_fsm = PC_FSM_NOOUT;
            break;
        }

        chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
#if DP_SUPPORT_TX && (DP_SUPPORT_RX == 0)
        // Set VFREQ for TX-only design in DP2.0 so the following MVID/NVID queries are valid
        if ((chan_coding & 0x2) == 0x2)
            dp2p0_dptxll_stream_set_vfreq(0, 1, 594000/MST_TX_STREAMS); // Enough for 1080p60, as used in Tx-only example design
#endif
        pixel_rate = 0ULL;
        if ((chan_coding & 0x2) == 0x2)
        {
            msa_reg = IORD(btc_dptx_baseaddr(0), DPTX1_REG_MSA_COLOUR);
            msa_bpc = msa_reg & 0x7;
            msa_color = (msa_reg >> 4) & 0xf;
            msa_color_range = (msa_reg >> 12) & 0x01;
            msa_color_colorimetry = (msa_reg >> 8) & 0x0f;
            msa_color_sdp = (msa_reg >> 13) & 0x01;
            btc_dptxll_stream_set_color_space(0, 1, msa_color, msa_bpc, msa_color_range, msa_color_colorimetry, msa_color_sdp);

            pixel_rate = IORD(btc_dptx_baseaddr(0), DPTX1_REG_MSA_NVID) & 0x00FFFFFF;
            pixel_rate = pixel_rate << 24;
            pixel_rate |= IORD(btc_dptx_baseaddr(0), DPTX1_REG_MSA_MVID) & 0x00FFFFFF;
            DGB_PRINTF("Vfreq  : %d Hz\n", pixel_rate);
            pixel_rate = pixel_rate/1000;
        }

        if (pixel_rate == 0)
        {
            pixel_rate = 594000 / MST_TX_STREAMS;
        }
        btc_dptxll_stream_set_pixel_rate(0, 1, pixel_rate);

        vcp_size = btc_dptxll_stream_calc_VCP_size(0, 1);
        if (!vcp_size || vcp_size > 63)
        {
            DGB_PRINTF("Could not allocate Stream 1, invalid vcp_size=%d\n", vcp_size);
            pc_fsm = PC_FSM_NOOUT;
        }
        else
        {
            btc_dptxll_stream_allocate_req(0, 1, &dev_ports[port1_idx]);
            pc_fsm = PC_FSM_WAIT_ALLOCATED_1;
        }
        break;

    case PC_FSM_WAIT_ALLOCATED_1:  // Wait for Stream 1 allocation
        i = btc_dptxll_stream_allocate_rep(0);
        if (i == 0)
        {
            if ((MST_TX_STREAMS > 2) && (port2_idx != 255))
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_2;
            }
            else
            {
                pc_fsm = PC_FSM_MST_ON;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_1;  // Retry
        }

        break;

    case PC_FSM_ALLOCATE_STREAM_2:  // Allocate Stream 2 to port2_idx
        if (!btc_dptx_is_link_up(0))
        {
            pc_fsm = PC_FSM_NOOUT;
            break;
        }

        chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
#if DP_SUPPORT_TX && (DP_SUPPORT_RX == 0)
        // Set VFREQ for TX-only design in DP2.0 so the following MVID/NVID queries are valid
        if ((chan_coding & 0x2) == 0x2)
            dp2p0_dptxll_stream_set_vfreq(0, 2, 594000/MST_TX_STREAMS); // Enough for 1080p60, as used in Tx-only example design
#endif
        pixel_rate = 0ULL;
        if ((chan_coding & 0x2) == 0x2)
        {
            msa_reg = IORD(btc_dptx_baseaddr(0), DPTX2_REG_MSA_COLOUR);
            msa_bpc = msa_reg & 0x7;
            msa_color = (msa_reg >> 4) & 0xf;
            msa_color_range = (msa_reg >> 12) & 0x01;
            msa_color_colorimetry = (msa_reg >> 8) & 0x0f;
            msa_color_sdp = (msa_reg >> 13) & 0x01;
            btc_dptxll_stream_set_color_space(0, 2, msa_color, msa_bpc, msa_color_range, msa_color_colorimetry, msa_color_sdp);

            pixel_rate = IORD(btc_dptx_baseaddr(0), DPTX2_REG_MSA_NVID) & 0x00FFFFFF;
            pixel_rate = pixel_rate << 24;
            pixel_rate |= IORD(btc_dptx_baseaddr(0), DPTX2_REG_MSA_MVID) & 0x00FFFFFF;
            DGB_PRINTF("Vfreq  : %d Hz\n", pixel_rate);
            pixel_rate = pixel_rate / 1000;
        }

        if (pixel_rate == 0)
        {
            pixel_rate = 594000 / MST_TX_STREAMS;
        }
        btc_dptxll_stream_set_pixel_rate(0, 2, pixel_rate);

        vcp_size = btc_dptxll_stream_calc_VCP_size(0, 2);
        if (!vcp_size || vcp_size > 63)
        {
            DGB_PRINTF("Could not allocate Stream 2, invalid vcp_size=%d\n", vcp_size);
            pc_fsm = PC_FSM_NOOUT;
        }
        else
        {
            btc_dptxll_stream_allocate_req(0, 2, &dev_ports[port2_idx]);
            pc_fsm = PC_FSM_WAIT_ALLOCATED_2;
        }
        break;

    case PC_FSM_WAIT_ALLOCATED_2:  // Wait for Stream 2 allocation
        i = btc_dptxll_stream_allocate_rep(0);
        if (i == 0)
        {
            if ((MST_TX_STREAMS > 3) && (port3_idx != 255))
            {
                pc_fsm = PC_FSM_ALLOCATE_STREAM_3;
            }
            else
            {
                pc_fsm = PC_FSM_MST_ON;
            }
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_2;  // Retry
        }
        break;

    // stream 3
    case PC_FSM_ALLOCATE_STREAM_3:  // Allocate Stream 3 to port3_idx
        if (!btc_dptx_is_link_up(0))
        {
            pc_fsm = PC_FSM_NOOUT;
            break;
        }

        chan_coding = (IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 10) & 0x03; // Use CHAN_CODING_SET
#if DP_SUPPORT_TX && (DP_SUPPORT_RX == 0)
        // Set VFREQ for TX-only design in DP2.0 so the following MVID/NVID queries are valid
        if ((chan_coding & 0x2) == 0x2)
            dp2p0_dptxll_stream_set_vfreq(0, 3, 594000/MST_TX_STREAMS); // Enough for 1080p60, as used in Tx-only example design
#endif
        pixel_rate = 0ULL;
        if ((chan_coding & 0x2) == 0x2)
        {
            msa_reg = IORD(btc_dptx_baseaddr(0), DPTX3_REG_MSA_COLOUR);
            msa_bpc = msa_reg & 0x7;
            msa_color = (msa_reg >> 4) & 0xf;
            msa_color_range = (msa_reg >> 12) & 0x01;
            msa_color_colorimetry = (msa_reg >> 8) & 0x0f;
            msa_color_sdp = (msa_reg >> 13) & 0x01;
            btc_dptxll_stream_set_color_space(0, 3, msa_color, msa_bpc, msa_color_range, msa_color_colorimetry, msa_color_sdp);

            pixel_rate = IORD(btc_dptx_baseaddr(0), DPTX3_REG_MSA_NVID) & 0x00FFFFFF;
            pixel_rate = pixel_rate << 24;
            pixel_rate |= IORD(btc_dptx_baseaddr(0), DPTX3_REG_MSA_MVID) & 0x00FFFFFF;
            DGB_PRINTF("Vfreq  : %d Hz\n", pixel_rate);
            pixel_rate = pixel_rate / 1000;
        }

        if (pixel_rate == 0)
        {
            pixel_rate = 594000 / MST_TX_STREAMS;
        }
        btc_dptxll_stream_set_pixel_rate(0, 3, pixel_rate);

        vcp_size = btc_dptxll_stream_calc_VCP_size(0, 3);
        if (!vcp_size || vcp_size > 63)
        {
            DGB_PRINTF("Could not allocate Stream 3, invalid vcp_size=%d\n", vcp_size);
            pc_fsm = PC_FSM_NOOUT;
        }
        else
        {
            btc_dptxll_stream_allocate_req(0, 3, &dev_ports[port3_idx]);
            pc_fsm = PC_FSM_WAIT_ALLOCATED_3;
        }
        break;

    case PC_FSM_WAIT_ALLOCATED_3:  // Wait for Stream 3 allocation
        i = btc_dptxll_stream_allocate_rep(0);
        if (i == 0)
        {
            pc_fsm = PC_FSM_MST_ON;
        }
        else if (i == 1)
        {
            pc_fsm = PC_FSM_ALLOCATE_STREAM_3;  // Retry
        }
        break;

    case PC_FSM_MST_DATA:  // Send MST payload  // can be removed
        pc_fsm = PC_FSM_MST_ON;
        break;

    case PC_FSM_MST_ON:  // MST ON!
        break;

    case PC_FSM_NOOUT:  // No suitable sink device available
        break;
    }

    // Restart the FSM on HPD transitions
    if (pc_fsm_force_hpd0)
    {
        pc_fsm_force_hpd0 = 0;
        pc_fsm = PC_FSM_HPD_0;
        DGB_PRINTF("pc_fsm: HPD forced to 0\n");
    }
    else if (pc_fsm_force_hpd1)
    {
        pc_fsm_force_hpd1 = 0;
        pc_fsm = PC_FSM_HPD_1;
        DGB_PRINTF("pc_fsm: HPD forced to 1\n");
    }
}

//******************************************************
// CONNECTION_STATUS_NOTIFY callback
//******************************************************
void bitec_csn_callback(BTC_MST_CONN_STAT_NOTIFY* csn_data)
{
}

//******************************************************
// Handle an HPD_IRQ for HDCP
//
// Input:   tx_idx       TX instance index (0 - 3)
// Return:  0 = success, 1 = fail
//******************************************************
int btc_dptxll_hpd_irq_hdcp(BYTE tx_idx)
{
    BYTE lstat[6], data[2], esi0;
    unsigned int status_ok, lane_count, link_rate, cntrl;
    int ret;

    btc_dptx_aux_read(tx_idx, DPCD_ADDR_SINK_COUNT, 6, lstat);  // Read link status
    btc_dptx_aux_read(tx_idx, DPCD_ADDR_TEST_REQUEST, 1, data);

    // Check Automated test request
    if ((lstat[1] & 0x02) && (data[0] != 0x00))
    {
        // Test Automation request
        btc_dptx_test_autom(tx_idx);
        return 0;
    }

    // Check CP_IRQ
    if (lstat[1] & 0x04)
    {
        // HDCP CP_IRQ
#if DP_SUPPORT_TX_HDCP
        hdcp_cp_irq(0);
#else
        BYTE data = 0x04;
        btc_dptx_aux_write(tx_idx, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR, 1, &data);  // Reset CP_IRQ
#endif
        return 0;
    }

    if (btc_dptxll_mst_sink_mst_cap(tx_idx))
    {
        // Check for MST messages
        btc_dptx_aux_read(tx_idx, DPCD_ADDR_DEVICE_SERVICE_IRQ_VECTOR_ESI0, 1, &esi0);
        if (esi0 & 0x10)
            btc_dptx_mst_down_rep_irq(tx_idx);  // DOWN_REP_MSG_RDY = 1
        if (esi0 & 0x20)
            btc_dptx_mst_up_req_irq(tx_idx);  // UP_REQ_MSG_RDY = 1
    }

    // Check Downstream port status change
    if (lstat[4] & (1 << 6))
        btc_dptx_edid_read(tx_idx, btc_dptxll_mst_sink_edid(tx_idx));  // Read the sink EDID

    // Get current link rate and lane count
    btc_dptx_aux_read(tx_idx, DPCD_ADDR_LINK_BW_SET, 2, data);
    link_rate = data[0] & 0xFF;
    lane_count = data[1] & 0x1F;

    // Check link status
    status_ok = lstat[4] & 0x01;  // Get inter-lane align
    switch (lane_count)
    {
    case 1:
        status_ok &= ((lstat[2] & 0x07) == 0x07);
        break;
    case 2:
        status_ok &= ((lstat[2] & 0x77) == 0x77);
        break;
    case 4:
        status_ok &= ((lstat[2] & 0x77) == 0x77) & ((lstat[3] & 0x77) == 0x77);
        break;
    default:
        break;
    }
    if (!status_ok)
    {
#if DP_SUPPORT_TX_HDCP
        hdcp_cp_irq(0);
#endif

#if (DP_SUPPORT_TX_FEC)
        ret = btc_dptx_link_training_fec(0, link_rate, lane_count,1);  // Enable FEC then do link training 
#else
        ret = btc_dptx_link_training(0, link_rate, lane_count);        // Do link training without FEC
#endif
        if (ret == 0)
        {
            // link was down and now is back up: refresh the (lost) VCP table to HW
            cntrl = IORD(btc_dptx_baseaddr(tx_idx), DPTX_REG_MST_CONTROL1);
            IOWR(btc_dptx_baseaddr(tx_idx), DPTX_REG_MST_CONTROL1,
                 cntrl | 0x80000000);  // Force VC table update
        }
        return ret;
    }

    return 0;
}

#endif
