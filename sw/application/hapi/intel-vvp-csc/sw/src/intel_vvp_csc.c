/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_csc.h"
#include "intel_vvp_csc_regs.h"
#include "intel_vvp_quantizer.h"

int intel_vvp_csc_init(intel_vvp_csc_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint32_t rounding;
    uint8_t regmap_version;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_CSC_PRODUCT_ID);
    
   // Check the regmap version if the the intel_vvp_core_instance initialize without errors (ie, vendor_id and product_id validated properly)
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_CSC_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_CSC_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpCscRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_DEBUG_ENABLED_REG));
        instance->bps_in = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_BPS_IN_REG);
        instance->bps_out = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_BPS_OUT_REG);
        instance->frac_bits = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_FRAC_BITS_REG);
        instance->coeffs_signed = (0 != INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_COEFFS_SIGNED_REG));
        instance->coeffs_int_bits = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_COEFFS_INT_BITS_REG);
        instance->summands_signed = (0 != INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_SUMMANDS_SIGNED_REG));
        instance->summands_int_bits = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_SUMMANDS_INT_BITS_REG);
        instance->binary_point_right_move = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_BINARY_POINT_RIGHT_MOVE_REG);
        rounding = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_ROUND_METHOD_REG);
        if ((rounding == kIntelVvpCscRoundUp) || (rounding == kIntelVvpCscRoundHalfEven) || (rounding == kIntelVvpCscRoundTruncate))
        {
            instance->rounding_method = (eIntelVvpCscRounding)rounding;
        }
        else 
        {
            instance->rounding_method = kIntelVvpCscRoundInvalid;
            init_ret = kIntelVvpCscRegMapVersionErr;
        }
    }
    return init_ret;
}
 
bool intel_vvp_csc_get_lite_mode(intel_vvp_csc_instance* instance)
{
    return instance ? instance->lite_mode : false;
}

bool intel_vvp_csc_get_debug_enabled(intel_vvp_csc_instance* instance)
{
    return instance ? instance->debug_enabled : false;
}

uint8_t intel_vvp_csc_get_bits_per_sample_in(intel_vvp_csc_instance* instance)
{
    return instance ? instance->bps_in : 0;
}

uint8_t intel_vvp_csc_get_bits_per_sample_out(intel_vvp_csc_instance* instance)
{
    return instance? instance->bps_out : 0;
}

uint8_t intel_vvp_csc_get_coeffs_frac_bits(intel_vvp_csc_instance* instance)
{
    return instance ? instance->frac_bits : 0;
}

bool intel_vvp_csc_are_coeffs_signed(intel_vvp_csc_instance* instance)
{
    return instance ? instance->coeffs_signed : false;
}

uint8_t intel_vvp_csc_get_coeffs_int_bits(intel_vvp_csc_instance* instance)
{
    return instance ? instance->coeffs_int_bits : 0;
}

bool intel_vvp_csc_are_summands_signed(intel_vvp_csc_instance* instance)
{
    return instance ? instance->summands_signed : false;
}

uint8_t intel_vvp_csc_get_summands_int_bits(intel_vvp_csc_instance* instance)
{
    return instance ? instance->summands_int_bits : 0;
}

int8_t intel_vvp_csc_get_binary_point_right_move(intel_vvp_csc_instance* instance)
{
    return instance ? instance->binary_point_right_move : 0;
}

eIntelVvpCscRounding intel_vvp_csc_get_rounding_method(intel_vvp_csc_instance* instance)
{
    return instance ? instance->rounding_method : kIntelVvpCscRoundInvalid;
}

bool intel_vvp_csc_is_running(intel_vvp_csc_instance *instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_STATUS_REG);
    return INTEL_VVP_CSC_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_csc_get_commit_status(intel_vvp_csc_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_STATUS_REG);
    return INTEL_VVP_CSC_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_csc_get_status(intel_vvp_csc_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_STATUS_REG);
    return status_reg;
}

