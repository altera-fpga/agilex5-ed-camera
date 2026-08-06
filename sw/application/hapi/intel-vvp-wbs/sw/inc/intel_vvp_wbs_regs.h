/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing WBS Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing WBS Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_WBS_REGS_H__
#define __INTEL_VVP_WBS_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_WBS_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, WBS, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_WBS_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, WBS, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_WBS_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, WBS, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_WBS_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, WBS, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_WBS_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, WBS, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_WBS_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, WBS, REGNAME_FIELD)

// Compile-time parameter map
#define INTEL_VVP_WBS_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_BPS_IN_REG                  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the bps_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_BPS_OUT_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the bps_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_NUM_COLOR_IN_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the num_color_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_NUM_COLOR_OUT_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the num_color_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_PIP_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the pip register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the max width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the max height register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_WBS_PRECISION_BITS_REG          (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)   ///< Offset of the precision_bits register in the register map (read-only compile-time IP parameter)

// Core-specific RO registers
#define INTEL_VVP_WBS_STATUS_REG                  (INTEL_VVP_CORE_RT_BASE_REG+0)              ///< Offset of the run-time read-only status register in the register map
#define INTEL_VVP_WBS_FRAME_STATS_REG             (INTEL_VVP_CORE_RT_BASE_REG+1)              ///< Offset of the run-time read-only frame statistics register in the register map
// Core-specific RO register offsets and masks
#define INTEL_VVP_WBS_STATUS_RUNNING_OFST         (0)                                         ///< Offset for the running bit
#define INTEL_VVP_WBS_STATUS_RUNNING_MSK          (0x00000001)                                ///< Mask for the running bit
#define INTEL_VVP_WBS_STATUS_COMMIT_PENDING_OFST  (1)                                         ///< Offset for the commit pending bit
#define INTEL_VVP_WBS_STATUS_COMMIT_PENDING_MSK   (0x00000002)                                ///< Mask for the commit pending bit
#define INTEL_VVP_WBS_STATUS_STATS_FROZEN_OFST    (2)                                         ///< Offset for the stats frozen bit
#define INTEL_VVP_WBS_STATUS_STATS_FROZEN_MSK     (0x00000004)                                ///< Mask for the stats frozen bit
#define INTEL_VVP_WBS_FRAME_STATS_CHKSM_OFST      (0)                                         ///< Offset for the frame statistics value
#define INTEL_VVP_WBS_FRAME_STATS_CHKSM_MSK       (0x000000FF)                                ///< Mask for the frame statistics value

