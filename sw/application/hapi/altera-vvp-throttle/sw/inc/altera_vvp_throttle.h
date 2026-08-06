/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the altera_vvp_throttle_instance and associated functions
 *
 * Driver for the Video & Vision Processing video throttle FPGA IP
 *
 * \see Altera Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_mixer_regs.h
 */

#ifndef __ALTERA_VVP_THROTTLE_H__
#define __ALTERA_VVP_THROTTLE_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"
#include "altera_vvp_throttle_regs.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ALTERA_VVP_THROTTLE_PRODUCT_ID                           0x0256u              ///< Throttle product ID
#define ALTERA_VVP_THROTTLE_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define ALTERA_VVP_THROTTLE_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define ALTERA_VVP_THROTTLE_MAX_NUM_LAYERS                       8                    ///< Maximum number of supported layers (including the background layer)

#define ALTERA_VVP_THROTTLE_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Throttle register read function
#define ALTERA_VVP_THROTTLE_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Throttle register write function

typedef enum {
    kAlteraVvpThrottleRegMapVersionErr = -100,       // register map issue at initialization
    kAlteraVvpThrottleParameterErr     = -101,       // parameter error in a function call
    kAlteraVvpThrottleLayerErr         = -102,       // layer parameter error in a function call
} eAlteraVvpThrottleErrors;


typedef struct altera_vvp_throttle_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance

} altera_vvp_throttle_instance;	   


/**
 * \brief Initialise a throttle instance
 * 
 * Initialization function for a VVP throttle instance.
 * Attempts to initialize the fields of the throttle and its base core
 * 
 * \param[in]    instance, pointer to the altera_vvp_throttle_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the mixer product id (0x0233)
 *               kIntelVvpMixerRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */
int altera_vvp_throttle_init(altera_vvp_throttle_instance* instance, intel_vvp_core_base base);

int altera_vvp_throttle_start(altera_vvp_throttle_instance* instance);
int altera_vvp_throttle_stop(altera_vvp_throttle_instance* instance);

int altera_vvp_throttle_set_clock_cycles_per_line(altera_vvp_throttle_instance* instance, uint32_t clock_cycles);
int altera_vvp_throttle_set_active_lines(altera_vvp_throttle_instance* instance, uint32_t num_lines);
int altera_vvp_throttle_set_v_blank_lines(altera_vvp_throttle_instance* instance, uint32_t num_lines);

int altera_vvp_throttle_commit_writes(altera_vvp_throttle_instance* instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __ALTERA_VVP_THROTTLE_H__
