/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_dpc.h"
#include "intel_vvp_dpc_regs.h"


int intel_vvp_dpc_init(intel_vvp_dpc_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_DPC_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_DPC_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_DPC_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpDpcRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_MAX_HEIGHT_REG);

        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_dpc_get_lite_mode(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_dpc_get_debug_enabled(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_dpc_get_bits_per_sample_in(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_dpc_get_bits_per_sample_out(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_dpc_get_num_color_planes_in(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_dpc_get_num_color_planes_out(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_dpc_get_pixels_in_parallel(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_dpc_get_max_width(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_dpc_get_max_height(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

bool intel_vvp_dpc_is_running(intel_vvp_dpc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_STATUS_REG);
    return INTEL_VVP_DPC_GET_FLAG(reg, STATUS_RUNNING);
}


bool intel_vvp_dpc_stats_are_frozen(intel_vvp_dpc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_STATUS_REG);
    return INTEL_VVP_DPC_GET_FLAG(reg, STATUS_STATS_FROZEN);
}

uint32_t intel_vvp_dpc_get_status(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_STATUS_REG);
}

int intel_vvp_dpc_get_frame_stats(intel_vvp_dpc_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (!intel_vvp_dpc_stats_are_frozen(instance)) return kIntelVvpDpcFreezePendingErr;
    if (stats_out == NULL) return kIntelVvpDpcPointerErr;

    *stats_out = INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_dpc_get_pix_lo_clip_cnt(intel_vvp_dpc_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (!intel_vvp_dpc_stats_are_frozen(instance)) return kIntelVvpDpcFreezePendingErr;
    if (stats_out == NULL) return kIntelVvpDpcPointerErr;

    *stats_out = INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_PIX_LO_COUNTER_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_dpc_get_pix_hi_clip_cnt(intel_vvp_dpc_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (!intel_vvp_dpc_stats_are_frozen(instance)) return kIntelVvpDpcFreezePendingErr;
    if (stats_out == NULL) return kIntelVvpDpcPointerErr;

    *stats_out = INTEL_VVP_DPC_REG_IORD(instance, INTEL_VVP_DPC_PIX_HI_COUNTER_REG);

    return kIntelVvpCoreOk;
}

bool intel_vvp_dpc_get_bypass(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_DPC_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_dpc_set_bypass(intel_vvp_dpc_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_DPC_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_DPC_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_dpc_get_cfa_phase(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_DPC_READ_FIELD(instance->settings_reg, SETTINGS_CFA_PHASE);
}

int intel_vvp_dpc_set_cfa_phase(intel_vvp_dpc_instance* instance, uint8_t cfa_phase)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cfa_phase > 3) return kIntelVvpDpcValueErr;

    INTEL_VVP_DPC_WRITE_FIELD(instance->settings_reg, cfa_phase, SETTINGS_CFA_PHASE);
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);
    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_dpc_get_sense_level(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_DPC_READ_FIELD(instance->settings_reg, SETTINGS_SENSE_LEVEL);
}

int intel_vvp_dpc_set_sense_level(intel_vvp_dpc_instance* instance, uint8_t sense_level)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (sense_level > 3) return kIntelVvpDpcValueErr;

    INTEL_VVP_DPC_WRITE_FIELD(instance->settings_reg, sense_level, SETTINGS_SENSE_LEVEL);
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);
    return kIntelVvpCoreOk;
}

bool intel_vvp_dpc_get_pix_clip_cnt_rst(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_DPC_GET_FLAG(instance->settings_reg, SETTINGS_PIX_CLIP_CNT_RST);
}

int intel_vvp_dpc_set_pix_clip_cnt_rst(intel_vvp_dpc_instance* instance, bool reset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (reset) {
        INTEL_VVP_DPC_SET_FLAG(instance->settings_reg, SETTINGS_PIX_CLIP_CNT_RST);
    } else {
        INTEL_VVP_DPC_CLEAR_FLAG(instance->settings_reg, SETTINGS_PIX_CLIP_CNT_RST);
    }
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_dpc_get_pix_clip_cnt_sel(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_DPC_READ_FIELD(instance->settings_reg, SETTINGS_PIX_CLIP_CNT_SEL);
}

int intel_vvp_dpc_set_pix_clip_cnt_sel(intel_vvp_dpc_instance* instance, uint8_t pix_clip_cnt_sel)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (pix_clip_cnt_sel > (instance->pip - 1)) return kIntelVvpDpcValueErr;

    INTEL_VVP_DPC_WRITE_FIELD(instance->settings_reg, pix_clip_cnt_sel, SETTINGS_PIX_CLIP_CNT_SEL);
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);
    return kIntelVvpCoreOk;
}

bool intel_vvp_dpc_get_freeze_stats_request(intel_vvp_dpc_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_DPC_GET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
}

int intel_vvp_dpc_set_freeze_stats_request(intel_vvp_dpc_instance* instance, bool freeze_stats)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (freeze_stats) {
        INTEL_VVP_DPC_SET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    } else {
        INTEL_VVP_DPC_CLEAR_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    }
    INTEL_VVP_DPC_REG_IOWR(instance, INTEL_VVP_DPC_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}