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
#include "intel_addr_span_expander.h"

namespace Hapi
{
    using AddrSpanExpander = HapiItem<int,
                            intel_addr_span_expander_instance,
                            intel_addr_span_expander_init,
                            {FPGA_CAPABILITY::IntelAddressSpanExtCtrl}>;
    using AddrSpanExpanderPtr = std::shared_ptr<AddrSpanExpander>;
}
