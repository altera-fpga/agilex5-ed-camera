/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once


#include "HapiItem.h"
#include "IspCommon.h"
#include <memory>
#include "intel_vvp_remosaic.h"


namespace Hapi
{
    using VvpRemosaic = HapiItem<int, intel_vvp_remosaic_instance, intel_vvp_remosaic_init_instance, {FPGA_CAPABILITY::intel_vvp_remosaic}>;
    using VvpRemosaicPtr = std::shared_ptr<VvpRemosaic>;
}

namespace SwApi
{

class Remosaic
{
public:
    Remosaic(Hapi::VvpRemosaicPtr remosaic);
    ~Remosaic();

    void SetCfaPhase(const TCfaPhase& phase);

private:
    Hapi::VvpRemosaicPtr _remosaic;
};

}