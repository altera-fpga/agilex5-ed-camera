/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Definition of the intel_vvp_snoop_instance and associated functions
 *
 * Driver for the Video & Vision Processing snoop
 *
 * \see Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 * \see intel_vvp_snoop_regs.h
 */
#ifndef __INTEL_VVP_SNOOP_H__
#define __INTEL_VVP_SNOOP_H__
 
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
 
#include "intel_vvp_core.h"
#include "intel_vvp_core_io.h"
 
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
 
#define INTEL_VVP_SNOOP_PRODUCT_ID                           0x0248u              ///< snoop product ID
#define INTEL_VVP_SNOOP_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_VVP_SNOOP_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version
 
#define INTEL_VVP_SNOOP_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< snoop register read function
#define INTEL_VVP_SNOOP_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< snoop register write function
 
typedef enum {
    kIntelVvpSnoopRegMapVersionErr = -100
} eIntelVvpSnoopErrors;
 
 
typedef struct intel_vvp_snoop_instance_s
{
    // intel VVP base instance
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
 
    // Compile-time parameterization
    bool                       lite_mode;            ///< instance lite mode parameter, compile-time constant for the hardware and set at initialization
    //bool                     debug_enabled;        ///< instance debug enabled parameter, skipped because the monitor is essentially a debug module
    uint8_t                    pip;                  ///< instance pixels in parallel, compile-time constant for the hardware and set at initialization
} intel_vvp_snoop_instance;
 
 
/**
 * \brief Initialise a monitor instance
 *
 * Initialization function for a VVP monitor instance, attempts accesses to the core
 * to initialize the fields of the snoop instance and its base core instance
 *
 * \param[in]    instance, the intel_vvp_snoop_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios, this is simply the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the snoop product id (0x0248)
 *               kIntelVvpsnoopRegMapVersionErr if the register map is not supported
 *               The initialization stops early if the vendor ID/product ID are not a match
 *               and the second register, containing version numbers, is not read
 * \pre          base is a proper accessor for an Intel VVP Monitor IP core
 */
int intel_vvp_snoop_init(intel_vvp_snoop_instance *instance, intel_vvp_core_base base);
 
/**
 * \brief Query the lite_mode parameter of a monitor instance
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the lite_mode field in the intel_vvp_snoop_instance
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
bool intel_vvp_snoop_get_lite_mode(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Query the debug_enabled parameter of a monitor instance
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     always true unless instance if invalid
 */
bool intel_vvp_snoop_get_debug_enabled(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Query the pixels_in_parallel parameter of a monitor instance
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the pip field in the intel_vvp_snoop_instance
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint8_t intel_vvp_snoop_get_pixels_in_parallel(intel_vvp_snoop_instance *instance);
 
/**
 * \brief get running status of the monitor instance
 *
 * \param[in]  instance, pointer to the intel_vvp_snoop_instance
 * \return     true if processing image data, false between fields
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
bool intel_vvp_snoop_is_running(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the status register
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the status register
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint8_t intel_vvp_snoop_get_status(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the number of good/valid fields
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the number of good fields register
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint16_t intel_vvp_snoop_get_num_good_fields(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the number of broken/invalid fields (counted from the broken flag in EOF packets in full variant)
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the number of broken fields register
 * \pre        instance is a valid intel_vvp_snoop_instance configured in full variant and
 *             was successfully initialized
 */
uint16_t intel_vvp_snoop_get_num_broken_fields(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the number of mismatch fields (mismatch between the image info and actual received field)
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the number of mismatch counter register
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint16_t intel_vvp_snoop_get_num_mismatch_fields(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the last number of lines
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the number of lines register (number of lines in the last field)
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint32_t intel_vvp_snoop_get_last_num_lines(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the last minimum width
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the last_min_width register (smnallest line in the last field)
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint32_t intel_vvp_snoop_get_last_min_width(intel_vvp_snoop_instance *instance);
 
/**
 * \brief Read the last maximum width
 *
 * \param[in]  instance, an intel_vvp_snoop_instance
 * \return     the value returned from a read to the last_max_width register (largest line in the last field)
 * \pre        instance is a valid intel_vvp_snoop_instance and was successfully initialized
 */
uint32_t intel_vvp_snoop_get_last_max_width(intel_vvp_snoop_instance *instance);
 
/**
 * \brief reset all counters
 *
 * \param[in]  instance, pointer to the intel_vvp_snoop_instance
 * \return     kIntelVvpCoreInstanceErr in case of errors, kIntelVvpCoreOk otherwise
 * \pre        instance is a valid intel_vvp_snoop_instance
 */
int intel_vvp_snoop_reset_counters(intel_vvp_snoop_instance* instance);
 
#ifdef __cplusplus
}
#endif /* __cplusplus */
 
#endif // __INTEL_VVP_SNOOP_H__