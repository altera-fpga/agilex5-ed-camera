/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef void* intel_vvp_remosaic_base_t;

typedef struct intel_vvp_remosaic_instance
{
    intel_vvp_remosaic_base_t base;
} intel_vvp_remosaic_instance_t;


int intel_vvp_remosaic_init_instance(intel_vvp_remosaic_instance_t* instance, intel_vvp_remosaic_base_t base);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
