/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_vvp_vfw.h"
#include "intel_vvp_vfw_regs.h"

int intel_vvp_vfw_init(intel_vvp_vfw_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint32_t regmap_version;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_VFW_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_VFW_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_VFW_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpVfwRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode        = (0 != INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_LITE_MODE_REG));
        instance->debug_enabled    = (0 != INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_DEBUG_ENABLED_REG));
        instance->max_height       = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_MAX_HEIGHT_REG);
        instance->max_width        = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_MAX_WIDTH_REG);
        instance->bps              = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BPS_REG);
        instance->num_color_planes = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_NUMBER_OF_COLOR_PLANES_REG);
        instance->pip              = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_PIXELS_IN_PARALLEL_REG);
        uint8_t packing            = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_PACKING_REG);

        instance->packing = kIntelVvpVfwInvalidPacking;
        if ((packing == kIntelVvpVfwPerfectPacking) || (packing == kIntelVvpVfwColorPacking) || (packing == kIntelVvpVfwPixelPacking))
        {
            instance->packing = (eIntelVvpVfwPacking)packing;
            // Stop the reader
            INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_RUN_REG, INTEL_VVP_VFW_STOP);
        }
        else
        {
            init_ret = kIntelVvpVfwRegMapVersionErr;
        }
    }
    return init_ret;
}

bool intel_vvp_vfw_get_lite_mode(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_vfw_get_debug_enabled(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint32_t intel_vvp_vfw_get_max_height(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

uint32_t intel_vvp_vfw_get_max_width(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint8_t intel_vvp_vfw_get_bps(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps;
}

uint8_t intel_vvp_vfw_get_number_of_color_planes(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_planes;
}

uint8_t intel_vvp_vfw_get_pixels_in_parallel(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

eIntelVvpVfwPacking intel_vvp_vfw_get_mem_word_packing(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return kIntelVvpVfwInvalidPacking;

    return instance->packing;
}

bool intel_vvp_vfw_is_running(intel_vvp_vfw_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_STATUS_REG);
    return INTEL_VVP_VFW_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_vfw_get_commit_status(intel_vvp_vfw_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_STATUS_REG);
    return INTEL_VVP_VFW_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint32_t intel_vvp_vfw_get_status(intel_vvp_vfw_instance* instance)
{
    uint8_t status_reg;

    if (instance == NULL) return 0xFF;

    status_reg = INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_STATUS_REG);
    return status_reg;
}

bool intel_vvp_vfw_is_buffer_available(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return false;

    return 0 != INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_AVAILABLE_REG);
}

uint32_t intel_vvp_vfw_get_buffer_write_count(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_WRITE_COUNT_REG);
}

uint32_t intel_vvp_vfw_get_buffer_start_addr(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_START_ADDRESS_REG);
}

bool intel_vvp_vfw_is_buffer_f1(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return false;

    return 0 != INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_F1_FLAG_REG);
}

uint32_t intel_vvp_vfw_get_buffer_width(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_WIDTH_REG);
}


uint32_t intel_vvp_vfw_get_buffer_height(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_HEIGHT_REG);
}

uint32_t intel_vvp_vfw_get_field_count(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || instance->lite_mode) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_FIELD_COUNT_REG);
}

int intel_vvp_vfw_acknowledge_buffer(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_BUFFER_ACKNOWLEDGE_REG, 1);

    return kIntelVvpCoreOk;
}

int intel_vvp_vfw_set_run_mode(intel_vvp_vfw_instance* instance, eIntelVvpVfwRunMode run_mode)
{
    if ((instance == NULL) || (run_mode == kIntelVvpVfwInvalidRunMode)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_RUN_REG, run_mode);

    return kIntelVvpCoreOk;
}

eIntelVvpVfwRunMode intel_vvp_vfw_get_run_mode(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpVfwInvalidRunMode;

    return (eIntelVvpVfwRunMode)INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_RUN_REG);
}

int intel_vvp_vfw_set_num_buffers(intel_vvp_vfw_instance* instance, uint32_t num_buffers)
{
    if ((instance == NULL) || (num_buffers == 0)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_NUM_BUFFERS_REG, num_buffers);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfw_get_num_buffers(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_NUM_BUFFERS_REG);
}

int intel_vvp_vfw_set_base_addr(intel_vvp_vfw_instance* instance, uint32_t base_addr)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_BUFFER_BASE_REG, base_addr);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfw_get_base_addr(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_BUFFER_BASE_REG);
}

int intel_vvp_vfw_set_inter_buffer_offset(intel_vvp_vfw_instance* instance, uint32_t inter_buffer_offset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_INTER_BUFFER_OFFSET_REG, inter_buffer_offset);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfw_get_inter_buffer_offset(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_INTER_BUFFER_OFFSET_REG);

}
int intel_vvp_vfw_set_inter_line_offset(intel_vvp_vfw_instance* instance, uint32_t inter_line_offset)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_INTER_LINE_OFFSET_REG, inter_line_offset);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfw_get_inter_line_offset(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_INTER_LINE_OFFSET_REG);
}


int intel_vvp_vfw_set_irq_line_threshold(intel_vvp_vfw_instance* instance, uint32_t line_threshold)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_IRQ_LINE_THRESHOLD_REG, line_threshold);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfw_get_irq_line_threshold(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_IRQ_LINE_THRESHOLD_REG);
}

int intel_vvp_vfw_overwrite_broken_fields(intel_vvp_vfw_instance* instance, bool overwrite_broken)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_OVERWRITE_BROKEN_REG, overwrite_broken ? 1 : 0);

    return kIntelVvpCoreOk;
}

bool intel_vvp_vfw_get_overwrite_broken_fields(intel_vvp_vfw_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return false;

    return 0 != INTEL_VVP_VFW_REG_IORD(instance, INTEL_VVP_VFW_OVERWRITE_BROKEN_REG);
}


int intel_vvp_vfw_commit_writes(intel_vvp_vfw_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFW_REG_IOWR(instance, INTEL_VVP_VFW_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}
