/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "Tiff_Encoder.h"

#include "Video_Type.h"

//STL include files
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

Tiff_Encoder::Tiff_Encoder(): Encoder_Interface()
{
    bytes_count = 0;
    bytes_per_frame = -1;
    initialise_variables();
}

Tiff_Encoder::~Tiff_Encoder()
{
    close();
}

void Tiff_Encoder::close()
{
    if (_file_stream.is_open())
    {
        _file_stream.close();
    }
}

bool Tiff_Encoder::set_filename(const std::string &filename)
{
    // Save the filename for re-opening if more than one frame is requested
    extension = filename.substr(filename.find_last_of("0123456789") + 1);
    basename = filename.substr(0, filename.find_last_of("0123456789") + 1);
    basename = basename.substr(0, basename.find_last_not_of("0123456789") + 1);

    // filename = basename + %d + extension, with %d being the last number in the file
    if (filename == (basename + extension))
    {
        frame_id = 0;
        extension = filename.substr(filename.find_last_of("."));
        basename = filename.substr(0, filename.find_last_of(".")) + "_";
    }
    else
    {
        sscanf(filename.substr(basename.length(),
                filename.length() - basename.length() - extension.length()).c_str(), "%d", &frame_id);
    }

    return open_filename(filename);
}


bool Tiff_Encoder::open_filename(const std::string &filename)
{
    _file_stream.open(filename.c_str(), std::fstream::out | std::fstream::binary);

    bool open_file_result = (_file_stream.is_open() && _file_stream.good());
    if (!open_file_result)
    {
        _file_stream.close();
    }
    return open_file_result;

}

