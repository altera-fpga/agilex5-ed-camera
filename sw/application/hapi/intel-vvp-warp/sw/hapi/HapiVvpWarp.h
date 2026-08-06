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
#include "intel_vvp_warp.h"
#include <memory>

namespace Hapi
{
    using VvpWarp = HapiItem<int, intel_vvp_warp_instance, intel_vvp_warp_init_instance,
                        {FPGA_CAPABILITY::IntelOSWP, "9e05a122-eb37-4bfc-9b64-526fc784b5ae"}>;

    using VvpWarpPtr = std::shared_ptr<VvpWarp>;
}