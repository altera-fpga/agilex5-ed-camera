/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_VFB_REGS_H__
#define __INTEL_VVP_VFB_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_VFB_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, VFB, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_VFB_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, VFB, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_VFB_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, VFB, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_VFB_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, VFB, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_VFB_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, VFB, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_VFB_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, VFB, REGNAME_FIELD) 

#define INTEL_VVP_VFB_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the max_width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the max_height register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_DROP_ENABLED_REG            (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the enable_drop register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_REPEAT_ENABLED_REG          (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the enable_repeat register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_INVALID_FRAMES_DROPPED_REG  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the drop_invalid register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_MEM_BASE_ADDR_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the memory base address register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_MEM_BUFFER_STRIDE_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the memory buffer (byte) stride register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_MEM_LINE_STRIDE_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)    ///< Offset of the memory line (byte) stride register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_BPS_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)   ///< Offset of the bits per sample register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_NUMBER_OF_COLOR_PLANES_REG  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+11)   ///< Offset of the number of color planes register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_PIXELS_IN_PARALLEL_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+12)   ///< Offset of the number of pixels in parallel register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFB_PACKING_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+13)   ///< Offset of the packing identifier register in the register map (read-only compile-time IP parameter)

// Values used in the packing register
#define INTEL_VVP_VFB_PERFECT_PACKING             0   ///< Color samples are packed as thightly as possible, memory words are fully utilized and a color sample may be split over multiple memory words
#define INTEL_VVP_VFB_COLOR_PACKING               1   ///< Color samples are packed as thightly as possible in a word, a color sample wouldn't be split over multiple memory words but a pixel may be split
#define INTEL_VVP_VFB_PIXEL_PACKING               2   ///< Pixels are packed as thightly as possible in a word, a pixel cannot be split over multiple memory words

#define INTEL_VVP_VFB_INPUT_STATUS_REG                    (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register (input-side) in the register map (read-only)
#define INTEL_VVP_VFB_INPUT_STATUS_RUNNING_MSK            (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_VFB_INPUT_STATUS_RUNNING_OFST           (0)                                 ///< Offset for the running bit

#define INTEL_VVP_VFB_NUM_INPUT_FIELDS_REG                (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the run-time input fields counter (input/write-side) in the register map (read-only)
#define INTEL_VVP_VFB_NUM_DROPPED_FIELDS_REG              (INTEL_VVP_CORE_RT_BASE_REG+2)      ///< Offset for the run-time dropped fields counter (input/write-side) in the register map (read-only)
#define INTEL_VVP_VFB_NUM_INVALID_FIELDS_REG              (INTEL_VVP_CORE_RT_BASE_REG+3)      ///< Offset for the run-time invalid fields counter (input/write-side) in the register map (read-only)

#define INTEL_VVP_VFB_OUTPUT_STATUS_REG                   (INTEL_VVP_CORE_RT_BASE_REG+4)      ///< Offset for the run-time status register (output/read-side) in the register map (read-only)
#define INTEL_VVP_VFB_OUTPUT_STATUS_RUNNING_MSK           (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_VFB_OUTPUT_STATUS_RUNNING_OFST          (0)                                 ///< Offset for the running bit

#define INTEL_VVP_VFB_NUM_OUTPUT_FIELDS_REG               (INTEL_VVP_CORE_RT_BASE_REG+5)      ///< Offset for the run-time output fields counter (output/read-side) in the register map (read-only)
#define INTEL_VVP_VFB_NUM_REPEATED_FIELDS_REG             (INTEL_VVP_CORE_RT_BASE_REG+6)      ///< Offset for the run-time repeated fields counter (output/read-side) in the register map (read-only)

#define INTEL_VVP_VFB_OUTPUT_CONTROL_REG                  (INTEL_VVP_CORE_RT_BASE_REG+7)      ///< Offset for the output control register (output/read-side) in the register map (read-only)
#define INTEL_VVP_VFB_OUTPUT_CONTROL_GO_MSK               (0x00000001)                        ///< Mask for the go bit in the control register
#define INTEL_VVP_VFB_OUTPUT_CONTROL_GO_OFST              (0)                                 ///< Offset for the running bit

#endif // __INTEL_VVP_VFB_REGS_H__
