/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_exposure_fusion_instance and associated functions
 *
 * Driver for the Video & Vision Processing EXPOSURE_FUSION Intel FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_exposure_fusion_regs.h
 */
#ifndef __INTEL_VVP_EXPOSURE_FUSION_H__
#define __INTEL_VVP_EXPOSURE_FUSION_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
#include "intel_vvp_exposure_fusion_regs.h"

#define INTEL_VVP_EXPOSURE_FUSION_PRODUCT_ID                           0x0255              ///< EXPOSURE_FUSION product ID
#define INTEL_VVP_EXPOSURE_FUSION_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_EXPOSURE_FUSION_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_VVP_EXPOSURE_FUSION_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< EXPOSURE_FUSION register read function
#define INTEL_VVP_EXPOSURE_FUSION_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< EXPOSURE_FUSION register write function

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum
{
    kIntelVvpExposureFusionRegMapVersionErr = -100,
    kIntelVvpExposureFusionParameterErr = -101,
    kIntelVvpExposureFusionValueErr = -102,
} eIntelVvpExposureFusionErrors;

typedef enum
{
    kIntelVvpExposureFusionModeFusion = 0,
    kIntelVvpExposureFusionModeShortExposure = 1,
    kIntelVvpExposureFusionModeLongExposure = 3
} eIntelVvpExposureFusionMode;

typedef struct intel_vvp_exposure_fusion_instance_s
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
} intel_vvp_exposure_fusion_instance;


/**
 * \brief Initialize an ip instance
 *
 * The initialization stops early if the vendor ID or product ID read at the base address are not
 * a match or if the register map version is not supported. Otherwise, the function proceeds to
 * read and store the IP compile-time parameterization. The instance is not fully initialized and
 * the application should not use it further if returning a non-zero error code.
 *
 * \param[in]    instance, pointer to the intel_vvp_exposure_fusion_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success, a negative error code in case of an error
 *               kIntelVvpCoreInstanceErr if the instance is a null pointer
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7)
 *               kIntelVvpCorePidErr if the product id of the core is not the EXPOSURE_FUSION product id (0x0177)
 *               kIntelVvpExposureFusionRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int intel_vvp_exposure_fusion_init(intel_vvp_exposure_fusion_instance* instance, intel_vvp_core_base base);

int intel_vvp_exposure_fusion_set_run_mode(intel_vvp_exposure_fusion_instance* instance, eIntelVvpExposureFusionMode mode);
eIntelVvpExposureFusionMode intel_vvp_exposure_fusion_get_run_mode(intel_vvp_exposure_fusion_instance* instance);

int intel_vvp_exposure_fusion_set_black_level(intel_vvp_exposure_fusion_instance* instance, uint16_t black_level);
uint16_t intel_vvp_exposure_fusion_get_black_level(intel_vvp_exposure_fusion_instance* instance);

int intel_vvp_exposure_fusion_set_exposure_ratio(intel_vvp_exposure_fusion_instance* instance, uint32_t ratio);
uint32_t intel_vvp_exposure_fusion_get_exposure_ratio(intel_vvp_exposure_fusion_instance* instance);

int intel_vvp_exposure_fusion_set_threshold(intel_vvp_exposure_fusion_instance* instance, uint16_t threshold);
uint16_t intel_vvp_exposure_fusion_get_threshold(intel_vvp_exposure_fusion_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_VVP_EXPOSURE_FUSION_H__ */
