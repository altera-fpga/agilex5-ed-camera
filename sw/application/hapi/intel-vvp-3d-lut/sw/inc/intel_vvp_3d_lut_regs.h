/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef __INTEL_VVP_3D_LUT_REGS_H__
#define __INTEL_VVP_3D_LUT_REGS_H__

// RO registers
#define INTEL_VVP_3D_LUT_CONFIG_EXTERNAL_MODE         (0x02)
#define INTEL_VVP_3D_LUT_CONFIG_PIXELS_IN_PARALLEL    (0x03)
#define INTEL_VVP_3D_LUT_CONFIG_INPUT_BPS             (0x04)
#define INTEL_VVP_3D_LUT_CONFIG_OUTPUT_BPS            (0x05)
#define INTEL_VVP_3D_LUT_CONFIG_LUT_ALPHA             (0x06)
#define INTEL_VVP_3D_LUT_CONFIG_LUT_DEPTH             (0x07)
#define INTEL_VVP_3D_LUT_CONFIG_LUT_DIMENSION         (0x08)
#define INTEL_VVP_3D_LUT_CONFIG_LUT_DOUBLE_BUFFERED   (0x09)
#define INTEL_VVP_3D_LUT_CONFIG_LUT_CPU_READABLE      (0x0A)

// RW registers
#define INTEL_VVP_3D_LUT_STATUS_REG_OFFSET    (0x50)  // byte 140
#define INTEL_VVP_3D_LUT_COMMIT_REG_OFFSET    (0x51)  // byte 144
#define INTEL_VVP_3D_LUT_CONTROL_REG_OFFSET   (0x52)  // byte 148

   // 2 Control registers, within the register named "Control Reg"
   #define INTEL_VVP_3D_LUT_CONTROL_ENABLE_OFST  (0)     
   #define INTEL_VVP_3D_LUT_CONTROL_ENABLE_MSK   (0x1)

   #define INTEL_VVP_3D_LUT_CONTROL_BUFFER_OFST  (1)
   #define INTEL_VVP_3D_LUT_CONTROL_BUFFER_MSK   (0x2)

// RAM registers
#define INTEL_VVP_3D_LUT_RAM_REGISTER(idx,reg) (0x60 + (4 * idx) + reg)

#define INTEL_VVP_3D_LUT_RAM_CTRL  (0)
#define INTEL_VVP_3D_LUT_RAM_DATA1 (2)
#define INTEL_VVP_3D_LUT_RAM_DATA2 (3)

#define INTEL_VVP_3D_LUT_RAM_CTRL_WRITE     (1<<28)
#define INTEL_VVP_3D_LUT_RAM_CTRL_AUTO_INC  (1<<29)

#define INTEL_VVP_3D_LUT_NUMBER_OF_RAMS (8)

#endif // __INTEL_VVP_3D_LUT_REGS_H__
