/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing Monitor
 *
 * Register map definitions for the Video & Vision Processing Monitor
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 * 
 * \see Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */
 
#ifndef __INTEL_VVP_SNOOP_REGS_H__
#define __INTEL_VVP_SNOOP_REGS_H__
 
// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"
 
 
// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register
 
 
// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_SNOOP_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, SNOOP, REGNAME_FIELD) 
 
// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_SNOOP_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, SNOOP, REGNAME_FIELD) 
 
// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_SNOOP_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, SNOOP, REGNAME_FIELD) 
 
// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_SNOOP_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, SNOOP, REGNAME_FIELD) 
 
// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_SNOOP_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, SNOOP, REGNAME_FIELD) 
 
// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_SNOOP_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, SNOOP, REGNAME_FIELD) 
 
 
// Compile-time map  (0-255)
// LITE_MODE compile-time parameter
#define INTEL_VVP_SNOOP_LITE_MODE_REG                         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)         ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
 
// DEBUG_ENABLED compile-time parameter
#define INTEL_VVP_SNOOP_DEBUG_ENABLED_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)         ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
 
// CLIPPING_MODE compile-time parameter
#define INTEL_VVP_SNOOP_PIXELS_IN_PARALLEL_REG                (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)         ///< Offset of the pixels_in_parallel register in the register map (read-only compile-time IP parameter)
 
 
// Snoop-specific run-time parameters
#define INTEL_VVP_SNOOP_STATUS_REG                            (INTEL_VVP_CORE_RT_BASE_REG+0)                   ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_SNOOP_STATUS_RUNNING_MSK                    (0x00000001)                                     ///< Mask for the running bit
#define INTEL_VVP_SNOOP_STATUS_RUNNING_OFST                   (0)                                              ///< Offset for the running bit
 
#define INTEL_VVP_SNOOP_NUM_GOOD_FIELDS_REG                   (INTEL_VVP_CORE_RT_BASE_REG+1)                   ///< Offset for the num_good_fields register (read-only)
#define INTEL_VVP_SNOOP_NUM_BROKEN_FIELDS_REG                 (INTEL_VVP_CORE_RT_BASE_REG+2)                   ///< Offset for the num_broken_fields register (read-only, full variant only)
#define INTEL_VVP_SNOOP_NUM_MISMATCH_FIELDS_REG               (INTEL_VVP_CORE_RT_BASE_REG+3)                   ///< Offset for the num_mismatch_fields register (read-only)
#define INTEL_VVP_SNOOP_LAST_NUM_LINES_REG                    (INTEL_VVP_CORE_RT_BASE_REG+4)                   ///< Offset for the num_good_fields register (read-only)
#define INTEL_VVP_SNOOP_LAST_MIN_WIDTH_REG                    (INTEL_VVP_CORE_RT_BASE_REG+5)                   ///< Offset for the num_good_fields register (read-only)
#define INTEL_VVP_SNOOP_LAST_MAX_WIDTH_REG                    (INTEL_VVP_CORE_RT_BASE_REG+6)                   ///< Offset for the num_good_fields register (read-only)
#define INTEL_VVP_SNOOP_RESERVED_REG                          (INTEL_VVP_CORE_RT_BASE_REG+7)                   ///< Reserved
 
#define INTEL_VVP_SNOOP_RESET_COUNTERS_REG                    (INTEL_VVP_CORE_RT_BASE_REG+8)                   ///< Offset for the reset_counters register in the register map (write-only)
    
    
#endif // __INTEL_VVP_SNOOP_REGS_H__
 