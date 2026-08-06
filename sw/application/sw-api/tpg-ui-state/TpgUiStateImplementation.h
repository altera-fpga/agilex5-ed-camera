/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "TpgImplementation.h"

#include <cstdint>

namespace SwApi {

namespace Tpg {

class TpgUiStateImplementation : public TpgImplementation {
public:
    TpgUiStateImplementation(Hapi::VvpTpgPtr spTpg,
                      uint32_t initialOutputWidth, uint32_t initialOutputHeight,
                      bool isProgressive = true) : TpgImplementation(spTpg, initialOutputWidth, initialOutputHeight, isProgressive)
    {
        _uiSpecifiedWidth = initialOutputWidth;
        _uiSpecifiedHeight = initialOutputHeight;
    };

    bool SetOutputWidth(uint32_t newWidth);
    bool SetOutputHeight(uint32_t newHeight);

    // New to the TPG Ui State - the UI doesn't control the Width/Height directly - the pipeline does
    uint32_t GetUiStateOutputWidth();
    uint32_t GetUiStateOutputHeight();
    bool ApplyUiStateResolution();

private:
    // New to the TPG Ui State
    uint32_t _uiSpecifiedWidth;
    uint32_t _uiSpecifiedHeight;
};

} // namespace Tpg

} // namespace SwApi
