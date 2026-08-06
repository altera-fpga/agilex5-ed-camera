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
#include <stdio.h>
#include <inttypes.h>
#include "intel_vvp_tmo.h"
#include "intel_vvp_tmo_regs.h"

// Forward declarations of local functions
static bool update_flow_control(intel_vvp_tmo_instance_t* instance);
static void program_region_of_interest(intel_vvp_tmo_instance_t* instance);

int intel_vvp_tmo_init_instance(intel_vvp_tmo_instance_t* instance, intel_vvp_tmo_base_t base)
{
	int ret = -1;

	if(instance != NULL)
	{
        // Clear instance structure
        *instance = (intel_vvp_tmo_instance_t){0};

        instance->base = base;

        uint32_t reg_val = INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_VID_PID_REG);
		uint32_t vendor_id  = (reg_val & INTEL_VVP_CORE_VID_PID_VENDOR_ID_MSK) >> INTEL_VVP_CORE_VID_PID_VENDOR_ID_OFST;
		uint32_t product_id = (reg_val & INTEL_VVP_CORE_VID_PID_PRODUCT_ID_MSK) >> INTEL_VVP_CORE_VID_PID_PRODUCT_ID_OFST;

        uint32_t hw_info_0 = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_HW_INFO_0);
		uint32_t C_PIXELS = (hw_info_0 & INTEL_VVP_TMO_HW_INFO_0_PIXELS_MSK) >> INTEL_VVP_TMO_HW_INFO_0_PIXELS_OFST;
        instance->_pixels_in_parallel = C_PIXELS;

        if(vendor_id == INTEL_VVP_VENDOR_ID)
		{
			if(product_id == INTEL_VVP_TMO_PRODUCT_ID)
			{
				reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_HW_INFO_0);

				INTEL_VVP_TMO_LOG("TILES:\t\t%"PRIu32"\n",	(reg_val & INTEL_VVP_TMO_HW_INFO_0_TILE_MSK)	>> INTEL_VVP_TMO_HW_INFO_0_TILE_OFST);
				INTEL_VVP_TMO_LOG("BPP:\t\t%"PRIu32"\n",	(reg_val & INTEL_VVP_TMO_HW_INFO_0_DEPTH_MSK)	>> INTEL_VVP_TMO_HW_INFO_0_DEPTH_OFST);
				INTEL_VVP_TMO_LOG("PPC:\t\t%"PRIu32"\n",	(reg_val & INTEL_VVP_TMO_HW_INFO_0_PIXELS_MSK)	>> INTEL_VVP_TMO_HW_INFO_0_PIXELS_OFST);
				ret = 0;

                // Get the state of instance->_bypass
                intel_vvp_tmo_get_bypass(instance);

                // Set default state of the ROI
                intel_vvp_tmo_enable_region_of_interest(instance, false);
                intel_vvp_tmo_set_region_of_interest_outside(instance, false);                
			}
			else
				ret = -2;	/* Product mismatch */
		}
		else
			ret = -1;	/* Vendor mismatch */
	}

	return ret;
}


