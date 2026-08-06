/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_AXI2CV_REGS_H__
#define __INTEL_AXI2CV_REGS_H__

#define INTEL_AXI2CV_STATUS_REG                       0x50 //RO
#define INTEL_AXI2CV_VIDEO_MODE_MATCH                 0x51 //RW
#define INTEL_AXI2CV_CONTROL_REG                      0x52 //RW
#define INTEL_AXI2CV_BANK_SELECT                      0x53 //RW
#define INTEL_AXI2CV_MODEX_CONTROL                    0x54 //RW
#define INTEL_AXI2CV_MODEX_SAMPLE_COUNT               0x55 //RW
#define INTEL_AXI2CV_MODEX_F0_LINE_COUNT              0x56 //RW
#define INTEL_AXI2CV_MODEX_F1_LINE_COUNT              0x57 //RW
#define INTEL_AXI2CV_MODEX_HORIZONTAL_FRONT_PORCH     0x58 //RW
#define INTEL_AXI2CV_MODEX_HORIZONTAL_SYNC_LENGTH     0x59 //RW
#define INTEL_AXI2CV_MODEX_HORIZONTAL_BLANKING        0x5A //RW
#define INTEL_AXI2CV_MODEX_VERTICAL_FRONT_PORCH       0x5B //RW
#define INTEL_AXI2CV_MODEX_VERTICAL_SYNC_LENGTH       0x5C //RW
#define INTEL_AXI2CV_MODEX_VERTICAL_BLANKING          0x5D //RW
#define INTEL_AXI2CV_MODEX_F0_VERTICAL_FRONT_PORCH    0x5E //RW
#define INTEL_AXI2CV_MODEX_F0_VERTICAL_SYNC_LENGTH    0x5F //RW
#define INTEL_AXI2CV_MODEX_F0_VERTICAL_BLANKING       0x60 //RW
#define INTEL_AXI2CV_MODEX_ACTIVE_PICTURE_LINE        0x61 //RW
#define INTEL_AXI2CV_MODEX_F0_VERTICAL_RISING         0x62 //RW
#define INTEL_AXI2CV_MODEX_FIELD_RISING               0x63 //RW
#define INTEL_AXI2CV_MODEX_FIELD_FALLING              0x64 //RW
#define INTEL_AXI2CV_MODEX_STANDARD                   0x65 //RW
#define INTEL_AXI2CV_MODEX_VPID_BYTE1                 0x66 //RW
#define INTEL_AXI2CV_MODEX_VPID_BYTE2                 0x67 //RW
#define INTEL_AXI2CV_MODEX_VPID_BYTE3                 0x68 //RW
#define INTEL_AXI2CV_MODEX_VPID_BYTE4                 0x69 //RW
#define INTEL_AXI2CV_MODEX_HSYNC_POLARITY             0x6B //RW
#define INTEL_AXI2CV_MODEX_VSYNC_POLARITY             0x6C //RW
#define INTEL_AXI2CV_MODEX_VALID                      0x6D //RW

#endif // __INTEL_AXI2CV_REGS_H__
