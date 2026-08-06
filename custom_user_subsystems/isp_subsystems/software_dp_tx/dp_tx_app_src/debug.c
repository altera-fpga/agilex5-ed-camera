/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// ********************************************************************************
// DisplayPort Core test code debug routines
//
// Description:
//
// ********************************************************************************
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <unistd.h>
#if DP_SUPPORT_RX 
#include "btc_dprx_syslib.h"
#endif /* DP_SUPPORT_RX */
#include "btc_dptx_syslib.h"
#include "debug.h"
#include "config.h"
#if (BITEC_TX_AUX_DEBUG && DP_SUPPORT_TX) || (BITEC_RX_AUX_DEBUG && DP_SUPPORT_RX)
#include "altera_avalon_fifo_regs.h"
#include "altera_avalon_fifo_util.h"
#endif
#include "sys/alt_timestamp.h"

#define DEBUG_PRINT_ENABLED 0
#if DEBUG_PRINT_ENABLED
#define DGB_PRINTF printf
#else
#define DGB_PRINTF(format, args...) ((void)0)
#endif

#if BITEC_STATUS_DEBUG

char btc_stdbuf[20];
BYTE btc_stdbuf_ptr = 0;

#if DP_SUPPORT_RX
void bitec_dp_dump_sink_msa(unsigned int base_addr)
{
    unsigned rx0_vbid, rx1_vbid;
    unsigned int vfreq0_dec, vfreq1_dec;
    BYTE ch_coding;
    ch_coding = (IORD(base_addr, DPRX_REG_RX_CONTROL)) >> 5 & 0x03;

    rx0_vbid = IORD(base_addr, DPRX0_REG_VBID);
    rx1_vbid = IORD(base_addr, DPRX1_REG_VBID);

    printf("------------------------------------------\n");
    printf("------   RX Main stream attributes  ------\n");
    printf("------------------------------------------\n");
    if (ch_coding == 0x2)
        printf("--- Channel Coding : 128b132b\n");
    else
        printf("--- Channel Coding : 8b10b\n");
    printf("------------------------------------------\n");
    printf("--- Stream 0 ---\n");
    printf("VB-ID lock : %1.1X   MSA lock : %1.1X\n", (rx0_vbid >> 6) & 1, (rx0_vbid >> 7) & 1);
    printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n",
           ((rx0_vbid & 0x100) >> 2) | (rx0_vbid & 0x3F), IORD(base_addr, DPRX0_REG_MSA_MISC0),
           IORD(base_addr, DPRX0_REG_MSA_MISC1));
    if (ch_coding == 0x2)
    {
        vfreq0_dec = IORD(base_addr, DPRX0_REG_MSA_MVID);
        vfreq0_dec &= 0x00FFFFFF;
        vfreq0_dec |= IORD(base_addr, DPRX0_REG_MSA_NVID) << 24;
        printf("Vfreq  : %X (%d Hz) \n", vfreq0_dec, vfreq0_dec);
    }
    else
        printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPRX0_REG_MSA_MVID),
               IORD(base_addr, DPRX0_REG_MSA_NVID));
    printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HTOTAL),
           IORD(base_addr, DPRX0_REG_MSA_VTOTAL));
    printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HSP),
           IORD(base_addr, DPRX0_REG_MSA_HSW));
    printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HSTART),
           IORD(base_addr, DPRX0_REG_MSA_VSTART));
    printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_VSP),
           IORD(base_addr, DPRX0_REG_MSA_VSW));
    printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPRX0_REG_MSA_HWIDTH),
           IORD(base_addr, DPRX0_REG_MSA_VHEIGHT));
    printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPRX0_REG_CRC_R),
           IORD(base_addr, DPRX0_REG_CRC_G), IORD(base_addr, DPRX0_REG_CRC_B));
    printf("--- Stream 1 ---\n");
    printf("VB-ID lock : %1.1X   MSA lock : %1.1X\n", (rx1_vbid >> 6) & 1, (rx1_vbid >> 7) & 1);
    printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n",
           ((rx1_vbid & 0x100) >> 2) | (rx1_vbid & 0x3F), IORD(base_addr, DPRX1_REG_MSA_MISC0),
           IORD(base_addr, DPRX1_REG_MSA_MISC1));
    if (ch_coding == 0x2)
    {
        vfreq1_dec = IORD(base_addr, DPRX1_REG_MSA_MVID);
        vfreq1_dec &= 0x00FFFFFF;
        vfreq1_dec |= IORD(base_addr, DPRX1_REG_MSA_NVID) << 24;
        printf("Vfreq  : %X (%d Hz) \n", vfreq1_dec, vfreq1_dec);
    }
    else
        printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPRX1_REG_MSA_MVID),
               IORD(base_addr, DPRX1_REG_MSA_NVID));
    printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPRX1_REG_MSA_HTOTAL),
           IORD(base_addr, DPRX1_REG_MSA_VTOTAL));
    printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPRX1_REG_MSA_HSP),
           IORD(base_addr, DPRX1_REG_MSA_HSW));
    printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPRX1_REG_MSA_HSTART),
           IORD(base_addr, DPRX1_REG_MSA_VSTART));
    printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPRX1_REG_MSA_VSP),
           IORD(base_addr, DPRX1_REG_MSA_VSW));
    printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPRX1_REG_MSA_HWIDTH),
           IORD(base_addr, DPRX1_REG_MSA_VHEIGHT));
    printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPRX1_REG_CRC_R),
           IORD(base_addr, DPRX1_REG_CRC_G), IORD(base_addr, DPRX1_REG_CRC_B));
}


