/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <ctime>
#include <string>

class OTimeSpan;
class OTime
{
public:
    OTime();
    OTime(time_t initial_time);
    ~OTime();

    std::string GetRFC1123();

    OTime operator+(const OTimeSpan& time_span);

private:
    time_t _the_time;
};

class OTimeSpan
{
public:
    OTimeSpan(int hours, int minutes, int seconds);
    int64_t GetTotalSeconds() const;

private:
    int64_t _seconds;
};
