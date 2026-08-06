/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <iosfwd>
#include <cstdint>

namespace SwApi {

enum class DpcSensitivityLevel : uint8_t {
    VeryWeak = 0b00,
    Weak = 0b01,
    Strong = 0b10,
    VeryStrong = 0b11,
};

const char* ToString(const DpcSensitivityLevel& sensitivity);

} // namespace SwApi

std::ostream& operator<<(std::ostream& o, const SwApi::DpcSensitivityLevel& sensitivity);