void bitec_dp_dump_sink_config(unsigned int base_addr)
{
    unsigned int ch_coding;
    unsigned int link_rate;
    unsigned ber_cntrl;
    unsigned int mst_control1;
    BYTE stream_id[4];            // Mapping between stream ands assigned payload IDs (valid payload ID are in the range 1-7)
    uint64_t mst_alloc[8];        // Allocation for payload IDs (ID 0==unassigned slots)
    BYTE slot_count[8];           // Slot count for payload IDs (ID 0==number of unassigned slots)
    BYTE idx, offset, s, slot;
    unsigned int slice;

    ch_coding = (IORD(base_addr, DPRX_REG_RX_CONTROL) >> 5) & 0x03;
    mst_control1 = IORD(base_addr, DPRX_REG_MST_CONTROL1);

    printf("------------------------------------------\n");
    printf("--------   RX Link configuration   -------\n");
    printf("------------------------------------------\n");
    printf("CR Done: %1.1X        SYM Done: %1.1X\n", IORD(base_addr, DPRX_REG_RX_STATUS) & 0x0f,
           (IORD(base_addr, DPRX_REG_RX_STATUS) & 0xf0) >> 4);
    printf("Lane count : %d\n", IORD(base_addr, DPRX_REG_RX_CONTROL) & 0x001f);

    if (ch_coding == 0x2)
    {
        printf("Channel Coding : 128b132b\n");
        link_rate = (IORD(base_addr, DPRX_REG_RX_CONTROL) >> 16) & 0xFF;
        switch (link_rate)
        {
        case 0x1:
            printf("Link rate  : 10 Gbps\n");
            break;
        case 0x2:
            printf("Link rate  : 20 Gbps\n");
            break;
        case 0x4:
            printf("Link rate  : 13.5 Gbps\n");
            break;
        default:
            printf("Link rate  : 0 Gbps\n");
            break;
        }
    }
    else
    {
        printf("Channel Coding : 8b10b\n");
        printf("Link rate  : %d Mbps\n",
               ((IORD(base_addr, DPRX_REG_RX_CONTROL) >> 16) & 0xff) * 270);
        if (mst_control1 & 0x01)
            printf("MST : on\n");
        else
            printf("MST : off\n");
    }
    printf("BER0   : %4.4X     BER1    : %4.4X\n", IORD(base_addr, DPRX_REG_BER_CNT0) & 0x7FFF,
           (IORD(base_addr, DPRX_REG_BER_CNT0) >> 16) & 0x7FFF);
    printf("BER2   : %4.4X     BER3    : %4.4X\n", IORD(base_addr, DPRX_REG_BER_CNT1) & 0x7FFF,
           (IORD(base_addr, DPRX_REG_BER_CNT1) >> 16) & 0x7FFF);
    
    // Check slot allocation
    if ((ch_coding == 0x2) || (mst_control1 & 0x01))
    {
        // Zero the allocation for each payload ID
        for (s = 0; s < 8; ++s)
        {
            slot_count[s] = 0;
            mst_alloc[s]  = 0ULL;
        }
        // Pull the mapping stream ID -> payload ID from DPRX_REG_MST_CONTROL1
        for (s = 0; s < 4; ++s)
        {
            stream_id[s] = (mst_control1 >> (4 * (s+1))) & 0x0F;
            // A value of 0xF is used to indicate an unasigned stream, map all values that are out of the valid range 1-7 to 0 (unassigned)
            if (stream_id[s] > 7) stream_id[s] = 0;
        }
        slot = 0;
        for (idx = 0; idx < 8; ++idx)
        {
            slice = IORD(base_addr, DPRX_REG_MST_VCPTAB0 + idx);
            offset = 0;
            // Skip the very first slot in HBR rates with MST enabled
            if ((ch_coding != 0x2) && (idx == 0))
            {
                offset = 1;
                slice = slice >> 4;
                ++slot;
            }
            while (offset < 8)
            {
                // map all values that are out of the valid range 1-7 to 0 (unassigned)
                s = slice & 0x0F;
                if (s > 7) s = 0;
                slice = slice >> 4;
                ++slot_count[s];
                mst_alloc[s] = mst_alloc[s] | (1ULL << slot);
                ++offset;
                ++slot;
            }
        }
        printf("Slot allocation:\n");
        for (s = 0; s < 4; ++s)
        {
            // If stream s was assigned a valid VC ID
            if (stream_id[s])
            {
                printf("stream%u, VC payload ID %u, %u slots: 0x%016llx\n", s, stream_id[s], slot_count[stream_id[s]], mst_alloc[stream_id[s]]);
                slot_count[stream_id[s]] = 0; // 0 to mark that this was processed
            }
        }
        if (slot_count[0])
        {
            printf("unallocated, %u slots: 0x%016llx\n", slot_count[0], mst_alloc[0]);
        }
        // Catch all remaining payload IDs in the VCP table that were not linked to a stream
        for (s = 1; s <8; ++s)
        {
            if (slot_count[s])
            {
                printf("unmapped VC payload ID %u, %u slots: 0x%016llx\n", s, slot_count[s], mst_alloc[s]);
            }
        }
    }

#if BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_HDCP1
    if (IORD(base_addr, DPRX_REG_HDCP1_STATUS) & 0x80000)
        printf("HDCP 1.3 decoder authenticated\n");
    else
        printf("HDCP 1.3 decoder not authenticated\n");
#endif
#if BITEC_DP_0_AV_RX_CONTROL_BITEC_CFG_RX_SUPPORT_HDCP2
    if (IORD(base_addr, DPRX_REG_HDCP2_STATUS) & 0x80000)
        printf("HDCP 2.2 decoder authenticated\n");
    else
        printf("HDCP 2.2 decoder not authenticated\n");
#endif
    ber_cntrl = IORD(base_addr, DPRX_REG_BER_CONTROL);
    IOWR(base_addr, DPRX_REG_BER_CONTROL, ber_cntrl | 0xF0000);  // Reset BER counters
#if DP_SUPPORT_RX_FEC
    unsigned fec_cntrl;
    fec_cntrl = IORD(base_addr, DPRX_REG_FEC_ERR_CNF);
    printf("FEC Configuration DPCD : %4.4X\n", fec_cntrl & 0xffff);
    if ((fec_cntrl & 0x00000007) == 0x0)
        printf("FEC Error Count Select : FEC_ERROR_COUNT_DIS\n");
    else if ((fec_cntrl & 0x00000007) == 0x1)
        printf("FEC Error Count Select : UNCORRECTED_BLOCK_ERROR_COUNT\n");
    else if ((fec_cntrl & 0x00000007) == 0x2)
        printf("FEC Error Count Select : CORRECTED_BLOCK_ERROR_COUNT\n");
    else if ((fec_cntrl & 0x00000007) == 0x3)
        printf("FEC Error Count Select : BIT_ERROR_COUNT\n");
    else if ((fec_cntrl & 0x00000007) == 0x4)
        printf("FEC Error Count Select : PARITY_BLOCK_ERROR_COUNT\n");
    else if ((fec_cntrl & 0x00000007) == 0x5)
        printf("FEC Error Count Select : PARITY_BIT_ERROR_COUNT\n");
    else
        printf("FEC Error Count Select :  RESERVED\n");

    printf("FEC Error Count Lane Select : %x\n", (fec_cntrl & 0x00000018) >> 3);

    printf("FEC_EN Detected        : %x\n", (IORD(base_addr, DPRX_REG_RX_STATUS) & 0x40000) >> 18);
    printf("FEC_DIS Detected       : %x\n", (IORD(base_addr, DPRX_REG_RX_STATUS) & 0x80000) >> 19);
    printf("FEC Error Count        : %4.4X\n", IORD(base_addr, DPRX_REG_FEC_ERR_CNT) & 0xffff);
#endif
}