// Core-specific RW registers
#define INTEL_VVP_WBS_COMMIT_REG                  (INTEL_VVP_CORE_RT_BASE_REG+2)              ///< Offset of the commit register (including lite mode)
#define INTEL_VVP_WBS_SETTINGS_REG                (INTEL_VVP_CORE_RT_BASE_REG+3)              ///< Offset of the settings register
#define INTEL_VVP_WBS_H_START_REG                 (INTEL_VVP_CORE_RT_BASE_REG+4)              ///< Offset of the h_start register
#define INTEL_VVP_WBS_V_START_REG                 (INTEL_VVP_CORE_RT_BASE_REG+5)              ///< Offset of the v_start register
#define INTEL_VVP_WBS_H_END_REG                   (INTEL_VVP_CORE_RT_BASE_REG+6)              ///< Offset of the h_end register
#define INTEL_VVP_WBS_V_END_REG                   (INTEL_VVP_CORE_RT_BASE_REG+7)              ///< Offset of the v_end register
#define INTEL_VVP_WBS_ZONE_H_COUNT_REG            (INTEL_VVP_CORE_RT_BASE_REG+8)              ///< Offset of the zone_h_count register
#define INTEL_VVP_WBS_ZONE_V_COUNT_REG            (INTEL_VVP_CORE_RT_BASE_REG+9)              ///< Offset of the zone_z_count register
#define INTEL_VVP_WBS_CFA_X0_RANGE_LO_REG         (INTEL_VVP_CORE_RT_BASE_REG+10)             ///< Offset of the cfa_x0_range_lo register
#define INTEL_VVP_WBS_CFA_X0_RANGE_HI_REG         (INTEL_VVP_CORE_RT_BASE_REG+11)             ///< Offset of the cfa_x0_range_hi register
#define INTEL_VVP_WBS_CFA_X1_RANGE_LO_REG         (INTEL_VVP_CORE_RT_BASE_REG+12)             ///< Offset of the cfa_x1_range_lo register
#define INTEL_VVP_WBS_CFA_X1_RANGE_HI_REG         (INTEL_VVP_CORE_RT_BASE_REG+13)             ///< Offset of the cfa_x1_range_hi register
// Core-specific RW register offsets and masks
#define INTEL_VVP_WBS_SETTINGS_BYPASS_OFST        (0)                                         ///< Offset for the bypass setting bit
#define INTEL_VVP_WBS_SETTINGS_BYPASS_MSK         (0x00000001)                                ///< Mask for the bypass setting bit
#define INTEL_VVP_WBS_SETTINGS_CFA_PHASE_OFST     (1)                                         ///< Offset for the cfa phase setting bits
#define INTEL_VVP_WBS_SETTINGS_CFA_PHASE_MSK      (0x00000006)                                ///< Mask for the cfa phase setting bits
#define INTEL_VVP_WBS_SETTINGS_CFA_INVERT_OFST    (3)                                         ///< Offset for the cfa invert setting bits
#define INTEL_VVP_WBS_SETTINGS_CFA_INVERT_MSK     (0x00000018)                                ///< Mask for the cfa invert setting bits
#define INTEL_VVP_WBS_SETTINGS_FREEZE_STATS_OFST  (31)                                        ///< Offset for the freeze statistics setting bit
#define INTEL_VVP_WBS_SETTINGS_FREEZE_STATS_MSK   (0x80000000)                                ///< Mask for the freeze statistics setting bit
#define INTEL_VVP_WBS_H_START_VALUE_OFST          (0)                                         ///< Offset for the h_start setting bits
#define INTEL_VVP_WBS_H_START_VALUE_MSK           (0x0000FFFF)                                ///< Mask for the h_start setting bits
#define INTEL_VVP_WBS_V_START_VALUE_OFST          (0)                                         ///< Offset for the v_start setting bits
#define INTEL_VVP_WBS_V_START_VALUE_MSK           (0x0000FFFF)                                ///< Mask for the v_start setting bits
#define INTEL_VVP_WBS_H_END_VALUE_OFST            (0)                                         ///< Offset for the h_end setting bits
#define INTEL_VVP_WBS_H_END_VALUE_MSK             (0x0000FFFF)                                ///< Mask for the h_end setting bits
#define INTEL_VVP_WBS_V_END_VALUE_OFST            (0)                                         ///< Offset for the v_end setting bits
#define INTEL_VVP_WBS_V_END_VALUE_MSK             (0x0000FFFF)                                ///< Mask for the v_end setting bits
#define INTEL_VVP_WBS_ZONE_H_COUNT_VALUE_OFST     (0)                                         ///< Offset for the zone_h_count setting bits
#define INTEL_VVP_WBS_ZONE_H_COUNT_VALUE_MSK      (0x0000FFFF)                                ///< Mask for the zone_h_count setting bits
#define INTEL_VVP_WBS_ZONE_V_COUNT_VALUE_OFST     (0)                                         ///< Offset for the zone_v_count setting bits
#define INTEL_VVP_WBS_ZONE_V_COUNT_VALUE_MSK      (0x0000FFFF)                                ///< Mask for the zone_v_count setting bits
#define INTEL_VVP_WBS_CFA_X0_RANGE_LO_VALUE_OFST  (0)                                         ///< Offset for the cfa_x0_range_lo setting bits
#define INTEL_VVP_WBS_CFA_X0_RANGE_LO_VALUE_MSK   (0xFFFFFFFF)                                ///< Mask for the cfa_x0_range_lo setting bits
#define INTEL_VVP_WBS_CFA_X0_RANGE_HI_VALUE_OFST  (0)                                         ///< Offset for the cfa_x0_range_hi setting bits
#define INTEL_VVP_WBS_CFA_X0_RANGE_HI_VALUE_MSK   (0xFFFFFFFF)                                ///< Mask for the cfa_x0_range_hi setting bits
#define INTEL_VVP_WBS_CFA_X1_RANGE_LO_VALUE_OFST  (0)                                         ///< Offset for the cfa_x1_range_lo setting bits
#define INTEL_VVP_WBS_CFA_X1_RANGE_LO_VALUE_MSK   (0xFFFFFFFF)                                ///< Mask for the cfa_x1_range_lo setting bits
#define INTEL_VVP_WBS_CFA_X1_RANGE_HI_VALUE_OFST  (0)                                         ///< Offset for the cfa_x1_range_hi setting bits
#define INTEL_VVP_WBS_CFA_X1_RANGE_HI_VALUE_MSK   (0xFFFFFFFF)                                ///< Mask for the cfa_x1_range_hi setting bits

