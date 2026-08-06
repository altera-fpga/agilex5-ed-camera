/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief  Common definitions for quantization functions
 *
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 */
    
#ifndef __INTEL_VVP_QUANTIZER_H__
#define __INTEL_VVP_QUANTIZER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    kIntelVvpQuantizerOk          = 0,
    kIntelVvpQuantizerNullPtrErr  = -200,
    kIntelVvpQuantizerParamErr    = -201,
    kIntelVvpQuantizerRangeErr    = -202,
} eIntelVvpQuantizationErrors;

/**
 * \brief Quantization function
 * 
 * \param[in]  float_coeffs, floating point values to quantize
 * \param[in]  num_coeffs, number of coefficients
 * \param[in]  is_signed, whether the quantization can use a sign bit
 * \param[in]  int_bits, number of integer bits available to quantize the coefficients (note that this does not include the sign bit)
 * \param[in]  frac_bits, number of fractional bits available to quantize the coefficients
 * \param[out] quantized_coeffs, quantized coefficients
 * \return     kIntelVvpQuantizerOk if no error detected,
 *             kIntelVvpQuantizerRangeErr if truncation has to be applied to keep the floating point value in quantized range
 *             kIntelVvpQuantizerNullPtrErr if one of the array parameter is a NULL pointers
 *             kIntelVvpQuantizerParamErr if the total number of bits av!ailable for quantization is over 32
 * \pre        float_coeffs and quantized_coeffs are valid pointers to num_coeffs coefficients
 * \pre        the number of bits used for the quantization is (is_signed ? 1 : 0) + int_bits + frac_bits.
 *             int_bits + frac_bits must be 31 bits or lower
 * \remarks    unused bits on 32-bit quantized coeffs are sign extended if is_signed == true
 */
eIntelVvpQuantizationErrors intel_vvp_quantize(const float *float_coeffs, int num_coeffs, bool is_signed, int int_bits, int frac_bits, int32_t *quantized_coeffs);

/**
 * \brief Quantization function with error propagation
 * 
 * \param[in]  float_coeffs, floating point values to quantize
 * \param[in]  num_coeffs, number of coefficients
 * \param[in]  is_signed, whether the quantization can use a sign bit
 * \param[in]  int_bits, number of integer bits available to quantize the coefficients (note that this does not include the sign bit)
 * \param[in]  frac_bits, number of fractional bits available to quantize the coefficients
 * \param[in]  invert_direction, set to true to quantize from num_coeffs-1 down to 0 (and propagate the error in that direction as well),
 *                               false to quantize in the natural ordering (0 to num_coeffs-1)
 * \param[out] quantized_coeffs, quantized coefficients
 * \return     kIntelVvpQuantizerOk if no error detected,
 *             kIntelVvpQuantizerRangeErr if truncation has to be applied to keep the floating point value in quantized range
 *             kIntelVvpQuantizerNullPtrErr if one of the array parameter is a NULL pointers
 *             kIntelVvpQuantizerParamErr if the total number of bits available for quantization is over 32
 * \pre        float_coeffs and quantized_coeffs are valid pointers to num_coeffs coefficients
 * \pre        the number of bits used for the quantization is (is_signed ? 1 : 0) + int_bits + frac_bits.
 *             int_bits + frac_bits must be 31 bits or lower
 * \remarks    unused bits on 32-bit quantized coeffs are sign extended if is_signed == true
 */
eIntelVvpQuantizationErrors intel_vvp_quantize_error_propagate(const float *float_coeffs, int num_coeffs, bool is_signed, int int_bits, int frac_bits, bool invert_direction, int32_t *quantized_coeffs);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_QUANTIZER_H__ */