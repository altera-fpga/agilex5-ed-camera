/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_pip_conv.h"
#include "intel_vvp_pip_conv_regs.h"

int intel_vvp_pip_conv_init(intel_vvp_pip_conv_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_PIP_CONV_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_PIP_CONV_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_PIP_CONV_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpPipConvRegMapVersionErr;
        }
    }

    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_PIP_CONV_REG_IORD(instance, INTEL_VVP_PIP_CONV_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_PIP_CONV_REG_IORD(instance, INTEL_VVP_PIP_CONV_DEBUG_ENABLED_REG));
        if (!instance->lite_mode)
        {
            init_ret = kIntelVvpPipConvRegMapVersionErr; // Something wrong there
        }
    }

    return init_ret;
}

bool intel_vvp_pip_conv_get_lite_mode(intel_vvp_pip_conv_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_pip_conv_get_debug_enabled(intel_vvp_pip_conv_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

bool intel_vvp_pip_conv_is_running(intel_vvp_pip_conv_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_PIP_CONV_REG_IORD(instance, INTEL_VVP_PIP_CONV_STATUS_REG);
    return INTEL_VVP_PIP_CONV_GET_FLAG(status_reg, STATUS_RUNNING);
}

uint8_t intel_vvp_pip_conv_get_status(intel_vvp_pip_conv_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_PIP_CONV_REG_IORD(instance, INTEL_VVP_PIP_CONV_STATUS_REG);
    return status_reg;
}
