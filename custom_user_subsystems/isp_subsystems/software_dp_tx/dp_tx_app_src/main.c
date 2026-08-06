/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "btc_dptx_syslib.h"
#include "btc_dptxll_syslib.h"
#include "config.h"
#include "debug.h"
#include "intel_fpga_i2c.h"
#include "aux_decoder.h"
#include "tx_utils.h"
#include "board.h"
#include "dptx_app_defs.h"
#include "dptx_formats.h"


#define AXI_OFFSET 0x300
#include "intel_axi2cv.h"

#undef DEBUG
#ifdef DEBUG
#define DPTX_PRINT(...)  printf(__VA_ARGS__)
#else
#define DPTX_PRINT(...)
#undef BITEC_STATUS_DEBUG
#endif /* DEBUG */

// Get the core capabilities (defined in QSYS and ported to system.h)
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LANE_COUNT


void bitec_menu_print();
void bitec_menu_cmd();

// Externally defined data and variables

// Defined in tx_utils.c
extern int new_rx;
extern BYTE tx_edid_data[512];  // Sink EDID

// Global data and state variables

intel_axi2cv_instance axi2cv;
int res_switch = 1;
uint32_t dp_tx_status = 0;
uint32_t output_format = 0x0;
uint32_t output_format_override = 0x0;
bool bpc10_support = false;

void update_supported_formats_from_dmt(const uint8_t* dmt)
{
    // Ignore descriptor if not DTD
    if((dmt[0] == 0x01) && (dmt[1] == 0x01))
        return;

    if(dmt[0] != 0x00)
    {
        const uint32_t width = (dmt[0] + 31) * 8;
        const uint32_t frame_rate = ((dmt[1] & 0x3F) + 60) * 100;

        uint32_t num = 16;
        uint32_t denom = 9;

        switch((dmt[1] >> 6) & 0x03)
        {
            case 0: // 16:10
                num = 16;
                denom = 10;
                break;
            case 1: // 4:3
                num = 4;
                denom = 3;
                break;
            case 2: // 5:4
                num = 5;
                denom = 4;
                break;
            case 3:
            default:
                break;
        }

        const uint32_t height = (width * denom) / num;
        
        DPTX_PRINT("# DMT: %" PRIu32 "x%" PRIu32 " @ %" PRIu32 " Hz\n", width, height, frame_rate);

        dptx_formats_set_supported(width, height, frame_rate);
    }
}

// Parse EDID DTD and update the list of supported formats
void update_supported_formats_from_dtd(const uint8_t* dtd)
{
    // Ignore descriptor if not DTD
    if((dtd[0] == 0x0) && (dtd[1] == 0x00))
        return;

    uint32_t pixel_clock = (dtd[0] | (dtd[1] << 8)) * 10000;

    uint32_t width = dtd[2];
    uint32_t h_blanking = dtd[3];
    width = width | ((dtd[4] & 0xf0) << 4);
    h_blanking = h_blanking | ((dtd[4] & 0x0f) << 8);

    uint32_t height = dtd[5];
    uint32_t v_blanking = dtd[6];
    height = height | ((dtd[7] & 0xf0) << 4);
    v_blanking = v_blanking | ((dtd[7] & 0x0f) << 8);

    uint32_t total_pixels = (width + h_blanking) * (height + v_blanking);
    uint32_t frame_rate_frac = ((pixel_clock % total_pixels) * 100) / total_pixels;
    // Frame rate x100 Hz
    uint32_t frame_rate = ((pixel_clock / total_pixels) * 100) + frame_rate_frac;

    DPTX_PRINT("# DTD: %" PRIu32 "x%" PRIu32 " @ %.2f Hz\n", width, height, frame_rate / 100.0);

    // Check and update the app supported formats
    dptx_formats_set_supported(width, height, frame_rate);
}

