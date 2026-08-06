/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// ********************************************************************************
// Implementation of an Aux Bus Decoder
//
// Description:
//
// ********************************************************************************
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

#if (BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX) || (BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX)
#include "aux_decoder.h"

#include "altera_avalon_fifo_regs.h"
#include "altera_avalon_fifo_util.h"

// Look-up the addr
char* bitec_dp_aux2string(unsigned int addr)
{
    switch (addr)
    {
    case 0x00:
        return ("DPCD_REV");
        break;
    case 0x01:
        return ("MAX_LINK_RATE");
        break;
    case 0x02:
        return ("MAX_LANE_COUNT");
        break;
    case 0x03:
        return ("MAX_DOWNSPREAD");
        break;
    case 0x04:
        return ("NORP");
        break;
    case 0x05:
        return ("DOWNSTREAMPORT_PRESENT");
        break;
    case 0x06:
        return ("MAIN_LINK_CHANNEL_CODING");
        break;
    case 0x07:
        return ("DPCD");
        break;
    case 0x08:
        return ("RECEIVE_PORT0_CAP_0");
        break;
    case 0x09:
        return ("RECEIVE_PORT0_CAP_1");
        break;
    case 0x0A:
        return ("RECEIVE_PORT1_CAP_0");
        break;
    case 0x0B:
        return ("RECEIVE_PORT1_CAP_1");
        break;
    case 0x0E:
        return ("8b10b_TRAINING_AUX_RD_INTERVAL");
        break;
    case 0x20:
        return ("SINK_VIDEO_FALLBACK_FORMATS");
        break;
    case 0x21:
        return ("MSTM_CAP");
        break;
    case 0x60:
        return ("DSC_SUPPORT");
        break;
    case 0x80:
        return ("DPCD");
        break;
    case 0x90:
        return ("FEC_CAPABILITY_0");
        break;
    case 0xA0:
        return ("BRANCH_DSC_OVERALL_THROUGHPUT_MODE_0");
        break;
    case 0xA1:
        return ("BRANCH_DSC_OVERALL_THROUGHPUT_MODE_0");
        break;
    case 0xB0:
        return ("PANEL_REPLAY_CAPABILITY_SUPPORTED");
        break;
    case 0x100:
        return ("LINK_BW_SET");
        break;
    case 0x101:
        return ("LANE_COUNT_SET");
        break;
    case 0x102:
        return ("TRAINING_PATTERN_SET");
        break;
    case 0x103:
        return ("TRAINING_LANE0_SET");
        break;
    case 0x104:
        return ("TRAINING_LANE1_SET");
        break;
    case 0x105:
        return ("TRAINING_LANE2_SET");
        break;
    case 0x106:
        return ("TRAINING_LANE3_SET");
        break;
    case 0x107:
        return ("DOWNSPREAD_CTRL");
        break;
    case 0x108:
        return ("MAIN_LINK_CHANNEL_CODING_SET");
        break;
    case 0x110:
        return ("CABLE_ATTRIBUTES_UPDATED_BY_DPTX");
        break;
    case 0x111:
        return ("MSTM_CTRL");
        break;
    case 0x120:
        return ("FEC_CONFIGURATION");
        break;
    case 0x1C0:
        return ("PAYLOAD_ALLOCATE_SET");
        break;
    case 0x200:
        return ("SINK_COUNT");
        break;
    case 0x201:
        return ("DEVICE_SERVICE_IRQ_VECTOR");
        break;
    case 0x202:
        return ("LANE0_1_STATUS");
        break;
    case 0x203:
        return ("LANE2_3_STATUS");
        break;
    case 0x204:
        return ("LANE_ALIGN_STATUS_UPDATED");
        break;
    case 0x205:
        return ("SINK_STATUS");
        break;
    case 0x206:
        return ("ADJUST_REQUEST_LANE0_1");
        break;
    case 0x207:
        return ("ADJUST_REQUEST_LANE2_3");
        break;
    case 0x208:
        return ("TRAINING_SCORE_LANE0");
        break;
    case 0x209:
        return ("TRAINING_SCORE_LANE1");
        break;
    case 0x20a:
        return ("TRAINING_SCORE_LANE2");
        break;
    case 0x20b:
        return ("TRAINING_SCORE_LANE3");
        break;
    case 0x210:
        return ("SYMBOL_ERROR_COUNT_LANE0_lsb");
        break;
    case 0x211:
        return ("SYMBOL_ERROR_COUNT_LANE0_msb");
        break;
    case 0x212:
        return ("SYMBOL_ERROR_COUNT_LANE1_lsb");
        break;
    case 0x213:
        return ("SYMBOL_ERROR_COUNT_LANE1_msb");
        break;
    case 0x214:
        return ("SYMBOL_ERROR_COUNT_LANE2_lsb");
        break;
    case 0x215:
        return ("SYMBOL_ERROR_COUNT_LANE2_msb");
        break;
    case 0x216:
        return ("SYMBOL_ERROR_COUNT_LANE3_lsb");
        break;
    case 0x217:
        return ("SYMBOL_ERROR_COUNT_LANE3_msb");
        break;
    case 0x218:
        return ("TEST_REQUEST");
        break;
    case 0x219:
        return ("TEST_LINK_RATE");
        break;
    case 0x220:
        return ("TEST_LANE_COUNT");
        break;
    case 0x221:
        return ("TEST_PATTERN");
        break;
    case 0x222:
        return ("TEST_H_TOTAL_lsb");
        break;
    case 0x223:
        return ("TEST_H_TOTAL_msb");
        break;
    case 0x224:
        return ("TEST_V_TOTAL_lsb");
        break;
    case 0x225:
        return ("TEST_V_TOTAL_msb");
        break;
    case 0x226:
        return ("TEST_H_START_lsb");
        break;
    case 0x227:
        return ("TEST_H_START_msb");
        break;
    case 0x228:
        return ("TEST_V_START_lsb");
        break;
    case 0x229:
        return ("TEST_V_START_msb");
        break;
    case 0x22a:
        return ("TEST_HSYNC_lsb");
        break;
    case 0x22b:
        return ("TEST_HSYNC_msb");
        break;
    case 0x22c:
        return ("TEST_VSYNC_lsb");
        break;
    case 0x22d:
        return ("TEST_VSYNC_msb");
        break;
    case 0x22e:
        return ("TEST_H_WIDTH_lsb");
        break;
    case 0x22f:
        return ("TEST_H_WIDTH_msb");
        break;
    case 0x230:
        return ("TEST_V_HEIGHT_lsb");
        break;
    case 0x231:
        return ("TEST_V_HEIGHT_msb");
        break;
    case 0x232:
        return ("TEST_MISC_lsb");
        break;
    case 0x233:
        return ("TEST_MISC_msb");
        break;
    case 0x234:
        return ("TEST_REFRESH_RATE_NUMERATOR");
        break;
    case 0x240:
        return ("TEST_CRC_R_Cr_lsb");
        break;
    case 0x241:
        return ("TEST_CRC_R_Cr_msb");
        break;
    case 0x242:
        return ("TEST_CRC_G_Y_lsb");
        break;
    case 0x243:
        return ("TEST_CRC_G_Y_msb");
        break;
    case 0x244:
        return ("TEST_CRC_B_Cb_lsb");
        break;
    case 0x245:
        return ("TEST_CRC_B_Cb_msb");
        break;
    case 0x246:
        return ("TEST_SINK_MISC");
        break;
    case 0x248:
        return ("DPCD");
        break;
    case 0x260:
        return ("TEST_RESPONSE");
        break;
    case 0x261:
        return ("TEST_EDID_CHECKSUM");
        break;
    case 0x270:
        return ("TEST_SINK");
        break;
    case 0x280:
        return ("FEC_STATUS");
        break;
    case 0x2C0:
        return ("PAYLOAD_TABLE_UPDATE_STATUS");
        break;
    case 0x600:
        return ("SET_POWER_AND_SET_DP_PWR_VOLTAGE");
        break;
    case 0x2003:
        return ("DEVICE_SERVICE_IRQ_VECTOR_ESI0");
        break;
    case 0x2004:
        return ("DEVICE_SERVICE_IRQ_VECTOR_ESI1");
        break;
    case 0x2200:
        return ("DPCD_REV");
        break;
    case 0x2201:
        return ("8b10b_MAX_LINK_RATE");
        break;
    case 0x2202:
        return ("MAX_LANE_COUNT");
        break;
    case 0x2203:
        return ("MAX_DOWNSPREAD");
        break;
    case 0x2204:
        return ("NORP");
        break;
    case 0x2205:
        return ("DOWNSTREAMPORT_PRESENT");
        break;
    case 0x2206:
        return ("MAIN_LINK_CHANNEL_CODING");
        break;
    case 0x2207:
        return ("DOWNSTREAM_PORT_COUNT");
        break;
    case 0x2208:
        return ("RECEIVE_PORT0_CAP_0");
        break;
    case 0x2209:
        return ("RECEIVE_PORT0_CAP_1");
        break;
    case 0x220A:
        return ("RECEIVE_PORT1_CAP_0");
        break;
    case 0x220B:
        return ("RECEIVE_PORT1_CAP_1");
        break;
    case 0x220E:
        return ("8b10b_TRAINING_AUX_RD_INTERVAL");
        break;
    case 0x2214:
        return ("DPRX_FEATURE_ENUMERATION_LIST_CONT_1");
        break;
    case 0x2215:
        return ("128b132b_SUPPORTED_LINK_RATES");
        break;
    case 0x2216:
        return ("128b132b_DP_TRAINING_AUX_RD_INTERVAL");
        break;
    case 0x2217:
        return ("CABLE_ATTRIBUTES_UPDATED_BY_DPRX");
        break;
    case 0xF0000:
        return ("LT_TUNABLE_PHY_REPEATER_FIELD_DATA_STRUCTURE_REV");
        break;
    case 0xF0003:
        return ("PHY_REPEATER_MODE");
        break;   
    default:
        return (" ");
        break;
    }
}

