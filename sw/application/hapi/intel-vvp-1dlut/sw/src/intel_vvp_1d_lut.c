/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_1d_lut.h"
#include "intel_vvp_1d_lut_regs.h"


int intel_vvp_1d_lut_init(intel_vvp_1d_lut_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_1D_LUT_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_1D_LUT_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_1D_LUT_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvp1dLutRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Parameters
        instance->lite_mode                = (0 != INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_LITE_MODE_REG));
        instance->debug_enabled            = (0 != INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_DEBUG_ENABLED_REG));
        instance->bps_in                   = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BPS_IN_REG);
        instance->bps_out                  = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BPS_OUT_REG);
        instance->num_color_in             = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_NUM_COLOR_IN_REG);
        instance->num_color_out            = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_NUM_COLOR_OUT_REG);
        instance->pip                      = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_PIP_REG);
        instance->max_width                = (uint32_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_MAX_WIDTH_REG);
        instance->max_height               = (uint32_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_MAX_HEIGHT_REG);
        instance->bits_lut                 = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BITS_LUT_REG);
        instance->equidistant              = (0 != INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_EQUIDISTANT_REG));
        instance->bits_seg                 = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BITS_SEG_REG);
        instance->bits_step                = (uint8_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BITS_STEP_REG);
        instance->reverse_lut              = (0 != INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_REVERSE_LUT_REG));
        // Internal states
        instance->settings_reg             = (uint32_t)1; // Start as bypass=1, everything else 0.
        instance->num_ext_data_regs        = instance->num_color_out * 0b1u << (uint32_t)INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_BITS_LUT_REG);
        // Write 1 to the settings register, so we start out in a consistent state between cached
        // instance->settings_reg and the real register on the IP instance.
        INTEL_VVP_1D_LUT_REG_IOWR(instance, INTEL_VVP_1D_LUT_SETTINGS_REG, instance->settings_reg);
    }

    return init_ret;
}

bool intel_vvp_1d_lut_get_lite_mode(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_1d_lut_get_debug_enabled(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_1d_lut_get_bits_per_sample_in(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_in;
}

uint8_t intel_vvp_1d_lut_get_bits_per_sample_out(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps_out;
}

uint8_t intel_vvp_1d_lut_get_num_color_planes_in(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_in;
}

uint8_t intel_vvp_1d_lut_get_num_color_planes_out(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_out;
}

uint8_t intel_vvp_1d_lut_get_pixels_in_parallel(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

uint32_t intel_vvp_1d_lut_get_max_width(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint32_t intel_vvp_1d_lut_get_max_height(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

bool intel_vvp_1d_lut_get_equidistant(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return instance->equidistant;
}

uint8_t intel_vvp_1d_lut_get_bits_lut(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bits_lut;
}

uint8_t intel_vvp_1d_lut_get_bits_seg(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bits_seg;
}

uint8_t intel_vvp_1d_lut_get_bits_step(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bits_step;
}

bool intel_vvp_1d_lut_get_reverse_lut(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return instance->reverse_lut;
}

bool intel_vvp_1d_lut_is_running(intel_vvp_1d_lut_instance* instance)
{
    uint32_t reg;

    if (instance == NULL) return false;

    reg = INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_STATUS_REG);
    return INTEL_VVP_1D_LUT_GET_FLAG(reg, STATUS_RUNNING);
}

uint32_t intel_vvp_1d_lut_get_status(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_STATUS_REG);
}

int intel_vvp_1d_lut_get_frame_stats(intel_vvp_1d_lut_instance* instance, uint32_t* stats_out)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (stats_out == NULL) return kIntelVvp1dLutPointerErr;

    *stats_out = INTEL_VVP_1D_LUT_REG_IORD(instance, INTEL_VVP_1D_LUT_FRAME_STATS_REG);

    return kIntelVvpCoreOk;
}

bool intel_vvp_1d_lut_get_bypass(intel_vvp_1d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return INTEL_VVP_1D_LUT_GET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
}

int intel_vvp_1d_lut_set_bypass(intel_vvp_1d_lut_instance* instance, bool bypass)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (bypass) {
        INTEL_VVP_1D_LUT_SET_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    } else {
        INTEL_VVP_1D_LUT_CLEAR_FLAG(instance->settings_reg, SETTINGS_BYPASS);
    }
    INTEL_VVP_1D_LUT_REG_IOWR(instance, INTEL_VVP_1D_LUT_SETTINGS_REG, instance->settings_reg);

    return kIntelVvpCoreOk;
}

int intel_vvp_1d_lut_write_data(intel_vvp_1d_lut_instance* instance, uint32_t data, uint32_t addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (addr < instance->num_ext_data_regs) {
        INTEL_VVP_1D_LUT_REG_IOWR(instance, INTEL_VVP_1D_LUT_EXT_DATA_BASE + addr, data);
    } else {
        return kIntelVvp1dLutOutOfBoundsErr;
    }

    return kIntelVvpCoreOk;
}

int intel_vvp_1d_lut_write_data_array(intel_vvp_1d_lut_instance* instance, const uint32_t* data_array, uint32_t length)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (data_array == NULL) return kIntelVvp1dLutPointerErr;

    for (int i = 0; i < length; i++) {
        if (i >= instance->num_ext_data_regs) return kIntelVvp1dLutOutOfBoundsErr;
        INTEL_VVP_1D_LUT_REG_IOWR(instance, INTEL_VVP_1D_LUT_EXT_DATA_BASE + i, data_array[i]);
    }

    return kIntelVvpCoreOk;
}
