/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_wbs_instance and associated functions
 *
 * Driver for the Video & Vision Processing WBS Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_wbs_regs.h
 */
#ifndef __INTEL_VVP_WBS_H__
#define __INTEL_VVP_WBS_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_wbs_regs.h"

#define INTEL_VVP_WBS_PRODUCT_ID                           0x0179u              ///< WBS product ID
#define INTEL_VVP_WBS_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_WBS_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_WBS_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< WBS register read function
#define INTEL_VVP_WBS_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< WBS register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpWbsRegMapVersionErr = -100,
    kIntelVvpWbsParameterErr = -101,
    kIntelVvpWbsValueErr = -102,
    kIntelVvpWbsPointerErr = -103,
    kIntelVvpWbsCommitPendingErr = -104,
    kIntelVvpWbsFreezePendingErr = -105,
    kIntelVvpWbsOutOfBoundsErr = -106,
} eIntelVvpWbsErrors;

typedef struct intel_vvp_wbs_instance_s
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
    uint8_t  precision_bits;
    // Internal states
    uint32_t settings_reg;

    uint8_t ratio_size_in_bits;
    uint8_t pixel_count_size_in_bits;
    uint8_t num_entries_per_table_result;
} intel_vvp_wbs_instance;

typedef struct intel_vvp_wbs_zone_result_s
{
    long unsigned int x0_integer;
    uint16_t x0_fraction;
    long unsigned int x1_integer;
    uint16_t x1_fraction;
    uint16_t num_pixels_accumulated;
} intel_vvp_wbs_zone_result;



/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_wbs_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the WBS product id (0x0179)
 *               kIntelVvpWbsRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code, the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_wbs_init(intel_vvp_wbs_instance* instance, intel_vvp_core_base base);

/**
 * \brief Returns if lite mode is on
 *
 * \param[in]   instance, pointer to the intel_vvp_wbs_instance
 * \return      the lite_mode field in the intel_vvp_wbs_instance
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_get_lite_mode(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns if debug features is on
 *
 * \param[in]   instance, pointer to the intel_vvp_wbs_instance
 * \return      the debug_enabled field in the intel_vvp_wbs_instance (true if R/W registers can be read back)
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_get_debug_enabled(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the number of bits per color sample for the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      number of bits
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_bits_per_sample_in(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the number of bits per color sample value for the streaming output interface
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \return     number of bits
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_bits_per_sample_out(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming input interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_num_color_planes_in(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the number of color planes of the streaming output interface
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      number of color planes
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_num_color_planes_out(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the pixels in parallel of the streaming input and output interfaces
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      number of pixels in parallel
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_pixels_in_parallel(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the horizontal dimension of
 *        an image or video frame.
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      maximum width set at compile-time
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_max_width(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns the maximum number of pixels that the IP supports on the vertical dimension of
 *        an image or video frame.
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      maximum height set at compile-time
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_max_height(intel_vvp_wbs_instance* instance);

/**
 * \brief Get the precision_bits parameter of an ip instance
 *
 * \param[in]   instance, pointer to the initialized intel_vvp_wbs_instance
 * \return      precision_bits
 * \pre         instance is a valid intel_vvp_wbs_instance that was successfully initialized
 * \remarks     This is the number of precision_bits
 */
uint8_t intel_vvp_wbs_get_precision_bits(intel_vvp_wbs_instance* instance);

