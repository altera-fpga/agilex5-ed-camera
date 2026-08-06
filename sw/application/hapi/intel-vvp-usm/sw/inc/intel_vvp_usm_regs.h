/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing Usm Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing Usm Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_USM_REGS_H__
#define __INTEL_VVP_USM_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_USM_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, USM, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_USM_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, USM, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_USM_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, USM, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_USM_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, USM, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_USM_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, USM, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_USM_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, USM, REGNAME_FIELD)

// Compile-time parameter map
#define INTEL_VVP_USM_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_USM_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_USM_BPS_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the bps_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_USM_PIP_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the pip register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_USM_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the max width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_USM_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the max height register in the register map (read-only compile-time IP parameter)

// Core-specific RO registers
#define INTEL_VVP_USM_STATUS_REG                  (INTEL_VVP_CORE_RT_BASE_REG+0)              ///< Offset of the run-time read-only status register in the register map
// Core-specific RO register offsets and masks
#define INTEL_VVP_USM_STATUS_RUNNING_OFST         (0)                                         ///< Offset for the running bit
#define INTEL_VVP_USM_STATUS_RUNNING_MSK          (0x00000001)                                ///< Mask for the running bit

// Core-specific RW registers
#define INTEL_VVP_USM_COMMIT_REG                  (INTEL_VVP_CORE_RT_BASE_REG+1)              ///< Offset of the commit register (full mode)
#define INTEL_VVP_USM_STRENGTH_REG                (INTEL_VVP_CORE_RT_BASE_REG+2)              ///< Offset of the settings register



//todo Strength only here, or bypass as well?

// Core-specific RW register offsets and masks
#define INTEL_VVP_USM_STRENGTH_OFST        (0)                                         ///< Offset for the strength setting bits
#define INTEL_VVP_USM_STRENGTH_MSK         (0x00003FFF)                                ///< Mask for the strength setting bits

#endif // __INTEL_VVP_USM_REGS_H__
