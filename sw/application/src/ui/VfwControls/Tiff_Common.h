/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
//! \brief  The headers of a tiff files

#ifndef _TIFF_COMMON_H_
#define _TIFF_COMMON_H_

namespace Tiff_Common
{
    // We are only interested in a subset of the TIFF header info
    struct t_tiff_info {
        int16_t file_type;
        int16_t tiff_conf;
        int32_t ifd_offset;
        
        int32_t NewSubfileType = 0;

        int32_t ImageWidth;
        int32_t ImageLength;
        int16_t BitsPerSample;
        int16_t stride;

        int16_t Compression = 1;
        int16_t PhotometricInterpretation = 2;

        int32_t StripOffsets;
        unsigned int OffsetNo;
        unsigned int OffsetDataSize;
        
        int16_t Orientation = 1;
        int32_t SamplesPerPixel = 1;

        int32_t RowsPerStrip;

        int32_t StripByteCounts;
        unsigned int NoOfStrips;
        unsigned int OffsetStripSize;

        int32_t XResolution[2];
        int32_t YResolution[2];
        int16_t ResolutionUnit;
        int16_t ColorMap;

        int16_t SampleFormat = 1;
        
        int32_t IFDOffset;
    };


    typedef unsigned short int uint16;
    typedef unsigned int uint32;
    struct Tiff_File_Header
    {
        uint16 type;        // must be set to "TIFF" to declare that this is a tiff file
        uint32 size;        // the size of the file in bytes
        uint16 reserved1;   // must equal zero
        uint16 reserved2;   // must also equal zero
        uint32 offset;      // offset from beginning of file to tiff data in bytes
    };

    struct Tiff_Info_Header
    {
        uint32 size;                // must be set to sizeof(TiffInfoHeader)
        uint32 width;               // the width of the image in pixels
        uint32 height;              // the height of the image in pixels
        uint16 planes;              // must be set to zero
        uint16 bpp;                 // 1 = 2 colours (palette), 4 = 16 colours (palette), 8 = 256 colours (palette), 24 = 8 bit RGB

        // everything from here down can be safely set to zero
        uint32 compression;         // compressed tiffs are crazy - set to zero
        uint32 image_size;          // can be set to zero because no compression
        uint32 x_pix_per_metre;     // horizontal pixels per metre on target device, or set to zero
        uint32 y_pix_per_metre;     // vertical pixels per metre on target device, or set to zero
        uint32 colours_used;        // number of colours used, if set to zero this is calculated from bpp
        uint32 colours_important;   // the number of colours which are "important", zero indicates that they all are
    };
};

#endif // _TIFF_COMMON_H_
