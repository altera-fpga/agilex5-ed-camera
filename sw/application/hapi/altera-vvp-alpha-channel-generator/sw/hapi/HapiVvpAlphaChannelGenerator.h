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
#include "altera_vvp_alpha_channel_generator.h"

namespace Hapi
{    
    using VvpAlphaChannelGenerator = HapiItem<int, altera_vvp_alpha_channel_generator_instance, altera_vvp_alpha_channel_generator_init, {FPGA_CAPABILITY::altera_vvp_alpha_gen}>;
    using VvpAlphaChannelGeneratorPtr = std::shared_ptr<VvpAlphaChannelGenerator>;
}