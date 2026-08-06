/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_exposure_fusion.h"
#include "intel_vvp_exposure_fusion_regs.h"

#include <stdio.h>


int intel_vvp_exposure_fusion_init(intel_vvp_exposure_fusion_instance* instance, intel_vvp_core_base base)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->core_instance.base = base;
    instance->core_instance.product_id = INTEL_VVP_EXPOSURE_FUSION_PRODUCT_ID;

    return kIntelVvpCoreOk;
}

int intel_vvp_exposure_fusion_set_run_mode(intel_vvp_exposure_fusion_instance* instance, eIntelVvpExposureFusionMode mode)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_EXPOSURE_FUSION_REG_IOWR(instance, INTEL_VVP_EXPOSURE_FUSION_MODE_REG, (uint32_t)mode);

    return kIntelVvpCoreOk;
}

eIntelVvpExposureFusionMode intel_vvp_exposure_fusion_get_run_mode(intel_vvp_exposure_fusion_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_EXPOSURE_FUSION_REG_IORD(instance, INTEL_VVP_EXPOSURE_FUSION_MODE_REG);
}

int intel_vvp_exposure_fusion_set_black_level(intel_vvp_exposure_fusion_instance* instance, uint16_t black_level)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_EXPOSURE_FUSION_REG_IOWR(instance, INTEL_VVP_EXPOSURE_FUSION_BLACK_LEVEL_REG, black_level);

    return kIntelVvpCoreOk;
}

uint16_t intel_vvp_exposure_fusion_get_black_level(intel_vvp_exposure_fusion_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_EXPOSURE_FUSION_REG_IORD(instance, INTEL_VVP_EXPOSURE_FUSION_BLACK_LEVEL_REG);
}

int intel_vvp_exposure_fusion_set_exposure_ratio(intel_vvp_exposure_fusion_instance* instance, uint32_t ratio)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_EXPOSURE_FUSION_REG_IOWR(instance, INTEL_VVP_EXPOSURE_FUSION_EXPOSURE_RATIO_REG, ratio);

    return kIntelVvpCoreOk;
}
uint32_t intel_vvp_exposure_fusion_get_exposure_ratio(intel_vvp_exposure_fusion_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_EXPOSURE_FUSION_REG_IORD(instance, INTEL_VVP_EXPOSURE_FUSION_EXPOSURE_RATIO_REG);
}

int intel_vvp_exposure_fusion_set_threshold(intel_vvp_exposure_fusion_instance* instance, uint16_t threshold)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_EXPOSURE_FUSION_REG_IOWR(instance, INTEL_VVP_EXPOSURE_FUSION_THRESHOLD_REG, threshold);

    return kIntelVvpCoreOk;
}
uint16_t intel_vvp_exposure_fusion_get_threshold(intel_vvp_exposure_fusion_instance* instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VVP_EXPOSURE_FUSION_REG_IORD(instance, INTEL_VVP_EXPOSURE_FUSION_THRESHOLD_REG);
}