#define INTEL_VVP_WBS_H_START_VALUE_MAX           (INTEL_VVP_WBS_H_START_VALUE_MSK)           ///< Maximum valid value for the h_start regs
#define INTEL_VVP_WBS_V_START_VALUE_MAX           (INTEL_VVP_WBS_V_START_VALUE_MSK)           ///< Maximum valid value for the v_start regs
#define INTEL_VVP_WBS_H_END_VALUE_MAX             (INTEL_VVP_WBS_H_END_VALUE_MSK)             ///< Maximum valid value for the h_end regs
#define INTEL_VVP_WBS_V_END_VALUE_MAX             (INTEL_VVP_WBS_V_END_VALUE_MSK)             ///< Maximum valid value for the v_end regs
#define INTEL_VVP_WBS_ZONE_H_COUNT_VALUE_MAX      (INTEL_VVP_WBS_ZONE_H_COUNT_VALUE_MSK)      ///< Maximum valid value for the zone_h_count regs
#define INTEL_VVP_WBS_ZONE_V_COUNT_VALUE_MAX      (INTEL_VVP_WBS_ZONE_V_COUNT_VALUE_MSK)      ///< Maximum valid value for the zone_v_count regs
#define INTEL_VVP_WBS_CFA_X0_RANGE_LO_VALUE_MAX   (INTEL_VVP_WBS_CFA_X0_RANGE_LO_VALUE_MSK)   ///< Maximum valid value for the cfa_x0_range_lo regs
#define INTEL_VVP_WBS_CFA_X0_RANGE_HI_VALUE_MAX   (INTEL_VVP_WBS_CFA_X1_RANGE_HI_VALUE_MSK)   ///< Maximum valid value for the cfa_x0_range_hi regs
#define INTEL_VVP_WBS_CFA_X1_RANGE_LO_VALUE_MAX   (INTEL_VVP_WBS_CFA_X0_RANGE_LO_VALUE_MSK)   ///< Maximum valid value for the cfa_x1_range_lo regs
#define INTEL_VVP_WBS_CFA_X1_RANGE_HI_VALUE_MAX   (INTEL_VVP_WBS_CFA_X1_RANGE_HI_VALUE_MSK)   ///< Maximum valid value for the cfa_x1_range_hi regs

// Core-specific Memory
#define INTEL_VVP_WBS_RESULTS_TABLE_BASE          (INTEL_VVP_CORE_RT_BASE_REG+48)

#define INTEL_VVP_WBS_RESULTS_TABLE_VALUE_OFST    (0)                                         ///< Offset for the results table values
#define INTEL_VVP_WBS_RESULTS_TABLE_VALUE_MSK     (0x000FFFFF)                                ///< Mask for the results table values

#endif // __INTEL_VVP_WBS_REGS_H__
