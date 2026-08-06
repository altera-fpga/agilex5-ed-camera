/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "intel_vab_core.h"

#ifndef __INTEL_AXI2CV_H__
#define __INTEL_AXI2CV_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// NOTE: CVO is ClockedVideoOutput

#define INTEL_AXI2CV_PRODUCT_ID                           0x0341u              ///< CVO product ID
#define INTEL_AXI2CV_MIN_SUPPORTED_REGMAP_VERSION         1                    ///< Minimum supported register map version
#define INTEL_AXI2CV_MAX_SUPPORTED_REGMAP_VERSION         1                    ///< Maximum supported register map version

#define INTEL_AXI2CV_REG_IORD(instance, reg)          INTEL_VAB_CORE_REG_IORD((&(instance->core_instance)), (reg))           ///< 3D LUT register read function
#define INTEL_AXI2CV_REG_IOWR(instance, reg, value)   INTEL_VAB_CORE_REG_IOWR((&(instance->core_instance)), (reg), (value))  ///< 3D LUT register write function

// Some convenient defines to use when calling the set_output_mode function
//                              interl, width, height, f1_h,      h blanking,     v_blanking,   f0_blanking,  active_line, field_toggles,     sync_pol,
#define CVO_720P_MODE           false,  1280,    720,    0,    110,  40,  370,    5,  5, 30,      0, 0,  0,           26,     0,   0, 0,   true,  true
#define CVO_1080I_MODE          true,   1920,    540,  540,     88,  44,  280,    2,  5, 22,      2, 5, 23,           22,   562, 564, 2,   true,  true
#define CVO_1080P_MODE          false,  1920,   1080,    0,     88,  44,  280,    4,  5, 45,      0, 0,  0,           42,     0,   0, 0,   true,  true
#define CVO_1440P60_MODE        false,  2560,   1440,    0,    192, 272,  928,    3,  5, 53,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_1440P30_MODE        false,  2560,   1440,    0,    120, 264,  768,    3,  5, 28,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_2160P_MODE          false,  3840,   2160,    0,    176,  88,  560,    8, 10, 90,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_2160P_420_MODE      false,  1920,   2160,    0,     88,  44,  280,    8, 10, 90,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_4320P_MODE          false,  7680,   4320,    0,    552, 176, 1320,   16, 20, 80,      0, 0,  0,           65,     0,   0, 0,   true,  true
#define CVO_4320P_420_MODE      false,  3840,   4320,    0,    276,  88,  660,   16, 20, 80,      0, 0,  0,           65,     0,   0, 0,   true,  true
#define CVO_480P_MODE           false,   720,    480,    0,     16,  62,  138,    9,  6, 45,      0, 0,  0,           43,     0,   0, 0,   false, false

// 32:9 adjusted timing
#define CVO_3840x1080P60_MODE   false,  3840,   1080,    0,     48,  32,  160,    3, 10, 157,     0, 0,  0,           25,     0,   0, 0,   true,  false // Hand adjusted for 297 MHz clk
#define CVO_5120x1440P34_MODE   false,  5120,   1440,    0,     48,  32,  160,    3, 10, 41,      0, 0,  0,           35,     0,   0, 0,   true,  false // CVT-RB reduced blanking TX_CLK_133_25
#define CVO_3840x1080P34_MODE   false,  3840,   1080,    0,     48,  32,  160,    3, 10, 31,      0, 0,  0,           25,     0,   0, 0,   true,  false // CVT-RB reduced blanking TX_CLK_73_125

