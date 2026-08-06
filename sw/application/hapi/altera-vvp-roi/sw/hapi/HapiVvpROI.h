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
#include "altera_vvp_roi.h"

namespace Hapi
{    
    using VvpROI = HapiItem<int, altera_vvp_roi_instance, altera_vvp_roi_init, {FPGA_CAPABILITY::altera_vvp_roi}>;
    using VvpROIPtr = std::shared_ptr<VvpROI>;
}