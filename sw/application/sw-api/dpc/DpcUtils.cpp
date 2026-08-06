/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "DpcUtils.h"

#include <iostream>

namespace SwApi {

const char* ToString(const DpcSensitivityLevel& sensitivity) {
    switch (sensitivity) {
        case DpcSensitivityLevel::VeryWeak: {
            return "VeryWeak";
        }
        case DpcSensitivityLevel::Weak: {
            return "Weak";
        }
        case DpcSensitivityLevel::Strong: {
            return "Strong";
        }
        case DpcSensitivityLevel::VeryStrong: {
            return "VeryStrong";
        }
        default: {
            return "DpcSensitivityLevelInvalid";
        }
    }
}

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const SwApi::DpcSensitivityLevel& sensitivity) {
    return o << SwApi::ToString(sensitivity);
}
