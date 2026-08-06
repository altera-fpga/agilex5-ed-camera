/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "intel_vvp_vfr.h"
#include "intel_vvp_vfr_regs.h"

int intel_vvp_vfr_init(intel_vvp_vfr_instance* instance, intel_vvp_core_base base)
{
    int init_ret;
    uint32_t regmap_version;
    uint8_t packing;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    init_ret = intel_vvp_core_init(&(instance->core_instance), base, INTEL_VVP_VFR_PRODUCT_ID);

    if (kIntelVvpCoreOk == init_ret)
    {
        regmap_version = intel_vvp_core_get_register_map_version(instance);
        if ((regmap_version < INTEL_VVP_VFR_MIN_SUPPORTED_REGMAP_VERSION) || (regmap_version > INTEL_VVP_VFR_MAX_SUPPORTED_REGMAP_VERSION))
        {
            init_ret = kIntelVvpVfrRegMapVersionErr;
        }
    }
    if (kIntelVvpCoreOk == init_ret)
    {
        instance->lite_mode        = (0 != INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_LITE_MODE_REG));
        instance->debug_enabled    = (0 != INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_DEBUG_ENABLED_REG));
        instance->max_buffer_sets  = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_MAX_BUFFER_SETS_REG);
        instance->max_height       = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_MAX_HEIGHT_REG);
        instance->max_width        = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_MAX_WIDTH_REG);
        instance->bps              = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BPS_REG);
        instance->num_color_planes = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_NUMBER_OF_COLOR_PLANES_REG);
        instance->pip              = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_PIXELS_IN_PARALLEL_REG);
        packing                    = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_PACKING_REG);

        instance->packing = kIntelVvpVfrInvalidPacking;
        if ((packing == kIntelVvpVfrPerfectPacking) || (packing == kIntelVvpVfrColorPacking) || (packing == kIntelVvpVfrPixelPacking))
        {
            instance->packing = (eIntelVvpVfrPacking)packing;
            // Stop the reader
            INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_RUN_REG, INTEL_VVP_VFR_STOP);
        }
        else
        {
            init_ret = kIntelVvpVfrRegMapVersionErr;
        }
    }
    return init_ret;
}

bool intel_vvp_vfr_get_lite_mode(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return false;

    return instance->lite_mode;
}

bool intel_vvp_vfr_get_debug_enabled(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return false;

    return instance->debug_enabled;
}

uint32_t intel_vvp_vfr_get_max_height(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_height;
}

uint32_t intel_vvp_vfr_get_max_width(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_width;
}

uint8_t intel_vvp_vfr_get_max_buffer_sets(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->max_buffer_sets;
}

uint8_t intel_vvp_vfr_get_bps(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->bps;
}

uint8_t intel_vvp_vfr_get_number_of_color_planes(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->num_color_planes;
}

uint8_t intel_vvp_vfr_get_pixels_in_parallel(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;

    return instance->pip;
}

eIntelVvpVfrPacking intel_vvp_vfr_get_mem_word_packing(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return kIntelVvpVfrInvalidPacking;

    return instance->packing;
}

bool intel_vvp_vfr_is_running(intel_vvp_vfr_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_STATUS_REG);
    return INTEL_VVP_VFR_GET_FLAG(status_reg, STATUS_RUNNING);
}

bool intel_vvp_vfr_get_commit_status(intel_vvp_vfr_instance* instance)
{
    uint32_t status_reg;

    if (instance == NULL) return false;

    status_reg = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_STATUS_REG);
    return INTEL_VVP_VFR_GET_FLAG(status_reg, STATUS_PENDING_COMMIT);
}

uint32_t intel_vvp_vfr_get_status(intel_vvp_vfr_instance* instance)
{
    uint8_t status_reg;

    if (instance == NULL) return 0xFF;

    status_reg = INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_STATUS_REG);
    return status_reg;
}

uint32_t intel_vvp_vfr_get_last_buffer_read_addr(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0xFFFFFFFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_LAST_BUFFER_READ_REG);
}

int intel_vvp_vfr_set_num_buffer_sets(intel_vvp_vfr_instance* instance, uint8_t num_sets)
{
    if ((instance == NULL) || (num_sets == 0) || (num_sets > instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_NUM_BUFFER_SETS_REG, num_sets);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_num_buffer_sets(intel_vvp_vfr_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_NUM_BUFFER_SETS_REG);
}

int intel_vvp_vfr_set_buffer_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrBufferMode buffer_mode)
{
    if ((instance == NULL) || (buffer_mode == kIntelVvpVfrInvalidBufferMode)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFFER_MODE_REG, buffer_mode);

    return kIntelVvpCoreOk;
}

eIntelVvpVfrBufferMode intel_vvp_vfr_get_buffer_mode(intel_vvp_vfr_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpVfrInvalidBufferMode;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFFER_MODE_REG);
}

