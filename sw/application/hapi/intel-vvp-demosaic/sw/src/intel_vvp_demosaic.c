/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_demosaic.h"
#include "intel_vvp_demosaic_regs.h"


int intel_vvp_demosaic_init(intel_vvp_demosaic_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_DEMOSAIC_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_DEMOSAIC_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_DEMOSAIC_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpDemosaicRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_MAX_HEIGHT_REG);
        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_DEMOSAIC_REG_IOWR(instance, INTEL_VVP_DEMOSAIC_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_demosaic_get_lite_mode(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_demosaic_get_debug_enabled(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_demosaic_get_bits_per_sample_in(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_demosaic_get_bits_per_sample_out(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_demosaic_get_num_color_planes_in(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_demosaic_get_num_color_planes_out(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_demosaic_get_pixels_in_parallel(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_demosaic_get_max_width(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_demosaic_get_max_height(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

bool intel_vvp_demosaic_is_running(intel_vvp_demosaic_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_STATUS_REG);
    return INTEL_VVP_DEMOSAIC_GET_FLAG(reg, STATUS_RUNNING);
}

uint32_t intel_vvp_demosaic_get_status(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_STATUS_REG);
}

int intel_vvp_demosaic_get_frame_stats(intel_vvp_demosaic_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (stats_out == NULL) return kIntelVvpDemosaicPointerErr;

    *stats_out = INTEL_VVP_DEMOSAIC_REG_IORD(instance, INTEL_VVP_DEMOSAIC_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

bool intel_vvp_demosaic_get_bypass(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_DEMOSAIC_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_demosaic_set_bypass(intel_vvp_demosaic_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_DEMOSAIC_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_DEMOSAIC_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_DEMOSAIC_REG_IOWR(instance, INTEL_VVP_DEMOSAIC_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_demosaic_get_cfa_phase(intel_vvp_demosaic_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_DEMOSAIC_READ_FIELD(instance->settings_reg, SETTINGS_CFA_PHASE);
}

int intel_vvp_demosaic_set_cfa_phase(intel_vvp_demosaic_instance* instance, uint8_t cfa_phase)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cfa_phase > 3) return kIntelVvpDemosaicValueErr;

    INTEL_VVP_DEMOSAIC_WRITE_FIELD(instance->settings_reg, cfa_phase, SETTINGS_CFA_PHASE);
    INTEL_VVP_DEMOSAIC_REG_IOWR(instance, INTEL_VVP_DEMOSAIC_SETTINGS_REG, instance->settings_reg);
    return kIntelVvpCoreOk;
}
