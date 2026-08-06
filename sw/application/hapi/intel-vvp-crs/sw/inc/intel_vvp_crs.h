/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_crs_instance and associated functions
 *
 * Driver for the Video & Vision Processing Guard Bands Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_crs_regs.h
 */

#ifndef __INTEL_VVP_CRS_H__
#define __INTEL_VVP_CRS_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "intel_vvp_core.h"
#include "intel_vvp_crs_regs.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// NOTE: CRS is Chroma ReSampler

#define INTEL_VVP_CRS_PRODUCT_ID                           0x022Eu              ///< CRS product ID
#define INTEL_VVP_CRS_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_CRS_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_CRS_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< 3D LUT register read function
#define INTEL_VVP_CRS_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< 3D LUT register write function


/**
 * \brief The enum for common CRS error codes
 * 
 * This is the definition for the error codes
 */
typedef enum {
    kIntelVvpCrsRegMapVersionErr = -100,
    kIntelVvpCrsParamErr         = -101,
} eIntelVvpCrsErrors;


/**
 * \brief The enum for subsampling modes
 * 
 * This is the definition for supported input/output subsamplings
 */
typedef enum {
    kIntelVvpCrsSubsampling420      = INTEL_VVP_CRS_SUBSAMPLING_420,
    kIntelVvpCrsSubsampling444      = INTEL_VVP_CRS_SUBSAMPLING_444, 
    kIntelVvpCrsSubsampling422      = INTEL_VVP_CRS_SUBSAMPLING_422, 
    kIntelVvpCrsSubsamplingInvalid  = INTEL_VVP_CRS_SUBSAMPLING_INVALID,
} eIntelCrsSubsampling;

const unsigned int kIntelVvpCrsNumSubsamplings    = INTEL_VVP_CRS_NUM_SUBSAMPLINGS;

/**
 * \brief The definition of an intel_vvp_crs_instance.
 * 
 * This is the definition for an intel_vvp_crs_instance. It starts with an intel_vvp_core_instance
 * and can be used as such in generic functions
 */
typedef struct intel_vvp_crs_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool        lite_mode;
    bool        debug_enabled;
    bool        supported_modes[INTEL_VVP_CRS_NUM_SUBSAMPLINGS][INTEL_VVP_CRS_NUM_SUBSAMPLINGS];
} intel_vvp_crs_instance;	   

/**
 * \brief Initialise a chroma resampler instance
 * 
 * Initialization function for a VVP CRS instance.
 * Attempts to initialize the fields of the CRS and its base core
 * 
 * \param[in]    instance, pointer to the intel_vvp_crs_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the crs product id (0x022E)
 *               kIntelVvpCrsRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */

int intel_vvp_crs_init(intel_vvp_crs_instance* instance, intel_vvp_core_base base);

/**
 * \brief Query the lite_mode parameter of a crs instance
 * 
 * \param[in]  instance, an intel_vvp_crs_instance
 * \return     the lite_mode field in the intel_vvp_crs_instance
 * \pre        instance is a valid, initialized intel_vvp_crs_instance
 */
bool intel_vvp_crs_get_lite_mode(intel_vvp_crs_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a crs instance
 * 
 * \param[in]  instance, pointer to the intel_vvp_crs_instance 
 * \return     the debug_enabled field in the intel_vvp_crs_instance
 * \pre        instance is a valid, initialized intel_vvp_crs_instance
 */
bool intel_vvp_crs_get_debug_enabled(intel_vvp_crs_instance* instance);

/**
 * \brief get running status of the crs IP
 * 
 * \param[in]  instance, pointer to the intel_vvp_crs_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid, initialized intel_vvp_crs_instance
 */
bool intel_vvp_crs_is_running(intel_vvp_crs_instance* instance);

/**
 * \brief Determine if the IP core has any writes that have NOT been commited
 * 
 * \param[in]  instance, pointer to the intel_vvp_crs_instance 
 * \return     true if there are outstanding writes
 * \pre        instance is a valid intel_vvp_crs_instance parameterized in full mode
 */
bool intel_vvp_crs_get_commit_status(intel_vvp_crs_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, an intel_vvp_crs_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid, initialized intel_vvp_crs_instance
 */
uint8_t intel_vvp_crs_get_status(intel_vvp_crs_instance *instance);

/**
 * \brief Determine whether the IP core supports a defined chroma resampling conversion
 * 
 * \param[in]    instance, pointer to the intel_vvp_crs_instance 
 * \param[in]    input_subsampling, input subsampling
 * \param[in]    output_subsampling, output subsampling
 * \return       true if the instance supports the conversion input_subsampling -> output_subsampling
 * \pre          instance is a valid, initialized intel_vvp_crs_instance
 */
bool intel_vvp_crs_is_conversion_supported(intel_vvp_crs_instance* instance,
                                           eIntelCrsSubsampling input_subsampling, eIntelCrsSubsampling output_subsampling);

/**
 * \brief Returns the current selected output subsampling
 *
 * \param[in]   instance, pointer to the intel_vvp_crs_instance
 * \return      current output subsampling
 * \pre         instance is a valid, initialized intel_vvp_crs_instance with debug enabled
 * \see         intel_vvp_core_get_img_info_subsampling to check the current input subsampling (in full mode with debug enabled)
 */
eIntelCrsSubsampling intel_vvp_crs_get_output_subsampling(intel_vvp_crs_instance* instance);

/**
 * \brief Set the current output subsampling
 *  
 * \param[in]     instance, pointer to the intel_vvp_pip_conv_instance
 * \param[in]     output_subsampling, the requested output subsampling
 * \return        kIntelVvpCoreOk in case of success
 *                kIntelVvpCoreInstanceErr, if the instance is invalid
 *                kIntelVvpCrsParamErr, if the output subsampling requested is invalid
 * \pre           instance is a valid, initialized intel_vvp_crs_instance
 * \pre           the output subsampling is supported for the current input_subsampling, the call does not
 *                validate this and the IP may implement a different behaviour if the conversion is unsupported
 * \see           intel_vvp_core_set_img_info_subsampling to set the current input subsampling (in lite mode)
 */
int intel_vvp_crs_set_output_subsampling(intel_vvp_crs_instance* instance, eIntelCrsSubsampling output_subsampling);

/**
 * \brief commit any outstanding writes from setxxxx commands
 * 
 * \param[in]    instance, pointer to the intel_vvp_crs_instance 
 * \return       kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre          instance is a valid, initialized instance
 * \pre          the IP is parameterized in full mode (commit is automatic in lite mode)
 */
int intel_vvp_crs_commit_writes(intel_vvp_crs_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_CRS_H__
