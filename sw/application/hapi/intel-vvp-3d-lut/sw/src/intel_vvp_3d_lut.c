/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <stddef.h>
#include "intel_vvp_3d_lut.h"
#include "intel_vvp_3d_lut_regs.h"

int intel_vvp_3d_lut_init(intel_vvp_3d_lut_instance* instance, intel_vvp_core_base base)
{
    int init_ret;

    // Abort initialization and return kIntelVvp3dLutInstanceErr if instance is a null pointer
    if(instance == 0) return kIntelVvp3dLutInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_3D_LUT_PRODUCT_ID);
    
    if (kIntelVvp3dLutOk == init_ret)
    {
        instance->external_mode       = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_EXTERNAL_MODE);
        instance->pixels_per_clock    = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_PIXELS_IN_PARALLEL);
        
        instance->cpu_readable        = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_LUT_CPU_READABLE);
        instance->lut_alpha           = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_LUT_ALPHA);
        instance->lut_depth           = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_LUT_DEPTH);
        instance->lut_dimension       = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_LUT_DIMENSION);
        instance->lut_double_buffered = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_LUT_DOUBLE_BUFFERED);
        
        instance->input_depth         = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_INPUT_BPS);
        instance->output_depth        = (uint8_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONFIG_OUTPUT_BPS);

        instance->data_width          = (instance->lut_alpha == 0) ? (instance->lut_depth * 3) : (instance->lut_depth * 4); 
        instance->data_val_mask       = 0xFFFF >> (16 - instance->lut_depth);

        uint8_t mask_bits;
        switch (instance->lut_dimension)
        {
            case 9:
                mask_bits = 7;
                break;
            case 17:
                mask_bits = 10;
                break;
            case 33:
                mask_bits = 13;
                break;
            default: //65
                mask_bits = 16;
                break;
        }
        if (instance->lut_double_buffered)
        {
            ++mask_bits;
        }
        instance->ram_address_mask = ~(~0<<mask_bits);
    }

    return init_ret;
}
    
bool intel_vvp_3d_lut_get_double_buffered(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return (bool)(instance->lut_double_buffered);
}
  
uint8_t intel_vvp_3d_lut_get_lut_depth(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->lut_depth;
}

uint8_t intel_vvp_3d_lut_get_dimension(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->lut_dimension;
}

uint8_t intel_vvp_3d_lut_get_input_depth(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->input_depth;
}
  
uint8_t intel_vvp_3d_lut_get_output_depth(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->output_depth;
}
  
uint8_t intel_vvp_3d_lut_get_pixels_per_clock(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pixels_per_clock;
}
  
bool intel_vvp_3d_lut_get_alpha_channel(intel_vvp_3d_lut_instance* instance)
{
    if (instance == NULL) return false;

    return (bool)(instance->lut_alpha);
}

int intel_vvp_3d_lut_enable(intel_vvp_3d_lut_instance* instance, bool enable)
{
    if (instance == NULL) return kIntelVvp3dLutInstanceErr;

    uint32_t control = INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONTROL_REG_OFFSET);
    
    if(enable)
    {
        control |= (1 << INTEL_VVP_3D_LUT_CONTROL_ENABLE_OFST) & INTEL_VVP_3D_LUT_CONTROL_ENABLE_MSK;
    }
    else
    {
        control &= ~((1 << INTEL_VVP_3D_LUT_CONTROL_ENABLE_OFST) & INTEL_VVP_3D_LUT_CONTROL_ENABLE_MSK);
    }
    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_CONTROL_REG_OFFSET, control);
    
    return kIntelVvp3dLutOk;
}

