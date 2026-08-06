/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_pio.h"
#include "intel_pio_regs.h"

int intel_pio_init(intel_pio_instance* instance, void* base)
{
    if (instance == NULL) return 1;

    instance->base = base; // So complex!

    return 0;
}

bool intel_pio_set_window_address(intel_pio_instance* instance, uint8_t window, uint64_t address)
{
    if (instance == NULL) return false;


    return true;
}

uint32_t intel_pio_read(intel_pio_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_PIO_REG_IORD(instance, ALTERA_AVALON_PIO_DATA);
}

void intel_pio_write(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_DATA, value);
}

uint32_t intel_pio_get_direction(intel_pio_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_PIO_REG_IORD(instance, ALTERA_AVALON_PIO_DIRECTION);
}

void intel_pio_set_direction(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_DIRECTION, value);
}

uint32_t intel_pio_get_irq_mask(intel_pio_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_PIO_REG_IORD(instance, ALTERA_AVALON_PIO_IRQ_MASK);
}

void intel_pio_set_irq_mask(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_IRQ_MASK, value);
}

uint32_t intel_pio_get_edge_cap(intel_pio_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_PIO_REG_IORD(instance, ALTERA_AVALON_PIO_EDGE_CAP);
}

void intel_pio_set_edge_cap(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_EDGE_CAP, value);
}

void intel_pio_set_bit(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_SET_BIT, value);
}

void intel_pio_clear_bit(intel_pio_instance* instance, uint32_t value)
{
    if (instance == NULL) return;

    INTEL_PIO_REG_IOWR(instance, ALTERA_AVALON_PIO_CLEAR_BITS, value);
}
