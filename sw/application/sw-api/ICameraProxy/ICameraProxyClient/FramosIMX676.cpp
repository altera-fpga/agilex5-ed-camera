/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "ICameraProxyClient.h"
#include "FramosIMX676.h"
#include <iostream>
#include <memory>

namespace SwApi
{
    std::shared_ptr<ICamera> FramosImx676::Create(const uint32_t idx, const uint32_t targetFrameRate)
    {
        std::shared_ptr<ICamera> iCamera;
        try{
            iCamera = std::make_shared<ICameraProxyClient>("framos-imx676", idx);
            iCamera->SetTargetFrameRate(targetFrameRate);
        }
        catch(const std::exception& e){
#ifdef DEBUG
            std::cerr << e.what() << "\n";
#endif /* DEBUG */
        };

        return iCamera;
    }
}