#if ENABLE_AUX_RING_BUFFER
// Initialize the ring buffer cache
void init_fifo(Fifo *pFifo, unsigned int base_addr_csr, unsigned int base_addr_fifo)
{
    pFifo->_base_addr_csr = base_addr_csr;
    pFifo->_base_addr_fifo = base_addr_fifo;
    pFifo->_rd = 0;
    pFifo->_wr = 0;
    pFifo->_full = false;
}

// How much is in the cache?
uint32_t cache_available(Fifo *pFifo)
{
    if (pFifo->_full)
    {
        return MAX_FIFO_BUFFER_SIZE;
    }
    else if (pFifo->_rd > pFifo->_wr)
    {
        return (MAX_FIFO_BUFFER_SIZE - pFifo->_rd) + pFifo->_wr;
    }
    return (pFifo->_wr - pFifo->_rd);
}

// Is there any space left in the cache
uint32_t cache_space(Fifo *pFifo)
{
    if (pFifo->_full)
    {
        return 0;
    }
    return MAX_FIFO_BUFFER_SIZE - cache_available(pFifo);
}

// Reads a sample, data + status, from the fifo it available
int32_t aux_read_fifo_sample(AuxDecoderInstance *pInstance, uint32_t *pfifo_data, uint32_t *pfifo_status)
{
    if ((pInstance == NULL) || (pfifo_data == NULL) || (pfifo_status == NULL))
    {
        return 0;
    }
    Fifo *pFifo = &(pInstance->_fifo);
    // Do we have any space in the fifo?
    uint32_t space = cache_space(pFifo);
    if (space)
    {
        // Try and burst from more data, NOTE: we're doing this in pairs
        uint32_t available = altera_avalon_fifo_read_level(pFifo->_base_addr_csr) * 2;
        if (available)
        {
            // only do upto MAX_FIFO_BURST_SAMPLE
            if (available > MAX_FIFO_BURST_SAMPLE)
                available = MAX_FIFO_BURST_SAMPLE;
            // will that fit ?
            if (available > space)
                available = space;
            uint32_t wr = pFifo->_wr;
            for (uint32_t i = 0; i < available; i += 2)
            {
                pFifo->_data[wr++] = IORD(pFifo->_base_addr_fifo, 0);
                pFifo->_data[wr++] = IORD(pFifo->_base_addr_fifo, 1);
                wr = wr % MAX_FIFO_BUFFER_SIZE;
            }
            pFifo->_wr = wr;
            pFifo->_full = (pFifo->_wr == pFifo->_rd);
        }
    }

    if (cache_available(pFifo))
    {
        *pfifo_data = pFifo->_data[pFifo->_rd++];
        *pfifo_status = pFifo->_data[pFifo->_rd++];
        pFifo->_rd %= MAX_FIFO_BUFFER_SIZE;
        pFifo->_full = false;
        return 1;
    }
    return 0;
}
#else  // ENABLE_AUX_RING_BUFFER
// Initialize the fifo
void init_fifo(Fifo *pFifo, unsigned int base_addr_csr, unsigned int base_addr_fifo)
{
    pFifo->_base_addr_csr = base_addr_csr;
    pFifo->_base_addr_fifo = base_addr_fifo;
}

