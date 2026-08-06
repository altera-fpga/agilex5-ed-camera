/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_wbs.h"
#include "intel_vvp_wbs_regs.h"

#include <string.h>

#define CLOG2(X) ((uint32_t) (1 + (31 - __builtin_clz(((X) - 1))))) // __builtin_clz = count leading zeroes

int intel_vvp_wbs_init(intel_vvp_wbs_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_WBS_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_WBS_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_WBS_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpWbsRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_MAX_HEIGHT_REG);
        instance->precision_bits           = (uint8_t)INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_PRECISION_BITS_REG);
        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        {
            uint64_t region_size_additional_bits_prelog = ((uint64_t)instance->max_width * (uint64_t)instance->max_height) / (49 * 2);
            instance->ratio_size_in_bits       = CLOG2(region_size_additional_bits_prelog) + instance->bps_in + instance->precision_bits + 1;
            instance->pixel_count_size_in_bits = CLOG2(region_size_additional_bits_prelog >> 1) + 1;
            instance->num_entries_per_table_result = 1 + ((((instance->ratio_size_in_bits * 2) + instance->pixel_count_size_in_bits) - 1) / 20);
        }
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_wbs_get_lite_mode(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_wbs_get_debug_enabled(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_wbs_get_bits_per_sample_in(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_wbs_get_bits_per_sample_out(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_wbs_get_num_color_planes_in(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_wbs_get_num_color_planes_out(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_wbs_get_pixels_in_parallel(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_wbs_get_max_width(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_wbs_get_max_height(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

uint8_t intel_vvp_wbs_get_precision_bits(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->precision_bits;
}

bool intel_vvp_wbs_is_running(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_STATUS_REG);
    return INTEL_VVP_WBS_GET_FLAG(reg, STATUS_RUNNING);
}

bool intel_vvp_wbs_commit_is_pending(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_STATUS_REG);
    return INTEL_VVP_WBS_GET_FLAG(reg, STATUS_COMMIT_PENDING);
}

bool intel_vvp_wbs_stats_are_frozen(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_STATUS_REG);
    return INTEL_VVP_WBS_GET_FLAG(reg, STATUS_STATS_FROZEN);
}

uint32_t intel_vvp_wbs_get_status(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_STATUS_REG);
}

int intel_vvp_wbs_get_frame_stats(intel_vvp_wbs_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (!intel_vvp_wbs_stats_are_frozen(instance)) return kIntelVvpWbsFreezePendingErr;
    if (stats_out == NULL) return kIntelVvpWbsPointerErr;

    *stats_out = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

int intel_vvp_wbs_commit(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}

bool intel_vvp_wbs_get_bypass(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_WBS_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_wbs_set_bypass(intel_vvp_wbs_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_WBS_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_WBS_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_SETTINGS_REG, instance->settings_reg);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint8_t intel_vvp_wbs_get_cfa_phase(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_WBS_READ_FIELD(instance->settings_reg, SETTINGS_CFA_PHASE);
}

int intel_vvp_wbs_set_cfa_phase(intel_vvp_wbs_instance* instance, uint8_t cfa_phase)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cfa_phase > 3) return kIntelVvpWbsValueErr;

    INTEL_VVP_WBS_WRITE_FIELD(instance->settings_reg, cfa_phase, SETTINGS_CFA_PHASE);
    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_SETTINGS_REG, instance->settings_reg);
    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint8_t intel_vvp_wbs_get_cfa_invert(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_WBS_READ_FIELD(instance->settings_reg, SETTINGS_CFA_INVERT);
}

int intel_vvp_wbs_set_cfa_invert(intel_vvp_wbs_instance* instance, uint8_t cfa_invert)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (cfa_invert > 3) return kIntelVvpWbsValueErr;

    INTEL_VVP_WBS_WRITE_FIELD(instance->settings_reg, cfa_invert, SETTINGS_CFA_INVERT);
    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_SETTINGS_REG, instance->settings_reg);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

bool intel_vvp_wbs_get_freeze_stats_request(intel_vvp_wbs_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_WBS_GET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
}

int intel_vvp_wbs_set_freeze_stats_request(intel_vvp_wbs_instance* instance, bool freeze_stats)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (freeze_stats) {
        INTEL_VVP_WBS_SET_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    } else {
        INTEL_VVP_WBS_CLEAR_FLAG(instance->settings_reg, SETTINGS_FREEZE_STATS);
    }
    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}
uint16_t intel_vvp_wbs_get_h_start(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_H_START_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, H_START_VALUE);
}

int intel_vvp_wbs_set_h_start(intel_vvp_wbs_instance* instance, uint16_t h_start_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_H_START_REG, h_start_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_wbs_get_v_start(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_V_START_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, V_START_VALUE);
}

int intel_vvp_wbs_set_v_start(intel_vvp_wbs_instance* instance, uint16_t v_start_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_V_START_REG, v_start_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_wbs_get_h_end(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_H_END_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, H_END_VALUE);
}

int intel_vvp_wbs_set_h_end(intel_vvp_wbs_instance* instance, uint16_t h_end_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_H_END_REG, h_end_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_wbs_get_v_end(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_V_END_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, V_END_VALUE);
}

int intel_vvp_wbs_set_v_end(intel_vvp_wbs_instance* instance, uint16_t v_end_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_V_END_REG, v_end_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_wbs_get_zone_h_count(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_ZONE_H_COUNT_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, ZONE_H_COUNT_VALUE);
}

int intel_vvp_wbs_set_zone_h_count(intel_vvp_wbs_instance* instance, uint16_t zone_h_count_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_ZONE_H_COUNT_REG, zone_h_count_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint16_t intel_vvp_wbs_get_zone_v_count(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_ZONE_V_COUNT_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, ZONE_V_COUNT_VALUE);
}

int intel_vvp_wbs_set_zone_v_count(intel_vvp_wbs_instance* instance, uint16_t zone_v_count_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_ZONE_V_COUNT_REG, zone_v_count_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_wbs_get_cfa_x0_range_lo(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_CFA_X0_RANGE_LO_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, CFA_X0_RANGE_LO_VALUE);
}

int intel_vvp_wbs_set_cfa_x0_range_lo(intel_vvp_wbs_instance* instance, uint32_t cfa_x0_range_lo_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_CFA_X0_RANGE_LO_REG, cfa_x0_range_lo_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_wbs_get_cfa_x0_range_hi(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_CFA_X0_RANGE_HI_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, CFA_X0_RANGE_HI_VALUE);
}

int intel_vvp_wbs_set_cfa_x0_range_hi(intel_vvp_wbs_instance* instance, uint32_t cfa_x0_range_hi_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_CFA_X0_RANGE_HI_REG, cfa_x0_range_hi_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_wbs_get_cfa_x1_range_lo(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_CFA_X1_RANGE_LO_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, CFA_X1_RANGE_LO_VALUE);
}

int intel_vvp_wbs_set_cfa_x1_range_lo(intel_vvp_wbs_instance* instance, uint32_t cfa_x1_range_lo_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_CFA_X1_RANGE_LO_REG, cfa_x1_range_lo_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

uint32_t intel_vvp_wbs_get_cfa_x1_range_hi(intel_vvp_wbs_instance* instance)
{
    uint32_t reg;

    if (instance == NULL || !instance->debug_enabled) return 0xFFFFFFFF;

    reg = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_CFA_X1_RANGE_HI_REG);
    return INTEL_VVP_WBS_READ_FIELD(reg, CFA_X1_RANGE_HI_VALUE);
}

int intel_vvp_wbs_set_cfa_x1_range_hi(intel_vvp_wbs_instance* instance, uint32_t cfa_x1_range_hi_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_WBS_REG_IOWR(instance, INTEL_VVP_WBS_CFA_X1_RANGE_HI_REG, cfa_x1_range_hi_value);

    return (intel_vvp_wbs_commit_is_pending(instance)) ? kIntelVvpWbsCommitPendingErr : kIntelVvpCoreOk;
}

int intel_vvp_wbs_get_table_entries(intel_vvp_wbs_instance* instance, uint8_t horiz, uint8_t vert, uint8_t* num_entries, uint32_t* entry_store)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (horiz > 6 || vert > 6) return kIntelVvpWbsValueErr; // Valid range is 0-6
    if (num_entries == NULL || entry_store == NULL) return kIntelVvpWbsPointerErr;

    uint32_t table_base_offset = ((vert * 7) + horiz) * instance->num_entries_per_table_result;

    *num_entries = instance->num_entries_per_table_result;

    for (int i = 0; i < instance->num_entries_per_table_result; i++)
    {
        entry_store[i] = INTEL_VVP_WBS_REG_IORD(instance, INTEL_VVP_WBS_RESULTS_TABLE_BASE + table_base_offset + i);
    }

    return kIntelVvpCoreOk;
}

intel_vvp_wbs_zone_result intel_vvp_wbs_get_formatted_table_result(intel_vvp_wbs_instance* instance, uint8_t horiz, uint8_t vert)
{
    uint8_t num_entries;
    intel_vvp_wbs_zone_result result;

    // If there's an error, return all F's as the error code
    memset((void*)&result, 0xF, sizeof(intel_vvp_wbs_zone_result));

    if (instance == NULL) return result;
    if (horiz > 6 || vert > 6) return result; // Valid range is 0-6

    uint32_t table_entries[instance->num_entries_per_table_result];

    if (intel_vvp_wbs_get_table_entries(instance, horiz, vert, &num_entries, (uint32_t*)table_entries) != kIntelVvpCoreOk)
    {
        return result;
    }

    uint8_t x0_start_reg = 0;
    uint8_t x0_end_reg = (instance->ratio_size_in_bits) / 20;
    uint8_t x1_start_reg = (instance->ratio_size_in_bits) / 20;
    uint8_t x1_end_reg = ((2 * instance->ratio_size_in_bits)) / 20;

    uint8_t pixel_count_start_reg = ((2 * instance->ratio_size_in_bits)) / 20;
    uint8_t pixel_count_end_reg = (((2 * instance->ratio_size_in_bits) - 1) + instance->pixel_count_size_in_bits) / 20;
    uint8_t pixel_count_start_bit = ((2 * instance->ratio_size_in_bits)) % 20;

    long unsigned int x0_accumulator = 0;
    long unsigned int x1_accumulator = 0;
    uint32_t pixel_count_accumulator = 0;

    uint8_t bit_counter_x0 = 0;
    uint8_t bit_counter_x1 = 0;

    memset((void*)&result, 0x0, sizeof(intel_vvp_wbs_zone_result));

    // First, read the pixel counter.

    pixel_count_accumulator = table_entries[pixel_count_start_reg] >> pixel_count_start_bit;
    if (pixel_count_start_reg != pixel_count_end_reg)
    {
        pixel_count_accumulator |= table_entries[pixel_count_end_reg] << (20 - pixel_count_start_bit);
    }

    // If there are no pixels, then the accumulators will be zero and can be ignored
    if (!pixel_count_accumulator)
    {
        return result; // Return the zeroes, don't carry on
    }

    // Otherwise, unpack the ratio accumulators
    uint8_t x0_index = x0_start_reg;
    uint8_t x1_index = x1_start_reg;

    x0_accumulator = table_entries[x0_index];
    x1_accumulator = table_entries[x1_index] >> (instance->ratio_size_in_bits % 20);
    bit_counter_x0 = 20;
    bit_counter_x1 = 20 - (instance->ratio_size_in_bits % 20);

    do
    {
        x0_index++;
        x1_index++;
        if (x0_index <= x0_end_reg)
        {
            x0_accumulator |= ((uint64_t)table_entries[x0_index]) << bit_counter_x0;
            bit_counter_x0 += 20;
        }
        if (x1_index <= x1_end_reg)
        {
            x1_accumulator |= ((uint64_t)table_entries[x1_index]) << bit_counter_x1;
            bit_counter_x1 += 20;
        }

    } while ((x0_index <= x0_end_reg) && (x1_index <= x1_end_reg));

    long unsigned ratio = ((((uint64_t)1) << instance->ratio_size_in_bits) - 1);

    x0_accumulator &= ratio; // Mask out extra bits
    x1_accumulator &= ratio; // Mask out extra bits

    result.num_pixels_accumulated = pixel_count_accumulator;
    result.x0_fraction = x0_accumulator & ((1 << instance->precision_bits) - 1);
    result.x0_integer = x0_accumulator >> instance->precision_bits;
    result.x1_fraction = x1_accumulator & ((1 << instance->precision_bits) - 1);
    result.x1_integer = x1_accumulator >> instance->precision_bits;

    return result;
}
