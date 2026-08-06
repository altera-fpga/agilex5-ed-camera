/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_on_chip_mem_ii.h"

int intel_on_chip_mem_ii_init(intel_on_chip_mem_ii_instance* instance, intel_vvp_core_base base)
{
    int init_ret;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->core_instance.base = base;
    init_ret = kIntelVvpCoreOk;
    //init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_ON_CHIP_MEM_II_PRODUCT_ID);
    
    
    return init_ret;
}

void intel_on_chip_mem_ii_write(intel_on_chip_mem_ii_instance* instance, uint32_t addr, uint32_t data)
{
    INTEL_VVP_CORE_REG_IOWR(instance, addr, data);
}

uint32_t intel_on_chip_mem_ii_read(intel_on_chip_mem_ii_instance* instance, uint32_t addr)
{
    return INTEL_VVP_CORE_REG_IORD(instance, addr);
}