int32_t aux_read_fifo_sample(AuxDecoderInstance *pInstance, uint32_t *pfifo_data, uint32_t *pfifo_status)
{
    if ((pInstance == NULL) || (pfifo_data == NULL) || (pfifo_status == NULL))
    {
        return 0;
    }

    Fifo *pFifo = &(pInstance->_fifo);

    uint32_t available = altera_avalon_fifo_read_level(pFifo->_base_addr_csr) * 2;
    if (available)
    {
        // Read a single entry from the fifo
        *pfifo_data = IORD(pFifo->_base_addr_fifo, 0);
        *pfifo_status = IORD(pFifo->_base_addr_fifo, 1);
        return 1;
    }

    return 0;
}
#endif // ENABLE_AUX_RING_BUFFER

int dp_dump_aux_debug_init(AuxDecoderInstance *pInstance, unsigned int base_addr_csr, unsigned int base_addr_fifo,
                           const bool is_sink)
{
    if (pInstance == NULL)
    {
        return -1; // Can't have a null pointer
    }
    // Set the instance
    memset(pInstance, 0, sizeof(AuxDecoderInstance));

    init_fifo(&pInstance->_fifo, base_addr_csr, base_addr_fifo);
    pInstance->_is_sink = is_sink;
    pInstance->_state = kIdle;
    pInstance->_time_stamp_old = 123456;
    pInstance->_last_req_native = false;

    // Make sure the message buffer is clear
    pInstance->_message_buffer[0] = '\0';
    return 0;
}

