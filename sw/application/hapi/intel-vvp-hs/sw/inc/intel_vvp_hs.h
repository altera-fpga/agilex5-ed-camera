/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_hs_instance and associated functions
 *
 * Driver for the Video & Vision Processing HS Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_hs_regs.h
 */
#ifndef __INTEL_VVP_HS_H__
#define __INTEL_VVP_HS_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_hs_regs.h"

#define INTEL_VVP_HS_PRODUCT_ID                           0x017bu              ///< Hs product ID
#define INTEL_VVP_HS_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_HS_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_HS_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< HS register read function
#define INTEL_VVP_HS_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< HS register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpHsRegMapVersionErr = -100,
    kIntelVvpHsParameterErr = -101,
    kIntelVvpHsValueErr = -102,
    kIntelVvpHsPointerErr = -103,
    kIntelVvpHsCommitPendingErr = -104,
    kIntelVvpHsFreezePendingErr = -105,
    kIntelVvpHsOutOfBoundsErr = -106,
} eIntelVvpHsErrors;

typedef struct intel_vvp_hs_instance_s
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
    uint16_t num_hist_bins;
    // Internal states
    uint32_t settings_reg;
    uint32_t frame_hist_base;
    uint32_t roi_hist_base ;
} intel_vvp_hs_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_hs_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the HS product id (0x017b)
 *               kIntelVvpHsRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_hs_init(intel_vvp_hs_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_hs_instance
 * \return      the lite_mode field in the intel_vvp_hs_instance
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_get_lite_mode(intel_vvp_hs_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_hs_instance
 * \return      the debug_enabled field in the intel_vvp_hs_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_get_debug_enabled(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint8_t intel_vvp_hs_get_bits_per_sample_in(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint8_t intel_vvp_hs_get_bits_per_sample_out(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint8_t intel_vvp_hs_get_num_color_planes_in(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint8_t intel_vvp_hs_get_num_color_planes_out(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint8_t intel_vvp_hs_get_pixels_in_parallel(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint32_t intel_vvp_hs_get_max_width(intel_vvp_hs_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of an
 *        image or video frame
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint32_t intel_vvp_hs_get_max_height(intel_vvp_hs_instance* instance);

/**
 * \brief Get the number of histogram bins
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_hs_instance
 * \return      number of histogram bins
 * \pre         instance is a valid intel_vvp_hs_instance that was successfully initialized
 * \remarks     This is the number of bins in each histogram statistics
 */
uint16_t intel_vvp_hs_get_num_hist_bins(intel_vvp_hs_instance* instance);

/**
 * \brief Reads if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_is_running(intel_vvp_hs_instance* instance);

/**
 * \brief Get the commit request status of the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_commit_is_pending(intel_vvp_hs_instance* instance);

/**
 * \brief Get the statistics are frozen status of the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     true if statistics are frozen, false otherwise
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_stats_are_frozen(intel_vvp_hs_instance* instance);

/**
 * \brief Reads the status register
 *
 * \param[in]  instance, an intel_vvp_hs_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint32_t intel_vvp_hs_get_status(intel_vvp_hs_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_hs_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpHsPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_get_frame_stats(intel_vvp_hs_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_commit(intel_vvp_hs_instance* instance);

/**
 * \brief Get freeze statistics request setting flag of the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     true if freeze statistics request is set, false if cleared
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
bool intel_vvp_hs_get_freeze_stats_request(intel_vvp_hs_instance* instance);

/**
 * \brief Set freeze statistics request setting flag of the HS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \param[in]  freeze_stats, freeze statistics request is set if true, cleared if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_set_freeze_stats_request(intel_vvp_hs_instance* instance, bool freeze_stats);

/**
 * \brief Get the region of interest's horizontal start position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     Region of interest horizontal start position
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint16_t intel_vvp_hs_get_h_start(intel_vvp_hs_instance* instance);

/**
 * \brief Set the region of interest's horizontal start position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \param[in]  h_start, region of interest horizontal start position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsValueErr if h_start is outside the valid range
 *             kIntelVvpHsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_set_h_start(intel_vvp_hs_instance* instance, uint16_t h_start);

/**
 * \brief Get the region of interest's vertical start position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     Region of interest vertical start position
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint16_t intel_vvp_hs_get_v_start(intel_vvp_hs_instance* instance);

/**
 * \brief Set the region of interest's vertical start position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \param[in]  v_start, region of interest vertical start position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsValueErr if v_start is outside the valid range
 *             kIntelVvpHsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_set_v_start(intel_vvp_hs_instance* instance, uint16_t v_start);

/**
 * \brief Get the region of interest's horizontal end position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     Region of interest horizontal end position
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint16_t intel_vvp_hs_get_h_end(intel_vvp_hs_instance* instance);

/**
 * \brief Set the region of interest's horizontal end position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \param[in]  h_end, region of interest horizontal end position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsValueErr if h_end is outside the valid range
 *             kIntelVvpHsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_set_h_end(intel_vvp_hs_instance* instance, uint16_t h_end);

/**
 * \brief Get the region of interest's vertical end position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \return     Region of interest vertical end position
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
uint16_t intel_vvp_hs_get_v_end(intel_vvp_hs_instance* instance);

/**
 * \brief Set the region of interest's vertical end position set within the HS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_hs_instance
 * \param[in]  v_end, region of interest vertical end position
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsValueErr if v_end is outside the valid range
 *             kIntelVvpHsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_set_v_end(intel_vvp_hs_instance* instance, uint16_t v_end);

/**
 * \brief Read a single bin from the histogram of the whole frame
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \param[in]  data, pointer of a variable used for returning the histogram bin value
 * \param[in]  addr, relative address of the histogram bin
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsPointerErr if data is a null pointer
 *             kIntelVvpHsFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpHsOutOfBoundsErr if address is out of range
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_read_frame_hist_bin(intel_vvp_hs_instance* instance, uint32_t* data, uint16_t addr);

/**
 * \brief Read a single bin from the histogram of the region-of-interest
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \param[in]  data, pointer of a variable used for returning the histogram bin value
 * \param[in]  addr, relative address of the histogram bin
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsPointerErr if data is a null pointer
 *             kIntelVvpHsFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpHsOutOfBoundsErr if address is out of range
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_read_roi_hist_bin(intel_vvp_hs_instance* instance, uint32_t* data, uint16_t addr);

/**
 * \brief Read histogram of the full frame to an array
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \param[in]  data_array, pointer of an array with a minimum length of number of histogram bins.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsPointerErr if data_array is a null pointer
 *             kIntelVvpHsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_read_frame_hist_array(intel_vvp_hs_instance* instance, uint32_t* data_array);

/**
 * \brief Read histogram of the region-of-interest to an array
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_hs_instance
 * \param[in]  data_array, pointer of an array with a minimum length of number of histogram bins.
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpHsPointerErr if data_array is a null pointer
 *             kIntelVvpHsFreezePendingErr if frame statistics are not ready yet
 * \pre        instance is a valid intel_vvp_hs_instance that was successfully initialized
 */
int intel_vvp_hs_read_roi_hist_array(intel_vvp_hs_instance* instance, uint32_t* data_array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_HS_H__ */
