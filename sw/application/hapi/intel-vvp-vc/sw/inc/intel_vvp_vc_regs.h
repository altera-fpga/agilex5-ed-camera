/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing VC Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing VC Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_VC_REGS_H__
#define __INTEL_VVP_VC_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_VC_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, VC, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_VC_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, VC, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_VC_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, VC, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_VC_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, VC, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_VC_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, VC, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_VC_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, VC, REGNAME_FIELD)

// Compile-time parameter map
#define INTEL_VVP_VC_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_BPS_IN_REG                  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the bps_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_BPS_OUT_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the bps_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_NUM_COLOR_IN_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the num_color_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_NUM_COLOR_OUT_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the num_color_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_PIP_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the pip register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the max width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the max height register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_CFA_ENABLE_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)   ///< Offset of the cfa_enable register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_MAX_GAIN_MESH_POINTS_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the max_gain_mesh_points register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VC_PER_COLOR_GAIN_ENABLE_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+11)   ///< Offset of the per_color_gain_enable register in the register map (read-only compile-time IP parameter)

// Core-specific RO registers
#define INTEL_VVP_VC_STATUS_REG                  (INTEL_VVP_CORE_RT_BASE_REG+0)              ///< Offset of the run-time read-only status register in the register map
#define INTEL_VVP_VC_FRAME_STATS_REG             (INTEL_VVP_CORE_RT_BASE_REG+1)              ///< Offset of the run-time read-only frame statistics register in the register map
// Core-specific RO register offsets and masks
#define INTEL_VVP_VC_STATUS_RUNNING_OFST         (0)                                         ///< Offset for the running bit
#define INTEL_VVP_VC_STATUS_RUNNING_MSK          (0x00000001)                                ///< Mask for the running bit
#define INTEL_VVP_VC_STATUS_COMMIT_PENDING_OFST  (1)                                         ///< Offset for the commit pending bit
#define INTEL_VVP_VC_STATUS_COMMIT_PENDING_MSK   (0x00000002)                                ///< Mask for the commit pending bit
#define INTEL_VVP_VC_STATUS_STATS_FROZEN_OFST    (2)                                         ///< Offset for the stats frozen bit
#define INTEL_VVP_VC_STATUS_STATS_FROZEN_MSK     (0x00000004)                                ///< Mask for the stats frozen bit
#define INTEL_VVP_VC_FRAME_STATS_CHKSM_OFST      (0)                                         ///< Offset for the frame statistics value
#define INTEL_VVP_VC_FRAME_STATS_CHKSM_MSK       (0x000000FF)                                ///< Mask for the frame statistics value

// Core-specific RW registers
#define INTEL_VVP_VC_COMMIT_REG                  (INTEL_VVP_CORE_RT_BASE_REG+2)              ///< Offset of the commit register (including lite mode)
#define INTEL_VVP_VC_SETTINGS_REG                (INTEL_VVP_CORE_RT_BASE_REG+3)              ///< Offset of the settings register
#define INTEL_VVP_VC_BLOCK_PIX_COUNT_REG         (INTEL_VVP_CORE_RT_BASE_REG+4)              ///< Offset of the block pix count register
#define INTEL_VVP_VC_BLOCK_LINE_COUNT_REG        (INTEL_VVP_CORE_RT_BASE_REG+5)              ///< Offset of the block line count register
#define INTEL_VVP_VC_H_NUM_BLOCKS_REG            (INTEL_VVP_CORE_RT_BASE_REG+6)              ///< Offset of the h num blocks register
#define INTEL_VVP_VC_V_NUM_BLOCKS_REG            (INTEL_VVP_CORE_RT_BASE_REG+7)              ///< Offset of the v num blocks register
#define INTEL_VVP_VC_H_RAMP_FRAC_REG             (INTEL_VVP_CORE_RT_BASE_REG+8)              ///< Offset of the h ramp frac register
#define INTEL_VVP_VC_H_RAMP_P1_FRAC_REG          (INTEL_VVP_CORE_RT_BASE_REG+9)              ///< Offset of the h ramp_p1 frac register
#define INTEL_VVP_VC_H_RAMP_M1_FRAC_REG          (INTEL_VVP_CORE_RT_BASE_REG+10)             ///< Offset of the h ramp_m1 frac register
#define INTEL_VVP_VC_V_RAMP_FRAC_REG             (INTEL_VVP_CORE_RT_BASE_REG+11)             ///< Offset of the v ramp frac register
#define INTEL_VVP_VC_V_RAMP_P1_FRAC_REG          (INTEL_VVP_CORE_RT_BASE_REG+12)             ///< Offset of the v ramp_p1 frac register
#define INTEL_VVP_VC_V_RAMP_M1_FRAC_REG          (INTEL_VVP_CORE_RT_BASE_REG+13)             ///< Offset of the v ramp_m1 frac register

