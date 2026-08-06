/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "altera_vvp_roi.h"
#include "altera_vvp_roi_regs.h"

// Forward declarations of local functions
static void program_region_of_interest(altera_vvp_roi_instance* instance);

int altera_vvp_roi_init(altera_vvp_roi_instance* instance, intel_vvp_core_base base)
{
    int  init_ret = kIntelVvpCoreOk;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->core_instance.base = base;
    instance->_control_reg = 0;
    instance->_enable = false;
    instance->_width = 0U;
    instance->_height = 0U;
    instance->_region_of_interest._x = 0.0F;
    instance->_region_of_interest._y = 0.0F;
    instance->_region_of_interest._width = 0.0F;
    instance->_region_of_interest._height = 0.0F;

    return init_ret;
}

int altera_vvp_roi_set_enable(altera_vvp_roi_instance* instance, bool enable)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->_enable = enable;
    if(instance->_enable)
    {
        program_region_of_interest(instance);
        ALTERA_VVP_ROI_SET_FLAG(instance->_control_reg, CONTROL_ENABLE);
    }
    else
    {
        ALTERA_VVP_ROI_CLEAR_FLAG(instance->_control_reg, CONTROL_ENABLE);
    }
    ALTERA_VVP_ROI_REG_IOWR(instance, ALTERA_VVP_ROI_CONTROL_REG, instance->_control_reg);

    return kIntelVvpCoreOk;
}

int altera_vvp_roi_set_mode(altera_vvp_roi_instance* instance, uint8_t mode)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    ALTERA_VVP_ROI_WRITE_FIELD(instance->_control_reg, mode, CONTROL_MODE);
    ALTERA_VVP_ROI_REG_IOWR(instance, ALTERA_VVP_ROI_CONTROL_REG, instance->_control_reg);

    return kIntelVvpCoreOk;
}

int altera_vvp_roi_set_resolution(altera_vvp_roi_instance* instance, uint32_t width, uint32_t height)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->_width = width;
    instance->_height = height;

    program_region_of_interest(instance);

    return kIntelVvpCoreOk;
}

int altera_vvp_roi_set_roi(altera_vvp_roi_instance* instance, 
                            altera_vvp_roi roi)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if ((roi._x < 0.0f) || (roi._y < 0.0f) || (roi._width < 0.0f) || (roi._height < 0.0f))
        return kAlteraVvpROIValueErr;
        
    if ((roi._x + roi._width) > 1.0f)
        return kAlteraVvpROIValueErr;

    if ((roi._y + roi._height) > 1.0f)
        return kAlteraVvpROIValueErr;

    instance->_region_of_interest = roi;

    if (instance->_enable)
    {
        program_region_of_interest(instance);
    }

    return kIntelVvpCoreOk;
}      

typedef struct 
{
    uint32_t _x;
    uint32_t _y;
    uint32_t _width;
    uint32_t _height;
} altera_vvp_roi_pixels;

static altera_vvp_roi_pixels get_roi_pixels(altera_vvp_roi_instance* instance)
{
    if (!instance)
        return (altera_vvp_roi_pixels){0};

    altera_vvp_roi* roi = &instance->_region_of_interest;
    altera_vvp_roi_pixels roi_pixels;
    roi_pixels._x = (uint32_t)(roi->_x * instance->_width + 0.5f);
    roi_pixels._width  = (uint32_t)(roi->_width * instance->_width + 0.5f);
    roi_pixels._y      = (uint32_t)(roi->_y * instance->_height + 0.5f);
    roi_pixels._height = (uint32_t)(roi->_height * instance->_height + 0.5f);

    return roi_pixels;
}

static void program_region_of_interest(altera_vvp_roi_instance* instance)
{
    if (instance)
    {
        altera_vvp_roi_pixels roi_pixels = get_roi_pixels(instance);
        uint32_t x0 = roi_pixels._x;
        uint32_t x1 = x0 + roi_pixels._width;
        uint32_t y0 = roi_pixels._y;
        uint32_t y1 = y0 + roi_pixels._height;

        if(x1 >= instance->_width)
        {
            x1 = instance->_width-1;
        }
        if(y1 >= instance->_height)
        {
            y1 = instance->_height-1;
        }

        uint32_t h_roi_reg = 0x0;
        ALTERA_VVP_ROI_WRITE_FIELD(h_roi_reg, x0, PIX_START);
        ALTERA_VVP_ROI_WRITE_FIELD(h_roi_reg, x1, PIX_STOP);
        ALTERA_VVP_ROI_REG_IOWR(instance, ALTERA_VVP_ROI_H_ROI_REG, h_roi_reg);

        uint32_t v_roi_reg = 0x0;
        ALTERA_VVP_ROI_WRITE_FIELD(v_roi_reg, y0, PIX_START);
        ALTERA_VVP_ROI_WRITE_FIELD(v_roi_reg, y1, PIX_STOP);
        ALTERA_VVP_ROI_REG_IOWR(instance, ALTERA_VVP_ROI_V_ROI_REG, v_roi_reg);
    }
}
