/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_pip_conv_instance and associated functions
 *
 * Driver for the Video & Vision Processing Guard Bands Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_pip_conv_regs.h
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifndef __INTEL_VVP_PIP_CONV_H__
#define __INTEL_VVP_PIP_CONV_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_PIP_CONV_PRODUCT_ID                           0x0239u              ///< pip converter product ID
#define INTEL_VVP_PIP_CONV_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_PIP_CONV_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_PIP_CONV_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< PIP_CONV register read function
#define INTEL_VVP_PIP_CONV_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< PIP_CONV register write function

typedef enum {
    kIntelVvpPipConvRegMapVersionErr = -100,
} eIntelVvpPipConvErrors;

typedef struct intel_vvp_pip_conv_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    bool    lite_mode;                               ///< for consistency (must be true otherwise the slave interface wouldn't be there)
    bool    debug_enabled;
} intel_vvp_pip_conv_instance;	   

/**
 * \brief Initialise a pip converter instance
 * 
 * Initialization function for a VVP pixels in parallel converter instance.
 * Attempts to initialize the fields of the pip converter and its base core
 * 
 * \param[in]  instance, pointer to the intel_vvp_pip_conv_instance to initialize
 * \param[in]  base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return     kIntelVvpCoreOk in case of success,
 *             kIntelVvpCoreInstanceErr if instance is NULL
 *             kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *             kIntelVvpCorePidErr if the product id of the core is not the pip converter product id (0x0239)
 *             kIntelVvpPipConvRegMapVersionErr if the register map is not supported
 * \remarks    On returning a non-zero error code the instance will not be initialized and
 *             cannot be used further by the application using this driver
 */
int intel_vvp_pip_conv_init(intel_vvp_pip_conv_instance* instance, intel_vvp_core_base base);
    

/**
 * \brief Query the lite_mode parameter of a pip converter instance
 * 
 * \param[in]  instance, pointer to the intel_vvp_pip_conv_instance 
 * \return     the lite_mode field in the intel_vvp_pip_conv_instance
 * \remarks    always true, the slave interface is not available in full mode
 * \pre        instance is valid and successfully initialized
 */
bool intel_vvp_pip_conv_get_lite_mode(intel_vvp_pip_conv_instance* instance);

/**
 * \brief Query the debug_enabled parameter of a pip converter instance
 * 
 * \param[in]  instance, pointer to the intel_vvp_pip_conv_instance 
 * \return     the debug_enabled field in the intel_vvp_pip_conv_instance
 * \pre        instance is valid and successfully initialized
 */
bool intel_vvp_pip_conv_get_debug_enabled(intel_vvp_pip_conv_instance* instance);

/**
 * \brief get running status of the guard-band IP
 * 
 * \param[in]  instance, pointer to the intel_vvp_pip_conv_instance 
 * \return     true if processing image data, false between fields
 * \pre        instance is valid and successfully initialized
 */
bool intel_vvp_pip_conv_is_running(intel_vvp_pip_conv_instance* instance);

/**
 * \brief Read the status register
 * 
 * \param[in]  instance, pointer to the intel_vvp_pip_conv_instance 
 * \return     the value returned from a read to the status register
 * \pre        instance is valid and successfully initialized
 */
uint8_t intel_vvp_pip_conv_get_status(intel_vvp_pip_conv_instance *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_VVP_PIP_CONV_H__
