/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_VVP_WARP_H__
#define __INTEL_VVP_WARP_H__

#include "intel_vvp_warp_io.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
#include <stdio.h>
#define INTEL_VVP_WARP_LOG(...)	printf(__VA_ARGS__)
#else
#define INTEL_VVP_WARP_LOG(...)
#endif /*INTEL_VVP_WARP_ENABLE_LOGGING*/


#define INTEL_VVP_WARP_MAX_INPUTS			(8)
#define INTEL_VVP_WARP_MAX_OUTPUTS			(8)
#define INTEL_VVP_WARP_MAX_ENGINES			(4)
#define INTEL_VVP_WARP_MAX_CHANNELS			(8)

#define INTEL_VVP_WARP_MAX_CHANNEL_ENGINES	(4)


struct intel_vvp_warp_instance;


typedef struct intel_vvp_warp_input
{
	uint32_t in_use;
	uint32_t idx;
} intel_vvp_warp_input_t;


typedef struct intel_vvp_warp_output
{
	uint32_t in_use;
	uint32_t idx;
	uint32_t easy_warp;
} intel_vvp_warp_output_t;


typedef struct intel_vvp_warp_engine
{
	uint32_t in_use;
	uint32_t idx;
} intel_vvp_warp_engine_t;


typedef enum intel_vvp_warp_pixel_filter
{
	ENotAvailable = 0,
	EBilinear,
	EBicubic
} intel_vvp_warp_pixel_filter_t;


typedef struct intel_vvp_warp_channel
{
	uint32_t in_use;
	uint32_t idx;
	struct intel_vvp_warp_instance* instance;
	intel_vvp_warp_input_t* input;
	intel_vvp_warp_engine_t* engines[INTEL_VVP_WARP_MAX_CHANNEL_ENGINES];
	intel_vvp_warp_output_t* output;

	uint32_t num_engines;
	uint32_t ram_addr;
	uint32_t width_output;
	uint32_t height_output;
	uint32_t output_block_scan;
	
	/*
	 * All engines within a warp channel must have the same capabilities
	 * e.g. the pixel filter.
	 * For some like the block cache size the software will use
	 * the smallest among all
	 */
	intel_vvp_warp_pixel_filter_t pixel_filter;
	uint32_t block_cache_size;	
} intel_vvp_warp_channel_t;


typedef enum intel_vvp_warp_cs
{
	EYUV = 0,
	ERGB_REDUCED,
	ERGB_FULL
} intel_vvp_warp_cs_t;


typedef struct intel_vvp_warp_channel_config
{
	uint32_t ram_addr;
	intel_vvp_warp_cs_t cs;
	uint32_t width_input;
	uint32_t height_input;
	uint32_t width_output;
	uint32_t height_output;
	uint8_t lfr;			/* When set to 1 the hardware will lower the output frame rate if unable to keep up */

} intel_vvp_warp_channel_config_t;


/*
 * Instance structure defintion. Each instance of the driver uses one
 * of these structures to hold its associated configuration and state.
*/
typedef struct intel_vvp_warp_instance
{
	intel_vvp_warp_base_t base;

	intel_vvp_warp_input_t inputs[INTEL_VVP_WARP_MAX_INPUTS];
	intel_vvp_warp_output_t outputs[INTEL_VVP_WARP_MAX_OUTPUTS];
	intel_vvp_warp_engine_t engines[INTEL_VVP_WARP_MAX_ENGINES];
	intel_vvp_warp_channel_t channels[INTEL_VVP_WARP_MAX_CHANNELS];

	uint32_t num_inputs;
	uint32_t num_engines;
	uint32_t num_outputs;

	uint32_t output_bounce;
	uint32_t mem_map;			/* SD, HD, UHD or 8KUHD */
	uint32_t mem_model;			/* Linear / scattered */

	uint32_t streams;
	uint32_t block_width;
	uint32_t block_height;

	uint32_t output_skip;
	uint32_t mipmap_enable;

	uint32_t cache_size_max;

} intel_vvp_warp_instance_t;


typedef struct intel_vvp_warp_engine_data
{
	uint32_t mesh_addr;
	uint32_t mesh_size;
	uint32_t filter_addr;
	uint32_t filter_size;
	uint32_t fetch_addr;
	uint32_t fetch_size;
	uint32_t start_h;
	uint32_t start_v;
	uint32_t end_h;
	uint32_t end_v;
	uint32_t mesh_stride;
} intel_vvp_warp_engine_data_t;


