/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_csc_instance and associated functions
 *
 * Driver for the Video & Vision Processing Color Space Converter Intel FPGA IP
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_csc_regs.h
 */
#ifndef __INTEL_VVP_CSC_H__
#define __INTEL_VVP_CSC_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_csc_regs.h"

#define INTEL_VVP_CSC_PRODUCT_ID                           0x022fu              ///< CSC product ID
#define INTEL_VVP_CSC_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_CSC_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_CSC_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< CSC register read function
#define INTEL_VVP_CSC_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< CSC register write function

// Some convenient defines to use when trying to set up a color space conversion
// Please be aware that the summand values given here are given assuming a specific value for input_bps.
// If you use these defines with input_bps not matching the assumptions, you probably want to apply
// a scaling factor (summand_rescale in the API) to the So,S1,S2 values in the presets given below
// summand_rescale=actual_input_bps - table_input_bps;
// new_summand_value = table_summand_value * (1 << summand_rescale);
// or, if summand_rescale is negative, new_summand_value = table_summand_value / (1 << -summand_rescale);

// Coeffs are given in the order of the register map:
// {{A0, B0, C0},
//  {A1, B1, C1},
//  {A2, B2, C2},
//  {S0, S1, S2}
// The CSC operation is:
// out_x           = Ax * in_0   +   Bx * in_1  +   Cx * in 2   +  Sx
// where out_0 and in_0  the color sample streamed on the least significant bits whereas out_2 and in_2 are on the most significant bits
// The usual streaming orders used in the Video and Vision Processing suite are   B,G,R   and   Cb,Y,Cr   with B/Cb on the least
// significant bits and R/Cr on the most significant bits. The tables given below are assuming this is the ordering used for inputs and outputs
#ifdef CSC_PASSTHROUGH_COEFFS
#undef CSC_PASSTHROUGH_COEFFS
#endif
#define CSC_PASSTHROUGH_COEFFS    {  {{1.0f, 0.0f, 0.0f},        \
                                      {0.0f, 1.0f, 0.0f},        \
                                      {0.0f, 0.0f, 1.0f}},       \
                                     { 0.0f, 0.0f, 0.0f }   }

// 8 bits (computer) BGR -> CbYCr SDTV
#undef CSC_RGB_TO_YCCSD_COEFFS
#define CSC_RGB_TO_YCCSD_COEFFS  {  {{   0.439f,   -0.291f,   -0.148f },        \
                                     {   0.098f,    0.504f,    0.257f },        \
                                     {  -0.071f,   -0.368f,    0.439f }},       \
                                    {  128.000f,   16.000f,  128.000f  }  }
// 8 bits CbYCr SDTV -> (computer) BGR
#undef CSC_YCCSD_TO_RGB_COEFFS
#define CSC_YCCSD_TO_RGB_COEFFS  {  {{   2.018f,    1.164f,    0.000f },        \
                                     {  -0.391f,    1.164f,   -0.813f },        \
                                     {   0.000f,    1.164f,    1.596f }},       \
                                    { -276.928f,  135.488f, -222.912f  }  }

// 8 bits (studio) BGR <-> CbYCr SDTV
#undef CSC_RGBS_TO_YCCSD_COEFFS
#define CSC_RGBS_TO_YCCSD_COEFFS {  {{   0.511f,   -0.339f,   -0.172f },        \
                                     {   0.114f,    0.587f,    0.299f },        \
                                     {  -0.083f,   -0.428f,    0.511f }},       \
                                    {  128.000f,    0.000f,  128.000f  }  }
// 8 bits CbYCr SDTV -> (studio) BGR
#undef CSC_YCCSD_TO_RGBS_COEFFS
#define CSC_YCCSD_TO_RGBS_COEFFS {  {{   1.732f,    1.000f,    0.000f },        \
                                     {  -0.336f,    1.000f,   -0.698f },        \
                                     {   0.000f,    1.000f,    1.371f }},       \
                                    { -221.696f,  132.352f, -175.488f  }  }

// 8 bits (computer) BGR -> CbYCr HDTV, beware that summands are set for a 8 bits per sample input
#undef CSC_RGB_TO_YCCHD_COEFFS
#define CSC_RGB_TO_YCCHD_COEFFS  {  {{    0.439f,   -0.338f,   -0.101f },        \
                                     {    0.062f,    0.614f,    0.183f },        \
                                     {   -0.040f,   -0.399f,    0.439f }},       \
                                    {  128.000f,   16.000f,  128.000f   }  }
// 10 bit CbYCr HDTV -> (computer) BGR, beware that summands are set for a 10 bits per sample input
#undef CSC_YCCHD_TO_RGB_COEFFS
#define CSC_YCCHD_TO_RGB_COEFFS  {  {{   2.115f,    1.164f,    0.000f },        \
                                     {  -0.213f,    1.164f,   -0.534f },        \
                                     {   0.000f,    1.164f,    1.793f }},       \
                                    {-1157.376f,  307.968f, -992.512f  }  }


                                  
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct intel_vvp_coefficients_s
{
    struct
    {
        float c1;
        float c2;
        float c3;
    } coeffs[INTEL_VVP_CSC_NUM_COLOR_PLANES];
    float s[INTEL_VVP_CSC_NUM_COLOR_PLANES];
} intel_vvp_coefficients;

