/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_VVP_TPG_REGS_H__
#define __INTEL_VVP_TPG_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_TPG_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, TPG, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_TPG_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, TPG, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_TPG_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, TPG, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_TPG_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, TPG, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_TPG_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, TPG, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_TPG_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, TPG, REGNAME_FIELD) 

// Compile-time map  (0-255), IP compile-time parameterization
#define INTEL_VVP_TPG_LITE_MODE_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register
#define INTEL_VVP_TPG_DEBUG_ENABLED_REG     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register
#define INTEL_VVP_TPG_NUM_PATTERNS_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the number of patterns register
#define INTEL_VVP_TPG_BPS_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the bits per color sample register
#define INTEL_VVP_TPG_PIP_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the pixels per beat register

// Currently unsupported
// #define INTEL_VVP_TPG_CTRL_IN_ENABLED_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the ctrl_in_enabled register

#define INTEL_VVP_TPG_MAX_NUM_PATTERNS      8
#define INTEL_VVP_TPG_PATTERN_BASE_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset for the first pattern register
#define INTEL_VVP_TPG_PATTERN_0_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the pattern 0 type identifier
#define INTEL_VVP_TPG_PATTERN_0_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the pattern 0 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_1_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the pattern 1 type identifier
#define INTEL_VVP_TPG_PATTERN_1_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)    ///< Offset of the pattern 1 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_2_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the pattern 2 type identifier
#define INTEL_VVP_TPG_PATTERN_2_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+11)   ///< Offset of the pattern 2 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_3_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+12)   ///< Offset of the pattern 3 type identifier
#define INTEL_VVP_TPG_PATTERN_3_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+13)   ///< Offset of the pattern 3 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_4_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+14)   ///< Offset of the pattern 4 type identifier
#define INTEL_VVP_TPG_PATTERN_4_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+15)   ///< Offset of the pattern 4 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_5_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+16)   ///< Offset of the pattern 5 type identifier
#define INTEL_VVP_TPG_PATTERN_5_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+17)   ///< Offset of the pattern 5 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_6_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+18)   ///< Offset of the pattern 6 type identifier
#define INTEL_VVP_TPG_PATTERN_6_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+19)   ///< Offset of the pattern 6 color_space/color_sampling identifier
#define INTEL_VVP_TPG_PATTERN_7_TYPE_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+20)   ///< Offset of the pattern 7 type identifier
#define INTEL_VVP_TPG_PATTERN_7_COLOR_REG   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+21)   ///< Offset of the pattern 7 color_space/color_sampling identifier

// values for INTEL_VVP_TPG_PATTERN_n_TYPE_REG above
#define INTEL_VVP_TPG_BARS_PATTERN                 0
#define INTEL_VVP_TPG_UNIFORM_PATTERN              1
#define INTEL_VVP_TPG_PATHOLOGICAL_PATTERN         2
#define INTEL_VVP_TPG_ZONE_PLATE_PATTERN           3
#define INTEL_VVP_TPG_DIGITAL_CLOCK_PATTERN		   4
#define INTEL_VVP_TPG_SIGNALTAP_COUNTER_PATTERN    5

// values for INTEL_VVP_PATTERN_n_COLOR_REG above
#define INTEL_VVP_TPG_RGB                  0
#define INTEL_VVP_TPG_YCC_444              1
#define INTEL_VVP_TPG_YCC_422              2
#define INTEL_VVP_TPG_YCC_420              3
#define INTEL_VVP_TPG_MONO                 4

// Run-time map, definitions that are specific to the TPG
#define INTEL_VVP_TPG_STATUS_REG                            (INTEL_VVP_CORE_RT_BASE_REG+0)    ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_TPG_STATUS_RUNNING_MSK                    (0x00000001)                      ///< Mask for the running bit
#define INTEL_VVP_TPG_STATUS_RUNNING_OFST                   (0)                               ///< Offset for the running bit
#define INTEL_VVP_TPG_STATUS_PENDING_COMMIT_MSK             (0x00000002)                      ///< Mask for the commit pending bit
#define INTEL_VVP_TPG_STATUS_PENDING_COMMIT_OFST            (1)                               ///< Offset for the commit pending bit

#define INTEL_VVP_TPG_FIELD_COUNT_REG                       (INTEL_VVP_CORE_RT_BASE_REG+1)    ///< Frame/field counter since last change (read-only)

#define INTEL_VVP_TPG_CONTROL_REG                           (INTEL_VVP_CORE_RT_BASE_REG+2)    ///< Control register (start/stop)
#define INTEL_VVP_TPG_CONTROL_GO_MSK                        (0x00000001)                      ///< Mask for the go bit
#define INTEL_VVP_TPG_CONTROL_GO_OFST                       (0)                               ///< Offset for the go bit

