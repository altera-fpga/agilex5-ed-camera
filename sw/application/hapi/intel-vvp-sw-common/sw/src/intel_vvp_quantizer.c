/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_quantizer.h"

eIntelVvpQuantizationErrors intel_vvp_quantize(const float *float_coeffs, int num_coeffs, bool is_signed, int int_bits, int frac_bits, int32_t *quantized_coeffs)
{
    int nb_bits;
    int c;
    float scaled;
    int32_t signed_lower;
    int32_t signed_upper;
    int32_t signed_coeff_data;
    uint32_t unsigned_upper;
    uint32_t unsigned_coeff_data;
    
  
    if ((float_coeffs == NULL) || (quantized_coeffs == NULL)) return kIntelVvpQuantizerNullPtrErr;
    
    nb_bits = (is_signed ? 1 : 0) + int_bits + frac_bits;
    if ((int_bits + frac_bits) > 31) return kIntelVvpQuantizerParamErr;

    eIntelVvpQuantizationErrors quant_ret = kIntelVvpQuantizerOk;
    if (is_signed)
    {
        signed_lower = -(1 << (nb_bits - 1));
        signed_upper = (1 << (nb_bits-1)) - 1;
        for (c = 0; c < num_coeffs; ++c)
        {
            scaled = float_coeffs[c] * (float)(1U << frac_bits);
            signed_coeff_data = (scaled >= 0.0f) ? scaled + 0.5f : scaled - 0.5f;   // Round away from 0
            if (signed_coeff_data < signed_lower)
            {
                signed_coeff_data = signed_lower;
                quant_ret = kIntelVvpQuantizerRangeErr;
            }
            else if (signed_coeff_data > signed_upper) {
                signed_coeff_data = signed_upper;
                quant_ret = kIntelVvpQuantizerRangeErr;
            }
            quantized_coeffs[c] = signed_coeff_data;
        } 
    }
    else
    {
        unsigned_upper = (1U << nb_bits) - 1; // should work for nb_bits == 32 as well
        for (c = 0; c < num_coeffs; ++c)
        {
            scaled = float_coeffs[c] * (float)(1U << frac_bits);
            unsigned_coeff_data = scaled + 0.5f;   // Round up
            if (scaled < 0.0f)
            {
                unsigned_coeff_data = 0;
                quant_ret = kIntelVvpQuantizerRangeErr;
            }
            else if (unsigned_coeff_data > unsigned_upper)
            {
                unsigned_coeff_data = unsigned_upper;
                quant_ret = kIntelVvpQuantizerRangeErr;
            }
            quantized_coeffs[c] = (int32_t)unsigned_coeff_data;
        }
    }
    return quant_ret;
}


eIntelVvpQuantizationErrors intel_vvp_quantize_error_propagate(const float *float_coeffs, int num_coeffs, bool is_signed, int int_bits, int frac_bits, bool invert_direction, int32_t *quantized_coeffs)
{
    int nb_bits;
    int c;
    int cur_coeff;
    float scaled;
    float scaled_err_prop;
    int32_t signed_lower;
    int32_t signed_upper;
    int32_t signed_coeff_data;
    uint32_t unsigned_upper;
    uint32_t unsigned_coeff_data;
    
    if ((float_coeffs == NULL) || (quantized_coeffs == NULL)) return kIntelVvpQuantizerNullPtrErr;
    
    nb_bits = (is_signed ? 1 : 0) + int_bits + frac_bits;
    if (nb_bits > 32) return kIntelVvpQuantizerParamErr;

    eIntelVvpQuantizationErrors quant_ret = kIntelVvpQuantizerOk;
    scaled = 0.0f;
    scaled_err_prop = 0.0f;
    if (is_signed)
    {
        signed_lower = -(1 << (nb_bits - 1));
        signed_upper = (1 << (nb_bits-1)) - 1;
        for (c = 0; c < num_coeffs; ++c) {
            cur_coeff = invert_direction ? ((num_coeffs-1)-c) : c;
            scaled = float_coeffs[cur_coeff] * (float)(1U << frac_bits);
            scaled_err_prop += scaled;
            signed_coeff_data = (scaled_err_prop >= 0.0f) ? scaled_err_prop + 0.5f : scaled_err_prop - 0.5f;   // Round away from 0
            if (signed_coeff_data < signed_lower)
            {
                signed_coeff_data = signed_lower;
                if (scaled < signed_lower) quant_ret = kIntelVvpQuantizerRangeErr;
            }
            else if (signed_coeff_data > signed_upper) {
                signed_coeff_data = signed_upper;
                if (scaled > signed_upper) quant_ret = kIntelVvpQuantizerRangeErr;
            }
            quantized_coeffs[cur_coeff] = signed_coeff_data;
            scaled_err_prop -= (float)signed_coeff_data;
        }
    }
    else
    {
        unsigned_upper = (uint32_t)(-1);

        if(nb_bits < 32)
            unsigned_upper = (1U << nb_bits) - 1;


        for (c = 0; c < num_coeffs; ++c) {
            cur_coeff = invert_direction ? ((num_coeffs-1)-c) : c;
            scaled = float_coeffs[cur_coeff] * (float)(1U << frac_bits);
            scaled_err_prop += scaled;
            unsigned_coeff_data = scaled_err_prop + 0.5f;   // Round up
            if (scaled_err_prop < 0.0f)
            {
                unsigned_coeff_data = 0;
                if (scaled < 0.0f) quant_ret = kIntelVvpQuantizerRangeErr;
            }
            else if (unsigned_coeff_data > unsigned_upper)
            {
                unsigned_coeff_data = unsigned_upper;
                if (scaled > unsigned_upper) quant_ret = kIntelVvpQuantizerRangeErr;
            }
            quantized_coeffs[cur_coeff] = (int32_t)unsigned_coeff_data;
            scaled_err_prop -= (float)unsigned_coeff_data;
        }
    }
    return quant_ret;
}
