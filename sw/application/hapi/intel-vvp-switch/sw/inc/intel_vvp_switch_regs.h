/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_VVP_SWITCH_REGS_H__
#define __INTEL_VVP_SWITCH_REGS_H__


// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the registers


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_SWITCH_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, SWITCH, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_SWITCH_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, SWITCH, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_SWITCH_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, SWITCH, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_SWITCH_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, SWITCH, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_SWITCH_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, SWITCH, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_SWITCH_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, SWITCH, REGNAME_FIELD) 


// Constants
#define INTEL_VVP_SWITCH_MAX_NUM_INPUTS                       16
#define INTEL_VVP_SWITCH_MAX_NUM_OUTPUTS                      16

// Compile-time map  (0-255)
// LITE_MODE compile-time parameter
#define INTEL_VVP_SWITCH_INTF_TYPE_REG                        (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)

// DEBUG_ENABLED compile-time parameter
#define INTEL_VVP_SWITCH_DEBUG_ENABLED_REG                    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)

// UNINTERRUPTED_INPUTS compile-time parameter
#define INTEL_VVP_SWITCH_UNINTERRUPTED_INPUTS_REG             (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the uninterrupted inputs register in the register map (read-only compile-time IP parameter)

// AUTO_CONSUME compile-time parameter
#define INTEL_VVP_SWITCH_AUTO_CONSUME_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the auto_consumme register in the register map (read-only compile-time IP parameter)

// NUM_INPUTS compile-time parameter
#define INTEL_VVP_SWITCH_NUM_INPUTS_REG                       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the num_inputs register in the register map (read-only compile-time IP parameter)

// NUM_OUTPUTS compile-time parameter
#define INTEL_VVP_SWITCH_NUM_OUTPUTS_REG                      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the num_outputs register in the register map (read-only compile-time IP parameter)

// USE_TREADIES compile-time parameter
#define INTEL_VVP_SWITCH_USE_TREADIES_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the use treadies register in the register map (read-only compile-time IP parameter)

// CRASH_SWITCH compile-time parameter
#define INTEL_VVP_SWITCH_CRASH_SWITCH_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the crash switching register in the register map (read-only compile-time IP parameter)

// switch-specific run-time parameters
#define INTEL_VVP_SWITCH_STATUS_REG                           (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the status register
#define INTEL_VVP_SWITCH_STATUS_RUNNING_MSK                   (0x00000001)                                     ///< Mask for the running bit
#define INTEL_VVP_SWITCH_STATUS_RUNNING_OFST                  (0)                                              ///< Offset for the running bit
#define INTEL_VVP_SWITCH_STATUS_PENDING_COMMIT_MSK            (0x00000002)                                     ///< Mask for the commit pending bit
#define INTEL_VVP_SWITCH_STATUS_PENDING_COMMIT_OFST           (1)                                              ///< Offset for the commit pending bit

#define INTEL_VVP_SWITCH_COMMIT_REG                           (INTEL_VVP_CORE_RT_BASE_REG+1)               ///< Offset for the commit settings register

#define INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG               (INTEL_VVP_CORE_RT_BASE_REG+2)
#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG              (INTEL_VVP_CORE_RT_BASE_REG+2+INTEL_VVP_SWITCH_MAX_NUM_INPUTS)

#define INTEL_VVP_SWITCH_INPUT_CONTROL_REG(input)             (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+(input))
#define INTEL_VVP_SWITCH_INPUT_0_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+0)
#define INTEL_VVP_SWITCH_INPUT_1_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+1)
#define INTEL_VVP_SWITCH_INPUT_2_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+2)
#define INTEL_VVP_SWITCH_INPUT_3_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+3)
#define INTEL_VVP_SWITCH_INPUT_4_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+4)
#define INTEL_VVP_SWITCH_INPUT_5_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+5)
#define INTEL_VVP_SWITCH_INPUT_6_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+6)
#define INTEL_VVP_SWITCH_INPUT_7_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+7)
#define INTEL_VVP_SWITCH_INPUT_8_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+8)
#define INTEL_VVP_SWITCH_INPUT_9_CONTROL_REG                  (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+9)
#define INTEL_VVP_SWITCH_INPUT_10_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+10)
#define INTEL_VVP_SWITCH_INPUT_11_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+11)
#define INTEL_VVP_SWITCH_INPUT_12_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+12)
#define INTEL_VVP_SWITCH_INPUT_13_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+13)
#define INTEL_VVP_SWITCH_INPUT_14_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+14)
#define INTEL_VVP_SWITCH_INPUT_15_CONTROL_REG                 (INTEL_VVP_SWITCH_INPUT_CONTROL_BASE_REG+15)

#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_REG(output)           (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+(output))
#define INTEL_VVP_SWITCH_OUTPUT_0_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+0)
#define INTEL_VVP_SWITCH_OUTPUT_1_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+1)
#define INTEL_VVP_SWITCH_OUTPUT_2_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+2)
#define INTEL_VVP_SWITCH_OUTPUT_3_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+3)
#define INTEL_VVP_SWITCH_OUTPUT_4_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+4)
#define INTEL_VVP_SWITCH_OUTPUT_5_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+5)
#define INTEL_VVP_SWITCH_OUTPUT_6_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+6)
#define INTEL_VVP_SWITCH_OUTPUT_7_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+7)
#define INTEL_VVP_SWITCH_OUTPUT_8_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+8)
#define INTEL_VVP_SWITCH_OUTPUT_9_CONTROL_REG                 (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+9)
#define INTEL_VVP_SWITCH_OUTPUT_10_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+10)
#define INTEL_VVP_SWITCH_OUTPUT_11_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+11)
#define INTEL_VVP_SWITCH_OUTPUT_12_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+12)
#define INTEL_VVP_SWITCH_OUTPUT_13_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+13)
#define INTEL_VVP_SWITCH_OUTPUT_14_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+14)
#define INTEL_VVP_SWITCH_OUTPUT_15_CONTROL_REG                (INTEL_VVP_SWITCH_OUTPUT_CONTROL_BASE_REG+15)

#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_INPUT_MSK             (0x0000000F)                                     ///< Mask for the input selection field
#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_INPUT_OFST            (0)                                              ///< Offset for the input selection field
#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_ENABLED_MSK           (0x00000100)                                     ///< Mask for the output enable bit
#define INTEL_VVP_SWITCH_OUTPUT_CONTROL_ENABLED_OFST          (8)                                              ///< Offset for the output enable bit

#endif // __INTEL_VVP_SWITCH_REGS_H__
