/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_vc.h"
#include "intel_vvp_vc_regs.h"


int intel_vvp_vc_init(intel_vvp_vc_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_VC_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_VC_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_VC_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpVcRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_MAX_HEIGHT_REG);
        instance->cfa_enable               = (0 != INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_CFA_ENABLE_REG));
        instance->max_gain_mesh_points     = (uint16_t)INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_MAX_GAIN_MESH_POINTS_REG);
        instance->per_color_gain_enable    = (0 != INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_PER_COLOR_GAIN_ENABLE_REG));
        // Initially, set instance->h_num_blocks and instance->v_num_blocks to zero.
        instance->h_num_blocks             = 0;
        instance->v_num_blocks             = 0;
        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_vc_get_lite_mode(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_vc_get_debug_enabled(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_vc_get_bits_per_sample_in(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_vc_get_bits_per_sample_out(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_vc_get_num_color_planes_in(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_vc_get_num_color_planes_out(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_vc_get_pixels_in_parallel(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_vc_get_max_width(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_vc_get_max_height(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

bool intel_vvp_vc_get_cfa_enable(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->cfa_enable;
}

uint16_t intel_vvp_vc_get_max_gain_mesh_points(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_gain_mesh_points;
}

bool intel_vvp_vc_get_per_color_gain_enable(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return false;

    return instance->per_color_gain_enable;
}

bool intel_vvp_vc_is_running(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_STATUS_REG);
    return INTEL_VVP_VC_GET_FLAG(reg, STATUS_RUNNING);
}

bool intel_vvp_vc_commit_is_pending(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_STATUS_REG);
    return INTEL_VVP_VC_GET_FLAG(reg, STATUS_COMMIT_PENDING);
}

uint32_t intel_vvp_vc_get_status(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_STATUS_REG);
}

int intel_vvp_vc_get_frame_stats(intel_vvp_vc_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (stats_out == NULL) return kIntelVvpVcPointerErr;

    *stats_out = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_vc_commit(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}

bool intel_vvp_vc_get_bypass(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_VC_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_vc_set_bypass(intel_vvp_vc_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_VC_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_VC_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_SETTINGS_REG, instance->settings_reg);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint8_t intel_vvp_vc_get_cfa_phase(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_VC_READ_FIELD(instance->settings_reg, SETTINGS_CFA_PHASE);
}

int intel_vvp_vc_set_cfa_phase(intel_vvp_vc_instance* instance, uint8_t cfa_phase)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cfa_phase > 3) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_WRITE_FIELD(instance->settings_reg, cfa_phase, SETTINGS_CFA_PHASE);
    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_SETTINGS_REG, instance->settings_reg);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_vc_get_block_pix_count(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_BLOCK_PIX_COUNT_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, BLOCK_PIX_COUNT_VALUE);
}

int intel_vvp_vc_set_block_pix_count(intel_vvp_vc_instance* instance, uint16_t block_pix_count_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (block_pix_count_value > INTEL_VVP_VC_BLOCK_PIX_COUNT_VALUE_MAX) return kIntelVvpVcValueErr;
    // If block_pix_count_value is below the minimum required value, report an error.
    if (block_pix_count_value < (8 * instance->pip)) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_BLOCK_PIX_COUNT_REG, block_pix_count_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_vc_get_block_line_count(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_BLOCK_LINE_COUNT_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, BLOCK_LINE_COUNT_VALUE);
}

int intel_vvp_vc_set_block_line_count(intel_vvp_vc_instance* instance, uint16_t block_line_count_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (block_line_count_value > INTEL_VVP_VC_BLOCK_LINE_COUNT_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_BLOCK_LINE_COUNT_REG, block_line_count_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_vc_get_h_num_blocks(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return instance->h_num_blocks;
}

int intel_vvp_vc_set_h_num_blocks(intel_vvp_vc_instance* instance, uint16_t h_num_blocks_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_num_blocks_value > INTEL_VVP_VC_H_NUM_BLOCKS_VALUE_MAX) return kIntelVvpVcValueErr;

    instance->h_num_blocks = h_num_blocks_value;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_H_NUM_BLOCKS_REG, instance->h_num_blocks);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_vc_get_v_num_blocks(intel_vvp_vc_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return instance->v_num_blocks;
}

int intel_vvp_vc_set_v_num_blocks(intel_vvp_vc_instance* instance, uint16_t v_num_blocks_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_num_blocks_value > INTEL_VVP_VC_V_NUM_BLOCKS_VALUE_MAX) return kIntelVvpVcValueErr;

    instance->v_num_blocks = v_num_blocks_value;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_V_NUM_BLOCKS_REG, instance->v_num_blocks);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_h_ramp_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_H_RAMP_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, H_RAMP_FRAC_VALUE);
}

int intel_vvp_vc_set_h_ramp_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_ramp_frac_value > INTEL_VVP_VC_H_RAMP_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_H_RAMP_FRAC_REG, h_ramp_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_h_ramp_p1_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_H_RAMP_P1_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, H_RAMP_P1_FRAC_VALUE);
}

int intel_vvp_vc_set_h_ramp_p1_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_p1_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_ramp_p1_frac_value > INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_H_RAMP_P1_FRAC_REG, h_ramp_p1_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_h_ramp_m1_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_H_RAMP_M1_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, H_RAMP_M1_FRAC_VALUE);
}

