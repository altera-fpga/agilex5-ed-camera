/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_VVP_MIXER_REGS_H__
#define __INTEL_VVP_MIXER_REGS_H__

// intel_vvp_core_regs.h defines all common registers used by each core in the Intel Video & Vision Processing Suite
#include "intel_vvp_core_regs.h"


// #define   REGNAME_REG                                  <- defines the register word address in the register map
// #define   REGNAME_FIELD_MSK                            <- mask to extract a specific field from the register
// #define   REGNAME_FIELD_OFST                           <- offset to extract a specific field from the register


// Macro to extract FIELD from reg_value by masking out other fields (this assumes reg_value was read from REGNAME and applies reg_value & REGNAME_FIELD_MSK)
#define INTEL_VVP_MIXER_MASK_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_MASK_FIELD(reg_value, MIXER, REGNAME_FIELD) 

// Macro to read FIELD from reg_value by masking out other fields and shifting FIELD down to offset 0 (this assumes reg_value was read from REGNAME and applies (reg_value & REGNAME_FIELD_MSK) >> REGNAME_FIELD_OFST)
#define INTEL_VVP_MIXER_READ_FIELD(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_READ_FIELD(reg_value, MIXER, REGNAME_FIELD) 

// Macro to write FIELD into reg_value leaving other fields untouched (this does reg_value = (reg_value & ~REGNAME_FIELD_MSK) | ((field_value << REGNAME_FIELD_OFST) & REGNAME_FIELD_MSK))
#define INTEL_VVP_MIXER_WRITE_FIELD(reg_value, field_value, REGNAME_FIELD)      INTEL_VVP_MACRO_WRITE_FIELD(reg_value, field_value, MIXER, REGNAME_FIELD) 

// Macro to check whether a flag/field from reg_value is non-zero (this assumes reg_value was read from REGNAME and returns (reg_value & REGNAME_FIELD_MSK) != 0)
#define INTEL_VVP_MIXER_GET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_GET_FLAG(reg_value, MIXER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 1 (SET). This does reg_value = (reg_value | REGNAME_FIELD_MSK)
#define INTEL_VVP_MIXER_SET_FLAG(reg_value, REGNAME_FIELD)                      INTEL_VVP_MACRO_SET_FLAG(reg_value, MIXER, REGNAME_FIELD) 

// Macro to set a flag/field in reg_value. All field bits are set to 0 (CLEAR). This does reg_value = (reg_value & ~REGNAME_FIELD_MSK)
#define INTEL_VVP_MIXER_CLEAR_FLAG(reg_value, REGNAME_FIELD)                    INTEL_VVP_MACRO_CLEAR_FLAG(reg_value, MIXER, REGNAME_FIELD) 

#define INTEL_VVP_MIXER_LITE_MODE_REG               (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+0)    ///< Offset of the lite_mode register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_MIXER_DEBUG_ENABLED_REG           (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+1)    ///< Offset of the debug_enabled register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_MIXER_PIXELS_IN_PARALLEL_REG      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+2)    ///< Offset of the pixels_in_parallel register in the register map (read-only compile-time IP parameter)
#define INTEL_VVP_MIXER_NUM_LAYERS_REG              (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+3)    ///< Offset of the number_layers register in the register map (read-only compile-time IP parameter)