void intel_vvp_tmo_set_resolution(intel_vvp_tmo_instance_t* instance, uint32_t width, uint32_t height)
{
	if(instance != NULL)
	{
        instance->_width = width;
        instance->_height = height;
        const uint32_t hw_info_0 = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_HW_INFO_0);
        const uint32_t hw_info_2 = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_HW_INFO_2);

		const uint32_t C_TILES_HW = (hw_info_0 & INTEL_VVP_TMO_HW_INFO_0_TILE_MSK) >> INTEL_VVP_TMO_HW_INFO_0_TILE_OFST;
		const uint32_t C_TILES	= C_TILES_HW ? C_TILES_HW : 1;
		const uint32_t C_PIXELS_HW = (hw_info_0 & INTEL_VVP_TMO_HW_INFO_0_PIXELS_MSK) >> INTEL_VVP_TMO_HW_INFO_0_PIXELS_OFST;
		const uint32_t C_PIXELS	= C_PIXELS_HW ? C_PIXELS_HW : 1;
		const uint32_t FRAC_PREC_NORM_FACT =	(hw_info_2 & INTEL_VVP_TMO_HW_INFO_2_NORM_FACT_MSK) >> INTEL_VVP_TMO_HW_INFO_2_NORM_FACT_OFST;

		const uint32_t TILE_WIDTH_FLOOR = width / C_TILES;
		const uint32_t TILE_HEIGHT_FLOOR = height / C_TILES;
		const uint32_t TILE_WIDTH = TILE_WIDTH_FLOOR % 2 == 0 ? TILE_WIDTH_FLOOR : TILE_WIDTH_FLOOR + 1;
        const uint32_t TILE_HEIGHT = TILE_HEIGHT_FLOOR % 2 == 0 ? TILE_HEIGHT_FLOOR : TILE_HEIGHT_FLOOR + 1;

		const uint32_t cpu_dim_x = TILE_WIDTH / C_PIXELS;
		const uint32_t cpu_dim_y = TILE_HEIGHT;
		const uint32_t cpu_dim_txcorner = TILE_WIDTH / 2;
		const uint32_t cpu_dim_tycorner = TILE_HEIGHT / 2;
		const uint32_t cpu_dim_txcentre = TILE_WIDTH;
		const uint32_t cpu_dim_tycentre = TILE_HEIGHT;

		const uint32_t cpu_normf_tcorner	= (128 * (1 << FRAC_PREC_NORM_FACT)) / (cpu_dim_txcorner * cpu_dim_tycorner);
		const uint32_t cpu_normf_txside		= (128 * (1 << FRAC_PREC_NORM_FACT)) / (cpu_dim_txcentre * cpu_dim_tycorner);
		const uint32_t cpu_normf_tyside		= (128 * (1 << FRAC_PREC_NORM_FACT)) / (cpu_dim_txcorner * cpu_dim_tycentre);
		const uint32_t cpu_normf_tcentre	= (128 * (1 << FRAC_PREC_NORM_FACT)) / (cpu_dim_txcentre * cpu_dim_tycentre);

		const uint32_t cpu_x_tdim_hgen_1m2	= cpu_dim_x - 2;
		const uint32_t cpu_x_tdim_hgen_2m2	= cpu_dim_x * 2 - 2;
		const uint32_t cpu_x_tdim_hgen_3m2	= cpu_dim_x * 3 - 2;
		const uint32_t cpu_x_tdim_hgen_4m1	= cpu_dim_x * 4 - 1;
		const uint32_t cpu_x_tdim_hgen_4m2	= cpu_dim_x * 4 - 2;

		const uint32_t cpu_y_tdim_hgen_1	= cpu_dim_y;
		const uint32_t cpu_y_tdim_hgen_1m1	= cpu_y_tdim_hgen_1 - 1;
		const uint32_t cpu_y_tdim_hgen_2	= cpu_dim_y * 2;
		const uint32_t cpu_y_tdim_hgen_2m1	= cpu_y_tdim_hgen_2 - 1;
		const uint32_t cpu_y_tdim_hgen_3	= cpu_dim_y * 3;
		const uint32_t cpu_y_tdim_hgen_3m1	= cpu_y_tdim_hgen_3 - 1;
		const uint32_t cpu_y_tdim_hgen_4	= cpu_dim_y * 4;
		const uint32_t cpu_y_tdim_hgen_4m1	= cpu_y_tdim_hgen_4 - 1;

		const uint32_t cpu_x_tdim_heq_1		= cpu_dim_txcentre >> 1;
		const uint32_t cpu_y_tdim_heq_1		= cpu_dim_tycentre >> 1;
		const uint32_t cpu_x_tdim_heq_2		= cpu_x_tdim_heq_1 + cpu_dim_txcentre;
		const uint32_t cpu_y_tdim_heq_2		= cpu_y_tdim_heq_1 + cpu_dim_tycentre;
		const uint32_t cpu_x_tdim_heq_3		= cpu_x_tdim_heq_2 + cpu_dim_txcentre;
		const uint32_t cpu_y_tdim_heq_3		= cpu_y_tdim_heq_2 + cpu_dim_tycentre;
		const uint32_t cpu_x_tdim_heq_4		= cpu_x_tdim_heq_3 + cpu_dim_txcentre;
		const uint32_t cpu_y_tdim_heq_4		= cpu_y_tdim_heq_3 + cpu_dim_tycentre;

		const uint32_t  cpu_xm1_tdim_heq_1	= cpu_x_tdim_heq_1 - 1;
		const uint32_t  cpu_xm1_tdim_heq_2	= cpu_x_tdim_heq_2 - 1;
		const uint32_t  cpu_xm1_tdim_heq_3	= cpu_x_tdim_heq_3 - 1;
		const uint32_t  cpu_xm1_tdim_heq_4	= cpu_x_tdim_heq_4 - 1;
		const uint32_t  cpu_xm1_tdim_heq_5	= width  - 1;

		const uint32_t  cpu_xm2_tdim_heq_1	= cpu_x_tdim_heq_1 - 2;
		const uint32_t  cpu_xm2_tdim_heq_2	= cpu_x_tdim_heq_2 - 2;
		const uint32_t  cpu_xm2_tdim_heq_3	= cpu_x_tdim_heq_3 - 2;
		const uint32_t  cpu_xm2_tdim_heq_4	= cpu_x_tdim_heq_4 - 2;
		const uint32_t  cpu_xm2_tdim_heq_5	= width  - 2;

		const uint32_t  cpu_xm3_tdim_heq_1	= cpu_x_tdim_heq_1 - 3;
		const uint32_t  cpu_xm3_tdim_heq_2	= cpu_x_tdim_heq_2 - 3;
		const uint32_t  cpu_xm3_tdim_heq_3	= cpu_x_tdim_heq_3 - 3;
		const uint32_t  cpu_xm3_tdim_heq_4	= cpu_x_tdim_heq_4 - 3;
		const uint32_t  cpu_xm3_tdim_heq_5	= width  - 3;

		const uint32_t  cpu_xm4_tdim_heq_1	= cpu_x_tdim_heq_1 - 4;
		const uint32_t  cpu_xm4_tdim_heq_2	= cpu_x_tdim_heq_2 - 4;
		const uint32_t  cpu_xm4_tdim_heq_3	= cpu_x_tdim_heq_3 - 4;
		const uint32_t  cpu_xm4_tdim_heq_4	= cpu_x_tdim_heq_4 - 4;
		const uint32_t  cpu_xm4_tdim_heq_5	= width  - 4;

		const uint32_t  cpu_ym1_tdim_heq_1	= cpu_y_tdim_heq_1 - 1;
		const uint32_t  cpu_ym1_tdim_heq_2	= cpu_y_tdim_heq_2 - 1;
		const uint32_t  cpu_ym1_tdim_heq_3	= cpu_y_tdim_heq_3 - 1;
		const uint32_t  cpu_ym1_tdim_heq_4	= cpu_y_tdim_heq_4 - 1;

		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_ACT_GEOM,				((width << INTEL_VVP_TMO_ACT_GEOM_WIDTH_OFST) & INTEL_VVP_TMO_ACT_GEOM_WIDTH_MASK) | ((height << INTEL_VVP_TMO_ACT_GEOM_HEIGHT_OFST) & INTEL_VVP_TMO_ACT_GEOM_HEIGHT_MASK));
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_CENTRE_TILE_WIDTH,	(TILE_WIDTH << INTEL_VVP_TMO_CENTRE_TILE_WIDTH_OFST) & INTEL_VVP_TMO_CENTRE_TILE_WIDTH_MASK);

		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_CORNER_TILE_NORM,		cpu_normf_tcorner);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_X_TILE_NORM,			cpu_normf_txside);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_Y_TILE_NORM,			cpu_normf_tyside);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_CENTRE_TILE_NORM,		cpu_normf_tcentre);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_ACT_TILE_DIM,			(cpu_dim_x << INTEL_VVP_TMO_ACT_TILE_DIM_WIDTH_OFST) | cpu_dim_y);

		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_00,		(cpu_x_tdim_hgen_1m2 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_x_tdim_hgen_2m2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_01,		(cpu_x_tdim_hgen_3m2 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_x_tdim_hgen_4m1);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_02,		(cpu_x_tdim_hgen_4m2 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_hgen_1);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_03,		(cpu_y_tdim_hgen_1m1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_hgen_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_04,		(cpu_y_tdim_hgen_2m1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_hgen_3);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_05,		(cpu_y_tdim_hgen_3m1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_hgen_4);

		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_06,		(cpu_y_tdim_hgen_4m1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_x_tdim_heq_1);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_07,		(   cpu_x_tdim_heq_2 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_x_tdim_heq_3);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_08,		(   cpu_x_tdim_heq_4 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_heq_1);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_09,		(   cpu_y_tdim_heq_2 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_heq_3);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_10,		(  		(height - 1) << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_y_tdim_heq_4);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_11,		( cpu_xm1_tdim_heq_1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm1_tdim_heq_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_12,		( cpu_xm1_tdim_heq_3 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm1_tdim_heq_4);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_13,		( cpu_xm2_tdim_heq_1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm2_tdim_heq_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_14,		( cpu_xm2_tdim_heq_3 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm2_tdim_heq_4);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_15,		( cpu_xm3_tdim_heq_1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm3_tdim_heq_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_16,		( cpu_xm3_tdim_heq_3 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm3_tdim_heq_4);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_17,		( cpu_xm4_tdim_heq_1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm4_tdim_heq_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_18,		( cpu_xm4_tdim_heq_3 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm4_tdim_heq_4);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_19,		( cpu_xm1_tdim_heq_5 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm2_tdim_heq_5);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_20,		( cpu_xm3_tdim_heq_5 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_xm4_tdim_heq_5);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_21,		( cpu_ym1_tdim_heq_1 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_ym1_tdim_heq_2);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_BORDER_INFO_22,		( cpu_ym1_tdim_heq_3 << INTEL_VVP_TMO_BORDER_INFO_GROUP_0_OFST) | cpu_ym1_tdim_heq_4);

        program_region_of_interest(instance);
    }
}


void intel_vvp_tmo_set_bypass(intel_vvp_tmo_instance_t* instance, uint32_t bypass)
{
	if(instance != NULL)
	{
        instance->_bypass = bypass ? true : false;
        update_flow_control(instance);
	}
}


void intel_vvp_tmo_set_volume(intel_vvp_tmo_instance_t* instance, uint32_t volume)
{
	if(instance != NULL)
	{
		uint32_t reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_VOL_CTL);
		reg_val &= ~(INTEL_VVP_TMO_VOL_CTL_VOLUME_MASK);
		reg_val |= ((volume << INTEL_VVP_TMO_VOL_CTL_VOLUME_OFST) & INTEL_VVP_TMO_VOL_CTL_VOLUME_MASK);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_VOL_CTL, reg_val);
	}
}


void intel_vvp_tmo_set_threshold(intel_vvp_tmo_instance_t* instance, uint32_t threshold)
{
	if(instance != NULL)
	{
		uint32_t reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_VOL_CTL);
		reg_val &= ~(INTEL_VVP_TMO_VOL_CTL_THRESHOLD_MASK);
		reg_val |= ((threshold << INTEL_VVP_TMO_VOL_CTL_THRESHOLD_OFST) & INTEL_VVP_TMO_VOL_CTL_THRESHOLD_MASK);
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_VOL_CTL, reg_val);
	}
}


