/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <assert.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "intel_vvp_warp.h"
#include "intel_vvp_warp_regs.h"
#include "intel_vvp_warp_private.h"


/**********************************************************************
 *
 *	Driver implementation and helper functions
 *
 *********************************************************************/

/* Round up value v to the nearest multiple of a (Where a is a power of two) */
static uint32_t roundup_pwr_two(uint32_t v, uint32_t a)
{
	return (v + (a - 1)) & ~(a - 1);
}


/* Get the next nearest power of two for the value v */
static uint32_t get_next_pwr_two(uint32_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}


static uint32_t min_val(uint32_t v1, uint32_t v2)
{
	return (v1 < v2 ? v1 : v2);
}


static int check_output_bounce(intel_vvp_warp_channel_t* ch)
{
	int ret = 0;
	
	/* For read/write reg macros */
	intel_vvp_warp_instance_t* instance = ch->instance;
	
	if(instance->output_bounce == EBOUNCE_CHANNEL)
	{
		uint32_t hw_idx = ch->output->idx;
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONFIG);
		reg_val &= INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_MSK;

		if((reg_val == INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_BLOCK) || (reg_val == INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_RASTER))
			ret = 1;		
	}
	else if(instance->output_bounce != EBOUNCE_NONE)
		ret = 1;

	return ret;
}


static uint32_t memctl_input_write_shift(intel_vvp_warp_channel_t* ch, uint32_t idx)
{
    uint32_t input_mx = ch->instance->mipmap_enable ? 2 : 1;
	return (4 + (input_mx * idx));
}


static uint32_t memctl_engine_write_shift(intel_vvp_warp_channel_t* ch, uint32_t idx)
{
	return memctl_input_write_shift(ch, ch->instance->num_inputs) + idx;
}


static uint32_t memctl_output_read_shift(intel_vvp_warp_channel_t* ch, uint32_t idx)
{
	return memctl_engine_write_shift(ch, ch->instance->num_engines) + idx;
}


static uint32_t memctl_engine_read_shift(intel_vvp_warp_channel_t* ch, uint32_t idx)
{	
    uint32_t engine_reads_shift = (check_output_bounce(ch) ? memctl_output_read_shift(ch, ch->instance->num_outputs) : memctl_input_write_shift(ch, ch->instance->num_inputs));
	return (engine_reads_shift + idx * 4); /* 4 read bits per engine */
}


/*
 * Max frame dimensions as per RTL implementation
 */
static uint32_t get_max_frame_dimension(intel_vvp_warp_instance_t* instance)
{
	uint32_t frame_dimension = 0;

	if(instance)
	{
		switch(instance->mem_map)
		{
		case ESDTV:
			frame_dimension = 1024;
			break;
		case EHDTV:
			frame_dimension = 2048;
			break;
		case EUHDTV:
			frame_dimension = 4096;
			break;
        case E8KUHDTV:
            frame_dimension = 8192;
            break;
		default:
			break;
		}
	}

	return frame_dimension;
}


static uint32_t get_max_frame_size(intel_vvp_warp_instance_t* instance)
{
    uint32_t pixel_size = instance->bit_depth == 12 ? 8 : 4;
	uint32_t frame_dimension = get_max_frame_dimension(instance);
	uint32_t frame_size = frame_dimension * frame_dimension * pixel_size;

	return frame_size;
}


static uint32_t get_input_framebuffer_size(intel_vvp_warp_channel_t* channel)
{
	static const uint32_t NUM_INPUT_BUFFERS = 4;

	uint32_t input_framebuffer_size = 0x0;

	if(channel && channel->instance)
	{
		uint32_t frame_size = get_max_frame_size(channel->instance);
        /* Mipmap data requires space equivalent to 2x more framebuffers */
		uint32_t input_buffers = NUM_INPUT_BUFFERS + (channel->instance->mipmap_enable ? 2 : 0);
		input_framebuffer_size = frame_size * input_buffers;
	}

	return input_framebuffer_size;
}


static uint32_t get_max_coeff_size(intel_vvp_warp_channel_t* channel)
{
	uint32_t total_coeff_size = 0;
	
	if(0 != intel_vvp_warp_check_easy_warp_capable(channel))
	{
		intel_vvp_warp_instance_t* instance = channel->instance;
		static const uint32_t MEGABYTE = 1024 * 1024;
		static const uint32_t MESH_ENTRY_SIZE = 4;	/* 4 bytes for a single mesh coefficient */
		static const uint32_t FILTER_ENTRY_SIZE = 32;
		static const uint32_t FETCH_ENTRY_SIZE = 8;

		uint32_t frame_dimension = get_max_frame_dimension(instance);

		/* Mesh coefficients */
		uint32_t mesh_grid_res = 8;
		uint32_t mesh_points = frame_dimension / mesh_grid_res + 1;
		uint32_t mesh_entries = mesh_points * mesh_points;	/* Same number of mesh points for H and V */
		uint32_t mesh_coeff_size_raw = mesh_entries * MESH_ENTRY_SIZE;
		uint32_t mesh_coeff_size = roundup_pwr_two(mesh_coeff_size_raw, MEGABYTE);

		/* Filter coefficients */
		uint32_t h_blocks = frame_dimension / instance->block_width;
		uint32_t v_blocks = frame_dimension / instance->block_height;
		uint32_t filter_entries = v_blocks * h_blocks;
		uint32_t filter_coeff_size_raw = filter_entries * FILTER_ENTRY_SIZE;
		uint32_t filter_coeff_size = roundup_pwr_two(filter_coeff_size_raw, MEGABYTE);

		/* Fetch coefficients */
		/*
		 * We assume each input block has to be fetched at least once
		 * Some might be more than once...
		 * We are doubling the total number of blocks to
		 * have enough space
		 */
		uint32_t fetch_entries = 2 * v_blocks * h_blocks;
		uint32_t fetch_coeff_size_raw = fetch_entries * FETCH_ENTRY_SIZE;
		uint32_t fetch_coeff_size = roundup_pwr_two(fetch_coeff_size_raw, MEGABYTE);

		total_coeff_size = mesh_coeff_size + filter_coeff_size + fetch_coeff_size;

		INTEL_VVP_WARP_LOG("# Mesh:\t%"PRIu32"\t0x%"PRIx32"\n", mesh_coeff_size, mesh_coeff_size);
		INTEL_VVP_WARP_LOG("# Filter:\t%"PRIu32"\t0x%"PRIx32"\n", filter_coeff_size, filter_coeff_size);
		INTEL_VVP_WARP_LOG("# Fetch:\t%"PRIu32"\t0x%"PRIx32"\n", fetch_coeff_size, fetch_coeff_size);
		INTEL_VVP_WARP_LOG("# Total:\t%"PRIu32"\t0x%"PRIx32"\n", total_coeff_size, total_coeff_size);
	}

	return total_coeff_size;
}


static int check_block_scan(intel_vvp_warp_channel_t* ch)
{
	int ret = -1;

	/* For read/write reg macros */
	intel_vvp_warp_instance_t* instance = ch->instance;

	if((instance->output_bounce == EBOUNCE_BLOCK) || (instance->output_bounce == EBOUNCE_RASTER))
		ret = 0;
	else if(instance->output_bounce == EBOUNCE_CHANNEL)
	{
		uint32_t hw_idx = ch->output->idx;
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONFIG);
		reg_val &= INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_MSK;

		if((reg_val == INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_NBLOCK) || (reg_val == INTEL_VVP_WARP_OUTPUT_CONFIG_BOUNCE_BLOCK))
			ret = 0;
	}

	return ret;
}


static uint32_t get_skip_ram_size()
{
	/* Skip RAM size is the same for SD/HD/UHD/8K maps */
	static const uint32_t SKIP_RAM_SIZE = 1024;
	return SKIP_RAM_SIZE;
}


static uint32_t skip_ram_map_to_data(const uint8_t* skip_map)
{
    uint32_t skip_data = (skip_map[0] & 0x1) << 0;
    skip_data |= (skip_map[1] & 0x1) << 1;
    skip_data |= (skip_map[2] & 0x1) << 2;
    skip_data |= (skip_map[3] & 0x1) << 3;
    skip_data |= (skip_map[4] & 0x1) << 4;
    skip_data |= (skip_map[5] & 0x1) << 5;
    skip_data |= (skip_map[6] & 0x1) << 6;
    skip_data |= (skip_map[7] & 0x1) << 7;
    return skip_data;
}


