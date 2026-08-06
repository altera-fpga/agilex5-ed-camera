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
#include "altera_vvp_throttle.h"

namespace Hapi
{    
    using VvpThrottle = HapiItem<int, altera_vvp_throttle_instance, altera_vvp_throttle_init, {FPGA_CAPABILITY::altera_vvp_throttle}>;
    using VvpThrottlePtr = std::shared_ptr<VvpThrottle>;
}