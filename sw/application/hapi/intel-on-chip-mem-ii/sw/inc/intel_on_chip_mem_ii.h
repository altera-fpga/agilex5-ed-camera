/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
/**
 * \brief Definition of the intel_on_chip_mem_ii_instance and associated functions
 *
 * Driver for the Video & Vision Processing Frame Buffer Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_coredla_regs.h
 */

#ifndef __INTEL_ON_CHIP_MEM_II_H__
#define __INTEL_ON_CHIP_MEM_II_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_ON_CHIP_MEM_II_PRODUCT_ID                           0x03ecu              ///< IntelOnChipMemII,Intel On-Chip Memory II (RAM or ROM) product ID

typedef struct intel_on_chip_mem_ii_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
} intel_on_chip_mem_ii_instance;	   

/**
 * \brief Initialise a CoreDLA stream controller instance
 * 
 * Initialization function for a CoreDLA stream controller instance.
 * 
 * \param[in]    instance, pointer to the intel_on_chip_mem_ii_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the core dla stream controller product id (0x03BB)
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 * \remarks      If already running, the frame buffer read side is stopped during initialization. It must be
 *               enabled, or re-enabled, for the frame buffer to output any data
 */
int intel_on_chip_mem_ii_init(intel_on_chip_mem_ii_instance* instance, intel_vvp_core_base base);

/**
 * \brief Write to stream controller register
 * 
 * \param[in]    instance, pointer to the intel_on_chip_mem_ii_instance 
 * \param[in]    addr, address of CSR register
 * \param[in]    data, register value to write
 * \return       void
 */
void intel_on_chip_mem_ii_write(intel_on_chip_mem_ii_instance* instance, uint32_t addr, uint32_t data);

/**
 * \brief Read from stream controller register
 * 
 * \param[in]    instance, pointer to the intel_on_chip_mem_ii_instance 
 * \param[in]    addr, address of CSR register
 * \return       register value
 */
uint32_t intel_on_chip_mem_ii_read(intel_on_chip_mem_ii_instance* instance, uint32_t addr);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_ON_CHIP_MEM_II_H__