static void update_skip_ram(intel_vvp_warp_channel_t* channel, intel_vvp_warp_data_t* data)
{
	assert(channel != NULL);
	assert(data != NULL);
	assert(data->num_engines > 0);
	assert(data->num_engines <= channel->num_engines);

	/* For read/write reg macros */
	intel_vvp_warp_instance_t* instance = channel->instance;

	static const uint32_t SKIP_RAM_WIDTH = 8;
		
	if(instance->output_skip)
    {
        static const uint32_t MEGABLOCK_SIZE = 8;		/* In blocks */
        static const uint32_t SKIP_RAM_COLS = 4;
        const uint32_t skip_ram_size = get_skip_ram_size();

        const uint32_t MB_MAX_H = ((data->engine_data[data->num_engines - 1].end_h) / MEGABLOCK_SIZE);	/* Rightmost megablock position for the current video width */
        const uint32_t MB_MAX_H_SKIP_RAM_COL = MB_MAX_H / SKIP_RAM_WIDTH;

        const uint32_t skip_ram_page = data->skip_ram_page;

        for(uint32_t i = 0; i < skip_ram_size; ++i)
        {
            uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_ENGINE_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_ENGINE_RAM_WR_PAGE_MSK);
            skip_addr |= ((i << INTEL_VVP_WARP_ENGINE_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_ENGINE_RAM_WR_ADDR_MSK);

            uint32_t data_idx = i * SKIP_RAM_WIDTH;
            uint32_t skip_mask = ((data->skip_megablock_data[data_idx + 0] & 0x1) << 0);
            skip_mask |= ((data->skip_megablock_data[data_idx + 1] & 0x1) << 1);
            skip_mask |= ((data->skip_megablock_data[data_idx + 2] & 0x1) << 2);
            skip_mask |= ((data->skip_megablock_data[data_idx + 3] & 0x1) << 3);
            skip_mask |= ((data->skip_megablock_data[data_idx + 4] & 0x1) << 4);
            skip_mask |= ((data->skip_megablock_data[data_idx + 5] & 0x1) << 5);
            skip_mask |= ((data->skip_megablock_data[data_idx + 6] & 0x1) << 6);
            skip_mask |= ((data->skip_megablock_data[data_idx + 7] & 0x1) << 7);

            /* Mask RAM */
            uint32_t mask_reg_val = INTEL_VVP_WARP_ENGINE_RAM_SELECT_SKIP_RAM | skip_addr | skip_mask;

            for(uint32_t e = 0; e < data->num_engines; ++e)
            {
                uint32_t eidx = channel->engines[e]->idx;
                INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_ENGINE_BASE(eidx) + INTEL_VVP_WARP_ENGINE_RAM, mask_reg_val);
            }

            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(channel->output->idx) + INTEL_VVP_WARP_OUTPUT_RAM, mask_reg_val);

            /* Fetch RAM */
            if((i % SKIP_RAM_COLS) == MB_MAX_H_SKIP_RAM_COL)
            {
                uint32_t last_mb_idx = MB_MAX_H % SKIP_RAM_WIDTH;
                uint32_t last_mb_mask = 0x1 << last_mb_idx;
                skip_mask &= ~last_mb_mask;
            }

            uint32_t fetch_reg_val = skip_addr | skip_mask;

            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(channel->output->idx) + INTEL_VVP_WARP_OUTPUT_RAM, fetch_reg_val);
        }
    }

    if(instance->mipmap_enable && data->skip_ram_input)
    {
        const uint32_t skip_ram_page = data->skip_ram_page;
        const uint32_t skip_ram_size = get_skip_ram_size();
        const uint8_t* skip_map = data->skip_ram_input;

        /* Original image Skip RAM */
        for(uint32_t i = 0; i < skip_ram_size; ++i)
        {
            uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_MSK);
            skip_addr |= ((i << INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_MSK);

            uint32_t skip_data = skip_ram_map_to_data(skip_map);
            skip_map += SKIP_RAM_WIDTH;

            uint32_t reg_val = INTEL_VVP_WARP_INPUT_RAM_SELECT_ORIG_RAM | skip_addr | skip_data;
            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_INPUT_BASE(channel->input->idx) + INTEL_VVP_WARP_INPUT_RAM, reg_val);
        }

        /* Mipmap pyramid Skip RAM */
        for(uint32_t i = 0; i < skip_ram_size; ++i)
        {
            uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_MSK);
            skip_addr |= ((i << INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_MSK);

            uint32_t skip_data = skip_ram_map_to_data(skip_map);
            skip_map += SKIP_RAM_WIDTH;
            
            uint32_t reg_val = INTEL_VVP_WARP_INPUT_RAM_SELECT_MM_RAM | skip_addr | skip_data;
            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_INPUT_BASE(channel->input->idx) + INTEL_VVP_WARP_INPUT_RAM, reg_val);
        }      
    }    
}


// Enable original input but skip the mipmap pyramid input
void intel_vvp_warp_reset_input_skip_ram(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page)
{
    intel_vvp_warp_instance_t* instance = channel->instance;

    if(instance->mipmap_enable)
    {
        const uint32_t skip_ram_size = get_skip_ram_size();

        /* Original image Skip RAM */
        for(uint32_t i = 0; i < skip_ram_size; ++i)
        {
            uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_MSK);
            skip_addr |= ((i << INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_MSK);

            uint32_t skip_data = 0x00;

            uint32_t reg_val = INTEL_VVP_WARP_INPUT_RAM_SELECT_ORIG_RAM | skip_addr | skip_data;
            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_INPUT_BASE(channel->input->idx) + INTEL_VVP_WARP_INPUT_RAM, reg_val);
        }

        /* Mipmap pyramid Skip RAM */
        for(uint32_t i = 0; i < skip_ram_size; ++i)
        {
            uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_PAGE_MSK);
            skip_addr |= ((i << INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_INPUT_RAM_WR_ADDR_MSK);

            uint32_t skip_data = 0xff;
            
            uint32_t reg_val = INTEL_VVP_WARP_INPUT_RAM_SELECT_MM_RAM | skip_addr | skip_data;
            INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_INPUT_BASE(channel->input->idx) + INTEL_VVP_WARP_INPUT_RAM, reg_val);
        }      
    }    
}


static intel_vvp_warp_pixel_filter_t get_channel_pixel_filter(intel_vvp_warp_channel_t* ch)
{
	intel_vvp_warp_pixel_filter_t pixel_filter = ENotAvailable;

	if(ch->num_engines)
	{
		intel_vvp_warp_instance_t* instance = ch->instance;

		uint32_t idx = ch->engines[0]->idx;
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(idx) + INTEL_VVP_WARP_ENGINE_CONFIG);

		pixel_filter = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_OFST;

		/* Configure the engines */
		for(uint32_t i = 1; i < ch->num_engines; ++i)
		{
			idx = ch->engines[i]->idx;
			reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(idx) + INTEL_VVP_WARP_ENGINE_CONFIG);
			uint32_t current_pixel_filter = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_OFST;

			if(current_pixel_filter != pixel_filter)
			{
				pixel_filter = ENotAvailable;
				break;
			}
		}
	}

	return pixel_filter;
}


static int check_engine_page_constraint(intel_vvp_warp_data_t* data)
{
	int ret = -1;
	const uint32_t page_mask = 0xff000000;
	uint32_t e = 0;

	while(e < data->num_engines)
	{
		uint32_t page = data->engine_data[e].mesh_addr & page_mask;
		uint32_t page1 = data->engine_data[e].filter_addr & page_mask;
		uint32_t page2 = data->engine_data[e].fetch_addr & page_mask;

		if((page != page1) || (page != page2))
			break;

		++e;
	}

	if(e == data->num_engines)
		ret = 0;

	return ret;
}


static uint32_t roundup_data_size(uint32_t size)
{
	uint32_t sr = 0;

	if(size > 0)
	{
		if (size < INTEL_VVP_WARP_DATA_SIZE_128KB)
			sr = INTEL_VVP_WARP_DATA_SIZE_128KB;
		else if (size <= INTEL_VVP_WARP_DATA_SIZE_8192KB)
			sr = get_next_pwr_two(size);	
	}

	return sr;
}


static uint32_t get_coef_ctrl_entry(uint32_t addr, uint32_t size)
{
	uint32_t val = 0x0;
	uint32_t offset_shift = 0x0;
	
	size = roundup_data_size(size);
	
	switch(size)
	{
		case INTEL_VVP_WARP_DATA_SIZE_128KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_128KB;
			offset_shift = 0x1;
		break;
		case INTEL_VVP_WARP_DATA_SIZE_256KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_256KB;
			offset_shift = 0x2;
		break;	
		case INTEL_VVP_WARP_DATA_SIZE_512KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_512KB;
			offset_shift = 0x3;
		break;
		case INTEL_VVP_WARP_DATA_SIZE_1024KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_1024KB;
			offset_shift = 0x4;
		break;
		case INTEL_VVP_WARP_DATA_SIZE_2048KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_2048KB;
			offset_shift = 0x5;
		break;
		case INTEL_VVP_WARP_DATA_SIZE_4096KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_4096KB;
			offset_shift = 0x6;
		break;
		case INTEL_VVP_WARP_DATA_SIZE_8192KB:
			val = INTEL_VVP_WARP_COEFF_CTRL_SIZE_8192KB;
			offset_shift = 0x7;
		break;		
		default:
		break;
	}
	
	if(val)
	{
		static const uint32_t page_mask = 0xff000000;
		const uint32_t page_offset = ((addr & ~(page_mask)) / size) << offset_shift;
		val |= page_offset;
	}

	return val;
}


