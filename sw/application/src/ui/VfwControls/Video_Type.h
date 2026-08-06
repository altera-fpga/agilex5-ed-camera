/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

//! \brief  The set of fourcc and types that should be recognized by
//          the Converters (raw stream<->ISFrame), contains a RGB_Info or YCBCr_Info
//          structure for packed formats, planar YCBCr formats (or monochrome frames)
//          are completely defined by their fourcc.
//          This class also "converts" the information packed in the original YCbCr fourcc
//          into something that we can handle more easily (eg, is_interlaced,
//          subsampling, ...)
#ifndef _VIDEO_TYPE_H_
#define _VIDEO_TYPE_H_

//STL include files
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <memory>

class Video_Type
{
public:
    /** Fourcc codes for raw image formats, source: http://www.fourcc.org */
    enum FourCC
    {
        /* RGB
         * palettised/RLE format are not supported and have to be "decompressed"
         * by the decoder
         * warning for each line of pixels one might have to be pad the line length
         * in bytes to be a multiple of 4 */
        RGB_BI_BITFIELDS      = 0x00000003, // raw RGB, arbitrary sample packing (init with rgb_info struct)

        RGB555,          // A common bitmap format accessed through BI_RGB, bpp=16
        RGB565,          // A common bitmap format accessed through BI_BITFIELDS and specifying a mask
        RGB24,           // A common bitmap format accessed through BI_RGB, bpp=24
        RGB32,           // A common bitmap format accessed through BI_RGB, bpp=32

        ARGB1555,        // A common bitmap format with alpha layer accessed through RGBA and specifying a mask
        ARGB32,          // A common bitmap format with alpha layer accessed through RGBA and specifying a mask
        ARGB4444,        // A common bitmap format with alpha layer accessed through RGBA and specifying a mask
        A2R10G10B10,     // A common bitmap format with alpha layer accessed through RGBA and specifying a mask
        A2B10G10R10,     // A common bitmap format with alpha layer accessed through RGBA and specifying a mask

        //monochrome => planar
        Y800         = 0x30303859, //monochrome, single Y plane
        Y8           = 0x20203859, //equivalent to Y800 (?)
        GREY         = 0x59455247, //duplicate of Y800 and probably Y8
        LGRY_16      = 0x48799704,

        //The DirectShow RGB types (biCompression field in BitmapInfoHeader)
        //Not that these type are often associated with a little endian byte order

        /* YUV */
        //cyuv         = 0x76757963, //equivalent to UYVY but height is reversed

        //common 4:4:4 (packed pixels)
        AYUV         = 0x56555941, //combined 4:4:4 YUV and alpha, 8 bits per pixel, AYUV ordering
        V655         = 0x35353656, //YUV 16 bits
        IYU2         = 0x32555949, //4:4:4 format U0Y0V0 U1Y1V1
        Y444         = 0x34343459, //equivalent to IYU2
        Y410         = 0x30313459, //equivalent to A2V10Y10U10    (U least significant bits)
        Y416         = 0x36313459, //equivalent to A16V16Y16U16   (U least significant bits)

        //common 4:2:2 (packed pixels)
        UYVY         = 0x59565955, //YUV 4:2:2 in U0Y0V0Y1 U2Y2V2Y3 order
        Y422         = 0x32323459, //equivalent to UYVY
        UYNV         = 0x564E5955, //equivalent to UYVY
        //HDYC         = 0x43594448, //YUV 4:2:2, equivalent to UYVY but using the BT109 color space
        YUYV         = 0x56595559, //YUV 4:2:2 in Y0U0Y1V0 Y2U2Y3V2 order
        YUY2         = 0x32595559, //equivalent YUYV
        YUNV         = 0x564E5559, //equivalent YUYV
        YVYU         = 0x55595659, //YUV 4:2:2, Y0V0Y1U0 Y2V2Y3U2 ordering
        VYUY         = 0x59555956, //YUV 4:2:2, V0Y0U0Y1 ordering
        Y216         = 0x36313259, //equivalent YUYV but with 16 bps
        Y210         = 0x30313259, //equivalent Y216 but 6 bits are not used

