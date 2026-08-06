/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __SUPPORTED_FORMATS_H__
#define __SUPPORTED_FORMATS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define TX_CLK_6_75     6750000
#define TX_CLK_9_281    9281250
#define TX_CLK_18_5625  18562500
#define TX_CLK_37_125   37125000
#define TX_CLK_74_25    74250000
#define TX_CLK_60_375   60375000
#define TX_CLK_133_25   133250000
#define TX_CLK_148_5    148500000
#define TX_CLK_120_75   120750000
#define TX_CLK_3_375    3375000
#define TX_CLK_234_5    234500000
#define TX_CLK_241_5    241500000
#define TX_CLK_297      297000000

#define TX_CLK_156_125  156125000
#define TX_CLK_73_125   73125000


typedef struct cvo_timing_info
{
    bool interlaced;
    uint32_t sample_count;
    uint32_t f0_line_count;
    uint32_t f1_line_count;
    uint32_t h_front_porch;
    uint32_t h_sync_length;
    uint32_t h_blanking;
    uint32_t v_front_porch;
    uint32_t v_sync_length;
    uint32_t v_blanking;
    uint32_t f0_v_front_porch;
    uint32_t f0_v_sync_length;
    uint32_t f0_v_blanking;
    uint32_t active_picture_line;
    uint32_t f0_v_rising;
    uint32_t field_rising;
    uint32_t field_falling;
    bool h_sync_polarity;
    bool v_sync_polarity;
} cvo_timing_info_t;


typedef struct video_format
{
    uint8_t supported;
    uint8_t vic;
    const char* str;
    cvo_timing_info_t timing;
    uint32_t tx_clk;
    uint16_t fps;       // x100 Hz
} video_format_t;


uint32_t dptx_formats_len();

void dptx_formats_clear_supported();
void dptx_formats_clear_supported_n(uint32_t idx);

void dptx_formats_set_supported(uint32_t width, uint32_t height, uint32_t fps);

void dptx_formats_set_supported_by_vic(uint8_t vic);

uint32_t dptx_formats_is_supported(uint32_t idx);

const video_format_t* dptx_formats_get(uint32_t idx);

uint32_t dptx_formats_get_fallback_idx();

void dptx_formats_print();

uint32_t dptx_min(const uint32_t a, const uint32_t b);
uint32_t dptx_max(const uint32_t a, const uint32_t b);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SUPPORTED_FORMATS_H__ */