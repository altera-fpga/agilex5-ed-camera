/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing Clipper Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing Clipper Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_CLIPPER_REGS_H__
#define __INTEL_VVP_CLIPPER_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_CLIPPER_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, CLIPPER, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_CLIPPER_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, CLIPPER, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_CLIPPER_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, CLIPPER, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_CLIPPER_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, CLIPPER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_CLIPPER_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, CLIPPER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_CLIPPER_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, CLIPPER, REGNAME_FIELD) 


// Compile-time map  (0-255)
// LITE_MODE compile-time parameter
#define INTEL_VVP_CLIPPER_LITE_MODE_REG                         (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)         ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)

// DEBUG_ENABLED compile-time parameter
#define INTEL_VVP_CLIPPER_DEBUG_ENABLED_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)         ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)

// CLIPPING_MODE compile-time parameter
#define INTEL_VVP_CLIPPER_CLIPPING_MODE_REG                     (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)         ///< Offset of the clipping_mode register in the register map (read-only compile-time IP parameter)


// clipper-specific run-time parameters
#define INTEL_VVP_CLIPPER_STATUS_REG                            (INTEL_VVP_CORE_RT_BASE_REG+0)                   ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_CLIPPER_STATUS_RUNNING_MSK                    (0x00000001)                                     ///< Mask for the running bit
#define INTEL_VVP_CLIPPER_STATUS_RUNNING_OFST                   (0)                                              ///< Offset for the running bit
#define INTEL_VVP_CLIPPER_STATUS_PENDING_COMMIT_MSK             (0x00000002)                                     ///< Mask for the commit pending bit
#define INTEL_VVP_CLIPPER_STATUS_PENDING_COMMIT_OFST            (1)                                              ///< Offset for the commit pending bit

#define INTEL_VVP_CLIPPER_COMMIT_REG                            (INTEL_VVP_CORE_RT_BASE_REG+1)                   ///< Offset for the commit settings register (full mode)

#define INTEL_VVP_CLIPPER_LEFT_OFFSET_REG                       (INTEL_VVP_CORE_RT_BASE_REG+2)                   ///< Offset for the left-offset register in the register map
    
#define INTEL_VVP_CLIPPER_TOP_OFFSET_REG                        (INTEL_VVP_CORE_RT_BASE_REG+3)                   ///< Offset for the top-offset register in the register map
    
// CLIP_WIDTH and RIGHT_OFFSET alias to the same location, CLIPPING_MODE defines the meaning of the register
#define INTEL_VVP_CLIPPER_RIGHT_OFFSET_REG                      (INTEL_VVP_CORE_RT_BASE_REG+4)                   ///< Offset for the right offset register in the register map (with CLIPPING_MODE == 0 == OFFSET_MODE)
#define INTEL_VVP_CLIPPER_CLIP_WIDTH_REG                        (INTEL_VVP_CORE_RT_BASE_REG+4)                   ///< Offset for the clipping width register in the register map (with CLIPPING_MODE == 1 == RECTANGLE_MODE)

// CLIP_HEIGHT and BOTTOM_OFFSET alias to the same location, RECTANGLE_MODE defines the meaning of the register
#define INTEL_VVP_CLIPPER_BOTTOM_OFFSET_REG                     (INTEL_VVP_CORE_RT_BASE_REG+5)                   ///< Offset for the bottom offset register in the register map (with CLIPPING_MODE == 0 == OFFSET_MODE)
#define INTEL_VVP_CLIPPER_CLIP_HEIGHT_REG                       (INTEL_VVP_CORE_RT_BASE_REG+5)                   ///< Offset for the clipping height register in the register map (with CLIPPING_MODE == 1 == RECTANGLE_MODE)
    
    
#endif // __INTEL_VVP_CLIPPER_REGS_H__

