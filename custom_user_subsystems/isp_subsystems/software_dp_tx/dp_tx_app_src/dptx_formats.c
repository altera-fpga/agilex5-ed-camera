/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdio.h>
#include "dptx_formats.h"
#include "dptx_app_defs.h"
#include "intel_axi2cv.h"


// Output video formats in order of preference
video_format_t sink_formats[] = {
    {0, EDID_NA_VIC,      "3840x1080p60",     {CVO_3840x1080P60_MODE},     TX_CLK_148_5,    5997},
    {0, EDID_2160P60_VIC, "3840x2160p60",     {CVO_2160P_MODE},            TX_CLK_297,      6000},
    {0, EDID_2160P30_VIC, "3840x2160p30",     {CVO_2160P_MODE},            TX_CLK_148_5,    3000},
    {0, EDID_1080P60_VIC, "1920x1080p60",     {CVO_1080P_MODE},            TX_CLK_74_25,    6000},
    {0, EDID_1080P30_VIC, "1920x1080p30",     {CVO_1080P_MODE},            TX_CLK_37_125,   3000},
    {0, EDID_720P60_VIC,  "1280x720p60",      {CVO_720P_MODE},             TX_CLK_37_125,   6000},
    {0, EDID_720P30_VIC,  "1280x720p30",      {CVO_720P_MODE},             TX_CLK_18_5625,  3000},
    {0, EDID_NA_VIC,      "5120x1440p34",     {CVO_5120x1440P34_MODE},     TX_CLK_148_5,    5997},
    {0, EDID_NA_VIC,      "3840x1080p34",     {CVO_3840x1080P34_MODE},     TX_CLK_74_25,    5997}
};

// This must point to an entry inside sink_formats array!
static const uint32_t FALLBACK_FORMAT_IDX = 6;


uint32_t dptx_formats_len()
{
    return sizeof(sink_formats) / sizeof(video_format_t);
}


void dptx_formats_clear_supported()
{
    for(uint32_t i = 0; i < dptx_formats_len(); ++i)
        sink_formats[i].supported = 0;
}


void dptx_formats_clear_supported_n(uint32_t idx)
{
    if(idx < dptx_formats_len())
        sink_formats[idx].supported = 0;
}


void dptx_formats_set_supported(uint32_t width, uint32_t height, uint32_t fps)
{
    for(uint32_t i = 0; i < dptx_formats_len(); ++i)
    {
        const uint32_t fps_min = dptx_min(sink_formats[i].fps, fps);
        const uint32_t fps_max = dptx_max(sink_formats[i].fps, fps);

        if((sink_formats[i].timing.sample_count == width) &&
            (sink_formats[i].timing.f0_line_count == height) &&
            ((fps_max - fps_min) < 2))  /* Allow 59.99Hz capable sinks */
        {
            sink_formats[i].supported = 1;
        }
    }
}


void dptx_formats_set_supported_by_vic(uint8_t vic)
{
    for(uint32_t i = 0; i < dptx_formats_len(); ++i)
        if(sink_formats[i].vic == vic)
            sink_formats[i].supported = 1;
}


uint32_t dptx_formats_is_supported(uint32_t idx)
{
    uint32_t ret = 0;

    if((idx < dptx_formats_len()) && (sink_formats[idx].supported))
        ret = 1;
    
    return ret;
}


const video_format_t* dptx_formats_get(uint32_t idx)
{
    return idx < dptx_formats_len() ? &sink_formats[idx] : NULL;
}


uint32_t dptx_formats_get_fallback_idx()
{
    return FALLBACK_FORMAT_IDX;
}


void dptx_formats_print()
{
    for(uint32_t i = 0; i < dptx_formats_len(); ++i)
    {
        printf("%s: %s\n", sink_formats[i].str, sink_formats[i].supported ? "Yes":"No");
    }
}


uint32_t dptx_min(const uint32_t a, const uint32_t b)
{
    return (a < b ? a : b);
}


uint32_t dptx_max(const uint32_t a, const uint32_t b)
{
    return (a > b ? a : b);
}