        //common 4:1:1 (packed pixels)
        Y411         = 0x31313459, //ordering U0 Y0 Y1 V0 Y2 Y3
        IYU1         = 0x31555949, //equivalent to Y411
        Y41P         = 0x50313459, //ordering U0Y0V0Y1 U4Y2V4Y3 Y4Y5Y6Y7 (then repeat motif)

        //rare 2:1:1 packed
        /* Y211         = 0x31313259, //ordering Y0 U0 Y2 V0 Y4 U4 Y6 V4 ... */

        //explicitly interlaced (0,2,4,...1,3,5,..)
        IUYV         = 0x56595549, //4:2:2 interlaced equivalent to UYVY
        IY41         = 0x31345949, //4:1:1 interlaced equivalent to Y41P

        //upside-down
        //V422         = 0x32323456, //equivalent to UYVY but height reversed (?)

        //exotic, component size != 8 bits
        /* YUVP         = 0x50565559, //YCbCr 4:2:2 (10 bits per components) YUYV ordering */
        CLJR         = 0x524A4C43, //Cirrus Logic format, similar to YUV 4:1:1 but less than 8bits per pix
                                   //ordering Y3(5b) Y2(5b) Y1(5b) Y0(5b) U(6b) V(6b) -> total = 32 bits/4 bytes
        V210         = 0x30313256, //10 bits 4:2:2 YCbCr (quicktime format)
                                   //(4*3 10-bits components are packed in 4*4 little-endian bytes)

        //planar 16:2:0 formats
        /* YVU9         = 0x39555659, //8-bits Y plane then 8-bits V and U planes, both 4*4 subsampled */
        /* YUV9         = 0x39565559, //8-bits Y plane then 8-bits V and U planes, both 4*4 subsampled */
        /* IFO9         = 0x39304649, //equivalent to YVU9 with an extra added delta plane (N/4*M/4 bytes) */

        // 4:2:2 planar formats
        YV16         = 0x36315659, // 8-bits Y plane followed by 8-bits V and U planes both 2*1 subsampled (4:2:2)
        Y42B         = 0x42323459, // Equivalent to YV16 but the U plane is before the V plane

        // 4:2:0 planar formats
        YV12         = 0x32315659, //8-bits Y plane followed by 8-bits V and U planes, both 2*2 subsampled (4:2:0)
        IYUV         = 0x56555949, //equivalent to YV12 but the U plane comes before the V plane
        I420         = 0x30323449, //equivalent to IYUV

        /* CLPL         = 0x4C504C43, //equivalent to IYUV (but one level of indirection in DirectShow?) */
        /* IMC1         = 0x31434D49, //equivalent to IYUV but padding is added so that the lines of U and V planes */
                                   //have same width as the Y plane and their first line starts on a multiple of
                                   //16 (which usually has no effect)
        /* IMC3         = 0x33434D49, //similar to IMC1 swapping the U and V order */


        // I invented the following four fourCC for raw files in JM and HM
        LV10         = 0x3031564C, //10-bits Y plane followed by 10-bits V and U planes, both 2*2 subsampled (4:2:0)
        LV16         = 0x3631564C, //16-bits Y plane followed by 16-bits V and U planes, both 2*2 subsampled (4:2:0)
        LB10         = 0x3031424C, //10-bits Y plane followed by 10-bits U and V planes, both 2*2 subsampled (4:2:0)
        LB16         = 0x3631424C, //16-bits Y plane followed by 16-bits U and V planes, both 2*2 subsampled (4:2:0)
        SV10         = 0x30315653, //10-bits Y plane followed by 10-bits V and U planes, both 2*1 subsampled (4:2:2)
        SV16         = 0x36315653, //16-bits Y plane followed by 16-bits V and U planes, both 2*1 subsampled (4:2:2)
        SB10         = 0x30314253, //10-bits Y plane followed by 10-bits U and V planes, both 2*1 subsampled (4:2:2)
        SB16         = 0x36314253, //16-bits Y plane followed by 16-bits U and V planes, both 2*1 subsampled (4:2:2)

