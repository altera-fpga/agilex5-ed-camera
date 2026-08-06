/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "LED.h"
#include "AtUtils.h"

bool LED::Enable(uint32_t ledIndex, bool state)
{
    std::string filenameString = AtUtils::FormatString("/sys/class/leds/a10sr-led%d/brightness", ledIndex);
    AtUtils::File ledFile;
    if (ledFile.Open(filenameString, AtUtils::FileOpenFlags::WRITE, false))
    {
        ledFile.Write(state ? '1' : '0');
        return true;
    }
    else
        return false;
}