#define INTEL_VVP_MIXER_LAYER1_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4)   ///< Supported blend modes register for layer 1 (first foreground layer)
#define INTEL_VVP_MIXER_LAYER1_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5)   ///< Restricted offsets register for layer 1 (first foreground layer)
#define INTEL_VVP_MIXER_LAYER2_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+6)   ///< Supported blend modes register for layer 2
#define INTEL_VVP_MIXER_LAYER2_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+7)   ///< Restricted offsets register for layer 2
#define INTEL_VVP_MIXER_LAYER3_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+8)   ///< Supported blend modes register for layer 3
#define INTEL_VVP_MIXER_LAYER3_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+9)   ///< Restricted offsets register for layer 3
#define INTEL_VVP_MIXER_LAYER4_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+10)  ///< Supported blend modes register for layer 4
#define INTEL_VVP_MIXER_LAYER4_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+11)  ///< Restricted offsets register for layer 4
#define INTEL_VVP_MIXER_LAYER5_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+12)  ///< Supported blend modes register for layer 5
#define INTEL_VVP_MIXER_LAYER5_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+13)  ///< Restricted offsets register for layer 5
#define INTEL_VVP_MIXER_LAYER6_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+14)  ///< Supported blend modes register for layer 6
#define INTEL_VVP_MIXER_LAYER6_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+15)  ///< Restricted offsets register for layer 6
#define INTEL_VVP_MIXER_LAYER7_SUPPORTED_BLEND_MODES_REG    (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+16)  ///< Supported blend modes register for layer 7
#define INTEL_VVP_MIXER_LAYER7_RESTRICTED_OFFSETS_REG       (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+17)  ///< Restricted offsets register for layer 7

#define INTEL_VVP_MIXER_LAYER_SUPPORTED_BLEND_MODES_REG(layer_id)   (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+4+(((layer_id)-1)*2))    ///< Supported blend modes register for layer layer_id (layer_id in range [1..7])
#define INTEL_VVP_MIXER_LAYER_RESTRICTED_OFFSETS_REG(layer_id)      (INTEL_VVP_CORE_COMPILE_TIME_BASE_REG+5+(((layer_id)-1)*2))    ///< Restricted offsets register for layer layer_id (layer_id in range [1..7])

#define INTEL_VVP_MIXER_STATUS_REG                     (INTEL_VVP_CORE_RT_BASE_REG+0)      ///< Offset for the run-time status register in the register map (read-only)
#define INTEL_VVP_MIXER_STATUS_RUNNING_MSK             (0x00000001)                        ///< Mask for the running bit
#define INTEL_VVP_MIXER_STATUS_RUNNING_OFST            (0)                                 ///< Offset for the running bit
#define INTEL_VVP_MIXER_STATUS_PENDING_COMMIT_MSK      (0x00000002)                        ///< Mask for the commit pending bit
#define INTEL_VVP_MIXER_STATUS_PENDING_COMMIT_OFST     (1)                                 ///< Offset for the commit pending bit

#define INTEL_VVP_MIXER_STATUS_LAYER1_ACTIVE_MSK       (0x00000100)                        ///< Mask for the layer1 (first foreground layer) active bit
#define INTEL_VVP_MIXER_STATUS_LAYER1_ACTIVE_OFST      (8)                                 ///< Offset for the layer1 (first foreground layer) active bit
#define INTEL_VVP_MIXER_STATUS_LAYER2_ACTIVE_MSK       (0x00000200)                        ///< Mask for the layer2 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER2_ACTIVE_OFST      (9)                                 ///< Offset for the layer2 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER3_ACTIVE_MSK       (0x00000400)                        ///< Mask for the layer3 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER3_ACTIVE_OFST      (10)                                ///< Offset for the layer3 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER4_ACTIVE_MSK       (0x00000800)                        ///< Mask for the layer4 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER4_ACTIVE_OFST      (11)                                ///< Offset for the layer4 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER5_ACTIVE_MSK       (0x00001000)                        ///< Mask for the layer5 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER5_ACTIVE_OFST      (12)                                ///< Offset for the layer5 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER6_ACTIVE_MSK       (0x00002000)                        ///< Mask for the layer6 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER6_ACTIVE_OFST      (13)                                ///< Offset for the layer6 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER7_ACTIVE_MSK       (0x00004000)                        ///< Mask for the layer7 active bit
#define INTEL_VVP_MIXER_STATUS_LAYER7_ACTIVE_OFST      (14)                                ///< Offset for the layer7 active bit

