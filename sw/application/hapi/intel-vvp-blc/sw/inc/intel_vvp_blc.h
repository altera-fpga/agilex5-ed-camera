/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_blc_instance and associated functions
 *
 * Driver for the Video & Vision Processing BLC Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_blc_regs.h
 */
#ifndef __INTEL_VVP_BLC_H__
#define __INTEL_VVP_BLC_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_blc_regs.h"

#define INTEL_VVP_BLC_PRODUCT_ID                           0x0177u              ///< BLC product ID
#define INTEL_VVP_BLC_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_BLC_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_BLC_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< BLC register read function
#define INTEL_VVP_BLC_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< BLC register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpBlcRegMapVersionErr = -100,
    kIntelVvpBlcParameterErr = -101,
    kIntelVvpBlcValueErr = -102,
    kIntelVvpBlcPointerErr = -103,
    kIntelVvpBlcCommitPendingErr = -104,
} eIntelVvpBlcErrors;

typedef struct intel_vvp_blc_instance_s
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
    bool     reflect_around_zero;
    // Internal states
    uint32_t settings_reg;
} intel_vvp_blc_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_blc_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the BLC product id (0x0177)
 *               kIntelVvpBlcRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_blc_init(intel_vvp_blc_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_blc_instance
 * \return      the lite_mode field in the intel_vvp_blc_instance
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_get_lite_mode(intel_vvp_blc_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_blc_instance
 * \return      the debug_enabled field in the intel_vvp_blc_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_get_debug_enabled(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_bits_per_sample_in(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_bits_per_sample_out(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_num_color_planes_in(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_num_color_planes_out(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_pixels_in_parallel(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_max_width(intel_vvp_blc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of an
 *        image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_blc_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_max_height(intel_vvp_blc_instance* instance);

/**
 * \brief Query the reflect_around_zero parameter of an ip instance
 *
 * \param[in]   instance, pointer to the intel_vvp_blc_instance
 * \return      the reflect_around_zero field in the intel_vvp_blc_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_get_reflect_around_zero(intel_vvp_blc_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_is_running(intel_vvp_blc_instance* instance);

/**
 * \brief Get the commit request status of the BLC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_commit_is_pending(intel_vvp_blc_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_blc_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_status(intel_vvp_blc_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_blc_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_get_frame_stats(intel_vvp_blc_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_commit(intel_vvp_blc_instance* instance);

/**
 * \brief Reads the bypass bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     true if bypass mode is enabled, false if bypass mode is disabled
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_get_bypass(intel_vvp_blc_instance* instance);

/**
 * \brief Writes the bypass bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  bypass, bypass mode is enabled if true, bypass mode is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_bypass(intel_vvp_blc_instance* instance, bool bypass);

/**
 * \brief Get color filter array phase setting of the BLC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array phase setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint8_t intel_vvp_blc_get_cfa_phase(intel_vvp_blc_instance* instance);

/**
 * \brief Set color filter array phase setting of the BLC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  cfa_phase, color filter array phase setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_phase(intel_vvp_blc_instance* instance, uint8_t cfa_phase);

/**
 * \brief Get clip zero enable setting flag of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     true if clip zero enable is enabled, false if clip zero enable is disabled
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
bool intel_vvp_blc_get_clip_zero_en(intel_vvp_blc_instance *instance);

/**
 * \brief Set clip zero enable setting flag of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  clip_zero_en, clip zero enable is enabled if true, clip zero enable is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_clip_zero_en(intel_vvp_blc_instance* instance, bool clip_zero_en);

/**
 * \brief Get color filter array 00 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 00 black pedestal setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_00_black_pedestal(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 00 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  pedestal, color filter array 00 black pedestal setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_00_black_pedestal(intel_vvp_blc_instance* instance, uint32_t pedestal);

/**
 * \brief Get color filter array 00 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 00 color scaler setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_00_color_scaler(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 00 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  scaler, color filter array 00 color scaler setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_00_color_scaler(intel_vvp_blc_instance* instance, uint32_t scaler);

/**
 * \brief Get color filter array 01 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 01 black pedestal setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_01_black_pedestal(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 01 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  pedestal, color filter array 01 black pedestal setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_01_black_pedestal(intel_vvp_blc_instance* instance, uint32_t pedestal);

/**
 * \brief Get color filter array 01 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 01 color scaler setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_01_color_scaler(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 01 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  scaler, color filter array 01 color scaler setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_01_color_scaler(intel_vvp_blc_instance* instance, uint32_t scaler);

/**
 * \brief Get color filter array 10 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 10 black pedestal setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_10_black_pedestal(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 10 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  pedestal, color filter array 10 black pedestal setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_10_black_pedestal(intel_vvp_blc_instance* instance, uint32_t pedestal);

/**
 * \brief Get color filter array 10 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 10 color scaler setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_10_color_scaler(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 10 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  scaler, color filter array 10 color scaler setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_10_color_scaler(intel_vvp_blc_instance* instance, uint32_t scaler);

/**
 * \brief Get color filter array 11 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 11 black pedestal setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_11_black_pedestal(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 11 black pedestal setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  pedestal, color filter array 11 black pedestal setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_11_black_pedestal(intel_vvp_blc_instance* instance, uint32_t pedestal);

/**
 * \brief Get color filter array 11 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the intel_vvp_blc_instance
 * \return     Color filter array 11 color scaler setting
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
uint32_t intel_vvp_blc_get_cfa_11_color_scaler(intel_vvp_blc_instance *instance);

/**
 * \brief Set color filter array 11 color scaler setting of the blc instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_blc_instance
 * \param[in]  scaler, color filter array 11 color scaler setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlcValueErr if the value is outside the valid range
 *             kIntelVvpBlcCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_blc_instance that was successfully initialized
 */
int intel_vvp_blc_set_cfa_11_color_scaler(intel_vvp_blc_instance* instance, uint32_t scaler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_BLC_H__ */