int intel_vvp_csc_set_quantized_coeff_data(intel_vvp_csc_instance* instance, const intel_vvp_quantized_coefficients* quantized_coeffs)
{
    int err_ret;
    int nb_bits;
    uint32_t sign_mask;
    uint32_t mask;
    uint32_t masked_coeff;
    const int32_t *quant_coeffs;
    int c;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (quantized_coeffs == NULL) return kIntelVvpCoreNullPtrErr;
    
    err_ret = kIntelVvpCoreOk;

    // Prepare a mask with unused bits set to 1. if (mask & coeff != 0) then the coefficient is out of range unless it is negative
    nb_bits = instance->coeffs_int_bits + instance->frac_bits;
    sign_mask = 1U << nb_bits;
    mask = ~(sign_mask - 1);
    quant_coeffs = (const int32_t*)quantized_coeffs->coeffs;
    for (c = 0; c < INTEL_VVP_CSC_NUM_COEFFS; ++c)
    {
        // Assume that having a bit set outside the number of allowed bits is an error but make allowance
        // for sign bit or possible sign extension to 32 bits
        masked_coeff = quant_coeffs[c] & mask;
        if ((masked_coeff != 0) && (!instance->coeffs_signed || ((masked_coeff != sign_mask) && (masked_coeff != mask))))
        {
            err_ret = kIntelVvpCscQuantizationErr;
        }
        INTEL_VVP_CSC_REG_IOWR(instance, INTEL_VVP_CSC_COEFFS_BASE_REG + c, quant_coeffs[c]);
    }
    
    // Repeat the operation for summands
    nb_bits = instance->summands_int_bits + instance->frac_bits;
    sign_mask = 1U << nb_bits;
    mask = ~(sign_mask - 1);
    for (c = 0; c < INTEL_VVP_CSC_NUM_SUMMANDS; ++c)
    {
        // Assume that having a bit set outside the number of allowed bits is an error but make allowance
        // for sign bit or possible sign extension to 32 bits
        masked_coeff = quantized_coeffs->s[c] & mask;
        if ((masked_coeff != 0) && (!instance->summands_signed || ((masked_coeff != sign_mask) && (masked_coeff != mask))))
        {
            err_ret = kIntelVvpCscQuantizationErr;
        }
        INTEL_VVP_CSC_REG_IOWR(instance, INTEL_VVP_CSC_SUMMANDS_BASE_REG + c, quantized_coeffs->s[c]);
    }
    
    return err_ret;
}


int intel_vvp_csc_get_quantized_coeff_data(intel_vvp_csc_instance* instance, intel_vvp_quantized_coefficients* quantized_coeffs)
{
    int32_t *coeffs;
    int c;
    
    if ((instance == NULL) || (!instance->debug_enabled)) return kIntelVvpCoreInstanceErr;
    if (quantized_coeffs == NULL) return kIntelVvpCoreNullPtrErr;
    
    coeffs = (int32_t*)quantized_coeffs;
    for (c = 0; c < (INTEL_VVP_CSC_NUM_COEFFS + INTEL_VVP_CSC_NUM_SUMMANDS); ++c)
    {
        coeffs[c] = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_COEFFS_BASE_REG + c);
    }
    return kIntelVvpCoreOk;
}

int intel_vvp_csc_set_coeff_data(intel_vvp_csc_instance* instance, const intel_vvp_coefficients* coeffs, int summand_rescale)
{
    intel_vvp_quantized_coefficients quantized_coeffs;
    float summands[INTEL_VVP_CSC_NUM_SUMMANDS];
    int err_coeffs_ret;
    int err_summands_ret;
    int err_quant_set;
    int c;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (coeffs == NULL) return kIntelVvpCoreNullPtrErr;

    for (c = 0; c < INTEL_VVP_CSC_NUM_SUMMANDS; ++c)
    {
        summands[c] = coeffs->s[c];
        if (summand_rescale > 0 )
        {
            summands[c] = summands[c] * (float)(1 << summand_rescale);
        }
        else if (summand_rescale < 0 )
        {
            summands[c] = summands[c] / (float)(1 << -summand_rescale);
        }
    }
    
    err_coeffs_ret = intel_vvp_quantize((float*)coeffs->coeffs, INTEL_VVP_CSC_NUM_COEFFS, instance->coeffs_signed, instance->coeffs_int_bits, instance->frac_bits, (int32_t*)quantized_coeffs.coeffs);
    err_summands_ret = intel_vvp_quantize(summands, INTEL_VVP_CSC_NUM_SUMMANDS,
                                              instance->summands_signed, instance->summands_int_bits, instance->frac_bits, quantized_coeffs.s);

    err_quant_set = intel_vvp_csc_set_quantized_coeff_data(instance, &quantized_coeffs);
    return (err_coeffs_ret || err_summands_ret) ? kIntelVvpCscQuantizationErr : err_quant_set;
}