void parse_displayid_type_i_timing(const uint8_t *data)
{
	uint32_t pixclk_khz = 10 * (1 + (data[0] + (data[1] << 8) + (data[2] << 16)));

	uint32_t width = 1 + (data[4] | (data[5] << 8));
	uint32_t hbl = 1 + (data[6] | (data[7] << 8));
	uint32_t hfp = 1 + (data[8] | ((data[9] & 0x7f) << 8));
	uint32_t hsync = 1 + (data[10] | (data[11] << 8));
	uint32_t hbp = hbl - hfp - hsync;

	uint32_t height = 1 + (data[12] | (data[13] << 8));
	uint32_t vbl = 1 + (data[14] | (data[15] << 8));

	uint32_t pixel_clock = pixclk_khz * 1000;
	uint32_t total_pixels = (width + hbl) * (height + vbl);

	// Frame rate x100 Hz
	uint32_t frame_rate_frac = ((pixel_clock % total_pixels) * 100) / total_pixels;
    uint32_t frame_rate = ((pixel_clock / total_pixels) * 100) + frame_rate_frac;

	DPTX_PRINT("# DTD: %" PRIu32 "x%" PRIu32 " @ %.2f Hz\n", width, height, frame_rate / 100.0f);

	dptx_formats_set_supported(width, height, frame_rate);
}

uint8_t process_displayid_data_block(uint32_t version, const uint8_t* data, uint8_t length)
{
	static const uint8_t DETAILED_TIMING_TYPE_I_LEN = 20;
	uint8_t tag = data[0];
	uint8_t data_block_len = (length < 3) ? 0 : data[2];
	data += 3; // Skip header (3 bytes)

    switch (tag) {
        case 0x03:
            for(uint32_t i = 0; i < data_block_len / DETAILED_TIMING_TYPE_I_LEN; i++){
                parse_displayid_type_i_timing(data);
				data += DETAILED_TIMING_TYPE_I_LEN;
			}
            break;
        default:
        break;        
    };

	return data_block_len + 3;
}

void process_displayid_extension_block(const uint8_t* data)
{
	uint8_t version = data[1];
	uint8_t length = data[2];

	DPTX_PRINT("DisplayID extension block Version: %u.%u Length: %u\n",
	       version >> 4, version & 0xf, length);

	const uint8_t* data_block = data + 5;

	while(length > 0) {
		uint8_t data_block_len = process_displayid_data_block(version, data_block, length);
		data_block += data_block_len;
		length -= data_block_len;
	}    
}

