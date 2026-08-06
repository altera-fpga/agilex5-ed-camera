/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
/**
 * \brief Definition of the intel_coredla_instance and associated functions
 *
 * Driver for the Video & Vision Processing Frame Buffer Intel FPGA IP
 *
 * \see Intel Video and Vision FPGA IP Suite User Guide
 * \see intel_vvp_core.h
 * \see intel_vvp_core_regs.h
 * \see intel_coredla_regs.h
 */

#ifndef __INTEL_COREDLA_H__
#define __INTEL_COREDLA_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "intel_vvp_core.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_COREDLA_PRODUCT_ID                           0x03b5u              ///< Frame buffer product ID

#define INTEL_COREDLA_REG_IORD(instance, reg)          INTEL_VVP_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< Frame buffer register read function
#define INTEL_COREDLA_REG_IOWR(instance, reg, value)   INTEL_VVP_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< Frame buffer register write function

typedef void (*interrupt_service_routine_signature)(int handle, void *data);


typedef struct intel_coredla_instance_s
{
    intel_vvp_core_instance core_instance;           ///< Base intel_vvp_core_instance
    uint8_t *_pDdrBase;

    struct _memmap
    {
        int _fd;
        void* _cpuPointer;
        size_t _sz;
    } memmap;
} intel_coredla_instance;	   

/**
 * \brief Initialise a frame buffer instance
 * 
 * Initialization function for a VVP frame buffer instance.
 * Attempts to initialize the fields of the frame buffer and its base core
 * 
 * \param[in]    instance, pointer to the intel_coredla_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return       kIntelVvpCoreOk in case of success,
 *               kIntelVvpCoreInstanceErr if instance is NULL
 *               kIntelVvpCoreVidErr if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               kIntelVvpCorePidErr if the product id of the core is not the frame buffer product id (0x0237)
 *               kIntelVvpVfbRegMapVersionErr if the register map is not supported
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 * \remarks      If already running, the frame buffer read side is stopped during initialization. It must be
 *               enabled, or re-enabled, for the frame buffer to output any data
 */
int intel_coredla_init(intel_coredla_instance* instance, intel_vvp_core_base base);

/**
 * \brief Register a function to run as the interrupt service routine
 * 
 * \param[in]    func, pointer to the interrupt service routine
 * \return       void
 */
void intel_coredla_RegisterISR(intel_coredla_instance* instance, interrupt_service_routine_signature func, void *data);

/**
 * \brief Write to CSR register
 * 
 * \param[in]    instance, pointer to the intel_coredla_instance 
 * \param[in]    addr, address of CSR register
 * \param[in]    data, register value to write
 * \return       void
 */
void intel_coredla_csr_write(intel_coredla_instance* instance, uint32_t addr, uint32_t data);

/**
 * \brief Read from CSR register
 * 
 * \param[in]    instance, pointer to the intel_coredla_instance 
 * \param[in]    addr, address of CSR register
 * \return       register value
 */
uint32_t intel_coredla_csr_read(intel_coredla_instance* instance, uint32_t addr);

/**
 * \brief Write data buffer to DDR
 * 
 * \param[in]    instance, pointer to the intel_coredla_instance 
 * \param[in]    addr, DDR address to fetch data from
 * \param[in]    lenght, size of data to fetch
 * \param[in]    data, pointer to data buffer to write from
 * \return       populated data from DDR
 */
void intel_coredla_ddr_write(intel_coredla_instance* instance, uint64_t addr, uint64_t length, const void *data);

/**
 * \brief Read data buffer from DDR
 * 
 * \param[in]    instance, pointer to the intel_coredla_instance 
 * \param[in]    addr, DDR address to fetch data from
 * \param[in]    lenght, size of data to fetch
 * \param[in]    data, pointer to data buffer to read into
 * \return       populated data from DDR
 */
void intel_coredla_ddr_read(intel_coredla_instance* instance, uint64_t addr, uint64_t length, void *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_COREDLA_H__