int intel_vvp_3d_lut_buffer_select(intel_vvp_3d_lut_instance* instance, uint8_t buffer)
{
    uint32_t control;

    if (instance == NULL) return kIntelVvp3dLutInstanceErr;

    // Function fails if double buffering not enabled or buffer is out of range
    if(instance->lut_double_buffered && (buffer == 0 || buffer == 1))
    {
        control = INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_CONTROL_REG_OFFSET);
        
        control &= ~((1 << INTEL_VVP_3D_LUT_CONTROL_BUFFER_OFST) & INTEL_VVP_3D_LUT_CONTROL_BUFFER_MSK);
        control |= (buffer << INTEL_VVP_3D_LUT_CONTROL_BUFFER_OFST) & INTEL_VVP_3D_LUT_CONTROL_BUFFER_MSK;
            
        INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_CONTROL_REG_OFFSET, control);
        return kIntelVvp3dLutOk;
    }
    return kIntelVvp3dLutErrBuffer;
}

int intel_vvp_3d_lut_load(intel_vvp_3d_lut_instance* instance,
                          uint16_t r_idx, uint16_t g_idx, uint16_t b_idx, uint8_t buffer,
                          uint16_t r_val, uint16_t g_val, uint16_t b_val, uint16_t a_val)
{
    if (instance == NULL) return kIntelVvp3dLutInstanceErr;

    uint32_t ram_address = 0;
    uint32_t ram_index = 0;
    uint32_t r_offset;
    uint32_t g_offset;
    uint32_t b_offset;
    uint64_t data = 0;

    // check rgb index range range
    if(r_idx >= instance->lut_dimension || g_idx >= instance->lut_dimension || b_idx >= instance->lut_dimension)
    {
        return kIntelVvp3dLutErrIndexRange;
    }

    // buffer must be equal to zero unless double buffered - then it may be equal to 1
    if((buffer != 0) && (instance->lut_double_buffered == 0 || buffer != 1))
    {
        return kIntelVvp3dLutErrBuffer;
    }

    // Alpha value must be zero if not supported
    if((instance->lut_alpha == 0) && a_val != 0)
    {
        return kIntelVvp3dLutErrAlpha;
    }

    // Check if parameter values exceed max lut value
    if((a_val & instance->data_val_mask) != a_val ||
       (r_val & instance->data_val_mask) != r_val ||
       (g_val & instance->data_val_mask) != g_val ||
       (b_val & instance->data_val_mask) != b_val)
    {
        return kIntelVvp3dLutErrValueRange;
    }
    
    // Data order is ABGR
    data = a_val & instance->data_val_mask;
    data = (data << instance->lut_depth) | (b_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (g_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (r_val & instance->data_val_mask);

    // ram_index = (r % 2) + 2*(g % 2) + 4*(b % 2)
    ram_index = (r_idx & 1) | ((g_idx & 1) << 1) | ((b_idx & 1) << 2);
    
    r_offset = ((ram_index & 1) == 0) ? ((instance->lut_dimension + 1) >> 1) : ((instance->lut_dimension - 1) >> 1);
    g_offset = ((ram_index & 2) == 0) ? ((instance->lut_dimension + 1) >> 1) : ((instance->lut_dimension - 1) >> 1);
    b_offset = ((ram_index & 4) == 0) ? ((instance->lut_dimension + 1) >> 1) : ((instance->lut_dimension - 1) >> 1);
    
    // addr = (r >> 1) + ((g >> 1) * r_off) + ((b >> 1) * g_off * r_off) + (buf * b_off * g_off *r_off)
    // Factorised:
    ram_address = (r_idx >> 1) + (r_offset * (
                  (g_idx >> 1) + (g_offset * (
                  (b_idx >> 1) + (buffer * b_offset)))));

    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA1),
                             (uint32_t)(data & 0xFFFFFFFF));

    if(instance->data_width > 32)
    {
        INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA2),
                                 (uint32_t)(data >> 32));
    }
    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_CTRL),
                             ram_address | INTEL_VVP_3D_LUT_RAM_CTRL_WRITE);
    return kIntelVvp3dLutOk;
}

