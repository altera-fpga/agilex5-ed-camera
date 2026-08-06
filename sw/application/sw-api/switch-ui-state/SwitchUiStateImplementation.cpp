/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwitchUiStateImplementation.h"

namespace SwApi {

namespace Switch {

bool SwitchUiStateImplementation::ConnectOnly(uint8_t inputChannelNumber, uint8_t outputChannelNumber)
{
    _uiStateOutputs[outputChannelNumber] = inputChannelNumber;
    return true;
}

uint8_t SwitchUiStateImplementation::GetUiStateInputForOutput(uint8_t output)
{
    return _uiStateOutputs[output];
}

bool SwitchUiStateImplementation::ApplyUiState()
{
    bool ret = true;
    if(_uiStateOutputs.size() == 1)
    {
        ret = SwitchImplementation::ConnectOnly(_uiStateOutputs[0], 0) && ret;
    }
    else
    {
        for(uint8_t output = 0; output < _uiStateOutputs.size(); output++)
        {
            ConnectInputToOutput(_uiStateOutputs[output], output);
        }
    }
    return ret;
}

} // namespace Switch

} // namespace SwApi
