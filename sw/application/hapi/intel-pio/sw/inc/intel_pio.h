/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_PIO_H__
#define __INTEL_PIO_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "intel_pio_io.h"
#include "intel_pio_regs.h"

#define INTEL_PIO_PRODUCT_ID                           0x0210              ///< Addr Expander product ID
#define INTEL_PIO_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_PIO_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct intel_pio_instance_s
{
    void* base;
} intel_pio_instance;

int intel_pio_init(intel_pio_instance* instance, void* base);

uint32_t intel_pio_read(intel_pio_instance* instance);
void intel_pio_write(intel_pio_instance* instance, uint32_t value);

uint32_t intel_pio_get_direction(intel_pio_instance* instance);
void intel_pio_set_direction(intel_pio_instance* instance, uint32_t value);

uint32_t intel_pio_get_irq_mask(intel_pio_instance* instance);
void intel_pio_set_irq_mask(intel_pio_instance* instance, uint32_t value);

uint32_t intel_pio_get_edge_cap(intel_pio_instance* instance);
void intel_pio_set_edge_cap(intel_pio_instance* instance, uint32_t value);

void intel_pio_set_bit(intel_pio_instance* instance, uint32_t value);
void intel_pio_clear_bit(intel_pio_instance* instance, uint32_t value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __INTEL_PIO_H__ */
