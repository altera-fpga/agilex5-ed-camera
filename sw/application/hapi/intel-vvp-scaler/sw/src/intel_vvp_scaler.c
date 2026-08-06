/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#include "intel_vvp_scaler.h"
#include "intel_vvp_scaler_regs.h"

#include "intel_vvp_quantizer.h"


int intel_vvp_scaler_init(intel_vvp_scaler_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_SCALER_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_SCALER_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_SCALER_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpScalerRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_DEBUG_ENABLED_REG));
        instance->pixels_in_parallel = (uint8_t)INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_PIXELS_IN_PARALLEL_REG);
        instance->max_input_width = (uint32_t)INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_MAX_INPUT_WIDTH_REG);
        instance->max_output_width = (uint32_t)INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_MAX_OUTPUT_WIDTH_REG);
        instance->scaling_algo = (eIntelVvpScalerAlgorithms)INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_ALGORITHM_REG);

        // Check the scaling algorithm
        if ((instance->scaling_algo < kIntelVvpScalerNearestNeighborScaling) || (instance->scaling_algo > kIntelVvpScalerPolyphaseScaling)) {
            instance->scaling_algo = kIntelVvpScalerInvalidScalingAlgo;
            init_ret = kIntelVvpScalerRegMapVersionErr;
        }

        instance->coeffs_runtime_load = false;
        instance->coeffs_mem_init = false;

        instance->v_scaling_enabled = false;
        instance->v_partial_scaling_enabled = false;
        instance->v_num_taps = 0;
        instance->v_num_phases = 0;
        instance->v_num_banks = 0;
        instance->v_coeffs_signed = false;
        instance->v_coeffs_int_bits =  0;
        instance->v_coeffs_frac_bits = 0;

        instance->h_scaling_enabled = false;
        instance->h_partial_scaling_enabled = false;
        instance->h_num_taps = 0;
        instance->h_num_phases = 0;
        instance->h_num_banks = 0;
        instance->h_coeffs_signed = false;
        instance->h_coeffs_int_bits =  0;
        instance->h_coeffs_frac_bits = 0;
        

        if (kIntelVvpScalerInvalidScalingAlgo != instance->scaling_algo)
        {
            if (kIntelVvpScalerPolyphaseScaling == instance->scaling_algo) {
                instance->coeffs_runtime_load = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_COEFFS_RUNTIME_LOAD_REG));
                instance->coeffs_mem_init = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_COEFFS_MEM_INIT_REG));
            }
            instance->v_scaling_enabled = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_SCALING_ENABLED_REG));
            if (instance->v_scaling_enabled)
            {
                instance->v_partial_scaling_enabled = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_PARTIAL_SCALING_ENABLED_REG));
                instance->v_num_taps = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_NUM_TAPS_REG);
                instance->v_num_phases = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_NUM_PHASES_REG);
                if (kIntelVvpScalerNearestNeighborScaling != instance->scaling_algo) {
                    instance->v_coeffs_frac_bits = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_COEFFS_FRAC_BITS_REG);
                }
                if (kIntelVvpScalerPolyphaseScaling == instance->scaling_algo) {
                    instance->v_num_banks = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_NUM_BANKS_REG);
                    instance->v_coeffs_signed = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_COEFFS_SIGNED_REG));
                    instance->v_coeffs_int_bits =  INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_COEFFS_INT_BITS_REG);
                    if ((instance->v_num_taps > INTEL_VVP_SCALER_MAX_NUM_TAPS) || (instance->v_num_taps < 2))
                        init_ret = kIntelVvpScalerRegMapVersionErr;
                    if ((instance->v_coeffs_int_bits + instance->v_coeffs_frac_bits) > INTEL_VVP_SCALER_MAX_COEFF_WIDTH)
                        init_ret = kIntelVvpScalerRegMapVersionErr;
                }
            }
            
            instance->h_scaling_enabled = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_SCALING_ENABLED_REG));
            if (instance->h_scaling_enabled)
            {
                instance->h_partial_scaling_enabled = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_PARTIAL_SCALING_ENABLED_REG));
                instance->h_num_taps = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_NUM_TAPS_REG);
                instance->h_num_phases = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_NUM_PHASES_REG);
                if (kIntelVvpScalerNearestNeighborScaling != instance->scaling_algo) {
                    instance->h_coeffs_frac_bits = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_COEFFS_FRAC_BITS_REG);
                }
                if (kIntelVvpScalerPolyphaseScaling == instance->scaling_algo) {
                    instance->h_num_banks = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_NUM_BANKS_REG);
                    instance->h_coeffs_signed = (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_COEFFS_SIGNED_REG));
                    instance->h_coeffs_int_bits =  INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_COEFFS_INT_BITS_REG);
                    if ((instance->h_num_taps > INTEL_VVP_SCALER_MAX_NUM_TAPS) || (instance->h_num_taps < 2))
                        init_ret = kIntelVvpScalerRegMapVersionErr;
                    if ((instance->h_coeffs_int_bits + instance->h_coeffs_frac_bits) > INTEL_VVP_SCALER_MAX_COEFF_WIDTH)
                        init_ret = kIntelVvpScalerRegMapVersionErr;
                }
            }
        }
    }

    return init_ret;
}
    