void Tiff_Encoder::write_tiff_header()
{
    auto result = get_compatible_encoding_methods(_video_type, (_video_type.get_rgb_info()->bytes_per_motif / 3));
    
    if (_video_type.get_fourcc() != Video_Type::RGB_BI_BITFIELDS &&
        _video_type.get_fourcc() != Video_Type::RGB24) 
    {
        throw std::runtime_error("Video Type is not a supported type.");
    }

    int16_t file_header_size = 12;
    tiff_out_info.file_type     = 0x4949;
    tiff_out_info.tiff_conf     = 42;
    tiff_out_info.ifd_offset    = 8;

    tiff_out_info.BitsPerSample = _video_type.get_rgb_info()->bytes_per_motif * 8;
    tiff_out_info.stride        = _video_type.get_stride();
    tiff_out_info.ImageLength   = _video_type.get_height();
    tiff_out_info.ImageWidth    =  tiff_out_info.stride / (tiff_out_info.BitsPerSample >> 3);
    unsigned int image_size     = _video_type.get_stride() * _video_type.get_height();
    
    tiff_out_info.StripOffsets = 360;
    tiff_out_info.SamplesPerPixel = 3;
    tiff_out_info.RowsPerStrip = tiff_out_info.ImageLength;
    tiff_out_info.StripByteCounts = tiff_out_info.stride * tiff_out_info.ImageLength;

    tiff_out_info.ResolutionUnit = 2;

    // Write tiff fileheader & infoheader
    _file_stream.write((char*)&tiff_out_info.file_type, sizeof(tiff_out_info.file_type)); // 0
    _file_stream.write((char*)&tiff_out_info.tiff_conf, sizeof(tiff_out_info.tiff_conf)); // 2
    _file_stream.write((char*)&tiff_out_info.ifd_offset, sizeof(tiff_out_info.ifd_offset)); // 4
    _file_stream.write((char*)&file_header_size, sizeof(file_header_size)); // 8

    int16_t defaultType = 3;
    int32_t defaultCount = 1;

    int16_t ImageWidthTag = 256;
    _file_stream.write((char*)&ImageWidthTag, sizeof(ImageWidthTag)); // 10
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 12
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 14
    _file_stream.write((char*)&tiff_out_info.ImageWidth, sizeof(int32_t)); // 18

    int16_t ImageLengthTag = 257;
    _file_stream.write((char*)&ImageLengthTag, sizeof(ImageLengthTag)); //22
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 24
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); //26
    _file_stream.write((char*)&tiff_out_info.ImageLength, sizeof(int32_t)); //30

    int16_t BitsPerSampleTag = 258;
    int32_t PointerToBPSAddress = 158;
    _file_stream.write((char*)&BitsPerSampleTag, sizeof(BitsPerSampleTag)); // 34
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 36
    _file_stream.write((char*)&tiff_out_info.SamplesPerPixel, sizeof(defaultCount)); // 38
    _file_stream.write((char*)&PointerToBPSAddress, sizeof(int32_t)); // 42
    
    int16_t CompressionTag = 259;
    _file_stream.write((char*)&CompressionTag, sizeof(CompressionTag)); // 46
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 48
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 50
    _file_stream.write((char*)&tiff_out_info.Compression, sizeof(int32_t)); // 54
    
    int16_t PhotometricInterpretationTag = 262;
    _file_stream.write((char*)&PhotometricInterpretationTag, sizeof(PhotometricInterpretationTag)); // 58
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 60
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 62
    _file_stream.write((char*)&tiff_out_info.PhotometricInterpretation, sizeof(int32_t)); // 66
    
    int16_t fourType = 4;

    int16_t StripOffsetsTag = 273;
    _file_stream.write((char*)&StripOffsetsTag, sizeof(StripOffsetsTag)); // 70
    _file_stream.write((char*)&fourType, sizeof(fourType)); // 72
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 74
    _file_stream.write((char*)&tiff_out_info.StripOffsets, sizeof(int32_t)); // 78
    
    int16_t SamplesPerPixelTag = 277;
    _file_stream.write((char*)&SamplesPerPixelTag, sizeof(SamplesPerPixelTag)); // 82
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 84
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 86
    _file_stream.write((char*)&tiff_out_info.SamplesPerPixel, sizeof(int32_t)); // 90
    
    int16_t RowsPerStripTag = 278;
    _file_stream.write((char*)&RowsPerStripTag, sizeof(RowsPerStripTag)); // 94
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 96
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 98
    _file_stream.write((char*)&tiff_out_info.RowsPerStrip, sizeof(int32_t)); // 102
    
    int16_t StripByteCountsTag = 279;
    _file_stream.write((char*)&StripByteCountsTag, sizeof(StripByteCountsTag)); // 106
    _file_stream.write((char*)&fourType, sizeof(fourType)); // 108
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 110
    _file_stream.write((char*)&tiff_out_info.StripByteCounts, sizeof(int32_t)); // 114
    
    int16_t fiveType = 5;

    int16_t XResolutionTag = 282;
    int32_t XResolutionAddress = 164;
    _file_stream.write((char*)&XResolutionTag, sizeof(XResolutionTag)); // 118
    _file_stream.write((char*)&fiveType, sizeof(fiveType)); // 120
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 122
    _file_stream.write((char*)&XResolutionAddress, sizeof(int32_t)); // 126
    
    int16_t YResolutionTag = 283;
    int32_t YResolutionAddress = 172;
    _file_stream.write((char*)&YResolutionTag, sizeof(YResolutionTag)); // 130
    _file_stream.write((char*)&fiveType, sizeof(fiveType)); // 132
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 134
    _file_stream.write((char*)&YResolutionAddress, sizeof(int32_t)); // 138
    
    int16_t ResolutionUnitTag = 296;
    _file_stream.write((char*)&ResolutionUnitTag, sizeof(ResolutionUnitTag)); // 142
    _file_stream.write((char*)&defaultType, sizeof(defaultType)); // 144
    _file_stream.write((char*)&defaultCount, sizeof(defaultCount)); // 146
    _file_stream.write((char*)&tiff_out_info.ResolutionUnit, sizeof(int32_t)); // 150

    
    tiff_out_info.IFDOffset = 0;
    _file_stream.write((char*)&tiff_out_info.IFDOffset, sizeof(tiff_out_info.IFDOffset)); // 154

    uint16_t bpsPerCP = tiff_out_info.BitsPerSample / tiff_out_info.SamplesPerPixel;
    for (int i = 0; i < tiff_out_info.SamplesPerPixel; ++i)
        _file_stream.write((char*)&bpsPerCP, sizeof(bpsPerCP)); // 158, 160, 162

    tiff_out_info.XResolution[0] = 95986;
    tiff_out_info.XResolution[1] = 1000;
    _file_stream.write((char*)&tiff_out_info.XResolution[0], sizeof(int32_t)); // 164
    _file_stream.write((char*)&tiff_out_info.XResolution[1], sizeof(int32_t)); // 168

    tiff_out_info.YResolution[0] = 95986;
    tiff_out_info.YResolution[1] = 1000;
    _file_stream.write((char*)&tiff_out_info.YResolution[0], sizeof(int32_t)); // 172
    _file_stream.write((char*)&tiff_out_info.YResolution[1], sizeof(int32_t)); // 176

    // Reset bytes_per_frame
    bytes_per_frame = image_size;
}