typedef struct intel_vvp_quantized_coefficients_s
{
    struct
    {
        int32_t c1;
        int32_t c2;
        int32_t c3;
    } coeffs[INTEL_VVP_CSC_NUM_COLOR_PLANES];
    int32_t s[INTEL_VVP_CSC_NUM_COLOR_PLANES];
} intel_vvp_quantized_coefficients;


typedef enum
{
    kIntelVvpCscRegMapVersionErr     = -100,
    kIntelVvpCscParameterErr  = -101,
    kIntelVvpCscQuantizationErr = -102,
} eIntelVvpCscErrors;

typedef enum
{
    kIntelVvpCscRoundUp           = INTEL_VVP_CSC_ROUND_UP,
    kIntelVvpCscRoundHalfEven     = INTEL_VVP_CSC_ROUND_HALF_EVEN,
    kIntelVvpCscRoundTruncate     = INTEL_VVP_CSC_ROUND_TRUNC,
    kIntelVvpCscRoundInvalid      = -1,
} eIntelVvpCscRounding;

typedef enum
{
    kIntelVvpCsRgb       = INTEL_VVP_CSC_OUTPUT_CS_RGB,
    kIntelVvpCsYcc       = INTEL_VVP_CSC_OUTPUT_CS_YCC,
    kIntelVvpCsMono      = INTEL_VVP_CSC_OUTPUT_CS_MONO,
    kIntelVvpCsInvalid   = -1,
} eIntelVvpCscColorSpace;

typedef struct intel_vvp_csc_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool    debug_enabled;
    bool    lite_mode;
    uint8_t bps_in;
    uint8_t bps_out;
    uint8_t frac_bits;
    bool    coeffs_signed;
    uint8_t coeffs_int_bits;
    bool    summands_signed;
    uint8_t summands_int_bits;
    int8_t  binary_point_right_move;
    eIntelVvpCscRounding rounding_method;

} intel_vvp_csc_instance;	   



/**
 * \brief Initialise a color space converter instance
 * 
 * Initialization function for a VVP color space converter instance.
 * Attempts to initialize the fields of the CSC and its base core
 * 
 * \param[in]    instance, pointer to the intel_vvp_csc_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the csc product id (0x022F)
 *               kIntelVvpCscRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_csc_init(intel_vvp_csc_instance* instance, intel_vvp_core_base base);
    
/**
 * \brief Query the lite_mode parameter of a csc instance
 *  
 * \param[in]   instance, pointer to the intel_vvp_csc_instance
 * \return      the lite_mode field in the intel_vvp_csc_instance
 * \pre         instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
bool intel_vvp_csc_get_lite_mode(intel_vvp_csc_instance* instance);

/**
 * \brief  Query the debug_enabled parameter of a csc instance
 *  
 * \param[in]   instance, pointer to the intel_vvp_csc_instance
 * \return      the debug_enabled field in the intel_vvp_csc_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
bool intel_vvp_csc_get_debug_enabled(intel_vvp_csc_instance* instance);

/**
 * \brief Get the the number of input bits per color plane of a csc instance
 * 
 * \param[in]   instance, pointer to the initialized intel_vvp_csc_instance
 * \return      number of bits 8-16
 * \pre         instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_bits_per_sample_in(intel_vvp_csc_instance* instance);

/**
 * \brief Get the the number of output bits per color plane of a csc instance
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     number of bits 8-16
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_bits_per_sample_out(intel_vvp_csc_instance* instance);

/**
 * \brief Get the number of fractional bits used for coeffients and summands
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     number of fractional bits used to quantize coefficients and summands
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_coeffs_frac_bits(intel_vvp_csc_instance* instance);

/**
 * \brief Get the signed/unsigned status of coefficients (summands excluded)
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     true if coefficients are signed and use an extra sign bit
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 * \see        intel_vvp_csc_summands_are_signed
 */
bool intel_vvp_csc_are_coeffs_signed(intel_vvp_csc_instance* instance);

