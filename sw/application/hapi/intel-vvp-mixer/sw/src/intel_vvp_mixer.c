/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_mixer.h"

int intel_vvp_mixer_init(intel_vvp_mixer_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;
    int l;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_MIXER_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_MIXER_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_MIXER_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpMixerRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_DEBUG_ENABLED_REG));
        instance->pixels_in_parallel = (uint8_t)INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_PIXELS_IN_PARALLEL_REG);
        instance->num_layers = (uint8_t)INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_NUM_LAYERS_REG);
        // Check the number of layers
        if (instance->num_layers > INTEL_VVP_MIXER_MAX_NUM_LAYERS) {
            instance->num_layers = INTEL_VVP_MIXER_MAX_NUM_LAYERS;
            init_ret = kIntelVvpMixerRegMapVersionErr;
        }
        if (instance->num_layers < 2) {
            instance->num_layers = 2;
            init_ret = kIntelVvpMixerRegMapVersionErr;
        }
        // Check pixels_in_parallel
        if (instance->pixels_in_parallel == 0) {
            instance->pixels_in_parallel = 1;
            init_ret = kIntelVvpMixerRegMapVersionErr; // Invalid pixels_in_parallel value, register map not supported
        }
        for (l = 1; l < instance->num_layers; ++l)
        {
            instance->foreground_layer_config[l-1].blend_mode_config = (eIntelVvpBlendModeConfig)INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_SUPPORTED_BLEND_MODES_REG(l));
            instance->foreground_layer_config[l-1].restricted_offsets = (0 != INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_RESTRICTED_OFFSETS_REG(l)));
            
            if ((instance->foreground_layer_config[l-1].blend_mode_config < kIntelVvpMixerBlendConfigNone) ||
                (instance->foreground_layer_config[l-1].blend_mode_config > kIntelVvpMixerBlendConfigStaticOrInputAlpha)) {
                init_ret = kIntelVvpMixerRegMapVersionErr; // Invalid/unknown supported blend mode value, register map not supported
            }
        }
        for (l = instance->num_layers; l < INTEL_VVP_MIXER_MAX_NUM_LAYERS; ++l) {
            instance->foreground_layer_config[l-1].blend_mode_config = kIntelVvpMixerBlendConfigNone;
            instance->foreground_layer_config[l-1].restricted_offsets = false;
        }
    }

    return init_ret;
}
    
bool intel_vvp_mixer_get_lite_mode(intel_vvp_mixer_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->lite_mode;
}

bool intel_vvp_mixer_get_debug_enabled(intel_vvp_mixer_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->debug_enabled;
}

uint8_t intel_vvp_mixer_get_num_layers(intel_vvp_mixer_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->num_layers;
}

uint8_t intel_vvp_mixer_get_pixels_in_parallel(intel_vvp_mixer_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->pixels_in_parallel;
}

eIntelVvpBlendModeConfig intel_vvp_mixer_get_supported_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerBlendConfigNone;
    
    return instance->foreground_layer_config[layer-1].blend_mode_config;
}

bool intel_vvp_mixer_requires_restricted_offsets(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (instance->pixels_in_parallel == 1) || (layer == 0) || (layer >= instance->num_layers)) return false;

    return instance->foreground_layer_config[layer-1].restricted_offsets;
}


bool intel_vvp_mixer_is_running(intel_vvp_mixer_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_STATUS_REG);
    return INTEL_VVP_MIXER_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_mixer_get_commit_status(intel_vvp_mixer_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_STATUS_REG);
    return INTEL_VVP_MIXER_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

bool intel_vvp_mixer_is_layer_active(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || (layer >= instance->num_layers)) return false;
    if (layer == 0) return true;
    
    status_reg = INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_STATUS_REG);
    return (0 != (status_reg & INTEL_VVP_MIXER_STATUS_LAYER_ACTIVE_MSK(layer)));
}

uint32_t intel_vvp_mixer_get_status(intel_vvp_mixer_instance* instance)
{
    if (instance == NULL) return 0;
    
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_STATUS_REG);
}

int intel_vvp_mixer_get_input_mode(intel_vvp_mixer_instance* instance, uint8_t layer, bool *enabled, bool *consumed, bool *soft_start)
{
    uint32_t input_mode_reg;
    
    if ((instance == NULL) || (!instance->debug_enabled)) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    if ((enabled == NULL) || (consumed == NULL) || (soft_start == NULL)) return kIntelVvpCoreNullPtrErr;
    
    input_mode_reg = INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_MODE_REG(layer));
    *enabled = INTEL_VVP_MIXER_GET_FLAG(input_mode_reg, LAYER_MODE_ENABLE);
    *consumed = INTEL_VVP_MIXER_GET_FLAG(input_mode_reg, LAYER_MODE_CONSUME);
    *soft_start = INTEL_VVP_MIXER_GET_FLAG(input_mode_reg, LAYER_MODE_SOFT_START);
                             
    return kIntelVvpCoreOk;
}

