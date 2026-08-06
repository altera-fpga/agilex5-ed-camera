/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#pragma once

#include <cstdint>
#include "HapiVvpThrottle.h"

namespace SwApi 
{

class VideoThrottle
{
public:
    VideoThrottle(Hapi::VvpThrottlePtr spVideoThrottle, const uint32_t videoClock);
    ~VideoThrottle();

    void Configure(const uint32_t frameRate, const uint32_t heightActive, const uint32_t blankLines = 0);

private:
    Hapi::VvpThrottlePtr _spVideoThrottle;
    uint32_t _videoClock;
};

} // namespace SwApi