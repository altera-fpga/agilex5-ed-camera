/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_CRS_REGS_H__
#define __INTEL_VVP_CRS_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_CRS_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, CRS, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_CRS_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, CRS, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_CRS_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, CRS, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_CRS_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, CRS, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_CRS_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, CRS, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_CRS_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, CRS, REGNAME_FIELD) 


// Compile-time map  (0-255)
// LITE_MODE compile-time parameter
#define INTEL_VVP_CRS_LITE_MODE_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)

// DEBUG_ENABLED compile-time parameter
#define INTEL_VVP_CRS_DEBUG_ENABLED_REG     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)

#define INTEL_VVP_CRS_ENABLE_444_444_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the 444 passthrough enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_444_422_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the 444->422 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_444_420_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the 444->420 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_422_444_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the 422->444 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_422_422_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the 422 passthrough enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_422_420_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the 422->420 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_420_444_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the 420->444 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_420_422_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)    ///< Offset of the 420->422 conversion enabled in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CRS_ENABLE_420_420_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the 420 passthrough enabled in the register map (read-only compile-time IP parameter)


// crs specific run-time parameters
#define INTEL_VVP_CRS_STATUS_REG                    (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_CRS_STATUS_RUNNING_MSK            (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_CRS_STATUS_RUNNING_OFST           (0)                                 ///< Offset for the running bit
#define INTEL_VVP_CRS_STATUS_PENDING_COMMIT_MSK     (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_CRS_STATUS_PENDING_COMMIT_OFST    (1)                                 ///< Offset for the commit pending bit

#define INTEL_VVP_CRS_COMMIT_REG                    (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the commit settings register (full mode)
#define INTEL_VVP_CRS_OUTPUT_SUBSAMPLING_REG        (INTEL_VVP_CORE_RT_BASE_REG+2)      ///< Offset for the output subsampling selection register
#define INTEL_VVP_CRS_SUBSAMPLING_420               0
#define INTEL_VVP_CRS_SUBSAMPLING_INVALID           1
#define INTEL_VVP_CRS_SUBSAMPLING_422               2
#define INTEL_VVP_CRS_SUBSAMPLING_444               3
#define INTEL_VVP_CRS_NUM_SUBSAMPLINGS              4

#endif // __INTEL_VVP_CRS_REGS_H__
