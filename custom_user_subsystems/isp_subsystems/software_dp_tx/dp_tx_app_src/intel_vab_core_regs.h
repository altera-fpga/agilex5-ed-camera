/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

/**
 * \brief   Common register map definitions for the Intel VAB FPGA IP Suite
 *
 * Common register map definitions for the IP cores of the
 * Video & Vision Processing Intel FPGA IP Suite
 * 
 * \see Intel FPGA Video & Vision IP Suite User Guide
 */

#ifndef __INTEL_VAB_CORE_REGS_H__
#define __INTEL_VAB_CORE_REGS_H__


// Macro setup to build field/flag accessors/setters for VAB cores
#define INTEL_VAB_MACRO_MASK_FIELD(reg_value, CORENAME, REGNAME_FIELD) \
    ((reg_value) & INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK)
#define INTEL_VAB_MACRO_READ_FIELD(reg_value, CORENAME, REGNAME_FIELD) \
    (((reg_value) & INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) >> INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_OFST)
#define INTEL_VAB_MACRO_WRITE_FIELD(reg_value, field_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ( ((reg_value) & ~INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) | (((field_value) << INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_OFST) & INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) ) )
#define INTEL_VAB_MACRO_GET_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( ((reg_value) & INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) != 0 )
#define INTEL_VAB_MACRO_SET_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ((reg_value) | INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) )
#define INTEL_VAB_MACRO_CLEAR_FLAG(reg_value, CORENAME, REGNAME_FIELD) \
    ( (reg_value) = ((reg_value) & ~INTEL_VAB_##CORENAME##_##REGNAME_FIELD##_MSK) )


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VAB_CORE_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VAB_MACRO_MASK_FIELD(reg_value, CORE, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VAB_CORE_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VAB_MACRO_READ_FIELD(reg_value, CORE, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VAB_CORE_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VAB_MACRO_WRITE_FIELD(reg_value, field_value, CORE, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VAB_CORE_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VAB_MACRO_GET_FLAG(reg_value, CORE, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VAB_CORE_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VAB_MACRO_SET_FLAG(reg_value, CORE, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VAB_CORE_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VAB_MACRO_CLEAR_FLAG(reg_value, CORE, REGNAME_FIELD) 


// Compile-time map  (0-63)
#define INTEL_VAB_CORE_COMPILE_TIME_MAP_LENGTH               (64)                 ///< Size of the read-only compile-time parameters area at the start of the register map

#define INTEL_VAB_CORE_VID_PID_REG                           (0)                  ///< Offset of the vendor_id/product_id (vid_pid) register in the register map (read-only compile-time IP parameter)
#define INTEL_VAB_CORE_VID_PID_PRODUCT_ID_MSK                (0x0000FFFF)         ///< Mask for the product_id field in the vendor_id/core_id register
#define INTEL_VAB_CORE_VID_PID_PRODUCT_ID_OFST               (0)                  ///< Offset of the product_id field in the vendor_id/core_id register
#define INTEL_VAB_CORE_VID_PID_VENDOR_ID_MSK                 (0xFFFF0000)         ///< Mask for the vendor_id field in the vendor_id/core_id register
#define INTEL_VAB_CORE_VID_PID_VENDOR_ID_OFST                (16)                 ///< Offset of the vendor_id field in the vendor_id/core_id register

#define INTEL_VAB_CORE_VERSION_REG                           (1)                  ///< Offset of the version register in the register map (read-only compile-time IP parameter)

#define INTEL_VAB_CORE_VERSION_QPDS_MAJOR_MSK                (0xFF000000)         ///< Mask for the major version number in the version register
#define INTEL_VAB_CORE_VERSION_QPDS_MAJOR_OFST               (24)                 ///< Offset of the major version number in the version register
#define INTEL_VAB_CORE_VERSION_QPDS_UPDATE_MSK               (0x00FF0000)         ///< Mask for the update version number in the version register
#define INTEL_VAB_CORE_VERSION_QPDS_UPDATE_OFST              (16)                 ///< Offset of the update version number in the version register
#define INTEL_VAB_CORE_VERSION_QPDS_PATCH_MSK                (0x0000FF00)         ///< Mask for the patch version number in the version register
#define INTEL_VAB_CORE_VERSION_QPDS_PATCH_OFST               (8)                  ///< Offset of the patch version number in the version register
#define INTEL_VAB_CORE_VERSION_REGMAP_VERSION_MSK            (0x000000FF)         ///< Mask for the register map version number in the version register
#define INTEL_VAB_CORE_VERSION_REGMAP_VERSION_OFST           (0)                  ///< Offset of the  register map version number in the version register

// Core specific compile-time parameter map (2-63)
#define INTEL_VAB_CORE_COMPILE_TIME_BASE_REG                 (2)                  ///< Offset for the start of the core-specific read-only compile-time parameters in the register map (2->63)

// IRQ map  (64-71)
#define INTEL_VAB_CORE_IRQ_MAP_LENGTH                        (8)                                                           ///< Size of the interrupt-related area
#define INTEL_VAB_CORE_IRQ_BASE_REG                          (INTEL_VAB_CORE_COMPILE_TIME_MAP_LENGTH)                      ///< Offset for the start of the interrupt-related registers in the register map (64->71)

// Image info map  (71-79)
#define INTEL_VAB_CORE_IMG_INFO_MAP_LENGTH                   (8)                                                           ///< Size of the image info area
#define INTEL_VAB_CORE_IMG_INFO_BASE_REG                     (INTEL_VAB_CORE_IRQ_BASE_REG+INTEL_VAB_CORE_IRQ_MAP_LENGTH)   ///< Offset for the start of the image info registers in the register map (72->79)

#define INTEL_VAB_CORE_IMG_INFO_WIDTH_REG                    (INTEL_VAB_CORE_IMG_INFO_BASE_REG+0)                          ///< Offset for the image info width register in the register map

#define INTEL_VAB_CORE_IMG_INFO_HEIGHT_REG                   (INTEL_VAB_CORE_IMG_INFO_BASE_REG+1)                          ///< Offset for the image info height register in the register map

#define INTEL_VAB_CORE_IMG_INFO_INTERLACE_REG                (INTEL_VAB_CORE_IMG_INFO_BASE_REG+2)                          ///< Offset for the image info interlace register in the register map

#define INTEL_VAB_CORE_IMG_INFO_BPS_REG                      (INTEL_VAB_CORE_IMG_INFO_BASE_REG+3)                          ///< Offset for the image info bps register in the register map

#define INTEL_VAB_CORE_IMG_INFO_COLORSPACE_REG               (INTEL_VAB_CORE_IMG_INFO_BASE_REG+4)                          ///< Offset for the image info colorpsace register in the register map

#define INTEL_VAB_CORE_IMG_INFO_SUBSAMPLING_REG              (INTEL_VAB_CORE_IMG_INFO_BASE_REG+5)                          ///< Offset for the image info subsampling register in the register map

#define INTEL_VAB_CORE_IMG_INFO_COSITING_REG                 (INTEL_VAB_CORE_IMG_INFO_BASE_REG+6)                          ///< Offset for the image info cositing register in the register map

#define INTEL_VAB_CORE_IMG_INFO_FIELD_COUNT_REG              (INTEL_VAB_CORE_IMG_INFO_BASE_REG+7)                          ///< Offset for the image info field count register in the register map (read-only, only in full mode)

// Core-specific run-time map  (72+)
#define INTEL_VAB_CORE_RT_BASE_REG                           (INTEL_VAB_CORE_IMG_INFO_BASE_REG+INTEL_VAB_CORE_IMG_INFO_MAP_LENGTH)  //< Offset for the start of the core-specific run-time area (80+)

    
#endif // __INTEL_VAB_CORE_REGS_H__