#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
static void print_engine_configuration(intel_vvp_warp_instance_t* instance, uint32_t idx)
{
	uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(idx) + INTEL_VVP_WARP_ENGINE_CONFIG);

	uint32_t cache_size = ((reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_CACHE_DEPTH_MSK) << 4);
	uint32_t mesh_filtering = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_MESH_FILTERING_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_MESH_FILTERING_OFST;
	uint32_t pixel_filtering = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_PIXEL_FILTERING_OFST;
	uint32_t output_bounce = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_OUTPUT_BOUNCE_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_OUTPUT_BOUNCE_OFST;
	uint32_t ca = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_CHROMA_ABERR_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_CHROMA_ABERR_OFST;

	INTEL_VVP_WARP_LOG("Engine: %" PRIu32 "\n", idx);
	INTEL_VVP_WARP_LOG("Cache size:\t%" PRIu32 "\n", cache_size);
	INTEL_VVP_WARP_LOG("Mesh filter:\t%s\n", mesh_filtering ? (mesh_filtering == 2 ? "Bicubic" : "Bilinear LUT") : "Bilinear DSP");
	INTEL_VVP_WARP_LOG("Pixel filter:\t%s\n", pixel_filtering ? "Bicubic" : "Bilinear");
	INTEL_VVP_WARP_LOG("Output bounce:\t%s\n", output_bounce ? (output_bounce == 2 ? "Raster" : "Block") : "N/A");
	INTEL_VVP_WARP_LOG("Chroma aberr:\t%" PRIu32 "\n", ca);
}

static void print_output_configuration(intel_vvp_warp_instance_t* instance, uint32_t idx)
{
	intel_vvp_warp_output_t* output = &(instance->outputs[idx]);

	INTEL_VVP_WARP_LOG("Output: %" PRIu32 "\n", idx);
	INTEL_VVP_WARP_LOG("Easy warp:\t%s\n", output->easy_warp ? "Yes" : "No");
}
#endif /*INTEL_VVP_WARP_ENABLE_LOGGING*/


static intel_vvp_warp_input_t* allocate_input(intel_vvp_warp_instance_t* instance, uint32_t idx)
{
	intel_vvp_warp_input_t* p = NULL;

	if(instance)
	{
		if(idx < instance->num_inputs)
		{
			if(!(instance->inputs[idx].in_use))
			{
				p = &(instance->inputs[idx]);
				p->idx = idx;
				p->in_use = 1;
			}
			else
				INTEL_VVP_WARP_LOG("Input: %"PRIu32" is busy\n", idx);
		}
		else
			INTEL_VVP_WARP_LOG("Incorrect input: %"PRIu32"\n", idx);
	}

	return p;
}


static int free_input(intel_vvp_warp_instance_t* instance, intel_vvp_warp_input_t* p)
{
	int ret = -1;

	if(instance)
		if(p && (p->idx < instance->num_inputs))
			if(p == &(instance->inputs[p->idx]))
			{
				p->in_use = 0;
				ret = 0;
			}

	return ret;
}


static intel_vvp_warp_engine_t* allocate_engine(intel_vvp_warp_instance_t* instance, uint32_t idx)
{
	intel_vvp_warp_engine_t* p = NULL;

	if(instance)
	{
		if(idx < instance->num_engines)
		{
			if(!(instance->engines[idx].in_use))
			{
				p = &(instance->engines[idx]);
				p->idx = idx;
				p->in_use = 1;
			}
			else
				INTEL_VVP_WARP_LOG("Engine: %"PRIu32" is busy\n", idx);
		}
		else
			INTEL_VVP_WARP_LOG("Incorrect engine: %"PRIu32"\n", idx);
	}

	return p;
}


static int free_engine(intel_vvp_warp_instance_t* instance, intel_vvp_warp_engine_t* p)
{
	int ret = -1;

	if(instance)
    {
		if(p && (p->idx < instance->num_engines))
        {
			if(p == &(instance->engines[p->idx]))
			{
                /* Stop the engine */
                uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(p->idx) + INTEL_VVP_WARP_ENGINE_CONTROL);
                reg_val &= ~INTEL_VVP_WARP_ENGINE_CONTROL_EN;
                INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(p->idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
                p->in_use = 0;                
				ret = 0;
			}
        }
    }

	return ret;
}


static intel_vvp_warp_output_t* allocate_output(intel_vvp_warp_instance_t* instance, uint32_t idx)
{
	intel_vvp_warp_output_t* p = NULL;

	if(instance)
	{
		if(idx < instance->num_outputs)
		{
			if(!(instance->outputs[idx].in_use))
			{
				p = &(instance->outputs[idx]);
				p->idx = idx;
				p->in_use = 1;
			}
			else
				INTEL_VVP_WARP_LOG("Output: %"PRIu32" is busy\n", idx);
		}
		else
			INTEL_VVP_WARP_LOG("Incorrect output: %"PRIu32"\n", idx);
	}

	return p;
}


static int free_output(intel_vvp_warp_instance_t* instance, intel_vvp_warp_output_t* p)
{
	int ret = -1;

	if(instance)
		if(p && (p->idx < instance->num_outputs))
			if(p == &(instance->outputs[p->idx]))
			{
				p->in_use = 0;
				ret = 0;
			}

	return ret;
}


static intel_vvp_warp_channel_t* create_engineless_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t output_idx)
{
	intel_vvp_warp_channel_t* channel = NULL;

	if(instance)
	{
		intel_vvp_warp_input_t* input = allocate_input(instance, input_idx);
		intel_vvp_warp_output_t* output = allocate_output(instance, output_idx);

		if(input && output)
		{
			for(uint32_t i = 0; i < INTEL_VVP_WARP_MAX_CHANNELS; ++i)
			{
				if(!instance->channels[i].in_use)
				{
					channel = &(instance->channels[i]);

					channel->instance = instance;
					channel->idx = i;
					channel->input = input;
					channel->output = output;
					channel->num_engines = 0;
					channel->in_use = 1;

					channel->pixel_filter = ENotAvailable;
					channel->block_cache_size = 0;
					channel->ram_addr = 0x0;
					channel->width_output = 0;
					channel->height_output = 0;
					channel->output_block_scan = check_block_scan(channel) ? 0 : 1;
					break;
				}
			}

			if(!channel)
			{
				free_input(instance, input);
				free_output(instance, output);
			}
		}
	}

	return channel;
}

/**********************************************************************
 *
 *	Main driver API
 *
 *********************************************************************/

uint32_t intel_vvp_warp_get_debug_register(intel_vvp_warp_instance_t *instance, uint32_t reg_offset)
{
	uint32_t reg_val = 0;

	if(instance)
	{
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_DEBUG_BASE + (reg_offset % INTEL_VVP_WARP_DEBUG_REGION_SIZE));
	}

	return reg_val;
}


uint32_t intel_vvp_warp_get_input_debug_register(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t reg_offset)
{
	uint32_t reg_val = 0;

	if(instance && (input_idx < (instance->num_inputs)))
	{
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_DEBUG_INPUT_BASE(input_idx) + (reg_offset % INTEL_VVP_WARP_DEBUG_REGION_SIZE));
	}

	return reg_val;
}


uint32_t intel_vvp_warp_get_output_debug_register(intel_vvp_warp_instance_t *instance, uint32_t output_idx, uint32_t reg_offset)
{
	uint32_t reg_val = 0;

	if(instance && (output_idx < (instance->num_outputs)))
	{
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_DEBUG_OUTPUT_BASE(output_idx) + (reg_offset % INTEL_VVP_WARP_DEBUG_REGION_SIZE));
	}

	return reg_val;
}


uint32_t intel_vvp_warp_get_engine_debug_register(intel_vvp_warp_instance_t *instance, uint32_t engine_idx, uint32_t reg_offset)
{
	uint32_t reg_val = 0;

	if(instance && (engine_idx < (instance->num_engines)))
	{
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_DEBUG_ENGINE_BASE(engine_idx) + (reg_offset % INTEL_VVP_WARP_DEBUG_REGION_SIZE));
	}

	return reg_val;
}