int intel_vvp_csc_get_coeff_data(intel_vvp_csc_instance* instance, intel_vvp_coefficients* coeffs)
{
    intel_vvp_quantized_coefficients quantized_coeffs;
    float *float_coeffs;
    int32_t *quant_coeffs;
    int err_ret;
    uint32_t sign_mask;
    uint32_t mask;
    int c_size;
    bool c_signed;
    bool neg;
    float divider;
    int c;
    
    if ((instance == NULL) || (!instance->debug_enabled)) return kIntelVvpCoreInstanceErr;
    if (coeffs == NULL) return kIntelVvpCoreNullPtrErr;
    
    err_ret = intel_vvp_csc_get_quantized_coeff_data(instance, &quantized_coeffs);
    if (err_ret) return err_ret;
    
    divider = 1.0f / (float)(1U << instance->frac_bits);
    
    c_signed = instance->coeffs_signed;
    c_size = instance->coeffs_int_bits + instance->frac_bits; // excluding sign bit if it exists
    sign_mask = 1 << c_size;
    mask = 0xFFFFFFFF << c_size;
    float_coeffs = (float*)(coeffs->coeffs);
    quant_coeffs = (int32_t*)(quantized_coeffs.coeffs);
    for (c = 0; c < INTEL_VVP_CSC_NUM_COEFFS; ++c)
    {
        if (c_signed)
        {
            // Masking of unused MSBs with sign extension
            neg = quant_coeffs[c] & sign_mask;
            quant_coeffs[c] = neg ? (mask | quant_coeffs[c]) : ((~mask) & quant_coeffs[c]);
        }
        else
        {
            // Masking of unused MSBs
            quant_coeffs[c] = (~mask) & quant_coeffs[c];
        }
        float_coeffs[c] = (float)quant_coeffs[c] * divider;
    }
    
    c_signed = instance->summands_signed;
    c_size = instance->summands_int_bits + instance->frac_bits; // excluding sign bit if it exists
    sign_mask = 1 << c_size;
    mask = 0xFFFFFFFF << c_size;
    for (c = 0; c < INTEL_VVP_CSC_NUM_SUMMANDS; ++c)
    {
        if (c_signed)
        {
            // Masking of unused MSBs with sign extension
            neg = quantized_coeffs.s[c] & sign_mask;
            quantized_coeffs.s[c] = neg ? (mask | quantized_coeffs.s[c]) : ((~mask) & quantized_coeffs.s[c]);
        }
        else
        {
            // Masking of unused MSBs
            quantized_coeffs.s[c] = (~mask) & quantized_coeffs.s[c];            
        }
        coeffs->s[c] = (float)quantized_coeffs.s[c] * divider;
    }
    return kIntelVvpCoreOk;
}

int intel_vvp_csc_set_output_color_space(intel_vvp_csc_instance* instance, int8_t output_color_space)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
    
    if (output_color_space < 0) return kIntelVvpCscParameterErr;
    
    INTEL_VVP_CSC_REG_IOWR(instance, INTEL_VVP_CSC_OUTPUT_CS_REG, output_color_space);
    
    return kIntelVvpCoreOk;
}

int8_t intel_vvp_csc_get_output_color_space(intel_vvp_csc_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode || !instance->debug_enabled) return kIntelVvpCsInvalid;
    
    uint32_t output_color_space = INTEL_VVP_CSC_REG_IORD(instance, INTEL_VVP_CSC_OUTPUT_CS_REG);
    
    return output_color_space;
}

int intel_vvp_csc_commit_writes(intel_vvp_csc_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;
    
    INTEL_VVP_CSC_REG_IOWR(instance, INTEL_VVP_CSC_COMMIT_REG, 1); // Any write would work
    return kIntelVvpCoreOk;
}