#define INTEL_VVP_MIXER_STATUS_LAYER_ACTIVE_OFST(layer_id)    (7 + (layer_id))                                              ///< Offset for the layer layer_id active bit (layer_id in range [1..7])
#define INTEL_VVP_MIXER_STATUS_LAYER_ACTIVE_MSK(layer_id)     (1 << INTEL_VVP_MIXER_STATUS_LAYER_ACTIVE_OFST((layer_id)))   ///< Mask for the layer layer_id active bit (layer_id in range [1..7])


#define INTEL_VVP_MIXER_COMMIT_REG                     (INTEL_VVP_CORE_RT_BASE_REG+1)      ///< Offset for the commit settings register (full mode)

#define INTEL_VVP_MIXER_LAYERS_BASE_REG                (INTEL_VVP_CORE_RT_BASE_REG+2)      ///< Base offset for the foreground layers run-time registers
#define INTEL_VVP_MIXER_LAYER_NUM_REGS                 (7)                                 ///< Number of run-time registers to setup the foreground layers (per layer)
#define INTEL_VVP_MIXER_LAYER1_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(0*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER2_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(1*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER3_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(2*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER4_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(3*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER5_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(4*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER6_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(5*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER7_BASE_REG                (INTEL_VVP_MIXER_LAYERS_BASE_REG+(6*INTEL_VVP_MIXER_LAYER_NUM_REGS))
#define INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)       (INTEL_VVP_MIXER_LAYERS_BASE_REG+(((layer_id)-1)*INTEL_VVP_MIXER_LAYER_NUM_REGS))