/* Get engine status flags */
uint32_t intel_vvp_warp_get_engine_status(intel_vvp_warp_instance_t *instance, uint32_t engine_idx)
{
	uint32_t val = 0;

	if(instance && (engine_idx < instance->num_engines))
	{
		/* Read frame failed to complete flag */
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(engine_idx) + INTEL_VVP_WARP_ENGINE_STATUS);
		reg_val = (reg_val & INTEL_VVP_WARP_ENGINE_STATUS_FRAME_FAIL_MSK) >> INTEL_VVP_WARP_ENGINE_STATUS_FRAME_FAIL_OFST;
		val |= reg_val;

		/* Clear frame failed to complete flag */
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(engine_idx) + INTEL_VVP_WARP_ENGINE_CONTROL);
		reg_val |= INTEL_VVP_WARP_ENGINE_CONTROL_FRAME_FAIL_CLEAR;
        INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_ENGINE_BASE(engine_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
		reg_val &= ~(INTEL_VVP_WARP_ENGINE_CONTROL_FRAME_FAIL_CLEAR);
        INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_ENGINE_BASE(engine_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
	}

	return val;
}


/* Get output status flags */
uint32_t intel_vvp_warp_get_output_status(intel_vvp_warp_instance_t *instance, uint32_t output_idx)
{
	uint32_t val = 0;

	if(instance && (output_idx < instance->num_outputs))
	{
		/* Read frame failed to complete flag */
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_STATUS);
		reg_val = (reg_val & INTEL_VVP_WARP_OUTPUT_STATUS_LFR_STICKY_MSK) >> INTEL_VVP_WARP_OUTPUT_STATUS_LFR_STICKY_OFST;
		val |= reg_val;

		/* Clear frame failed to complete flag */
		reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);
		reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_STATUS_CLEAR;
        INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
		reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_STATUS_CLEAR);
        INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
	}

	return val;
}


uint32_t intel_vvp_warp_get_channel_framebuffer_size(intel_vvp_warp_channel_t* channel)
{
	static const uint32_t NUM_OUTPUT_BUFFERS = 2;

	uint32_t channel_framebuffer_size = 0x0;

	if(channel && channel->instance)
	{
		uint32_t frame_size = get_max_frame_size(channel->instance);
		uint32_t num_output_buffers = (check_output_bounce(channel) ? NUM_OUTPUT_BUFFERS : 0);
		channel_framebuffer_size = get_input_framebuffer_size(channel) + frame_size * num_output_buffers;
	}

	return channel_framebuffer_size;
}


uint32_t intel_vvp_warp_get_channel_ram_size(intel_vvp_warp_channel_t* channel)
{
	return intel_vvp_warp_get_channel_framebuffer_size(channel) + get_max_coeff_size(channel);
}


void intel_vvp_warp_set_output_latency(intel_vvp_warp_channel_t* channel, uint32_t clock_offset)
{
	if(channel)
	{
		if(channel->instance && channel->output)
		{
			intel_vvp_warp_instance_t* instance = channel->instance;
			uint32_t hw_idx = channel->output->idx;
			uint32_t reg_val = ((clock_offset << INTEL_VVP_WARP_OUTPUT_LATENCY_OFFSET_OFST) & INTEL_VVP_WARP_OUTPUT_LATENCY_OFFSET_MSK);
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_LATENCY, reg_val);

            /* Update skip RAM latency bit */
            reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
            reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_SKIPRAM_LATENCY);

            /*
                Set skip RAM letency to 1 frame for single bounce - always
                for double bounce - only when in low latency mode
            */
            if((check_output_bounce(channel) == 0) || (clock_offset != 0))
                reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_SKIPRAM_LATENCY;

            INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);
		}
	}
}


int intel_vvp_warp_check_easy_warp_capable(intel_vvp_warp_channel_t* channel)
{
	int ret = -1;

	if(channel && channel->output)
	{
		intel_vvp_warp_output_t* output = channel->output;

		if(output->easy_warp)
			ret = 0;
	}

	return ret;
}


int intel_vvp_warp_set_easy_warp(intel_vvp_warp_channel_t* channel, uint32_t easy_warp)
{
	int ret = intel_vvp_warp_check_easy_warp_capable(channel);

	if(0 == ret)
	{
		intel_vvp_warp_output_t* output = channel->output;

		if(output)
		{
			intel_vvp_warp_instance_t* instance = channel->instance;
			uint32_t hw_idx = output->idx;
			uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);
			reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_EASY_WARP_MSK);
			reg_val |= ((easy_warp << INTEL_VVP_WARP_OUTPUT_CONTROL_EASY_WARP_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_EASY_WARP_MSK);
			reg_val |= (INTEL_VVP_WARP_OUTPUT_CONTROL_EN);
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
			ret = 0;
		}
	}
	else
		INTEL_VVP_WARP_LOG("Channel: %" PRIu32 " does not support easy warp!\n", channel->idx);

	return ret;
}


void intel_vvp_warp_reset_skip_ram(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page)
{
	if (channel)
	{
		const uint32_t skip_ram_size = get_skip_ram_size();
		
		/* For read/write reg macros */
		intel_vvp_warp_instance_t* instance = channel->instance;
		
		for(uint32_t i = 0; i < skip_ram_size; ++i)
		{
			uint32_t skip_addr = ((skip_ram_page << INTEL_VVP_WARP_ENGINE_RAM_WR_PAGE_OFST) & INTEL_VVP_WARP_ENGINE_RAM_WR_PAGE_MSK);
			skip_addr |= ((i << INTEL_VVP_WARP_ENGINE_RAM_WR_ADDR_OFST) & INTEL_VVP_WARP_ENGINE_RAM_WR_ADDR_MSK);

			uint32_t skip_mask = 0x0;

			/* Mask RAM */
			uint32_t mask_reg_val = INTEL_VVP_WARP_ENGINE_RAM_SELECT_SKIP_RAM | skip_addr | skip_mask;

			for(uint32_t e = 0; e < channel->num_engines; ++e)
			{
				if(channel->engines[e])
				{
					uint32_t eidx = channel->engines[e]->idx;
					INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_ENGINE_BASE(eidx) + INTEL_VVP_WARP_ENGINE_RAM, mask_reg_val);
				}
			}

			INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(channel->output->idx) + INTEL_VVP_WARP_OUTPUT_RAM, mask_reg_val);

			/* Fetch RAM */
			uint32_t fetch_reg_val = skip_addr | skip_mask;

			INTEL_VVP_WARP_REG_IOWR_QUIET(INTEL_VVP_WARP_OUTPUT_BASE(channel->output->idx) + INTEL_VVP_WARP_OUTPUT_RAM, fetch_reg_val);
		}		
	}
}

 
void intel_vvp_warp_debug_block(intel_vvp_warp_channel_t* channel, uint32_t v, uint32_t h)
{
	if(channel)
	{
		if(channel->output && channel->instance)
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;
			uint32_t output_idx = channel->output->idx;

            uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
            reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_V_BLOCK_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_H_BLOCK_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_DBG_BLOCK_EN);
			reg_val |= (v << INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_V_BLOCK_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_V_BLOCK_MSK;
			reg_val |= (h << INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_H_BLOCK_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_H_BLOCK_MSK;
			reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_DBG_BLOCK_EN;
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);
		}
	}
}


void intel_vvp_warp_debug_block_disable(intel_vvp_warp_channel_t* channel)
{
	if(channel)
	{
		if(channel->output && channel->instance)
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

			uint32_t output_idx = channel->output->idx;
			uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
			reg_val &= ~INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_DBG_BLOCK_EN;
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);
		}
	}
}


void intel_vvp_warp_freerun_enable(intel_vvp_warp_channel_t* channel)
{
	if(channel)
	{
		if(channel->output && channel->instance)
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

			uint32_t output_idx = channel->output->idx;
			uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
			reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_FREERUN;
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);
		}
	}
}


void intel_vvp_warp_freerun_disable(intel_vvp_warp_channel_t* channel)
{
	if(channel)
	{
		if(channel->output && channel->instance)
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

			uint32_t output_idx = channel->output->idx;
			uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
			reg_val &= ~INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_FREERUN;
			INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(output_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);
		}
	}
}


