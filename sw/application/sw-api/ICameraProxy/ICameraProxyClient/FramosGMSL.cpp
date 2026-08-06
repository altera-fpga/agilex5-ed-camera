/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "FramosGMSL.h"

#include <fcntl.h>
#include <unistd.h>
#include <filesystem>

namespace SwApi
{
    bool FramosGMSL::Create(const uint32_t idx)
    {
        bool rc = false;

        std::filesystem::path sPath = std::filesystem::path{"/dev/icamera/framos-gmsl"} / std::to_string(idx);

        int fdDriver = open(sPath.c_str(), O_RDWR);
        if(fdDriver >= 0)
        {
            close(fdDriver);
            rc = true;
        }
        return rc;
    }
}