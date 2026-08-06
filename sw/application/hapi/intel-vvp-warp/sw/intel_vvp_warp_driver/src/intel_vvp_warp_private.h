/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_VVP_WARP_PRIVATE_H__
#define __INTEL_VVP_WARP_PRIVATE_H__


#include <stdint.h>
#include "intel_vvp_warp.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define INTEL_VVP_WARP_CACHE_SIZE_MAX		(0x400)

typedef enum engine_scan_pattern
{
	EBLOCK = 0,
	EMEGABLOCK,
	ERASTER
} engine_scan_pattern_t;


typedef enum engine_mesh_resolution
{
    EMESH_8x8 = 0,
    EMESH_16x16 = 1,
    EMESH_32x32 = 2
    
} engine_mesh_resolution_t;


typedef enum engine_mesh_precision
{
    EMESH_4_FRACT_BITS = 0,
    EMESH_3_FRACT_BITS = 3,
    
} engine_mesh_precision_t;


typedef enum intel_vvp_warp_output_bounce
{
	EBOUNCE_NONE = 0,
	EBOUNCE_BLOCK,
	EBOUNCE_RASTER,
	EBOUNCE_CHANNEL
} intel_vvp_warp_output_bounce_t;


typedef enum intel_vvp_warp_mem_map
{
	ESDTV = 0,
	EHDTV,
	EUHDTV,
    E8KUHDTV
} intel_vvp_warp_mem_map_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INTEL_VVP_WARP_PRIVATE_H__ */