bool intel_vvp_scaler_get_lite_mode(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->lite_mode;
}

bool intel_vvp_scaler_get_debug_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->debug_enabled;
}

uint8_t intel_vvp_scaler_get_pixels_in_parallel(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->pixels_in_parallel;
}


uint32_t intel_vvp_scaler_get_max_input_width(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->max_input_width;
}

uint32_t intel_vvp_scaler_get_max_output_width(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->max_output_width;
}

eIntelVvpScalerAlgorithms intel_vvp_scaler_get_scaling_algorithm(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return kIntelVvpScalerInvalidScalingAlgo;
    
    return instance->scaling_algo;
}

bool intel_vvp_scaler_is_coeffs_loading_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->coeffs_runtime_load;
}

bool intel_vvp_scaler_are_coeffs_preinitialized(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->coeffs_mem_init;
}

  
bool intel_vvp_scaler_is_vertical_scaling_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->v_scaling_enabled;
}

bool intel_vvp_scaler_is_partial_vertical_scaling_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->v_partial_scaling_enabled;
}

uint8_t intel_vvp_scaler_get_num_vertical_taps(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->v_num_taps;
}

uint16_t intel_vvp_scaler_get_num_vertical_phases(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->v_num_phases;
}


uint8_t intel_vvp_scaler_get_num_vertical_banks(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->v_num_banks;
}


bool intel_vvp_scaler_are_vertical_coeffs_signed(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->v_coeffs_signed;
}


uint8_t intel_vvp_scaler_get_vertical_coeffs_int_bits(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->v_coeffs_int_bits;
}


uint8_t intel_vvp_scaler_get_vertical_coeffs_frac_bits(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->v_coeffs_frac_bits;
}

bool intel_vvp_scaler_is_horizontal_scaling_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->h_scaling_enabled;
}

bool intel_vvp_scaler_is_partial_horizontal_scaling_enabled(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->h_partial_scaling_enabled;
}

uint8_t intel_vvp_scaler_get_num_horizontal_taps(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->h_num_taps;
}

uint16_t intel_vvp_scaler_get_num_horizontal_phases(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->h_num_phases;
}

uint8_t intel_vvp_scaler_get_num_horizontal_banks(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->h_num_banks;
}

bool intel_vvp_scaler_are_horizontal_coeffs_signed(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->h_coeffs_signed;
}

uint8_t intel_vvp_scaler_get_horizontal_coeffs_int_bits(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->h_coeffs_int_bits;
}

uint8_t intel_vvp_scaler_get_horizontal_coeffs_frac_bits(intel_vvp_scaler_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->h_coeffs_frac_bits;
}

bool intel_vvp_scaler_is_running(intel_vvp_scaler_instance *instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_STATUS_REG);
    return INTEL_VVP_SCALER_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_scaler_get_commit_status(intel_vvp_scaler_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_STATUS_REG);
    return INTEL_VVP_SCALER_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_scaler_get_status(intel_vvp_scaler_instance *instance)
{
    if (instance == NULL) return 0xFF;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_STATUS_REG);
}


