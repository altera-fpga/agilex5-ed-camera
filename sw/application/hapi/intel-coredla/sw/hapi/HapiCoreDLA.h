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
#include "intel_coredla.h"

namespace Hapi
{   
    using CoreDLA = HapiItem<int, intel_coredla_instance, intel_coredla_init, {FPGA_CAPABILITY::IntelCoreDLA}>;
    using CoreDLAPtr = std::shared_ptr<CoreDLA>;
}