bool Tiff_Encoder::bad() const
{
    return (!_file_stream || _file_stream.bad());
}

std::vector<Encoder_Interface::Encoding_Offer> Tiff_Encoder::get_compatible_encoding_methods(const Video_Type &video_type,
                       unsigned int /*bps*/)
{
    std::vector<Encoder_Interface::Encoding_Offer> result;
    //get compatible offer for each fourcc
    for (std::vector<Encoding_Offer>::const_iterator iter_offers = offers.begin();
         iter_offers != offers.end(); ++iter_offers)
    {
        if (iter_offers->solution.fourcc == video_type.get_fourcc())
        {
            result.emplace_back(*iter_offers);                
        }
    }

    return result;
}


bool Tiff_Encoder::set_encoding_method(const Encoding_Solution &encoding_method,
                                      Video_Type &video_type)
{
    if (video_type.get_fourcc() != encoding_method.fourcc) return false;

    _video_type = video_type;

    // Bottom-top RGB format
    _video_type.set_inversed(false);

    // Little endian ordering for cps
    if (_video_type.get_fourcc() == Video_Type::RGB_BI_BITFIELDS)
        _video_type.set_endianness(Video_Type::L_ENDIAN);
    else _video_type.set_endianness(Video_Type::B_ENDIAN);

    // >8bps support
    if (_video_type.get_fourcc() == Video_Type::RGB_BI_BITFIELDS) {
        std::vector<unsigned char>     red_mask = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
        std::vector<unsigned char>     green_mask = {0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
        std::vector<unsigned char>     blue_mask = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
        _video_type.initRGB(6, red_mask, green_mask, blue_mask, std::vector<unsigned char>());
    }

    return true;
}

unsigned int Tiff_Encoder::store_bytes(const unsigned char * buffer, unsigned int size)
{
    if (!_file_stream) {
        throw std::runtime_error("No file stream available.");
    }

    // Re-open the file for writing when end of file is reached (we waited until there is
    // more data to avoid creating an empty file that will never be written to)
    if (bytes_count == bytes_per_frame)
    {
    	++frame_id;
        _file_stream.close();

        std::stringstream ss;
        ss << basename << frame_id << extension;
        bool res = open_filename(ss.str());
        if (res)
        {
            bytes_count = 0;
        }
        else
        {
        	return 0;
        }
    }

    // Store the tiff header at the beginning of each file
    if (bytes_count == 0)
    {
        write_tiff_header();
    }
    // Write as much as requested but do not go over the frame size
    unsigned int max_size = std::min(bytes_per_frame - bytes_count, size);
    _file_stream.write((char*)buffer, max_size); // 180 onwards

    // Increment bytes_count
    bytes_count += max_size;

    // In all cases return the actual number of bytes written
    return max_size;
}


void Tiff_Encoder::initialise_variables()
{
    // Many tiff types are recognized and can be stored into raw format
    Encoding_Offer encoding_offer;
    encoding_offer.loss = LOSSLESS;

    encoding_offer.solution.name = "Tiff 24-bit";
    encoding_offer.solution.fourcc = Video_Type::RGB24;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 32-bit";
    encoding_offer.solution.fourcc = Video_Type::RGB32;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 48-bit";
    encoding_offer.solution.fourcc = Video_Type::RGB_BI_BITFIELDS;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 2-bit alpha, RGB 10 bit color samples";
    encoding_offer.solution.fourcc = Video_Type::A2R10G10B10;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 2-bit alpha, BGR 10 bit color samples";
    encoding_offer.solution.fourcc = Video_Type::A2B10G10R10;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 32-bit with 8-bit alpha";
    encoding_offer.solution.fourcc = Video_Type::ARGB32;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 15-bit";
    encoding_offer.solution.fourcc = Video_Type::RGB555;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 15-bit with alpha";
    encoding_offer.solution.fourcc = Video_Type::ARGB1555;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 16-bit";
    encoding_offer.solution.fourcc = Video_Type::RGB565;
    offers.emplace_back(encoding_offer);
    encoding_offer.solution.name = "Tiff 16-bit with alpha";
    encoding_offer.solution.fourcc = Video_Type::ARGB4444;
    offers.emplace_back(std::move(encoding_offer));
}

Video_Type Tiff_Encoder::get_video_type()
{
    return _video_type;
}