int intel_vvp_scaler_set_h_bank(intel_vvp_scaler_instance *instance, uint8_t bank)
{
    if ((instance == NULL) || (instance->scaling_algo != kIntelVvpScalerPolyphaseScaling) ||
            !instance->h_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (bank >= instance->h_num_banks) return kIntelVvpScalerParameterErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_BANK_REG, bank);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_v_bank(intel_vvp_scaler_instance *instance, uint8_t bank)
{
    if ((instance == NULL) || (instance->scaling_algo != kIntelVvpScalerPolyphaseScaling) ||
            !instance->v_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (bank >= instance->v_num_banks) return kIntelVvpScalerParameterErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_BANK_REG, bank);

    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_set_output_width(intel_vvp_scaler_instance *instance, uint32_t output_width)
{
    if ((instance == NULL) || !instance->h_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if ((output_width == 0) || (output_width > instance->max_output_width)) return kIntelVvpScalerParameterErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_OUTPUT_WIDTH_REG, output_width);

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_output_height(intel_vvp_scaler_instance *instance, uint32_t output_height)
{
    if ((instance == NULL) || !instance->v_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (output_height == 0) return kIntelVvpScalerParameterErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_OUTPUT_HEIGHT_REG, output_height);

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_total_input_width(intel_vvp_scaler_instance *instance, uint32_t total_input_width)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (total_input_width == 0) return kIntelVvpScalerParameterErr;

    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_IN_WIDTH_REG, total_input_width);

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_total_output_width(intel_vvp_scaler_instance *instance, uint32_t total_output_width)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (total_output_width == 0) return kIntelVvpScalerParameterErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_OUT_WIDTH_REG, total_output_width);

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_total_input_height(intel_vvp_scaler_instance *instance, uint32_t total_input_height)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (total_input_height == 0) return kIntelVvpScalerParameterErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_IN_HEIGHT_REG, total_input_height);
    
    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_set_total_output_height(intel_vvp_scaler_instance *instance, uint32_t total_output_height)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (total_output_height == 0) return kIntelVvpScalerParameterErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_OUT_HEIGHT_REG, total_output_height);
    
    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_set_h_pos_error(intel_vvp_scaler_instance *instance, uint32_t h_pos_err)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_POS_ERROR_REG, h_pos_err);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_v_pos_error(intel_vvp_scaler_instance *instance, uint32_t v_pos_err)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_POS_ERROR_REG, v_pos_err);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_h_extended_fill(intel_vvp_scaler_instance *instance, bool extended_fill)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_EXTENDED_FILL_REG, extended_fill ? 1 : 0);

    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_set_v_extended_fill(intel_vvp_scaler_instance *instance, bool extended_fill)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_EXTENDED_FILL_REG, extended_fill ? 1 : 0);
    
    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_set_h_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t phase_offset, uint32_t phase_error)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (kIntelVvpScalerPolyphaseScaling != instance->scaling_algo) return kIntelVvpCoreInstanceErr;
        
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_PHASE_OFFSET_REG, phase_offset);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_PHASE_ERROR_REG, phase_error);

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_v_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t phase_offset, uint32_t phase_error)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if (kIntelVvpScalerPolyphaseScaling != instance->scaling_algo) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_PHASE_OFFSET_REG, phase_offset);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_PHASE_ERROR_REG, phase_error);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_split_h_scaling(intel_vvp_scaler_instance *instance,
                                         uint32_t total_input_width, uint32_t total_output_width,
                                         uint32_t split_output_width, uint32_t split_output_left_offset, bool extended_fill,
                                         uint32_t *split_input_width, uint32_t *split_input_left_offset)
{
    uint32_t left_window;
    uint32_t right_window;
    uint32_t input_error;
    uint32_t input_phase;
    uint32_t input_phase_error;
    uint32_t last_output_pix;
    uint32_t input_left_offset;
    uint32_t last_input_pix;
    uint32_t input_width;
    uint32_t total_split_input_width;
    uint8_t pip;
    
    
    if ((instance == NULL) || !instance->h_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if ((total_input_width == 0) || (total_output_width == 0) || (split_output_width == 0)) return kIntelVvpScalerParameterErr;
    if (split_output_width > instance->max_output_width) return kIntelVvpScalerParameterErr;
    
    last_output_pix = (split_output_width + split_output_left_offset) - 1;
    if (last_output_pix >= total_output_width) return kIntelVvpScalerParameterErr;
    
    input_left_offset = (split_output_left_offset*total_input_width)/total_output_width;
    last_input_pix = (last_output_pix*total_input_width)/total_output_width;
    input_width = (last_input_pix + 1) - input_left_offset;

    left_window = 0;
    right_window = 0;
    
    // extended_fill/overscan; update split_input_left_offset and split_input_width for overscan window
    if (extended_fill)
    {
        // left window
        pip = instance->pixels_in_parallel;
        left_window = (instance->h_num_taps - 1) / 2;
        left_window = ( (left_window + pip - 1) / pip ) * pip; // Round the left window to a multiple of pixels in parallel
        if (input_left_offset < left_window) left_window = 0; // Not enough pixels on the left to fill the taps

        // right window
        right_window = instance->h_num_taps / 2;
        if ((input_left_offset + input_width + right_window) > total_input_width)
        {
            right_window = total_input_width - (input_left_offset + input_width);
        }
    }
    total_split_input_width = input_width + left_window + right_window;
    
    if (split_input_left_offset != NULL)
    {
        *split_input_left_offset = input_left_offset - left_window;
    }
     if (split_input_width != NULL) {
         *split_input_width = total_split_input_width;
     }

    input_error = (split_output_left_offset*total_input_width)%total_output_width;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_OUTPUT_WIDTH_REG, split_output_width);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_IN_WIDTH_REG, total_input_width);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_OUT_WIDTH_REG, total_output_width);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_POS_ERROR_REG, input_error);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_EXTENDED_FILL_REG, (left_window != 0) ? 1 : 0);

    if (kIntelVvpScalerPolyphaseScaling == instance->scaling_algo) {
        input_phase = (split_output_left_offset*total_input_width*instance->h_num_phases)/total_output_width;
        input_phase_error = (split_output_left_offset*total_input_width*instance->h_num_phases)%total_output_width;
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_PHASE_OFFSET_REG, input_phase);
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_H_PHASE_ERROR_REG, input_phase_error);
    }

    if (total_split_input_width > instance->max_input_width) return kIntelVvpScalerParameterErr;

    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_split_v_scaling(intel_vvp_scaler_instance *instance,
                                         uint32_t total_input_height, uint32_t total_output_height,
                                         uint32_t split_output_height, uint32_t split_output_top_offset, bool extended_fill,
                                         uint32_t *split_input_height, uint32_t *split_input_top_offset)
{
    uint32_t top_window;
    uint32_t bottom_window;
    uint32_t input_error;
    uint32_t input_phase;
    uint32_t input_phase_error;
    uint32_t last_output_line;
    uint32_t last_input_line;
    uint32_t input_height;
    uint32_t input_top_offset;
    
    if ((instance == NULL) || !instance->v_partial_scaling_enabled) return kIntelVvpCoreInstanceErr;
    if ((total_input_height == 0) || (total_output_height == 0) || (split_output_height == 0)) return kIntelVvpScalerParameterErr;
    
    last_output_line = (split_output_height + split_output_top_offset) - 1;
    if (last_output_line >= total_output_height) return kIntelVvpScalerParameterErr;
    
    
    input_top_offset = (split_output_top_offset*total_input_height)/total_output_height;
    last_input_line = (last_output_line*total_input_height)/total_output_height;

    input_height = (last_input_line + 1) - input_top_offset;
    top_window = 0;
    bottom_window = 0;
    
    // extended_fill/overscan; update split_input_left_offset and split_input_width for overscan window
    if (extended_fill)
    {
        // left window
        top_window = (instance->v_num_taps - 1) / 2;
        if (input_top_offset < top_window) top_window = 0; // Not enough pixels on the top to fill the taps

        // right window
        bottom_window = instance->v_num_taps / 2;
        if ((input_top_offset + input_height + bottom_window) > total_input_height)
        {
            bottom_window = total_input_height - (input_top_offset+ input_height);
        }
    }
    
    if (split_input_top_offset) *split_input_top_offset = input_top_offset - top_window;
    
    if (split_input_height) *split_input_height = input_height + top_window + bottom_window;
    
    input_error = (split_output_top_offset*total_input_height)%total_output_height;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_OUTPUT_HEIGHT_REG, split_output_height);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_IN_HEIGHT_REG, total_input_height);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_TOTAL_OUT_HEIGHT_REG, total_output_height);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_POS_ERROR_REG, input_error);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_EXTENDED_FILL_REG, (top_window != 0) ? 1 : 0);

    if (kIntelVvpScalerPolyphaseScaling == instance->scaling_algo) {
        input_phase = (split_output_top_offset*total_input_height*instance->v_num_phases)/total_output_height;
        input_phase_error = (split_output_top_offset*total_input_height*instance->v_num_phases)%total_output_height;
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_PHASE_OFFSET_REG, input_phase);
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_V_PHASE_ERROR_REG, input_phase_error);
    }
    
    return kIntelVvpCoreOk;
}


