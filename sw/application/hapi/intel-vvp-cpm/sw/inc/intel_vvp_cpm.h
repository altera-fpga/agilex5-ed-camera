/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_cpm_instance and associated functions
 *
 * Driver for the Video & Vision Processing Color Plane Manager Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_cpm_regs.h
 */

#ifndef __INTEL_VVP_CPM_H__
#define __INTEL_VVP_CPM_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_CPM_PRODUCT_ID                           0x023Bu              ///< CPM product ID
#define INTEL_VVP_CPM_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_CPM_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version
#define INTEL_VVP_CPM_MAX_PLANES                           4

#define INTEL_VVP_CPM_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< CPM register read function
#define INTEL_VVP_CPM_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< CPM register write function

typedef enum {
    kIntelVvpColorPlaneManagerRegMapVersionErr = -100,
    kIntelVvpColorPlaneManagerParameterErr     = -101
} eIntelVvpColorPlaneManagerErrors;

/**
 * \brief The definition of an intel_vvp_cpm_instance.
 *
 * This is the definition for an intel_vvp_cpm_instance. It starts with an intel_vvp_core_instance
 * and can be used as such in generic functions
 */
typedef struct intel_vvp_cpm_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance

    // Compile-time parameterization
    bool       lite_mode;                            ///< instance lite mode parameter, compile-time constant for the hardware and set at initialization
    bool       debug_enabled;                        ///< instance debug_enabled parameter, compile-time constant for the hardware and set at initialization
    uint8_t    num_planes_in;
    uint8_t    num_planes_out;                       ///< instance num_planes parameter, compile-time constant for the hardware and set at initialization

} intel_vvp_cpm_instance;

typedef uint16_t intel_vvp_cpm_padding[INTEL_VVP_CPM_MAX_PLANES];

/**
 * \brief Initialise a Color Plane Manager instance
 *
 * Initialization function for a VVP Color Plane Manager instance.
 * Attempts to initialize the fields of the Color Plane Manager and its base core
 *
 * \param[in]    instance, pointer to the intel_vvp_cpm_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the CPR product id (0x0231)
 *               kIntelVvpColorPlaneManagerRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_cpm_init(intel_vvp_cpm_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a Color Plane Manager instance
 *
 * \param[in]  instance, an intel_vvp_cpm_instance
 * \return     the lite_mode field in the intel_vvp_cpm_instance
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
bool intel_vvp_cpm_get_lite_mode(intel_vvp_cpm_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a Color Plane Manager instance
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     the debug_enabled field in the intel_vvp_cpm_instance
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
bool intel_vvp_cpm_get_debug_enabled(intel_vvp_cpm_instance* instance);

/**
 * \brief Fetch and return the compile-time bits per (color) sample  value
 *
 * \param[in]    instance, pointer to the intel_vvp_cpm_instance
 * \return       the number of bits per color sample
 */
uint8_t intel_vvp_cpm_get_bits_per_sample(intel_vvp_cpm_instance* instance);

/**
 * \brief Fetch and return the number of pixels in parallelcompile-time parameter
 *
 * \param[in]    instance, pointer to the intel_vvp_cpm_instance
 * \return       the number of pixels in parallel per beat
 */
uint8_t intel_vvp_cpm_get_pixels_in_parallel(intel_vvp_cpm_instance* instance);

/**
 * \brief Get the number of video planes present at input
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     Returns the number of video planes present at input. It may return 0 if the instance is invalid
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
uint8_t intel_vvp_cpm_get_num_planes_in(intel_vvp_cpm_instance* instance);

/**
 * \brief Get the number of video planes present at output
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     Returns the number of video planes present at output. It may return 0 if the instance is invalid
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
uint8_t intel_vvp_cpm_get_num_planes_out(intel_vvp_cpm_instance* instance);

/**
 * \brief get running status of the Color Plane Manager IP
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
bool intel_vvp_cpm_is_running(intel_vvp_cpm_instance* instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_cpm_instance parameterized in full mode
 */
bool intel_vvp_cpm_get_commit_status(intel_vvp_cpm_instance* instance);

/**
 * \brief Read the status register
 *
 * \param[in]  instance, an intel_vvp_cpm_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
uint8_t intel_vvp_cpm_get_status(intel_vvp_cpm_instance *instance);

/**
 * \brief Set the register map padding value for a specific plane
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \param[in]  plane, selected color plane
 * \param[in]  padding_value, new padding_value value
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if instance is invalid
 *             kIntelVvpColorPlaneManagerParameterErr if the color plane is invalid
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
int intel_vvp_cpm_set_plane_padding(intel_vvp_cpm_instance* instance, uint8_t plane, uint16_t padding_value);

/**
 * \brief Set the min/max values for each plane fitted
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \param[in]  padding_values, array of padding values
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr is instance is invalid
 *             kIntelVvpCoreNullPtrErr if padding_values is the NULL pointer
 * \remarks    padding values for absent output planes will be ignored
 * \pre        instance is a valid intel_vvp_cpm_instance successfully initialized
 */
int intel_vvp_cpm_set_padding(intel_vvp_cpm_instance* instance, const intel_vvp_cpm_padding padding_values);

/**
 * \brief Get the register map padding value for a specific plane
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \param[in]  plane, selected color plane in the range [0..num_planes_out-1]
 * \return     padding value, returns 0 for error cases (invalid instance/plane, debug disabled, ...)
 * \pre        instance is a valid intel_vvp_cpm_instance parameterized with debug enabled, plane is a valid plane
 */
uint16_t intel_vvp_cpm_get_plane_padding(intel_vvp_cpm_instance* instance, uint8_t plane);

/**
 * \brief Get the register map padding values for each plane fitted
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \param[out] padding_values, pointer to the intel_vvp_cpm_padding structure that will be filled with num_planes sets of padding data
 * \return     kIntelVvpCoreOk in case of success
 *             kIntelVvpCoreInstanceErr if instance is invalid or with debug disabled
 *             kIntelVvpCoreNullPtrErr if min_max is the NULL pointer
 * \pre        min/max values returned are undefined when used without debug enabled
 * \remarks    min/max values for absent planes will be set to 0
 * \pre        instance is a valid intel_vvp_cpm_instance parameterized with debug enabled
 */
int intel_vvp_cpm_get_padding(intel_vvp_cpm_instance* instance, intel_vvp_cpm_padding padding_values);


/**
 * \brief commit any outstanding writes from setxxxx commands
 *
 * \param[in]  instance, pointer to the intel_vvp_cpm_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_cpm_instance parameterized in full mode
 *             (commit is automatic in lite mode)
 */
int intel_vvp_cpm_commit_writes(intel_vvp_cpm_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_CPM_H__