#define APPEND_MESSAGE(pInstance, format, ...) \
    { \
        uint32_t len = strlen(pInstance->_message_buffer); \
        snprintf(pInstance->_message_buffer + len, MAX_AUX_MESSAGE_BUFFER_SIZE-len, format __VA_OPT__(,) __VA_ARGS__); \
    } 

#define OUTPUT_MESSAGE_BUFFER(pInstance) \
    { printf("%s\n", pInstance->_message_buffer); pInstance->_message_buffer[0] = '\0';}

#define AUX_NATIVE_WR 0x80
#define AUX_NATIVE_RD 0x90
#define AUX_I2C_WR 0x00
#define AUX_I2C_RD 0x10
#define AUX_I2C_UPDATE 0x20
#define AUX_I2C_MOT 0x40

#define AUX_ACK 0x00
#define AUX_NACK 0x10
#define AUX_DEFER 0x20
#define AUX_I2C_ACK 0x00
#define AUX_I2C_NACK 0x40
#define AUX_I2C_DEFER 0x80

void dp_dump_aux_debug(AuxDecoderInstance *pInstance)
{
    uint32_t fifo_data, fifo_status;
    bool data_TX;
    uint32_t cmd;

    if (pInstance == NULL)
    {
        return;
    }

    // clear the non-blocking flag. TODO: Is the necessary?
    int flags = 0;
    flags = fcntl(STDOUT_FILENO, F_GETFL);
    fcntl(STDOUT_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    if (aux_read_fifo_sample(pInstance, &fifo_data, &fifo_status))
    {
        // Is data available in DEBUG FIFO?
        if (fifo_status & 1)
        {
            if (pInstance->_state != kIdle)
            {
                // Clear the last command
                APPEND_MESSAGE(pInstance, " ???");
                OUTPUT_MESSAGE_BUFFER(pInstance);
                pInstance->_state = kIdle;
            }

            // Check for start of packet
            if (pInstance->_time_stamp_old == 123456)
            {
                APPEND_MESSAGE(pInstance, "%8.8d ", 0); // Print time stamp
            }
            else
            {
                APPEND_MESSAGE(pInstance, "%8.8ld ", ((fifo_data >> 8) - pInstance->_time_stamp_old) ); // Print time stamp
            }
            // remember the current timestamp
            pInstance->_time_stamp_old = (fifo_data >> 8);

            if ((fifo_status & (3 << 8)) == 0x200) // We have an event, channel 2
            {
                if (pInstance->_is_sink)
                {
                    APPEND_MESSAGE(pInstance, "[SNK] - Event -  ");
                }
                else
                {
                    APPEND_MESSAGE(pInstance, "[SRC] - Event -  ");
                }

                if (fifo_data & 0x01)
                { // HPD Event
                    APPEND_MESSAGE(pInstance, "HPD=%d ", (fifo_data & 0x02) ? 1 : 0);
                }
                if (fifo_data & 0x04)
                { // Cable Event
                    APPEND_MESSAGE(pInstance, "Cable=%d ", (fifo_data & 0x08) ? 1 : 0);
                }
                if (fifo_data & 0x10)
                { // Power Event
                    APPEND_MESSAGE(pInstance, "Power=%d ", (fifo_data & 0x20) ? 1 : 0);
                }
                OUTPUT_MESSAGE_BUFFER(pInstance);
            }
            else
            {
                data_TX = ((fifo_status & (3 << 8)) == 0x100); // Is this a data_TX , channel 0 or 1
                if ((data_TX && !pInstance->_is_sink) || (!data_TX && pInstance->_is_sink))
                {
                    // We are source and sent a Request or we are a sink and got a Request
                    cmd = fifo_data;
                    cmd = cmd & 0xFF;
                    if (pInstance->_is_sink)
                    {
                        APPEND_MESSAGE(pInstance, "[SNK] Req got    ");
                    }
                    else
                    {
                        APPEND_MESSAGE(pInstance, "[SRC] Req sent   ");
                    }
                    if (cmd & 0x80)
                    {
                        // Native
                        pInstance->_last_req_native = true;
                        if (cmd & 0x10)
                        {
                            APPEND_MESSAGE(pInstance, "AUX_RD @ ");
                        }
                        else
                        {
                            APPEND_MESSAGE(pInstance, "AUX_WR @ ");
                        }
                        pInstance->_aux_addr = (cmd & 0x0F) << 16;
                        pInstance->_state = kWaitingForAddr1; // Next time we should get the first addr byte
                    }
                    else
                    {
                        // I2C
                        pInstance->_last_req_native = false;
                        if ((cmd & 0x30) == AUX_I2C_WR) 
                        {
                            APPEND_MESSAGE(pInstance, "I2C_WR ");
                        }
                        else if ((cmd & 0x30) == AUX_I2C_RD)
                        {
                            APPEND_MESSAGE(pInstance, "I2C_RD ");
                        }
                        else if ((cmd & 0x30) == AUX_I2C_UPDATE)
                        {
                            APPEND_MESSAGE(pInstance, "I2C_UPD ");
                        }
                        else
                        {
                            APPEND_MESSAGE(pInstance, "I2C_?? ");
                        }

                        if (cmd & AUX_I2C_MOT)
                        {
                            APPEND_MESSAGE(pInstance, "MOT=1");
                        }
                        else
                        {
                            APPEND_MESSAGE(pInstance, "MOT=0");
                        }
                        APPEND_MESSAGE(pInstance, " %02lX", cmd);
                        pInstance->_state = kReadingPayload;
                    }
                }
                else if ((!data_TX && !pInstance->_is_sink) || (data_TX && pInstance->_is_sink))
                {
                    // We are a source and got a Reply or we are a sink and send a Reply
                    cmd = fifo_data;
                    cmd = cmd & 0xFF;
                    if (pInstance->_is_sink)
                    {
                        APPEND_MESSAGE(pInstance, "[SNK] Reply sent ");
                    }
                    else
                    {
                        APPEND_MESSAGE(pInstance, "[SRC] Reply got  ");
                    }
                    if ((cmd & 0x30) == AUX_ACK)
                    {
                        APPEND_MESSAGE(pInstance, "AUX_ACK");
                    }
                    else if ((cmd & 0x30) == AUX_NACK)
                    {
                        APPEND_MESSAGE(pInstance, "AUX_NACK");
                    }
                    else if ((cmd & 0x30) == AUX_DEFER)
                    {
                        APPEND_MESSAGE(pInstance, "AUX_DEFER");
                    }
                    if (!pInstance->_last_req_native)
                    {
                        if ((cmd & 0xC0) == AUX_I2C_ACK)
                        {
                            APPEND_MESSAGE(pInstance, "|I2C_ACK");
                        }
                        else if ((cmd & 0xC0) == AUX_I2C_NACK)
                        {
                            APPEND_MESSAGE(pInstance, "|I2C_NACK");
                        }
                        else if ((cmd & 0xC0) == AUX_I2C_DEFER)
                        {
                            APPEND_MESSAGE(pInstance, "|I2C_DEFER");
                        }
                    }
                    APPEND_MESSAGE(pInstance, " %02lX", cmd);
                    pInstance->_state = kReadingPayload;
                }
            }
        } // if(fifo_status & 1)
        else if (fifo_status & 2)
        {
            // Discard EOP
            OUTPUT_MESSAGE_BUFFER(pInstance);
            pInstance->_state = kIdle;
        } // else if (fifo_status & 2)
        else
        {
            switch (pInstance->_state)
            {
            case kWaitingForAddr1:
                pInstance->_aux_addr |= (fifo_data & 0xFF) << 8;
                pInstance->_state = kWaitingForAddr2;
                break;
            case kWaitingForAddr2:
                pInstance->_aux_addr |= (fifo_data & 0xFF);
                APPEND_MESSAGE(pInstance, "%04lX (%s)", pInstance->_aux_addr, bitec_dp_aux2string(pInstance->_aux_addr));
                APPEND_MESSAGE(pInstance, " %02lX %02lX %02lX", (pInstance->_aux_addr >> 16) & 0x0F,
                        (pInstance->_aux_addr >> 8) & 0xFF, pInstance->_aux_addr & 0xFF);
                pInstance->_state = kReadingPayload;
                break;
            case kReadingPayload:
                APPEND_MESSAGE(pInstance, " %02lX", fifo_data & 0xFF);
                break;
            default:
                APPEND_MESSAGE(pInstance, " %02lX", fifo_data & 0xFF);
                break;
            }
        }
    }
    // Reset the state. TODO : Is this necessary?
    fcntl(STDOUT_FILENO, F_SETFL, flags);
}
#endif // (BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX) || (BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX)