// Process sink EDID and upate list of supported video formats
void process_sink_edid()
{
    // Check basic display parameters for 10bpc support
    if(tx_edid_data[EDID_INDEX_BDP_START] & EDID_INDEX_BDP_DIGITAL_INPUT)
        bpc10_support = ((tx_edid_data[EDID_INDEX_BDP_START] & EDID_INDEX_BDP_BIT_DEPTH_MASK) == EDID_INDEX_BDP_BIT_DEPTH_10);

    //-- Process standard timings
    for(uint32_t i = EDID_INDEX_STANDARD_TIMING_START; i < EDID_INDEX_STANDARD_TIMING_END; i += 2)
    {        
        update_supported_formats_from_dmt(&tx_edid_data[i]);
    }        

    //-- Process DTDs in the main block
    for (uint8_t startOfDescriptor = EDID_INDEX_DESCRIPTOR1_START;
        startOfDescriptor <= EDID_INDEX_DESCRIPTOR4_START;
        startOfDescriptor += EDID_INDEX_DESCRIPTOR_SIZE)
    {
        update_supported_formats_from_dtd(&tx_edid_data[startOfDescriptor]);
    }

    //-- Does the EDID have an extension block?
    uint8_t num_ext_blocks = tx_edid_data[EDID_INDEX_EXT_BLOCK_COUNT];

    //-- If present, check the checksum on the extension block
    for(uint8_t i = 0; i < num_ext_blocks; ++i)
    {
        const unsigned char* extBlockData = tx_edid_data + EDID_BLOCK_SIZE * (i + 1);
        
        printf("EXTENSION TYPE: %d - 0x%02x\n", extBlockData[0], extBlockData[0]);

        if(extBlockData[EDID_INDEX_EXT_TAG] == 0x02) // CEA extension
        {
            uint32_t start_of_DTD = extBlockData[EDID_INDEX_EXT_DTD_START];

            //-- DTD must start within the EDID block
            if (start_of_DTD < EDID_BLOCK_SIZE)
            {
                //-- Loop through the Data Blocks but check that start of DTD indicates having space for
                //them
                if (start_of_DTD > EDID_INDEX_EXT_DB_START)
                {
                    //-
                    //- Detailed timing descriptors
                    //-
                    const uint8_t* dtd = extBlockData + start_of_DTD;

                    while(dtd[0] != 0x0 || dtd[1] != 0x0)
                    {
                        update_supported_formats_from_dtd(dtd);
                        dtd += EDID_DTD_SIZE;
                    }
                }
            }
            else
            {
                DPTX_PRINT("EDID: Invalid DTD start\n");
            }

            uint32_t dataBlockStart = EDID_INDEX_EXT_DB_START;

            //-- Must stay within the region below the DTD and not go outside the EDID block
            while ((dataBlockStart < start_of_DTD) && (dataBlockStart < EDID_BLOCK_SIZE))
            {
                //-- First byte in a Data Block encodes type and length
                //--      bit 7..5: Block Type Tag (1 is audio, 2 is video, 3 is vendor specific, 4 is speaker
                //--                allocation, all other values Reserved)
                //--      bit 4..0: Total number of bytes in this block following this byte
                uint8_t dataBlockType = (extBlockData[dataBlockStart] & 0xE0) >> 5;
                uint8_t dataBlockLength = (extBlockData[dataBlockStart] & 0x1F);  //-- Max length of 31bytes

                //-- Sanity check on the indicated block length
                if ((dataBlockStart + dataBlockLength) < EDID_BLOCK_SIZE)
                {
                    if (dataBlockType == EDID_VIDEO_DATA_BLOCK)
                    {
                        //-- Loop through looking for the VIC codes of interest.
                        //-- Note that we only loop over the indicated block length and this
                        //-- has been verified above.
                        for (int byteLoop = 1; byteLoop <= dataBlockLength; byteLoop++)
                        {
                            uint8_t vic = extBlockData[dataBlockStart + byteLoop] & 0x7F;
                            dptx_formats_set_supported_by_vic(vic);
                        }
                    }

                    if ((dataBlockType == EDID_VENDOR_SPECIFIC_DATA_BLOCK) &&
                        (dataBlockLength >= EDID_INDEX_EXT_VSDB_MIN_LENGTH))
                    {
                        //-- Look for VSDB data block that defines colour depth
                        if ((extBlockData[dataBlockStart + 1] == 0x03) &&
                            (extBlockData[dataBlockStart + 2] == 0x0C) &&
                            (extBlockData[dataBlockStart + 3] == 0x00))
                        {
                            //-- Is 10 bpc supported?
                            if ((extBlockData[dataBlockStart + 6] & 0x70) > 0)
                            {
                                bpc10_support |= true;
                            }
                        }
                    }

                    //----------------------------------------------------------------------------------------------------
                    //-- Already checked that adding the block length (+1) doesn't take us outside the
                    //EDID block
                    //-- Add one to pointer as the block length value does not include the first byte
                    //(header)
                    dataBlockStart += dataBlockLength;
                    dataBlockStart += 1;                                        
                }
                else
                {
                    DPTX_PRINT("EDID: Invalid extended block length\n");
                }
            }
        }
        else if(extBlockData[EDID_INDEX_EXT_TAG] == 0x70) // DisplayID extension
        {
            process_displayid_extension_block(extBlockData);
        }        
    }    
}


