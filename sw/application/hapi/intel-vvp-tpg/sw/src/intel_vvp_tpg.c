/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_vvp_tpg.h"
#include "intel_vvp_tpg_regs.h"

int intel_vvp_tpg_init(intel_vvp_tpg_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_TPG_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_TPG_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_TPG_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpTpgRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_DEBUG_ENABLED_REG));
        instance->num_patterns = (uint8_t)INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_NUM_PATTERNS_REG);
        instance->bps = (uint8_t)INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_BPS_REG);
        instance->pip = (uint8_t)INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_PIP_REG);
        instance->max_color_value = (0x1 << (instance->bps)) - 1;    // set max value a color plane can have for checks below
        
        if (instance->num_patterns > INTEL_VVP_TPG_MAX_NUM_PATTERNS)
        {
            init_ret = kIntelVvpTpgRegMapVersionErr;
        }
        
        intel_vvp_tpg_stop(instance);
    }

    return init_ret;
}

bool intel_vvp_tpg_get_lite_mode(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_tpg_get_debug_enabled(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->debug_enabled;
}

uint8_t intel_vvp_tpg_get_num_patterns(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_patterns;
}

uint8_t intel_vvp_tpg_get_bits_per_sample(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps;
}

uint8_t intel_vvp_tpg_get_pixels_in_parallel(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

int intel_vvp_tpg_get_pattern_data(intel_vvp_tpg_instance* instance, uint8_t pattern, intel_vvp_tpg_pattern* pattern_data)
{
    uint32_t pattern_type;
    uint32_t pattern_color;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    if (pattern >= instance->num_patterns) return kIntelVvpTpgPatternNumErr;
     
    if (pattern_data == NULL) return kIntelVvpCoreNullPtrErr;
        
    pattern_type = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_PATTERN_BASE_REG + pattern * 2);
    switch (pattern_type)
    {
        case INTEL_VVP_TPG_BARS_PATTERN:         pattern_data->type  = kIntelVvpTpgBarsPattern; break;
        case INTEL_VVP_TPG_UNIFORM_PATTERN:      pattern_data->type  = kIntelVvpTpgUniformPattern; break;
        case INTEL_VVP_TPG_PATHOLOGICAL_PATTERN: pattern_data->type  = kIntelVvpTpgPathologicalPattern; break;
        case INTEL_VVP_TPG_ZONE_PLATE_PATTERN:   pattern_data->type  = kIntelVvpTpgZonePlatePattern; break;
        case INTEL_VVP_TPG_DIGITAL_CLOCK_PATTERN:       pattern_data->type  = kIntelVvpTpgDigitalClockPattern; break;
        case INTEL_VVP_TPG_SIGNALTAP_COUNTER_PATTERN:   pattern_data->type  = kIntelVvpTpgSignalTapPattern; break;
        default:                                 pattern_data->type  = kIntelVvpTpgInvalidPattern;
    }
    pattern_color = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_PATTERN_BASE_REG + (pattern * 2) + 1);
    switch (pattern_color)
    {
        case INTEL_VVP_TPG_RGB:     pattern_data->color = kIntelVvpTpgRgb; break;
        case INTEL_VVP_TPG_YCC_444: pattern_data->color = kIntelVvpTpgYcc444; break;
        case INTEL_VVP_TPG_YCC_422: pattern_data->color = kIntelVvpTpgYcc422; break;
        case INTEL_VVP_TPG_YCC_420: pattern_data->color = kIntelVvpTpgYcc420; break;
        case INTEL_VVP_TPG_MONO:    pattern_data->color = kIntelVvpTpgMono; break;
        default:                    pattern_data->color = kIntelVvpTpgInvalidColor;
    }
    return kIntelVvpCoreOk;
}

bool intel_vvp_tpg_is_running(intel_vvp_tpg_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_STATUS_REG);
    return INTEL_VVP_TPG_GET_FLAG(status_reg, STATUS_RUNNING);
}


bool intel_vvp_tpg_get_commit_status(intel_vvp_tpg_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_STATUS_REG);
    return INTEL_VVP_TPG_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_tpg_get_status(intel_vvp_tpg_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_STATUS_REG);
    return status_reg;
}

