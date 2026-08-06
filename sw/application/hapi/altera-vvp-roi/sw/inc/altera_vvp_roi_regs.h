/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing Alpha Channel Generator
 *
 * Register map definitions for the Video & Vision Processing Alpha Channel Generator Altera FPGA IP
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __ALTERA_VVP_ROI_REGS_H__
#define __ALTERA_VVP_ROI_REGS_H__

#include "intel_vvp_core_regs.h"

// Macro setup to build field/flag accessors/setters for VVP cores
#define ALTERA_VVP_MACRO_MASK_FIELD(reg_value, CORENAME, REGNAME_FIELD) \
    ((reg_value) & ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK)
#define ALTERA_VVP_MACRO_READ_FIELD(reg_value, CORENAME, REGNAME_FIELD) \
    (((reg_value) & ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) >> ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_OFST)
#define ALTERA_VVP_MACRO_WRITE_FIELD(reg_value, field_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ( ((reg_value) & ~ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) | (((field_value) << ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_OFST) & ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) ) )
#define ALTERA_VVP_MACRO_GET_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( ((reg_value) & ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) != 0 )
#define ALTERA_VVP_MACRO_SET_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ((reg_value) | ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) )
#define ALTERA_VVP_MACRO_CLEAR_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ((reg_value) & ~ALTERA_VVP_##CORENAME##_##REGNAME_FIELD##_MSK) )

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define ALTERA_VVP_ROI_MASK_FILED(reg_value, REGNAME_FIELD)                     ALTERA_VVP_MACRO_MASK_FIELD(reg_value, ROI, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define ALTERA_VVP_ROI_READ_FIELD(reg_value, REGNAME_FIELD)                     ALTERA_VVP_MACRO_READ_FIELD(reg_value, ROI, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define ALTERA_VVP_ROI_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)       ALTERA_VVP_MACRO_WRITE_FIELD(reg_value, field_value, ROI, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define ALTERA_VVP_ROI_GET_FLAG(reg_value, REGNAME_FIELD)                       ALTERA_VVP_MACRO_GET_FLAG(reg_value, ROI, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define ALTERA_VVP_ROI_SET_FLAG(reg_value, REGNAME_FIELD)                       ALTERA_VVP_MACRO_SET_FLAG(reg_value, ROI, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define ALTERA_VVP_ROI_CLEAR_FLAG(reg_value, REGNAME_FIELD)                     ALTERA_VVP_MACRO_CLEAR_FLAG(reg_value, ROI, REGNAME_FIELD)

// Core-specific registers
#define ALTERA_VVP_ROI_CONTROL_REG                      (0x1)              ///< Offset of the control register
#define ALTERA_VVP_ROI_H_ROI_REG                        (0x2)              ///< Offset of the horizontal region of interest register
#define ALTERA_VVP_ROI_V_ROI_REG                        (0x3)              ///< Offset of the vertical region of interest register

// Core-specific register offsets and masks
#define ALTERA_VVP_ROI_CONTROL_ENABLE_MSK               (0x00000001)       ///< Mask for the enable bit
#define ALTERA_VVP_ROI_CONTROL_ENABLE_OFST              (0)                ///< Offset for the enable bit
#define ALTERA_VVP_ROI_CONTROL_MODE_OFST                (2)                ///< Offset for the mode bits
#define ALTERA_VVP_ROI_CONTROL_MODE_MSK                 (0x0000000C)       ///< Mask for the mode bits

// Core-specific RW register offsets and masks
#define ALTERA_VVP_ROI_PIX_START_OFST                   (0)                ///< Offset for the horizontal region of interest pixel start bits
#define ALTERA_VVP_ROI_PIX_START_MSK                    (0x0000FFFF)       ///< Mask for the horizontal region of interest pixel start bits
#define ALTERA_VVP_ROI_PIX_STOP_OFST                    (16)               ///< Offset for the horizontal region of interest pixel start bits
#define ALTERA_VVP_ROI_PIX_STOP_MSK                     (0xFFFF0000)       ///< Mask for the horizontal region of interest pixel start bits

#endif // __ALTERA_VVP_ROI_REGS_H__