        // I invented the following fourCC for VSS
        VSS1         = 0x31335356, // Macroblock-ordered 8-bit format (samples stored in two bytes) 16x16 macroblock-ordered Y plane followed by interleaved macroblock-ordered 8x8 Cb, 8x8 Cr plane
        VSS2         = 0x32335356, // Macroblock-ordered 10-bit format (samples stored in two bytes) 16x16 macroblock-ordered Y plane, followed by interleaved macroblock-ordered 8x8 Cb, 8x8 Cr plane
        VST1         = 0x31345356, // Macroblock-ordered 8-bit format (samples stored in two bytes) 16x16 macroblock-ordered Y plane, followed by interleaved macroblock-ordered 8x16 Cb, 8x16 Cr plane
        VST2         = 0x32345356, // Macroblock-ordered 10-bit format (samples stored in two bytes) 16x16 macroblock-ordered Y plane, followed by interleaved macroblock-ordered 8x16 Cb, 8x16 Cr plane

        //4:1:1 planar format
        Y41B         = 0x42313459,

        //planar&packed formats
        NV12         = 0x3231564E, //8-bits Y plane followed by a Cb+Cr plane (Cb and Cr intertwined) of half the height
        NV21         = 0x3132564E, //same as NV12 but the second plane is a Cr+Cb plane (instead of Cb+Cr).
        IMC2         = 0x32434D49, //8-bits Y plane followed by a "VU" plane with equal width and half-height. Each line
                                   //of the "VU" plane is composed by V samples in the first half and U samples in the
                                   //second half.
        IMC4         = 0x34434D49, //similar to IMC2 swapping the U and V order

        P216         = 0x36313250, // Same as NV12 in 4:2:2 with 16 bits per color samples
        P210         = 0x30313250, // Same as P216 with 10bps, memory map is the same but 6 bits are unused

        P016         = 0x36313050, // Same as NV12 with 16 bits per color samples
        P010         = 0x30313050, // Same as P216 with 10bps, memory map is the same but 6 bits are unused

        FOURCC_UNKNOWN = 0xFFFFFFFF
    };

    /** Image Formats, almost equivalent to the fourcc but without duplicate and
     *  with names that are (slightly) more explicit */
    enum Image_Format
    {
        //monochrome format (a single Y plane)
        MONO,
        MONO_16,

        //RGB formats (with and without alpha channel)
        RGB_555,
        RGB_565,
        RGB_24,
        RGB_32,
        ARGB_32,
        ARGB_1555,
        ARGB_4444,
        ARGB_2101010,
        ABGR_2101010,
        RGB_MASK,      //other RGBs, _rgb_info has to be filled "manually" with the mask
        ARGB_MASK,     //other RGBs, _rgb_info has to be filled "manually" with the mask

        //packed 4:4:4
        AYCbCr,
        CbYCr,           //24 bits CbYCr (stride?)
        Y6Cb5Cr5,        //16 bits YCbCr (stride?) but V655 is advertised as 4:2:2 ???
        ACbYCr_16,
        ACbYCr_2101010,

        //packed 4:2:2
        YCbYCr,
        YCbYCr_10,       //10 bits per component, packed with 16 bps, only the 10 most significant bits are used
        YCbYCr_16,       //16 bits per component
        YCrYCb,
        //YCrYCb_10,       //10 bits per component, packed with 16 bps, only the 10 most significant bits are used
        //YCrYCb_16,       //16 bits per component
        CrYCbY,
        CbYCrY,
        CbYCrY_inv,     //CbYCrY but last line comes first
        Cb10Y10Cr10Y10_LE_6pack, //v210 packing

        //YCbYCr_211, //packed 2:1:1