void program_cvo_timing(intel_axi2cv_instance* cvo, const cvo_timing_info_t* t)
{
    if(cvo && t)
    {
        intel_axi2cv_set_output_mode(cvo, 0,
            t->interlaced,
            t->sample_count,
            t->f0_line_count,
            t->f1_line_count,
            t->h_front_porch,
            t->h_sync_length,
            t->h_blanking,
            t->v_front_porch,
            t->v_sync_length,
            t->v_blanking,
            t->f0_v_front_porch,
            t->f0_v_sync_length,
            t->f0_v_blanking,
            t->active_picture_line,
            t->f0_v_rising,
            t->field_rising,
            t->field_falling,
            t->h_sync_polarity,
            t->v_sync_polarity
        );
    }
}


uint32_t get_tx_link_rate()
{
    return (((IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_CONTROL) >> 21) & 0xff) * 270);
}



//==================================================================
// A few global variables used to hanlde menu actions
//==================================================================

#if BITEC_TX_AUX_DEBUG
char auxTxDebugEnable;
static AuxDecoderInstance _gTxAuxInstance;
#endif

unsigned int lt_tx_link_rate;
unsigned int lt_tx_lane_count;


//==================================================================
// Main Program
//==================================================================
int main()
{
    // Enable non-blocking jtag uart
    int res = 0;
    res = fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    res = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    if(res == -1)
    {
        DPTX_PRINT("FCNTL Failed\n");
    }

    DPTX_PRINT("Started...\n");

    // Perform Devkit Specific initialisations
    board_configure();

    // Init Bitec DP system library

    btc_dptx_syslib_add_tx(0, DP_TX_DP_SOURCE_BASE, DP_TX_DP_SOURCE_IRQ_INTERRUPT_CONTROLLER_ID,
                           DP_TX_DP_SOURCE_IRQ);
    btc_dptx_syslib_init();

    intel_vab_core_base axi2cv_addr_base = (intel_vab_core_base)(DP_TX_DP_SOURCE_BASE + (AXI_OFFSET << 2));

    res = intel_axi2cv_init(&axi2cv, axi2cv_addr_base);

    if(res)
    {
        DPTX_PRINT("Error initialising AXI2CV\nExiting...");
        return res;
    }

    // Init the source
    bitec_dptx_init();

#if BITEC_TX_AUX_DEBUG
    dp_dump_aux_debug_init(&_gTxAuxInstance, DP_TX_AUX_TX_DEBUG_FIFO_IN_CSR_BASE, DP_TX_AUX_TX_DEBUG_FIFO_OUT_BASE, false);
    auxTxDebugEnable=0;
#endif

    lt_tx_link_rate = TX_MAX_LINK_RATE;
    lt_tx_lane_count = TX_MAX_LANE_COUNT;

    // Check if a Sink is readily connected

    unsigned int sr = IORD(btc_dptx_baseaddr(0), DPTX_REG_TX_STATUS);  // Reading SR clears IRQ

    if (sr & 0x04)
    {
#if BITEC_TX_CAPAB_MST
        btc_dptxll_hpd_change(0, 1);
        pc_fsm = PC_FSM_START;
#else
        btc_dptx_hpd_change(0, 1);
        bitec_dptx_linktrain();
#endif
    }

    BTC_DPTX_ENABLE_HPD_IRQ(0);  // Enable IRQ on HPD changes from the sink

    bitec_menu_print();

    new_rx = 1;

    // Main loop
    while(1)
    {
        // Serve Syslib periodic tasks
        btc_dptx_syslib_monitor();

#if BITEC_TX_CAPAB_MST
        btc_dptxll_syslib_monitor();
        // Simulate the user MST TX application
        bitec_dptx_pc();
#endif

#if BITEC_TX_AUX_DEBUG
        if (auxTxDebugEnable&1)
            dp_dump_aux_debug(&_gTxAuxInstance);
#endif
        // Serve menu commands, if any
        bitec_menu_cmd();

        // Check current Tx status and update status register
        uint32_t dp_tx_vid_ena_1b    = IORD(DP_TX_DP_SOURCE_BASE, 0x350) & 0x1; 
        uint32_t dp_tx_vid_match_1b  = IORD(DP_TX_DP_SOURCE_BASE, 0x351) & 0x1; 
        uint32_t dp_tx_vid_valid_1b  = IORD(DP_TX_DP_SOURCE_BASE, 0x36D) & 0x1; 
    
        uint32_t dp_tx_status_current =  dp_tx_vid_ena_1b   << 2 |
                                            dp_tx_vid_match_1b << 1 |
                                            dp_tx_vid_valid_1b;

        if(dp_tx_status != dp_tx_status_current)
        {
            dp_tx_status = dp_tx_status_current;
            IOWR(DP_TX_PIO_STATUS_BASE, 0, dp_tx_status);
        }

        // Handle new sink here
        if(new_rx)
        {
            new_rx = 0;

            dptx_formats_clear_supported();
            bpc10_support = false;

            process_sink_edid();

            // Disable QHD for now. Need correct timing data
            dptx_formats_clear_supported_n(4);
            dptx_formats_clear_supported_n(4);
            
#ifdef DEBUG            
            dptx_formats_print();
#endif /* DEBUG */            
            DPTX_PRINT("10bpc: %s\n", bpc10_support ? "Yes" : "No");

            // Update supported formats register
            uint32_t reg_val = 0x0;

            const uint32_t num_supported_formats = dptx_min(dptx_formats_len(), MAX_SUPPORTED_FORMATS);

            for(uint32_t i = 0; i < num_supported_formats; ++i)
            {
                uint32_t v = (dptx_formats_is_supported(i) ? 1 : 0);
                reg_val |= (v << i);
            }

            if(bpc10_support)
                reg_val |= FORMATS_REG_10BPC;
        
            IOWR(DP_TX_PIO_SUPPORTD_FMATS_BASE, 0, reg_val);

            res_switch = 1;
        }

        // Check for output format override request
        uint32_t output_format_override_new = IORD(DP_TX_PIO_FORMAT_OVRRIDE_BASE, 0);

        // ToDo: validate output_format_override_new!
        if(output_format_override != output_format_override_new)
        {
            DPTX_PRINT("Format override: %" PRIx32 "\n", output_format_override_new);
            output_format_override = output_format_override_new;
            res_switch = 1;
        }

		if (res_switch == 1)
		{
            const video_format_t* format = NULL;
            uint32_t output_format_new = 0x0;

            // Use requested output format if provided
            // otherwise pick the first supported from the list
            uint32_t override_val = output_format_override & FORMATS_REG_MASK;

            if(override_val)
            {
                uint32_t v = override_val;
                uint32_t idx = 0;

                while((v & 0x1) == 0)
                {
                    ++idx;
                    v = v >> 1;
                }

                const video_format_t* f = dptx_formats_get(idx);
                
                if(f && f->supported)
                {
                    format = f;
                    output_format_new = (0x1 << idx);

                    if(bpc10_support && (output_format_override & FORMATS_REG_10BPC))
                        output_format_new |= FORMATS_REG_10BPC;
                }
            }
            else
            {
                for(uint32_t idx = 0; idx < dptx_formats_len(); ++idx)
                {
                    const video_format_t* f = dptx_formats_get(idx);

                    if(f && f->supported)
                    {
                        format = f;
                        output_format_new = (0x1 << idx);

                        if(bpc10_support)
                            output_format_new |= FORMATS_REG_10BPC;

                        break;
                    }
                }
            }

            if(!format)
            {
                uint32_t idx = dptx_formats_get_fallback_idx();
                format = dptx_formats_get(idx);
                output_format_new = (0x1 << idx);
            }

            if(format)
            {
                DPTX_PRINT("New format: %s\n", format->str);
                program_cvo_timing(&axi2cv, &format->timing);
                board_tx_freq(format->tx_clk);

                /*
                    If the sink reports 10bpc support we also need to check
                    if the current link rate is highe enough to produce stable
                    10 bpc output
                */

                /* Default to 8 bpc */
                uint8_t bpc = 1;

                if(output_format_new & FORMATS_REG_10BPC)
                {
                    const cvo_timing_info_t* timing = &format->timing;

                    if(timing)
                    {
                        if((timing->sample_count >= 3840) && (timing->f0_line_count >= 2160) && (format->fps >= 6000))
                        {
                            const uint32_t link_rate = get_tx_link_rate();

                            if(link_rate >= 8100)
                                bpc = 2; /* 10 bpc output is possible */
                            else
                                output_format_new &= ~(FORMATS_REG_10BPC); // Fall back to 8bpc
                        }
                    }
                }

                btc_dptx_set_color_space(0, 0, bpc, 0, 0, 0);
            }

            if(output_format != output_format_new)
            {
                output_format = output_format_new;
                IOWR(DP_TX_PIO_CURR_FORMAT_BASE, 0, output_format);

                DPTX_PRINT("New format: 0x%08lx\n", output_format);
            }               

			res_switch = 0;           
		}        
    }

    return 0;  // Should never get here
}


