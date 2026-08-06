/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_anr.h"
#include "intel_vvp_anr_regs.h"


int intel_vvp_anr_init(intel_vvp_anr_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_ANR_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_ANR_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_ANR_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpAnrRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_BPS_OUT_REG);
        instance->num_color_planes         = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_NUM_COLOR_PLANES_REG);
        instance->cfa_enabled              = (0 != INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_CFA_ENABLED_REG));
        instance->pip                      = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_MAX_WIDTH_REG);
        instance->num_h_taps               = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_NUM_H_TAPS_REG);
        instance->num_v_taps               = (uint8_t)INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_NUM_V_TAPS_REG);
        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        {
            if (instance->num_h_taps && instance->num_v_taps)
            {
                uint8_t spatial_lut_divider = (instance->cfa_enabled) ? (4) : (2);
                instance->spatial_lut_depth = ((instance->num_h_taps - 1) / spatial_lut_divider) +
                                              ((instance->num_v_taps - 1) / spatial_lut_divider);
            }
            else
            {
                instance->spatial_lut_depth = 0;
            }
        }
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_anr_get_lite_mode(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_anr_get_debug_enabled(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_anr_get_bits_per_sample_in(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_anr_get_bits_per_sample_out(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_anr_get_num_color_planes(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_planes;
}

bool intel_vvp_anr_get_cfa_enabled(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return false;

    return instance->cfa_enabled;
}

uint8_t intel_vvp_anr_get_pixels_in_parallel(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_anr_get_max_width(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint8_t intel_vvp_anr_get_num_h_taps(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_h_taps;
}

uint8_t intel_vvp_anr_get_num_v_taps(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_v_taps;
}

bool intel_vvp_anr_is_running(intel_vvp_anr_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_STATUS_REG);
    return INTEL_VVP_ANR_GET_FLAG(reg, STATUS_RUNNING);
}

bool intel_vvp_anr_commit_is_pending(intel_vvp_anr_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_STATUS_REG);
    return INTEL_VVP_ANR_GET_FLAG(reg, STATUS_COMMIT_PENDING);
}

uint32_t intel_vvp_anr_get_status(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_STATUS_REG);
}

int intel_vvp_anr_get_frame_stats(intel_vvp_anr_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (stats_out == NULL) return kIntelVvpAnrPointerErr;

    *stats_out = INTEL_VVP_ANR_REG_IORD(instance, INTEL_VVP_ANR_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_anr_commit(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}

bool intel_vvp_anr_get_bypass(intel_vvp_anr_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_ANR_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_anr_set_bypass(intel_vvp_anr_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_ANR_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_ANR_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_SETTINGS_REG, instance->settings_reg);

    return (intel_vvp_anr_commit_is_pending(instance)) ? kIntelVvpAnrCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_anr_write_intensity_lut_entry(intel_vvp_anr_instance* instance, uint32_t data, uint32_t addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (addr < INTEL_VVP_ANR_INTENSITY_LUT_ENTRIES) {
        INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_INTENSITY_LUT_BASE + addr, data);
    } else {
        return kIntelVvpAnrOutOfBoundsErr;
    }

    return (intel_vvp_anr_commit_is_pending(instance)) ? kIntelVvpAnrCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_anr_write_intensity_lut_array(intel_vvp_anr_instance* instance, const uint32_t* data_array)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data_array == NULL) return kIntelVvpAnrPointerErr;

    for (int i = 0; i < INTEL_VVP_ANR_INTENSITY_LUT_ENTRIES; i++) {
        INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_INTENSITY_LUT_BASE + i, data_array[i]);
    }

    return (intel_vvp_anr_commit_is_pending(instance)) ? kIntelVvpAnrCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_anr_write_spatial_lut_entry(intel_vvp_anr_instance* instance, uint32_t data, uint32_t addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (addr < instance->spatial_lut_depth) {
        INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_SPATIAL_LUT_BASE + addr, data);
    } else {
        return kIntelVvpAnrOutOfBoundsErr;
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_anr_write_spatial_lut_array(intel_vvp_anr_instance* instance, const uint32_t* data_array)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data_array == NULL) return kIntelVvpAnrPointerErr;

    for (int i = 0; i < instance->spatial_lut_depth; i++) {
        INTEL_VVP_ANR_REG_IOWR(instance, INTEL_VVP_ANR_SPATIAL_LUT_BASE + i, data_array[i]);
    }

    return kIntelVvpCoreOk;
}