#endif /* DP_SUPPORT_RX */

void bitec_dp_dump_source_msa(unsigned int base_addr)
{
    unsigned int vfreq0_dec = 0, vfreq1_dec = 0;
    BYTE ch_coding;
    ch_coding = (IORD(base_addr, DPTX_REG_TX_CONTROL)) >> 10 & 0x03;

    printf("------------------------------------------\n");
    printf("------   TX Main stream attributes  ------\n");
    printf("------------------------------------------\n");
    if (ch_coding == 0x2)
        printf("--- Channel Coding : 128b132b\n");
    else
        printf("--- Channel Coding : 8b10b\n");
    printf("------------------------------------------\n");
    printf("--- Stream 0 ---\n");
    printf("MSA lock : %1.1X\n", (IORD(base_addr, DPTX0_REG_VBID) >> 7) & 1);
    printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n", IORD(base_addr, DPTX0_REG_VBID) & 0x7F,
           IORD(base_addr, DPTX0_REG_MSA_MISC0), IORD(base_addr, DPTX0_REG_MSA_MISC1));
           
    if (ch_coding == 0x2)
    {
        vfreq0_dec = IORD(base_addr, DPTX0_REG_MSA_MVID);
        vfreq0_dec &= 0x00FFFFFF;
        vfreq0_dec |= IORD(base_addr, DPTX0_REG_MSA_NVID) << 24;
        printf("Vfreq  : %X (%d Hz) \n", vfreq0_dec, vfreq0_dec);
    }
    else
        printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPTX0_REG_MSA_MVID),
               IORD(base_addr, DPTX0_REG_MSA_NVID));
    printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HTOTAL),
           IORD(base_addr, DPTX0_REG_MSA_VTOTAL));
    printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HSP),
           IORD(base_addr, DPTX0_REG_MSA_HSW));
    printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HSTART),
           IORD(base_addr, DPTX0_REG_MSA_VSTART));
    printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_VSP),
           IORD(base_addr, DPTX0_REG_MSA_VSW));
    printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPTX0_REG_MSA_HWIDTH),
           IORD(base_addr, DPTX0_REG_MSA_VHEIGHT));
    printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPTX0_REG_CRC_R),
           IORD(base_addr, DPTX0_REG_CRC_G), IORD(base_addr, DPTX0_REG_CRC_B));
    printf("--- Stream 1 ---\n");
    printf("MSA lock : %1.1X\n", (IORD(base_addr, DPTX1_REG_VBID) >> 7) & 1);
    printf("VB-ID : %2.2X  MISC0 : %2.2X  MISC1 : %2.2X\n", IORD(base_addr, DPTX1_REG_VBID) & 0x7F,
           IORD(base_addr, DPTX1_REG_MSA_MISC0), IORD(base_addr, DPTX1_REG_MSA_MISC1));
    if (ch_coding == 0x2)
    {
        vfreq1_dec = IORD(base_addr, DPTX0_REG_MSA_MVID);
        vfreq1_dec &= 0x00FFFFFF;
        vfreq1_dec |= IORD(base_addr, DPTX0_REG_MSA_NVID) << 24;
        printf("Vfreq  : %X (%d Hz) \n", vfreq1_dec, vfreq1_dec);
    }
    else
        printf("Mvid   : %4.4X     Nvid    : %4.4X\n", IORD(base_addr, DPTX1_REG_MSA_MVID),
               IORD(base_addr, DPTX1_REG_MSA_NVID));
    printf("Htotal : %4.4d     Vtotal  : %4.4d\n", IORD(base_addr, DPTX1_REG_MSA_HTOTAL),
           IORD(base_addr, DPTX1_REG_MSA_VTOTAL));
    printf("HSP    : %4.4d     HSW     : %4.4d\n", IORD(base_addr, DPTX1_REG_MSA_HSP),
           IORD(base_addr, DPTX1_REG_MSA_HSW));
    printf("Hstart : %4.4d     Vstart  : %4.4d\n", IORD(base_addr, DPTX1_REG_MSA_HSTART),
           IORD(base_addr, DPTX1_REG_MSA_VSTART));
    printf("VSP    : %4.4d     VSW     : %4.4d\n", IORD(base_addr, DPTX1_REG_MSA_VSP),
           IORD(base_addr, DPTX1_REG_MSA_VSW));
    printf("Hwidth : %4.4d     Vheight : %4.4d\n", IORD(base_addr, DPTX1_REG_MSA_HWIDTH),
           IORD(base_addr, DPTX1_REG_MSA_VHEIGHT));
    printf("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", IORD(base_addr, DPTX1_REG_CRC_R),
           IORD(base_addr, DPTX1_REG_CRC_G), IORD(base_addr, DPTX1_REG_CRC_B));
}