#define INTEL_VVP_TPG_COMMIT_REG                            (INTEL_VVP_CORE_RT_BASE_REG+3)    ///< Offset for the Commit Settings register (commit pending changes in full mode)

#define INTEL_VVP_TPG_PATTERN_SELECT_REG                    (INTEL_VVP_CORE_RT_BASE_REG+4)    ///< Offset for the pattern selection register

// Currently unsupported
// #define INTEL_VVP_TPG_PRE_FIELD_CTRL_PKTS_REG               (INTEL_VVP_CORE_RT_BASE_REG+5)    ///< Number of aux/user packets from the control input to be inserted before an output field image info packet (full mode)
// #define INTEL_VVP_TPG_POST_FIELD_CTRL_PKTS_REG              (INTEL_VVP_CORE_RT_BASE_REG+6)    ///< Number of aux/user packets from the control input to be inserted before an output field end-of-field packet (full mode) 

#define INTEL_VVP_TPG_C0_REG                                (INTEL_VVP_CORE_RT_BASE_REG+7)    ///< C0 color sample (Cb or B) for the patterns that require it
#define INTEL_VVP_TPG_C1_REG                                (INTEL_VVP_CORE_RT_BASE_REG+8)    ///< C1 color sample (Y or G) for the patterns that require it
#define INTEL_VVP_TPG_C2_REG                                (INTEL_VVP_CORE_RT_BASE_REG+9)    ///< C2 color sample (Cr or R) for the patterns that require it

#define INTEL_VVP_TPG_BARS_SELECT_REG                       (INTEL_VVP_CORE_RT_BASE_REG+10)   ///< Bars-style selection when the color bars pattern is enabled
#define INTEL_VVP_TPG_COLOR_BARS                            0    // full color bars
#define INTEL_VVP_TPG_GREY_BARS                             1    // grey scale bars
#define INTEL_VVP_TPG_BLACK_WHITE_BARS                      2    // black and white bars
#define INTEL_VVP_TPG_MIXED_BARS						    3	 // mixed type bars

#define INTEL_VVP_TPG_ZONE_X_ORIGIN_REG                     (INTEL_VVP_CORE_RT_BASE_REG+11)   ///< X part of zone plate origin
#define INTEL_VVP_TPG_ZONE_Y_ORIGIN_REG                     (INTEL_VVP_CORE_RT_BASE_REG+12)   ///< Y part of zone plate origin
#define INTEL_VVP_TPG_ZONE_SCALING_FACTOR_REG               (INTEL_VVP_CORE_RT_BASE_REG+13)   ///< Coarse scaling factor of zone plate size
#define INTEL_VVP_TPG_ZONE_FINE_TUNE_FACTOR_REG             (INTEL_VVP_CORE_RT_BASE_REG+14)   ///< Fine tune scaling factor of zone plate size

#define INTEL_VVP_TPG_CLOCK_B_BACKGROUND_REG                (INTEL_VVP_CORE_RT_BASE_REG+15)
#define INTEL_VVP_TPG_CLOCK_G_BACKGROUND_REG                (INTEL_VVP_CORE_RT_BASE_REG+16)
#define INTEL_VVP_TPG_CLOCK_R_BACKGROUND_REG                (INTEL_VVP_CORE_RT_BASE_REG+17)
#define INTEL_VVP_TPG_CLOCK_B_FONT_REG                      (INTEL_VVP_CORE_RT_BASE_REG+18)
#define INTEL_VVP_TPG_CLOCK_G_FONT_REG                      (INTEL_VVP_CORE_RT_BASE_REG+19)
#define INTEL_VVP_TPG_CLOCK_R_FONT_REG                      (INTEL_VVP_CORE_RT_BASE_REG+20)
#define INTEL_VVP_TPG_CLOCK_LOCATION_X_REG                  (INTEL_VVP_CORE_RT_BASE_REG+21)
#define INTEL_VVP_TPG_CLOCK_LOCATION_Y_REG                  (INTEL_VVP_CORE_RT_BASE_REG+22)
#define INTEL_VVP_TPG_CLOCK_SCALE_FACTOR_REG                (INTEL_VVP_CORE_RT_BASE_REG+23)
#define INTEL_VVP_TPG_CLOCK_FPS_REG                         (INTEL_VVP_CORE_RT_BASE_REG+24)

#endif // __INTEL_VVP_TPG_REGS_H__