        //packed 4:1:1
        CbYYCrYY,
        CbYCrYCbYCrYYYYY,
        Y5Y5Y5Y5Cb6Cr6, //variable number of bits per components, warning Y in reverse order Y3Y2Y1Y0CbCr

        //explicitly interlaced, top field first (0,2,4,...1,3,5,..)
        CbYCrY_interlaced,
        CbYCrYCbYCrYYYYY_interlaced,

        //planar & hybrid formats
        //YCrCb_planar_1620,
        //YCrCb_planar_1620_Dstride,  // the Indeo YVU9, with an extra delta plane (w/4,h/4)
        //YCbCr_planar_1620,
        //YCrCb_planar_410,           // stride?
        //YCbCr_planar_410,           // stride?
        YCbCr_planar_411,             // stride?
        YCbCr_planar_420,             // stride?
        YCbCr_planar_420_10,          // stride?
        YCbCr_planar_420_16,          // stride?

        YCrCb_planar_420,             // stride?
        YCrCb_planar_420_10,          // stride?
        YCrCb_planar_420_16,          // stride?

        //YCrCb_planar_420_IMCstride, // lines of the Cb and Cr plane are extended to the total width
        //YCbCr_planar_420_IMCstride, // lines of the Cb and Cr plane are extended to the total width
        YCrCb_planar_422,             // stride?
        YCbCr_planar_422,             // stride?
        YCrCb_planar_422_10,          // stride?
        YCbCr_planar_422_10,          // stride?
        YCrCb_planar_422_16,          // stride?
        YCbCr_planar_422_16,          // stride?

        Y_CbCrP_422,                  // first a Y plane then a Cb+Cr plane (alternating samples)
        Y_CbCrP_422_16,               // Y_CbCrP_422 with 16 bps
        Y_CbCrP_422_10,               // Y_CbCrP_422 with 10 bps
        Y_CrCbP_422,                  // first a Y plane then a Cr+Cb plane (alternating samples)
        Y_CrCbP_422_16,               // Y_CrCbP_422 with 16 bps
        Y_CrCbP_422_10,               // Y_CrCbP_422 with 10 bps

        Y_CbCrP_420,                  // first a Y plane then a Cb+Cr plane (alternating samples)
        Y_CbCrP_420_16,               // Y_CbCrP_420 with 16 bps
        Y_CbCrP_420_10,               // Y_CbCrP_420 with 10 bps
        Y_CrCbP_420,                  // first a Y plane then a Cr+Cb plane (alternating samples)
        Y_CrCbP_420_16,               // Y_CrCbP_420 with 16 bps
        Y_CrCbP_420_10,               // Y_CrCbP_420 with 10 bps

        Y_CbCrH_420,                  // Cr occupies second half of each line and Cb first half
        Y_CrCbH_420,                  // Cr occupies first half of each line and Cb second half


        YCbCr_mb_420_8,               // Macroblock-ordered 8-bit format (stored in two bytes) 16x16 Y, 8x8 Cb, 8x8 Cr
        YCbCr_mb_422_8,               // Macroblock-ordered 8-bit format (stored in two bytes) 16x16 Y, 8x16 Cb, 8x16 Cr
        YCbCr_mb_420_10,              // Macroblock-ordered 10-bit format (stored in two bytes) 16x16 Y, 8x8 Cb, 8x8 Cr
        YCbCr_mb_422_10               // Macroblock-ordered 10-bit format (stored in two bytes) 16x16 Y, 8x16 Cb, 8x16 Cr
    };

    enum Subsampling
    {
        S_NONE,
        //S_211,  not supported yet...
        //S_1620, not supported yet...
        //S_410, //no corresponding fourcc yet
        S_411,
        S_420,
        S_422,
        S_444,
        S_NOT_INITIALIZED
    };

    enum Colourspace
    {
        CS_RGB,
        CS_YCbCr,
        CS_MONO,
        CS_NOT_INITIALIZED
    };

