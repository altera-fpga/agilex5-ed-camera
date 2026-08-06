/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "BlsUtils.h"

#include <iostream>
#include <sstream>

std::string ToString(const SwApi::BlackRegion& b)
{
    std::stringstream ss;
    ss << b;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const SwApi::BlackRegion& b)
{
    return os << "Bls::BlackRegion("
              << "h_start: " << b.h_start << ", "
              << "v_start: " << b.v_start << ", "
              << "h_end: " << b.h_end << ", "
              << "v_end: " << b.v_end
              << ")";
}
