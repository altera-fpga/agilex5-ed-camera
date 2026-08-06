/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_usm.h"
#include "intel_vvp_usm_regs.h"


int intel_vvp_usm_init(intel_vvp_usm_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_USM_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_USM_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_USM_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpUsmRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_DEBUG_ENABLED_REG));
        instance->bps                      = (uint8_t)INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_BPS_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_MAX_HEIGHT_REG);
        // Internal states
        instance->strength_reg             = (uint32_t)1;
        INTEL_VVP_USM_REG_IOWR(instance, INTEL_VVP_USM_STRENGTH_REG, instance->strength_reg);
    }


    return init_ret;
}

bool intel_vvp_usm_get_lite_mode(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_usm_get_debug_enabled(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_usm_get_bits_per_sample(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps;
}

uint8_t intel_vvp_usm_get_pixels_in_parallel(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_usm_get_max_width(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_usm_get_max_height(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

bool intel_vvp_usm_is_running(intel_vvp_usm_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_STATUS_REG);
    return INTEL_VVP_USM_GET_FLAG(status_reg, STATUS_RUNNING);
}

uint8_t intel_vvp_usm_get_status(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_STATUS_REG);
}

uint32_t intel_vvp_usm_get_sharpening_strength(intel_vvp_usm_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_USM_REG_IORD(instance, INTEL_VVP_USM_STRENGTH_REG) & INTEL_VVP_USM_STRENGTH_MSK;
}

int intel_vvp_usm_set_sharpening_strength(intel_vvp_usm_instance* instance, uint16_t strength)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_USM_REG_IOWR(instance, INTEL_VVP_USM_STRENGTH_REG, strength & INTEL_VVP_USM_STRENGTH_MSK);

    return kIntelVvpCoreOk;
}

bool intel_vvp_usm_set_output_height(intel_vvp_usm_instance* instance, uint32_t new_height)
{
    if (intel_vvp_core_set_img_info_height(instance, new_height) == kIntelVvpCoreOk)
    {
        return true;
    }

    return false;
}
bool intel_vvp_usm_set_output_width(intel_vvp_usm_instance* instance, uint32_t new_width)
{
    if (intel_vvp_core_set_img_info_width(instance, new_width) == kIntelVvpCoreOk)
    {
        return true;
    }

    return false;
}
