/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_cpm.h"
#include "intel_vvp_cpm_regs.h"

int intel_vvp_cpm_init(intel_vvp_cpm_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_CPM_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_CPM_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_CPM_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpColorPlaneManagerRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode = (0 != INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_LITE_MODE_REG));
        instance->debug_enabled = (0 != INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_DEBUG_ENABLED_REG));

        instance->num_planes_in = (uint8_t)INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_NUM_PLANES_IN_REG);
        instance->num_planes_out = (uint8_t)INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_NUM_PLANES_OUT_REG);
        if ((instance->num_planes_in > INTEL_VVP_CPM_MAX_PLANES)  || (instance->num_planes_out > INTEL_VVP_CPM_MAX_PLANES)) {
            init_ret = kIntelVvpColorPlaneManagerRegMapVersionErr;
        }
    }

    return init_ret;
}


bool intel_vvp_cpm_get_lite_mode(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}


bool intel_vvp_cpm_get_debug_enabled(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint8_t intel_vvp_cpm_get_bits_per_sample(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_BPS_REG);
}

uint8_t intel_vvp_cpm_get_pixels_in_parallel(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_PIXELS_IN_PARALLEL_REG);
}

uint8_t intel_vvp_cpm_get_num_planes_in(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_planes_in;
}

uint8_t intel_vvp_cpm_get_num_planes_out(intel_vvp_cpm_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_planes_out;
}

bool intel_vvp_cpm_is_running(intel_vvp_cpm_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_STATUS_REG);
    return INTEL_VVP_CPM_GET_FLAG(status_reg, STATUS_RUNNING);
}


bool intel_vvp_cpm_get_commit_status(intel_vvp_cpm_instance* instance)
{
    uint32_t status_reg;

    if ((instance == NULL) || instance->lite_mode) return false;

    status_reg = INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_STATUS_REG);
    return INTEL_VVP_CPM_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint8_t intel_vvp_cpm_get_status(intel_vvp_cpm_instance *instance)
{
    uint8_t status_reg;

    if (instance == NULL) return 0xFF;

    status_reg = INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_STATUS_REG);
    return status_reg;
}

int intel_vvp_cpm_set_plane_padding(intel_vvp_cpm_instance* instance, uint8_t plane, uint16_t padding_value)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    if (plane >= instance->num_planes_out) return kIntelVvpColorPlaneManagerParameterErr;

    INTEL_VVP_CPM_REG_IOWR(instance, INTEL_VVP_CPM_PADDING_0_REG + plane, padding_value);
    return kIntelVvpCoreOk;
}

int intel_vvp_cpm_set_padding(intel_vvp_cpm_instance* instance, const intel_vvp_cpm_padding padding)
{
    uint8_t i;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    if (padding == NULL) return kIntelVvpCoreNullPtrErr;

    for (i = 0; i < instance->num_planes_out; ++i)
    {
        INTEL_VVP_CPM_REG_IOWR(instance, INTEL_VVP_CPM_PADDING_0_REG + i, padding[i]);
    }

    return kIntelVvpCoreOk;
}


uint16_t intel_vvp_cpm_get_plane_padding(intel_vvp_cpm_instance* instance, uint8_t plane)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0;
    if (plane >= instance->num_planes_out) return 0;

    return INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_PADDING_0_REG + plane);
}

int intel_vvp_cpm_get_padding(intel_vvp_cpm_instance* instance, intel_vvp_cpm_padding padding)
{
    uint8_t i;

    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpCoreInstanceErr;
    if (padding == NULL) return kIntelVvpCoreNullPtrErr;

    for (i = 0; i < instance->num_planes_out; ++i)
    {
        padding[i] = INTEL_VVP_CPM_REG_IORD(instance, INTEL_VVP_CPM_PADDING_0_REG + i);
    }
    for (i = instance->num_planes_out; i < INTEL_VVP_CPM_MAX_PLANES; ++i)
    {
        padding[i] = 0;
    }

    return kIntelVvpCoreOk;
}


int intel_vvp_cpm_commit_writes(intel_vvp_cpm_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_CPM_REG_IOWR(instance, INTEL_VVP_CPM_COMMIT_REG, 1);
    return kIntelVvpCoreOk;
}
