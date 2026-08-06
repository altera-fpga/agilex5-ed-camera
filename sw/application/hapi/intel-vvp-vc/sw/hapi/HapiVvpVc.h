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
#include "intel_vvp_vc.h"

namespace Hapi
{
    using VvpVc = HapiItem<int, intel_vvp_vc_instance, intel_vvp_vc_init, {FPGA_CAPABILITY::intel_vvp_vignette_correction}>;
    using VvpVcPtr = std::shared_ptr<VvpVc>;
} // namespace Hapi
