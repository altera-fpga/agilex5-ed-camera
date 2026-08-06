/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwitchUtils.h"

#include <sstream>

const char* SwApi::ToString(const SwApi::SwitchInputConfig& inputConfig) {
    switch (inputConfig) {
        case SwApi::SwitchInputConfig::SwitchInputDisabled: {
            return "SwitchInputDisabled";
            break;
        }
        case SwApi::SwitchInputConfig::SwitchInputEnabled: {
            return "SwitchInputEnabled";
            break;
        }
        case SwApi::SwitchInputConfig::SwitchInputConsumed: {
            return "SwitchInputConsumed";
            break;
        }
        case SwApi::SwitchInputConfig::SwitchInputInvalid:
        default: {
            return "SwitchInputInvalid";
            break;
        }
    }
};

std::string SwApi::ToString(const SwitchOutputConfig& outputConfig) {
    std::stringstream ss;
    ss << outputConfig; // Uses the ostream operator<< defined below.
    return ss.str();
};

std::ostream& operator<<(std::ostream& o, const SwApi::SwitchInputConfig& inputConfig) {
    return o << SwApi::ToString(inputConfig);
};
std::ostream& operator<<(std::ostream& o, const SwApi::SwitchOutputConfig& outputConfig) {
    return o << "SwitchOutputConfig(enabled=" << outputConfig.isEnabled
             << ", connectedInput=" << outputConfig.connectedInput << ")";
};
