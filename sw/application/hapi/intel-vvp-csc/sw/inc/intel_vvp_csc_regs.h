/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing Color Space Converter Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing Color Space Converter Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_CSC_REGS_H__
#define __INTEL_VVP_CSC_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Useful defines
#define INTEL_VVP_CSC_NUM_COLOR_PLANES        3
#define INTEL_VVP_CSC_NUM_COEFFS              9
#define INTEL_VVP_CSC_NUM_SUMMANDS            3


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_CSC_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, CSC, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_CSC_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, CSC, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_CSC_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, CSC, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_CSC_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, CSC, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_CSC_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, CSC, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_CSC_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, CSC, REGNAME_FIELD) 


// Compile-time map  (0-255)
#define INTEL_VVP_CSC_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_BPS_IN_REG                  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the bps_in register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_BPS_OUT_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the bps_out register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_FRAC_BITS_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the frac_bits register in the register map (read-only compile-time IP parameter, number of fractional bits for coefs and summands)
#define INTEL_VVP_CSC_COEFFS_SIGNED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the coef_signed register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_COEFFS_INT_BITS_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the coeff_int_bits register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_SUMMANDS_SIGNED_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the summand_signed register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_SUMMANDS_INT_BITS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the summand_int_bits register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_BINARY_POINT_RIGHT_MOVE_REG (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)    ///< Offset of the binary_point_right_move register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_CSC_ROUND_METHOD_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the round_method register in the register map (read-only compile-time IP parameter)

// Rounding methods indicated through the INTEL_VVP_CSC_ROUND_METHOD_REG register
#define INTEL_VVP_CSC_ROUND_UP               1
#define INTEL_VVP_CSC_ROUND_HALF_EVEN        2
#define INTEL_VVP_CSC_ROUND_TRUNC            3

#define INTEL_VVP_CSC_STATUS_REG                                (INTEL_VVP_CORE_RT_BASE_REG+0)
#define INTEL_VVP_CSC_STATUS_RUNNING_MSK                        (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_CSC_STATUS_RUNNING_OFST                       (0)                                 ///< Offset for the running bit
#define INTEL_VVP_CSC_STATUS_PENDING_COMMIT_MSK                 (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_CSC_STATUS_PENDING_COMMIT_OFST                (1)                                 ///< Offset for the commit pending bit

#define INTEL_VVP_CSC_COMMIT_REG                                (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the commit settings register (full mode)

// Run-time coefficient/summand registers (R/W)
#define INTEL_VVP_CSC_COEFFS_BASE_REG         (INTEL_VVP_CORE_RT_BASE_REG+2)
#define INTEL_VVP_CSC_SUMMANDS_BASE_REG       (INTEL_VVP_CORE_RT_BASE_REG+11)

#define INTEL_VVP_CSC_COEFF_A0_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+0)
#define INTEL_VVP_CSC_COEFF_B0_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+1)
#define INTEL_VVP_CSC_COEFF_C0_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+2)
#define INTEL_VVP_CSC_COEFF_A1_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+3)
#define INTEL_VVP_CSC_COEFF_B1_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+4)
#define INTEL_VVP_CSC_COEFF_C1_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+5)
#define INTEL_VVP_CSC_COEFF_A2_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+6)
#define INTEL_VVP_CSC_COEFF_B2_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+7)
#define INTEL_VVP_CSC_COEFF_C2_REG            (INTEL_VVP_CSC_COEFFS_BASE_REG+8)
#define INTEL_VVP_CSC_SUMMAND_0_REG           (INTEL_VVP_CSC_SUMMANDS_BASE_REG+0)
#define INTEL_VVP_CSC_SUMMAND_1_REG           (INTEL_VVP_CSC_SUMMANDS_BASE_REG+1)
#define INTEL_VVP_CSC_SUMMAND_2_REG           (INTEL_VVP_CSC_SUMMANDS_BASE_REG+2)



#define INTEL_VVP_CSC_OUTPUT_CS_REG           (INTEL_VVP_CORE_RT_BASE_REG+14)                       ///< Offset for the output color space register (full mode)

// Common colorspace code that may be indicated through the INTEL_VVP_CSC_OUTPUT_CS_REG register
#define INTEL_VVP_CSC_OUTPUT_CS_RGB        0
#define INTEL_VVP_CSC_OUTPUT_CS_YCC        1
#define INTEL_VVP_CSC_OUTPUT_CS_MONO       2

#endif // __INTEL_VVP_CSC_REGS_H__
