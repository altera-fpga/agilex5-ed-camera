/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwitchImplementation.h"
#include "SwitchUtils.h"
#include "intel_vvp_switch.h"

namespace SwApi {

std::shared_ptr<ISwitch> ISwitch::Create(Hapi::VvpSwitchPtr spSwitch) {
    return std::make_shared<Switch::SwitchImplementation>(spSwitch);
}

namespace Switch {

SwitchImplementation::SwitchImplementation(Hapi::VvpSwitchPtr spSwitch)
    : _spSwitch(spSwitch)
    , _numInputs(1)
    , _numOutputs(1)
{
    _numInputs = intel_vvp_switch_get_num_inputs(_spSwitch->GetInstance());
    _numOutputs = intel_vvp_switch_get_num_outputs(_spSwitch->GetInstance());

    // Set up config lists for each input and output.
    _inputConfigs.resize(_numInputs);
    _outputConfigs.resize(_numOutputs);

    for (uint8_t i = 0; i < _numInputs; i++) 
    {
        // Sensibly pre-configure the i-th input.
        this->SetInputConfig(i, DefaultInitialInputConfig);
    }

    for (uint8_t i = 0; i < _numOutputs; i++) 
    {
        // Sensibly pre-configure the i-th output.
        this->SetOutputConfig(i, DefaultInitialOutputConfig);
    }

    intel_vvp_switch_commit_writes(_spSwitch->GetInstance());
}

bool SwitchImplementation::IsRunning() 
{
    return intel_vvp_switch_is_running(_spSwitch->GetInstance());
}

bool SwitchImplementation::SetOutputWidth(uint32_t newWidth) {
    auto ret = intel_vvp_core_set_img_info_width(_spSwitch->GetInstance(), newWidth);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        _outputWidth = newWidth;
        return true;
    } else {
        return false;
    }
}

uint32_t SwitchImplementation::GetOutputWidth() {
    return _outputWidth;
}

bool SwitchImplementation::SetOutputHeight(uint32_t newHeight) {
    auto ret = intel_vvp_core_set_img_info_height(_spSwitch->GetInstance(), newHeight);
    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) {
        _outputHeight = newHeight;
        return true;
    } else {
        return false;
    }
}

uint32_t SwitchImplementation::GetOutputHeight() {
    return _outputHeight;
}

uint8_t SwitchImplementation::GetNumberOfInputsAvailable() {
    return intel_vvp_switch_get_num_inputs(_spSwitch->GetInstance());
}

uint8_t SwitchImplementation::GetNumberOfOutputsAvailable() 
{
    return intel_vvp_switch_get_num_outputs(_spSwitch->GetInstance());
}

bool SwitchImplementation::ConnectOnly(uint8_t chosenInputChannel, uint8_t chosenOutputChannel) 
{
    auto numberOfSwitchInps = this->GetNumberOfInputsAvailable();
    if (numberOfSwitchInps <= chosenInputChannel) 
    {
        return false;
    }

    for (auto inputChan = 0; inputChan < numberOfSwitchInps; inputChan++) 
    {
        // If we're on the chosen input channel, enable it.
        // Otherwise, set it to consume.
        SwitchInputConfig inputChanConfig;
        if (inputChan == chosenInputChannel) 
        {
            inputChanConfig = SwitchInputConfig::SwitchInputEnabled;
        } 
        else 
        {
            inputChanConfig = SwitchInputConfig::SwitchInputConsumed;
        }
        // Apply the configuration
        if (not this->SetInputConfig(inputChan, inputChanConfig)) 
        {
            return false;
        }
    }

    auto numberOfSwitchOutps = this->GetNumberOfOutputsAvailable();
    if (numberOfSwitchOutps <= chosenOutputChannel) 
    {
        return false;
    }

    for (auto outputChan = 0; outputChan < numberOfSwitchOutps; outputChan++) 
    {
        // If we're on the chosen output channel, enable it and connect it to the chosen input.
        // Otherwise, we'll disable and connect to 0.
        SwitchOutputConfig outputChanConfig;
        if (outputChan == chosenOutputChannel) 
        {
            outputChanConfig = {true, chosenInputChannel};
        } 
        else 
        {
            outputChanConfig = {false, 0};
        }
        // Set the output config as applicable
        if (not this->SetOutputConfig(outputChan, outputChanConfig)) 
        {
            return false;
        }
    }
    return true;
}

bool SwitchImplementation::ConnectInputToOutput(uint8_t input, uint8_t output)
{
    uint8_t numberOfSwitchInps = GetNumberOfInputsAvailable();
    if (numberOfSwitchInps <= input) 
    {
        return false;
    }
    uint8_t numberOfSwitchOuts = GetNumberOfOutputsAvailable();
    if (numberOfSwitchOuts <= output) 
    {
        return false;
    }
    bool success = true;

    if (GetInputConfig(input) != SwitchInputConfig::SwitchInputEnabled)
    {
        success &= SetInputConfig(input, SwitchInputConfig::SwitchInputEnabled);
    }

    SwitchOutputConfig config = {true, input};
    success &= SetOutputConfig(output, config);

    return success;
}

bool SwitchImplementation::SetInputConfig(uint8_t inputChannelNumber, SwitchInputConfig inputConfig)  
{
    auto ret = intel_vvp_switch_set_input_config(_spSwitch->GetInstance(),
                                                 inputChannelNumber,
                                                 (eIntelVvpSwitchInputConfig)inputConfig);

    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) 
    {
        intel_vvp_switch_commit_writes(_spSwitch->GetInstance());
        _inputConfigs[inputChannelNumber] = inputConfig;
        return true;
    } 
    else 
    {
        return false;
    }
}

SwitchInputConfig SwitchImplementation::GetInputConfig(uint8_t inputChannelNumber)  
{
    return _inputConfigs[inputChannelNumber];
}

bool SwitchImplementation::SetOutputConfig(uint8_t outputChannelNumber, SwitchOutputConfig outputConfig)  
{
    auto ret = intel_vvp_switch_set_output_config(_spSwitch->GetInstance(),
                                                  outputChannelNumber,
                                                  outputConfig.isEnabled,
                                                  outputConfig.connectedInput);

    bool succeeded = (ret == kIntelVvpCoreOk);
    if (succeeded) 
    {
        intel_vvp_switch_commit_writes(_spSwitch->GetInstance());
        _outputConfigs[outputChannelNumber] = outputConfig;
        return true;
    } 
    else 
    {
        return false;
    }
}

SwitchOutputConfig SwitchImplementation::GetOutputConfig(uint8_t outputChannelNumber)  
{
    return _outputConfigs[outputChannelNumber];
}

bool SwitchImplementation::IsOutputEnabled(uint8_t outputChannelNumber)  
{
    return GetOutputConfig(outputChannelNumber).isEnabled;
}

uint8_t SwitchImplementation::GetConnectedInputForOutput(uint8_t outputChannelNumber)  
{
    return GetOutputConfig(outputChannelNumber).connectedInput;
}

std::vector<uint8_t> SwitchImplementation::GetOutputsConnectedToInput(uint8_t inputChannelNumber)  
{
    std::vector<uint8_t> outputsConnected;
    for (int i = 0; i < _numOutputs; i++)
    {
        if (GetConnectedInputForOutput(i) == inputChannelNumber)
        {
            outputsConnected.push_back(i);
        }
    }
    return outputsConnected;
}

} // namespace Switch

} // namespace SwApi
