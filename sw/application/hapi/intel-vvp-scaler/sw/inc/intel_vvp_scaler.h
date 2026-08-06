/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
/**
 * \brief Definition of the intel_vvp_scaler_instance and associated functions
 *
 * Driver for the Video & Vision Processing Scaler Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_scaler_regs.h
 */

#ifndef __INTEL_VVP_SCALER_H__
#define __INTEL_VVP_SCALER_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"
#include "intel_vvp_scaler_regs.h"

#define INTEL_VVP_SCALER_PRODUCT_ID                           0x0234u              ///< Scaler product ID
#define INTEL_VVP_SCALER_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_SCALER_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version
#define INTEL_VVP_SCALER_MAX_NUM_TAPS                        64
#define INTEL_VVP_SCALER_MAX_COEFF_WIDTH                     31

#define INTEL_VVP_SCALER_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Scaler register read function
#define INTEL_VVP_SCALER_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Scaler register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


typedef enum {
    kIntelVvpScalerRegMapVersionErr = -100,
    kIntelVvpScalerParameterErr     = -101,
    kIntelVvpScalerQuantizationErr  = -102
} eIntelVvpScalerErrors;

typedef enum {
    kIntelVvpScalerNearestNeighborScaling = INTEL_VVP_SCALER_NEAREST_NEIGHBOR_SCALING,
    kIntelVvpScalerBilinearScaling        = INTEL_VVP_SCALER_BILINEAR_SCALING,
    kIntelVvpScalerPolyphaseScaling       = INTEL_VVP_SCALER_POLYPHASE_SCALING,
    kIntelVvpScalerInvalidScalingAlgo     = -1
} eIntelVvpScalerAlgorithms;


/**
 * \brief The definition of an intel_vvp_scaler_instance.
 *
 * This is the definition for an intel_vvp_scaler_instance. It starts with an intel_vvp_core_instance
 * and can be used as such in generic functions
 */
typedef struct intel_vvp_scaler_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance

    bool      debug_enabled;
    bool      lite_mode;
    uint8_t   pixels_in_parallel;
    uint32_t  max_input_width;
    uint32_t  max_output_width;
    eIntelVvpScalerAlgorithms scaling_algo;

    bool    coeffs_runtime_load;
    bool    coeffs_mem_init;

    bool v_scaling_enabled;
    bool v_partial_scaling_enabled;
    uint8_t v_num_taps;
    uint16_t v_num_phases;
    uint8_t v_num_banks;
    bool v_coeffs_signed;
    uint8_t v_coeffs_int_bits; // sign bit excluded
    uint8_t v_coeffs_frac_bits;

    bool h_scaling_enabled;
    bool h_partial_scaling_enabled;
    uint8_t h_num_taps;
    uint16_t h_num_phases;
    uint8_t h_num_banks;
    bool h_coeffs_signed;
    uint8_t h_coeffs_int_bits; // sign bit excluded
    uint8_t h_coeffs_frac_bits;
} intel_vvp_scaler_instance;

/**
 * \brief Initialise a scaler instance
 *
 * Initialization function for a VVP Scaler instance.
 * Attempts to initialize the fields of the scaler and its base core
 *
 * \param[in]    instance, pointer to the intel_vvp_scaler_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the scaler product id (0x0234)
 *               kIntelVvpScalerRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be fully initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_scaler_init(intel_vvp_scaler_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the lite_mode field in the intel_vvp_scaler_instance
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_get_lite_mode(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the debug_enabled parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the debug_enabled field in the intel_vvp_scaler_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_get_debug_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the pixels_in_parallel parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the pixels_in_parallel field in the intel_vvp_scaler_instance
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
uint8_t intel_vvp_scaler_get_pixels_in_parallel(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the max_input_width parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the max_input_width field in the intel_vvp_scaler_instance
 * \remarks     this value may be larger than the one used to configure the IP
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
uint32_t intel_vvp_scaler_get_max_input_width(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the max_output_width parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the max_output_width field in the intel_vvp_scaler_instance
 * \remarks     this value may be larger than the one used to configure the IP
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
uint32_t intel_vvp_scaler_get_max_output_width(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the scaling_algo parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the scaling_algo field in the intel_vvp_scaler_instance
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
eIntelVvpScalerAlgorithms intel_vvp_scaler_get_scaling_algorithm(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the coeffs_runtime_load parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether coefficients can be loaded and updated at run-time
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_coeffs_loading_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the coeffs_mem_init parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      the coeffs_mem_init field in the intel_vvp_scaler_instance
 * \remarks     this indicates whether the IP starts with a set of predetermined coefficients
 *              after reset (not applicable for nearest neighbor and bilinear scaling)
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_are_coeffs_preinitialized(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_scaling_enabled parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether vertical resizing is enabled
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_vertical_scaling_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_partial_scaling_enabled parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether partial/split vertical resizing is enabled
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_partial_vertical_scaling_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_num_taps parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of taps used for vertical resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical polyphase scaling,
 *              default values are returned for NN/bilinear scaling
 */