int intel_vvp_3d_lut_set_ram_ctrl(intel_vvp_3d_lut_instance* instance,
                                 uint8_t ram_index, uint32_t address, bool write_enable, bool auto_inc)
{
    if (instance == NULL) return kIntelVvp3dLutInstanceErr;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return kIntelVvp3dLutErrRamIndexRange;

    uint32_t write_data = address & instance->ram_address_mask;
    if (write_enable) write_data |= INTEL_VVP_3D_LUT_RAM_CTRL_WRITE;
    if (auto_inc)     write_data |= INTEL_VVP_3D_LUT_RAM_CTRL_AUTO_INC;

    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_CTRL),
                             write_data);

    return kIntelVvp3dLutOk;
}

int intel_vvp_3d_lut_set_ram_data_lower(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index, uint32_t data)
{
    if (instance == NULL) return kIntelVvp3dLutInstanceErr;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return kIntelVvp3dLutErrRamIndexRange;

    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA1),
                             data);

    return kIntelVvp3dLutOk;
}

int intel_vvp_3d_lut_set_ram_data_upper(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index, uint32_t data)
{
    if (instance == NULL) return kIntelVvp3dLutInstanceErr;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return kIntelVvp3dLutErrRamIndexRange;

    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA2),
                             data);

    return kIntelVvp3dLutOk;
}

uint32_t intel_vvp_3d_lut_get_ram_data_lower(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index)
{
    if (instance == NULL) return 0;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return 0;

    return (uint32_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA1));
}

uint32_t intel_vvp_3d_lut_get_ram_data_upper(intel_vvp_3d_lut_instance* instance,
                                       uint8_t ram_index)
{
    if (instance == NULL) return 0;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return 0;

    return (uint32_t)INTEL_VVP_3D_LUT_REG_IORD(instance,INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA2));
}

int intel_vvp_3d_lut_set_ram_data_rgb(intel_vvp_3d_lut_instance* instance,
                                     uint8_t ram_index,
                                     uint16_t r_val, uint16_t g_val, uint16_t b_val)
{
    uint64_t data = 0;

    if (instance == NULL) return kIntelVvp3dLutInstanceErr;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return kIntelVvp3dLutErrRamIndexRange;

    // Check if parameter values exceed max lut value
    if((r_val & instance->data_val_mask) != r_val ||
       (g_val & instance->data_val_mask) != g_val ||
       (b_val & instance->data_val_mask) != b_val)
    {
        return kIntelVvp3dLutErrValueRange;
    }
    
    // Data order is BGR
    data = (b_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (g_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (r_val & instance->data_val_mask);
    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA1),
                             (uint32_t)data);

    if(instance->data_width > 32)
    {
        INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA2),
                                 (uint32_t)(data >> 32));
    }

    return kIntelVvp3dLutOk;
}

int intel_vvp_3d_lut_set_ram_data_rgba(intel_vvp_3d_lut_instance* instance,
                                      uint8_t ram_index,
                                      uint16_t r_val, uint16_t g_val, uint16_t b_val, uint16_t a_val)
{
    uint64_t data = 0;

    if (instance == NULL) return kIntelVvp3dLutInstanceErr;
    if (ram_index >= INTEL_VVP_3D_LUT_NUMBER_OF_RAMS) return kIntelVvp3dLutErrRamIndexRange;

    // Check if parameter values exceed max lut value
    if((a_val & instance->data_val_mask) != a_val ||
       (r_val & instance->data_val_mask) != r_val ||
       (g_val & instance->data_val_mask) != g_val ||
       (b_val & instance->data_val_mask) != b_val)
    {
        return kIntelVvp3dLutErrValueRange;
    }
    
    // Data order is ABGR
    data = a_val & instance->data_val_mask;
    data = (data << instance->lut_depth) | (b_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (g_val & instance->data_val_mask);
    data = (data << instance->lut_depth) | (r_val & instance->data_val_mask);
    INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA1),
                             (uint32_t)data);

    if(instance->data_width > 32)
    {
        INTEL_VVP_3D_LUT_REG_IOWR(instance, INTEL_VVP_3D_LUT_RAM_REGISTER(ram_index,INTEL_VVP_3D_LUT_RAM_DATA2),
                                 (uint32_t)(data >> 32));
    }

    return kIntelVvp3dLutOk;
}

//eof