    //A mask is a vector of character, its size is the size of the motif
    typedef std::vector<unsigned char> Mask;

    //RGBA mask
    struct RGB_Info
    {
        Mask R_mask;
        Mask G_mask;
        Mask B_mask;
        Mask A_mask;
        unsigned int bytes_per_motif;    //must be 2, 3, 4 or 8
    };

    //YCbCr mask, since there might be more than one of each component per motif,
    //declare a set of masks per component
    struct YCbCr_Info
    {
        std::vector<Mask> Y_masks;
        std::vector<Mask> Cb_masks;
        std::vector<Mask> Cr_masks;
        std::vector<Mask> A_masks;
        unsigned int bytes_per_motif;    //must be 2, 3, 4 or 8
        unsigned int pixels_per_motif;
    };

    //Interlace_Flag is used by the convertors raw_image_stream <-> ISframe,
    //with PROGRESSIVE and INTERLACED
    enum Interlace_Flag
    {
        PROGRESSIVE, //the video and the ISframe are in progressive format
        INTERLACED //the video  and the ISframe are both interlaced,
    };

    enum Endianness
    {
        B_ENDIAN,
        L_ENDIAN,
    };

    //! constructor
    //! initialize class variables if necessary
    Video_Type();

    //! destructor
    //! free memory associated with _rgb_info and _ycbcr_info if necessary
    ~Video_Type();

    //! initialise the object according to a FourCC, in case it is a RGB format,
    //! the user is also responsible for the initialisation of the _rgb_info structure
    //! \param[in]  a fourcc, (the unsigned int that corresponds to the 4 letters)
    //! \param[in]  width and height of the frame
    //! \return     false if the fourcc is unknown OR not handled
    bool init(FourCC fourcc, unsigned int width, unsigned int height, unsigned int stride);

    //! initialise the object from a spec file
    // \return  true if initialized properly with a valid spec_file
    bool init(const std::string &spec_file_name);

    //! initialise the RGB info structure (using user's input)
    //! \param[in]  the number of bytes in the motif (usually 2, 3 or 4)
    //! \param[in]  masks, a set of vector of size number_bytes
    bool initRGB(unsigned int number_bytes, const std::vector<unsigned char> &red_mask,
                                  const std::vector<unsigned char> &green_mask,
                                  const std::vector<unsigned char> &blue_mask,
                                  const std::vector<unsigned char> &alpha_mask);

    //! get the width of the frame
    //! \return  the width
    //! \precondition  init was called and returned true
    unsigned int get_width() const
    {
        return _width;
    }

    //! get the height of the frame
    //! \return  the height
    //! \precondition  init was called and returned true
    unsigned int get_height() const
    {
        return _height;
    }

    //! get the stride for the frames
    //! \return  the stride (number of bytes per line)
    //! \precondition  init was called and returned true
    unsigned int get_stride() const
    {
        return _stride;
    }

    //! set the stride for the frames
    //! \param[in]  the stride (number of bytes per line)
    //! \precondition  init was called and returned true
    void set_stride(unsigned int new_stride)
    {
        _stride = new_stride;
    }

    //! \return true if last line comes first
    bool is_inversed() const
    {
        return _inverse_flag;
    }

    //! \return true if last line comes first
    void set_inversed(bool inverse_flag)
    {
        _inverse_flag = inverse_flag;
    }

    //! is_interlaced returns true if the ISFrames are or have to be created interlaced during
    //! a conversion to/from a raw image stream, the original or destination video is interlaced
    //! if, and only if, get_interlace()==INTERLACED, the four other possible flags when it is
    //! necessary to perform a conversion between interlaced ISFrames and progressive video files.
    //! \return        false if _interlace_flag==PROGRESSIVE, true in case of real or simulated interlace
    //! \precondition  init was called and returned true
    bool is_interlaced() const
    {
        return (_interlace_flag!=PROGRESSIVE);
    }

