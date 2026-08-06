/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_addr_span_expander.h"
#include "intel_addr_span_expander_regs.h"

int intel_addr_span_expander_init(intel_addr_span_expander_instance* instance, void* base)
{
    if (instance == NULL) return 1;

    instance->base = base; // So complex!

    return 0;
}

bool intel_addr_span_expander_set_window_address(intel_addr_span_expander_instance* instance, uint8_t window, uint64_t address)
{
    if (instance == NULL) return false;

    INTEL_ADDR_SPAN_EXP_REG_IOWR(instance, LOWER_DWORD_FOR_WINDOW(window), address & 0xFFFFFFFF);
    INTEL_ADDR_SPAN_EXP_REG_IOWR(instance, UPPER_DWORD_FOR_WINDOW(window), address >> 32);

    return true;
}