uint8_t intel_vvp_scaler_get_h_bank(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || (instance->scaling_algo != kIntelVvpScalerPolyphaseScaling) ||
            !instance->h_scaling_enabled || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_BANK_REG);
}

uint8_t intel_vvp_scaler_get_v_bank(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || (instance->scaling_algo != kIntelVvpScalerPolyphaseScaling) ||
            !instance->v_scaling_enabled || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_BANK_REG);
}

uint32_t intel_vvp_scaler_get_output_width(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->h_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_OUTPUT_WIDTH_REG);
}

uint32_t intel_vvp_scaler_get_output_height(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->v_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_OUTPUT_HEIGHT_REG);
}

uint32_t intel_vvp_scaler_get_total_input_width(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_TOTAL_IN_WIDTH_REG);
}

uint32_t intel_vvp_scaler_get_total_output_width(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_TOTAL_OUT_WIDTH_REG);
}


uint32_t intel_vvp_scaler_get_total_input_height(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_TOTAL_IN_HEIGHT_REG);
}

uint32_t intel_vvp_scaler_get_total_output_height(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_TOTAL_OUT_HEIGHT_REG);
}

uint32_t intel_vvp_scaler_get_h_pos_error(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_POS_ERROR_REG);
}

uint32_t intel_vvp_scaler_get_v_pos_error(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled || !instance->debug_enabled) return 0;
    
    return INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_POS_ERROR_REG);
}

