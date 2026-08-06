/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __ALTERA_VVP_THROTTLE_REGS_H__
#define __ALTERA_VVP_THROTTLE_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register



#define ALTERA_VVP_THROTTLE_STATUS_REG                 (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register in the register map (read-only)
#define ALTERA_VVP_THROTTLE_STATUS_RUNNING_MSK         (0x00000001)                        ///< Mask for the running bit
#define ALTERA_VVP_THROTTLE_STATUS_RUNNING_OFST        (0)                                 ///< Offset for the running bit
#define ALTERA_VVP_THROTTLE_STATUS_PENDING_COMMIT_MSK  (0x00000002)                        ///< Mask for the commit pending bit
#define ALTERA_VVP_THROTTLE_STATUS_PENDING_COMMIT_OFST (1)                                 ///< Offset for the commit pending bit


#define ALTERA_VVP_THROTTLE_CONTROL_REG                (INTEL_VVP_CORE_RT_BASE_REG+1)
#define ALTERA_VVP_THROTTLE_CLEAR_STICKY_REG           (INTEL_VVP_CORE_RT_BASE_REG+2)
#define ALTERA_VVP_THROTTLE_COMMIT_REG                 (INTEL_VVP_CORE_RT_BASE_REG+3)
#define ALTERA_VVP_THROTTLE_CLOCK_CYCLES_PER_LINE_REG  (INTEL_VVP_CORE_RT_BASE_REG+4)
#define ALTERA_VVP_THROTTLE_NUL_V_BLANKING_LINES_REG   (INTEL_VVP_CORE_RT_BASE_REG+5)


#endif // __ALTERA_VVP_THROTTLE_REGS_H__
