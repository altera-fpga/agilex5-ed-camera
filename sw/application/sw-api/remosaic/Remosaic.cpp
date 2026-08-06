/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "Remosaic.h"

namespace SwApi
{


Remosaic::Remosaic(Hapi::VvpRemosaicPtr remosaic):
    _remosaic{remosaic}
{
}


Remosaic::~Remosaic()
{
}


void Remosaic::SetCfaPhase(const TCfaPhase& phase)
{
    if(_remosaic)
    {
        // RGGB RMS = 10010100 = 0x94   DMS = 0x0
        // GRBG RMS = 01100001 = 0x61   DMS = 0x2
        // GBRG RMS = 01001001 = 0x49   DMS = 0x4 
        // BGGR RMS = 00010110 = 0x16   DMS = 0x6

        uint32_t reg_val = 0x0;

        switch(phase)
        {
            case TCfaPhase::RGGB:
                reg_val = 0x94;
            break;
            case TCfaPhase::GRBG:
                reg_val = 0x61;
            break;
            case TCfaPhase::GBRG:
                reg_val = 0x49;
            break;
            case TCfaPhase::BGGR:
                reg_val = 0x16;
            break;
            default:
            break;
        }

        static constexpr uint32_t REMOSAIC_SETTINGS_REGISTER = 1;

        auto instance = _remosaic->GetInstance();

        if(instance)
            HapiWriteRegister((instance->base), REMOSAIC_SETTINGS_REGISTER, reg_val);
    }
}


}