/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "HapiItem.h"
#include "intel_on_chip_mem_ii.h"

namespace Hapi
{   
    using HapiOnChipMemII = HapiItem<int, intel_on_chip_mem_ii_instance, intel_on_chip_mem_ii_init, {FPGA_CAPABILITY::IntelOnChipMemII}>;
    using HapiOnChipMemIIPtr = std::shared_ptr<HapiOnChipMemII>;
}