/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpSwitch.h"
#include "SwitchUtils.h"

#include <vector>

namespace SwApi {

/// High-level interface definition for the VVP Switch IP.
///
/// A switch instance has some number of available inputs, and some number of available outputs.
/// Each output can be connected to one of the available inputs, or, left disabled.
/// This is determined by the SwitchOutputConfig that's set for that output -
/// switchOutputConfig.isEnabled sets the state of the output, and
/// switchOutputConfig.connectedInput decides which input it reads from.
///
/// Similarly, each input can be configured individually, from the
/// SwitchInputConfig options - SwitchInputDisabled, SwitchInputEnabled, and
/// SwitchInputConsumed.
struct ISwitch {
    static std::shared_ptr<ISwitch> Create(Hapi::VvpSwitchPtr spSwitch);
    virtual ~ISwitch() {};

    virtual bool IsRunning() = 0;

    virtual bool SetOutputWidth(uint32_t newWidth) = 0;
    virtual uint32_t GetOutputWidth() = 0;

    virtual bool SetOutputHeight(uint32_t newHeight) = 0;
    virtual uint32_t GetOutputHeight() = 0;

    virtual uint8_t GetNumberOfInputsAvailable() = 0;
    virtual uint8_t GetNumberOfOutputsAvailable() = 0;

    virtual bool ConnectOnly(uint8_t inputChannelNumber, uint8_t outputChannelNumber) = 0;
    virtual bool ConnectInputToOutput(uint8_t input, uint8_t output) = 0;

    virtual bool SetInputConfig(uint8_t inputChannelNumber, SwitchInputConfig config) = 0;
    virtual SwitchInputConfig GetInputConfig(uint8_t inputChannelNumber) = 0;

    virtual bool SetOutputConfig(uint8_t outputChannelNumber, SwitchOutputConfig outputConfig) = 0;
    virtual SwitchOutputConfig GetOutputConfig(uint8_t outputChannelNumber) = 0;
    virtual bool IsOutputEnabled(uint8_t outputChannelNumber) = 0;
    virtual uint8_t GetConnectedInputForOutput(uint8_t outputChannelNumber) = 0;
    virtual std::vector<uint8_t> GetOutputsConnectedToInput(uint8_t inputChannelNumber) = 0;

    static constexpr auto VVP_SWITCH_PRODUCT_ID = INTEL_VVP_SWITCH_PRODUCT_ID;
    static constexpr SwitchInputConfig DefaultInitialInputConfig
        = SwitchInputConfig::SwitchInputConsumed;
    static constexpr SwitchOutputConfig DefaultInitialOutputConfig = { false, 0 };

};

}
