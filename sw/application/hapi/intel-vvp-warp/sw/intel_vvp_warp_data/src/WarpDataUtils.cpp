/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpDataContext.h"
#include "WarpDataUtils.h"


namespace intel_vvp_warp
{


// Note: These values are the absolute maximum used by the RTL
// for framebuffer strides etc.
// Actual maximum image dimensions that could be processed correctly
// depend on several other factors

uint32_t max_frame_dim(const WarpMemMap& mem_map)
{
    uint32_t dim = 0;
    
    switch(mem_map)
    {
        case WarpMemMap::ESDTV:
            dim = 1024;
        break;                
        case WarpMemMap::EHDTV:
            dim = 2048;
        break;                
        case WarpMemMap::EUHDTV:
            dim = 4096;
        break;                
        case WarpMemMap::E8KUHDTV:
            dim = 8192;
        break;
        default:
        break;
    };

    return dim;
}

}