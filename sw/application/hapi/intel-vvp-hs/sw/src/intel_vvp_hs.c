/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_hs.h"
#include "intel_vvp_hs_regs.h"


int intel_vvp_hs_init(intel_vvp_hs_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_HS_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_HS_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_HS_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpHsRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_MAX_HEIGHT_REG);
        instance->num_hist_bins            = (uint16_t)INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_NUM_HIST_BINS_REG);
        // Internal states
        instance->settings_reg             = (uint32_t)0; // Start as bypass=1, everything else 0.
                                                          // HS is *always* bypassing.
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_SETTINGS_REG, instance->settings_reg);
        instance->frame_hist_base          = INTEL_VVP_HS_EXT_DATA_BASE;
        instance->roi_hist_base            = INTEL_VVP_HS_EXT_DATA_BASE + instance->num_hist_bins;
    }

    return init_ret;
}

bool intel_vvp_hs_get_lite_mode(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_hs_get_debug_enabled(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_hs_get_bits_per_sample_in(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_hs_get_bits_per_sample_out(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_hs_get_num_color_planes_in(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_hs_get_num_color_planes_out(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_hs_get_pixels_in_parallel(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_hs_get_max_width(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_hs_get_max_height(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

uint16_t intel_vvp_hs_get_num_hist_bins(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_hist_bins;
}

bool intel_vvp_hs_is_running(intel_vvp_hs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_STATUS_REG);
    return INTEL_VVP_HS_GET_FLAG(reg, STATUS_RUNNING);
}

bool intel_vvp_hs_commit_is_pending(intel_vvp_hs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_STATUS_REG);
    return INTEL_VVP_HS_GET_FLAG(reg, STATUS_COMMIT_PENDING);
}

bool intel_vvp_hs_stats_are_frozen(intel_vvp_hs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_STATUS_REG);
    return INTEL_VVP_HS_GET_FLAG(reg, STATUS_STATS_FROZEN);
}

uint32_t intel_vvp_hs_get_status(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_STATUS_REG);
}

int intel_vvp_hs_get_frame_stats(intel_vvp_hs_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (!intel_vvp_hs_stats_are_frozen(instance)) return kIntelVvpHsFreezePendingErr;
    if (stats_out == NULL) return kIntelVvpHsPointerErr;

    *stats_out = INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_hs_commit(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}

bool intel_vvp_hs_get_freeze_stats_request(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_HS_GET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
}

int intel_vvp_hs_set_freeze_stats_request(intel_vvp_hs_instance* instance, bool freeze_stats)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (freeze_stats) {
        INTEL_VVP_HS_SET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    } else {
        INTEL_VVP_HS_CLEAR_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    }
    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}

uint16_t intel_vvp_hs_get_h_start(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_H_START_REG);
}

int intel_vvp_hs_set_h_start(intel_vvp_hs_instance* instance, uint16_t h_start)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_start >= instance->max_width) return kIntelVvpHsValueErr;

    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_H_START_REG, h_start);

    return (intel_vvp_hs_commit_is_pending(instance)) ? kIntelVvpHsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_hs_get_v_start(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_V_START_REG);
}

int intel_vvp_hs_set_v_start(intel_vvp_hs_instance* instance, uint16_t v_start)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_start >= instance->max_height) return kIntelVvpHsValueErr;

    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_V_START_REG, v_start);

    return (intel_vvp_hs_commit_is_pending(instance)) ? kIntelVvpHsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_hs_get_h_end(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_H_END_REG);
}

int intel_vvp_hs_set_h_end(intel_vvp_hs_instance* instance, uint16_t h_end)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_end >= instance->max_width) return kIntelVvpHsValueErr;

    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_H_END_REG, h_end);

    return (intel_vvp_hs_commit_is_pending(instance)) ? kIntelVvpHsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_hs_get_v_end(intel_vvp_hs_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_HS_REG_IORD(instance, INTEL_VVP_HS_V_END_REG);
}

int intel_vvp_hs_set_v_end(intel_vvp_hs_instance* instance, uint16_t v_end)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_end >= instance->max_height) return kIntelVvpHsValueErr;

    INTEL_VVP_HS_REG_IOWR(instance, INTEL_VVP_HS_V_END_REG, v_end);

    return (intel_vvp_hs_commit_is_pending(instance)) ? kIntelVvpHsCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_hs_read_frame_hist_bin(intel_vvp_hs_instance* instance, uint32_t* data, uint16_t addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data == NULL) return kIntelVvpHsPointerErr;

    if (addr < instance->num_hist_bins) {
        *data = INTEL_VVP_HS_REG_IORD(instance, instance->frame_hist_base + addr);
    } else {
        return kIntelVvpHsOutOfBoundsErr;
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_hs_read_roi_hist_bin(intel_vvp_hs_instance* instance, uint32_t* data, uint16_t addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data == NULL) return kIntelVvpHsPointerErr;

    if (addr < instance->num_hist_bins) {
        *data = INTEL_VVP_HS_REG_IORD(instance, instance->roi_hist_base + addr);
    } else {
        return kIntelVvpHsOutOfBoundsErr;
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_hs_read_frame_hist_array(intel_vvp_hs_instance* instance, uint32_t* data_array)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data_array == NULL) return kIntelVvpHsPointerErr;

    for (int i = 0; i < instance->num_hist_bins; i++) {
        data_array[i] = INTEL_VVP_HS_REG_IORD(instance, instance->frame_hist_base + i);
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_hs_read_roi_hist_array(intel_vvp_hs_instance* instance, uint32_t* data_array)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data_array == NULL) return kIntelVvpHsPointerErr;

    for (int i = 0; i < instance->num_hist_bins; i++) {
        data_array[i] = INTEL_VVP_HS_REG_IORD(instance, instance->roi_hist_base + i);
    }

    return kIntelVvpCoreOk;
}