//Profiles for SDI
//                                interl, width, height, f1_h,      h blanking,  v_blanking,   f0_blanking,  active_line, field_toggles
#define CVO_486I_SDI_MODE           true,  720,     244,  243,             138,          19,            19,           20,   264, 266, 4
#define CVO_576I_SDI_MODE           true,  720,     288,  288,             144,          24,            25,           23,   311, 313, 1
#define CVO_720P_SDI_MODE          false,  1280,    720,    0,             370,          30,             0,           26,     0,   0, 0
#define CVO_1080I_SDI_MODE          true,  1920,    540,  540,             280,          22,            23,           21,   561, 564, 1
#define CVO_1080P_SDI_MODE_2048    false,  2048,   1080,    0,             152,          45,             0,           42,     0,   0, 0
#define CVO_1080P_SDI_MODE_1920    false,  1920,   1080,    0,             280,          45,             0,           42,     0,   0, 0
#define CVO_2160P_SDI_MODE_3840    false,  3840,   2160,    0,             560,          90,             0,           42,     0,   0, 0
#define CVO_2160P_SDI_MODE_4096    false,  4096,   2160,    0,             304,          90,             0,           42,     0,   0, 0

enum CVOModeXControlBit {
    CVO_INTERLACED_OUTPUT      = 0,
    CVO_SEQUENTIAL_OUTPUT      = 1,
};

typedef struct intel_axi2cv_instance_s
{
    intel_vab_core_instance core_instance;           ///< Base intel_vab_core_instance
    bool        _can_debug;
} intel_axi2cv_instance;	   

/**
 * \brief Initialise a AXI2CV instance
 * 
 * Initialization function for an AXI2CV instance.
 * Attempts to initialize the fields of the AXI2CV and its base core
 * 
 * \param[in]    instance, pointer to the intel_axi2cv_instance to initialize
 * \param[in]    base, the accessor for the core (on Nios this is a pointer to the base address of the core)
 * \return        0 success
 *               -1 if the vendor id of the core is not the IntelFPGA vendor ID (0x6AF7).
 *               -2 if the product id of the core is not the CVO product id (0x0165)
 * \remarks      On returning a non-zero error code the instance will not be initialized and
 *               cannot be used further by the application using this driver
 */

int intel_axi2cv_init(intel_axi2cv_instance* instance, intel_vab_core_base base);

/**
 * \brief Get debug status of IP core
 * 
 * Debug status function for an AXI2CV instance.
 * 
 * \param[in]    instance, pointer to the intel_axi2cv_instance
 * \return       true can read(debug) register values
 *               false can NOT read(debug) register values
 */

bool intel_axi2cv_get_debug_enabled(intel_axi2cv_instance* instance);


/**
 * \brief Set the current output mode
 * 
 * Set the current output mode for an AXI2CV instance.
 * 
 * \param[in]    instance, pointer to the intel_axi2cv_instance
 * \param[in]    output_mode, new output mode
 */

 void intel_axi2cv_set_output_mode(intel_axi2cv_instance* instance, unsigned int bank_sel,
                                bool interlaced,
                                unsigned int sample_count, unsigned int f0_line_count, unsigned int f1_line_count,
                                unsigned int h_front_porch, unsigned int h_sync_length, unsigned int h_blanking,
                                unsigned int v_front_porch, unsigned int v_sync_length, unsigned int v_blanking,
                                unsigned int f0_v_front_porch, unsigned int f0_v_sync_length, unsigned int f0_v_blanking,
                                unsigned int active_picture_line, unsigned int f0_v_rising, unsigned int field_rising, unsigned int field_falling,
                                bool h_sync_polarity, bool v_sync_polarity);

/**
 * \brief Set the current output mode for SDI
 * 
 * Set the current output mode for an AXI2CV instance.
 * 
 * \param[in]    instance, pointer to the intel_axi2cv_instance
 * \param[in]    output_mode, new output mode
 */

 void intel_axi2cv_set_output_mode_sdi(intel_axi2cv_instance* instance, unsigned int bank_sel,
                                bool interlaced,
                                unsigned int sample_count, unsigned int f0_line_count, unsigned int f1_line_count,
                                unsigned int h_blanking,
                                unsigned int v_blanking,
                                unsigned int f0_v_blanking,
                                unsigned int active_picture_line, unsigned int f0_v_rising, unsigned int field_rising, unsigned int field_falling,
                                unsigned int vid_std, unsigned int vpid_byte1, unsigned int vpid_byte2, unsigned int vpid_byte3, unsigned int vpid_byte4);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __INTEL_AXI2CV_H__
