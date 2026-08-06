/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef __INTEL_VVP_VFR_REGS_H__
#define __INTEL_VVP_VFR_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_VFR_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, VFR, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_VFR_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, VFR, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_VFR_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, VFR, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_VFR_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, VFR, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_VFR_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, VFR, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_VFR_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, VFR, REGNAME_FIELD)

#define INTEL_VVP_VFR_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_MAX_BUFFER_SETS_REG         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the maximum number of usable buffer sets (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_MAX_HEIGHT_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the max_height register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_MAX_WIDTH_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)    ///< Offset of the max_width register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_BPS_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)    ///< Offset of the bits per sample register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_NUMBER_OF_COLOR_PLANES_REG  (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)    ///< Offset of the number of color planes register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_PIXELS_IN_PARALLEL_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)    ///< Offset of the number of pixels in parallel register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_VFR_PACKING_REG                 (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)    ///< Offset of the packing identifier register in the register map (read-only compile-time IP parameter)

// Values used in the packing register
#define INTEL_VVP_VFR_PERFECT_PACKING             0   ///< Color samples are packed as thightly as possible, memory words are fully utilized and a color sample may be split over multiple memory words
#define INTEL_VVP_VFR_COLOR_PACKING               1   ///< Color samples are packed as thightly as possible in a word, a color sample wouldn't be split over multiple memory words but a pixel may be split
#define INTEL_VVP_VFR_PIXEL_PACKING               2   ///< Pixels are packed as thightly as possible in a word, a pixel cannot be split over multiple memory words

#define INTEL_VVP_VFR_STATUS_REG                  (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register (input-side) in the register map (read-only)
#define INTEL_VVP_VFR_STATUS_RUNNING_MSK          (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_VFR_STATUS_RUNNING_OFST         (0)                                 ///< Offset for the running bit
#define INTEL_VVP_VFR_STATUS_PENDING_COMMIT_MSK   (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_VFR_STATUS_PENDING_COMMIT_OFST  (1)                                 ///< Offset for the commit pending bit

#define INTEL_VVP_VFR_LAST_BUFFER_READ_REG        (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the last buffer read address register in the register map (read-only)

#define INTEL_VVP_VFR_COMMIT_REG                  (INTEL_VVP_CORE_RT_BASE_REG+20)     ///< Offset for the commit settings register
#define INTEL_VVP_VFR_NUM_BUFFER_SETS_REG         (INTEL_VVP_CORE_RT_BASE_REG+21)     ///< Offset for the number of buffer sets register
#define INTEL_VVP_VFR_BUFFER_MODE_REG             (INTEL_VVP_CORE_RT_BASE_REG+22)     ///< Offset for the buffer read mode register
#define INTEL_VVP_VFR_STARTING_BUFFER_SET_REG     (INTEL_VVP_CORE_RT_BASE_REG+23)     ///< Offset for the starting buffer set register
#define INTEL_VVP_VFR_RUN_REG                     (INTEL_VVP_CORE_RT_BASE_REG+24)     ///< Offset for the run mode selection register
#define INTEL_VVP_VFR_FSYNC_PULSE_MODE_REG        (INTEL_VVP_CORE_RT_BASE_REG+25)     ///< Offset for the fsync pulse mode definition register

// Values used in the INTEL_VVP_VFR_BUFFER_MODE_REG register
#define INTEL_VVP_VFR_SINGLE_SET_BUFFER_MODE      0   ///< Use only one set
#define INTEL_VVP_VFR_ALL_SETS_BUFFER_MODE        1   ///< Use all requested sets

// Values used in the INTEL_VVP_VFR_RUN_REG register
#define INTEL_VVP_VFR_STOP                        0   ///< Stop until new commit
#define INTEL_VVP_VFR_FREE_RUNNING                1   ///< Round-robin through the enabled buffer sets and buffers
#define INTEL_VVP_VFR_FSYNC_RUNNING               2   ///< Round-robin through the enabled buffer sets and buffers using fsync to trigger the reading of new frames
#define INTEL_VVP_VFR_SINGLE_SHOT                 3   ///< Use all requested sets and buffers then wait for a new commit

#define INTEL_VVP_VFR_FSYNC_ON_BOTH_EDGES         0   ///< Assume fsync indicates a change and triggers a read on rising and falling edges
#define INTEL_VVP_VFR_FSYNC_ON_RISING_EDGE        1   ///< Assume fsync is a pulse and trigger a read on rising edge