void intel_vvp_warp_lfr(intel_vvp_warp_channel_t* channel, uint32_t enable)
{
	if(channel)
	{
		if(channel->instance && channel->output)
		{
            // Always disable Low frame rate fallback in single bounce setup
            enable = check_output_bounce(channel) ? enable : 0x0;

            /* For read/write reg macros */
            intel_vvp_warp_instance_t* instance = channel->instance;
            uint32_t hw_idx = channel->output->idx;
            uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);

            if(enable == 0)
                reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_LFR;
            else
                reg_val &= ~INTEL_VVP_WARP_OUTPUT_CONTROL_LFR;

            INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
		}
	}
}
 
 
int intel_vvp_warp_bypass(intel_vvp_warp_channel_t* channel, uint32_t bypass, uint32_t skip_ram_page, uint32_t width, uint32_t height)
{
	int ret = -1;

	if(channel && channel->instance)
	{
		/* Bypass only possible if the output bounce is present */
		if(check_output_bounce(channel))
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

			/* Configure individual engines */
			for(uint32_t i = 0; i < channel->num_engines; ++i)
			{
				intel_vvp_warp_engine_t* engine = channel->engines[i];

				if(engine)
				{
					uint32_t hw_idx = engine->idx;

					uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL);

					/* Clear relevant bits */
					reg_val &= ~INTEL_VVP_WARP_ENGINE_CONTROL_EN;

					if(!bypass)
					{
						reg_val |= INTEL_VVP_WARP_ENGINE_CONTROL_EN;
					}

					INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
				}
			}
			
			/* Configure input */
			intel_vvp_warp_input_t* input = channel->input;
			
			if(input)
			{
				uint32_t hw_idx = input->idx;
				uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL);
				reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_STORE_RASTER);

				if(bypass && instance->output_bounce == EBOUNCE_RASTER)
					reg_val |= INTEL_VVP_WARP_INPUT_CONTROL_STORE_RASTER;

                /* Set input skip RAM page */
                if(instance->mipmap_enable)
                {
                    reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
                    reg_val |= ((skip_ram_page << INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
                }                    
					
				INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL, reg_val);
			}		

			/* Configure output */
			intel_vvp_warp_output_t* output = channel->output;

			if(output)
			{
				uint32_t hw_idx = output->idx;

				/* Update output resolution if necessary */
				uint32_t width_output = channel->width_output;
				uint32_t height_output = channel->height_output;

				if(width && height)
				{
					width_output = width;
					height_output = height;
				}

				uint32_t reg_val = ((width_output - 1) << INTEL_VVP_WARP_OUTPUT_RESOLUTION_H_OFST) & INTEL_VVP_WARP_OUTPUT_RESOLUTION_H_MSK;
				reg_val |= ((height_output - 1) << INTEL_VVP_WARP_OUTPUT_RESOLUTION_V_OFST) & INTEL_VVP_WARP_OUTPUT_RESOLUTION_V_MSK;

				INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_RESOLUTION, reg_val);                

			    uint32_t output_ram_addr = channel->ram_addr;
				reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);
				reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_MEM_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_EASY_WARP_MSK);

				if(!bypass)
				{
					output_ram_addr += get_input_framebuffer_size(channel);
				}

				reg_val |= (output_ram_addr & INTEL_VVP_WARP_OUTPUT_CONTROL_MEM_MSK);
				reg_val |= ((skip_ram_page << INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK);

                /* Make sure output is enabled */
                reg_val |= (INTEL_VVP_WARP_OUTPUT_CONTROL_EN);

				INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
			}

			ret = 0;
		}
	}

	return ret;
}


int intel_vvp_warp_apply_transform(intel_vvp_warp_channel_t* channel, intel_vvp_warp_data_t* data)
{
	int ret = -1;

	if(0 != intel_vvp_warp_check_easy_warp_capable(channel))
	{
		if(channel && channel->instance && data)
		{
			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

			if(data->num_engines <= channel->num_engines)
			{
				if(0 == check_engine_page_constraint(data))
				{
                    update_skip_ram(channel, data);

					const uint32_t skip_ram_page = data->skip_ram_page;
					static const uint32_t page_mask = 0xff000000;

					for(uint32_t i = 0; i < data->num_engines; ++i)
					{
						uint32_t page = data->engine_data[i].mesh_addr & page_mask;

						const uint32_t filter_entry	= get_coef_ctrl_entry(data->engine_data[i].filter_addr,	data->engine_data[i].filter_size);
						const uint32_t mesh_entry	= get_coef_ctrl_entry(data->engine_data[i].mesh_addr,	data->engine_data[i].mesh_size);
						const uint32_t fetch_entry	= get_coef_ctrl_entry(data->engine_data[i].fetch_addr,	data->engine_data[i].fetch_size);

						uint32_t reg_val = page;
						
						reg_val |= ((filter_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_MSK);
						reg_val |= ((mesh_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_MSK);
						reg_val |= ((fetch_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_MSK);

						uint32_t hw_idx = channel->engines[i]->idx;

						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_COEF_CTRL, reg_val);

						reg_val = (data->engine_data[i].start_v << INTEL_VVP_WARP_ENGINE_GEOMETRY_START_V_OFST) & INTEL_VVP_WARP_ENGINE_GEOMETRY_START_V_MSK;
						reg_val |= (data->engine_data[i].start_h << INTEL_VVP_WARP_ENGINE_GEOMETRY_START_H_OFST) & INTEL_VVP_WARP_ENGINE_GEOMETRY_START_H_MSK;
						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_GEOMETRY_START, reg_val);

						reg_val = (data->engine_data[i].end_v << INTEL_VVP_WARP_ENGINE_GEOMETRY_END_V_OFST) & INTEL_VVP_WARP_ENGINE_GEOMETRY_END_V_MSK;
						reg_val |= (data->engine_data[i].end_h << INTEL_VVP_WARP_ENGINE_GEOMETRY_END_H_OFST) & INTEL_VVP_WARP_ENGINE_GEOMETRY_END_H_MSK;
						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_GEOMETRY_END, reg_val);

                        /* Mesh stride, resolution and precision */
						reg_val = (data->engine_data[i].mesh_stride << INTEL_VVP_WARP_ENGINE_MESH_CTL_STRIDE_OFST) & INTEL_VVP_WARP_ENGINE_MESH_CTL_STRIDE_MSK;

                        if(data->mesh_step == 16)
                        {
                            reg_val |= (EMESH_16x16 << INTEL_VVP_WARP_ENGINE_MESH_CTL_RES_OFST) & INTEL_VVP_WARP_ENGINE_MESH_CTL_RES_MSK;
                            reg_val |= (EMESH_3_FRACT_BITS << INTEL_VVP_WARP_ENGINE_MESH_CTL_PREC_OFST) & INTEL_VVP_WARP_ENGINE_MESH_CTL_PREC_MSK;
                        }

                        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_MESH_CTL, reg_val);

						/* Set skip RAM page and enable the engine */
						reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL);
						reg_val &= ~(INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_MSK);
						reg_val |= ((skip_ram_page << INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_MSK);
						reg_val |= INTEL_VVP_WARP_ENGINE_CONTROL_EN;
						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
					}

					/* Update input */
					intel_vvp_warp_input_t* input = channel->input;

					if(input)
					{
                        uint32_t hw_idx = input->idx;
						uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL);

                        /* Clear Store Raster flag which might have been set for the bypass mode */
						if(instance->output_bounce == EBOUNCE_RASTER)
						{
							reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_STORE_RASTER);							
						}

                        /* Set mipmap level */
                        if(instance->mipmap_enable)
                        {
                            reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_MSK | INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
                            reg_val |= ((data->mipmap_level << INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_MSK);
                            reg_val |= ((skip_ram_page << INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
                        }

                        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL, reg_val);
					}
					
					/* Update output */
					intel_vvp_warp_output_t* output = channel->output;

					if(output)
					{
						uint32_t hw_idx = output->idx;

						/* Update output resolution if necessary */
						uint32_t width_output = channel->width_output;
						uint32_t height_output = channel->height_output;

						uint32_t reg_val = ((width_output - 1) << INTEL_VVP_WARP_OUTPUT_RESOLUTION_H_OFST) & INTEL_VVP_WARP_OUTPUT_RESOLUTION_H_MSK;
						reg_val |= ((height_output - 1) << INTEL_VVP_WARP_OUTPUT_RESOLUTION_V_OFST) & INTEL_VVP_WARP_OUTPUT_RESOLUTION_V_MSK;

						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_RESOLUTION, reg_val);

						reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);

						reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_MEM_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK);

						if(check_output_bounce(channel))
						{
                            uint32_t output_ram_addr = channel->ram_addr + get_input_framebuffer_size(channel);
							reg_val |= (output_ram_addr & INTEL_VVP_WARP_OUTPUT_CONTROL_MEM_MSK);
						}

						reg_val |= ((skip_ram_page << INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK);

						/* Make sure output is enabled */
						if((width_output != 0) && (height_output != 0))
						{
                        	reg_val |= (INTEL_VVP_WARP_OUTPUT_CONTROL_EN);
						}

						INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
					}

					ret = 0;
				}
				else
				{
					INTEL_VVP_WARP_LOG("Engine coefficient tables must be on the same 16Mb page!\n");
				}
			}
			else
			{
				INTEL_VVP_WARP_LOG("Too many engines: %" PRIu32 "\n", data->num_engines);
			}
		}
	}
	else
	{
		INTEL_VVP_WARP_LOG("Channel: %" PRIu32 " supports easy warp only!\n", channel->idx);
	}

	return ret;
}


