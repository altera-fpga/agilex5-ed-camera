/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef _INTEL_VVP_BLC_IO_H_
#define _INTEL_VVP_BLC_IO_H_

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef void* intel_vvp_blc_base_t;

#ifdef HAPI

#include "HapiRegisterAccess.h"

#define INTEL_VVP_BLC_REG_IORD(instance, reg) HapiReadRegister((instance->base), (reg))
#define INTEL_VVP_BLC_REG_IOWR(instance, reg, value) HapiWriteRegister((instance->base), (reg), (value))

#endif /* HAPI */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _INTEL_VVP_BLC_IO_H_ */