    //! get_interlace, get the value of the current _interlace_flag
    Interlace_Flag get_interlace() const
    {
        return _interlace_flag;
    }

    //! set_interlace, override the info from the fourcc and set a new interlace flag
    //! \param[in]        the new interlace_flag value
    //! \param[in]        bottom_field_first (opt.), true if the bottom field comes
    //!                   first temporally and in the raw stream(ONLY with INTERLACED)
    //! \return           true if the change is allowed
    bool set_interlace(Interlace_Flag new_interlace_flag, bool bottom_field_first = false);

    //! is the odd field (lines 1,3,5,...) temporally earlier than the
    //! even field (lines 0,2,4,...) ?
    //! \return  _bottom_field_first, true if the bottom field is first
    //!          temporally
    bool bottom_field_first() const
    {
        return _bottom_field_first;
    }

    //! get_frame_rate, retrieve the frame rate for display purpose
    //! \return        the frame_rate previously stored
    double get_frame_rate() const
    {
        return _frame_rate;
    }

    //! set_frame_rate, store the frame rate for display purpose
    //! \input        the new frame_rate value
    void set_frame_rate(double frame_rate)
    {
        _frame_rate = frame_rate;
    }

    //! get endianess
    //! little endian means that
    Endianness endianness() const
    {
        return _endianness;
    }

    //! get endianess
    //! little endian means that high order bytes are last
    void set_endianness(Endianness endianness)
    {
        _endianness = endianness;
    }

    //! rgb format?
    //! \return  true if rgb format
    //! \precondition  init was called and returned true
    bool is_rgb() const
    {
        return _colourspace == CS_RGB;
    }

    //! ycbcr format?
    //! \return  true if ycbcr format
    //! \precondition  init was called and returned true
    bool is_ycbcr() const
    {
        return _colourspace == CS_YCbCr;
    }

    //! mono format?
    //! \return  true if monochrome plane
    //! \precondition  init was called and returned true
    bool is_mono() const
    {
        return _colourspace == CS_MONO;
    }

    //! planar format?
    //! \return  true if this is a planar format (also return true for hybrid plane/packed format
    //! \precondition  init was called and returned true
    bool is_planar() const
    {
        return _is_planar;
    }

    //! macroblock format?
    //! \return  true if this is a macroblock ordered format
    //! \precondition  init was called and returned true
    bool is_macroblock() const
    {
        return _is_macroblock;
    }

    //! subsampling?
    //! \return  SubSampling variable
    //! \precondition  init was called and returned true
    Subsampling get_subsampling() const
    {
        return _subsampling;
    }

    //! colourspace?
    //! \return  Colourspace variable
    //! \precondition  init was called and returned true
    Colourspace get_colourspace() const
    {
        return _colourspace;
    }


    //! return the image format?
    //! \return image format
    //! \precondition  init was called and returned true
    Image_Format get_image_format() const
    {
        return _image_format;
    }

    //! return the _rgb_info structure (NULL if not RGB format)
    const std::shared_ptr<RGB_Info> get_rgb_info() const
    {
        if (!is_rgb())
            throw std::runtime_error("Cannot obtain RGB data as VidField is not defined as RGB.");
        return _rgb_info;
    }

    //! return the _ycbcr_info structure (NULL if not YCbCr format)
    //! \return   the _ycbcr_info structure, NULL if not a packed YCbCr
    const std::shared_ptr<YCbCr_Info> get_ycbcr_info() const
    {
        if (!is_ycbcr())
            throw std::runtime_error("Cannot obtain YCbCr data as VidField is not defined as YCbCr.");
        if (_is_planar || _is_macroblock)
            throw std::runtime_error("Cannot obtain YCbCr masks as VidField is defined as planar.");
        //no YCbCr masks with planar formats
        return _ycbcr_info;
    }

    //! return the fourcc of the Video_Type
    //! \return       a FourCC
    //! \precondition init has been called
    Video_Type::FourCC get_fourcc() const
    {
        return _fourcc;
    }

