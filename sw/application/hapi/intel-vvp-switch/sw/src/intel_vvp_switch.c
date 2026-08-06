/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_vvp_switch.h"
#include "intel_vvp_switch_regs.h"


int intel_vvp_switch_init(intel_vvp_switch_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_SWITCH_PRODUCT_ID);
    
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_SWITCH_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_SWITCH_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpSwitchRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->intf_type = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_INTF_TYPE_REG);
        instance->debug_enabled = (0 != INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_DEBUG_ENABLED_REG));
        instance->uninterrupted_inputs = (0 != INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_UNINTERRUPTED_INPUTS_REG));
        instance->auto_consume = (0 != INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_AUTO_CONSUME_REG));
        instance->num_inputs = (uint8_t)INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_NUM_INPUTS_REG);
        instance->num_outputs = (uint8_t)INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_NUM_OUTPUTS_REG);
        instance->use_treadies = (0 != INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_USE_TREADIES_REG));
        instance->crash_switch = (0 != INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_CRASH_SWITCH_REG));
        if ((instance->num_inputs > INTEL_VVP_SWITCH_MAX_NUM_INPUTS) || (instance->num_outputs > INTEL_VVP_SWITCH_MAX_NUM_OUTPUTS) ||
            ((instance->intf_type != kIntelVvpSwitchIntfFull) && (instance->intf_type != kIntelVvpSwitchIntfLite) && (instance->intf_type != kIntelVvpSwitchIntfFr)))
        {
            init_ret = kIntelVvpSwitchRegMapVersionErr;
        }
    }

    return init_ret;
}
 
eIntelVvpSwitchIntfType intel_vvp_switch_get_intf_type(intel_vvp_switch_instance* instance)
{
    if (instance == NULL) return kIntelVvpSwitchIntfInvalid;
    
    return instance->intf_type;
}

bool intel_vvp_switch_get_debug_enabled(intel_vvp_switch_instance* instance)
{
    if (instance == NULL) return false;
    
    return instance->debug_enabled;
}

bool intel_vvp_switch_has_uninterrupted_inputs(intel_vvp_switch_instance *instance)
{
    if (instance == NULL) return false;
    
    return instance->uninterrupted_inputs;
}

bool intel_vvp_switch_has_auto_consume(intel_vvp_switch_instance *instance)
{
    if (instance == NULL) return false;
    
    return instance->auto_consume;
}

uint8_t intel_vvp_switch_get_num_inputs(intel_vvp_switch_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->num_inputs;
}

uint8_t intel_vvp_switch_get_num_outputs(intel_vvp_switch_instance* instance)
{
    if (instance == NULL) return 0;
    
    return instance->num_outputs;
}

bool intel_vvp_switch_is_crash_switch(intel_vvp_switch_instance *instance)
{
    if (instance == NULL) return false;
    
    return instance->crash_switch;
}

bool intel_vvp_switch_uses_tready(intel_vvp_switch_instance *instance)
{
    if (instance == NULL) return false;
    
    return instance->use_treadies;
}

bool intel_vvp_switch_is_running(intel_vvp_switch_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_STATUS_REG);
    return INTEL_VVP_SWITCH_GET_FLAG(status_reg, STATUS_RUNNING);
}


bool intel_vvp_switch_get_commit_status(intel_vvp_switch_instance* instance)
{
    uint32_t status_reg;
    
    if (instance == NULL) return false;
    
    status_reg = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_STATUS_REG);
    return INTEL_VVP_SWITCH_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_switch_get_status(intel_vvp_switch_instance *instance)
{
    uint8_t status_reg;
    
    if (instance == NULL) return 0xFF;
    
    status_reg = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_STATUS_REG);
    return status_reg;
}

int intel_vvp_switch_set_input_config(intel_vvp_switch_instance *instance, uint8_t input, eIntelVvpSwitchInputConfig input_config)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    if (input >= instance->num_inputs) return kIntelVvpSwitchParameterErr;
    
    if ((input_config != kIntelVvpSwitchInputDisabled) && (input_config != kIntelVvpSwitchInputConsumed) && (input_config != kIntelVvpSwitchInputEnabled))
    {
        return kIntelVvpSwitchParameterErr;
    }
    
    INTEL_VVP_SWITCH_REG_IOWR(instance, INTEL_VVP_SWITCH_INPUT_CONTROL_REG(input), input_config);
    
    return kIntelVvpCoreOk;
}

eIntelVvpSwitchInputConfig intel_vvp_switch_get_input_config(intel_vvp_switch_instance *instance, uint8_t input)
{
    uint32_t input_config;
    
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpSwitchInputConfigInvalid;
    
    if (!instance->debug_enabled || (input >= instance->num_inputs)) return kIntelVvpSwitchInputConfigInvalid;
    
    input_config = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_INPUT_CONTROL_REG(input)) & 0x3;
    
    if ((input_config == kIntelVvpSwitchInputDisabled) || (input_config == kIntelVvpSwitchInputConsumed))
        return (eIntelVvpSwitchInputConfig)input_config;
    
    return kIntelVvpSwitchInputEnabled;
}

int intel_vvp_switch_set_output_config(intel_vvp_switch_instance *instance, uint8_t output, bool enabled, uint8_t input)
{
    uint32_t output_config;
    
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    if (output >= instance->num_outputs) return kIntelVvpSwitchParameterErr;
    
    // Overwrite enabled if the input is invalid
    int ret = kIntelVvpCoreOk;
    if (enabled && (input >= instance->num_inputs))
    {
        enabled = false;
        ret = kIntelVvpSwitchParameterErr;
    }
    
    output_config = 0u;
    if (enabled)
    {
        INTEL_VVP_SWITCH_WRITE_FIELD(output_config, input, OUTPUT_CONTROL_INPUT);
        INTEL_VVP_SWITCH_SET_FLAG(output_config, OUTPUT_CONTROL_ENABLED);
    }
    INTEL_VVP_SWITCH_REG_IOWR(instance, INTEL_VVP_SWITCH_OUTPUT_CONTROL_REG(output), output_config);

    return ret;
}
    
int intel_vvp_switch_get_output_config(intel_vvp_switch_instance *instance, uint8_t output)
{
    uint32_t output_config;
    bool enabled;
    uint8_t input;
    
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;
    
    if (output >= instance->num_outputs) return kIntelVvpSwitchParameterErr;
    
    output_config = INTEL_VVP_SWITCH_REG_IORD(instance, INTEL_VVP_SWITCH_OUTPUT_CONTROL_REG(output));
    
    enabled = INTEL_VVP_SWITCH_GET_FLAG(output_config, OUTPUT_CONTROL_ENABLED);
    input   = INTEL_VVP_SWITCH_READ_FIELD(output_config, OUTPUT_CONTROL_INPUT);
    if (!enabled || (input >= instance->num_inputs))
    {
        return -1;
    }
    
    return (int)input;
    
    
}

int intel_vvp_switch_commit_writes(intel_vvp_switch_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_SWITCH_REG_IOWR(instance, INTEL_VVP_SWITCH_COMMIT_REG, 1);
    return kIntelVvpCoreOk;
}