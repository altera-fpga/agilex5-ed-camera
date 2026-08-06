/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace SwApi
{

struct BlackRegion {
    uint16_t h_start;
    uint16_t v_start;
    uint16_t h_end;
    uint16_t v_end;
};

}; // namespace SwApi

std::string ToString(const SwApi::BlackRegion& b);
std::ostream& operator<<(std::ostream& os, const SwApi::BlackRegion& b);