int intel_vvp_tpg_enable(intel_vvp_tpg_instance* instance, bool enabled)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CONTROL_REG, enabled ? INTEL_VVP_TPG_CONTROL_GO_MSK : 0);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_start(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CONTROL_REG, INTEL_VVP_TPG_CONTROL_GO_MSK);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_stop(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CONTROL_REG, 0);
    return kIntelVvpCoreOk;
}

bool intel_vvp_tpg_is_enabled(intel_vvp_tpg_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return false;
    
    return 0 != (INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CONTROL_REG) & INTEL_VVP_TPG_CONTROL_GO_MSK);
}

int intel_vvp_tpg_set_pattern(intel_vvp_tpg_instance* instance, uint8_t pattern)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    else if (pattern >= instance->num_patterns) return kIntelVvpTpgPatternNumErr;
    
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_PATTERN_SELECT_REG, pattern);
    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_tpg_get_pattern(intel_vvp_tpg_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFF;
    return (uint8_t)INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_PATTERN_SELECT_REG);
}

uint32_t intel_vvp_tpg_get_fields_since_sequence_reset(intel_vvp_tpg_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_FIELD_COUNT_REG);
}

// Currently unsupported
//
// int intel_vvp_tpg_set_pre_field_ctrl_pkts(intel_vvp_tpg_instance* instance, uint32_t pkt_count)
// {
//     if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
    
//     INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_PRE_FIELD_CTRL_PKTS_REG, pkt_count);
//     return kIntelVvpCoreOk;
// }

// uint32_t intel_vvp_tpg_get_pre_field_ctrl_pkts(intel_vvp_tpg_instance* instance)
// {
//     if ((instance == NULL) || instance->lite_mode || !instance->debug_enabled) return 0xFFFFFFFF;

//     return INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_PRE_FIELD_CTRL_PKTS_REG);
// }

// int intel_vvp_tpg_set_post_field_ctrl_pkts(intel_vvp_tpg_instance* instance, uint32_t pkt_count)
// {
//     if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
        
//     INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_POST_FIELD_CTRL_PKTS_REG, pkt_count);
//     return kIntelVvpCoreOk;
// }

// uint32_t intel_vvp_tpg_get_post_field_ctrl_pkts(intel_vvp_tpg_instance* instance)
// {
//     if ((instance == NULL) || instance->lite_mode || !instance->debug_enabled) return 0xFFFFFFFF;
        
//     return INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_POST_FIELD_CTRL_PKTS_REG);
// }

int intel_vvp_tpg_set_bars_type(intel_vvp_tpg_instance* instance, eIntelVvpTpgBarsType bar_type)
{
    uint32_t new_bar_type;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    switch(bar_type)
    {
        case kIntelVvpTpgColorBars:      new_bar_type = INTEL_VVP_TPG_COLOR_BARS; break;
        case kIntelVvpTpgGreyBars:       new_bar_type = INTEL_VVP_TPG_GREY_BARS; break;
        case kIntelVvpTpgBlackWhiteBars: new_bar_type = INTEL_VVP_TPG_BLACK_WHITE_BARS; break;
        case kIntelVvpTpgMixedBars:      new_bar_type = INTEL_VVP_TPG_MIXED_BARS; break;
        default: return kIntelVvpTpgBarTypeErr;
    }
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_BARS_SELECT_REG, new_bar_type);
    
    return kIntelVvpCoreOk;
}

eIntelVvpTpgBarsType intel_vvp_tpg_get_bars_type(intel_vvp_tpg_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpTpgInvalidBars;

    switch (INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_BARS_SELECT_REG))
    {
        case INTEL_VVP_TPG_COLOR_BARS: return kIntelVvpTpgColorBars;
        case INTEL_VVP_TPG_GREY_BARS: return kIntelVvpTpgGreyBars;
        case INTEL_VVP_TPG_BLACK_WHITE_BARS: return kIntelVvpTpgBlackWhiteBars;
        case INTEL_VVP_TPG_MIXED_BARS: return kIntelVvpTpgMixedBars;
        default: return kIntelVvpTpgInvalidBars;
    }
}

