/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_vab_core.h"
#include "intel_vab_core_regs.h"

eIntelVabCoreErrors intel_vab_core_init(intel_vab_core_instance *instance, intel_vab_core_base base, uint16_t expected_product_id)
{
    int init_ret;
    uint32_t read_reg;

    // Abort initialization and return kIntelVabCoreInstanceErr if instance is a null pointer
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    instance->base = base;
    read_reg = INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_VID_PID_REG);
    instance->vendor_id  = INTEL_VAB_CORE_READ_FIELD(read_reg, VID_PID_VENDOR_ID);
    instance->product_id = INTEL_VAB_CORE_READ_FIELD(read_reg, VID_PID_PRODUCT_ID);

    init_ret = kIntelVabCoreOk;
    if (INTEL_VAB_VENDOR_ID != instance->vendor_id)
    {
        // Complete initialization and return kIntelVabCoreVidErr if vendor mismatch
        init_ret = kIntelVabCoreVidErr;
    }
    else if (expected_product_id != instance->product_id)
    {
        // Complete initialization and return kIntelVabCorePidErr if vendor mismatch
        init_ret = kIntelVabCorePidErr;
    }
    if (kIntelVabCoreOk == init_ret)
    {
        // Proceed with a read to the version register if, and only if, the pid_vid register matches with expectations
        read_reg = INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_VERSION_REG);
        instance->qpds_major     = INTEL_VAB_CORE_READ_FIELD(read_reg, VERSION_QPDS_MAJOR);
        instance->qpds_update    = INTEL_VAB_CORE_READ_FIELD(read_reg, VERSION_QPDS_UPDATE);
        instance->qpds_patch     = INTEL_VAB_CORE_READ_FIELD(read_reg, VERSION_QPDS_PATCH);
        instance->regmap_version = INTEL_VAB_CORE_READ_FIELD(read_reg, VERSION_REGMAP_VERSION);
    }

    return init_ret;
}

uint16_t intel_vab_core_get_vendor_id(void *instance)
{
    if (instance == NULL) return 0xFFFF;

    return ((intel_vab_core_instance *)instance)->vendor_id;
}

uint16_t intel_vab_core_get_product_id(void *instance)
{
    if (instance == NULL) return 0xFFFF;

    return ((intel_vab_core_instance *)instance)->product_id;
}

uint8_t intel_vab_core_get_qpds_major(void *instance)
{
    if (instance == NULL) return 0xFF;
    return ((intel_vab_core_instance *)instance)->qpds_major;
}

uint8_t intel_vab_core_get_qpds_update(void *instance)
{
    if (instance == NULL) return 0xFF;

    return ((intel_vab_core_instance *)instance)->qpds_update;
}

uint8_t intel_vab_core_get_qpds_patch(void *instance)
{
    if (instance == NULL) return 0xFF;

    return ((intel_vab_core_instance *)instance)->qpds_patch;
}

uint8_t intel_vab_core_get_register_map_version(void *instance)
{
    if (instance == NULL) return 0xFF;

    return ((intel_vab_core_instance *)instance)->regmap_version;
}

uint32_t intel_vab_core_get_img_info_width(void *instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_WIDTH_REG);
}

uint32_t intel_vab_core_get_img_info_height(void *instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_HEIGHT_REG);
}

uint8_t intel_vab_core_get_img_info_interlace(void *instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_INTERLACE_REG);
}

uint8_t intel_vab_core_get_img_info_bps(void *instance)
{
    if (instance == NULL) return 0;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_BPS_REG);
}

uint8_t intel_vab_core_get_img_info_colorspace(void *instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_COLORSPACE_REG);
}

uint8_t intel_vab_core_get_img_info_subsampling(void *instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_SUBSAMPLING_REG);
}

uint8_t intel_vab_core_get_img_info_cositing(void *instance)
{
    if (instance == NULL) return 0xFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_COSITING_REG);
}

uint16_t intel_vab_core_get_img_info_field_count(void *instance)
{
    if (instance == NULL) return 0xFFFF;

    return INTEL_VAB_CORE_REG_IORD(instance, INTEL_VAB_CORE_IMG_INFO_FIELD_COUNT_REG);
}

eIntelVabCoreErrors intel_vab_core_set_img_info_width(void *instance, uint32_t width)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_WIDTH_REG, width);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_height(void *instance, uint32_t height)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_HEIGHT_REG, height);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_interlace(void *instance, uint8_t interlace)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_INTERLACE_REG, interlace);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_bps(void *instance, uint8_t bps)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_BPS_REG, bps);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_colorspace(void *instance, uint8_t colorspace)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_COLORSPACE_REG, colorspace);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_subsampling(void *instance, uint8_t subsampling)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_SUBSAMPLING_REG, subsampling);
    return kIntelVabCoreOk;
}

eIntelVabCoreErrors intel_vab_core_set_img_info_cositing(void *instance, uint8_t cositing)
{
    if (instance == NULL) return kIntelVabCoreInstanceErr;

    INTEL_VAB_CORE_REG_IOWR(instance, INTEL_VAB_CORE_IMG_INFO_COSITING_REG, cositing);
    return kIntelVabCoreOk;
}