bool intel_vvp_scaler_get_h_extended_fill(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled || !instance->debug_enabled) return false;
    
    return (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_EXTENDED_FILL_REG));
}

bool intel_vvp_scaler_get_v_extended_fill(intel_vvp_scaler_instance *instance)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled || !instance->debug_enabled) return false;
    
    return (0 != INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_EXTENDED_FILL_REG));
}


int intel_vvp_scaler_get_h_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t *phase_offset, uint32_t *phase_error)
{
    if ((instance == NULL) || !instance->h_partial_scaling_enabled ||
        (kIntelVvpScalerPolyphaseScaling != instance->scaling_algo) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;
    
    if ((phase_offset == NULL) || (phase_error == NULL)) return kIntelVvpCoreNullPtrErr;
    
    *phase_offset = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_PHASE_OFFSET_REG);
    *phase_error = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_H_PHASE_ERROR_REG);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_get_v_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t *phase_offset, uint32_t *phase_error)
{
    if ((instance == NULL) || !instance->v_partial_scaling_enabled ||
        (kIntelVvpScalerPolyphaseScaling != instance->scaling_algo) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;
    
    if ((phase_offset == NULL) || (phase_error == NULL)) return kIntelVvpCoreNullPtrErr;
    
    *phase_offset = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_PHASE_OFFSET_REG);
    *phase_error = INTEL_VVP_SCALER_REG_IORD(instance, INTEL_VVP_SCALER_V_PHASE_ERROR_REG);
    
    return kIntelVvpCoreOk;
}


int intel_vvp_scaler_check_set_coeffs_priv(intel_vvp_scaler_instance* instance, bool h_coeffs, uint8_t num_coeffs, uint8_t bank, uint8_t phase)
{
    uint8_t num_banks;
    uint8_t num_phases;
    uint8_t num_taps;

    if ((instance == NULL) || !(instance->coeffs_runtime_load)) return kIntelVvpCoreInstanceErr;
    
    if (h_coeffs) 
    {
        if (!instance->h_scaling_enabled) return kIntelVvpCoreInstanceErr;
        
        num_banks = instance->h_num_banks;
        num_phases = instance->h_num_phases;
        num_taps = instance->h_num_taps;
    }
    else
    {
        if (!instance->v_scaling_enabled) return kIntelVvpCoreInstanceErr;
        
        num_banks =  instance->v_num_banks;
        num_phases = instance->v_num_phases;
        num_taps = instance->v_num_taps;
    }

    if ((bank >= num_banks) || (phase >= num_phases) || (num_coeffs != num_taps)) return kIntelVvpScalerParameterErr;
    
    return kIntelVvpCoreOk;
}