void intel_vvp_warp_prepare_preset_4K(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page)
{
        /* For read/write reg macros */
    intel_vvp_warp_instance_t* instance = channel->instance;

    intel_vvp_warp_reset_skip_ram(channel, skip_ram_page);

    const uint32_t num_engines = channel->num_engines;

    for(uint32_t i = 0; i < num_engines; ++i)
    {
        // Update skip RAM page and make sure engine is enabled
        uint32_t hw_idx = channel->engines[i]->idx;
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL);
		reg_val &= ~(INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_MSK);
		reg_val |= ((skip_ram_page << INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_MSK);
		reg_val |= INTEL_VVP_WARP_ENGINE_CONTROL_EN;
		INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
    }

    /* Update output skip RAM page and make sure output is enabled */
    intel_vvp_warp_output_t* output = channel->output;

    if(output)
    {
        uint32_t hw_idx = output->idx;
        uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);
        reg_val &= ~INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK;
        reg_val |= (INTEL_VVP_WARP_OUTPUT_CONTROL_EN);
        reg_val |= ((skip_ram_page << INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_SKIPP_MSK);
        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);
    }

    // Disable mipmaps for the demo
    if(instance->mipmap_enable)
    {
        intel_vvp_warp_reset_input_skip_ram(channel, skip_ram_page);

        intel_vvp_warp_input_t* input = channel->input;

		if(input)
		{
            uint32_t hw_idx = input->idx;
            uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL);            
            reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_MSK | INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
            reg_val |= ((0x1 << INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_MM_LEVEL_MSK);
            reg_val |= ((skip_ram_page << INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_SKIPP_MSK);
            INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL, reg_val);
        }        
    }    
}


int intel_vvp_warp_apply_preset_4K(intel_vvp_warp_channel_t* channel, uint32_t data_base)
{
	int ret = -1;

    /* For read/write reg macros */
    intel_vvp_warp_instance_t* instance = channel->instance;

    static const uint32_t page_mask = 0xff000000;
    const uint32_t num_engines = channel->num_engines;

    for(uint32_t i = 0; i < num_engines; ++i)
    {
        uint32_t engine_data_base = data_base + i * 0x200000;
        uint32_t page = engine_data_base & page_mask;

        const uint32_t filter_entry	= get_coef_ctrl_entry(engine_data_base,	            0x100000);
        const uint32_t mesh_entry	= get_coef_ctrl_entry(engine_data_base + 0x100000,	0x080000);
        const uint32_t fetch_entry	= get_coef_ctrl_entry(engine_data_base + 0x180000,	0x080000);        

        uint32_t reg_val = page;
        
        reg_val |= ((filter_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_MSK);
        reg_val |= ((mesh_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_MSK);
        reg_val |= ((fetch_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_MSK);

        uint32_t hw_idx = channel->engines[i]->idx;

        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_COEF_CTRL, reg_val);
    }   

    ret = 0;

	return ret;
}


int intel_vvp_warp_apply_preset_8K(intel_vvp_warp_channel_t* channel, uint32_t data_base)
{
	int ret = -1;

    /* For read/write reg macros */
    intel_vvp_warp_instance_t* instance = channel->instance;

    static const uint32_t page_mask = 0xff000000;
    const uint32_t num_engines = channel->num_engines;

    for(uint32_t i = 0; i < num_engines; ++i)
    {
        uint32_t engine_data_base = data_base + i * 0x400000;
        uint32_t page = engine_data_base & page_mask;

        const uint32_t filter_entry	= get_coef_ctrl_entry(engine_data_base,	            0x200000);
        const uint32_t fetch_entry	= get_coef_ctrl_entry(engine_data_base + 0x200000,	0x100000);
        const uint32_t mesh_entry	= get_coef_ctrl_entry(engine_data_base + 0x300000,	0x080000);

        uint32_t reg_val = page;
        
        reg_val |= ((filter_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FILTER_MSK);
        reg_val |= ((mesh_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_MESH_MSK);
        reg_val |= ((fetch_entry << INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_OFST) & INTEL_VVP_WARP_ENGINE_COEF_CTRL_FETCH_MSK);

        uint32_t hw_idx = channel->engines[i]->idx;

        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_COEF_CTRL, reg_val);
    }   

    ret = 0;

	return ret;
}


int intel_vvp_warp_configure_channel(intel_vvp_warp_channel_t* channel, intel_vvp_warp_channel_config_t* cfg)
{
	int ret = -1;

	if(channel && channel->instance && cfg)
	{
		/* Channel must have input and output */
		if(channel->input && channel->output)
		{
			uint32_t memctl_val = 0x0;

			channel->ram_addr = cfg->ram_addr;
			channel->pixel_filter = ENotAvailable;

			/* For read/write reg macros */
			intel_vvp_warp_instance_t* instance = channel->instance;

            /* Configure the output */
            intel_vvp_warp_output_t* output = channel->output;

            {
                /*
                    Defer output resolution and control register changes until
                    the warp is applied or bypass enabled
                */
                channel->width_output = cfg->width_output;
                channel->height_output = cfg->height_output;

                uint32_t hw_idx = output->idx;

                uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL);

                /* Set input binding and colourspace; Disable output */
                reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_CS_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_BIND_MSK | INTEL_VVP_WARP_OUTPUT_CONTROL_EN);
                reg_val |= (channel->input->idx << INTEL_VVP_WARP_OUTPUT_CONTROL_BIND_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_BIND_MSK;
                reg_val |= ((cfg->cs << INTEL_VVP_WARP_OUTPUT_CONTROL_CS_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_CS_MSK);
                reg_val |= ((cfg->cs << INTEL_VVP_WARP_OUTPUT_CONTROL_CS_OFST) & INTEL_VVP_WARP_OUTPUT_CONTROL_CS_MSK);
                /* Disable low frame rate fallback by default */
                reg_val |= INTEL_VVP_WARP_OUTPUT_CONTROL_LFR;

                INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL, reg_val);

                /* Set low frame rate fallback bit */
                intel_vvp_warp_lfr(channel, cfg->lfr);

                /* Clear Enable debug block flag */
                reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT);
                reg_val &= ~(INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_DBG_BLOCK_EN | INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_SKIPRAM_LATENCY);

                /* Debugging video lockup */
                /* By default reduce skip RAM latency to 1 frame for the single bounce configuration */
                reg_val |= (check_output_bounce(channel) == 0 ? INTEL_VVP_WARP_OUTPUT_CONTROL_EXT_SKIPRAM_LATENCY : 0x0);

                INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_OUTPUT_BASE(hw_idx) + INTEL_VVP_WARP_OUTPUT_CONTROL_EXT, reg_val);

                /* Output reads */
                memctl_val |= (check_output_bounce(channel) ? (0x1 << memctl_output_read_shift(channel, hw_idx)) : 0x0);
            }            

			ret = channel->num_engines ? -1 : 0;

			// Configure engines if required
			if(channel->num_engines)
			{
				/* Check all channel engines support the same pixel filter */
				intel_vvp_warp_pixel_filter_t pixel_filter = get_channel_pixel_filter(channel);

				if(pixel_filter != ENotAvailable)
				{
					uint32_t block_cache_size = 0;

					/* Configure the engines */
					uint32_t num_engines = channel->num_engines;

					if(num_engines > INTEL_VVP_WARP_MAX_CHANNEL_ENGINES)
						channel->num_engines = INTEL_VVP_WARP_MAX_CHANNEL_ENGINES;

					for(uint32_t i = 0; i < num_engines; ++i)
					{
						intel_vvp_warp_engine_t* engine = channel->engines[i];

						if(engine)
						{
							uint32_t hw_idx = engine->idx;

							uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONFIG);

							uint32_t tmp_cache_size = (reg_val & INTEL_VVP_WARP_ENGINE_CONFIG_CACHE_DEPTH_MSK) >> INTEL_VVP_WARP_ENGINE_CONFIG_CACHE_DEPTH_OFST;
							tmp_cache_size = tmp_cache_size << 4;	/* Reported cache size is divided by 16 */
							block_cache_size = ((block_cache_size == 0) ? tmp_cache_size : min_val(block_cache_size, tmp_cache_size));
							
							uint32_t scan_pattern = (channel->output_block_scan ? EMEGABLOCK : ERASTER);

							reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL);

							reg_val &= ~(INTEL_VVP_WARP_ENGINE_CONTROL_EN | INTEL_VVP_WARP_ENGINE_CONTROL_BIND_MSK |
										 INTEL_VVP_WARP_ENGINE_CONTROL_CS_MSK | INTEL_VVP_WARP_ENGINE_CONTROL_SCAN_MSK | INTEL_VVP_WARP_ENGINE_CONTROL_SKIPP_MSK);

							/* Note the engine(s) is not enabled until a warp has been configured */
							reg_val |= ((channel->output->idx << INTEL_VVP_WARP_ENGINE_CONTROL_BIND_OFST) & INTEL_VVP_WARP_ENGINE_CONTROL_BIND_MSK);
							reg_val |= ((cfg->cs << INTEL_VVP_WARP_ENGINE_CONTROL_CS_OFST) & INTEL_VVP_WARP_ENGINE_CONTROL_CS_MSK);
							reg_val |= ((scan_pattern << INTEL_VVP_WARP_ENGINE_CONTROL_SCAN_OFST) & INTEL_VVP_WARP_ENGINE_CONTROL_SCAN_MSK);
							reg_val |= INTEL_VVP_WARP_ENGINE_CONTROL_LOWLAT_PROTECT;

							/* Engine writes */
							memctl_val |= (check_output_bounce(channel) ? (0x1 << memctl_engine_write_shift(channel, hw_idx)) : 0x0);

							/* Engine reads */
							memctl_val |= (0xf << memctl_engine_read_shift(channel, hw_idx));

							INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(hw_idx) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);
						}
					}

					channel->pixel_filter = pixel_filter;
					channel->block_cache_size = block_cache_size;

					ret = 0;
				}
			}

			// Continue if engines have been configured
			if(0 == ret)
			{
				/* Configure the input */
				intel_vvp_warp_input_t* input = channel->input;

				{
					uint32_t hw_idx = input->idx;

					uint32_t reg_val = ((cfg->width_input - 1) << INTEL_VVP_WARP_INPUT_RESOLUTION_H_OFST) & INTEL_VVP_WARP_INPUT_RESOLUTION_H_MSK;
					reg_val |= ((cfg->height_input - 1) << INTEL_VVP_WARP_INPUT_RESOLUTION_V_OFST) & INTEL_VVP_WARP_INPUT_RESOLUTION_V_MSK;

					INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_RESOLUTION, reg_val);

					reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL);
					reg_val &= ~(INTEL_VVP_WARP_INPUT_CONTROL_EN | INTEL_VVP_WARP_INPUT_CONTROL_CS_MSK | INTEL_VVP_WARP_INPUT_CONTROL_MEM_MSK | INTEL_VVP_WARP_INPUT_CONTROL_FRAME_CTL_MSK);

					reg_val |= ((cfg->cs << INTEL_VVP_WARP_INPUT_CONTROL_CS_OFST) & INTEL_VVP_WARP_INPUT_CONTROL_CS_MSK);

					reg_val |= (channel->ram_addr & INTEL_VVP_WARP_INPUT_CONTROL_MEM_MSK);
					reg_val |= INTEL_VVP_WARP_INPUT_CONTROL_EN;

					INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL, reg_val);

					/* Input writes */
                    uint32_t input_write = channel->instance->mipmap_enable ? 0x3 : 0x1;
					memctl_val |= (input_write << memctl_input_write_shift(channel, hw_idx));
				}

				/* Make sure relevant memory controllers are enabled */
				{
					uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_REGMAP_BASE + INTEL_VVP_WARP_MEM_CONTROL);
					reg_val |= memctl_val;
					INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_REGMAP_BASE + INTEL_VVP_WARP_MEM_CONTROL, reg_val);
				}

                /* Always enable for VVP Warp */
                intel_vvp_warp_freerun_enable(channel);
			}
		}
	}

	return ret;
}


