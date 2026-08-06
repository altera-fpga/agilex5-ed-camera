/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_vfb.h"
#include "intel_vvp_vfb_regs.h"

int intel_vvp_vfb_init(intel_vvp_vfb_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint32_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_VFB_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_VFB_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_VFB_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpVfbRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode     = (0 != INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_DEBUG_ENABLED_REG));
        instance->max_width     = INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_MAX_WIDTH_REG);
        instance->max_height    = INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_MAX_HEIGHT_REG);
        instance->frame_drop_enabled = (0 != INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_DROP_ENABLED_REG));
        instance->frame_repeat_enabled = (0 != INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_REPEAT_ENABLED_REG));
        instance->invalid_frames_dropped = (0 != INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_INVALID_FRAMES_DROPPED_REG));
        
        // Stop the reader
        intel_vvp_vfb_stop_output(instance);
    }

    return init_ret;
}
 

bool intel_vvp_vfb_get_lite_mode(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->lite_mode;
}


bool intel_vvp_vfb_get_debug_enabled(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->debug_enabled;
}

uint32_t intel_vvp_vfb_get_max_width(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->max_width;
}

uint32_t intel_vvp_vfb_get_max_height(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->max_height;
}

bool intel_vvp_vfb_is_frame_drop_enabled(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->frame_drop_enabled;
}

bool intel_vvp_vfb_is_frame_repeat_enabled(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->frame_repeat_enabled;
}

bool intel_vvp_are_invalid_frames_dropped(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->invalid_frames_dropped;
}

uint32_t intel_vvp_vfb_get_mem_base_address(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_MEM_BASE_ADDR_REG);
}

uint32_t intel_vvp_vfb_get_buffer_stride(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_MEM_BUFFER_STRIDE_REG);
}

uint32_t intel_vvp_vfb_get_line_stride(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_MEM_LINE_STRIDE_REG);
}

uint8_t intel_vvp_vfb_get_bits_per_sample(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_BPS_REG);
}

uint8_t intel_vvp_vfb_get_num_color_planes(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUMBER_OF_COLOR_PLANES_REG);
}

uint8_t intel_vvp_vfb_get_pixels_in_parallel(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_PIXELS_IN_PARALLEL_REG);
}

eIntelVvpVfbPacking intel_vvp_vfb_get_mem_word_packing(intel_vvp_vfb_instance* instance)
{
    int32_t reg_read;
    
    if (instance == NULL) return kIntelVvpVfbInvalidPacking;
    
    reg_read = INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_PACKING_REG);
    
    switch (reg_read)
    {
        case INTEL_VVP_VFB_PERFECT_PACKING: return kIntelVvpVfbPerfectPacking;
        case INTEL_VVP_VFB_COLOR_PACKING: return kIntelVvpVfbColorPacking;
        case INTEL_VVP_VFB_PIXEL_PACKING: return kIntelVvpVfbPixelPacking;
        default: return kIntelVvpVfbInvalidPacking;
    }
}

bool intel_vvp_vfb_is_input_running(intel_vvp_vfb_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_INPUT_STATUS_REG);
    return INTEL_VVP_VFB_GET_FLAG(status_reg, INPUT_STATUS_RUNNING);
}

uint32_t intel_vvp_vfb_get_input_status(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_INPUT_STATUS_REG);
}

bool intel_vvp_vfb_is_output_running(intel_vvp_vfb_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_OUTPUT_STATUS_REG);
    return INTEL_VVP_VFB_GET_FLAG(status_reg, OUTPUT_STATUS_RUNNING);
}

uint32_t intel_vvp_vfb_get_output_status(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_OUTPUT_STATUS_REG);
}

uint32_t intel_vvp_vfb_get_num_input_fields(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUM_INPUT_FIELDS_REG);
}


uint32_t intel_vvp_vfb_get_num_dropped_fields(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUM_DROPPED_FIELDS_REG);
}


uint32_t intel_vvp_vfb_get_num_invalid_fields(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUM_INVALID_FIELDS_REG);
}

uint32_t intel_vvp_vfb_get_num_output_fields(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUM_OUTPUT_FIELDS_REG);
}


uint32_t intel_vvp_vfb_get_num_repeated_fields(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;
    
    return INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_NUM_REPEATED_FIELDS_REG);
}

int intel_vvp_vfb_output_enable(intel_vvp_vfb_instance* instance, bool enabled)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    INTEL_VVP_VFB_REG_IOWR(instance, INTEL_VVP_VFB_OUTPUT_CONTROL_REG, (enabled? INTEL_VVP_VFB_OUTPUT_CONTROL_GO_MSK : 0));
    
    return kIntelVvpCoreOk;
}

int intel_vvp_vfb_start_output(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    INTEL_VVP_VFB_REG_IOWR(instance, INTEL_VVP_VFB_OUTPUT_CONTROL_REG, INTEL_VVP_VFB_OUTPUT_CONTROL_GO_MSK);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_vfb_stop_output(intel_vvp_vfb_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    INTEL_VVP_VFB_REG_IOWR(instance, INTEL_VVP_VFB_OUTPUT_CONTROL_REG, 0);
    
    return kIntelVvpCoreOk;
}

bool intel_vvp_vfb_is_output_enabled(intel_vvp_vfb_instance* instance)
{
    if ((instance == NULL) || (!instance->debug_enabled)) return false;

    return (0 != (INTEL_VVP_VFB_REG_IORD(instance, INTEL_VVP_VFB_OUTPUT_CONTROL_REG) & INTEL_VVP_VFB_OUTPUT_CONTROL_GO_MSK));
}


