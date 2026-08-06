/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <cstdint>

namespace intel_vvp_warp
{

//-----------------------------------------------------------------------------
//Enums used in the API
//-----------------------------------------------------------------------------

//Image/Screen edge
enum EEdgeId
{
	ETop = 0,
	EBottom,
	ELeft,
	ERight,
	ETotalEdges
};

//Image/Screen corner
enum ECornerId
{
	ETopLeft = 0,
	ETopRight,
	EBottomLeft,
	EBottomRight,
	ETotalCorners,
};

//Image/Screen axis
enum EAxisId
{
	EHorizontal = 0,
	EVertical,
	ETotalAxis
};

//-----------------------------------------------------------------------------
//Typedefs used in the API
//-----------------------------------------------------------------------------

typedef float point_t[2];

struct bounding_rect_t
{
	float _x1;
	float _y1;
	float _x2;
	float _y2;
};

struct radial_t
{
	// Focus position
	point_t _focus;
	// Coefficients
	float _k1;
	float _k2;
};

using ITransformerPtr = std::shared_ptr<class ITransformer>;
using ITransformerPtrs = std::vector<ITransformerPtr>;

} // intel_vvp_warp
