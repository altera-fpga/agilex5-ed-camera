/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_core.h"

#include "intel_vvp_core_regs.h"

eIntelVvpCoreErrors intel_vvp_core_init(intel_vvp_core_instance *instance, intel_vvp_core_base base, uint16_t expected_product_id)
{
    int init_ret;
    uint32_t read_reg;

    // Abort initialization and return kIntelVvpCoreInstanceErr if instance is a null pointer
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    instance->base = base;
    read_reg = INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_VID_PID_REG);
    instance->vendor_id  = INTEL_VVP_CORE_READ_FIELD(read_reg, VID_PID_VENDOR_ID);
    instance->product_id = INTEL_VVP_CORE_READ_FIELD(read_reg, VID_PID_PRODUCT_ID);

    init_ret = kIntelVvpCoreOk;
    if (INTEL_VVP_VENDOR_ID != instance->vendor_id)
    {
        // Complete initialization and return kIntelVvpCoreVidErr if vendor mismatch
        init_ret = kIntelVvpCoreVidErr;
    } else if (expected_product_id != instance->product_id)
    {
        // Complete initialization and return kIntelVvpCorePidErr if vendor mismatch
        init_ret = kIntelVvpCorePidErr;
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Proceed with a read to the version register if, and only if, the pid_vid register matches with expectations
        read_reg = INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_VERSION_REG);
        instance->qpds_major     = INTEL_VVP_CORE_READ_FIELD(read_reg, VERSION_QPDS_MAJOR);
        instance->qpds_update    = INTEL_VVP_CORE_READ_FIELD(read_reg, VERSION_QPDS_UPDATE);
        instance->qpds_patch     = INTEL_VVP_CORE_READ_FIELD(read_reg, VERSION_QPDS_PATCH);
        instance->regmap_version = INTEL_VVP_CORE_READ_FIELD(read_reg, VERSION_REGMAP_VERSION);
    }

    return init_ret;
}

uint16_t intel_vvp_core_get_vendor_id(void *instance)
{
    if (instance == NULL) return 0;

    return ((intel_vvp_core_instance *)instance)->vendor_id;
}

uint16_t intel_vvp_core_get_product_id(void *instance)
{
    if (instance == NULL) return 0;

    return ((intel_vvp_core_instance *)instance)->product_id;
}

uint8_t intel_vvp_core_get_qpds_major(void *instance)
{
    if (instance == NULL) return 0;
    return ((intel_vvp_core_instance *)instance)->qpds_major;
}

uint8_t intel_vvp_core_get_qpds_update(void *instance)
{
    if (instance == NULL) return 0;

    return ((intel_vvp_core_instance *)instance)->qpds_update;
}

uint8_t intel_vvp_core_get_qpds_patch(void *instance)
{
    if (instance == NULL) return 0;

    return ((intel_vvp_core_instance *)instance)->qpds_patch;
}

uint8_t intel_vvp_core_get_register_map_version(void *instance)
{
    if (instance == NULL) return 0;

    return ((intel_vvp_core_instance *)instance)->regmap_version;
}

uint32_t intel_vvp_core_get_img_info_width(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_WIDTH_REG);
}

uint32_t intel_vvp_core_get_img_info_height(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_HEIGHT_REG);
}

uint8_t intel_vvp_core_get_img_info_interlace(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_INTERLACE_REG);
}

eIntelVvpCoreErrors intel_vvp_core_get_img_info_bps_code(void *instance, bool *fp_flag, uint8_t *bps_val)
{
    uint32_t read_reg;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if ((fp_flag == NULL) || (bps_val == NULL)) return kIntelVvpCoreNullPtrErr;

    read_reg = INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_BPS_CODE_REG);
    *fp_flag = INTEL_VVP_CORE_READ_FIELD(read_reg, IMG_INFO_BPS_CODE_FP_FLAG);
    *bps_val = INTEL_VVP_CORE_READ_FIELD(read_reg, IMG_INFO_BPS_CODE_VAL);
    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_core_get_img_info_colorspace(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_COLORSPACE_REG);
}

uint8_t intel_vvp_core_get_img_info_subsampling(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_SUBSAMPLING_REG);
}

uint8_t intel_vvp_core_get_img_info_cositing(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CORE_REG_IORD(instance, INTEL_VVP_CORE_IMG_INFO_COSITING_REG);
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_width(void *instance, uint32_t width)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_WIDTH_REG, width);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_height(void *instance, uint32_t height)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_HEIGHT_REG, height);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_interlace(void *instance, uint8_t interlace)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_INTERLACE_REG, interlace);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_bps_code(void *instance, uint8_t fp_flag, uint8_t bps_val)
{
    uint32_t write_reg;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    write_reg = 0;
    INTEL_VVP_CORE_WRITE_FIELD(write_reg, fp_flag, IMG_INFO_BPS_CODE_FP_FLAG);
    INTEL_VVP_CORE_WRITE_FIELD(write_reg, bps_val, IMG_INFO_BPS_CODE_VAL);
    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_BPS_CODE_REG, write_reg);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_colorspace(void *instance, uint8_t colorspace)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_COLORSPACE_REG, colorspace);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_subsampling(void *instance, uint8_t subsampling)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_SUBSAMPLING_REG, subsampling);
    return kIntelVvpCoreOk;
}

eIntelVvpCoreErrors intel_vvp_core_set_img_info_cositing(void *instance, uint8_t cositing)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CORE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_COSITING_REG, cositing);
    return kIntelVvpCoreOk;
}
