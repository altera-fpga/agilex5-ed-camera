/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_anr_instance and associated functions
 *
 * Driver for the Video & Vision Processing ANR Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_anr_regs.h
 */
#ifndef __INTEL_VVP_ANR_H__
#define __INTEL_VVP_ANR_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_anr_regs.h"

#define INTEL_VVP_ANR_PRODUCT_ID                           0x0176u              ///< ANR product ID
#define INTEL_VVP_ANR_MIN_SUPPORTED_REGMAP_VERSION         2                    ///< Minimum supported register map version
#define INTEL_VVP_ANR_MAX_SUPPORTED_REGMAP_VERSION         2                    ///< Maximum supported register map version

#define INTEL_VVP_ANR_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< ANR register read function
#define INTEL_VVP_ANR_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< ANR register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpAnrRegMapVersionErr = -100,
    kIntelVvpAnrParameterErr = -101,
    kIntelVvpAnrValueErr = -102,
    kIntelVvpAnrPointerErr = -103,
    kIntelVvpAnrCommitPendingErr = -104,
    kIntelVvpAnrOutOfBoundsErr = -105,
} eIntelVvpAnrErrors;

typedef struct intel_vvp_anr_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    // Parameters
    bool     lite_mode;
    bool     debug_enabled;
    uint8_t  bps_in;
    uint8_t  bps_out;
    uint8_t  num_color_planes;
    bool     cfa_enabled;
    uint8_t  pip;
    uint32_t max_width;
    uint8_t  num_h_taps;
    uint8_t  num_v_taps;
    // Internal states
    uint32_t settings_reg;
    uint16_t spatial_lut_depth;
} intel_vvp_anr_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_anr_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the ANR product id (0x0176)
 *               kIntelVvpAnrRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_anr_init(intel_vvp_anr_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_anr_instance
 * \return      the lite_mode field in the intel_vvp_anr_instance
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_get_lite_mode(intel_vvp_anr_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_anr_instance
 * \return      the debug_enabled field in the intel_vvp_anr_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_get_debug_enabled(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint8_t intel_vvp_anr_get_bits_per_sample_in(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint8_t intel_vvp_anr_get_bits_per_sample_out(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint8_t intel_vvp_anr_get_num_color_planes(intel_vvp_anr_instance* instance);

/**
 * \brief Returns whether the core is set up for bayer pattern denoising
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      cfa enable state
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_get_cfa_enabled(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint8_t intel_vvp_anr_get_pixels_in_parallel(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint32_t intel_vvp_anr_get_max_width(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the number of horizontal taps
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      number of horizontal taps
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 * \remarks     This is the full width of the ANR kernel before folding
 */
uint8_t intel_vvp_anr_get_num_h_taps(intel_vvp_anr_instance* instance);

/**
 * \brief Returns the number of vertical taps
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_anr_instance
 * \return      number of vertical taps
 * \pre         instance is a valid intel_vvp_anr_instance that was successfully initialized
 * \remarks     This is the full height of the ANR kernel before folding
 */
uint8_t intel_vvp_anr_get_num_v_taps(intel_vvp_anr_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_anr_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_is_running(intel_vvp_anr_instance* instance);

/**
 * \brief Get the commit request status of the ANR instance
 *
 * \param[in]  instance, pointer to the intel_vvp_anr_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_commit_is_pending(intel_vvp_anr_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_anr_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
uint32_t intel_vvp_anr_get_status(intel_vvp_anr_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_anr_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_get_frame_stats(intel_vvp_anr_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_anr_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_commit(intel_vvp_anr_instance* instance);

/**
 * \brief Reads the bypass bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_anr_instance
 * \return     true if bypass mode is enabled, false if bypass mode is disabled
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
bool intel_vvp_anr_get_bypass(intel_vvp_anr_instance* instance);

/**
 * \brief Writes the bypass bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \param[in]  bypass, bypass mode is enabled if true, bypass mode is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_set_bypass(intel_vvp_anr_instance* instance, bool bypass);

/**
 * \brief Write a single value to the intensity lut memory
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \param[in]  data, the value to be written to the intensity lut memory
 * \param[in]  addr, relative address of the intensity lut memory to write to
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrPointerErr if data_array is a null pointer
 *             kIntelVvpAnrOutOfBoundsErr if address is out of range
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_write_intensity_lut_entry(intel_vvp_anr_instance* instance,
                                                         uint32_t data, uint32_t addr);

/**
 * \brief Write the whole intensity lut memory from an array
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \param[in]  data_array, pointer of an array with a minimum length of number of data registers.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrPointerErr if data_array is a null pointer
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_write_intensity_lut_array(intel_vvp_anr_instance* instance, const uint32_t* data_array);

/**
 * \brief Write a single value to the spatial lut memory
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \param[in]  data, the value to be written to the spatial lut memory
 * \param[in]  addr, relative address of the spatial lut memory to write to
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrPointerErr if data_array is a null pointer
 *             kIntelVvpAnrOutOfBoundsErr if address is out of range
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_write_spatial_lut_entry(intel_vvp_anr_instance* instance,
                                                         uint32_t data, uint32_t addr);

/**
 * \brief Write the whole spatial lut memory from an array
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_anr_instance
 * \param[in]  data_array, pointer of an array with a minimum length of number of data registers.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpAnrPointerErr if data_array is a null pointer
 * \pre        instance is a valid intel_vvp_anr_instance that was successfully initialized
 */
int intel_vvp_anr_write_spatial_lut_array(intel_vvp_anr_instance* instance, const uint32_t* data_array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_ANR_H__ */
