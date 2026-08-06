/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

/**
 * \brief   Common read/write Nios accessor defines for the Intel VAB FPGA IP Suite
 *
 * Common defines functions to perform a direct read/write register access
 * on IP in the Video & Vision Processing Intel FPGA IP Suite
 * 
 */

#ifndef __INTEL_VAB_CORE_IO_H__
#define __INTEL_VAB_CORE_IO_H__

#include <io.h>
#include <stdint.h>

typedef uint32_t* intel_vab_core_base;  ///< the definition of the instance accessor, on Nios this is simply the base address of the core

/**
 * \brief Define for a Nios register read access on a vab_core instance.
 *
 * \param[in]  instance, pointer to a intel_vab_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \return     the 32-bit value read
 * \pre        reg must be a valid register offset for the given vab_core instance
 */
#define INTEL_VAB_CORE_REG_IORD(instance, reg)          IORD( ( (intel_vab_core_instance*)(instance) )->base, (reg))

/**
 * \brief Define for a Nios register write access on a vab_core instance.
 *
 * \param[in]  instance, pointer to a intel_vab_core_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \param[in]  value, the 32-bit value to write
 * \pre        reg must be a valid register offset for the given vab_core instance
 */
#define INTEL_VAB_CORE_REG_IOWR(instance, reg, value)   IOWR( ( (intel_vab_core_instance*)(instance) )->base, (reg), (value))

#endif  // __INTEL_VAB_CORE_IO_H__
