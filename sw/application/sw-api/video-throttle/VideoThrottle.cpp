/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#include <stdexcept>
#include "VideoThrottle.h"

namespace SwApi 
{

VideoThrottle::VideoThrottle(Hapi::VvpThrottlePtr spVideoThrottle, const uint32_t videoClock):
    _spVideoThrottle{spVideoThrottle},
    _videoClock{videoClock}
{
    if(!_spVideoThrottle || !_spVideoThrottle->GetInstance())
        throw std::runtime_error("Invalid VVP Throttle instance");

    altera_vvp_throttle_stop(_spVideoThrottle->GetInstance());
}

VideoThrottle::~VideoThrottle()
{
}

void VideoThrottle::Configure(const uint32_t frameRate, const uint32_t heightActive, const uint32_t blankLines)
{
    const auto instance = _spVideoThrottle->GetInstance();

    if(!instance)
        return;

    if((heightActive == 0) or (frameRate == 0))
        return;

    // Calculae clocks per line based on the requrested frame rate
    // If exceeded the maximm possible value adjust the blank lines to achieve the requested frame rate
    // Only do so if blank lines were not specified.
    static constexpr uint32_t CLOCKS_PER_LINE_MAX = 0x1ffffu;   // HW limit
    const uint32_t clocksPerFrame = _videoClock / frameRate;
    uint32_t clocksPerLine = clocksPerFrame / (heightActive + blankLines);

    uint32_t blankLinesAdj = 0;

    if(clocksPerLine > CLOCKS_PER_LINE_MAX)
    {
        if(blankLines == 0)
        {
            const uint32_t heightTotal = clocksPerFrame / CLOCKS_PER_LINE_MAX;
            blankLinesAdj = heightTotal - heightActive;
        }
        
        clocksPerLine = CLOCKS_PER_LINE_MAX;
    }

    altera_vvp_throttle_stop(instance);
    
    altera_vvp_throttle_set_clock_cycles_per_line(instance, clocksPerLine);
    altera_vvp_throttle_set_active_lines(instance, heightActive);
    altera_vvp_throttle_set_v_blank_lines(instance, blankLinesAdj);
    altera_vvp_throttle_commit_writes(instance);

    altera_vvp_throttle_start(instance);
}

} // namespace SwApi