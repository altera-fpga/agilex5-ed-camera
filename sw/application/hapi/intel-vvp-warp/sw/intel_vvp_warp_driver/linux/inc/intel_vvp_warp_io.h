/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef _INTEL_VVP_WARP_IO_H_
#define _INTEL_VVP_WARP_IO_H_

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef void* intel_vvp_warp_base_t;

#ifdef HAPI

#include "HapiRegisterAccess.h"

#define INTEL_VVP_WARP_REG_IORD(x)			HapiReadRegister((instance->base), (x))

#ifdef INTEL_VVP_WARP_ENABLE_LOGGING
#define INTEL_VVP_WARP_REG_IOWR(x,y) {\
	printf("W: 0x%08" PRIx32 "\tV:0x%08" PRIx32 "\n",((uint32_t)(x))*4,(uint32_t)y); \
	HapiWriteRegister((instance->base), (x), (y)); \
}
#else
#define INTEL_VVP_WARP_REG_IOWR(x,y)		HapiWriteRegister((instance->base), (x), (y))
#endif /*INTEL_VVP_WARP_ENABLE_LOGGING*/

/* #define INTEL_VVP_WARP_REG_IOWR(x,y)		HapiWriteRegister((instance->base), (x), (y)) */
#define INTEL_VVP_WARP_REG_IOWR_QUIET(x, y)	HapiWriteRegister((instance->base), (x), (y))

#endif /* HAPI */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _INTEL_VVP_WARP_IO_H_ */