//==================================================================
// Printout all menu commands
//==================================================================
void bitec_menu_print()
{
#if BITEC_STATUS_DEBUG
    DPTX_PRINT("h = Help\n");
    DPTX_PRINT("s  = Status\n");
    DPTX_PRINT("v  = Print versions\n");
    DPTX_PRINT("c  = Read Sink DPCD CRC\n");
    DPTX_PRINT("t+ = Increase TX max link rate (requires retrain)\n");
    DPTX_PRINT("t- = Decrease TX max link rate (requires retrain)\n");
    DPTX_PRINT("x+ = Increase TX max num lanes (requires retrain)\n");
    DPTX_PRINT("x- = Decrease TX max num lanes (requires retrain)\n");
    DPTX_PRINT("y  = TX retrain\n");
#if BITEC_TX_AUX_DEBUG
    DPTX_PRINT("a = Enable/Disable Tx AUX Log\n");
#endif
#endif // BITEC_STATUS_DEBUG
}

//==================================================================
// Implementation of menu commands
//==================================================================
void bitec_menu_cmd()
{
#if BITEC_STATUS_DEBUG
    char* cmd;

    cmd = bitec_get_stdin();
    if (cmd != NULL)
    {
        switch (cmd[0])
        {
        // Status, dump MSA and config
        case 's':
            bitec_dp_dump_source_msa(btc_dptx_baseaddr(0));
            bitec_dp_dump_source_config(btc_dptx_baseaddr(0));
            break;
        // Version
        case 'v':
            BYTE maj, min;
            unsigned int ver;
            // Print versions
            btc_dptx_sw_ver(&maj, &min, &ver);
            btc_dptx_rtl_ver(&maj, &min, &ver);
            DPTX_PRINT("TX SW library ver. %u.%u.%u\n", maj, min, ver);
            DPTX_PRINT("TX RTL core ver. %u.%u.%u\n", maj, min, ver);
            break;
        // Help (or HDCP)
        case 'h':
#if BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP1 || \
    BITEC_DP_0_AV_TX_CONTROL_BITEC_CFG_TX_SUPPORT_HDCP2
                switch (cmd[1])
                {
                case '\0':
                    bitec_menu_print();
                    break;
                default:
                    // HDCP to be implemented
                }
#else
                bitec_menu_print();
#endif
            break;
        // Change TX max num lanes
        case 'x':
            if ((cmd[1] == '+') && (lt_tx_lane_count != TX_MAX_LANE_COUNT))
            {
                lt_tx_lane_count = lt_tx_lane_count * 2;
            }
            else if ((cmd[1] == '-') && (lt_tx_lane_count != 1))
            {
                lt_tx_lane_count = lt_tx_lane_count / 2;
            }
            DPTX_PRINT("Set TX max lane count to %d\n", lt_tx_lane_count);
            break;
        // Change TX max link rate
        case 't':
            if ((cmd[1] == '+') && (lt_tx_link_rate != TX_MAX_LINK_RATE))
            {
                switch (lt_tx_link_rate)
                {
                case 0x06: lt_tx_link_rate = 0x0A; break;
                case 0x0A: lt_tx_link_rate = 0x14; break;
                case 0x14: lt_tx_link_rate = 0x1E; break;
                case 0x1E: lt_tx_link_rate = 0x01; break;
                case 0x01: lt_tx_link_rate = 0x04; break;
                case 0x04: lt_tx_link_rate = 0x02; break;
                }
            }
            else if (cmd[1] == '-')
            {
                switch (lt_tx_link_rate)
                {
                case 0x0A: lt_tx_link_rate = 0x06; break;
                case 0x14: lt_tx_link_rate = 0x0A; break;
                case 0x1E: lt_tx_link_rate = 0x14; break;
                case 0x01: lt_tx_link_rate = 0x1E; break;
                case 0x04: lt_tx_link_rate = 0x01; break;
                case 0x02: lt_tx_link_rate = 0x04; break;
                }
            }
            switch (lt_tx_link_rate)
            {
            case 0x06: DPTX_PRINT("Set TX max link rate to 1.62Gbps\n"); break;
            case 0x0A: DPTX_PRINT("Set TX max link rate to 2.7Gbps\n"); break;
            case 0x14: DPTX_PRINT("Set TX max link rate to 5.4Gbps\n"); break;
            case 0x1E: DPTX_PRINT("Set TX max link rate to 8.1Gbps\n"); break;
            case 0x01: DPTX_PRINT("Set TX max link rate to 10Gbps\n"); break;
            case 0x04: DPTX_PRINT("Set TX max link rate to 13.5Gbps\n"); break;
            case 0x02: DPTX_PRINT("Set TX max link rate to 20Gbps\n"); break;
            }
            break;
        // Restart TX link training
        case 'y':
#if BITEC_TX_CAPAB_MST
            btc_dptxll_syslib_set_max_link_rate(0, lt_tx_link_rate);
            btc_dptxll_syslib_set_max_lane_count(0, lt_tx_lane_count);
            btc_dptxll_hpd_change(0, 0);    // HPD = 0
            // Wait for 150+ ms to have a long enough HPD
            {
                alt_timestamp_start();
                unsigned int timeout = alt_timestamp_freq() / 6;
                while (alt_timestamp() < timeout) ; //delay
            }
            btc_dptxll_hpd_change(0, 1);    // HPD = 1
            pc_fsm = PC_FSM_START;
#else
            btc_dptx_hpd_change(0, 1);
            bitec_dptx_linktrain_parameterized(lt_tx_link_rate, lt_tx_lane_count, DP_TX_BPS);
#endif
            break;
        case 'c':
            BYTE d[6];
            // Sink CRC
            d[0] = 1;
            btc_dptx_aux_write(0, DPCD_ADDR_TEST_SINK, 1, d);
            usleep(50000);
            btc_dptx_aux_read(0, DPCD_ADDR_TEST_CRC_R_CR_LSB, 6, d);
            DPTX_PRINT("CRC R : %4.4x  CRC G : %4.4x  CRC B : %4.4x\n", d[0] + (d[1] << 8),
                    d[2] + (d[3] << 8), d[4] + (d[5] << 8));
            d[0] = 0;
            btc_dptx_aux_write(0, DPCD_ADDR_TEST_SINK, 1, d);
            break;
#if BITEC_TX_AUX_DEBUG
        // Enable/disable TX aux debug
        case 'a':
            auxTxDebugEnable++;
            if (auxTxDebugEnable&0x01) DPTX_PRINT ("TX_AUX_DEBUG ENABLED\n");
            else DPTX_PRINT ("TX_AUX_DEBUG DISABLED\n");
            break;
#endif
        } // switch cmd[0]
    } // cmd != NULL
#endif   // BITEC_STATUS_DEBUG
}