#define INTEL_VVP_MIXER_LAYER1_MODE_REG                (INTEL_VVP_MIXER_LAYER1_BASE_REG+0)   ///< Mixing mode register for layer 1
#define INTEL_VVP_MIXER_LAYER1_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER1_BASE_REG+1)   ///< Blending mode register for layer 1
#define INTEL_VVP_MIXER_LAYER1_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER1_BASE_REG+2)   ///< Static alpha value register for layer 1
#define INTEL_VVP_MIXER_LAYER1_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER1_BASE_REG+3)   ///< x-offset register for layer 1
#define INTEL_VVP_MIXER_LAYER1_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER1_BASE_REG+4)   ///< y-offset register for layer 1
#define INTEL_VVP_MIXER_LAYER1_WIDTH_REG               (INTEL_VVP_MIXER_LAYER1_BASE_REG+5)   ///< layer width register for layer 1 (lite mode only)
#define INTEL_VVP_MIXER_LAYER1_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER1_BASE_REG+6)   ///< layer height register for layer 1 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER2_MODE_REG                (INTEL_VVP_MIXER_LAYER2_BASE_REG+0)   ///< Mixing mode register for layer 2
#define INTEL_VVP_MIXER_LAYER2_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER2_BASE_REG+1)   ///< Blending mode register for layer 2
#define INTEL_VVP_MIXER_LAYER2_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER2_BASE_REG+2)   ///< Static alpha value register for layer 2
#define INTEL_VVP_MIXER_LAYER2_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER2_BASE_REG+3)   ///< x-offset register for layer 2
#define INTEL_VVP_MIXER_LAYER2_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER2_BASE_REG+4)   ///< y-offset register for layer 2
#define INTEL_VVP_MIXER_LAYER2_WIDTH_REG               (INTEL_VVP_MIXER_LAYER2_BASE_REG+5)   ///< layer width register for layer 2 (lite mode only)
#define INTEL_VVP_MIXER_LAYER2_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER2_BASE_REG+6)   ///< layer height register for layer 2 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER3_MODE_REG                (INTEL_VVP_MIXER_LAYER3_BASE_REG+0)   ///< Mixing mode register for layer 3
#define INTEL_VVP_MIXER_LAYER3_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER3_BASE_REG+1)   ///< Blending mode register for layer 3
#define INTEL_VVP_MIXER_LAYER3_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER3_BASE_REG+2)   ///< Static alpha value register for layer 3
#define INTEL_VVP_MIXER_LAYER3_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER3_BASE_REG+3)   ///< x-offset register for layer 3
#define INTEL_VVP_MIXER_LAYER3_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER3_BASE_REG+4)   ///< y-offset register for layer 3
#define INTEL_VVP_MIXER_LAYER3_WIDTH_REG               (INTEL_VVP_MIXER_LAYER3_BASE_REG+5)   ///< layer width register for layer 3 (lite mode only)
#define INTEL_VVP_MIXER_LAYER3_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER3_BASE_REG+6)   ///< layer height register for layer 3 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER4_MODE_REG                (INTEL_VVP_MIXER_LAYER4_BASE_REG+0)   ///< Mixing mode register for layer 4
#define INTEL_VVP_MIXER_LAYER4_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER4_BASE_REG+1)   ///< Blending mode register for layer 4
#define INTEL_VVP_MIXER_LAYER4_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER4_BASE_REG+2)   ///< Static alpha value register for layer 4
#define INTEL_VVP_MIXER_LAYER4_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER4_BASE_REG+3)   ///< x-offset register for layer 4
#define INTEL_VVP_MIXER_LAYER4_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER4_BASE_REG+4)   ///< y-offset register for layer 4
#define INTEL_VVP_MIXER_LAYER4_WIDTH_REG               (INTEL_VVP_MIXER_LAYER4_BASE_REG+5)   ///< layer width register for layer 4 (lite mode only)
#define INTEL_VVP_MIXER_LAYER4_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER4_BASE_REG+6)   ///< layer height register for layer 4 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER5_MODE_REG                (INTEL_VVP_MIXER_LAYER5_BASE_REG+0)   ///< Mixing mode register for layer 5
#define INTEL_VVP_MIXER_LAYER5_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER5_BASE_REG+1)   ///< Blending mode register for layer 5
#define INTEL_VVP_MIXER_LAYER5_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER5_BASE_REG+2)   ///< Static alpha value register for layer 5
#define INTEL_VVP_MIXER_LAYER5_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER5_BASE_REG+3)   ///< x-offset register for layer 5
#define INTEL_VVP_MIXER_LAYER5_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER5_BASE_REG+4)   ///< y-offset register for layer 5
#define INTEL_VVP_MIXER_LAYER5_WIDTH_REG               (INTEL_VVP_MIXER_LAYER5_BASE_REG+5)   ///< layer width register for layer 5 (lite mode only)
#define INTEL_VVP_MIXER_LAYER5_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER5_BASE_REG+6)   ///< layer height register for layer 5 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER6_MODE_REG                (INTEL_VVP_MIXER_LAYER6_BASE_REG+0)   ///< Mixing mode register for layer 6
#define INTEL_VVP_MIXER_LAYER6_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER6_BASE_REG+1)   ///< Blending mode register for layer 6
#define INTEL_VVP_MIXER_LAYER6_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER6_BASE_REG+2)   ///< Static alpha value register for layer 6
#define INTEL_VVP_MIXER_LAYER6_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER6_BASE_REG+3)   ///< x-offset register for layer 6
#define INTEL_VVP_MIXER_LAYER6_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER6_BASE_REG+4)   ///< y-offset register for layer 6
#define INTEL_VVP_MIXER_LAYER6_WIDTH_REG               (INTEL_VVP_MIXER_LAYER6_BASE_REG+5)   ///< layer width register for layer 6 (lite mode only)
#define INTEL_VVP_MIXER_LAYER6_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER6_BASE_REG+6)   ///< layer height register for layer 1 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER7_MODE_REG                (INTEL_VVP_MIXER_LAYER7_BASE_REG+0)   ///< Mixing mode register for layer 7
#define INTEL_VVP_MIXER_LAYER7_BLEND_MODE_REG          (INTEL_VVP_MIXER_LAYER7_BASE_REG+1)   ///< Blending mode register for layer 7
#define INTEL_VVP_MIXER_LAYER7_STATIC_ALPHA_REG        (INTEL_VVP_MIXER_LAYER7_BASE_REG+2)   ///< Static alpha value register for layer 7
#define INTEL_VVP_MIXER_LAYER7_H_OFFSET_REG            (INTEL_VVP_MIXER_LAYER7_BASE_REG+3)   ///< x-offset register for layer 7
#define INTEL_VVP_MIXER_LAYER7_V_OFFSET_REG            (INTEL_VVP_MIXER_LAYER7_BASE_REG+4)   ///< y-offset register for layer 7
#define INTEL_VVP_MIXER_LAYER7_WIDTH_REG               (INTEL_VVP_MIXER_LAYER7_BASE_REG+5)   ///< layer width register for layer 7 (lite mode only)
#define INTEL_VVP_MIXER_LAYER7_HEIGHT_REG              (INTEL_VVP_MIXER_LAYER7_BASE_REG+6)   ///< layer height register for layer 7 (lite mode only)
                 
