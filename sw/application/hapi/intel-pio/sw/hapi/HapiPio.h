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
#include "intel_pio.h"

namespace Hapi
{
    using Pio = HapiItem<int,
                            intel_pio_instance,
                            intel_pio_init,
                            {FPGA_CAPABILITY::IntelParallelIO}>;
    using PioPtr = std::shared_ptr<Pio>;
}