uint32_t intel_vvp_tmo_get_bypass(intel_vvp_tmo_instance_t* instance)
{
	uint32_t reg_val = 0x0;

	if(instance != NULL)
	{
		reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_FLOW_CTL);
		reg_val &= INTEL_VVP_TMO_FLOW_CTL_PASSTHROUGH_MASK;

        instance->_bypass = ((reg_val & 0x1) != 0);
        return instance->_bypass ? 1 : 0;
    }
    else
        return 0;
}


uint32_t intel_vvp_tmo_get_volume(intel_vvp_tmo_instance_t* instance)
{
	uint32_t reg_val = 0x0;

	if(instance != NULL)
	{
		reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_VOL_CTL);
		reg_val = ((reg_val & INTEL_VVP_TMO_VOL_CTL_VOLUME_MASK) >> INTEL_VVP_TMO_VOL_CTL_VOLUME_OFST);
	}

	return reg_val;
}


uint32_t intel_vvp_tmo_get_threshold(intel_vvp_tmo_instance_t* instance)
{
	uint32_t reg_val = 0x0;

	if(instance != NULL)
	{
		reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_VOL_CTL);
		reg_val = ((reg_val & INTEL_VVP_TMO_VOL_CTL_THRESHOLD_MASK) >> INTEL_VVP_TMO_VOL_CTL_THRESHOLD_OFST);
	}

	return reg_val;
}

