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
#include <filesystem>
#include <string>
#include <vector>
#include "IspVfrInput.h"

namespace SwApi
{

bool IsTiffFileExtension(const std::filesystem::path& path);

VfrImage LoadBasicTiffImage8(const std::filesystem::path& path, std::string& error);

} // namespace SwApi