void intel_vvp_warp_free_channel(intel_vvp_warp_channel_t* channel)
{
	if(channel && channel->instance)
	{
		intel_vvp_warp_instance_t* instance = channel->instance;

		for(uint32_t i = 0; i < INTEL_VVP_WARP_MAX_CHANNELS; ++i)
		{
			if(channel == &(instance->channels[i]))
			{
				/* Release HW resources */
				free_input(instance, channel->input);
				channel->input = NULL;

				for(uint32_t j = 0; j < channel->num_engines; ++j)
				{
					free_engine(instance, channel->engines[j]);
					channel->engines[j] = NULL;
				}

				channel->num_engines = 0;

				free_output(instance, channel->output);
				channel->output = NULL;

				channel->in_use = 0;
				break;
			}
		}
	}
}


int intel_vvp_warp_add_engine_to_channel(intel_vvp_warp_channel_t* channel, uint32_t engine_idx)
{
	int ret = -1;

	if(channel && channel->instance)
	{
		if(channel->num_engines < INTEL_VVP_WARP_MAX_CHANNEL_ENGINES)
		{
			intel_vvp_warp_engine_t* engine = allocate_engine(channel->instance, engine_idx);

			if(engine)
			{
				channel->engines[channel->num_engines] = engine;
				channel->num_engines += 1;
				ret = 0;
			}
		}
	}

	return ret;
}


intel_vvp_warp_channel_t* intel_vvp_warp_create_channel(intel_vvp_warp_instance_t* instance, uint32_t input_idx, uint32_t engine_idx, uint32_t output_idx)
{
	intel_vvp_warp_channel_t* channel = NULL;

	if(instance)
	{
		channel = create_engineless_channel(instance, input_idx, output_idx);

		if(channel)
		{
			int ret = intel_vvp_warp_add_engine_to_channel(channel, engine_idx);

			if(ret)
			{
				INTEL_VVP_WARP_LOG("Error adding engine %"PRIu32" to channel\n", engine_idx);
				intel_vvp_warp_free_channel(channel);
				channel = NULL;
			}
		}
	}

	return channel;
}


intel_vvp_warp_channel_t* intel_vvp_warp_create_double_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t engine1_idx, uint32_t engine2_idx, uint32_t output_idx)
{
	intel_vvp_warp_channel_t* channel = NULL;

	if(instance)
	{
		channel = intel_vvp_warp_create_channel(instance, input_idx, engine1_idx, output_idx);

		if(channel)
		{
			int ret = intel_vvp_warp_add_engine_to_channel(channel, engine2_idx);

			if(ret)
			{
				INTEL_VVP_WARP_LOG("Error adding engine %"PRIu32" to channel\n", engine2_idx);
				intel_vvp_warp_free_channel(channel);
				channel = NULL;
			}
		}
	}

	return channel;
}


intel_vvp_warp_channel_t* intel_vvp_warp_create_quad_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t output_idx)
{
	intel_vvp_warp_channel_t* channel = NULL;

	if(instance)
	{
		channel = intel_vvp_warp_create_channel(instance, input_idx, 0, output_idx);

		if(channel)
		{
			int ret = intel_vvp_warp_add_engine_to_channel(channel, 1);

			if(ret)
			{
				INTEL_VVP_WARP_LOG("Error adding engine %"PRIu32" to channel\n", 1);
				intel_vvp_warp_free_channel(channel);
				channel = NULL;
			}
            else
            {
                ret = intel_vvp_warp_add_engine_to_channel(channel, 2);

                if(ret)
                {
                    INTEL_VVP_WARP_LOG("Error adding engine %"PRIu32" to channel\n", 2);
                    intel_vvp_warp_free_channel(channel);
                    channel = NULL;
                }
                else
                {
                    ret = intel_vvp_warp_add_engine_to_channel(channel, 3);

                    if(ret)
                    {
                        INTEL_VVP_WARP_LOG("Error adding engine %"PRIu32" to channel\n", 3);
                        intel_vvp_warp_free_channel(channel);
                        channel = NULL;
                    }                    
                }
            }
		}
	}

	return channel;
}


intel_vvp_warp_channel_t* intel_vvp_warp_create_easy_warp_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t output_idx)
{
	intel_vvp_warp_channel_t* channel = NULL;

	if(instance)
	{
		channel = create_engineless_channel(instance, input_idx, output_idx);

		if(channel)
		{
			int ret = intel_vvp_warp_check_easy_warp_capable(channel);

			if(ret)
			{
				INTEL_VVP_WARP_LOG("Error: requested warp output %"PRIu32" isn't capable of easy warp!\n", output_idx);
				intel_vvp_warp_free_channel(channel);
				channel = NULL;
			}
		}
	}

	return channel;
}


int intel_vvp_warp_init_instance(intel_vvp_warp_instance_t* instance, intel_vvp_warp_base_t base)
{
	int ret = -1;

	/* Sanity checks */
	assert(instance != NULL);

	instance->base = base;

	uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_REGMAP_BASE + INTEL_VVP_WARP_CONFIG);

#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
	uint32_t ver_min		= (reg_val & INTEL_VVP_WARP_CONFIG_VER_MINOR_MSK) >> INTEL_VVP_WARP_CONFIG_VER_MINOR_OFST;
	uint32_t ver_maj		= (reg_val & INTEL_VVP_WARP_CONFIG_VER_MAJOR_MSK) >> INTEL_VVP_WARP_CONFIG_VER_MAJOR_OFST;
	uint32_t warp_rt		= (reg_val & INTEL_VVP_WARP_CONFIG_WARP_RT);
	uint32_t raster_scan	= (reg_val & INTEL_VVP_WARP_CONFIG_RASTER_SCAN);
	static const char* output_bounce_str[] = {"none", "block", "raster", "per channel"};
	static const char* memory_map_str[] = {"SDTV", "HDTV", "4KUHD", "8KUHD"};