int intel_vvp_tpg_set_colors(intel_vvp_tpg_instance* instance, uint16_t color1, uint16_t color2, uint16_t color3)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (color1 > instance->max_color_value) return kIntelVvpTpgC1Err;
    if (color2 > instance->max_color_value) return kIntelVvpTpgC2Err;
    if (color3 > instance->max_color_value) return kIntelVvpTpgC3Err;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_C0_REG, color1);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_C1_REG, color2);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_C2_REG, color3);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_colors(intel_vvp_tpg_instance* instance, uint16_t* color1, uint16_t* color2, uint16_t* color3)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;
    
    if ((color1 == NULL) || (color2 == NULL) || (color3 == NULL)) return kIntelVvpCoreNullPtrErr;

    *color1 = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_C0_REG);
    *color2 = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_C1_REG);
    *color3 = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_C2_REG);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_zone_plate_origin(intel_vvp_tpg_instance* instance, uint16_t x_coord, uint16_t y_coord)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_ZONE_X_ORIGIN_REG, x_coord);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_ZONE_Y_ORIGIN_REG, y_coord);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_zone_plate_origin(intel_vvp_tpg_instance* instance, uint16_t* x_coord, uint16_t* y_coord)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if ((x_coord == NULL) || (y_coord) == NULL) return kIntelVvpCoreNullPtrErr;

    *x_coord = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_ZONE_X_ORIGIN_REG);
    *y_coord = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_ZONE_Y_ORIGIN_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_zone_plate_factors(intel_vvp_tpg_instance* instance, uint16_t scaling_factor, uint16_t fine_tune_factor)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_ZONE_SCALING_FACTOR_REG, scaling_factor);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_ZONE_FINE_TUNE_FACTOR_REG, fine_tune_factor);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_zone_plate_factors(intel_vvp_tpg_instance* instance, uint16_t* scaling_factor, uint16_t* fine_tune_factor)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if ((scaling_factor == NULL) || (fine_tune_factor) == NULL) return kIntelVvpCoreNullPtrErr;

    *scaling_factor = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_ZONE_SCALING_FACTOR_REG);
    *fine_tune_factor = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_ZONE_FINE_TUNE_FACTOR_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_digital_clock_background_color(intel_vvp_tpg_instance* instance, uint16_t background_r, uint16_t background_g , uint16_t background_b)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(background_r > instance->max_color_value) return kIntelVvpTpgC3Err;
    if(background_g > instance->max_color_value) return kIntelVvpTpgC2Err;
    if(background_b > instance->max_color_value) return kIntelVvpTpgC1Err;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_B_BACKGROUND_REG, background_b);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_G_BACKGROUND_REG, background_g);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_R_BACKGROUND_REG, background_r);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_digital_clock_background_color(intel_vvp_tpg_instance* instance, uint16_t* background_r, uint16_t* background_g, uint16_t* background_b)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((background_r == NULL) || (background_g == NULL) || (background_b == NULL)) return kIntelVvpCoreNullPtrErr;

    *background_b = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_B_BACKGROUND_REG);
    *background_g = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_G_BACKGROUND_REG);
    *background_r = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_R_BACKGROUND_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_digital_clock_font_color(intel_vvp_tpg_instance* instance, uint16_t font_r, uint16_t font_g, uint16_t font_b)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(font_r > instance->max_color_value) return kIntelVvpTpgC3Err;
    if(font_g > instance->max_color_value) return kIntelVvpTpgC2Err;
    if(font_b > instance->max_color_value) return kIntelVvpTpgC1Err;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_B_FONT_REG, font_b);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_G_FONT_REG, font_g);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_R_FONT_REG, font_r);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_digital_clock_font_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((font_r == NULL) || (font_g == NULL) || (font_b == NULL)) return kIntelVvpCoreNullPtrErr;

    *font_r = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_R_FONT_REG);
    *font_g = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_G_FONT_REG);
    *font_b = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_B_FONT_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_digital_clock_location(intel_vvp_tpg_instance* instance, uint16_t location_x, uint16_t location_y)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_LOCATION_X_REG, location_x);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_LOCATION_Y_REG, location_y);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_digital_clock_location(intel_vvp_tpg_instance* instance, uint16_t* location_x, uint16_t* location_y)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((location_x == NULL) || (location_y == NULL)) return kIntelVvpCoreNullPtrErr;

    *location_x = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_LOCATION_X_REG);
    *location_y = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_LOCATION_Y_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_digital_clock_scale_factor(intel_vvp_tpg_instance* instance, uint16_t scale_factor)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(scale_factor > 0x7ff) return kIntelVvpTpgScaleFactorErr; // maximum of 11 bits

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_SCALE_FACTOR_REG,scale_factor);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_digital_clock_scale_factor(intel_vvp_tpg_instance* instance, uint16_t* scale_factor)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if(scale_factor == NULL) return kIntelVvpCoreNullPtrErr;

    *scale_factor = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_SCALE_FACTOR_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_digital_clock_fps(intel_vvp_tpg_instance* instance, uint8_t fps)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(fps > 0x3f) return kIntelVvpTpgFpsErr; // maximum of 63 

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_FPS_REG, fps);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_digital_clock_fps(intel_vvp_tpg_instance* instance, uint8_t* fps)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if( fps == NULL ) return kIntelVvpCoreNullPtrErr;

    *fps = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_FPS_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_signaltap_background_color(intel_vvp_tpg_instance* instance, uint16_t background_r, uint16_t background_g , uint16_t background_b)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(background_r > instance->max_color_value) return kIntelVvpTpgC3Err;
    if(background_g > instance->max_color_value) return kIntelVvpTpgC2Err;
    if(background_b > instance->max_color_value) return kIntelVvpTpgC1Err;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_B_BACKGROUND_REG, background_b);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_G_BACKGROUND_REG, background_g);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_R_BACKGROUND_REG, background_r);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_signaltap_background_color(intel_vvp_tpg_instance* instance, uint16_t* background_r, uint16_t* background_g, uint16_t* background_b)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((background_r == NULL) || (background_g == NULL) || (background_b == NULL)) return kIntelVvpCoreNullPtrErr;

    *background_b = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_B_BACKGROUND_REG);
    *background_g = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_G_BACKGROUND_REG);
    *background_r = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_R_BACKGROUND_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_signaltap_font_color(intel_vvp_tpg_instance* instance, uint16_t font_r, uint16_t font_g, uint16_t font_b)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(font_r > instance->max_color_value) return kIntelVvpTpgC3Err;
    if(font_g > instance->max_color_value) return kIntelVvpTpgC2Err;
    if(font_b > instance->max_color_value) return kIntelVvpTpgC1Err;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_B_FONT_REG, font_b);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_G_FONT_REG, font_g);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_R_FONT_REG, font_r);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_signaltap_font_color(intel_vvp_tpg_instance* instance, uint16_t* font_r, uint16_t* font_g, uint16_t* font_b)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((font_r == NULL) || (font_g == NULL) || (font_b == NULL)) return kIntelVvpCoreNullPtrErr;

    *font_r = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_R_FONT_REG);
    *font_g = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_G_FONT_REG);
    *font_b = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_B_FONT_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_signaltap_location(intel_vvp_tpg_instance* instance, uint16_t location_x, uint16_t location_y)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_LOCATION_X_REG, location_x);
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_LOCATION_Y_REG, location_y);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_signaltap_location(intel_vvp_tpg_instance* instance, uint16_t* location_x, uint16_t* location_y)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if((location_x == NULL) || (location_y == NULL)) return kIntelVvpCoreNullPtrErr;

    *location_x = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_LOCATION_X_REG);
    *location_y = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_LOCATION_Y_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_set_signaltap_fps(intel_vvp_tpg_instance* instance, uint8_t fps)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if(fps > 0x3f) return kIntelVvpTpgFpsErr; // maximum of 63 

    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_CLOCK_FPS_REG, fps);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_get_signaltap_fps(intel_vvp_tpg_instance* instance, uint8_t* fps)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;

    if( fps == NULL ) return kIntelVvpCoreNullPtrErr;

    *fps = INTEL_VVP_TPG_REG_IORD(instance, INTEL_VVP_TPG_CLOCK_FPS_REG);
    return kIntelVvpCoreOk;
}

int intel_vvp_tpg_commit_writes(intel_vvp_tpg_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
    INTEL_VVP_TPG_REG_IOWR(instance, INTEL_VVP_TPG_COMMIT_REG, 1); // Any write would do
    return kIntelVvpCoreOk;
}

