/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_ADDR_SPAN_EXP_H__
#define __INTEL_ADDR_SPAN_EXP_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_addr_span_expander_io.h"
#include "intel_addr_span_expander_regs.h"

#define INTEL_ADDR_SPAN_EXP_PRODUCT_ID                           0x0212              ///< Addr Expander product ID
#define INTEL_ADDR_SPAN_EXP_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_ADDR_SPAN_EXP_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

//#define INTEL_ADDR_SPAN_EXP_REG_IORD(instance, reg)          IORD( ( (intel_addr_span_expander_instance*)(instance) )->base, (reg))
//#define INTEL_ADDR_SPAN_EXP_REG_IOWR(instance, reg, value)   IOWR( ( (intel_addr_span_expander_instance*)(instance) )->base, (reg), (value))

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct intel_addr_span_expander_instance_s
{
    void* base;
} intel_addr_span_expander_instance;

int intel_addr_span_expander_init(intel_addr_span_expander_instance* instance, void* base);

bool intel_addr_span_expander_set_window_address(intel_addr_span_expander_instance* instance, uint8_t window, uint64_t address);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_ADDR_SPAN_EXP_H__ */
