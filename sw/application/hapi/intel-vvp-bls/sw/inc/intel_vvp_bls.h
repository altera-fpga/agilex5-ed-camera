/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_bls_instance and associated functions
 *
 * Driver for the Video & Vision Processing BLS Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_bls_regs.h
 */
#ifndef __INTEL_VVP_BLS_H__
#define __INTEL_VVP_BLS_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_bls_regs.h"

#define INTEL_VVP_BLS_PRODUCT_ID                           0x0174u              ///< Bls product ID
#define INTEL_VVP_BLS_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_BLS_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_BLS_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< BLS register read function
#define INTEL_VVP_BLS_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< BLS register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpBlsRegMapVersionErr = -100,
    kIntelVvpBlsParameterErr = -101,
    kIntelVvpBlsValueErr = -102,
    kIntelVvpBlsPointerErr = -103,
    kIntelVvpBlsCommitPendingErr = -104,
    kIntelVvpBlsFreezePendingErr = -105,
    kIntelVvpBlsOutOfBoundsErr = -106,
} eIntelVvpBlsErrors;

typedef struct intel_vvp_bls_instance_s
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
} intel_vvp_bls_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_bls_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the BLS product id (0x0174)
 *               kIntelVvpBlsRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_bls_init(intel_vvp_bls_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_bls_instance
 * \return      the lite_mode field in the intel_vvp_bls_instance
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_get_lite_mode(intel_vvp_bls_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_bls_instance
 * \return      the debug_enabled field in the intel_vvp_bls_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_get_debug_enabled(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_bits_per_sample_in(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_bls_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_bits_per_sample_out(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_num_color_planes_in(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_num_color_planes_out(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_pixels_in_parallel(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint32_t intel_vvp_bls_get_max_width(intel_vvp_bls_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of an
 *        image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_bls_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint32_t intel_vvp_bls_get_max_height(intel_vvp_bls_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_is_running(intel_vvp_bls_instance* instance);

/**
 * \brief Get the commit request status of the BLS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_commit_is_pending(intel_vvp_bls_instance* instance);

/**
 * \brief Get the statistics are frozen status of the BLS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     true if statistics are frozen, false otherwise
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_stats_are_frozen(intel_vvp_bls_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_bls_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint32_t intel_vvp_bls_get_status(intel_vvp_bls_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_bls_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsPointerErr if stats_out is a null pointer
 *             kIntelVvpBlsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_get_frame_stats(intel_vvp_bls_instance* instance, uint32_t* stats_out);

/**
 * \brief Get the contents of the optical black accumulator corresponding to cfa phase 00
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  data, pointer of a variable to return optical black accumulator contents for
 *             color filter array phase 0
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsPointerErr if data is a null pointer
 *             kIntelVvpBlsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_get_cfa_00_sum(intel_vvp_bls_instance* instance, uint32_t* stats_out);

/**
 * \brief Get the contents of the optical black accumulator corresponding to cfa phase 01
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  data, pointer of a variable to return optical black accumulator contents for
 *             color filter array phase 1
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsPointerErr if data is a null pointer
 *             kIntelVvpBlsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_get_cfa_01_sum(intel_vvp_bls_instance* instance, uint32_t* stats_out);

/**
 * \brief Get the contents of the optical black accumulator corresponding to cfa phase 10
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  data, pointer of a variable to return optical black accumulator contents for
 *             color filter array phase 2
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsPointerErr if data is a null pointer
 *             kIntelVvpBlsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_get_cfa_10_sum(intel_vvp_bls_instance* instance, uint32_t* stats_out);

/**
 * \brief Get the contents of the optical black accumulator corresponding to cfa phase 11
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  data, pointer of a variable to return optical black accumulator contents for
 *             color filter array phase 3
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsPointerErr if data is a null pointer
 *             kIntelVvpBlsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_get_cfa_11_sum(intel_vvp_bls_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_commit(intel_vvp_bls_instance* instance);

/**
 * \brief Get color filter array phase setting of the BLS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     Color filter array phase setting
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint8_t intel_vvp_bls_get_cfa_phase(intel_vvp_bls_instance* instance);

/**
 * \brief Set color filter array phase setting of the BLS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_bls_instance
 * \param[in]  cfa_phase, color filter array phase setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsValueErr if the value is outside the valid range
 *             kIntelVvpBlsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_cfa_phase(intel_vvp_bls_instance* instance, uint8_t cfa_phase);

/**
 * \brief Get freeze statistics request setting flag of the BLS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     true if freeze statistics request is set, false if cleared
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
bool intel_vvp_bls_get_freeze_stats_request(intel_vvp_bls_instance* instance);

/**
 * \brief Set freeze statistics request setting flag of the BLS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_bls_instance
 * \param[in]  freeze_stats, freeze statistics request is set if true, cleared if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_freeze_stats_request(intel_vvp_bls_instance* instance, bool freeze_stats);

/**
 * \brief Get the optical black's horizontal start position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     Optical black horizontal start position
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint16_t intel_vvp_bls_get_h_start(intel_vvp_bls_instance* instance);

/**
 * \brief Set the optical black's horizontal start position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  h_start, optical black horizontal start position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_h_start(intel_vvp_bls_instance* instance, uint16_t h_start);

/**
 * \brief Get the optical black's vertical start position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     Optical black vertical start position
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint16_t intel_vvp_bls_get_v_start(intel_vvp_bls_instance* instance);

/**
 * \brief Set the optical black's vertical start position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  v_start, optical black vertical start position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_v_start(intel_vvp_bls_instance* instance, uint16_t v_start);

/**
 * \brief Get the optical black's horizontal end position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     Optical black horizontal end position
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint16_t intel_vvp_bls_get_h_end(intel_vvp_bls_instance* instance);

/**
 * \brief Set the optical black's horizontal end position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  h_end, optical black horizontal end position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_h_end(intel_vvp_bls_instance* instance, uint16_t h_end);

/**
 * \brief Get the optical black's vertical end position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \return     Optical black vertical end position
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
uint16_t intel_vvp_bls_get_v_end(intel_vvp_bls_instance* instance);

/**
 * \brief Set the optical black's vertical end position set within the bls instance
 *
 * \param[in]  instance, pointer to the intel_vvp_bls_instance
 * \param[in]  v_end, optical black vertical end position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpBlsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_bls_instance that was successfully initialized
 */
int intel_vvp_bls_set_v_end(intel_vvp_bls_instance* instance, uint16_t v_end);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_BLS_H__ */
