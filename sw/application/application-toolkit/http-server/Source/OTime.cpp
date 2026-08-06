/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "OTime.h"
#include "AppToolkitOS.h"

OTime::OTime()
: _the_time( AtUtils::OS::Get()->GetCTime() )  // Get the current time
{
}

OTime::OTime(time_t initial_time)
: _the_time(initial_time)
{
}

OTime::~OTime()
{
}

std::string OTime::GetRFC1123()
{
    struct tm* utc_time = gmtime(&_the_time);
    if (utc_time)
    {
        char time_str[0x80];
        strftime(time_str, 0x80, "%a, %d %b %Y %H:%M:%S GMT", utc_time);
        return time_str;
    }
    else
        return "";
}

OTime OTime::operator+(const OTimeSpan& time_span)
{
    time_t result_time = _the_time + time_span.GetTotalSeconds();
    return result_time;
}

///////////////

OTimeSpan::OTimeSpan(int hours, int minutes, int seconds)
{
    _seconds = int64_t{seconds} + int64_t{minutes} * 60 + int64_t{hours} * 3600;
}

int64_t OTimeSpan::GetTotalSeconds() const
{
    return _seconds;
}
