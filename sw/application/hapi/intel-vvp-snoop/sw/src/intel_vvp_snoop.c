/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_snoop.h"
#include "intel_vvp_snoop_regs.h"
 
int intel_vvp_snoop_init(intel_vvp_snoop_instance *instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;
 
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
 
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_SNOOP_PRODUCT_ID);
    // Check the regmap version if the the intel_vvp_core_instance initialize without errors (ie, vendor_id and product_id validated properly)
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_SNOOP_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_SNOOP_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpSnoopRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Load the snoop compile-time configuration
        instance->lite_mode      = (0 != INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_LITE_MODE_REG));
        instance->pip            = (uint8_t)INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_PIXELS_IN_PARALLEL_REG);
    }
    return init_ret;
}
 
bool intel_vvp_snoop_get_lite_mode(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return false;
 
    return instance->lite_mode;
}
 
bool intel_vvp_snoop_get_debug_enabled(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return false;
 
    return true;
}
 
uint8_t intel_vvp_snoop_get_pixels_in_parallel(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0;
 
    return instance->pip;
}
 
bool intel_vvp_snoop_is_running(intel_vvp_snoop_instance *instance)
{
    uint32_t status_reg;
 
    if (instance == NULL) return false;
 
    status_reg = INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_STATUS_REG);
    return INTEL_VVP_SNOOP_GET_FLAG(status_reg, STATUS_RUNNING);
}
 
uint8_t intel_vvp_snoop_get_status(intel_vvp_snoop_instance *instance)
{
    uint8_t status_reg;
 
    if (instance == NULL) return 0xFF;
 
    status_reg = INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_STATUS_REG);
    return status_reg;
}
 
uint16_t intel_vvp_snoop_get_num_good_fields(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_NUM_GOOD_FIELDS_REG);
}
 
uint16_t intel_vvp_snoop_get_num_broken_fields(intel_vvp_snoop_instance *instance)
{
    if ((instance == NULL) || instance->lite_mode) return 0;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_NUM_BROKEN_FIELDS_REG);
}
 
uint16_t intel_vvp_snoop_get_num_mismatch_fields(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_NUM_MISMATCH_FIELDS_REG);
}
 
 
uint32_t intel_vvp_snoop_get_last_num_lines(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_LAST_NUM_LINES_REG);
}
 
 
uint32_t intel_vvp_snoop_get_last_min_width(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0x10000;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_LAST_MIN_WIDTH_REG);
}
 
uint32_t intel_vvp_snoop_get_last_max_width(intel_vvp_snoop_instance *instance)
{
    if (instance == NULL) return 0;
 
    return INTEL_VVP_SNOOP_REG_IORD(instance, INTEL_VVP_SNOOP_LAST_MAX_WIDTH_REG);
}
 
int intel_vvp_snoop_reset_counters(intel_vvp_snoop_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
 
    INTEL_VVP_SNOOP_REG_IOWR(instance, INTEL_VVP_SNOOP_RESET_COUNTERS_REG, 1);
    return kIntelVvpCoreOk;
}