#define INTEL_VVP_MIXER_LAYER_MODE_REG(layer_id)                (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+0)   ///< Mixing mode register for layer layer_id (in the range [1..7])
#define INTEL_VVP_MIXER_LAYER_BLEND_MODE_REG(layer_id)          (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+1)   ///< Blending mode register for layer layer_id (in the range [1..7])
#define INTEL_VVP_MIXER_LAYER_STATIC_ALPHA_REG(layer_id)        (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+2)   ///< Static alpha value register for layer layer_id (in the range [1..7])
#define INTEL_VVP_MIXER_LAYER_H_OFFSET_REG(layer_id)            (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+3)   ///< x-offset register for layer layer_id (in the range [1..7])
#define INTEL_VVP_MIXER_LAYER_V_OFFSET_REG(layer_id)            (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+4)   ///< y-offset register for layer layer_id (in the range [1..7])
#define INTEL_VVP_MIXER_LAYER_WIDTH_REG(layer_id)               (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+5)   ///< layer width register for layer layer_id (in the range [1..7], lite mode only)
#define INTEL_VVP_MIXER_LAYER_HEIGHT_REG(layer_id)              (INTEL_VVP_MIXER_LAYER_BASE_REG(layer_id)+6)   ///< layer height register for layer layer_id (in the range [1..7], lite mode only)


 // Values for INTEL_VVP_MIXER_LAYERx_MODE_REG above
#define INTEL_VVP_MIXER_LAYER_MODE_ENABLE_MSK             (0x00000001)   ///< Mask for enabling a layer
#define INTEL_VVP_MIXER_LAYER_MODE_ENABLE_OFST            (0)            ///< Offset for the enable bit
#define INTEL_VVP_MIXER_LAYER_MODE_CONSUME_MSK            (0x00000002)   ///< Mask for consumming/discarding a layer rather than ovelaying it; the layer must also be enabled
#define INTEL_VVP_MIXER_LAYER_MODE_CONSUME_OFST           (1)            ///< Offset for the consumme bit
#define INTEL_VVP_MIXER_LAYER_MODE_SOFT_START_MSK         (0x00000004)   ///< Mask for soft starting, the layer isn't actually enabled/activated and won't stall other inputs until an initial valid beat is seen
#define INTEL_VVP_MIXER_LAYER_MODE_SOFT_START_OFST        (2)            ///< Offset for the enable bit
//"Hard" Mode: synchronization must happen at the next frame. The output will stop and wait if the layer video is missing. Output may glitch if a layer is enabled before the video is there
//"Soft" mode: synchronization may not happen at the next frame. If the layer is missing, the mixer will proceed with the next output frame without this layer

// Values for INTEL_VVP_MIXER_LAYERx_BLEND_MODE_REG above 
#define INTEL_VVP_MIXER_BLEND_TRANSPARENT         0        ///< Foreground layer is fully transparent and doesn't hide the layers it overlays
#define INTEL_VVP_MIXER_BLEND_OPAQUE              1        ///< Foreground layer is fully opaque
#define INTEL_VVP_MIXER_BLEND_STATIC_ALPHA        2        ///< Foreground layer is using the static
#define INTEL_VVP_MIXER_BLEND_INPUT_ALPHA         3

#endif // __INTEL_VVP_MIXER_REGS_H__
