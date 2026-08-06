/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
/**
 * \brief Register map definitions for the Video & Vision Processing Scaler Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing Scaler Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */


#ifndef __INTEL_VVP_SCALER_REGS_H__
#define __INTEL_VVP_SCALER_REGS_H__

#include "intel_vvp_core_regs.h"
// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_SCALER_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, SCALER, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_SCALER_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, SCALER, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_SCALER_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, SCALER, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_SCALER_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, SCALER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_SCALER_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, SCALER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_SCALER_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, SCALER, REGNAME_FIELD) 


// The identifier associated with the different scaling algorihms (used in INTEL_VVP_SCALER_ALGORITHM_REG)
#define INTEL_VVP_SCALER_NEAREST_NEIGHBOR_SCALING               0
#define INTEL_VVP_SCALER_BILINEAR_SCALING                       1
#define INTEL_VVP_SCALER_POLYPHASE_SCALING                      2

// Compile-time map  (0-255)
#define INTEL_VVP_SCALER_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_PIXELS_IN_PARALLEL_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the pixels_in_parallel register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_MAX_INPUT_WIDTH_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the max_input_width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_MAX_OUTPUT_WIDTH_REG        (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the max_output_width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_ALGORITHM_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the scaling algorithm register in the register map (read-only compile-time IP parameter, defined above)
#define INTEL_VVP_SCALER_COEFFS_RUNTIME_LOAD_REG     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the run-time coefficient loading register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_COEFFS_MEM_INIT_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the coefficient memory initialization register in the register map (read-only compile-time IP parameter)

#define INTEL_VVP_SCALER_V_SCALING_ENABLED_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the vertical scaling enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_NUM_TAPS_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)    ///< Offset of the vertical number of taps register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_NUM_PHASES_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the vertical number of phases register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_NUM_BANKS_REG             (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+11)   ///< Offset of the vertical number of banks register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_COEFFS_SIGNED_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+12)   ///< Offset of the vertical coef_signed register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_COEFFS_INT_BITS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+13)   ///< Offset of the vertical coeff integer bits register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_V_COEFFS_FRAC_BITS_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+14)   ///< Offset of the vertical coefficient fractional bits register in the register map (read-only compile-time IP parameter)

#define INTEL_VVP_SCALER_H_SCALING_ENABLED_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+15)   ///< Offset of the horizontal scaling enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_NUM_TAPS_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+16)   ///< Offset of the horizontal number of taps register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_NUM_PHASES_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+17)   ///< Offset of the horizontal number of phases register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_NUM_BANKS_REG             (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+18)   ///< Offset of the horizontal number of banks register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_COEFFS_SIGNED_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+19)   ///< Offset of the horizontal coef_signed register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_COEFFS_INT_BITS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+20)   ///< Offset of the horizontal coeff integer bits register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_COEFFS_FRAC_BITS_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+21)   ///< Offset of the horizontal coefficient fractional bits register in the register map (read-only compile-time IP parameter)

#define INTEL_VVP_SCALER_V_PARTIAL_SCALING_ENABLED_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+24)    ///< Offset of the vertical partial scaling enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_SCALER_H_PARTIAL_SCALING_ENABLED_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+25)    ///< Offset of the horizontal partial scaling enabled register in the register map (read-only compile-time IP parameter)


// Run-time map  (256-512)
// Read-only registers
#define INTEL_VVP_SCALER_STATUS_REG                     (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_SCALER_STATUS_RUNNING_MSK             (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_SCALER_STATUS_RUNNING_OFST            (0)                                 ///< Offset for the running bit
#define INTEL_VVP_SCALER_STATUS_PENDING_COMMIT_MSK      (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_SCALER_STATUS_PENDING_COMMIT_OFST     (1)                                 ///< Offset for the commit pending bit

// Writeable registers
#define INTEL_VVP_SCALER_COMMIT_REG                     (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the commit settings register (full mode)
#define INTEL_VVP_SCALER_OUTPUT_WIDTH_REG               (INTEL_VVP_CORE_RT_BASE_REG+2)      ///< Offset for the output width register
#define INTEL_VVP_SCALER_OUTPUT_HEIGHT_REG              (INTEL_VVP_CORE_RT_BASE_REG+3)      ///< Offset for the output height register
#define INTEL_VVP_SCALER_H_BANK_REG                     (INTEL_VVP_CORE_RT_BASE_REG+4)      ///< Offset for the horizontal coefficient bank selection register
#define INTEL_VVP_SCALER_V_BANK_REG                     (INTEL_VVP_CORE_RT_BASE_REG+5)      ///< Offset for the vertical coefficient bank selection register

// Writeable registers (horizontal partial scaling)
#define INTEL_VVP_SCALER_TOTAL_IN_WIDTH_REG             (INTEL_VVP_CORE_RT_BASE_REG+6)      ///< Offset for the total input width register when split horizontal scaling is enabled
#define INTEL_VVP_SCALER_TOTAL_OUT_WIDTH_REG            (INTEL_VVP_CORE_RT_BASE_REG+7)      ///< Offset for the total output width register when split horizontal scaling is enabled
#define INTEL_VVP_SCALER_H_POS_ERROR_REG                (INTEL_VVP_CORE_RT_BASE_REG+8)      ///< Offset for the first pixel position error register when split horizontal scaling is enabled
                                                                                            ///< This is (first_out_pixel_x * total_in_width) % total_out_width    where
                                                                                            ///< first_pixel_x is the x coordinate of the first pixel of the band (excluding h_extended side
                                                                                            ///< band if used)