/**
 * \brief Get the number of integer bits used for coefficients (summands excluded)
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     number of integer bits used to quantize coefficients
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_coeffs_int_bits(intel_vvp_csc_instance* instance);

/**
 * \brief  Get the signed/unsigned status of summands
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     true if summands are signed and use an extra sign bit
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
bool intel_vvp_csc_are_summands_signed(intel_vvp_csc_instance* instance);

/**
 * \brief Get the number of integer bits used for summands
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     number of integer bits used to quantize summands
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_summands_int_bits(intel_vvp_csc_instance* instance);

/**
 * \brief Get the compile-time left/right binary point move applied to the computation
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     the value of the right displacement of the binary point in number of bits.
 *             Positive values imply a multiplication of the output, ie, a left shift (typically for input_bps < output_bps)
 *             Negative values imply a division of the output, ie, a right shift (typically for output_bps < input_bps)
 * \remarks    shifting is used by the IP as a quick way to implement doubling/halving of
 *             coefficients and summands when get_bits_per_sample_in != get_bits_per_sample_out
 *             This could be implemented just as well with different coefficient/summand values
 *             but is, arguably, easier this way so that standard conversion tables can still be
 *             used as-is
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
int8_t intel_vvp_csc_get_binary_point_right_move(intel_vvp_csc_instance* instance);

/**
 * \brief Get internal rounding method used
 *  
 * \param[in]  instance, pointer to the intel_vvp_csc_instance
 * \return     rounding method enumerated type
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
eIntelVvpCscRounding intel_vvp_csc_get_rounding_method(intel_vvp_csc_instance* instance);

/**
 * \brief get running status of the csc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_csc_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
bool intel_vvp_csc_is_running(intel_vvp_csc_instance *instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]  instance, pointer to the intel_vvp_csc_instance 
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_csc_instance parameterized in full mode
 */
bool intel_vvp_csc_get_commit_status(intel_vvp_csc_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, an intel_vvp_csc_instance
 * \return the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
uint8_t intel_vvp_csc_get_status(intel_vvp_csc_instance *instance);

/**
 * \brief Set the current quantized coefficients (and summands)
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \param[in]  coeffs, pointer to the memory space to store returned values
 * \return     kIntelVvpCoreOk if instance and coeffs are valid pointers
 *             kIntelVvpCoreNullPtrErr, if the quantized_coeffs is the NULL pointer
 *             kIntelVvpCscQuantizationErr, if provided coefficients are out of range
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 */
int intel_vvp_csc_set_quantized_coeff_data(intel_vvp_csc_instance* instance, const intel_vvp_quantized_coefficients* quantized_coeffs);

/**
 * \brief Get the current quantized coefficients (and summands)
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \param[out] coeffs, pointer to the memory space to store returned values
 * \return     kIntelVvpCoreOk if instance and coeffs are non-NULL pointers
 *             kIntelVvpCoreNullPtrErr, if the quantized_coeffs is the NULL pointer
 *             kIntelVvpCoreInstanceErr if the instance is invalid or debug is not enabled
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized and with debug enabled
 */
int intel_vvp_csc_get_quantized_coeff_data(intel_vvp_csc_instance* instance, intel_vvp_quantized_coefficients* quantized_coeffs);

/**
 * \brief Set the current coefficients (and summands)
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \param[in]  coeffs, pointer to the memory space to store returned values
 * \param[in]  summand_rescale, shift applied to the summand value if applying an existing
 *             conversion table that was specified for a different number of bits than bits_per_sample_in.
 *             Typically 0 (no shift) or often 2/-2 if applying a 8-bit conversion table to 10-bit samples or vice-versa
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr, if the instance is invalid
 *             kIntelVvpCoreNullPtrErr, if coeffs is the NULL pointer
 *             kIntelVvpCscQuantizationErr if the quantization is not achievable
 *             (ie, negative values for unsigned coeffs or out-of-range values
 *             for the number of integer bits)
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized
 * \see intel_vvp_quantization.h
 */
int intel_vvp_csc_set_coeff_data(intel_vvp_csc_instance* instance, const intel_vvp_coefficients* coeffs, int summand_rescale);

/**
 * \brief Load the current coefficients (and summands) from the IP
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \param[out] coeffs, pointer to the memory space to place data
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is invalid or debug is not enabled
 *             kIntelVvpCoreNullPtrErr, if coeffs is the NULL pointer
 * \pre        instance is a valid intel_vvp_csc_instance that was successfully initialized and with debug enabled
 */
int intel_vvp_csc_get_coeff_data(intel_vvp_csc_instance* instance, intel_vvp_coefficients* coeffs);

/**
 * \brief Set the current output color space in full mode
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \param[in]  output_color_space, new output color space
 * \return     kIntelVvpCoreOk if instance is instance and color space are valid
 * \remarks    this call is used to set a color space value on output image info packet
 * \pre        instance is a valid intel_vvp_csc_instance parameterized in full mode
 * \see        eIntelVvpCscColorSpace for known color spaces
 */
int intel_vvp_csc_set_output_color_space(intel_vvp_csc_instance* instance, int8_t output_color_space);

/**
 * \brief Get the current output color space
 * 
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     output color space (kIntelVvpCsInvalid if !debug_enabled or lite_mode==true)
 * \pre        instance is a valid intel_vvp_csc_instance parameterized in full mode with debug enabled
 */
int8_t intel_vvp_csc_get_output_color_space(intel_vvp_csc_instance* instance);


/**
 * \brief commit all outstanding writes at next frame interval
 *  
 * \param[in]  instance, pointer to the initialized intel_vvp_csc_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_csc_instance parameterized in full mode
 */
int intel_vvp_csc_commit_writes(intel_vvp_csc_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_CSC_H__ */
