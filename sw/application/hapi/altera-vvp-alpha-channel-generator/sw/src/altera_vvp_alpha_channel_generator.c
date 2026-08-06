/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "altera_vvp_alpha_channel_generator.h"
#include "altera_vvp_alpha_channel_generator_regs.h"


int altera_vvp_alpha_channel_generator_init(altera_vvp_alpha_channel_generator_instance* instance, intel_vvp_core_base base)
{
    int  init_ret = kIntelVvpCoreOk;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->core_instance.base = base;

    instance->num_LUT_entries   = 1024U;
    instance->config_reg        = 0U;

    return init_ret;
}

int altera_vvp_alpha_channel_generator_set_config(altera_vvp_alpha_channel_generator_instance* instance, uint16_t pixel_start, uint16_t pixel_stop)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_WRITE_FIELD(instance->config_reg, (uint32_t)pixel_start, CONFIG_PIX_START);
    ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_WRITE_FIELD(instance->config_reg, (uint32_t)pixel_stop, CONFIG_PIX_STOP);
    ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_REG_IOWR(instance, ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_CONFIG_REG, instance->config_reg);

    return kIntelVvpCoreOk;
}

int altera_vvp_alpha_channel_generator_write_data(altera_vvp_alpha_channel_generator_instance* instance, uint32_t addr, uint16_t odd_alpha, uint16_t even_alpha )
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (addr < instance->num_LUT_entries) {
        uint32_t lutWrite = 0U;
        ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_WRITE_FIELD(lutWrite, (uint32_t)addr, LUT_WRITE_ADDRESS);
        ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_WRITE_FIELD(lutWrite, (uint32_t)odd_alpha, LUT_WRITE_ODD_PIXEL_ALPHA);
        ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_WRITE_FIELD(lutWrite, (uint32_t)even_alpha, LUT_WRITE_EVEN_PIXEL_ALPHA);
        ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_REG_IOWR(instance, ALTERA_VVP_ALPHA_CHANNEL_GENERATOR_LUT_WRITE_REG, lutWrite);
    } else {
        return kAlteraVvpAlphaChannelGeneratorOutOfBoundsErr;
    }

    return kIntelVvpCoreOk;
}