/**
 * \brief Returns if the IP is running
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_is_running(intel_vvp_wbs_instance* instance);

/**
 * \brief Get the commit request status of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     true if commit request is pending, false otherwise
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_commit_is_pending(intel_vvp_wbs_instance* instance);

/**
 * \brief Get the statistics are frozen status of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     true if statistics are frozen, false otherwise
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_stats_are_frozen(intel_vvp_wbs_instance* instance);

/**
 * \brief Read the status register
 *
 * \param[in]  instance, an intel_vvp_wbs_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_status(intel_vvp_wbs_instance* instance);

/**
 * \brief Read the frame statistics register
 *
 * \param[in]  instance, an intel_vvp_wbs_instance
 * \param[in]  stats_out, pointer of a variable used for returning the frame statistics value read
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsFreezePendingErr if frame statistics are not ready yet
 *             kIntelVvpWbsPointerErr if stats_out is a null pointer
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_get_frame_stats(intel_vvp_wbs_instance* instance, uint32_t* stats_out);

/**
 * \brief Send a commit request Used for atomic access to the core-specific writable registers
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_commit(intel_vvp_wbs_instance* instance);

/**
 * \brief Reads the bypass bit from the status register
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     true if bypass mode is enabled, false if bypass mode is disabled
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_get_bypass(intel_vvp_wbs_instance* instance);

/**
 * \brief Writes the bypass bit of the status register
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  bypass, bypass mode is enabled if true, bypass mode is disabled if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_bypass(intel_vvp_wbs_instance* instance, bool bypass);

/**
 * \brief Get color filter array phase setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     Color filter array phase setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_cfa_phase(intel_vvp_wbs_instance* instance);

/**
 * \brief Set color filter array phase setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_phase, color filter array phase setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if the value is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_phase(intel_vvp_wbs_instance* instance, uint8_t cfa_phase);

/**
 * \brief Get color filter array invert setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     Color filter array invert setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint8_t intel_vvp_wbs_get_cfa_invert(intel_vvp_wbs_instance* instance);

/**
 * \brief Set color filter array invert setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_invert, color filter array invert setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if cfa_invert is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_invert(intel_vvp_wbs_instance* instance, uint8_t cfa_invert);

/**
 * \brief Get freeze statistics request setting flag of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     true if freeze statistics request is set, false if cleared
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
bool intel_vvp_wbs_get_freeze_stats_request(intel_vvp_wbs_instance* instance);

/**
 * \brief Set freeze statistics request setting flag of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  freeze_stats, freeze statistics request is set if true, cleared if false
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_freeze_stats_request(intel_vvp_wbs_instance* instance, bool freeze_stats);

/**
 * \brief Get h start setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     h start setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_h_start(intel_vvp_wbs_instance* instance);

/**
 * \brief Set h start setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  h_start, h start setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if h_start is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_h_start(intel_vvp_wbs_instance* instance, uint16_t h_start);

/**
 * \brief Get v start setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     v start setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_v_start(intel_vvp_wbs_instance* instance);

/**
 * \brief Set v start setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  v_start, v start setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if v_start is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_v_start(intel_vvp_wbs_instance* instance, uint16_t v_start);

/**
 * \brief Get h end setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     h end setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_h_end(intel_vvp_wbs_instance* instance);

/**
 * \brief Set h end setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  h_end, h end setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if h_end is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_h_end(intel_vvp_wbs_instance* instance, uint16_t h_end);

/**
 * \brief Get v end setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     v end setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_v_end(intel_vvp_wbs_instance* instance);

/**
 * \brief Set v end setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  v_end, v end setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if v_end is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_v_end(intel_vvp_wbs_instance* instance, uint16_t v_end);

/**
 * \brief Get zone h count setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     zone h count setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_zone_h_count(intel_vvp_wbs_instance* instance);

/**
 * \brief Set zone h count setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  zone_h_count, zone h count setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if zone_h_count is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_zone_h_count(intel_vvp_wbs_instance* instance, uint16_t zone_h_count);

/**
 * \brief Get zone v count setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     zone v count setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint16_t intel_vvp_wbs_get_zone_v_count(intel_vvp_wbs_instance* instance);

/**
 * \brief Set zone v count setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  zone_v_count, zone v count setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if zone_v_count is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_zone_v_count(intel_vvp_wbs_instance* instance, uint16_t zone_v_count);

/**
 * \brief Get cfa x0 range lo setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     cfa x0 range lo setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_cfa_x0_range_lo(intel_vvp_wbs_instance* instance);

/**
 * \brief Set cfa x0 range lo setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_x0_range_lo, cfa x0 range lo setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if cfa_x0_range_lo is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_x0_range_lo(intel_vvp_wbs_instance* instance, uint32_t cfa_x0_range_lo);

/**
 * \brief Get cfa x0 range hi setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     cfa x0 range hi setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_cfa_x0_range_hi(intel_vvp_wbs_instance* instance);

/**
 * \brief Set cfa x0 range hi setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_x0_range_hi, cfa x0 range hi setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if cfa_x0_range_hi is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_x0_range_hi(intel_vvp_wbs_instance* instance, uint32_t cfa_x0_range_hi);

/**
 * \brief Get cfa x1 range lo setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     cfa x1 range lo setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_cfa_x1_range_lo(intel_vvp_wbs_instance* instance);

/**
 * \brief Set cfa x1 range lo setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_x1_range_lo, cfa x1 range lo setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if cfa_x1_range_lo is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_x1_range_lo(intel_vvp_wbs_instance* instance, uint32_t cfa_x1_range_lo);

/**
 * \brief Get cfa x1 range hi setting of the WBS instance
 *
 * \param[in]  instance, pointer to the intel_vvp_wbs_instance
 * \return     cfa x1 range hi setting
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
uint32_t intel_vvp_wbs_get_cfa_x1_range_hi(intel_vvp_wbs_instance* instance);

/**
 * \brief Set cfa x1 range hi setting of the WBS instance
 *
 * \param[in]  instance, pointer to the initialized intel_vvp_wbs_instance
 * \param[in]  cfa_x1_range_hi, cfa x1 range hi setting
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if cfa_x1_range_hi is outside the valid range
 *             kIntelVvpWbsCommitPendingErr if a previous commit request is pending
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_set_cfa_x1_range_hi(intel_vvp_wbs_instance* instance, uint32_t cfa_x1_range_hi);

/**
 * \brief Get the raw results table entries from a specified zone
 *
 * \param[in]  vert, vertical coordinate of the zone subregion. Valid values are 0-6
 * \param[in]  horiz, horizontal coordinate of the zone subregion. Valid values are 0-6
 * \param[out] num_entries, provided a valid pointer, the number of 20-bit entries is entered into the memory location
 * \param[out] entry_store, provided a valid pointer to an array of uint32_t variables, the table contents are copied into the array
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if the instance is a null pointer
 *             kIntelVvpWbsValueErr if arguments are outside their valid ranges
 *             kIntelVvpWbsPointerErr if at least one of the pointers is null
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
int intel_vvp_wbs_get_table_entries(intel_vvp_wbs_instance* instance, uint8_t vert, uint8_t horiz, uint8_t* num_entries, uint32_t* entry_store);

/**
 * \brief Get formatted results from a zone in the region of interest
 *
 * \param[in]  vert, vertical coordinate of the zone subregion. Valid values are 0-6
 * \param[in]  horiz, horizontal coordinate of the zone subregion. Valid values are 0-6
 * \return     intel_vvp_wbs_zone_result struct containing the x0/x1 ratios and pixel accumulation values,
 *             extracted from the result table and reconstructed to be easier to work with.
 *             Will return all FF's if the instance is invalid.
 * \pre        instance is a valid intel_vvp_wbs_instance that was successfully initialized
 */
intel_vvp_wbs_zone_result intel_vvp_wbs_get_formatted_table_result(intel_vvp_wbs_instance* instance, uint8_t vert, uint8_t horiz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_WBS_H__ */