#endif /*INTEL_VVP_WARP_ENABLE_LOGGING*/

    uint32_t bit_depth		= (reg_val & INTEL_VVP_WARP_CONFIG_DEPTH_MSK) >> INTEL_VVP_WARP_CONFIG_DEPTH_OFST;
	uint32_t streams		= (reg_val & INTEL_VVP_WARP_CONFIG_STREAMS_MSK) >> INTEL_VVP_WARP_CONFIG_STREAMS_OFST;
	uint32_t num_inputs		= (reg_val & INTEL_VVP_WARP_CONFIG_NUM_INPUT_MSK) >> INTEL_VVP_WARP_CONFIG_NUM_INPUT_OFST;
	uint32_t num_outputs	= (reg_val & INTEL_VVP_WARP_CONFIG_NUM_OUTPUT_MSK) >> INTEL_VVP_WARP_CONFIG_NUM_OUTPUT_OFST;
	uint32_t num_engines	= (reg_val & INTEL_VVP_WARP_CONFIG_NUM_ENGINE_MSK) >> INTEL_VVP_WARP_CONFIG_NUM_ENGINE_OFST;
	uint32_t output_bounce	= (reg_val & INTEL_VVP_WARP_CONFIG_OBOUNCE_MSK) >> INTEL_VVP_WARP_CONFIG_OBOUNCE_OFST;
	uint32_t output_skip	= (reg_val & INTEL_VVP_WARP_CONFIG_OUTPUT_SKIP);
	uint32_t chr_aberration	= (reg_val & INTEL_VVP_WARP_CONFIG_CHROMA_ABERR_SUPPORT);

	INTEL_VVP_WARP_LOG("\nintel_vvp_warp v%"PRIu32".%"PRIu32"\n", ver_maj, ver_min);
	INTEL_VVP_WARP_LOG("bit depth:         %"PRIu32"\n", bit_depth);
	INTEL_VVP_WARP_LOG("streams:           %"PRIu32"\n", streams);
	INTEL_VVP_WARP_LOG("warp RT:           %s\n", warp_rt ? "yes":"no");
	INTEL_VVP_WARP_LOG("scan:              %s\n", raster_scan ? "raster":"block");
	INTEL_VVP_WARP_LOG("inputs:            %"PRIu32"\n", num_inputs);
	INTEL_VVP_WARP_LOG("outputs:           %"PRIu32"\n", num_outputs);
	INTEL_VVP_WARP_LOG("engines:           %"PRIu32"\n", num_engines);
	INTEL_VVP_WARP_LOG("output bounce:     %s\n", output_bounce_str[output_bounce]);
	INTEL_VVP_WARP_LOG("output skip:       %s\n", output_skip ? "yes":"no");
	INTEL_VVP_WARP_LOG("chroma aberration: %s\n", chr_aberration ? "yes":"no");

	reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_REGMAP_BASE + INTEL_VVP_WARP_CONFIG_EXT);

	uint32_t mem_if_width	= 0x1 << ((reg_val & INTEL_VVP_WARP_CONFIG_EXT_MEMIF_WIDTH_MSK) >> INTEL_VVP_WARP_CONFIG_EXT_MEMIF_WIDTH_OFST);
	uint32_t mem_map		= (reg_val & INTEL_VVP_WARP_CONFIG_EXT_MEM_MAP_MSK) >> INTEL_VVP_WARP_CONFIG_EXT_MEM_MAP_OFST;
	uint32_t mem_model		= (reg_val & INTEL_VVP_WARP_CONFIG_EXT_BANK_LOC_MSK) >> INTEL_VVP_WARP_CONFIG_EXT_BANK_LOC_OFST;
	uint32_t mipmap_enable	= (reg_val & INTEL_VVP_WARP_CONFIG_EXT_MIPMAP_ENABLE);

	INTEL_VVP_WARP_LOG("memory map:        %s\n", memory_map_str[mem_map]);
	INTEL_VVP_WARP_LOG("mipmap support:    %s\n", mipmap_enable ? "yes":"no");

	instance->output_bounce = output_bounce;
	instance->mem_map		= mem_map;
	instance->mem_model		= (mem_model == 3) ? 1 : 0;

	memset(instance->inputs, 0, sizeof(instance->inputs));
	memset(instance->outputs, 0, sizeof(instance->outputs));
	memset(instance->engines, 0, sizeof(instance->engines));
	memset(instance->channels, 0, sizeof(instance->channels));

	instance->num_inputs = (num_inputs > INTEL_VVP_WARP_MAX_INPUTS ? INTEL_VVP_WARP_MAX_INPUTS : num_inputs);
	instance->num_engines = (num_engines > INTEL_VVP_WARP_MAX_ENGINES ? INTEL_VVP_WARP_MAX_ENGINES : num_engines);
	instance->num_outputs = (num_outputs > INTEL_VVP_WARP_MAX_OUTPUTS ? INTEL_VVP_WARP_MAX_OUTPUTS : num_outputs);

	instance->cache_size_max = INTEL_VVP_WARP_CACHE_SIZE_MAX;

	instance->streams = (chr_aberration ? streams : 1);
	instance->bit_depth = bit_depth;
	instance->block_width = mem_if_width == 64 ? 8 : 16;
	instance->block_height = mem_if_width == 64 ? 4 : 8;
	instance->output_skip = (output_skip ? 1 : 0);
	instance->mipmap_enable = (mipmap_enable ? 1 : 0);

	for(uint32_t i = 0; i < instance->num_engines; ++i)
    {
         /* Stop all engines */
        uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_ENGINE_BASE(i) + INTEL_VVP_WARP_ENGINE_CONTROL);
        reg_val &= ~INTEL_VVP_WARP_ENGINE_CONTROL_EN;
        INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_ENGINE_BASE(i) + INTEL_VVP_WARP_ENGINE_CONTROL, reg_val);

        /* Print engine configuration */
#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
        print_engine_configuration(instance, i);
#endif /* INTEL_VVP_WARP_ENABLE_LOGGING */        
    }

	for(uint32_t i = 0; i < instance->num_outputs; ++i)
	{
        /* Check if outputs are easy warp capable */
		intel_vvp_warp_output_t* output = &(instance->outputs[i]);
		uint32_t reg_val = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_OUTPUT_BASE(i) + INTEL_VVP_WARP_OUTPUT_CONFIG);

		if(reg_val & INTEL_VVP_WARP_OUTPUT_CONFIG_EASY_WARP_CAPABLE)
			output->easy_warp = 1;

        /* Print output configuration */
#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
		print_output_configuration(instance, i);
#endif /*INTEL_VVP_WARP_ENABLE_LOGGING	*/            
	}    

	ret = 0;

	return ret;
}


/*
 * Frame capture extensions. Used in SoC demo
*/

static uint32_t assign_bit(uint32_t value, uint32_t bit_value, bool state)
{
	if (state)
		return value | bit_value;
	else
		return value &= ~bit_value;
}


bool intel_vvp_warp_enable_capture(intel_vvp_warp_instance_t* instance, bool state)
{
	/* Global input control first */
	uint32_t register_value = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_CONTROL);
	register_value = assign_bit(register_value, INTEL_VVP_WARP_INPUT_CONTROL_LOCK_BUFFER, state);
	INTEL_VVP_WARP_REG_IOWR(INTEL_VVP_WARP_INPUT_CONTROL, register_value);

	/* Enable capture on the channels */
	for (uint32_t i = 0; i < INTEL_VVP_WARP_MAX_CHANNELS; i++)
	{
		if (instance->channels[i].in_use)
		{
			intel_vvp_warp_input_t* input = instance->channels[i].input;
			uint32_t hw_idx = input->idx;

			uint32_t channel_input_control_register = INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_CONTROL;
			register_value = INTEL_VVP_WARP_REG_IORD(channel_input_control_register);
			register_value = assign_bit(register_value, INTEL_VVP_WARP_INPUT_CONTROL_LOCK_BUFFER, state);
			INTEL_VVP_WARP_REG_IOWR(channel_input_control_register, register_value);
		}
	}

	return true;
}


uint32_t intel_vvp_warp_get_locked_buffer_index(intel_vvp_warp_channel_t* channel)
{
	if (!channel)
		return 0;

	intel_vvp_warp_instance_t* instance = channel->instance;
	intel_vvp_warp_input_t* input = channel->input;
	uint32_t hw_idx = input->idx;
	uint32_t registerValue = INTEL_VVP_WARP_REG_IORD(INTEL_VVP_WARP_INPUT_BASE(hw_idx) + INTEL_VVP_WARP_INPUT_STATUS);
	uint32_t index = (registerValue >> 30) & 0x3;
	return index;
}


static const uint32_t capture_data_size = 0x4000000;

uint32_t intel_vvp_warp_get_capture_address(intel_vvp_warp_channel_t* channel, uint32_t buffer_index)
{
	if (!channel)
		return 0;

	return channel->ram_addr + capture_data_size * buffer_index;
}