int intel_vvp_mixer_set_input_mode(intel_vvp_mixer_instance* instance, uint8_t layer, bool enable, bool consume, bool soft_start)
{
    uint32_t input_mode_reg;
    int return_error_code;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;

    return_error_code = kIntelVvpCoreOk;
    
    input_mode_reg = 0;
    if (consume)
    {
        INTEL_VVP_MIXER_SET_FLAG(input_mode_reg, LAYER_MODE_CONSUME);
        if (!enable)
        {
            return_error_code = kIntelVvpMixerParameterErr; // Proceed by forcing enabled to true but still return an error code
            enable = true;
        }
    }
    if (enable)
    {
        INTEL_VVP_MIXER_SET_FLAG(input_mode_reg, LAYER_MODE_ENABLE);
    }
    if (soft_start)
    {
        if (!enable)
        {
            return_error_code = kIntelVvpMixerParameterErr; // Proceed by ignoring soft_start and leaving it unset
        }
        else
        {
            INTEL_VVP_MIXER_SET_FLAG(input_mode_reg, LAYER_MODE_SOFT_START);
        }
    }
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_MODE_REG(layer), input_mode_reg);

    return return_error_code;
}

eIntelVvpBlendMode intel_vvp_mixer_get_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || (layer == 0) ||
        (layer >= instance->num_layers)) return kIntelVvpMixerBlendTransparent;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_BLEND_MODE_REG(layer));
}
 
int intel_vvp_mixer_set_blend_mode(intel_vvp_mixer_instance* instance, uint8_t layer, eIntelVvpBlendMode blend_mode)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    switch (blend_mode)
    {
        case kIntelVvpMixerBlendStaticAlpha:
        {
            if ((instance->foreground_layer_config[layer-1].blend_mode_config != kIntelVvpMixerBlendConfigStaticAlpha) &&
                (instance->foreground_layer_config[layer-1].blend_mode_config != kIntelVvpMixerBlendConfigStaticOrInputAlpha))
            {
                return kIntelVvpMixerParameterErr;
            }
            break;
        }
        case kIntelVvpMixerBlendInputAlpha:
        {
            if ((instance->foreground_layer_config[layer-1].blend_mode_config != kIntelVvpMixerBlendConfigInputAlpha) &&
                (instance->foreground_layer_config[layer-1].blend_mode_config != kIntelVvpMixerBlendConfigStaticOrInputAlpha))
            {
                return kIntelVvpMixerParameterErr;
            }
            break;
        }
        case kIntelVvpMixerBlendTransparent: case kIntelVvpMixerBlendOpaque:
        {
            break;
        }
        default:
        {
            // Invalid blend_mode
            return kIntelVvpMixerParameterErr;
        }
    }
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_BLEND_MODE_REG(layer), blend_mode);
    
    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_mixer_get_static_alpha(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || (layer == 0) ||
        (layer >= instance->num_layers)) return 0xFFFFFFFF;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_STATIC_ALPHA_REG(layer));
}

int intel_vvp_mixer_set_static_alpha(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t static_alpha)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_STATIC_ALPHA_REG(layer), static_alpha);
    
    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_mixer_get_horiz_offset(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || (layer == 0) ||
        (layer >= instance->num_layers)) return 0xFFFFFFFF;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_H_OFFSET_REG(layer));
}

int intel_vvp_mixer_set_horiz_offset(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t offset)
{
    int return_error_code;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    return_error_code = kIntelVvpCoreOk;
    if (instance->foreground_layer_config[layer-1].restricted_offsets && ((offset % instance->pixels_in_parallel) != 0))
    {
        return_error_code = kIntelVvpMixerParameterErr;
    }
    
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_H_OFFSET_REG(layer), offset);
    
    return return_error_code;
}

uint32_t intel_vvp_mixer_get_vert_offset(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || (layer == 0) ||
        (layer >= instance->num_layers)) return 0xFFFFFFFF;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_V_OFFSET_REG(layer));
}

int intel_vvp_mixer_set_vert_offset(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t offset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_V_OFFSET_REG(layer), offset);
    
    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_mixer_get_width(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || !instance->lite_mode ||
        (layer == 0) || (layer >= instance->num_layers)) return 0xFFFFFFFF;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_WIDTH_REG(layer));
}

int intel_vvp_mixer_set_width(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t width)
{
    if ((instance == NULL) || !instance->lite_mode) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_WIDTH_REG(layer), width);
    
    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_mixer_get_height(intel_vvp_mixer_instance* instance, uint8_t layer)
{
    if ((instance == NULL) || (!instance->debug_enabled) || !instance->lite_mode ||
        (layer == 0) || (layer >= instance->num_layers)) return 0xFFFFFFFF;
   
    return INTEL_VVP_MIXER_REG_IORD(instance, INTEL_VVP_MIXER_LAYER_HEIGHT_REG(layer));
}

int intel_vvp_mixer_set_height(intel_vvp_mixer_instance* instance, uint8_t layer, uint32_t height)
{
    if ((instance == NULL) || !instance->lite_mode) return kIntelVvpCoreInstanceErr;
    if ((layer == 0) || (layer >= instance->num_layers)) return kIntelVvpMixerLayerErr;
    
    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_LAYER_HEIGHT_REG(layer), height);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_mixer_commit_writes(intel_vvp_mixer_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_MIXER_REG_IOWR(instance, INTEL_VVP_MIXER_COMMIT_REG, 1);
    return kIntelVvpCoreOk;
}