/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_axi2cv.h"
#include "intel_axi2cv_regs.h"

int intel_axi2cv_init(intel_axi2cv_instance* instance, intel_vab_core_base base)
{
    int init_ret;

    init_ret = intel_vab_core_init(&(instance->core_instance), base, INTEL_AXI2CV_PRODUCT_ID);

    if (0 == init_ret)
    {
       instance->_can_debug = 0;
    }

    return init_ret;
}

bool intel_axi2cv_get_debug_enabled(intel_axi2cv_instance* instance)
{
    if (instance != NULL)
        return instance->_can_debug;
    return 0;
}

void intel_axi2cv_set_output_mode(intel_axi2cv_instance* instance, unsigned int bank_sel,
                                bool interlaced,
                                unsigned int sample_count, unsigned int f0_line_count, unsigned int f1_line_count,
                                unsigned int h_front_porch, unsigned int h_sync_length, unsigned int h_blanking,
                                unsigned int v_front_porch, unsigned int v_sync_length, unsigned int v_blanking,
                                unsigned int f0_v_front_porch, unsigned int f0_v_sync_length, unsigned int f0_v_blanking,
                                unsigned int active_picture_line, unsigned int f0_v_rising, unsigned int field_rising, unsigned int field_falling,
                                bool h_sync_polarity, bool v_sync_polarity)
{
    if (instance != NULL)
    {
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_BANK_SELECT, bank_sel);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VALID, false);

       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_CONTROL, ((interlaced ? 1 : 0) << CVO_INTERLACED_OUTPUT));

       // Dimensions
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_SAMPLE_COUNT, sample_count);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_LINE_COUNT, f0_line_count);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F1_LINE_COUNT, f1_line_count);

       // Blanking
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_HORIZONTAL_FRONT_PORCH, h_front_porch);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_HORIZONTAL_SYNC_LENGTH, h_sync_length);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_HORIZONTAL_BLANKING, h_blanking);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VERTICAL_FRONT_PORCH, v_front_porch);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VERTICAL_SYNC_LENGTH, v_sync_length);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VERTICAL_BLANKING, v_blanking);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_FRONT_PORCH, f0_v_front_porch);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_SYNC_LENGTH, f0_v_sync_length);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_BLANKING, f0_v_blanking);

       // Active data start
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_ACTIVE_PICTURE_LINE, active_picture_line);

       // Field toggle parameterization
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_RISING, f0_v_rising);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_FIELD_RISING, field_rising);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_FIELD_FALLING, field_falling);

       // h_sync/v_sync polarity
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_HSYNC_POLARITY, h_sync_polarity);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VSYNC_POLARITY, v_sync_polarity);

       // Revalidate the bank
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VALID, true);
    }
}

void intel_axi2cv_set_output_mode_sdi(intel_axi2cv_instance* instance, unsigned int bank_sel,
                                bool interlaced,
                                unsigned int sample_count, unsigned int f0_line_count, unsigned int f1_line_count,
                                unsigned int h_blanking,
                                unsigned int v_blanking,
                                unsigned int f0_v_blanking,
                                unsigned int active_picture_line, unsigned int f0_v_rising, unsigned int field_rising, unsigned int field_falling,
                                unsigned int vid_std, unsigned int vpid_byte1, unsigned int vpid_byte2, unsigned int vpid_byte3, unsigned int vpid_byte4)
{
    if (instance != NULL)
    {
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_BANK_SELECT, bank_sel);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VALID, false);

       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_CONTROL, ((interlaced ? 1 : 0) << CVO_INTERLACED_OUTPUT));

       // Dimensions
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_SAMPLE_COUNT, sample_count);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_LINE_COUNT, f0_line_count);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F1_LINE_COUNT, f1_line_count);

       // Blanking
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_HORIZONTAL_BLANKING, h_blanking);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VERTICAL_BLANKING, v_blanking);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_BLANKING, f0_v_blanking);

       // Active data start
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_ACTIVE_PICTURE_LINE, active_picture_line);

       // Field toggle parameterization
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_F0_VERTICAL_RISING, f0_v_rising);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_FIELD_RISING, field_rising);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_FIELD_FALLING, field_falling);

       // VPID
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_STANDARD, vid_std);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VPID_BYTE1, vpid_byte1);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VPID_BYTE2, vpid_byte2);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VPID_BYTE3, vpid_byte3);
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VPID_BYTE4, vpid_byte4);

       // Revalidate the bank
       INTEL_AXI2CV_REG_IOWR(instance, INTEL_AXI2CV_MODEX_VALID, true);
    }
}