int intel_vvp_scaler_set_quantized_coeffs(intel_vvp_scaler_instance* instance, bool h_coeffs, const int32_t* quantized_coeffs,
                                          uint8_t num_coeffs, uint8_t bank, uint8_t phase)
{
    int err_ret;
    bool coeffs_signed;
    uint16_t nb_bits;
    uint32_t sign_mask;
    uint32_t mask;
    uint32_t masked_coeff;
    int c;
    
    err_ret = intel_vvp_scaler_check_set_coeffs_priv(instance, h_coeffs, num_coeffs, bank, phase);
    
    if (err_ret != kIntelVvpCoreOk) return err_ret;
    if (quantized_coeffs == NULL) return kIntelVvpCoreNullPtrErr;

    if (h_coeffs) 
    {
        coeffs_signed = instance->h_coeffs_signed;
        nb_bits = instance->h_coeffs_int_bits + instance->h_coeffs_frac_bits;
    }
    else
    {
        coeffs_signed = instance->v_coeffs_signed;
        nb_bits = instance->v_coeffs_int_bits + instance->v_coeffs_frac_bits;
    }

    // Prepare a mask with unused bits set to 1. if (mask & coeff != 0) then the coefficient is out of range unless it is negative
    if (nb_bits > 31)
    {
        err_ret = kIntelVvpScalerQuantizationErr;
    }
    else
    {
        sign_mask = 1U << nb_bits;
        mask = ~(sign_mask - 1);
        for (c = 0; c < num_coeffs; ++c)
        {
            // Assume that having a bit set outside the number of allowed bits is an error but make allowance
            // for sign extension
            masked_coeff = quantized_coeffs[c] & mask;
            if ((masked_coeff != 0) && (!coeffs_signed || ((masked_coeff != sign_mask) && (masked_coeff != mask))))
            {
                err_ret = kIntelVvpScalerQuantizationErr;
            }
        }
    }
    // Write the coefficients out even if a quantization error was flagged
    for (c = 0; c < num_coeffs; ++c)
    {
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_TAP_REG(c), quantized_coeffs[c]);
    }
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_BANK_SELECT_REG, bank);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_PHASE_SELECT_REG, phase);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_COMMIT_REG, h_coeffs);
    
    
    return err_ret;    
}

int intel_vvp_scaler_set_coeffs(intel_vvp_scaler_instance* instance, bool h_coeffs, const float* coeffs, uint8_t num_coeffs, uint8_t bank, uint8_t phase)
{
    int err_ret;
    int quant_err;
    bool coeffs_signed;
    uint8_t coeffs_int_bits;
    uint8_t coeffs_frac_bits;
    uint8_t num_phases;
    int c;
    int32_t quantized_coeffs[INTEL_VVP_SCALER_MAX_NUM_TAPS];

    err_ret = intel_vvp_scaler_check_set_coeffs_priv(instance, h_coeffs, num_coeffs, bank, phase);
    
    if (err_ret != kIntelVvpCoreOk) return err_ret;
    if (coeffs == NULL) return kIntelVvpCoreNullPtrErr;
    if (num_coeffs > INTEL_VVP_SCALER_MAX_NUM_TAPS) return kIntelVvpScalerParameterErr;

    if (h_coeffs) 
    {
        coeffs_signed = instance->h_coeffs_signed;
        coeffs_int_bits = instance->h_coeffs_int_bits;
        coeffs_frac_bits = instance->h_coeffs_frac_bits;
        num_phases = instance->h_num_phases;
    }
    else
    {
        coeffs_signed = instance->v_coeffs_signed;
        coeffs_int_bits = instance->v_coeffs_int_bits;
        coeffs_frac_bits = instance->v_coeffs_frac_bits;
        num_phases = instance->v_num_phases;
    }

    quant_err = intel_vvp_quantize_error_propagate(coeffs, num_coeffs, coeffs_signed, coeffs_int_bits, coeffs_frac_bits,
                                                   phase >= (num_phases+1)/2, quantized_coeffs);

    // Write the coefficients out even if a quantization error was flagged
    for (c = 0; c < num_coeffs; ++c)
    {
        INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_TAP_REG(c), quantized_coeffs[c]);
    }
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_BANK_SELECT_REG, bank);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_PHASE_SELECT_REG, phase);
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_RT_COEFF_LOAD_COMMIT_REG, h_coeffs);

    return (quant_err != 0) ? kIntelVvpScalerQuantizationErr : kIntelVvpCoreOk;
}


int intel_vvp_scaler_commit_writes(intel_vvp_scaler_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
    
    INTEL_VVP_SCALER_REG_IOWR(instance, INTEL_VVP_SCALER_COMMIT_REG, 1); // Any write would work
    return kIntelVvpCoreOk;
}