#define INTEL_VVP_VFR_BUFFER_SETS_BASE_REG        (INTEL_VVP_CORE_RT_BASE_REG+28)   ///< Offset for the parameterization of the first buffer set
#define INTEL_VVP_VFR_MAX_BUFFER_SETS_NUM         (16)                              ///< Maximum number of buffer sets allowed (superceded by the IP compile-time parameter)

//

#define INTEL_VVP_VFR_BUFSET_NUM_REGS          (16)      ///< Number of registers reserved to parameterized each buffer set (bufset)

// Base register for a given set of buffers (bufset)
#define INTEL_VVP_VFR_BUFSET_BASE_REG(bufset) (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+((bufset)*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_0_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(0*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_1_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(1*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_2_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(2*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_3_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(3*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_4_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(4*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_5_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(5*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_6_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(6*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_7_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(7*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_8_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(8*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_9_BASE_REG       (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(9*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_10_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(10*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_11_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(11*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_12_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(12*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_13_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(13*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_14_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(14*INTEL_VVP_VFR_BUFSET_NUM_REGS))
#define INTEL_VVP_VFR_BUFSET_15_BASE_REG      (INTEL_VVP_VFR_BUFFER_SETS_BASE_REG+(15*INTEL_VVP_VFR_BUFSET_NUM_REGS))


#define INTEL_VVP_VFR_BUFSET_BASE_ADDR_REG_OFFSET            (0)      ///< Offset for the base address register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_NUM_BUFFERS_REG_OFFSET          (1)      ///< Offset for the number of buffers register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_INTER_BUFFER_OFFSET_REG_OFFSET  (2)      ///< Offset for the inter-buffer byte-offset in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_INTER_LINE_OFFSET_REG_OFFSET    (3)      ///< Offset for the inter-line byte-offset register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_WIDTH_REG_OFFSET                (4)      ///< Offset for the field width register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_HEIGHT_REG_OFFSET               (5)      ///< Offset for the field height register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_INTERLACE_REG_OFFSET            (6)      ///< Offset for the interlace register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_COLORSPACE_REG_OFFSET           (7)      ///< Offset for the colorspace register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_SUBSAMPLING_REG_OFFSET          (8)      ///< Offset for the subsampling register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_COSITING_REG_OFFSET             (9)      ///< Offset for the cositing register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_BPS_REG_OFFSET                  (10)     ///< Offset for the bps register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))
#define INTEL_VVP_VFR_BUFSET_FIELD_COUNT_REG_OFFSET          (11)     ///< Offset for the field count register in a buffer set (offset from INTEL_VVP_VFR_BUFSET_BASE_REG(bufset))

#define INTEL_VVP_VFR_BUFSET_BASE_ADDR_REG(bufset)           (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_BASE_ADDR_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_NUM_BUFFERS_REG(bufset)         (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_NUM_BUFFERS_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_INTER_BUFFER_OFFSET_REG(bufset) (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_INTER_BUFFER_OFFSET_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_INTER_LINE_OFFSET_REG(bufset)   (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_INTER_LINE_OFFSET_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_WIDTH_REG(bufset)               (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_WIDTH_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_HEIGHT_REG(bufset)              (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_HEIGHT_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_INTERLACE_REG(bufset)           (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_INTERLACE_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_COLORSPACE_REG(bufset)          (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_COLORSPACE_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_SUBSAMPLING_REG(bufset)         (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_SUBSAMPLING_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_COSITING_REG(bufset)            (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_COSITING_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_BPS_REG(bufset)                 (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_BPS_REG_OFFSET)
#define INTEL_VVP_VFR_BUFSET_FIELD_COUNT_REG(bufset)         (INTEL_VVP_VFR_BUFSET_BASE_REG(bufset)+INTEL_VVP_VFR_BUFSET_FIELD_COUNT_REG_OFFSET)

#define VFR_REG_IRQ_CONTROL (INTEL_VVP_CORE_IRQ_BASE_REG+0)
#define VFR_REG_IRQ_STATUS (INTEL_VVP_CORE_IRQ_BASE_REG+1)

#endif // __INTEL_VVP_VFR_REGS_H__
