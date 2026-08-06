/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "SwitchImplementation.h"

#include <vector>

namespace SwApi 
{

namespace Switch 
{

class SwitchUiStateImplementation : public SwitchImplementation 
{
public:
    SwitchUiStateImplementation(Hapi::VvpSwitchPtr spSwitch) : SwitchImplementation(spSwitch)
    {
        _uiStateOutputs.resize(SwitchImplementation::GetNumberOfOutputsAvailable());
    };

    bool ConnectOnly(uint8_t inputChannelNumber, uint8_t outputChannelNumber);
    uint8_t GetUiStateInputForOutput(uint8_t output);
    bool ApplyUiState();

private:
    std::vector<uint8_t> _uiStateOutputs;
};


} // namespace Switch


} // namespace SwApi
