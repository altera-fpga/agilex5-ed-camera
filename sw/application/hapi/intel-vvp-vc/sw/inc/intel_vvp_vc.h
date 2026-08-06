/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_vc_instance and associated functions
 *
 * Driver for the Video & Vision Processing VC Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_vc_regs.h
 */
#ifndef __INTEL_VVP_VC_H__
#define __INTEL_VVP_VC_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_vc_regs.h"

#define INTEL_VVP_VC_PRODUCT_ID                           0x0178u              ///< VC product ID
#define INTEL_VVP_VC_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_VC_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_VC_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< VC register read function
#define INTEL_VVP_VC_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< VC register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpVcRegMapVersionErr = -100,
    kIntelVvpVcParameterErr = -101,
    kIntelVvpVcValueErr = -102,
    kIntelVvpVcPointerErr = -103,
    kIntelVvpVcCommitPendingErr = -104,
    kIntelVvpVcLutNotAvailableErr = -105,
    kIntelVvpVcLutInvalidNumBlocksErr = -106,
} eIntelVvpVcErrors;

typedef struct intel_vvp_vc_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    // Parameters
    bool     lite_mode;
    bool     debug_enabled;
    uint8_t  bps_in;
    uint8_t  bps_out;
    uint8_t  num_color_in;
    uint8_t  num_color_out;
    uint8_t  pip;
    uint32_t max_width;
    uint32_t max_height;
    uint8_t  h_num_blocks;
    uint8_t  v_num_blocks;
    bool     cfa_enable;
    uint16_t max_gain_mesh_points;
    bool     per_color_gain_enable;
    // Internal states
    uint32_t settings_reg;
} intel_vvp_vc_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_vc_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the VC product id (0x0178)
 *               kIntelVvpVcRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_vc_init(intel_vvp_vc_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_vc_instance
 * \return      the lite_mode field in the intel_vvp_vc_instance
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_get_lite_mode(intel_vvp_vc_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_vc_instance
 * \return      the debug_enabled field in the intel_vvp_vc_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_get_debug_enabled(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_bits_per_sample_in(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_bits_per_sample_out(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_num_color_planes_in(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_num_color_planes_out(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_pixels_in_parallel(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_max_width(intel_vvp_vc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of an
 *        image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_max_height(intel_vvp_vc_instance* instance);

/**
 * \brief Query the cfa_enable parameter of an ip instance
 *
 * \param[in]   instance, pointer to the intel_vvp_vc_instance
 * \return      the cfa_enable field in the intel_vvp_vc_instance
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_get_cfa_enable(intel_vvp_vc_instance* instance);

/**
 * \brief Get the max_gain_mesh_points parameter of an ip instance
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_vc_instance
 * \return      max_gain_mesh_points
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 * \remarks     This is the max_gain_mesh_points
 */
uint16_t intel_vvp_vc_get_max_gain_mesh_points(intel_vvp_vc_instance* instance);

/**
 * \brief Query the per_color_gain_enable parameter of an ip instance
 *
 * \param[in]   instance, pointer to the intel_vvp_vc_instance
 * \return      the per_color_gain_enable field in the intel_vvp_vc_instance
 * \pre         instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_get_per_color_gain_enable(intel_vvp_vc_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_is_running(intel_vvp_vc_instance* instance);

/**
 * \brief Get the commit request status of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_commit_is_pending(intel_vvp_vc_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_vc_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_status(intel_vvp_vc_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_vc_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_get_frame_stats(intel_vvp_vc_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_commit(intel_vvp_vc_instance* instance);

/**
 * \brief Reads the bypass bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     true if bypass mode is enabled, false if bypass mode is disabled
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
bool intel_vvp_vc_get_bypass(intel_vvp_vc_instance* instance);

/**
 * \brief Writes the bypass bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  bypass, bypass mode is enabled if true, bypass mode is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_bypass(intel_vvp_vc_instance* instance, bool bypass);

/**
 * \brief Get color filter array phase setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     Color filter array phase setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint8_t intel_vvp_vc_get_cfa_phase(intel_vvp_vc_instance* instance);

/**
 * \brief Set color filter array phase setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  cfa_phase, color filter array phase setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if the value is outside the valid range
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_cfa_phase(intel_vvp_vc_instance* instance, uint8_t cfa_phase);

/**
 * \brief Get block_pix_count setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     block_pix_count setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint16_t intel_vvp_vc_get_block_pix_count(intel_vvp_vc_instance* instance);

/**
 * \brief Set block_pix_count setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  block_pix_count, block_pix_count setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if block_pix_count is above the maximum allowed value
 *             kIntelVvpVcValueErr if block_pix_count is below the minimum required value (8 * pip)
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_block_pix_count(intel_vvp_vc_instance* instance, uint16_t block_pix_count);

/**
 * \brief Get block_line_count setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     block_line_count setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint16_t intel_vvp_vc_get_block_line_count(intel_vvp_vc_instance* instance);

/**
 * \brief Set block_line_count setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  block_line_count, block_line_count setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if block_line_count is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_block_line_count(intel_vvp_vc_instance* instance, uint16_t block_line_count);

/**
 * \brief Get h_num_blocks setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     h_num_blocks setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint16_t intel_vvp_vc_get_h_num_blocks(intel_vvp_vc_instance* instance);

/**
 * \brief Set h_num_blocks setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  h_num_blocks, h_num_blocks setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if h_num_blocks is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_h_num_blocks(intel_vvp_vc_instance* instance, uint16_t h_num_blocks);

/**
 * \brief Get v_num_blocks setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     v_num_blocks setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint16_t intel_vvp_vc_get_v_num_blocks(intel_vvp_vc_instance* instance);

/**
 * \brief Set v_num_blocks setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  v_num_blocks, v_num_blocks setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if v_num_blocks is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_v_num_blocks(intel_vvp_vc_instance* instance, uint16_t v_num_blocks);

/**
 * \brief Get h_ramp_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     h_ramp_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_h_ramp_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set h_ramp_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  h_ramp_frac, h_ramp_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if h_ramp_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_h_ramp_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_frac);

/**
 * \brief Get h_ramp_p1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     h_ramp_p1_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_h_ramp_p1_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set h_ramp_p1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  h_ramp_p1_frac, h_ramp_p1_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if h_ramp_p1_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_h_ramp_p1_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_p1_frac);

/**
 * \brief Get h_ramp_m1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     h_ramp_m1_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_h_ramp_m1_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set h_ramp_m1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  h_ramp_m1_frac, h_ramp_m1_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if h_ramp_m1_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_h_ramp_m1_frac(intel_vvp_vc_instance* instance, uint32_t h_ramp_m1_frac);

/**
 * \brief Get v_ramp_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     v_ramp_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_v_ramp_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set v_ramp_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  v_ramp_frac, v_ramp_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if v_ramp_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_v_ramp_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_frac);

/**
 * \brief Get v_ramp_p1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     v_ramp_p1_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_v_ramp_p1_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set v_ramp_p1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  v_ramp_p1_frac, v_ramp_p1_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if v_ramp_p1_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_v_ramp_p1_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_p1_frac);

/**
 * \brief Get v_ramp_m1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_vc_instance
 * \return     v_ramp_m1_frac setting
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
uint32_t intel_vvp_vc_get_v_ramp_m1_frac(intel_vvp_vc_instance* instance);

/**
 * \brief Set v_ramp_m1_frac setting of the VC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  v_ramp_m1_frac, v_ramp_m1_frac setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcValueErr if v_ramp_m1_frac is greater than the maximum value allowed
 *             kIntelVvpVcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 */
int intel_vvp_vc_set_v_ramp_m1_frac(intel_vvp_vc_instance* instance, uint32_t v_ramp_m1_frac);

/**
 * \brief CP LUT Update/Load
 *
 * You MUST use intel_vvp_vc_set_h_num_blocks() and
 * intel_vvp_vc_set_v_num_blocks() before calling this procedure to inform the
 * core of the number of grid points required.
 *
 * This procedure will upload exactly `(h_num_blocks+1) * (v_num_blocks+1)` *
 * entries from `cp_lut_values` to the corresponding CP LUT of the instance.
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  cp_lut_idx, the index of the CP LUT to write to, in the range [0, 3] inclusive.
 * \param[in]  cp_lut_values, a pointer to the table of gain values to write to the CP LUT.
 *                            This table must be atleast (h_num_blocks+1) * (v_num_blocks+1) wide.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcPointerErr if cp_lut_values pointer is NULL
 *             kIntelVvpVcParameterErr if cp_lut_idx is out of range
 *             kIntelVvpVcLutNotAvailableErr if the LUT is not available for this instance
 * \pre        the number of horizontal blocks has been set, using intel_vvp_vc_set_h_num_blocks()
 * \pre        the number of vertical blocks has been set, using intel_vvp_vc_set_v_num_blocks()
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 * \pre        cp_lut_values points to an array atleast (h_num_blocks+1) * (v_num_blocks+1) wide
 */
int intel_vvp_vc_update_cp_lut(intel_vvp_vc_instance* instance, uint8_t cp_lut_idx, const uint32_t* cp_lut_values);

/**
 * \brief Step LUT Update/Load
 *
 * You MUST use intel_vvp_vc_set_h_num_blocks() and
 * intel_vvp_vc_set_v_num_blocks() before calling this procedure to inform the
 * core of the number of grid points required.
 *
 * This procedure will upload exactly `(h_num_blocks+1) * (v_num_blocks+1)`
 * entries from `step_lut_values` to the Step LUT of the instance.
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_vc_instance
 * \param[in]  step_lut_values, a pointer to the table of gain values to write to the Step LUT.
 *                              This table must be atleast (h_num_blocks+1) * (v_num_blocks+1) wide.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpVcPointerErr if step_lut_values is NULL
 *             kIntelVvpVcLutInvalidNumBlocksErr if the number of horizontal or vertical blocks is zero
 * \pre        the number of horizontal blocks has been set, using intel_vvp_vc_set_h_num_blocks()
 * \pre        the number of vertical blocks has been set, using intel_vvp_vc_set_v_num_blocks()
 * \pre        instance is a valid intel_vvp_vc_instance that was successfully initialized
 * \pre        step_lut_values points to an array atleast (h_num_blocks+1) * (v_num_blocks+1) wide
 */
int intel_vvp_vc_update_step_lut(intel_vvp_vc_instance* instance, const uint32_t* step_lut_values);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_VC_H__ */
