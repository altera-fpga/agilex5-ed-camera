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
#include <iosfwd>
#include <string>

namespace SwApi {

enum class SwitchInputConfig {
    SwitchInputDisabled = kIntelVvpSwitchInputDisabled,
    SwitchInputEnabled = kIntelVvpSwitchInputEnabled,
    SwitchInputConsumed = kIntelVvpSwitchInputConsumed,
    SwitchInputInvalid = kIntelVvpSwitchInputConfigInvalid
};

const char* ToString(const SwitchInputConfig& inputConfig);

struct SwitchOutputConfig {
    bool isEnabled = false;
    uint8_t connectedInput = 0;
};

std::string ToString(const SwitchOutputConfig& outputConfig);

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const SwApi::SwitchInputConfig& inputConfig);
std::ostream& operator<<(std::ostream& o, const SwApi::SwitchOutputConfig& outputConfig);