uint32_t intel_vvp_tmo_get_debug(intel_vvp_tmo_instance_t* instance)
{
	uint32_t reg_val = 0x0;

	if(instance != NULL)
	{
		reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_DBG_INFO);
	}

	return reg_val;
}

void intel_vvp_tmo_set_debug(intel_vvp_tmo_instance_t* instance, uint32_t val)
{
	if(instance != NULL)
	{
		INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_DBG_INFO, val);
	}
}

typedef struct 
{
    uint32_t _x;
    uint32_t _y;
    uint32_t _width;
    uint32_t _height;
} intel_vvp_tmo_roi_pixels;

static intel_vvp_tmo_roi_pixels get_roi_pixels(intel_vvp_tmo_instance_t* instance)
{
    if (!instance)
        return (intel_vvp_tmo_roi_pixels){0};

    intel_vvp_tmo_roi* roi = &instance->_region_of_interest;
    intel_vvp_tmo_roi_pixels roi_pixels;
    float width_unit = instance->_width / (float)instance->_pixels_in_parallel;
    roi_pixels._x = (uint32_t)(roi->_x * width_unit + 0.5f);
    roi_pixels._width  = (uint32_t)(roi->_width * width_unit + 0.5f);
    roi_pixels._y      = (uint32_t)(roi->_y * instance->_height + 0.5f);
    roi_pixels._height = (uint32_t)(roi->_height * instance->_height + 0.5f);
    return roi_pixels;
}