void bitec_dp_dump_source_config(unsigned int base_addr)
{
    unsigned int ch_coding;
    unsigned int link_rate;
    unsigned int mst_control1;
    BYTE stream_id[4];            // Mapping between stream and assigned payload IDs (valid payload ID are in the range 1-7)
    uint64_t mst_alloc[8];        // Allocation for payload IDs (ID 0==unassigned slots)
    BYTE slot_count[8];           // Slot count for payload IDs (ID 0==number of unassigned slots)
    BYTE idx, offset, s, slot;
    unsigned int slice;
        
    ch_coding = (IORD(base_addr, DPTX_REG_TX_CONTROL) >> 10) & 0x03;
    mst_control1 = IORD(base_addr, DPTX_REG_MST_CONTROL1);

    printf("------------------------------------------\n");
    printf("--------   TX Link configuration   -------\n");
    printf("------------------------------------------\n");
    printf("Lane count : %d\n", (IORD(base_addr, DPTX_REG_TX_CONTROL) >> 5) & 0x1f);

    if (ch_coding == 0x2)
    {
        printf("Channel Coding : 128b132b\n");
        link_rate = (IORD(base_addr, DPTX_REG_TX_CONTROL)) >> 21 & 0xFF;
        switch (link_rate)
        {
        case 0x1:
            printf("Link rate  : 10 Gbps\n");
            break;
        case 0x2:
            printf("Link rate  : 20 Gbps\n");
            break;
        case 0x4:
            printf("Link rate  : 13.5 Gbps\n");
            break;
        default:
            printf("Link rate  : 0 Gbps\n");
            break;
        }
    }
    else
    {
        printf("Channel Coding : 8b10b\n");
        printf("Link rate  : %d Mbps\n",
               ((IORD(base_addr, DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);
        if (mst_control1 & 0x01)
            printf("MST : on\n");
        else
            printf("MST : off\n");
    }
    
    // Check slot allocation
    if ((ch_coding == 0x2) || (mst_control1 & 0x01))
    {
        // Zero the allocation for each payload ID
        for (s = 0; s < 8; ++s)
        {
            slot_count[s] = 0;
            mst_alloc[s]  = 0ULL;
        }
        // Pull the mapping stream ID -> payload ID from DPTX_REG_MST_CONTROL1
        for (s = 0; s < 4; ++s)
        {
            stream_id[s] = (mst_control1 >> (4 * (s+1))) & 0x0F;
            // A value of 0xF is used to indicate an unasigned stream, map all values that are out of the valid range 1-7 to 0 (unassigned)
            if (stream_id[s] > 7) stream_id[s] = 0;
        }
        slot = 0;
        for (idx = 0; idx < 8; ++idx)
        {
            slice = IORD(base_addr, DPTX_REG_MST_VCPTAB0 + idx);
            offset = 0;
            // Skip the very first slot in HBR rates with MST enabled
            if ((ch_coding != 0x2) && (idx == 0))
            {
                offset = 1;
                slice = slice >> 4;
                ++slot;
            }
            while (offset < 8)
            {
                // map all values that are out of the valid range 1-7 to 0 (unassigned)
                s = slice & 0x0F;
                if (s > 7) s = 0;
                slice = slice >> 4;
                ++slot_count[s];
                mst_alloc[s] = mst_alloc[s] | (1ULL << slot);
                ++offset;
                ++slot;
            }
        }
        printf("Slot allocation:\n");
        for (s = 0; s < 4; ++s)
        {
            // If stream s was assigned a valid VC ID 
            if (stream_id[s])
            {
                printf("stream%u, VC payload ID %u, %u slots: 0x%016llx\n", s, stream_id[s], slot_count[stream_id[s]], mst_alloc[stream_id[s]]);
                slot_count[stream_id[s]] = 0; // 0 to mark that this was processed
            }
        }
        if (slot_count[0])
        {
            printf("unallocated, %u slots: 0x%016llx\n", slot_count[0], mst_alloc[0]);
        }
        // Catch all remaining payload IDs in the VCP table that were not linked to a stream
        for (s = 1; s < 8; ++s)
        {
            if (slot_count[s])
            {
                printf("unmapped VC payload ID %u, %u slots: 0x%016llx\n", s, slot_count[s], mst_alloc[s]);
            }
        }
    }

#if BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP1
    if (IORD(base_addr, DPTX_REG_HDCP1_STATUS) & 0x80000)
        printf("HDCP 1.3 encoder authenticated\n");
    else
        printf("HDCP 1.3 encoder not authenticated\n");
#endif
#if BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP2
    if (IORD(base_addr, DPTX_REG_HDCP2_STATUS) & 0x80000)
        printf("HDCP 2.2 encoder authenticated\n");
    else
        printf("HDCP 2.2 encoder not authenticated\n");
#endif
}

char* bitec_get_stdin()
{
    int d, i;
    char* c = (char*)&d;

    i = read(0, (void*)&d, 1);  // 0 = stdin
    if ((i < 1) || (*c == EOF))
        return NULL;

    printf("%c", d);
    if (*c == '\n' || *c == 0xd)
    {
        // input is complete
        if (*c == 0xd)
            printf("\n");
        btc_stdbuf[btc_stdbuf_ptr] = 0x00;
        btc_stdbuf_ptr = 0;
        return btc_stdbuf;
    }

    btc_stdbuf[btc_stdbuf_ptr++] = *c;
    if ((btc_stdbuf_ptr + 1) == sizeof(btc_stdbuf))
    {
        // buffer is full
        btc_stdbuf[btc_stdbuf_ptr] = 0x00;
        btc_stdbuf_ptr = 0;
        return btc_stdbuf;
    }

    return NULL;
}

#endif  // BITEC_STATUS_DEBUG
