/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "HapiVvpTpg.h"

#include <iosfwd>

const char* ToString(const eIntelVvpTpgPatternType& type);

const char* ToString(const eIntelVvpTpgPatternColor& color);

std::ostream& operator<<(std::ostream& o, eIntelVvpTpgPatternType& patternType);

std::ostream& operator<<(std::ostream& o, const eIntelVvpTpgPatternColor& patternColor);

std::ostream& operator<<(std::ostream& o, const intel_vvp_tpg_pattern& pattern);
