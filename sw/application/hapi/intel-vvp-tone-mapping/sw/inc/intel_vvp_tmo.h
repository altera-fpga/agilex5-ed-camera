/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef _INTEL_VVP_TMO_H_
#define _INTEL_VVP_TMO_H_

#include <stdint.h>
#include <stdbool.h>
#include "intel_vvp_tmo_io.h"
#include "intel_vvp_core.h"
#include "intel_vvp_core_regs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define INTEL_VVP_TMO_PRODUCT_ID                           0x0163u              /* TMO product ID */

// Normalized region of interest
typedef struct
{
    float _x;
    float _y;
    float _width;
    float _height;
} intel_vvp_tmo_roi;

typedef struct intel_vvp_tmo_instance
{
	intel_vvp_tmo_base_t base;
    bool _bypass;
    uint32_t _width;
    uint32_t _height;
    uint32_t _pixels_in_parallel;
    intel_vvp_tmo_roi _region_of_interest;
    bool _enable_region_of_interest;
    bool _outside_region_of_interest;
} intel_vvp_tmo_instance_t;

int intel_vvp_tmo_init_instance(intel_vvp_tmo_instance_t* instance, intel_vvp_tmo_base_t base);
void intel_vvp_tmo_set_bypass(intel_vvp_tmo_instance_t* instance, uint32_t bypass);
void intel_vvp_tmo_set_resolution(intel_vvp_tmo_instance_t* instance, uint32_t width, uint32_t height);
void intel_vvp_tmo_set_volume(intel_vvp_tmo_instance_t* instance, uint32_t volume);
void intel_vvp_tmo_set_threshold(intel_vvp_tmo_instance_t* instance, uint32_t threshold);

uint32_t intel_vvp_tmo_get_bypass(intel_vvp_tmo_instance_t* instance);
uint32_t intel_vvp_tmo_get_volume(intel_vvp_tmo_instance_t* instance);
uint32_t intel_vvp_tmo_get_threshold(intel_vvp_tmo_instance_t* instance);

uint32_t intel_vvp_tmo_get_debug(intel_vvp_tmo_instance_t* instance);
void intel_vvp_tmo_set_debug(intel_vvp_tmo_instance_t* instance, uint32_t val);

bool intel_vvp_tmo_set_region_of_interest(intel_vvp_tmo_instance_t* instance, intel_vvp_tmo_roi roi);
bool intel_vvp_tmo_enable_region_of_interest(intel_vvp_tmo_instance_t* instance, bool enable);
bool intel_vvp_tmo_set_region_of_interest_outside(intel_vvp_tmo_instance_t* instance, bool outside);
bool intel_vvp_tmo_get_region_of_interest(intel_vvp_tmo_instance_t* instance, intel_vvp_tmo_roi* roi, bool* enabled, bool* outside);

#ifdef INTEL_VVP_TMO_ENABLE_LOGGING
#include <stdio.h>
#define INTEL_VVP_TMO_LOG(...)	printf(__VA_ARGS__)
#else
#define INTEL_VVP_TMO_LOG(...)
#endif /*INTEL_VVP_TMO_ENABLE_LOGGING*/


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _INTEL_VVP_TMO_H_ */
