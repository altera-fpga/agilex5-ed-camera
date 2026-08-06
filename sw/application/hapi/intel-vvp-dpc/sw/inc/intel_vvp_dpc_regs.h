/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing DPC Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing DPC Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_DPC_REGS_H__
#define __INTEL_VVP_DPC_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_DPC_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, DPC, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_DPC_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, DPC, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_DPC_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, DPC, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_DPC_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, DPC, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_DPC_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, DPC, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_DPC_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, DPC, REGNAME_FIELD)

// Compile-time parameter map
#define INTEL_VVP_DPC_LITE_MODE_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_DEBUG_ENABLED_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_BPS_IN_REG                        (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the bps_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_BPS_OUT_REG                       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the bps_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_NUM_COLOR_IN_REG                  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the num_color_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_NUM_COLOR_OUT_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the num_color_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_PIP_REG                           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the pip register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_MAX_WIDTH_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the max width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_DPC_MAX_HEIGHT_REG                    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the max height register in the register map (read-only compile-time IP parameter)

// Core-specific RO registers
#define INTEL_VVP_DPC_STATUS_REG                        (INTEL_VVP_CORE_RT_BASE_REG+0)              ///< Offset of the run-time read-only status register in the register map
#define INTEL_VVP_DPC_FRAME_STATS_REG                   (INTEL_VVP_CORE_RT_BASE_REG+1)              ///< Offset of the run-time read-only frame statistics register in the register map
#define INTEL_VVP_DPC_PIX_LO_COUNTER_REG                (INTEL_VVP_CORE_RT_BASE_REG+2)              ///< Offset of the run-time read-only pixel low clip counter register in the register map
#define INTEL_VVP_DPC_PIX_HI_COUNTER_REG                (INTEL_VVP_CORE_RT_BASE_REG+3)              ///< Offset of the run-time read-only pixel high clip counter register in the register map
// Core-specific RO register offsets and masks
#define INTEL_VVP_DPC_STATUS_RUNNING_OFST               (0)                                         ///< Offset for the running bit
#define INTEL_VVP_DPC_STATUS_RUNNING_MSK                (0x00000001)                                ///< Mask for the running bit
#define INTEL_VVP_DPC_STATUS_STATS_FROZEN_OFST          (2)                                         ///< Offset for the stats frozen bit
#define INTEL_VVP_DPC_STATUS_STATS_FROZEN_MSK           (0x00000004)                                ///< Mask for the stats frozen bit
#define INTEL_VVP_DPC_FRAME_STATS_CHKSM_OFST            (0)                                         ///< Offset for the frame statistics value
#define INTEL_VVP_DPC_FRAME_STATS_CHKSM_MSK             (0x000000FF)                                ///< Mask for the frame statistics value

// Core-specific RW registers
#define INTEL_VVP_DPC_COMMIT_REG                  		(INTEL_VVP_CORE_RT_BASE_REG+4)              ///< Offset of the commit register (full mode)
#define INTEL_VVP_DPC_SETTINGS_REG                		(INTEL_VVP_CORE_RT_BASE_REG+5)              ///< Offset of the settings register
// Core-specific RW register offsets and masks
#define INTEL_VVP_DPC_SETTINGS_BYPASS_OFST              (0)                                         ///< Offset for the bypass setting bit
#define INTEL_VVP_DPC_SETTINGS_BYPASS_MSK               (0x00000001)                                ///< Mask for the bypass setting bit
#define INTEL_VVP_DPC_SETTINGS_CFA_PHASE_OFST           (1)                                         ///< Offset for the cfa phase setting bits
#define INTEL_VVP_DPC_SETTINGS_CFA_PHASE_MSK            (0x00000006)                                ///< Mask for the cfa phase setting bits
#define INTEL_VVP_DPC_SETTINGS_SENSE_LEVEL_OFST         (3)                                         ///< Offset for the sense level setting bits
#define INTEL_VVP_DPC_SETTINGS_SENSE_LEVEL_MSK          (0x00000018)                                ///< Mask for the sense level setting bits
#define INTEL_VVP_DPC_SETTINGS_PIX_CLIP_CNT_RST_OFST    (5)                                         ///< Offset for the pixel clip counter reset setting bit
#define INTEL_VVP_DPC_SETTINGS_PIX_CLIP_CNT_RST_MSK     (0x00000020)                                ///< Mask for the pixel clip counter reset setting bit
#define INTEL_VVP_DPC_SETTINGS_PIX_CLIP_CNT_SEL_OFST    (6)                                         ///< Offset for the pixel clip counter select setting bits
#define INTEL_VVP_DPC_SETTINGS_PIX_CLIP_CNT_SEL_MSK     (0x000001c0)                                ///< Mask for the pixel clip counter select setting bits
#define INTEL_VVP_DPC_SETTINGS_FREEZE_STATS_OFST        (31)                                        ///< Offset for the freeze statistics setting bit
#define INTEL_VVP_DPC_SETTINGS_FREEZE_STATS_MSK         (0x80000000)                                ///< Mask for the freeze statistics setting bit

#endif // __INTEL_VVP_DPC_REGS_H__
