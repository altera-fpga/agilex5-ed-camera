/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "TpgUiStateImplementation.h"

namespace SwApi {

namespace Tpg {

bool TpgUiStateImplementation::SetOutputWidth(uint32_t newWidth) {
    _uiSpecifiedWidth = newWidth;
    return true;
}


bool TpgUiStateImplementation::SetOutputHeight(uint32_t newHeight) {
    _uiSpecifiedHeight = newHeight;
    return true;
}


uint32_t TpgUiStateImplementation::GetUiStateOutputWidth()
{
    return _uiSpecifiedWidth;
}

uint32_t TpgUiStateImplementation::GetUiStateOutputHeight()
{
    return _uiSpecifiedHeight;
}

bool TpgUiStateImplementation::ApplyUiStateResolution()
{
    TpgImplementation::SetOutputWidth(_uiSpecifiedWidth);
    TpgImplementation::SetOutputHeight(_uiSpecifiedHeight);
    return true;
}

} // namespace Tpg

} // namespace SwApi
