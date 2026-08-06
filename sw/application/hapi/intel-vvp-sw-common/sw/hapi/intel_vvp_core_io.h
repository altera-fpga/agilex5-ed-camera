/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief   Common read/write Nios accessor defines for the Intel VVP FPGA IP Suite
 *
 * Common defines functions to perform a direct read/write register access
 * on IP in the Video & Vision Processing Intel FPGA IP Suite
 * 
 */

#ifndef __INTEL_VVP_CORE_IO_H__
#define __INTEL_VVP_CORE_IO_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef void* intel_vvp_core_base;  

#ifdef HAPI

#include "HapiRegisterAccess.h"

/**
 * \brief Define for a Nios register read access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_vvp_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \return     the 32-bit value read
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define INTEL_VVP_CORE_REG_IORD(instance, reg)          HapiReadRegister( ((intel_vvp_core_instance*)instance)->base, (reg))

/**
 * \brief Define for a Nios register write access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_vvp_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \param[in]  value, the 32-bit value to write
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define INTEL_VVP_CORE_REG_IOWR(instance, reg, value)   HapiWriteRegister( ((intel_vvp_core_instance*)instance)->base, (reg), (value))

#endif /* HAPI */

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif  // __INTEL_VVP_CORE_IO_H__