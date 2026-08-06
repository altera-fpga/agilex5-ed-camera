/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __INTEL_AXI2CV_MODES_H__
#define __INTEL_AXI2CV_MODES_H__

// Some convenient defines to use when calling the set_output_mode function
//                           interl, width, height, f1_h,      h blanking,     v_blanking,   f0_blanking,  active_line, field_toggles,     sync_pol,
#define CVO_720P_MODE         false,  1280,    720,    0,    110,  40,  370,    5,  5, 30,      0, 0,  0,           26,     0,   0, 0,   true,  true
#define CVO_1080I_MODE         true,  1920,    540,  540,     88,  44,  280,    2,  5, 22,      2, 5, 23,           22,   562, 564, 2,   true,  true
#define CVO_1080P_MODE        false,  1920,   1080,    0,     88,  44,  280,    4,  5, 45,      0, 0,  0,           42,     0,   0, 0,   true,  true
#define CVO_2160P_MODE        false,  3840,   2160,    0,    176,  88,  560,    8, 10, 90,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_2160P_420_MODE    false,  1920,   2160,    0,     88,  44,  280,    8, 10, 90,      0, 0,  0,           84,     0,   0, 0,   true,  true
#define CVO_4320P_MODE        false,  7680,   4320,    0,    552, 176, 1320,   16, 20, 80,      0, 0,  0,           65,     0,   0, 0,   true,  true
#define CVO_4320P_420_MODE    false,  3840,   4320,    0,    276,  88,  660,   16, 20, 80,      0, 0,  0,           65,     0,   0, 0,   true,  true
#define CVO_480P_MODE         false,   720,    480,    0,     16,  62,  138,    9,  6, 45,      0, 0,  0,           43,     0,   0, 0,   false, false

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

#endif /* __INTEL_AXI2CV_MODES_H__ */