    //! return the fourcc of the Video_Type
    //! \return       a FourCC
    //! \precondition init has been called
    const std::string get_fourcc_string() const
    {
        return (fourcc_to_string.find(_fourcc))->second;
    }

    //! print Video_Type info on the given output stream
    //! \return   the _ycbcr_info structure, NULL if not a packed YCbCr
    void print(std::ostream& output_stream) const;

    //! return a FourCC id from a string
    //! \return   a FourCC
    Video_Type::FourCC get_fourcc(const std::string&) const;

    //! return a FourCC id from a string
    //! \return   a FourCC
    const std::string get_fourcc_string(Video_Type::FourCC fourcc) const;

    //! access the maps
    const std::map<FourCC, std::string> &get_fourcc_list() const;
    const std::map<FourCC, Image_Format> &get_image_format_map() const;
    const std::multimap<Image_Format, FourCC> &get_fourcc_multimap() const;

    //! read a file to retrieve the lines parameter = value
    //! \return   a map<string param, string value>
    std::map<std::string,std::string> parse_spec(const std::string &spec_file_name);

    //! parse the string str_mask into vector<uchar> mask
    //! \return   false if str_mask is not following the correct pattern "0xFFFFFF"
    //            or not matching number_bytes
    static bool parse_mask(const char* name, const std::string &str_mask,
                std::vector<unsigned char> &mask, unsigned int number_bytes);

    //! return a set of fourcc that matches the parameters
    //! \return       compatible set of FourCC
    std::vector<Video_Type::FourCC> get_compatible_fourccs(Colourspace cs,
                                                       Subsampling subsampling,
                                                       Interlace_Flag interlace_flag) const;

private:


    //! initialise the RGB info structure (automatically from _image_format)
    //! \precondition _image_format is a RGB type and different than [A]RGB_MASK
    void initRGB();
    //! initialise the YCbCr info structure
    //! called automatically at initialisation if the fourcc is a non-planar YCbCr format
    void initYCbCr();

    //fourcc and corresponding image_format
    FourCC _fourcc;
    Image_Format _image_format;
    //width, height in pixels
    unsigned int _width, _height;
    //stride (in number of bytes)
    unsigned int _stride;
    //the interlace type, can be progressive, interlaced or simulated interlace/deinterlace
    Interlace_Flag _interlace_flag;
    //subsampling
    Subsampling _subsampling;
    //Colourspace, mono, YCbCr or RGB
    Colourspace _colourspace;
    //the _rgb_info structure, used with RGB formats
    std::shared_ptr<RGB_Info> _rgb_info;
    //the _ycbcr_info structure is only used with packed YCbCr formats
    std::shared_ptr<YCbCr_Info> _ycbcr_info;
    //planar format?
    bool _is_planar;
    //macroblock format?
    bool _is_macroblock;

    //this is used as a warning flag to warn that the images are streamed upside down
    bool _inverse_flag;
    //this is used as a flag to warn that the bottom field comes first (temporally
    //and in the stream from/to the decoder
    bool _bottom_field_first;
    //endianess flag
    Endianness _endianness;
    //the frame rate for information purpose or to make the clip
    double _frame_rate;

    //conversions fourcc <-> strings
    std::map<FourCC, std::string> fourcc_to_string;
    std::map<std::string, FourCC> string_to_fourcc;

    //conversion between fourCC and ImageFormat (ImageFormat = fourcc without redundancy)
    std::map<FourCC, Image_Format> image_format_map;
    std::multimap<Image_Format, FourCC> fourcc_multimap;

    std::set<Image_Format> RGB_format_set;
    std::set<Image_Format> YCbCr_format_set;
    std::set<Image_Format> is_interlaced_set;
    std::set<Image_Format> is_planar_set;
    std::set<Image_Format> is_macroblock_set;
    std::map<Image_Format, Subsampling> subsampling_map;

    //! initialise the maps/sets
    void init_structures();
};

#endif // _VIDEO_TYPE_H_