// Core-specific RW register offsets and masks
#define INTEL_VVP_VC_SETTINGS_BYPASS_OFST        (0)                                         ///< Offset for the bypass setting bit
#define INTEL_VVP_VC_SETTINGS_BYPASS_MSK         (0x00000001)                                ///< Mask for the bypass setting bit
#define INTEL_VVP_VC_SETTINGS_CFA_PHASE_OFST     (1)                                         ///< Offset for the cfa phase setting bits
#define INTEL_VVP_VC_SETTINGS_CFA_PHASE_MSK      (0x00000006)                                ///< Mask for the cfa phase setting bits
#define INTEL_VVP_VC_BLOCK_PIX_COUNT_VALUE_OFST  (0)                                         ///< Offset for the block_pix_count setting bits
#define INTEL_VVP_VC_BLOCK_PIX_COUNT_VALUE_MSK   (0x00000FFF)                                ///< Mask for the block_pix_count setting bits
#define INTEL_VVP_VC_BLOCK_LINE_COUNT_VALUE_OFST (0)                                         ///< Offset for the block_line_count setting bits
#define INTEL_VVP_VC_BLOCK_LINE_COUNT_VALUE_MSK  (0x00000FFF)                                ///< Mask for the block_line_count setting bits
#define INTEL_VVP_VC_H_NUM_BLOCKS_VALUE_OFST     (0)                                         ///< Offset for the h_num_blocks setting bits
#define INTEL_VVP_VC_H_NUM_BLOCKS_VALUE_MSK      (0x000001FF)                                ///< Mask for the h_num_blocks setting bits
#define INTEL_VVP_VC_V_NUM_BLOCKS_VALUE_OFST     (0)                                         ///< Offset for the v_num_blocks setting bits
#define INTEL_VVP_VC_V_NUM_BLOCKS_VALUE_MSK      (0x000001FF)                                ///< Mask for the v_num_blocks setting bits
#define INTEL_VVP_VC_H_RAMP_FRAC_VALUE_OFST      (0)                                         ///< Offset for the h_ramp_frac setting bits
#define INTEL_VVP_VC_H_RAMP_FRAC_VALUE_MSK       (0x003FFFFF)                                ///< Mask for the h_ramp_frac setting bits
#define INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_OFST   (0)                                         ///< Offset for the h_ramp_p1_frac setting bits
#define INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_MSK    (0x003FFFFF)                                ///< Mask for the h_ramp_p1_frac setting bits
#define INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_OFST   (0)                                         ///< Offset for the s_ramp_m1_frac setting bits
#define INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MSK    (0x003FFFFF)                                ///< Mask for the s_ramp_m1_frac setting bits
#define INTEL_VVP_VC_V_RAMP_FRAC_VALUE_OFST      (0)                                         ///< Offset for the v_ramp_frac setting bits
#define INTEL_VVP_VC_V_RAMP_FRAC_VALUE_MSK       (0x003FFFFF)                                ///< Mask for the v_ramp_frac setting bits
#define INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_OFST   (0)                                         ///< Offset for the v_ramp_p1_frac setting bits
#define INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_MSK    (0x003FFFFF)                                ///< Mask for the v_ramp_p1_frac setting bits
#define INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_OFST   (0)                                         ///< Offset for the v_ramp_m1_frac setting bits
#define INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_MSK    (0x003FFFFF)                                ///< Mask for the v_ramp_m1_frac setting bits

