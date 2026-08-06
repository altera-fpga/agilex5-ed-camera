/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_crs.h"
#include "intel_vvp_crs_regs.h"

 
int intel_vvp_crs_init(intel_vvp_crs_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;
    unsigned int s;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_CRS_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_CRS_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_CRS_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpCrsRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_DEBUG_ENABLED_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling444][kIntelVvpCrsSubsampling444] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_444_444_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling444][kIntelVvpCrsSubsampling422] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_444_422_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling444][kIntelVvpCrsSubsampling420] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_444_420_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling422][kIntelVvpCrsSubsampling444] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_422_444_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling422][kIntelVvpCrsSubsampling422] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_422_422_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling422][kIntelVvpCrsSubsampling420] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_422_420_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling420][kIntelVvpCrsSubsampling444] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_420_444_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling420][kIntelVvpCrsSubsampling422] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_420_422_REG));
        instance->supported_modes[kIntelVvpCrsSubsampling420][kIntelVvpCrsSubsampling420] =
            (0 != INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_ENABLE_420_420_REG));
        
        for (s = 0; s <  kIntelVvpCrsNumSubsamplings; ++s) 
        {
            instance->supported_modes[s][kIntelVvpCrsSubsamplingInvalid] = false;
            instance->supported_modes[kIntelVvpCrsSubsamplingInvalid][s] = false;
        }
    }

    return init_ret;
}
    

bool intel_vvp_crs_get_lite_mode(intel_vvp_crs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}


bool intel_vvp_crs_get_debug_enabled(intel_vvp_crs_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}


bool intel_vvp_crs_is_running(intel_vvp_crs_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_STATUS_REG);
    return INTEL_VVP_CRS_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_crs_get_commit_status(intel_vvp_crs_instance* instance)
{
    uint32_t status_reg;
    
    if ((instance == NULL) || instance->lite_mode) return false;
    
    status_reg = INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_STATUS_REG);
    return INTEL_VVP_CRS_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_crs_get_status(intel_vvp_crs_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_STATUS_REG);
    return status_reg;
}

bool intel_vvp_crs_is_conversion_supported(intel_vvp_crs_instance* instance,
                                           eIntelCrsSubsampling input_subsampling, eIntelCrsSubsampling output_subsampling)
{
    if (instance == NULL) return false;
    
    if ((input_subsampling < 0) || (output_subsampling < 0) ||
        (input_subsampling >= kIntelVvpCrsNumSubsamplings) ||
        (output_subsampling >= kIntelVvpCrsNumSubsamplings))  return false;
    
    return instance->supported_modes[input_subsampling][output_subsampling];
}

eIntelCrsSubsampling intel_vvp_crs_get_output_subsampling(intel_vvp_crs_instance* instance)
{
    eIntelCrsSubsampling output_subsampling = kIntelVvpCrsSubsamplingInvalid;
    if ((instance != NULL) && instance->debug_enabled)
    {
        uint8_t output_subs = (uint8_t)INTEL_VVP_CRS_REG_IORD(instance, INTEL_VVP_CRS_OUTPUT_SUBSAMPLING_REG);
        switch (output_subs)
        {
            case INTEL_VVP_CRS_SUBSAMPLING_444: output_subsampling = kIntelVvpCrsSubsampling444; break;
            case INTEL_VVP_CRS_SUBSAMPLING_422: output_subsampling = kIntelVvpCrsSubsampling422; break;
            case INTEL_VVP_CRS_SUBSAMPLING_420: output_subsampling = kIntelVvpCrsSubsampling420; break;
            default: output_subsampling = kIntelVvpCrsSubsamplingInvalid;
        }
    }
    return output_subsampling;
}


int intel_vvp_crs_set_output_subsampling(intel_vvp_crs_instance* instance, eIntelCrsSubsampling output_subsampling)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if ((output_subsampling != kIntelVvpCrsSubsampling444) &&
        (output_subsampling != kIntelVvpCrsSubsampling422) &&
        (output_subsampling != kIntelVvpCrsSubsampling420)) return kIntelVvpCrsParamErr;
    
    INTEL_VVP_CRS_REG_IOWR(instance, INTEL_VVP_CRS_OUTPUT_SUBSAMPLING_REG, output_subsampling);
    
    return kIntelVvpCoreOk;
}

int intel_vvp_crs_commit_writes(intel_vvp_crs_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CRS_REG_IOWR(instance, INTEL_VVP_CRS_COMMIT_REG, 1);
    return kIntelVvpCoreOk;
}