/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_dpc_instance and associated functions
 *
 * Driver for the Video & Vision Processing DPC Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_dpc_regs.h
 */
#ifndef __INTEL_VVP_DPC_H__
#define __INTEL_VVP_DPC_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_dpc_regs.h"

#define INTEL_VVP_DPC_PRODUCT_ID                           0x0175u              ///< DPC product ID
#define INTEL_VVP_DPC_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_DPC_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_DPC_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< DPC register read function
#define INTEL_VVP_DPC_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< DPC register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpDpcRegMapVersionErr = -100,
    kIntelVvpDpcParameterErr = -101,
    kIntelVvpDpcValueErr = -102,
    kIntelVvpDpcPointerErr = -103,
    kIntelVvpDpcFreezePendingErr = -105,
    kIntelVvpDpcOutOfBoundsErr = -106,
} eIntelVvpDpcErrors;

typedef struct intel_vvp_dpc_instance_s
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
    // Internal states
    uint32_t settings_reg;
} intel_vvp_dpc_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_dpc_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the DPC product id (0x0175)
 *               kIntelVvpDpcRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_dpc_init(intel_vvp_dpc_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_dpc_instance
 * \return      the lite_mode field in the intel_vvp_dpc_instance
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_get_lite_mode(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_dpc_instance
 * \return      the debug_enabled field in the intel_vvp_dpc_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_get_debug_enabled(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_bits_per_sample_in(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_bits_per_sample_out(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_num_color_planes_in(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_num_color_planes_out(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_pixels_in_parallel(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint32_t intel_vvp_dpc_get_max_width(intel_vvp_dpc_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_dpc_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint32_t intel_vvp_dpc_get_max_height(intel_vvp_dpc_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_is_running(intel_vvp_dpc_instance* instance);

/**
 * \brief Get the statistics are frozen status of the DPC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     true if statistics are frozen, false otherwise
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_stats_are_frozen(intel_vvp_dpc_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_dpc_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint32_t intel_vvp_dpc_get_status(intel_vvp_dpc_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_dpc_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpDpcPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_get_frame_stats(intel_vvp_dpc_instance* instance, uint32_t* stats_out);

/**
 * \brief Read the pixel low clip counter register
 *
 * \param[in]  instance, an intel_vvp_dpc_instance
 * \param[in]  stats_out, pointer of a variable used for returning the low clip counter statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpDpcPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 *             and configured with debug enabled.
 */
int intel_vvp_dpc_get_pix_lo_clip_cnt(intel_vvp_dpc_instance* instance, uint32_t* stats_out);

/**
 * \brief Read the pixel high clip counter register
 *
 * \param[in]  instance, an intel_vvp_dpc_instance
 * \param[in]  stats_out, pointer of a variable used for returning the high clip counter statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpDpcPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 *             and configured with debug enabled.
 */
int intel_vvp_dpc_get_pix_hi_clip_cnt(intel_vvp_dpc_instance* instance, uint32_t* stats_out);

/**
 * \brief Reads the bypass bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     true if bypass mode is enabled, false if bypass mode is disabled
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_get_bypass(intel_vvp_dpc_instance* instance);

/**
 * \brief Writes the bypass bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  bypass, bypass mode is enabled if true, bypass mode is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_bypass(intel_vvp_dpc_instance* instance, bool bypass);

/**
 * \brief Get color filter array phase setting of the DPC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     Color filter array phase setting
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
uint8_t intel_vvp_dpc_get_cfa_phase(intel_vvp_dpc_instance* instance);

/**
 * \brief Set color filter array phase setting of the DPC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  cfa_phase, color filter array phase setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcValueErr if the value is outside the valid range
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_cfa_phase(intel_vvp_dpc_instance* instance, uint8_t cfa_phase);

/**
 * \brief Get sense level setting of the DPC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     Sense Level setting on success.
 *             0xFF if instance is null or debug is disabled.
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 *             and configured with debug enabled.
 */
uint8_t intel_vvp_dpc_get_sense_level(intel_vvp_dpc_instance* instance);

/**
 * \brief Set Sense Level setting of the DPC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  sense_level, sense level setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcValueErr if the value is outside the valid range
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_sense_level(intel_vvp_dpc_instance* instance, uint8_t sense_level);

/**
 * \brief Reads the pixel clip counter reset bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     true if pixel clip counter reset is set
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_get_pix_clip_cnt_rst(intel_vvp_dpc_instance* instance);

/**
 * \brief Writes the pixel clip counter reset bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  reset, reset pixel counters if true
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_pix_clip_cnt_rst(intel_vvp_dpc_instance* instance, bool reset);

/**
 * \brief Get pixel clip counter select setting of the DPC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     pixel clip counter select setting on success.
 *             0xFF if instance is invalid or debug is disabled.
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 *             and configured with debug enabled.
 */
uint8_t intel_vvp_dpc_get_pix_clip_cnt_sel(intel_vvp_dpc_instance* instance);

/**
 * \brief Set pixel clip counter select setting of the DPC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  pix_clip_cnt_sel, pixel clip counter select setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpDpcValueErr if the value is outside the valid range
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_pix_clip_cnt_sel(intel_vvp_dpc_instance* instance, uint8_t pix_clip_cnt_sel);

/**
 * \brief Get freeze statistics request setting flag of the DPC instance
 *
 * \param[in]  instance, pointer to the intel_vvp_dpc_instance
 * \return     true if freeze statistics request is set, false if cleared
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
bool intel_vvp_dpc_get_freeze_stats_request(intel_vvp_dpc_instance* instance);

/**
 * \brief Set freeze statistics request setting flag of the DPC instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_dpc_instance
 * \param[in]  freeze_stats, freeze statistics request is set if true, cleared if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_dpc_instance that was successfully initialized
 */
int intel_vvp_dpc_set_freeze_stats_request(intel_vvp_dpc_instance* instance, bool freeze_stats);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_DPC_H__ */
