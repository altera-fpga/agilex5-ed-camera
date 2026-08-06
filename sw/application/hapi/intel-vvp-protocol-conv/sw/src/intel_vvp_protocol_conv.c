/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_protocol_conv.h"

#include <assert.h>

int intel_vvp_protocol_conv_init(intel_vvp_protocol_conv_instance *instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_PROTOCOL_CONV_PRODUCT_ID);
    // Check the regmap version if the the intel_vvp_core_instance initialize without errors (ie, vendor_id and product_id validated properly)
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_PROTOCOL_CONV_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_PROTOCOL_CONV_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpProtocolConvRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->conv_mode  = INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_CONVERSION_MODE_REG);
        instance->debug_enabled  = (0 != INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_DEBUG_ENABLED_REG));
        // Initialize the control register and leave the core in a stopped state
        instance->ctrl = 0;
        INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, INTEL_VVP_PROTOCOL_CONV_CTRL_REG, instance->ctrl);
        if ((instance->conv_mode < kIntelVvpProtocolConvVipToVvpLite) || (instance->conv_mode > kIntelVvpProtocolConvVvpFullToVvpLite))
        {
            instance->conv_mode = kIntelVvpProtocolConvInvalid;
            init_ret = kIntelVvpProtocolConvRegMapVersionErr;
        }
    }

    return init_ret;
}

// Core specific
eIntelVvpProtocolConvConversionMode intel_vvp_protocol_conv_get_conversion_mode(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return kIntelVvpProtocolConvInvalid;

    return instance->conv_mode;
}

bool intel_vvp_protocol_conv_get_debug_enabled(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_protocol_conv_get_status(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_STATUS_REG);
}

bool intel_vvp_protocol_conv_is_running(intel_vvp_protocol_conv_instance *instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_STATUS_REG);
    return INTEL_VVP_PROTOCOL_CONV_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_protocol_conv_has_received_field(intel_vvp_protocol_conv_instance *instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_STATUS_REG);
    return INTEL_VVP_PROTOCOL_CONV_GET_FLAG(status_reg, STATUS_FIELD_RECEIVED);
}

bool intel_vvp_protocol_conv_was_last_field_broken(intel_vvp_protocol_conv_instance *instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_STATUS_REG);
    // The broken field bit is undefined until a first field has been received after reset with a VIP input (and will be 0 for VVP LITE/FULL inputs as expected)
    // So AND the field_received flag with the broken_field flag to guarantee 0 is returned in all cases when the first field was not received
    return INTEL_VVP_PROTOCOL_CONV_GET_FLAG(status_reg, STATUS_FIELD_RECEIVED) && INTEL_VVP_PROTOCOL_CONV_GET_FLAG(status_reg, STATUS_BROKEN_FIELD);
}


uint16_t intel_vvp_protocol_conv_get_field_count(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return 0xFFFF;

    // With VVP full input, the field count register is set from the content of the input EOP packet and will be undefined until this point
    if ((instance->conv_mode == kIntelVvpProtocolConvVvpFullToVip) || (instance->conv_mode == kIntelVvpProtocolConvVvpFullToVvpLite))
    {
        if (!intel_vvp_protocol_has_received_field(instance))
        {
            return 0;
        }
    }
    return INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_FIELD_COUNT_REG);
}

uint32_t intel_vvp_protocol_conv_get_vip_width(intel_vvp_protocol_conv_instance *instance)
{
    if ((instance == NULL) ||
        ((instance->conv_mode != kIntelVvpProtocolConvVipToVvpLite) && (instance->conv_mode != kIntelVvpProtocolConvVipToVvpFull)))
    {
        return 0;
    }

    return INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_VIP_WIDTH_REG);
}

uint32_t intel_vvp_protocol_conv_get_vip_height(intel_vvp_protocol_conv_instance *instance)
{
    if ((instance == NULL) ||
        ((instance->conv_mode != kIntelVvpProtocolConvVipToVvpLite) && (instance->conv_mode != kIntelVvpProtocolConvVipToVvpFull)))
    {
        return 0;
    }

    return INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_VIP_HEIGHT_REG);
}

uint8_t intel_vvp_protocol_conv_get_vip_interlace(intel_vvp_protocol_conv_instance *instance)
{
    if ((instance == NULL) ||
        ((instance->conv_mode != kIntelVvpProtocolConvVipToVvpLite) && (instance->conv_mode != kIntelVvpProtocolConvVipToVvpFull)))
    {
        return 0xFF;
    }

    return INTEL_VVP_PROTOCOL_CONV_REG_IORD(instance, INTEL_VVP_PROTOCOL_CONV_VIP_INTERLACE_REG);
}

int intel_vvp_protocol_conv_enable(intel_vvp_protocol_conv_instance* instance, bool enabled)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (enabled) {
        INTEL_VVP_PROTOCOL_CONV_SET_FLAG(instance->ctrl, CTRL_GO);
    } else {
        INTEL_VVP_PROTOCOL_CONV_CLEAR_FLAG(instance->ctrl, CTRL_GO);
    }

    INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, INTEL_VVP_PROTOCOL_CONV_CTRL_REG, instance->ctrl);
    return kIntelVvpCoreOk;
}

int intel_vvp_protocol_conv_start(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_PROTOCOL_CONV_SET_FLAG(instance->ctrl, CTRL_GO);
    INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, INTEL_VVP_PROTOCOL_CONV_CTRL_REG, instance->ctrl);
    return kIntelVvpCoreOk;
}

int intel_vvp_protocol_conv_stop(intel_vvp_protocol_conv_instance *instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_PROTOCOL_CONV_CLEAR_FLAG(instance->ctrl, CTRL_GO);
    INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, INTEL_VVP_PROTOCOL_CONV_CTRL_REG, instance->ctrl);
    return kIntelVvpCoreOk;
}

int intel_vvp_protocol_conv_reset_field_count(intel_vvp_protocol_conv_instance *instance)
{
    if ((instance == NULL) ||
        (instance->conv_mode == kIntelVvpProtocolConvVvpFullToVvpLite) ||
        (instance->conv_mode == kIntelVvpProtocolConvVvpFullToVip)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_PROTOCOL_CONV_REG_IOWR(instance, INTEL_VVP_PROTOCOL_CONV_FIELD_COUNT_RESET_REG, 1); // Any write value would do
    return kIntelVvpCoreOk;
}