int intel_vvp_vfr_set_starting_buffer_set(intel_vvp_vfr_instance* instance, uint8_t starting_buffer_set)
{
    if ((instance == NULL) || (starting_buffer_set >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_STARTING_BUFFER_SET_REG, starting_buffer_set);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_starting_buffer_set(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return 0;
    if (!instance->debug_enabled) return instance->max_buffer_sets;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_STARTING_BUFFER_SET_REG);
}

int intel_vvp_vfr_set_run_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrRunMode run_mode)
{
    if ((instance == NULL) || (run_mode == kIntelVvpVfrInvalidRunMode)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_RUN_REG, run_mode);

    return kIntelVvpCoreOk;
}

eIntelVvpVfrRunMode intel_vvp_vfr_get_run_mode(intel_vvp_vfr_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpVfrInvalidRunMode;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_RUN_REG);
}


int intel_vvp_vfr_set_fsync_mode(intel_vvp_vfr_instance* instance, eIntelVvpVfrFsyncMode sync_mode)
{
    if ((instance == NULL) || (sync_mode == kIntelVvpVfrInvalidFsyncMode)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_FSYNC_PULSE_MODE_REG, sync_mode);

    return kIntelVvpCoreOk;
}

eIntelVvpVfrFsyncMode intel_vvp_vfr_get_fsync_mode(intel_vvp_vfr_instance* instance)
{
    if ((instance == NULL) || !instance->debug_enabled) return kIntelVvpVfrInvalidFsyncMode;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_FSYNC_PULSE_MODE_REG);
}

int intel_vvp_vfr_set_bufset_base_addr(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t base_addr)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_BASE_ADDR_REG(bufset), base_addr);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_base_addr(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_BASE_ADDR_REG(bufset));
}


int intel_vvp_vfr_set_bufset_num_buffers(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t num_buffers)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_NUM_BUFFERS_REG(bufset), num_buffers);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_num_buffers(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_NUM_BUFFERS_REG(bufset));
}

int intel_vvp_vfr_set_bufset_inter_buffer_offset(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t inter_buffer_offset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_INTER_BUFFER_OFFSET_REG(bufset), inter_buffer_offset);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_inter_buffer_offset(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_INTER_BUFFER_OFFSET_REG(bufset));
}

int intel_vvp_vfr_set_bufset_inter_line_offset(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t inter_line_offset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_INTER_LINE_OFFSET_REG(bufset), inter_line_offset);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_inter_line_offset(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFFFFFFFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_INTER_LINE_OFFSET_REG(bufset));
}

int intel_vvp_vfr_set_bufset_height(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t height)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_HEIGHT_REG(bufset), height);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_height(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_HEIGHT_REG(bufset));
}

int intel_vvp_vfr_set_bufset_width(intel_vvp_vfr_instance* instance, uint8_t bufset, uint32_t width)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_WIDTH_REG(bufset), width);

    return kIntelVvpCoreOk;
}

uint32_t intel_vvp_vfr_get_bufset_width(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_WIDTH_REG(bufset));
}

int intel_vvp_vfr_set_bufset_interlace(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t interlace)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_INTERLACE_REG(bufset), interlace);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_bufset_interlace(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_INTERLACE_REG(bufset));
}

int intel_vvp_vfr_set_bufset_colorspace(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t colorspace)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_COLORSPACE_REG(bufset), colorspace);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_bufset_colorspace(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_COLORSPACE_REG(bufset));
}

int intel_vvp_vfr_set_bufset_subsampling(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t subsampling)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_SUBSAMPLING_REG(bufset), subsampling);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_bufset_subsampling(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_SUBSAMPLING_REG(bufset));
}

int intel_vvp_vfr_set_bufset_cositing(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t cositing)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_COSITING_REG(bufset), cositing);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_bufset_cositing(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_COSITING_REG(bufset));
}

int intel_vvp_vfr_set_bufset_bps(intel_vvp_vfr_instance* instance, uint8_t bufset, uint8_t bps)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_BPS_REG(bufset), bps);

    return kIntelVvpCoreOk;
}

uint8_t intel_vvp_vfr_get_bufset_bps(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_BPS_REG(bufset));
}

int intel_vvp_vfr_set_bufset_field_count(intel_vvp_vfr_instance* instance, uint8_t bufset, uint16_t field_count)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets)) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_BUFSET_FIELD_COUNT_REG(bufset), field_count);

    return kIntelVvpCoreOk;
}

uint16_t intel_vvp_vfr_get_bufset_field_count(intel_vvp_vfr_instance* instance, uint8_t bufset)
{
    if ((instance == NULL) || (bufset >= instance->max_buffer_sets) || !instance->debug_enabled) return 0xFFFF;

    return INTEL_VVP_VFR_REG_IORD(instance, INTEL_VVP_VFR_BUFSET_FIELD_COUNT_REG(bufset));
}

int intel_vvp_vfr_commit_writes(intel_vvp_vfr_instance* instance)
{
    if (instance == NULL) return kIntelVvpCoreInstanceErr;

    INTEL_VVP_VFR_REG_IOWR(instance, INTEL_VVP_VFR_COMMIT_REG, 1);

    return kIntelVvpCoreOk;
}