#define INTEL_VVP_VC_BLOCK_PIX_COUNT_VALUE_MAX   (INTEL_VVP_VC_BLOCK_PIX_COUNT_VALUE_MSK)    ///< Maximum valid value for the block pix count regs
#define INTEL_VVP_VC_BLOCK_LINE_COUNT_VALUE_MAX  (INTEL_VVP_VC_BLOCK_LINE_COUNT_VALUE_MSK)   ///< Maximum valid value for the block line count regs
#define INTEL_VVP_VC_H_NUM_BLOCKS_VALUE_MAX      (INTEL_VVP_VC_H_NUM_BLOCKS_VALUE_MSK)       ///< Maximum valid value for the h num blocks regs
#define INTEL_VVP_VC_V_NUM_BLOCKS_VALUE_MAX      (INTEL_VVP_VC_V_NUM_BLOCKS_VALUE_MSK)       ///< Maximum valid value for the v num blocks regs
#define INTEL_VVP_VC_H_RAMP_FRAC_VALUE_MAX       (INTEL_VVP_VC_H_RAMP_FRAC_VALUE_MSK)        ///< Maximum valid value for the h ramp frac regs
#define INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_MAX    (INTEL_VVP_VC_H_RAMP_P1_FRAC_VALUE_MSK)     ///< Maximum valid value for the h ramp_p1 frac regs
#define INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MAX    (INTEL_VVP_VC_H_RAMP_M1_FRAC_VALUE_MSK)     ///< Maximum valid value for the h ramp_m1 frac regs
#define INTEL_VVP_VC_V_RAMP_FRAC_VALUE_MAX       (INTEL_VVP_VC_V_RAMP_FRAC_VALUE_MSK)        ///< Maximum valid value for the v ramp frac regs
#define INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_MAX    (INTEL_VVP_VC_V_RAMP_P1_FRAC_VALUE_MSK)     ///< Maximum valid value for the v ramp_p1 frac regs
#define INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_MAX    (INTEL_VVP_VC_V_RAMP_M1_FRAC_VALUE_MSK)     ///< Maximum valid value for the v ramp_m1 frac regs

// Core-specific Memory
#define INTEL_VVP_VC_LUT_BASE_REG                (INTEL_VVP_CORE_RT_BASE_REG+48)

#define INTEL_VVP_VC_CP_LUT_SIZE                 (4096)                                                            ///< Size of each CP LUT
#define INTEL_VVP_VC_CP_LUT_BASE(lutidx)         (INTEL_VVP_VC_LUT_BASE_REG + (INTEL_VVP_VC_CP_LUT_SIZE * lutidx)) ///< Base address of the LUT corresponding to `lutidx`

#define INTEL_VVP_VC_CP_LUT(lutidx, i)           (INTEL_VVP_VC_CP_LUT_BASE(lutidx) + i)                            ///< Address of the `i`th value in the LUT `lutidx`

#define INTEL_VVP_VC_CP_LUT_GAIN_VALUE_OFST      (0)                                         ///< Offset for the gain value setting bits
#define INTEL_VVP_VC_CP_LUT_GAIN_VALUE_MSK       (0x0007FFFF)                                ///< Mask for the gain value setting bits
#define INTEL_VVP_VC_CP_LUT_GAIN_VALUE_MAX       (INTEL_VVP_VC_CP_LUT_GAIN_VALUE_MSK)        ///< Maximum valid value for the gain value setting


#define INTEL_VVP_VC_STEP_LUT_BASE               (INTEL_VVP_VC_LUT_BASE_REG + (INTEL_VVP_VC_CP_LUT_SIZE * 4)) ///< Base address of the Step LUT.
#define INTEL_VVP_VC_STEP_LUT(i)                 (INTEL_VVP_VC_STEP_LUT_BASE + i)                             ///< Address for the `i`th entry in the Step LUT.

#define INTEL_VVP_VC_STEP_LUT_STEP_VALUE_OFST    (0)                                         ///< Offset for the step value setting bits
#define INTEL_VVP_VC_STEP_LUT_STEP_VALUE_MSK     (0x000001FF)                                ///< Mask for the step value setting bits
#define INTEL_VVP_VC_STEP_LUT_STEP_VALUE_MAX     (INTEL_VVP_VC_STEP_LUT_STEP_VALUE_MSK)      ///< Maximum valid value for the step value setting


#endif // __INTEL_VVP_VC_REGS_H__
