/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <stdint.h>

namespace intel_vvp_warp
{

class ITransformer
{
public:
	virtual ~ITransformer() {}
	virtual void Transform(float& x, float& y, uint32_t idx = 0) = 0;
};


} // namespace intel_vvp_warp