int intel_vvp_vc_set_h_ramp_m1_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_m1_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (h_ramp_m1_frac_value > INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_H_RAMP_M1_FRAC_REG, h_ramp_m1_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_v_ramp_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_V_RAMP_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, V_RAMP_FRAC_VALUE);
}

int intel_vvp_vc_set_v_ramp_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_ramp_frac_value > INTEL_VVP_VC_V_RAMP_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_V_RAMP_FRAC_REG, v_ramp_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_v_ramp_p1_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_V_RAMP_P1_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, V_RAMP_P1_FRAC_VALUE);
}

int intel_vvp_vc_set_v_ramp_p1_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_p1_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_ramp_p1_frac_value > INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_V_RAMP_P1_FRAC_REG, v_ramp_p1_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_vc_get_v_ramp_m1_frac(intel_vvp_vc_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_VC_REG_IORD(instance, INTEL_VVP_VC_V_RAMP_M1_FRAC_REG);
    return INTEL_VVP_VC_READ_FIELD(reg, V_RAMP_M1_FRAC_VALUE);
}

int intel_vvp_vc_set_v_ramp_m1_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_m1_frac_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (v_ramp_m1_frac_value > INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_MAX) return kIntelVvpVcValueErr;

    INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_V_RAMP_M1_FRAC_REG, v_ramp_m1_frac_value);

    return (intel_vvp_vc_commit_is_pending(instance)) ? kIntelVvpVcCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_vc_update_cp_lut(intel_vvp_vc_instance* instance, uint8_t cp_lut_idx, const uint32_t* cp_lut_values)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cp_lut_values == NULL) return kIntelVvpVcPointerErr;
    if (instance->h_num_blocks == 0 || instance->v_num_blocks == 0) return kIntelVvpVcLutInvalidNumBlocksErr;
    if (cp_lut_idx > 3) return kIntelVvpVcParameterErr;
    if (cp_lut_idx != 0) {
        // Make sure that the instance was configured with per_color_gain_enable.
        if (!instance->per_color_gain_enable) {
            return kIntelVvpVcLutNotAvailableErr;
        }
    }

    uint16_t no_of_lut_entries = (instance->h_num_blocks + 1) * (instance->v_num_blocks + 1);
    for (int i = 0; i < no_of_lut_entries; i++) {
        INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_CP_LUT(cp_lut_idx, i), cp_lut_values[i]);
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_vc_update_step_lut(intel_vvp_vc_instance* instance, const uint32_t* step_lut_values)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (step_lut_values == NULL) return kIntelVvpVcPointerErr;
    if (instance->h_num_blocks == 0 || instance->v_num_blocks == 0) return kIntelVvpVcLutInvalidNumBlocksErr;

    uint16_t no_of_lut_entries = (instance->h_num_blocks + 1) * (instance->v_num_blocks + 1);
    for (int i = 0; i < no_of_lut_entries; i++) {
        INTEL_VVP_VC_REG_IOWR(instance, INTEL_VVP_VC_STEP_LUT(i), step_lut_values[i]);
    }

    return kIntelVvpCoreOk;
}
