/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef _IMAGEREADER_H_
#define _IMAGEREADER_H_

#include <string>
#include <cstdint>

typedef unsigned char uchar;

//////////////////////////////////////////////////////////////////////////////
// Load a .bmp or a .bin file to a channel-major pixel data array
//////////////////////////////////////////////////////////////////////////////

class ImageReader
{
public:
    static float* readBMP(std::string &bmp_file_path, uint32_t &img_depth,
                          uint32_t &img_height, uint32_t &img_width);

    static float* readBMPNCHW(std::string &bmp_file_path, uint32_t &img_depth,
                          uint32_t &img_height, uint32_t &img_width);

    static uint8_t* readBMPuint8(std::string &bmp_file_path, uint32_t &img_depth,
            uint32_t &img_height, uint32_t &img_width, bool flip=true);

    static float* readBIN(std::string &bin_file_path, uint32_t &img_depth,
                          uint32_t &img_height, uint32_t &img_width);
};

#endif    // _IMAGEREADER_H_
