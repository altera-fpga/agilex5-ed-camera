/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_clipper.h"

#include "intel_vvp_clipper_regs.h"

int intel_vvp_clipper_init(intel_vvp_clipper_instance *instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_CLIPPER_PRODUCT_ID);
    // Check the regmap version if the the intel_vvp_core_instance initialize without errors (ie, vendor_id and product_id validated properly)
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_CLIPPER_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_CLIPPER_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpClipperRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Load the clipper compile-time configuration
        instance->lite_mode      = (0 != INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_LITE_MODE_REG));
        instance->debug_enabled  = (0 != INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_DEBUG_ENABLED_REG));
        
        instance->clip_mode      = (eIntelVvpClippingMode)INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_CLIPPING_MODE_REG);
        if ((instance->clip_mode != kIntelVvpOffsetClipping) && (instance->clip_mode != kIntelVvpRectangleClipping))
        {
            instance->clip_mode = kIntelVvpInvalidClipping;
            init_ret = kIntelVvpClipperRegMapVersionErr;
        }
    }
    return init_ret;
}

// Clipper specific
bool intel_vvp_clipper_get_lite_mode(intel_vvp_clipper_instance *instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_clipper_get_debug_enabled(intel_vvp_clipper_instance *instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

eIntelVvpClippingMode intel_vvp_clipper_get_clipping_mode(intel_vvp_clipper_instance *instance)
{
    if (instance == NULL) return kIntelVvpInvalidClipping;

    return instance->clip_mode;
}

bool intel_vvp_clipper_is_running(intel_vvp_clipper_instance *instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_STATUS_REG);
    return INTEL_VVP_CLIPPER_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_clipper_get_commit_status(intel_vvp_clipper_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_STATUS_REG);
    return INTEL_VVP_CLIPPER_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_clipper_get_status(intel_vvp_clipper_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_STATUS_REG);
    return status_reg;
}

uint16_t intel_vvp_clipper_get_left_offset(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG);
}

uint16_t intel_vvp_clipper_get_top_offset(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG);
}

int intel_vvp_clipper_set_left_offset(intel_vvp_clipper_instance *instance, uint16_t left_offset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG, left_offset);

    return kIntelVvpCoreOk;
}

int intel_vvp_clipper_set_top_offset(intel_vvp_clipper_instance *instance, uint16_t top_offset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG, top_offset);

    return kIntelVvpCoreOk;
}


// Clipper specific, precondition clip_mode==OFFSET_MODE
uint16_t intel_vvp_clipper_get_right_offset(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpOffsetClipping)) return 0xFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_RIGHT_OFFSET_REG);
}

uint16_t intel_vvp_clipper_get_bottom_offset(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpOffsetClipping)) return 0xFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_BOTTOM_OFFSET_REG);
}

int intel_vvp_clipper_get_clip_offsets(intel_vvp_clipper_instance *instance,
                                        uint16_t *left_offset, uint16_t *top_offset, 
                                        uint16_t *right_offset, uint16_t *bottom_offset)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpOffsetClipping)) return kIntelVvpCoreInstanceErr;
    
    if ((left_offset == NULL) || (top_offset == NULL) ||
         (right_offset == NULL) || (bottom_offset == NULL)) return kIntelVvpCoreNullPtrErr;

    *left_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG);
    *top_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG);
    *right_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_RIGHT_OFFSET_REG);
    *bottom_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_BOTTOM_OFFSET_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_clipper_set_right_offset(intel_vvp_clipper_instance *instance, uint16_t right_offset)
{
    if ((instance == NULL) || (instance->clip_mode != kIntelVvpOffsetClipping)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_RIGHT_OFFSET_REG, right_offset);

    return kIntelVvpCoreOk;
}

int intel_vvp_clipper_set_bottom_offset(intel_vvp_clipper_instance *instance, uint16_t bottom_offset)
{
    if ((instance == NULL) || (instance->clip_mode != kIntelVvpOffsetClipping)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_BOTTOM_OFFSET_REG, bottom_offset);

    return kIntelVvpCoreOk;
}

int intel_vvp_clipper_set_clip_offsets(intel_vvp_clipper_instance *instance,
                                        uint16_t left_offset, uint16_t top_offset, 
                                        uint16_t right_offset, uint16_t bottom_offset)
{
    if ((instance == NULL) || (instance->clip_mode != kIntelVvpOffsetClipping))  return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG, left_offset);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG, top_offset);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_RIGHT_OFFSET_REG, right_offset);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_BOTTOM_OFFSET_REG, bottom_offset);

    return kIntelVvpCoreOk;
}



// Clipper specific, precondition clip_mode==RECTANGLE_MODE
uint32_t intel_vvp_clipper_get_clip_width(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpRectangleClipping)) return 0xFFFFFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_CLIP_WIDTH_REG);
}

uint32_t intel_vvp_clipper_get_clip_height(intel_vvp_clipper_instance *instance)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpRectangleClipping)) return 0xFFFFFFFF;

    return INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_CLIP_HEIGHT_REG);
}

int intel_vvp_clipper_get_clip_area(intel_vvp_clipper_instance *instance,
                                     uint16_t *left_offset, uint16_t *top_offset,
                                     uint32_t *clip_width, uint32_t *clip_height)
{
    if ((instance == NULL) || !instance->debug_enabled || (instance->clip_mode != kIntelVvpRectangleClipping)) return kIntelVvpCoreInstanceErr;
    
    if ((left_offset == NULL) || (top_offset == NULL) ||
         (clip_width == NULL) || (clip_height == NULL)) return kIntelVvpCoreNullPtrErr;

    *left_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG);
    *top_offset = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG);
    *clip_width = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_CLIP_WIDTH_REG);
    *clip_height = INTEL_VVP_CLIPPER_REG_IORD(instance, INTEL_VVP_CLIPPER_CLIP_HEIGHT_REG);

    return kIntelVvpCoreOk;
}


int intel_vvp_clipper_set_clip_width(intel_vvp_clipper_instance *instance, uint32_t clip_width)
{
    if ((instance == NULL) || (instance->clip_mode != kIntelVvpRectangleClipping)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_CLIP_WIDTH_REG, clip_width);

    return kIntelVvpCoreOk;
}


int intel_vvp_clipper_set_clip_height(intel_vvp_clipper_instance *instance, uint32_t clip_height)
{
    if ((instance == NULL) || (instance->clip_mode != kIntelVvpRectangleClipping)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_CLIP_HEIGHT_REG, clip_height);

    return kIntelVvpCoreOk;
}


int intel_vvp_clipper_set_clip_area(intel_vvp_clipper_instance *instance,
                                     uint16_t left_offset, uint16_t top_offset,
                                     uint32_t clip_width, uint32_t clip_height)
{
    if ((instance == NULL)  || (instance->clip_mode != kIntelVvpRectangleClipping)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_LEFT_OFFSET_REG, left_offset);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_TOP_OFFSET_REG, top_offset);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_CLIP_WIDTH_REG, clip_width);
    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_CLIP_HEIGHT_REG, clip_height);

    return kIntelVvpCoreOk;
}

int intel_vvp_clipper_commit_writes(intel_vvp_clipper_instance* instance)
{
    if ((instance == NULL)  || instance->lite_mode) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CLIPPER_REG_IOWR(instance, INTEL_VVP_CLIPPER_COMMIT_REG, 1);
    return kIntelVvpCoreOk;
}