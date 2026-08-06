/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_VFW_REGS_H__
#define __INTEL_VVP_VFW_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_VFW_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, VFW, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_VFW_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, VFW, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_VFW_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, VFW, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_VFW_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, VFW, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_VFW_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, VFW, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_VFW_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, VFW, REGNAME_FIELD)

#define INTEL_VVP_VFW_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the max_height register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the max_width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_BPS_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the bits per sample register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_NUMBER_OF_COLOR_PLANES_REG  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the number of color planes register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_PIXELS_IN_PARALLEL_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the number of pixels in parallel register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFW_PACKING_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the packing identifier register in the register map (read-only compile-time IP parameter)

// Values used in the packing register
#define INTEL_VVP_VFW_PERFECT_PACKING             0   ///< Color samples are packed as thightly as possible, memory words are fully utilized and a color sample may be split over multiple memory words
#define INTEL_VVP_VFW_COLOR_PACKING               1   ///< Color samples are packed as thightly as possible in a word, a color sample wouldn't be split over multiple memory words but a pixel may be split
#define INTEL_VVP_VFW_PIXEL_PACKING               2   ///< Pixels are packed as thightly as possible in a word, a pixel cannot be split over multiple memory words

#define INTEL_VVP_VFW_STATUS_REG                  (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register (input-side) in the register map (read-only)
#define INTEL_VVP_VFW_STATUS_RUNNING_MSK          (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_VFW_STATUS_RUNNING_OFST         (0)                                 ///< Offset for the running bit
#define INTEL_VVP_VFW_STATUS_PENDING_COMMIT_MSK   (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_VFW_STATUS_PENDING_COMMIT_OFST  (1)                                 ///< Offset for the commit pending bit

#define INTEL_VVP_VFW_BUFFER_AVAILABLE_REG        (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the buffer available register in the register map (read-only)
#define INTEL_VVP_VFW_BUFFER_WRITE_COUNT_REG      (INTEL_VVP_CORE_RT_BASE_REG+2)      ///< Offset for the buffer write count register in the register map (read-only)
#define INTEL_VVP_VFW_BUFFER_START_ADDRESS_REG    (INTEL_VVP_CORE_RT_BASE_REG+3)      ///< Offset for the buffer start address register in the register map (read-only)
#define INTEL_VVP_VFW_BUFFER_F1_FLAG_REG          (INTEL_VVP_CORE_RT_BASE_REG+4)      ///< Offset for the buffer F1 flag register in the register map (read-only)
#define INTEL_VVP_VFW_BUFFER_WIDTH_REG            (INTEL_VVP_CORE_RT_BASE_REG+5)      ///< Offset for the buffer width register in the register map (read-only)
#define INTEL_VVP_VFW_BUFFER_HEIGHT_REG           (INTEL_VVP_CORE_RT_BASE_REG+6)      ///< Offset for the buffer height register in the register map (read-only)
//#define INTEL_VVP_VFW_ERROR_CODE_REG              (INTEL_VVP_CORE_RT_BASE_REG+7)      ///< Offset for the error code register in the register map (read-only)
#define INTEL_VVP_VFW_FIELD_COUNT_REG             (INTEL_VVP_CORE_RT_BASE_REG+8)      ///< Offset for the field count register in the register map (read-only)

#define INTEL_VVP_VFW_COMMIT_REG                  (INTEL_VVP_CORE_RT_BASE_REG+9)     ///< Offset for the commit settings register
#define INTEL_VVP_VFW_BUFFER_ACKNOWLEDGE_REG      (INTEL_VVP_CORE_RT_BASE_REG+10)     ///< Offset for the buffer ack register
#define INTEL_VVP_VFW_RUN_REG                     (INTEL_VVP_CORE_RT_BASE_REG+11)     ///< Offset for the run register
#define INTEL_VVP_VFW_NUM_BUFFERS_REG             (INTEL_VVP_CORE_RT_BASE_REG+12)     ///< Offset for the num buffers register
#define INTEL_VVP_VFW_BUFFER_BASE_REG             (INTEL_VVP_CORE_RT_BASE_REG+13)     ///< Offset for the buffer base address register
#define INTEL_VVP_VFW_INTER_BUFFER_OFFSET_REG     (INTEL_VVP_CORE_RT_BASE_REG+14)     ///< Offset for the inter-buffer offset register
#define INTEL_VVP_VFW_INTER_LINE_OFFSET_REG       (INTEL_VVP_CORE_RT_BASE_REG+15)     ///< Offset for the inter-line offset register
#define INTEL_VVP_VFW_IRQ_LINE_THRESHOLD_REG      (INTEL_VVP_CORE_RT_BASE_REG+21)     ///< Offset for the irq line threshold register
#define INTEL_VVP_VFW_OVERWRITE_BROKEN_REG        (INTEL_VVP_CORE_RT_BASE_REG+22)     ///< Offset for the overwrite broken register
//#define INTEL_VVP_VFW_ERROR_RESET_REG             (INTEL_VVP_CORE_RT_BASE_REG+23)     ///< Offset for the error reset register

// Values used in the INTEL_VVP_VFW_RUN_REG register
#define INTEL_VVP_VFW_STOP                        0   ///< Stop until new commit
#define INTEL_VVP_VFW_FREE_RUNNING                1   ///< Round-robin through the enabled buffer sets and buffers
#define INTEL_VVP_VFW_SINGLE_SHOT                 3   ///< Use all requested sets and buffers then wait for a new commit

#endif // __INTEL_VVP_VFW_REGS_H__
