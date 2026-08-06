/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_PROTOCOL_CONV_REGS_H__
#define __INTEL_VVP_PROTOCOL_CONV_REGS_H__

#include "intel_vvp_core_regs.h"

// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_PROTOCOL_CONV_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, PROTOCOL_CONV, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_PROTOCOL_CONV_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, PROTOCOL_CONV, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_PROTOCOL_CONV_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, PROTOCOL_CONV, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_PROTOCOL_CONV_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, PROTOCOL_CONV, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_PROTOCOL_CONV_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, PROTOCOL_CONV, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_PROTOCOL_CONV_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, PROTOCOL_CONV, REGNAME_FIELD) 


// Compile-time map  (0-255)

// CONVERSION_MODE compile-time parameter, conversion mode is a number in the range 0-5
#define INTEL_VVP_PROTOCOL_CONV_CONVERSION_MODE_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the conversion_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_PROTOCOL_CONV_VIP_TO_VVP_LITE               (0)                                         ///< VIP to VVP Lite conversion
#define INTEL_VVP_PROTOCOL_CONV_VVP_LITE_TO_VIP               (1)                                         ///< VVP Lite to VIP conversion
#define INTEL_VVP_PROTOCOL_CONV_VIP_TO_VVP_FULL               (2)                                         ///< VIP to VVP Full conversion
#define INTEL_VVP_PROTOCOL_CONV_VVP_FULL_TO_VIP               (3)                                         ///< VVP Full to VIP conversion
#define INTEL_VVP_PROTOCOL_CONV_VVP_LITE_TO_VVP_FULL          (4)                                         ///< VVP Lite to VVP Full conversion
#define INTEL_VVP_PROTOCOL_CONV_VVP_FULL_TO_VVP_LITE          (5)                                         ///< VVP Full to VVP Lite conversion

// DEBUG_ENABLED compile-time parameter
#define INTEL_VVP_PROTOCOL_CONV_DEBUG_ENABLED_REG             (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map  (read-only compile-time IP parameter)

// core-specific run-time parameters
#define INTEL_VVP_PROTOCOL_CONV_STATUS_REG                    (INTEL_VVP_CORE_RT_BASE_REG+0)              ///< Offset of the run-time read-only status register in the register map 
#define INTEL_VVP_PROTOCOL_CONV_STATUS_RUNNING_MSK            (0x00000001)                                ///< Mask for the running flag in the status register
#define INTEL_VVP_PROTOCOL_CONV_STATUS_RUNNING_OFST           (0)                                         ///< Offset of the running flag in the status register
#define INTEL_VVP_PROTOCOL_CONV_STATUS_FIELD_RECEIVED_MSK     (0x00000002)                                ///< Mask for the field_received flag in the status register
#define INTEL_VVP_PROTOCOL_CONV_STATUS_FIELD_RECEIVED_OFST    (1)                                         ///< Offest of the field_received flag in the status register
#define INTEL_VVP_PROTOCOL_CONV_STATUS_BROKEN_FIELD_MSK       (0x00000004)                                ///< Mask for the broken_field flag in the status register
#define INTEL_VVP_PROTOCOL_CONV_STATUS_BROKEN_FIELD_OFST      (2)                                         ///< Offset of the broken_field flag in the status register

#define INTEL_VVP_PROTOCOL_CONV_FIELD_COUNT_REG               (INTEL_VVP_CORE_RT_BASE_REG+1)              ///< Offset of the run-time read-only field_count register in the register map 

#define INTEL_VVP_PROTOCOL_CONV_VIP_WIDTH_REG                 (INTEL_VVP_CORE_RT_BASE_REG+2)              ///< Offset of the run-time read-only vip_width register in the register map 

#define INTEL_VVP_PROTOCOL_CONV_VIP_HEIGHT_REG                (INTEL_VVP_CORE_RT_BASE_REG+3)              ///< Offset of the run-time read-only vip_height register in the register map 

#define INTEL_VVP_PROTOCOL_CONV_VIP_INTERLACE_REG             (INTEL_VVP_CORE_RT_BASE_REG+4)              ///< Offset of the run-time read-only vip_interlace register in the register map 
    
#define INTEL_VVP_PROTOCOL_CONV_CTRL_REG                      (INTEL_VVP_CORE_RT_BASE_REG+5)              ///< Offset of the writable control register in the register map 
#define INTEL_VVP_PROTOCOL_CONV_CTRL_GO_MSK                   (0x00000001)                                ///< Mask for the go bit in the control register
#define INTEL_VVP_PROTOCOL_CONV_CTRL_GO_OFST                  (0)                                         ///< Offset of the go bit in the control register

#define INTEL_VVP_PROTOCOL_CONV_FIELD_COUNT_RESET_REG         (INTEL_VVP_CORE_RT_BASE_REG+6)              ///< Offset of the writable field_count_reset register in the register map 
        
#endif // __INTEL_VVP_PROTOCOL_CONV_REGS_H__

