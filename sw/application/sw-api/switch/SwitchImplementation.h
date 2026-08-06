/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "ISwitch.h"
#include "SwitchUtils.h"

#include <cstdint>

namespace SwApi 
{

namespace Switch 
{

class SwitchImplementation : public ISwitch 
{
public:
    SwitchImplementation(Hapi::VvpSwitchPtr spSwitch);

    bool IsRunning() override;

    bool SetOutputWidth(uint32_t newWidth) override;
    uint32_t GetOutputWidth() override;

    bool SetOutputHeight(uint32_t newHeight) override;
    uint32_t GetOutputHeight() override;

    uint8_t GetNumberOfInputsAvailable() override;
    uint8_t GetNumberOfOutputsAvailable() override;

    bool ConnectOnly(uint8_t inputChannelNumber, uint8_t outputChannelNumber) override;
    bool ConnectInputToOutput(uint8_t input, uint8_t output) override;

    bool SetInputConfig(uint8_t inputChannelNumber, SwitchInputConfig inputConfig) override;
    SwitchInputConfig GetInputConfig(uint8_t inputChannelNumber) override;

    bool SetOutputConfig(uint8_t outputChannelNumber, SwitchOutputConfig outputConfig) override;
    SwitchOutputConfig GetOutputConfig(uint8_t outputChannelNumber) override;
    bool IsOutputEnabled(uint8_t outputChannelNumber) override;
    uint8_t GetConnectedInputForOutput(uint8_t outputChannelNumber) override;
    std::vector<uint8_t> GetOutputsConnectedToInput(uint8_t inputChannelNumber) override;
private:
    Hapi::VvpSwitchPtr _spSwitch = nullptr;

    uint8_t _numInputs = 1;
    uint8_t _numOutputs = 1;

    uint32_t _outputWidth = 1920;
    uint32_t _outputHeight = 1080;

    std::vector<SwitchInputConfig> _inputConfigs;
    std::vector<SwitchOutputConfig> _outputConfigs;
};


} // namespace Switch


} // namespace SwApi