static void program_region_of_interest(intel_vvp_tmo_instance_t* instance)
{
    if (instance)
    {
        intel_vvp_tmo_roi_pixels roi_pixels = get_roi_pixels(instance);
        uint32_t x0 = roi_pixels._x;
        uint32_t x1 = x0 + roi_pixels._width;
        uint32_t y0 = roi_pixels._y;
        uint32_t y1 = y0 + roi_pixels._height;
        uint32_t reg_val = ((x0 & 0xffff) << 16) | (x1 & 0x3fff);
        INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_ROI_X0X1, reg_val);

        reg_val = ((y0 & 0xffff) << 16) | (y1 & 0x3fff);
        INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_ROI_Y0Y1, reg_val);
    }
}

bool intel_vvp_tmo_set_region_of_interest(intel_vvp_tmo_instance_t* instance, 
                                          intel_vvp_tmo_roi roi)
{
    if (!instance)
        return false;

    if ((roi._x < 0.0f) || (roi._y < 0.0f) || (roi._width < 0.0f) || (roi._height < 0.0f))
        return false;
        
    if ((roi._x + roi._width) > 1.0f)
        return false;

    if ((roi._y + roi._height) > 1.0f)
        return false;

    instance->_region_of_interest = roi;

    if (instance->_enable_region_of_interest)
    {
        program_region_of_interest(instance);
    }

    return true;
}      

static bool update_flow_control(intel_vvp_tmo_instance_t* instance)
{
    if (!instance)
        return false;

    uint32_t reg_val = INTEL_VVP_TMO_REG_IORD(instance, INTEL_VVP_TMO_FLOW_CTL);
    reg_val &= ~7;

    if (instance->_enable_region_of_interest)
        reg_val |= 0x2;

    if (instance->_outside_region_of_interest)
        reg_val |= 0x4;

    if (instance->_bypass)
        reg_val |= 1;

    INTEL_VVP_TMO_REG_IOWR(instance, INTEL_VVP_TMO_FLOW_CTL, reg_val);
    return true;
}

bool intel_vvp_tmo_enable_region_of_interest(intel_vvp_tmo_instance_t* instance, bool enable)
{
    if (!instance)
        return false;

    instance->_enable_region_of_interest = enable;

    if (instance->_enable_region_of_interest)
    {
        program_region_of_interest(instance);
    }

    return update_flow_control(instance);
}

bool intel_vvp_tmo_set_region_of_interest_outside(intel_vvp_tmo_instance_t* instance, bool outside)
{
    if (!instance)
        return false;

    instance->_outside_region_of_interest = outside;
    return update_flow_control(instance);
}

bool intel_vvp_tmo_get_region_of_interest(intel_vvp_tmo_instance_t* instance,
                                          intel_vvp_tmo_roi* roi,
                                          bool* enabled,
                                          bool* outside)
{
    if (!instance)
        return false;

    if (roi)
        *roi = instance->_region_of_interest;

    if (enabled)
        *enabled = instance->_enable_region_of_interest;

    if (outside)
        *outside = instance->_outside_region_of_interest;

    return true;
}