#define INTEL_VVP_SCALER_H_PHASE_OFFSET_REG             (INTEL_VVP_CORE_RT_BASE_REG+9)      ///< Offset for the phase offset register when split horizontal scaling is enabled
                                                                                            ///< This is (first_out_pixel_x * total_in_width * num_h_phases) / total_out_width
#define INTEL_VVP_SCALER_H_PHASE_ERROR_REG              (INTEL_VVP_CORE_RT_BASE_REG+10)     ///< Offset for the phase offset error register when split horizontal scaling is enabled
                                                                                            ///< This is (first_out_pixel_x * total_in_width * num_h_phases) % total_out_width
#define INTEL_VVP_SCALER_H_EXTENDED_FILL_REG            (INTEL_VVP_CORE_RT_BASE_REG+11)     ///< Offset for the "extended fill" register that indicates whether the partial input lines contains 
                                                                                            ///< extra pixels (num_h_phases-1)/2 on the left to generating proper edge pixels. This must be set to false
                                                                                            ///< for the leftmost band (first_out_pixel_x==0)
                                                                                            ///< When a side band is used, the first input pixel of the partial line is first_pixel_x - (num_phases-1)/2
                                                                                            
// Writeable registers (vertical partial scaling)
#define INTEL_VVP_SCALER_TOTAL_IN_HEIGHT_REG            (INTEL_VVP_CORE_RT_BASE_REG+12)     ///< Offset for the total input height register when split vertical scaling is enabled
#define INTEL_VVP_SCALER_TOTAL_OUT_HEIGHT_REG           (INTEL_VVP_CORE_RT_BASE_REG+13)     ///< Offset for the total output height register when split vertical scaling is enabled
#define INTEL_VVP_SCALER_V_POS_ERROR_REG                (INTEL_VVP_CORE_RT_BASE_REG+14)     ///< Offset for the first pixel position error register when split horizontal scaling is enabled
                                                                                            ///< This is (first_out_pixel_y * total_in_height ) % total_out_height
#define INTEL_VVP_SCALER_V_PHASE_OFFSET_REG             (INTEL_VVP_CORE_RT_BASE_REG+15)     ///< Offset for the phase offset register when split vertical scaling is enabled
                                                                                            ///< This is (((first_out_pixel_y * total_in_height * num_v_phases) / total_out_height
#define INTEL_VVP_SCALER_V_PHASE_ERROR_REG              (INTEL_VVP_CORE_RT_BASE_REG+16)     ///< Offset for the phase offset error register when split vertical scaling is enabled
                                                                                            ///< This is (((first_out_pixel_y * total_in_height * num_v_phases) % total_out_height
#define INTEL_VVP_SCALER_V_EXTENDED_FILL_REG            (INTEL_VVP_CORE_RT_BASE_REG+17)     ///< Offset for the "extended fill" register that indicates whether the partial input image contains 
                                                                                            ///< extra top lines for generating proper top edge pixels. This must be set to false
                                                                                            ///< for the topmost band (first_out_pixel_y==0)

// Coefficient map  (512-1024), this space is only used in polyphase mode when run-time loading of coefficients is enabled by the IP parameterization
// This space is write-only. Please refer to the user guide for details on multitaps polyphase scaling and coefficients banks.
#define INTEL_VVP_RT_COEFF_LOAD_BASE_REG                    (128)                                                ///< Offset for the first register of the run-time coefficient space
#define INTEL_VVP_SCALER_RT_COEFF_LOAD_BANK_SELECT_REG      (INTEL_VVP_RT_COEFF_LOAD_BASE_REG+0)                 ///< Offset for the coefficient bank selection register (this is the bank that will be targetted by the next commit)
#define INTEL_VVP_SCALER_RT_COEFF_LOAD_PHASE_SELECT_REG     (INTEL_VVP_RT_COEFF_LOAD_BASE_REG+1)                 ///< Offset for the coefficient phase selection register (this is the phase that will be targetted by the next commit)
#define INTEL_VVP_SCALER_RT_COEFF_LOAD_COMMIT_REG           (INTEL_VVP_RT_COEFF_LOAD_BASE_REG+2)                 ///< Offset for the coefficient commit register.
                                                                                                                 ///< Write 0 to commit the [TAP_BASE : TAP_BASE+V_NUM_TAPS-1] coefficients as vertical phase PHASE_SELECT, bank BANK_SELECT
                                                                                                                 ///< Write 1 to commit the [TAP_BASE : TAP_BASE+H_NUM_TAPS-1] coefficients as horizontal phase PHASE_SELECT, bank BANK_SELECT
#define INTEL_VVP_SCALER_RT_COEFF_LOAD_TAP_BASE_REG         (INTEL_VVP_RT_COEFF_LOAD_BASE_REG+3)                 ///< Base register for the first tap coefficient, write V_NUM_TAPS/H_NUM_TAPS coefficients from this register to
                                                                                                                 ///< respectively setup the coefficients of a vertical phase or horizontal phase
#define INTEL_VVP_SCALER_RT_COEFF_LOAD_TAP_REG(tap)         (INTEL_VVP_SCALER_RT_COEFF_LOAD_TAP_BASE_REG+tap)    ///< Offset for coefficient number "tap"

#endif // __INTEL_VVP_SCALER_REGS_H__
