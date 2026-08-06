/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#ifndef __WARP_DATA_HELPER_H__
#define __WARP_DATA_HELPER_H__

#include "intel_vvp_warp.h"
#include "WarpDataContext.h"
#include "WarpData.h"
#include <memory>

namespace intel_vvp_warp
{

namespace WarpDataHelper
{

using WarpDriverDataPtr = std::shared_ptr<intel_vvp_warp_data_t>;
	
WarpHwContextPtr GetHwContext(intel_vvp_warp_channel_t* ch);

WarpDriverDataPtr GenerateDriverData(const WarpDataContext& ctx, const WarpDataPtr user_data, const uint32_t coef_base_address, const uint32_t skip_ram_page);

float GetBlockCacheUtilization(const WarpDataContext& ctx, const WarpDataPtr user_data, const uint32_t engine);

} // WarpDataHelper

} // intel_vvp_warp

#endif /* __WARP_DATA_HELPER_H__ */