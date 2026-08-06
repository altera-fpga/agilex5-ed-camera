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

bool IsPgmFileExtension(const std::filesystem::path& path);

/**
 * @brief Load a PGM (Portable Gray Map) binary format (P5) image.
 * 
 * Parses a PGM P5 format file and returns grayscale image data as 8-bit normalized pixels.
 * The maxval can be 8-bit or 16-bit; all values are normalized to 8-bit range.
 * 
 * @param path Path to the PGM file
 * @param outImage Output VfrImage struct with _width, _height, _channels (1), and _pixels
 * @param error Output error message if loading fails
 * @return True if successfully loaded, false with error message otherwise
 */
//bool LoadBasicPgmImage8(const std::filesystem::path& path, VfrImage& outImage, std::string& error);

VfrImage LoadPgmImage(const std::filesystem::path& path, std::string& error);

} // namespace SwApi