uint8_t intel_vvp_scaler_get_num_vertical_taps(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_num_phases parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of phases used for vertical resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical polyphase scaling,
 *              default values are returned for NN/bilinear scaling
 */
uint16_t intel_vvp_scaler_get_num_vertical_phases(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query thev_num_banks parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of coefficients banks reserved for vertical resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical polyphase scaling
 *              with run-time loading of coefficients
 */
uint8_t intel_vvp_scaler_get_num_vertical_banks(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_coeffs_signed parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether vertical coefficients have a sign bit
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical polyphase scaling
 */
bool intel_vvp_scaler_are_vertical_coeffs_signed(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_coeffs_int_bits parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of integer bits used to quantize vertical coefficients
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical polyphase scaling
 */
uint8_t intel_vvp_scaler_get_vertical_coeffs_int_bits(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the v_coeffs_frac_bits parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of fractional bits used to quantize vertical coefficients
 * \pre         instance is a valid intel_vvp_scaler_instance configured for vertical
 *              bilinear/polyphase scaling, a default value is used for bilinear scaling
 */
uint8_t intel_vvp_scaler_get_vertical_coeffs_frac_bits(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_scaling_enabled parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether horizontal resizing is enabled
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_horizontal_scaling_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_partial_scaling_enabled parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether partial/split horizontal resizing is enabled
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_partial_horizontal_scaling_enabled(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_num_taps parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of taps used for horizontal resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal scaling,
 *              default values are returned for NN/bilinear scaling
 */
uint8_t intel_vvp_scaler_get_num_horizontal_taps(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_num_phases parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of phases used for horizontal resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal scaling,
 *              default values are returned for NN/bilinear scaling
 */
uint16_t intel_vvp_scaler_get_num_horizontal_phases(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_num_banks parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of coefficients banks reserved for horizontal resizing
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal polyphase
 *              scaling with run-time loading of coefficients
 */
uint8_t intel_vvp_scaler_get_num_horizontal_banks(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_coeffs_signed parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      whether horizontal coefficients have a sign bit
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal polyphase scaling
 */
bool intel_vvp_scaler_are_horizontal_coeffs_signed(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_coeffs_int_bits parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of integer bits used to quantize horizontal coefficients
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal polyphase scaling
 */
uint8_t intel_vvp_scaler_get_horizontal_coeffs_int_bits(intel_vvp_scaler_instance* instance);

/**
 * \brief  Query the h_coeffs_frac_bits parameter of a scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      number of fractional bits used to quantize horizontal coefficients
 * \pre         instance is a valid intel_vvp_scaler_instance configured for horizontal
 *              bilinear/polyphase scaling, a default value is used for bilinear scaling
 */
uint8_t intel_vvp_scaler_get_horizontal_coeffs_frac_bits(intel_vvp_scaler_instance* instance);


/**
 * \brief get running status of the scaler instance
 *
 * \param[in]   instance, pointer to the intel_vvp_scaler_instance
 * \return      true if processing image data, false between fields
 * \pre         instance is a valid intel_vvp_scaler_instance and successfully initialized
 */
bool intel_vvp_scaler_is_running(intel_vvp_scaler_instance *instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 *
 * \param[in]    instance, pointer to the intel_vvp_scaler_instance
 * \return       true if there are outstanding writes
 * \pre          the IP is parameterized in full mode
 */
bool intel_vvp_scaler_get_commit_status(intel_vvp_scaler_instance* instance);

/**
 * \brief Read the status register
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return the value returned from a read to the status register
 */
uint8_t intel_vvp_scaler_get_status(intel_vvp_scaler_instance *instance);

/**
 * \brief Select the bank of horizontal coefficients to use
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  bank, the bank, in the range [0..get_num_horizontal_banks()-1]
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support horizontal polyphase scaling
 *             kIntelVvpScalerParameterErr, if the requested bank doesn't exist
 */
int intel_vvp_scaler_set_h_bank(intel_vvp_scaler_instance *instance, uint8_t bank);

/**
 * \brief Select the bank of vertical coefficients to use
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  bank, the bank
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support vertical polyphase scaling
 *             kIntelVvpScalerParameterErr, if the requested bank doesn't exist
 */
int intel_vvp_scaler_set_v_bank(intel_vvp_scaler_instance *instance, uint8_t bank);

/**
 * \brief Select the output width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  output_width, the selected output width
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support horizontal rescaling
 *             kIntelVvpScalerParameterErr, if the output width is invalid or greater than what the compile-time parameterization allows
 * \remarks    When horizontal scaling is disabled, the output width is assumed to be equal to the input width and
 *             need not be set
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 * \see        intel_vvp_core_set_img_info_width to set the input width in lite mode
 */
int intel_vvp_scaler_set_output_width(intel_vvp_scaler_instance *instance, uint32_t output_width);

/**
 * \brief Select the output height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  output_height, the selected output height
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support vertical rescaling
 *             kIntelVvpScalerParameterErr, if the output height is invalid
 * \remarks    When vertical scaling is disabled, the output height is assumed to be equal to the input height and
 *             doesn't have to be set
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 * \see        intel_vvp_core_set_img_info_height to set the input height in lite mode
 */
int intel_vvp_scaler_set_output_height(intel_vvp_scaler_instance *instance, uint32_t output_height);


/**
 * \brief Sets the total input width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  total_input_width, the selected total input width
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial horizontal rescaling
 *             kIntelVvpScalerParameterErr, if the width is invalid
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 */
int intel_vvp_scaler_set_total_input_width(intel_vvp_scaler_instance *instance, uint32_t total_input_width);

/**
 * \brief Sets the total output width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  total_output_width, the selected total output width
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial horizontal rescaling
 *             kIntelVvpScalerParameterErr, if the width is invalid
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 */
int intel_vvp_scaler_set_total_output_width(intel_vvp_scaler_instance *instance, uint32_t total_output_width);

/**
 * \brief Sets the total input height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  total_input_height, the selected total input height
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial vertical rescaling
 *             kIntelVvpScalerParameterErr, if the height is invalid
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 */
int intel_vvp_scaler_set_total_input_height(intel_vvp_scaler_instance *instance, uint32_t total_input_height);

/**
 * \brief Sets the total output height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  total_output_height, the selected total output height
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial vertical rescaling
 *             kIntelVvpScalerParameterErr, if the height is invalid
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 */
int intel_vvp_scaler_set_total_output_height(intel_vvp_scaler_instance *instance, uint32_t total_output_height);

/**
 * \brief Sets the horizontal offset error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  h_pos_err, horizontal offset error
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial horizontal rescaling
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 * \remarks    This is computed as follows: (split_out_offset_x * total_in_width) % total_out_width
 *             beware that this parameter
 */
int intel_vvp_scaler_set_h_pos_error(intel_vvp_scaler_instance *instance, uint32_t h_pos_err);

/**
 * \brief Sets the vertical offset error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  v_pos_err, vertical offset error
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial vertical rescaling
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 * \remarks    This is computed as follows: (split_out_offset_y * total_in_height) % total_out_height
 */
int intel_vvp_scaler_set_v_pos_error(intel_vvp_scaler_instance *instance, uint32_t v_pos_err);


/**
 * \brief Sets the horizontal extended fill status
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  extended_fill, whether horizontal extended should be enabled
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial horizontal rescaling
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 * \remarks    This value should be set to true for split horizontal scaling when the split
 *             image is not the leftmost split image and extra pixels ((num_h_taps-1)/2) will be provided
 *             on the left for seamless joints. It has no effect for NN and bilinear scaling.
 */
int intel_vvp_scaler_set_h_extended_fill(intel_vvp_scaler_instance *instance, bool extended_fill);

/**
 * \brief Sets the vertical extended fill status
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  extended_fill, whether vertical extended fill should be enabled
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support partial vertical rescaling
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 * \remarks    This value should be set to true for split vertical scaling when the split
 *             image is not the topmost split image and extra lines ((num_v_taps-1)/2) will be provided on the top
 *             for seamless joints. It has no effect for NN and bilinear scaling.
 */
int intel_vvp_scaler_set_v_extended_fill(intel_vvp_scaler_instance *instance, bool extended_fill);

/**
 * \brief Sets the horizontal phase offset and error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  phase_offset, horizontal phase offset
 * \param[in]  phase_error, horizontal phase offset error
 * \return     kIntelVvpCoreInstanceErr if partial horizontal scaling is disabled or not polyphase scaling,
 *             kIntelVvpCoreOk otherwise
 * \remarks    Consider using intel_vvp_scaler_set_split_h_scaling to completely set up the horizontal scaling
 *             when partial/split horizontal scaling is enabled.
 * \remarks    This is computed as follows:
 *                           phase_offset = (split_out_offset_x * total_in_width * num_h_phases) / total_out_width
 *                           phase_error  = (split_out_offset_x * total_in_width * num_h_phases) % total_out_width
 */
int intel_vvp_scaler_set_h_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t phase_offset, uint32_t phase_error);

/**
 * \brief Sets the vertical phase offset and error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[in]  phase_offset, vertical phase offset
 * \param[in]  phase_error, vertical phase offset error
 * \return     kIntelVvpCoreInstanceErr if partial vertical scaling is disabled or not polyphase scaling,
 *             kIntelVvpCoreOk otherwise
 * \remarks    Consider using intel_vvp_scaler_set_split_v_scaling to completely set up the vertical scaling
 *             when partial/split vertical scaling is enabled.
 * \remarks    This is computed as follows:
 *                           phase_offset = (split_out_offset_y * total_in_height * num_v_phases) / total_out_height
 *                           phase_error  = (split_out_offset_y * total_in_height * num_v_phases) % total_out_height
 */
int intel_vvp_scaler_set_v_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t phase_offset, uint32_t phase_error);


/**
 * \brief Fully setup the horizontal scaling when the instance is configured to do split horizontal scaling
 *
 * \param[in]  instance,                   an intel_vvp_scaler_instance
 * \param[in]  total_input_width,          the input width for the full image
 * \param[in]  total_output_width,         the output width for the full image
 * \param[in]  split_output_left_offset,   the requested left offset for the the first output pixel of the split
 * \param[in]  split_output_width,         the requested width of the current split (on the output side, after scaling)
 * \param[in]  extended_fill,              whether the input lines should contain extra left/right pixels to avoid artefacts at the edges of individual bands.
 *                                         If these are available, the number of extra pixels should be (num_h_taps-1)/2 pixels, rounded up to the next multiple
 *                                         of pixels in parallel, on the left and num_h_taps/2 pixels on the right. This parameter has no effect for NN and bilinear
 *                                         scaling
 * \param[out] split_input_left_offset,    the required left offset for the first input pixel of the split, computed during the call. This
 *                                         will include the extra left pixels if needed for extended_fill. Optional, use NULL pointer if not needed.
 * \param[out] split_input_width,          the required width for the input split, computed during the call. This will include the extra left and right pixels
 *                                         if needed for extended_fill. Optional, use NULL pointer if not needed
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support horizontal rescaling
 *             kIntelVvpCoreInstanceErr, if the instance is not configured for partial/split horizontal scaling
 *             kIntelVvpScalerParameterErr, if one of the dimensions is zero
 *             kIntelVvpScalerParameterErr, if split_output_width is greater than what the compile-time parameterization allows
 *             kIntelVvpScalerParameterErr, if (split_output_left_offset + split_output_width) > total_output_width
 *             kIntelVvpScalerParameterErr, if the function computes a split_input_width value that is greater than the compile time max_input_width
 *                                          note that, for this specific error, the call will still program the scaler instance to attempt the
 *                                          requested split
 * \remarks    Split horizontal scaling is setup by specifying the full frame input and output widths and the location/width ot the requested
 *             output band. The function computes the offset and the width for the required input band on the input frame and this should be
 *             used to configure the system upstream and feed the scaler with the appropriate input band.
 * \remarks    In lite mode, intel_vvp_core_set_img_info_width should still be called to set the input width. The recommended call should be
 *             intel_vvp_core_set_img_info_width(instance, split_input_width) where split_input_width is the value returned by this function
 */
int intel_vvp_scaler_set_split_h_scaling(intel_vvp_scaler_instance *instance,
                                         uint32_t total_input_width, uint32_t total_output_width,
                                         uint32_t split_output_width, uint32_t split_output_left_offset, bool extended_fill,
                                         uint32_t *split_input_width, uint32_t *split_input_left_offset);


/**
 * \brief Fully setup the vertical scaling when the instance is configured to do split horizontal scaling
 *
 * \param[in]  instance,                an intel_vvp_scaler_instance
 * \param[in]  total_input_height,      the input height for the full image
 * \param[in]  total_output_height,     the output height for the full image
 * \param[in]  split_output_top_offset, the requested top offset for the the first output line of the split
 * \param[in]  split_output_height,     the requested height of the current split (on the output side, after scaling)
 * \param[in]  extended_fill,           whether the input fields should contain extra top/bottom lines to avoid artefacts at the edges of
 *                                      individual bands. If these are available, the number of extra lines should be (num_v_taps-1)/2 lines
 *                                      at the top and num_v_taps/2 lines at the bottom. This parameter has no effect for NN and bilinear
 *                                      scaling
 * \param[out] split_input_top_offset,  the required top offset for the first input line of the split, computed during the call. This
 *                                      will include the top lines needed for extended_fill. Optional, use NULL pointer if not needed
 * \param[out] split_input_height,      the required height for the input split, computed during the call. This will include the bottom
 *                                      lines needed for extended_fill. Optional, use NULL pointer if not needed
 * \return     kIntelVvpCoreOk if no error is detected
 *             kIntelVvpCoreInstanceErr, if the instance is invalid or does not support vertical rescaling
 *             kIntelVvpCoreInstanceErr, if the instance is not configured for partial/split vertical scaling
 *             kIntelVvpScalerParameterErr, if one of the dimensions is zero
 *             kIntelVvpScalerParameterErr, if (split_output_top_offset + split_output_height) > total_output_height
 * \remarks    Split vertical scaling is setup by specifying the full frame input and output heights and the location/height ot the requested
 *             output band. The function computes the offset and the height for the required input band on the input frame and this should be
 *             used to configure the system upstream and feed the scaler with the appropriate input band.
 * \remarks    In lite mode, intel_vvp_core_set_img_info_height should still be called to set the input height. The recommended call should be
 *             intel_vvp_core_set_img_info_height(instance, split_input_height) where split_input_height is the value returned by this function
 */
int intel_vvp_scaler_set_split_v_scaling(intel_vvp_scaler_instance *instance,
                                         uint32_t total_input_height, uint32_t total_output_height,
                                         uint32_t split_output_height, uint32_t split_output_top_offset, bool extended_fill,
                                         uint32_t *split_input_height, uint32_t *split_input_top_offset);

/**
 * \brief Retrieve the bank of horizontal coefficients in use
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the h_bank register
 * \remarks    0 is returned if the register doesn't exist (horizontal scaling is disabled or not polyphase scaling)
 *             or cannot be read back (debug is disabled)
 */
uint8_t intel_vvp_scaler_get_h_bank(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the bank of vertical coefficients in use
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the v_bank register
 * \remarks    0 is returned if the register doesn't exist (vertical scaling is disabled or not polyphase scaling)
 *             or cannot be read back (debug is disabled)
 */
uint8_t intel_vvp_scaler_get_v_bank(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the output width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the output width register
 * \remarks    0 is returned if the register doesn't exist (horizontal scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_output_width(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the output height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the output height register
 * \remarks    0 is returned if the register doesn't exist (vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_output_height(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the total input width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the total input width register
 * \remarks    0 is returned if the register doesn't exist (partial vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_total_input_width(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the total output width
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the total output width register
 * \remarks    0 is returned if the register doesn't exist (partial horizontal scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_total_output_width(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the total input height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the total input height register
 * \remarks    0 is returned if the register doesn't exist (partial vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_total_input_height(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the total output height
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the total output height register
 * \remarks    0 is returned if the register doesn't exist (partial vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 */
uint32_t intel_vvp_scaler_get_total_output_height(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the horizontal offset error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the horizontal position offset error register
 * \remarks    0 is returned if the register doesn't exist (partial horizontal scaling is disabled)
 *             or cannot be read back (debug is disabled)
 * \remarks    This is computed as follows: (split_out_offset_x * total_in_width) % total_out_width
 */
uint32_t intel_vvp_scaler_get_h_pos_error(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the vertical offset error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the vertical position offset error register
 * \remarks    0 is returned if the register doesn't exist (partial vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 * \remarks    This is computed as follows: (split_out_offset_y * total_in_height) % total_out_height
 */
uint32_t intel_vvp_scaler_get_v_pos_error(intel_vvp_scaler_instance *instance);


/**
 * \brief Retrieve the horizontal extended fill status
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the horizontal extended fill register
 * \remarks    false is returned if the register doesn't exist (partial horizontal scaling is disabled)
 *             or cannot be read back (debug is disabled)
 * \remarks    This value should have been set to true for split horizontal scaling when the split
 *             image is not the leftmost split image and extra pixels are provided on the left for
 *             seamless joints
 */
bool intel_vvp_scaler_get_h_extended_fill(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the vertical extended fill status
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \return     The last value written in the vertical position offset error register
 * \remarks    false is returned if the register doesn't exist (partial vertical scaling is disabled)
 *             or cannot be read back (debug is disabled)
 * \remarks    This value should have been set to true for split vertical scaling when the split
 *             image is not the topmost split image and extra lines are provided on the top for
 *             seamless joints
 */
bool intel_vvp_scaler_get_v_extended_fill(intel_vvp_scaler_instance *instance);

/**
 * \brief Retrieve the horizontal phase offset and error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[out] phase_offset, the value read from the horizontal phase offset register
 * \param[out] phase_error, the value read from the horizontal phase offset error register
 * \return     kIntelVvpCoreInstanceErr is returned if the register doesn't exist (partial
 *             horizontal scaling is disabled or not polyphase scaling) or cannot be read back
 *             (debug is disabled). kIntelVvpCoreNullPtrErr is phase_offset or phase_error
 *             are NULL. kIntelVvpCoreOk otherwise
 * \remarks    This is computed as follows:
 *                           phase_offset = (split_out_offset_x * total_in_width * num_h_phases) / total_out_width
 *                           phase_error  = (split_out_offset_x * total_in_width * num_h_phases) % total_out_width
 */
int intel_vvp_scaler_get_h_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t *phase_offset, uint32_t *phase_error);

/**
 * \brief Retrieve the vertical phase offset and error
 *
 * \param[in]  instance, an intel_vvp_scaler_instance
 * \param[out] phase_offset, the value read from the horizontal phase offset register
 * \param[out] phase_error, the value read from the horizontal phase offset error register
 * \return     kIntelVvpCoreInstanceErr is returned if the register doesn't exist (partial
 *             vertical scaling is disabled or not polyphase scaling) or cannot be read back
 *             (debug is disabled). kIntelVvpCoreNullPtrErr is phase_offset or phase_error
 *             are NULL. kIntelVvpCoreOk otherwise
 * \remarks    This is computed as follows:
 *                           phase_offset = (split_out_offset_y * total_in_height * num_v_phases) / total_out_height
 *                           phase_error  = (split_out_offset_y * total_in_height * num_v_phases) % total_out_height
 */
int intel_vvp_scaler_get_v_phase_offset_error(intel_vvp_scaler_instance *instance, uint32_t *phase_offset, uint32_t *phase_error);

/**
 * \brief       Write and commit a set of quantized coefficients
 *
 * \param[in]       instance, pointer to the initialized intel_vvp_csc_instance
 * \param[in]       h_coeffs, true if uploading horizontal coefficients, false for vertical coefficients
 * \param[in]       quantized_coeffs, pointer to the memory space to read the coefficient
 * \param[in]       num_coeffs, the number of valid coefficients in the quantized_coeffs array (the call will fail if this does not match the expected number of taps)
 * \param[in]       bank, the bank where the coefficients should be loaded
 * \param[in]       phase, the phase for which the coefficients should be applied
 * \return          kIntelVvpCoreOk if no error is detected
 *                  kIntelVvpCoreInstanceErr, if the instance is invalid or does not support run-time loading of vertical/horizontal coefficients (whichever is relevant)
 *                  kIntelVvpCoreNullPtrErr, if quantized_coeffs is NULL
 *                  kIntelVvpScalerParameterErr, if bank/phase are incompatible with the given instance or num_coeffs is not equal to the number of
 *                                               coefficients/taps needed
 *                  kIntelVvpScalerQuantizationErr, if provided coefficients are out of range
 * \pre             the memory allocated for quantized_coeffs was at least num_coeffs int32_t values
 * \pre             num_coeffs == num_h_taps or num_coeffs == num_vtaps, whichever of the two is relevelant according to the boolean h_coeffs parameter
 */
int intel_vvp_scaler_set_quantized_coeffs(intel_vvp_scaler_instance* instance, bool h_coeffs, const int32_t* quantized_coeffs, uint8_t num_coeffs, uint8_t bank, uint8_t phase);

/**
 * \brief       Quantize, write and commit a set of coefficients
 *
 * \param[in]       instance, pointer to the initialized intel_vvp_csc_instance
 * \param[in]       h_coeffs, true if loading horizontal coefficients, false for vertical coefficients
 * \param[in]       coeffs, pointer to the memory space to read coefficients
 * \param[in]       num_coeffs, the number of valid coefficients in the quantized_coeffs array (the call will fail if this does not match the expected number of taps)
 * \param[in]       bank, the bank where the coefficients should be loaded
 * \param[in]       phase, the phase for which the coefficients should be applied
 * \return          kIntelVvpCoreOk if no error is detected
 *                  kIntelVvpCoreInstanceErr, if the instance is invalid or does not support run-time loading of vertical/horizontal coefficients (whichever is relevant)
 *                  kIntelVvpCoreNullPtrErr, if coeffs is NULL
 *                  kIntelVvpScalerParameterErr, if bank/phase are incompatible with the given instance or num_coeffs is not equal to the number of
 *                                               coefficients/taps needed
 *                  kIntelVvpScalerQuantizationErr if the quantization is not achievable (ie, negative values for unsigned coeffs or out-of-range
 *                  values for the number of integer bits)
 * \pre             the memory allocated for coeffs was at least num_coeffs float values
 * \pre             num_coeffs == num_h_taps or num_coeffs == num_vtaps, whichever of the two is relevelant according to the boolean h_coeffs parameter
 * \remarks         Coefficients are quantized using intel_vvp_quantize_error_propagate to try and minimize the overall quantization error at the
 *                  expense of the quantization error for each individual coefficient. Coefficients are quantized from last to first when
 *                  phase>num_phases/2 to preserve symmetry when quantizing phase x and phase num_phases-1-x
 * \see intel_vvp_quantizer.h
 */
int intel_vvp_scaler_set_coeffs(intel_vvp_scaler_instance* instance, bool h_coeffs, const float* coeffs, uint8_t num_coeffs, uint8_t bank, uint8_t phase);


/**
 * \brief commit all outstanding writes at next frame interval
 *
 * \param[in]    instance, pointer to the initialized intel_vvp_scaler_instance
 * \return       kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre          the IP is parameterized in full mode (commit is automatic in lite mode)
 */
int intel_vvp_scaler_commit_writes(intel_vvp_scaler_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_SCALER_H__
