/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef _INTEL_ADDR_SPAN_EXP_IO_H_
#define _INTEL_ADDR_SPAN_EXP_IO_H_

#include <asm/io.h>
#include <linux/types.h>

typedef void __iomem *intel_addr_span_expander_base_t;

/**
 * \brief Define for a driver register read access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_addr_span_expander_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \return     the 32-bit value read
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define INTEL_ADDR_SPAN_EXP_REG_IORD(instance, reg)          ioread32( (void __iomem *)( (uint32_t __iomem *)( (intel_addr_span_expander_instance*)(instance) )->base + (reg) ) )

/**
 * \brief Define for a driver register write access on a vvp_core instance.
 *
 * \param[in]  instance, pointer to a intel_addr_span_expander_instance structure (or derived structure)
 * \param[in]  reg, register offset from the base of the register map
 * \param[in]  value, the 32-bit value to write
 * \pre        reg must be a valid register offset for the given vvp_core instance
 */
#define INTEL_ADDR_SPAN_EXP_REG_IOWR(instance, reg, value)   iowrite32( (value), (void __iomem *)((uint32_t __iomem *)( (intel_addr_span_expander_instance*)(instance) )->base + (reg) ) )

#endif  // _INTEL_ADDR_SPAN_EXP_IO_H_