typedef struct intel_vvp_warp_data
{
	uint32_t num_engines;
    const uint8_t* skip_ram_input;
	const uint8_t* skip_megablock_data;
	uint32_t skip_ram_page;
    uint32_t mipmap_level;
    uint32_t mesh_step;
	intel_vvp_warp_engine_data_t engine_data[0];
} intel_vvp_warp_data_t;


int intel_vvp_warp_init_instance(intel_vvp_warp_instance_t *instance, intel_vvp_warp_base_t base);

intel_vvp_warp_channel_t* intel_vvp_warp_create_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t engine_idx, uint32_t output_idx);
intel_vvp_warp_channel_t* intel_vvp_warp_create_double_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t engine1_idx, uint32_t engine2_idx, uint32_t output_idx);
intel_vvp_warp_channel_t* intel_vvp_warp_create_quad_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t output_idx);
intel_vvp_warp_channel_t* intel_vvp_warp_create_easy_warp_channel(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t output_idx);
int intel_vvp_warp_add_engine_to_channel(intel_vvp_warp_channel_t* channel, uint32_t engine_idx);
void intel_vvp_warp_free_channel(intel_vvp_warp_channel_t* channel);
bool intel_vvp_warp_enable_capture(intel_vvp_warp_instance_t *instance, bool state);

int intel_vvp_warp_configure_channel(intel_vvp_warp_channel_t* channel, intel_vvp_warp_channel_config_t* cfg);
int intel_vvp_warp_apply_transform(intel_vvp_warp_channel_t* channel, intel_vvp_warp_data_t* data);
int intel_vvp_warp_bypass(intel_vvp_warp_channel_t* channel, uint32_t bypass, uint32_t skip_ram_page, uint32_t width, uint32_t height);

void intel_vvp_warp_debug_block(intel_vvp_warp_channel_t* channel, uint32_t v, uint32_t h);
void intel_vvp_warp_debug_block_disable(intel_vvp_warp_channel_t* channel);
void intel_vvp_warp_freerun_enable(intel_vvp_warp_channel_t* channel);
void intel_vvp_warp_freerun_disable(intel_vvp_warp_channel_t* channel);
void intel_vvp_warp_lfr(intel_vvp_warp_channel_t* channel, uint32_t enable);

void intel_vvp_warp_reset_skip_ram(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page);
void intel_vvp_warp_reset_input_skip_ram(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page);

int intel_vvp_warp_check_easy_warp_capable(intel_vvp_warp_channel_t* channel);
int intel_vvp_warp_set_easy_warp(intel_vvp_warp_channel_t* channel, uint32_t easy_warp);

void intel_vvp_warp_set_output_latency(intel_vvp_warp_channel_t* channel, uint32_t clock_offset);

uint32_t intel_vvp_warp_get_channel_framebuffer_size(intel_vvp_warp_channel_t* channel);
uint32_t intel_vvp_warp_get_channel_ram_size(intel_vvp_warp_channel_t* channel);

/* Access to the debug registers */
uint32_t intel_vvp_warp_get_debug_register(intel_vvp_warp_instance_t *instance, uint32_t reg_offset);
uint32_t intel_vvp_warp_get_input_debug_register(intel_vvp_warp_instance_t *instance, uint32_t input_idx, uint32_t reg_offset);
uint32_t intel_vvp_warp_get_output_debug_register(intel_vvp_warp_instance_t *instance, uint32_t output_idx, uint32_t reg_offset);
uint32_t intel_vvp_warp_get_engine_debug_register(intel_vvp_warp_instance_t *instance, uint32_t engine_idx, uint32_t reg_offset);

/* Get a status flag */
uint32_t intel_vvp_warp_get_engine_status(intel_vvp_warp_instance_t *instance, uint32_t engine_idx);
uint32_t intel_vvp_warp_get_output_status(intel_vvp_warp_instance_t *instance, uint32_t output_idx);

uint32_t intel_vvp_warp_get_locked_buffer_index(intel_vvp_warp_channel_t *channel);
uint32_t intel_vvp_warp_get_capture_address(intel_vvp_warp_channel_t *channel, uint32_t buffer_index);

int intel_vvp_warp_apply_preset_4K(intel_vvp_warp_channel_t* channel, uint32_t data_base);
void intel_vvp_warp_prepare_preset_4K(intel_vvp_warp_channel_t* channel, uint32_t skip_ram_page);

int intel_vvp_warp_apply_preset_8K(intel_vvp_warp_channel_t* channel, uint32_t data_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INTEL_VVP_WARP_H__ */
