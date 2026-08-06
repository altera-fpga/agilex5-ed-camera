/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "altera_vvp_throttle.h"
#include "altera_vvp_throttle_regs.h"
 
int altera_vvp_throttle_init(altera_vvp_throttle_instance *instance, intel_vvp_core_base base)
{
    int init_ret;
    uint8_t regmap_version;
 
    if (instance == NULL) return kIntelVvpCoreInstanceErr;
 
    init_ret = intel_vvp_core_init(&(instance->core_instance), base, ALTERA_VVP_THROTTLE_PRODUCT_ID);
    // Check the regmap version if the the intel_vvp_core_instance initialize without errors (ie, vendor_id and product_id validated properly)
    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < ALTERA_VVP_THROTTLE_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > ALTERA_VVP_THROTTLE_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kAlteraVvpThrottleRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        // Load the throttle compile-time configuration
        //instance->lite_mode      = (0 != ALTERA_VVP_THROTTLE_REG_IORD(instance, ALTERA_VVP_THROTTLE_LITE_MODE_REG));
        //instance->pip            = (uint8_t)ALTERA_VVP_THROTTLE_REG_IORD(instance, ALTERA_VVP_THROTTLE_PIXELS_IN_PARALLEL_REG);
    }

    return init_ret;
}



int altera_vvp_throttle_start(altera_vvp_throttle_instance* instance)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, ALTERA_VVP_THROTTLE_CONTROL_REG, 0x1);
    return 0;
}

int altera_vvp_throttle_stop(altera_vvp_throttle_instance* instance)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, ALTERA_VVP_THROTTLE_CONTROL_REG, 0x0);
    return 0;
}

int altera_vvp_throttle_set_clock_cycles_per_line(altera_vvp_throttle_instance* instance, uint32_t clock_cycles)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, ALTERA_VVP_THROTTLE_CLOCK_CYCLES_PER_LINE_REG, clock_cycles);
    return 0;
}

int altera_vvp_throttle_set_active_lines(altera_vvp_throttle_instance* instance, uint32_t num_lines)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, INTEL_VVP_CORE_IMG_INFO_HEIGHT_REG, num_lines);
    return 0;
}

int altera_vvp_throttle_set_v_blank_lines(altera_vvp_throttle_instance* instance, uint32_t num_lines)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, ALTERA_VVP_THROTTLE_NUL_V_BLANKING_LINES_REG, num_lines);
    return 0;
}

int altera_vvp_throttle_commit_writes(altera_vvp_throttle_instance* instance)
{
    ALTERA_VVP_THROTTLE_REG_IOWR(instance, ALTERA_VVP_THROTTLE_COMMIT_REG, 0x1);
    return 0;
}
 
#if 0
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

#endif /* 0 */