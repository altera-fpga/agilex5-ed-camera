/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "SwitchRouter.h"

#define VC_INPUT 0
#define WBC_INPUT 1
#define WBS_INPUT 2

#define WBC_OUTPUT 0
#define WBS_OUTPUT 1
#define DMS_OUTPUT 2

SwitchRouter::SwitchRouter(const std::shared_ptr<SwApi::ISwitch>& spSwitch)
: _spSwitch(spSwitch)
{
    Reset();
}

void SwitchRouter::Reset()
{
    _spSwitch->ConnectInputToOutput(VC_INPUT, WBC_OUTPUT);
    _spSwitch->ConnectInputToOutput(WBC_INPUT, WBS_OUTPUT);
    _spSwitch->ConnectInputToOutput(WBC_INPUT, DMS_OUTPUT);

    _spSwitch->SetInputConfig(WBS_INPUT, SwApi::SwitchInputConfig::SwitchInputConsumed);
}

void SwitchRouter::SetWBSInput(SwitchRouterInput input)
{
    switch (input)
    {
        case SwitchRouterInput::PostWBC:
        {
            _spSwitch->ConnectInputToOutput(WBC_INPUT, WBS_OUTPUT);
            break;
        }
        default:
        case SwitchRouterInput::PreWBC:
        {
            _spSwitch->ConnectInputToOutput(VC_INPUT, WBS_OUTPUT);
            break;
        }
    }
}
