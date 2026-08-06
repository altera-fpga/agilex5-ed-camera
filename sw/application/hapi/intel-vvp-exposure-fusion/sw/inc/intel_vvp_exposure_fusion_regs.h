/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

/**
 * \brief Register map definitions for the Video & Vision Processing EXPOSURE_FUSION Intel FPGA IP
 *
 * Register map definitions for the Video & Vision Processing EXPOSURE_FUSION Intel FPGA IP
 * This extends the common register definitions already provided in intel_vvp_core_regs.h
 *
 * \see Intel FPGA Video & Vision IP Suite User Guide
 * \see intel_vvp_core_regs.h
 */

#ifndef __INTEL_VVP_EXPOSURE_FUSION_REGS_H__
#define __INTEL_VVP_EXPOSURE_FUSION_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"

// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_EXPOSURE_FUSION_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, EXPOSURE_FUSION, REGNAME_FIELD)

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_EXPOSURE_FUSION_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, EXPOSURE_FUSION, REGNAME_FIELD)

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_EXPOSURE_FUSION_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, EXPOSURE_FUSION, REGNAME_FIELD)

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_EXPOSURE_FUSION_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, EXPOSURE_FUSION, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_EXPOSURE_FUSION_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, EXPOSURE_FUSION, REGNAME_FIELD)

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_EXPOSURE_FUSION_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, EXPOSURE_FUSION, REGNAME_FIELD)

#define INTEL_VVP_EXPOSURE_FUSION_MODE_REG                    (1)
#define INTEL_VVP_EXPOSURE_FUSION_BLACK_LEVEL_REG             (2)
#define INTEL_VVP_EXPOSURE_FUSION_EXPOSURE_RATIO_REG          (3)
#define INTEL_VVP_EXPOSURE_FUSION_THRESHOLD_REG               (4)

#endif // __INTEL_VVP_EXPOSURE_FUSION_REGS_H__
