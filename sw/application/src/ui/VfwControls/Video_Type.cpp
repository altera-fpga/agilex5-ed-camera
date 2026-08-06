/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "Video_Type.h"

//STL includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <stdexcept>

Video_Type::Video_Type()
{
    //initialise maps/sets
    init_structures();

    _image_format = Video_Type::MONO;
    _width = 0;
    _height = 0;
    _stride = 0;
    _colourspace = CS_NOT_INITIALIZED;
    _is_planar = false;
    _is_macroblock = false;
    _interlace_flag = PROGRESSIVE;
    _subsampling = S_NOT_INITIALIZED;
    _frame_rate = 0.0;
    _fourcc = FOURCC_UNKNOWN;
    _bottom_field_first = false;
    _inverse_flag = false;
    _endianness = L_ENDIAN;
}

Video_Type::~Video_Type()
{
}

bool Video_Type::init(const std::string &spec_file_name)
{
    std::map<std::string,std::string> spec_params = parse_spec(spec_file_name);
    bool parse_spec_result = !spec_params.empty();

    if (parse_spec_result)
    {
        FourCC fourcc = get_fourcc(spec_params["fourcc"]);
        if (fourcc == Video_Type::FOURCC_UNKNOWN)
        {
            std::cerr << "Error: " << spec_params["fourcc"]
                      << " is not a recognized/handled fourcc" << std::endl;
            parse_spec_result = false;
        }
        else
        {
            unsigned int width = 0;
            unsigned int stride = 0;
            unsigned int height = 0;
            sscanf( spec_params["width"].c_str(), "%d", &width);
            sscanf( spec_params["height"].c_str(), "%d", &height);
            sscanf( spec_params["stride"].c_str(), "%d", &stride);
            if ((width < 1) || (height < 1))
            {
                std::cerr << "Error: cannot parse width or height from the raw spec file" << std::endl;
                parse_spec_result = false;
            }
            if (parse_spec_result)
            {
                parse_spec_result = init(fourcc, width, height, stride);
            }
            if (parse_spec_result && (_image_format == Video_Type::RGB_MASK))
            {
                unsigned int number_bytes;
                sscanf( spec_params["number_bytes_per_sample"].c_str(), "%d", &number_bytes);
                if (number_bytes<1)
                {
                    std::cerr << "Error: " << "cannot find/parse the \"number of bytes per pixel\""
                              << "from the raw spec file" << std::endl;
                    parse_spec_result = false;
                }
                if (parse_spec_result)
                {
                    std::vector<unsigned char> R_mask, G_mask, B_mask, A_mask;
                    A_mask.clear();
                    parse_spec_result = parse_mask("red_mask", spec_params["red_mask"],
                                                                       R_mask, number_bytes);
                    parse_spec_result = parse_spec_result &&
                           parse_mask("green_mask", spec_params["green_mask"],
                                                                       G_mask, number_bytes);
                    parse_spec_result = parse_spec_result &&
                             parse_mask("blue_mask", spec_params["blue_mask"],
                                                                       B_mask, number_bytes);
                    if (spec_params["alpha_mask"] != "")
                    {
                        parse_spec_result = parse_spec_result &&
                            parse_mask("alpha_mask", spec_params["alpha_mask"],
                                                                   A_mask, number_bytes);
                    }
                    if (parse_spec_result)
                    {
                        parse_spec_result = initRGB(number_bytes, R_mask, G_mask, B_mask, A_mask);
                    }
                }
            }
        }
        if (parse_spec_result)
        {
            set_interlace(PROGRESSIVE);
            if ((spec_params["interlaced"][0] == 'Y') || (spec_params["interlaced"][0] == 'y'))
            {
                if ((spec_params["bottom_field_first"][0] == 'Y') || (spec_params["bottom_field_first"][0] == 'y'))
                {
                    set_interlace(INTERLACED, true);
                }
                else
                {
                    set_interlace(INTERLACED, false);
                }
            }
            if ((_height%2) && (_interlace_flag != PROGRESSIVE))
            {
                std::cerr << "Interlaced formats with an odd number of lines are not supported" << std::endl;
                parse_spec_result = false;
            }
            set_endianness(B_ENDIAN); // Default when reading from a file
            if (spec_params["endianness"] == "little_endian")
            {
                set_endianness(L_ENDIAN);
            }
            set_inversed(false);
            if ((spec_params["inversed"][0] == 'Y') || (spec_params["inversed"][0] == 'y'))
            {
                set_inversed(true);
            }
            if (spec_params.find("frame_rate") != spec_params.end()) {
            	float frame_rate;
            	sscanf(spec_params["frame_rate"].c_str(), "%f", &frame_rate);
            	set_frame_rate(frame_rate);
            }
        }
    }
    return parse_spec_result;
}

std::map<std::string,std::string> Video_Type::parse_spec(const std::string &spec_file_name)
{
    std::map<std::string,std::string> map_res;
    map_res.clear();
    std::ifstream spec_file(spec_file_name.c_str());
    if (!spec_file.good())
    {
        std::cerr << "Specification file " << spec_file_name << " for the raw video format not found" << std::endl;
        return map_res; //early return on error
    }

    char buffer[1000];
    char param[1000];
    char value[1000];
    spec_file >> std::ws;
    while (!spec_file.eof())
    {
        spec_file.getline(buffer, 800);
        int res = sscanf(buffer, "%s = %s;", param, value);
        if (res != 2)
        {
            std::cerr << "Specification file " << spec_file_name
                      << ", cannot parse line: \"" << buffer << "\"" <<std::endl;
        }
        else
        {
            char *iter = param;
            while (*iter) tolower(*iter++); //all params are used lowercase
            map_res[param]= value;
        }
        spec_file >> std::ws;
    }
    return map_res;
}

bool Video_Type::parse_mask(const char* name, const std::string &str_mask,
                std::vector<unsigned char> &mask, unsigned int number_bytes)
{
    mask.clear();
    bool parse_result = true;
    if ((str_mask[0] != '0') || (str_mask[1] != 'x') ||
        (str_mask.length() != (number_bytes*2+2)))
    {
        parse_result = false;
    }
    else{
        if (str_mask.find_first_not_of("0123456789ABCDEFabcdef", 2) != std::string::npos)
        {
            parse_result = false;
        }
        int byte_value;   //storage for the future byte
        char str_byte[3]; //string to prepare the 2 hex digits in a byte
        for (unsigned int i = 0; i < number_bytes; ++i)
        {
            //print the byte (2 chars)
            sprintf(str_byte, "%c%c", str_mask[2*i+2], str_mask[2*i+3]);
            //parse the byte as an hex number
            sscanf(str_byte, "%x", &byte_value);
            mask.push_back(static_cast<unsigned char>(byte_value));
        }
    }
    if (parse_result==false){
        std::string correct_pattern = "0x";
        for (unsigned int i = 0; i < number_bytes; ++i)
        {
            correct_pattern += "FF";
        }
        std::cerr << "Error, invalid " << name << ", expected pattern: \""
                  << correct_pattern << "\"" << std::endl;
    }
    return parse_result;
}

bool Video_Type::init(FourCC fourcc, unsigned int width, unsigned int height, unsigned int stride)
{
    _width = width;
    _stride = stride;
    _height = height;
    _fourcc = fourcc;
    std::map< FourCC, Image_Format >::iterator iter_search =
                                                   image_format_map.find(_fourcc);                                                  ;
    if(iter_search == image_format_map.end())
    {
        return false; //early return; FourCC is not handled
    }
    _image_format = iter_search->second; //FourCC recognized, store the Image_format
    
    if (RGB_format_set.find(_image_format) != RGB_format_set.end())
    {
        _colourspace = CS_RGB;
        _subsampling = S_NONE;
        //initialise RGB if type is known
        if (_image_format!=RGB_MASK)
        {
            initRGB(); //stride initialised in initRGB if it is not given
        }

        //_subsampling, _interlaced and _is_planar are properly initialised at construction
    }
    else{
        if ((_image_format==MONO) || (_image_format==MONO_16))
        {
            _colourspace = CS_MONO;
            _subsampling = S_NONE;
            _is_planar = true;
            if (!_stride) _stride = _width; //init stride if not given
        }
        else
        {
            _colourspace = CS_YCbCr;
            //All YCbCr formats have associated subsampling, can be interlaced and planar
            _subsampling = subsampling_map[_image_format];
            if (is_interlaced_set.find(_image_format) != is_interlaced_set.end())
            {
                _interlace_flag = INTERLACED;
                if (height%2)
                {
                    std::cerr << "Interlaced format with an odd number of lines are not supported"
                              << std::endl;
                    return false;
                }
            }
            _is_planar = (is_planar_set.find(_image_format) != is_planar_set.end());
            _is_macroblock = (is_macroblock_set.find(_image_format) != is_macroblock_set.end());
            //_rgb_info is the responsibility of the decoder but _ycbcr_info is not
            if (!_is_planar && !_is_macroblock)
            {
                initYCbCr(); //stride initialised in this function
                if (width%_ycbcr_info->pixels_per_motif){
                    std::cerr << "The given fourcc does not match well with the width" << std::endl;
                }
            }
            else
            {
                if (!_stride)//init stride for planar format if not given
                {
                    switch (_image_format)
                    {
                    case YCbYCr_10: case YCbYCr_16:
                    case YCbCr_planar_420_10: case YCbCr_planar_420_16: case YCrCb_planar_420_10: case YCrCb_planar_420_16:
                    case YCbCr_planar_422_10: case YCbCr_planar_422_16: case YCrCb_planar_422_10: case YCrCb_planar_422_16:
                    case Y_CbCrP_422_10: case Y_CbCrP_422_16: case Y_CrCbP_422_10: case Y_CrCbP_422_16:
                        _stride = _width * 2;
                        break;
                    case YCbCr_mb_420_8: case YCbCr_mb_420_10: case YCbCr_mb_422_8: case YCbCr_mb_422_10:
                        _stride = 16*16*2;  // Size of the luma MB in bytes
                        break;
                    default:
                        _stride = _width;
                    }
                }
            }
            if ((_subsampling == S_422) || (_subsampling == S_420))
            {
                if (_width%2)
                {
                    std::cerr << "Horizontal subsampling of 1/2 is not compatible with odd width"
                              << std::endl;
                    return false;
                }
                if ((_subsampling == S_420) && (_height%2))
                {
                    std::cerr << "Vertical subsampling of 1/2 is not compatible with odd height"
                              << std::endl;
                    return false;
                }
            }
            if ((_subsampling == S_411) && (_width%4))
            {
                std::cerr << "Horizontal subsampling of 1/4 is not compatible with the given width"
                          << std::endl;
                return false;
            }
            if (_is_macroblock && ((_width%16) || (_height%16)))
            {
                std::cerr << "Macroblock format is not compatible with the given width or height"
                          << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool Video_Type::initRGB(unsigned int number_bytes,
                                    const std::vector<unsigned char> &red_mask,
                                    const std::vector<unsigned char> &green_mask,
                                    const std::vector<unsigned char> &blue_mask,
                                    const std::vector<unsigned char> &alpha_mask)
{
    if (!_stride)//init stride if not given
    {
        _stride = number_bytes*_width;
    }

    if (_rgb_info != NULL) {
        throw std::runtime_error("RGB Info is not null.");
    }
    if (_colourspace != CS_RGB) {
        throw std::runtime_error("Colorspace is not RGB.");
    }
    if (red_mask.size() != number_bytes) {
        throw std::runtime_error("Red mask is not equal to the number of bytes.");
    }
    if (green_mask.size() != number_bytes) {
        throw std::runtime_error("Green mask is not equal to the number of bytes.");
    }
    if (blue_mask.size() != number_bytes) {
        throw std::runtime_error("Blue mask is not equal to the number of bytes.");
    }
    if (!alpha_mask.empty() && (alpha_mask.size() != number_bytes)) {
        throw std::runtime_error("Alpha mask is not valid.");
    }
    _rgb_info = std::make_shared<RGB_Info>();
    _rgb_info->bytes_per_motif = number_bytes;
    _rgb_info->R_mask = red_mask;
    _rgb_info->G_mask = green_mask;
    _rgb_info->B_mask = blue_mask;
    //use 0-length vector for alpha mask if absent
    _rgb_info->A_mask.clear();
    bool null_mask = true;
    std::vector<unsigned char>::const_iterator iter;
    for (iter = alpha_mask.begin(); iter != alpha_mask.end(); ++iter)
    {
        null_mask = null_mask && (*iter == 0);
    }
    if (!null_mask)
    {
        _rgb_info->A_mask = alpha_mask;
    }
    return true;
}

//! initialise the RGB info structure, an automatic shortcut of the previous version when type is common
void Video_Type::initRGB()
{
    if (_rgb_info != NULL) {
        throw std::runtime_error("RGB Info is not null.");
    }
    _rgb_info = std::make_shared<RGB_Info>();
    switch (_image_format)
    {
        case RGB_555: case RGB_565: case ARGB_1555: case ARGB_4444:
            _rgb_info->bytes_per_motif = 2;
        break;
        case RGB_24:
            _rgb_info->bytes_per_motif = 3;
        break;
        case RGB_32: case ARGB_32: case ARGB_2101010: case ABGR_2101010:
            _rgb_info->bytes_per_motif = 4;
        break;
        default:
            throw std::runtime_error("Video Type is incompatible with the ISP.");
    }
    if (!_stride) //init stride if not given (round to multiple of 4 as appropriate for RGB)
    {
        _stride = (_rgb_info->bytes_per_motif*_width + 3) & ~0x03;
    }
    std::vector<unsigned char> null_vector(_rgb_info->bytes_per_motif);
    std::fill(null_vector.begin(), null_vector.end(), static_cast<unsigned char>(0));
    _rgb_info->R_mask = null_vector;
    _rgb_info->G_mask = null_vector;
    _rgb_info->B_mask = null_vector;
    _rgb_info->A_mask.clear();
    //alpha mask?
    if ((_image_format==ARGB_32) ||
        (_image_format==ARGB_1555) || (_image_format==ARGB_4444) ||
        (_image_format==ARGB_2101010) || (_image_format==ABGR_2101010))
    {
        _rgb_info->A_mask = std::move(null_vector);
    }
    switch (_image_format)
    {
        //NB:RGB types are often associated with little_endianness. The last byte of the mask is used first because
        //low order bytes comes first on the stream.
        case RGB_555:
            _rgb_info->R_mask[0] = 0x7C;
            _rgb_info->G_mask[0] = 0x03;
            _rgb_info->G_mask[1] = 0xE0;
            _rgb_info->B_mask[1] = 0x1F;
        break;
        case RGB_565:
            _rgb_info->R_mask[0] = 0xF8;
            _rgb_info->G_mask[0] = 0x07;
            _rgb_info->G_mask[1] = 0xE0;
            _rgb_info->B_mask[1] = 0x1F;
        break;
        case ARGB_1555:
            _rgb_info->A_mask[0] = 0x80;
            _rgb_info->R_mask[0] = 0x7C;
            _rgb_info->G_mask[0] = 0x03;
            _rgb_info->G_mask[1] = 0xE0;
            _rgb_info->B_mask[1] = 0x1F;
        break;
        case ARGB_4444:
            _rgb_info->A_mask[0] = 0xF0;
            _rgb_info->R_mask[0] = 0x0F;
            _rgb_info->G_mask[1] = 0xF0;
            _rgb_info->B_mask[1] = 0x0F;
        break;
        case RGB_24:
            _rgb_info->R_mask[0] = 0xFF;
            _rgb_info->G_mask[1] = 0xFF;
            _rgb_info->B_mask[2] = 0xFF;
        break;
        case RGB_32:
            _rgb_info->R_mask[1] = 0xFF;
            _rgb_info->G_mask[2] = 0xFF;
            _rgb_info->B_mask[3] = 0xFF;
        break;
        case ARGB_32:
            _rgb_info->A_mask[0] = 0xFF;
            _rgb_info->R_mask[1] = 0xFF;
            _rgb_info->G_mask[2] = 0xFF;
            _rgb_info->B_mask[3] = 0xFF;
        break;
        case ARGB_2101010:
            _rgb_info->A_mask[0] = 0xC0;
            _rgb_info->R_mask[0] = 0x3F;
            _rgb_info->R_mask[1] = 0xF0;
            _rgb_info->G_mask[1] = 0x0F;
            _rgb_info->G_mask[2] = 0xFC;
            _rgb_info->B_mask[2] = 0x03;
            _rgb_info->B_mask[3] = 0xFF;
        break;
        case ABGR_2101010:
            _rgb_info->A_mask[0] = 0xC0;
            _rgb_info->B_mask[0] = 0x3F;
            _rgb_info->B_mask[1] = 0xF0;
            _rgb_info->G_mask[1] = 0x0F;
            _rgb_info->G_mask[2] = 0xFC;
            _rgb_info->R_mask[2] = 0x03;
            _rgb_info->R_mask[3] = 0xFF;
        break;
        default:
            throw std::runtime_error("RGB Info is incompatible with the ISP.");
    }
}

void Video_Type::initYCbCr()
{
    if (_ycbcr_info != NULL) {
        throw std::runtime_error("YCbCr Info is not null.");
    }
    _ycbcr_info = std::make_shared<YCbCr_Info>();
    std::vector<unsigned char> all0_mask;

    //find the number of bytes and pixels per motif
    //find the number of each components per motif
    switch (_image_format)
    {
        case CbYCrYCbYCrYYYYY: case CbYCrYCbYCrYYYYY_interlaced:
        {
            _ycbcr_info->bytes_per_motif = 12;
            _ycbcr_info->pixels_per_motif = 8;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(8);  //8 Y components
            _ycbcr_info->Cb_masks.resize(2); //2 Cb components
            _ycbcr_info->Cr_masks.resize(2); //2 Cr components
        }
        break;
        case CbYYCrYY:
        {
            _ycbcr_info->bytes_per_motif = 6;
            _ycbcr_info->pixels_per_motif = 4;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(4);  //4 Y components
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case AYCbCr: case ACbYCr_2101010:
        {
            _ycbcr_info->bytes_per_motif = 4;
            _ycbcr_info->pixels_per_motif = 1;
            _ycbcr_info->A_masks.resize(1);  //1 A component
            _ycbcr_info->Y_masks.resize(1);  //1 Y component
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case YCbYCr: case YCrYCb: case CrYCbY: case CbYCrY:
                     case CbYCrY_interlaced: case CbYCrY_inv:
        {
            _ycbcr_info->bytes_per_motif = 4;
            _ycbcr_info->pixels_per_motif = 2;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(2);  //2 Y components
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case YCbYCr_10: case YCbYCr_16:
            _ycbcr_info->bytes_per_motif = 8;
            _ycbcr_info->pixels_per_motif = 2;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(2);  //2 Y components
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        break;
        case Y5Y5Y5Y5Cb6Cr6:
        {
            _ycbcr_info->bytes_per_motif = 4;
            _ycbcr_info->pixels_per_motif = 4;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(4);  //4 Y components
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case CbYCr:
        {
            _ycbcr_info->bytes_per_motif = 3;
            _ycbcr_info->pixels_per_motif = 1;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(1);  //1 Y component
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case ACbYCr_16:
        {
            _ycbcr_info->bytes_per_motif = 8;
            _ycbcr_info->pixels_per_motif = 1;
            _ycbcr_info->A_masks.resize(1);  //1 A component
            _ycbcr_info->Y_masks.resize(1);  //1 Y component
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case Y6Cb5Cr5:
        {
            _ycbcr_info->bytes_per_motif = 2;
            _ycbcr_info->pixels_per_motif = 1;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(1);  //1 Y component
            _ycbcr_info->Cb_masks.resize(1); //1 Cb component
            _ycbcr_info->Cr_masks.resize(1); //1 Cr component
        }
        break;
        case Cb10Y10Cr10Y10_LE_6pack:
            _ycbcr_info->bytes_per_motif = 16;
            _ycbcr_info->pixels_per_motif = 6;
            _ycbcr_info->A_masks.clear();    //no A component
            _ycbcr_info->Y_masks.resize(6);  //6 Y component
            _ycbcr_info->Cb_masks.resize(3); //3 Cb component
            _ycbcr_info->Cr_masks.resize(3); //3 Cr component
        break;
        default:
        {
            throw std::runtime_error("YCbCr Info is incompatible with the ISP.");
        }
    }
    if (!_stride)//init stride if not given
    {
        _stride = (_ycbcr_info->bytes_per_motif*_width) /
                          _ycbcr_info->pixels_per_motif;
    }

    //initialise a all-zero mask...
    all0_mask.resize(_ycbcr_info->bytes_per_motif);
    std::fill(all0_mask.begin(), all0_mask.end(), static_cast<unsigned char>(0));
    //...and use it to zero all the mask created in the previous step
    std::fill(_ycbcr_info->A_masks.begin(), _ycbcr_info->A_masks.end(), all0_mask);
    std::fill(_ycbcr_info->Y_masks.begin(), _ycbcr_info->Y_masks.end(), all0_mask);
    std::fill(_ycbcr_info->Cb_masks.begin(), _ycbcr_info->Cb_masks.end(), all0_mask);
    std::fill(_ycbcr_info->Cr_masks.begin(), _ycbcr_info->Cr_masks.end(), all0_mask);

    //initialise the masks properly
    switch (_image_format)
    {
        case CbYCrYCbYCrYYYYY: case CbYCrYCbYCrYYYYY_interlaced:
        {
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[1][3] = 0xFF;
            _ycbcr_info->Y_masks[2][5] = 0xFF;
            _ycbcr_info->Y_masks[3][7] = 0xFF;
            _ycbcr_info->Y_masks[4][8] = 0xFF;
            _ycbcr_info->Y_masks[5][9] = 0xFF;
            _ycbcr_info->Y_masks[6][10] = 0xFF;
            _ycbcr_info->Y_masks[7][11] = 0xFF;
            _ycbcr_info->Cb_masks[0][0] = 0xFF;
            _ycbcr_info->Cb_masks[1][4] = 0xFF;
            _ycbcr_info->Cr_masks[0][2] = 0xFF;
            _ycbcr_info->Cr_masks[1][6] = 0xFF;
        }
        break;
        case CbYYCrYY:
        {
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[1][2] = 0xFF;
            _ycbcr_info->Y_masks[2][4] = 0xFF;
            _ycbcr_info->Y_masks[3][5] = 0xFF;
            _ycbcr_info->Cb_masks[0][0] = 0xFF;
            _ycbcr_info->Cr_masks[0][3] = 0xFF;
        }
        break;
        case AYCbCr:
        {
            _ycbcr_info->A_masks[0][0] = 0xFF;
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Cb_masks[0][2] = 0xFF;
            _ycbcr_info->Cr_masks[0][3] = 0xFF;
        }
        break;
        case ACbYCr_2101010:
        {
            _ycbcr_info->A_masks[0][0] = 0xC0;
            _ycbcr_info->Cb_masks[0][0] = 0x3F;
            _ycbcr_info->Cb_masks[0][1] = 0xF0;
            _ycbcr_info->Y_masks[0][1] = 0x0F;
            _ycbcr_info->Y_masks[0][2] = 0xFC;
            _ycbcr_info->Cr_masks[0][2] = 0x03;
            _ycbcr_info->Cr_masks[0][3] = 0xFF;
        }
        break;
        case YCbYCr:
        {
            _ycbcr_info->Y_masks[0][0] = 0xFF;
            _ycbcr_info->Cb_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[1][2] = 0xFF;
            _ycbcr_info->Cr_masks[0][3] = 0xFF;
        }
        break;
        case YCbYCr_10:
        {
            _ycbcr_info->Y_masks[0][0] = 0xFF;
            _ycbcr_info->Y_masks[0][1] = 0xC0;
            _ycbcr_info->Cb_masks[0][2] = 0xFF;
            _ycbcr_info->Cb_masks[0][3] = 0xC0;
            _ycbcr_info->Y_masks[1][4] = 0xFF;
            _ycbcr_info->Y_masks[1][5] = 0xC0;
            _ycbcr_info->Cr_masks[0][6] = 0xFF;
            _ycbcr_info->Cr_masks[0][7] = 0xC0;
        }
        break;
        case YCbYCr_16:
        {
            _ycbcr_info->Y_masks[0][0] = 0xFF;
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Cb_masks[0][2] = 0xFF;
            _ycbcr_info->Cb_masks[0][3] = 0xFF;
            _ycbcr_info->Y_masks[1][4] = 0xFF;
            _ycbcr_info->Y_masks[1][5] = 0xFF;
            _ycbcr_info->Cr_masks[0][6] = 0xFF;
            _ycbcr_info->Cr_masks[0][7] = 0xFF;
        }
        break;
        case YCrYCb:
        {
            _ycbcr_info->Y_masks[0][0] = 0xFF;
            _ycbcr_info->Cr_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[1][2] = 0xFF;
            _ycbcr_info->Cb_masks[0][3] = 0xFF;
        }
        break;
        case CrYCbY:
        {
            _ycbcr_info->Cr_masks[0][0] = 0xFF;
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Cb_masks[0][2] = 0xFF;
            _ycbcr_info->Y_masks[1][3] = 0xFF;
        }
        break;
        case CbYCrY: case CbYCrY_interlaced: case CbYCrY_inv:
        {
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[1][3] = 0xFF;
            _ycbcr_info->Cb_masks[0][0] = 0xFF;
            _ycbcr_info->Cr_masks[0][2] = 0xFF;
        }
        break;
        case Y5Y5Y5Y5Cb6Cr6:
        {
            //Warning, the Y are inverted with this packing format
            _ycbcr_info->Y_masks[3][0] = 0xF8;
            _ycbcr_info->Y_masks[2][0] = 0x07;
            _ycbcr_info->Y_masks[2][1] = 0xC0;
            _ycbcr_info->Y_masks[1][1] = 0x3E;
            _ycbcr_info->Y_masks[0][1] = 0x01;
            _ycbcr_info->Y_masks[0][2] = 0xF0;
            _ycbcr_info->Cb_masks[0][2] = 0x0F;
            _ycbcr_info->Cb_masks[0][3] = 0xC0;
            _ycbcr_info->Cr_masks[0][3] = 0x3F;
        }
        break;
        case CbYCr:
        {
            _ycbcr_info->Cb_masks[0][0] = 0xFF;
            _ycbcr_info->Y_masks[0][1] = 0xFF;
            _ycbcr_info->Cr_masks[0][2] = 0xFF;
        }
        break;
        case ACbYCr_16:
        {
            _ycbcr_info->A_masks[0][0] = 0xFF;
            _ycbcr_info->A_masks[0][1] = 0xFF;
            _ycbcr_info->Y_masks[0][2] = 0xFF;
            _ycbcr_info->Y_masks[0][3] = 0xFF;
            _ycbcr_info->Cb_masks[0][4] = 0xFF;
            _ycbcr_info->Cb_masks[0][5] = 0xFF;
            _ycbcr_info->Cr_masks[0][6] = 0xFF;
            _ycbcr_info->Cr_masks[0][7] = 0xFF;
        }
        break;
        case Y6Cb5Cr5:
        {
            _ycbcr_info->Y_masks[0][0] = 0xFC;
            _ycbcr_info->Cb_masks[0][0] = 0x03;
            _ycbcr_info->Cb_masks[0][1] = 0xE0;
            _ycbcr_info->Cr_masks[0][1] = 0x1F;
        }
        break;
        case Cb10Y10Cr10Y10_LE_6pack:
        {
            _ycbcr_info->Cb_masks[0][14] = 0x03;
            _ycbcr_info->Cb_masks[0][15] = 0xFF;
            _ycbcr_info->Cb_masks[1][9] = 0x0F;
            _ycbcr_info->Cb_masks[1][10] = 0xFC;
            _ycbcr_info->Cb_masks[2][4] = 0x3F;
            _ycbcr_info->Cb_masks[2][5] = 0xF0;

            _ycbcr_info->Cr_masks[0][12] = 0x3F;
            _ycbcr_info->Cr_masks[0][13] = 0xF0;
            _ycbcr_info->Cr_masks[1][6] = 0x03;
            _ycbcr_info->Cr_masks[1][7] = 0xFF;
            _ycbcr_info->Cr_masks[2][1] = 0x0F;
            _ycbcr_info->Cr_masks[2][2] = 0xFC;

            _ycbcr_info->Y_masks[0][13] = 0x0F;
            _ycbcr_info->Y_masks[0][14] = 0xFC;

            _ycbcr_info->Y_masks[1][10] = 0x03;
            _ycbcr_info->Y_masks[1][11] = 0xFF;
            _ycbcr_info->Y_masks[2][8] = 0x3F;
            _ycbcr_info->Y_masks[2][9] = 0xF0;

            _ycbcr_info->Y_masks[3][5] = 0x0F;
            _ycbcr_info->Y_masks[3][6] = 0xFC;

            _ycbcr_info->Y_masks[4][2] = 0x03;
            _ycbcr_info->Y_masks[4][3] = 0xFF;
            _ycbcr_info->Y_masks[5][0] = 0x3F;
            _ycbcr_info->Y_masks[5][1] = 0xF0;
        }
        break;
        default:
        {
            throw std::runtime_error("YCbCr Info is incompatible with the ISP.");
        }
    }
}

bool Video_Type::set_interlace(Interlace_Flag new_interlace_flag,
                               bool bottom_field_first)
{
    if (((_height%2) && (new_interlace_flag != PROGRESSIVE)) ||
        ((_subsampling == S_420) && (new_interlace_flag != PROGRESSIVE)))
    {
        std::cerr << "Interlacing is not supported with an odd number of lines "
                  << "or with vertical subsampling." << std::endl;
        return false;
    }
    _interlace_flag = new_interlace_flag;
    _bottom_field_first = bottom_field_first;
    return true;
}

void Video_Type::print(std::ostream& output_stream) const
{
    std::map<FourCC, std::string>::const_iterator iter = fourcc_to_string.find(_fourcc);
    if (iter == fourcc_to_string.end()) --iter; //not found?? go to the last element UNKNOWN
    output_stream << "fourcc = " << iter->second << std::endl;
    output_stream << "width = " << _width << std::endl;
    output_stream << "height = " << _height << std::endl;
    output_stream << "stride = " << _stride << std::endl;
    if (is_interlaced())
    {
        output_stream << "interlaced = yes" << std::endl;
        if (bottom_field_first())
        {
            output_stream << "bottom_field_first = yes" << std::endl;
        }
    }
    if (is_inversed()) output_stream << "inversed = yes" << std::endl;
    if (endianness() == L_ENDIAN)
    {
        output_stream << "endianness = little_endian" << std::endl;
    }
    output_stream << "frame_rate = " << _frame_rate << std::endl;
    if (_image_format == RGB_MASK)
    {
        output_stream << "number_bytes_per_sample = " << get_rgb_info()->R_mask.size() << std::endl;
        output_stream << std::hex;
        //an ostream_iterator to print directly the unsigned char in masks to the output
        output_stream << "red_mask = 0x";
        for (Mask::const_reverse_iterator iter = get_rgb_info()->R_mask.rbegin(); iter != get_rgb_info()->R_mask.rend(); ++iter)
        {
            output_stream << (unsigned int)(*iter);
        }
        output_stream << std::endl << "green_mask = 0x";
        for (Mask::const_reverse_iterator iter = get_rgb_info()->G_mask.rbegin(); iter != get_rgb_info()->G_mask.rend(); ++iter)
        {
            output_stream << std::hex << (unsigned int)(*iter);
        }
        output_stream << std::endl << "blue_mask = 0x";
        for (Mask::const_reverse_iterator iter = get_rgb_info()->B_mask.rbegin(); iter != get_rgb_info()->B_mask.rend(); ++iter)
        {
            output_stream << (unsigned int)(*iter);
        }
        if (!get_rgb_info()->A_mask.empty())
        {
            output_stream << std::endl << "alpha_mask = 0x";
            for (Mask::const_reverse_iterator iter = get_rgb_info()->A_mask.rbegin(); iter != get_rgb_info()->A_mask.rend(); ++iter)
            {
                output_stream << (unsigned int)(*iter);
            }
        }
        output_stream << std::endl << std::dec;
    }
}

std::vector<Video_Type::FourCC> Video_Type::get_compatible_fourccs(Colourspace cs,
                                                      Subsampling subsampling,
                                                      Interlace_Flag interlace_flag) const
{

    std::vector<Video_Type::FourCC> res;

    switch(cs)
    {
        case CS_MONO:
        {
            if ((interlace_flag == INTERLACED) || (subsampling != S_NONE))
            {
                //no solution, monochrome image cannot be subsampled, and
                //we do not allow for interlace
            }
            else
            {
                res.push_back(Y8);
                res.push_back(Y800);
                res.push_back(GREY);
                res.push_back(LGRY_16);
            }
        }
        break;
        case CS_RGB:
        {
            if ((interlace_flag == INTERLACED) || (subsampling != S_NONE))
            {
                // no solution, no RGB fourcc can be interlaced or subsampled
            }
            else
            {
                //push all RGB formats
                for (std::set<Image_Format>::const_iterator iter = RGB_format_set.begin();
                     iter != RGB_format_set.end(); ++iter)
                {
                    //insert all fourcc that match the RGB ImageFormat
                    std::pair< std::multimap<Image_Format, FourCC>::const_iterator,
                       std::multimap<Image_Format, FourCC>::const_iterator > iter_range =
                                                      fourcc_multimap.equal_range(*iter);
                    std::multimap<Image_Format, FourCC>::const_iterator ins_iter;
                    for (ins_iter = iter_range.first; ins_iter != iter_range.second;
                         ++ins_iter)
                    {
                        res.insert(res.end(), ins_iter->second);
                    }
                }
            }
        }
        break;
        case CS_YCbCr:
        {
            for (std::set<Image_Format>::const_iterator iter = YCbCr_format_set.begin();
                 iter != YCbCr_format_set.end(); ++iter)
            {
                std::set<Image_Format>::const_iterator iter_search =
                                                    is_interlaced_set.find(*iter);
                if (((interlace_flag == INTERLACED) &&
                     (iter_search != is_interlaced_set.end())) ||
                    ((interlace_flag != INTERLACED) &&
                     (iter_search == is_interlaced_set.end())))
                {
                    //keep ImageFormat if the subsampling is the same
                    if ((subsampling_map.find(*iter))->second == subsampling)
                    {
                        //insert all fourcc that match the ImageFormat
                        std::pair< std::multimap<Image_Format, FourCC>::const_iterator,
                           std::multimap<Image_Format, FourCC>::const_iterator > iter_range =
                                                      fourcc_multimap.equal_range(*iter);
                        std::multimap<Image_Format, FourCC>::const_iterator ins_iter;
                        for (ins_iter = iter_range.first; ins_iter != iter_range.second;
                             ++ins_iter)
                        {
                            res.insert(res.end(), ins_iter->second);
                        }
                    }
                }
            }
        }
        break;
        default:
        {
            throw std::runtime_error("Video Type is incompatible with the ISP.");
        }
    }
    return res;
}

Video_Type::FourCC Video_Type::get_fourcc(const std::string& str) const
{
    std::map<std::string, FourCC>::const_iterator iter = string_to_fourcc.find(str);
    if (iter == string_to_fourcc.end())
    {
        return FOURCC_UNKNOWN;
    }
    else
    {
        return iter->second;
    }
}

const std::string Video_Type::get_fourcc_string(Video_Type::FourCC fourcc) const
{
    auto iter = fourcc_to_string.find(fourcc);
    if (iter == fourcc_to_string.end())
    {
        return "Unknown";
    }
    return iter->second;
}

const std::map<Video_Type::FourCC, std::string> &Video_Type::get_fourcc_list() const
{
    return fourcc_to_string;
}

const std::map<Video_Type::FourCC, Video_Type::Image_Format> &
                                                       Video_Type::get_image_format_map() const
{
    return image_format_map;
}

const std::multimap<Video_Type::Image_Format, Video_Type::FourCC> &
                                                        Video_Type::get_fourcc_multimap() const
{
    return fourcc_multimap;
}


void Video_Type::init_structures()
{

    /******** monochrome formats *******************************************************/
    image_format_map[Y800] = MONO;
    image_format_map[Y8]   = MONO;
    image_format_map[GREY] = MONO;
    image_format_map[LGRY_16] = MONO_16;

    subsampling_map[MONO]  = S_NONE;
    subsampling_map[MONO_16]  = S_NONE;

    is_planar_set.insert(MONO);
    is_planar_set.insert(MONO_16);

    fourcc_to_string[Y800] = "Y800";
    fourcc_to_string[Y8] = "Y8";
    fourcc_to_string[GREY] = "GREY";
    fourcc_to_string[LGRY_16] = "LGRY_16";

    /******** RGB formats **************************************************************/
    image_format_map[RGB555] = RGB_555;
    image_format_map[RGB565] = RGB_565;
    image_format_map[RGB24] = RGB_24;
    image_format_map[RGB32] = RGB_32;
    image_format_map[ARGB32] = ARGB_32;
    image_format_map[ARGB1555] = ARGB_1555;
    image_format_map[ARGB4444] = ARGB_4444;
    image_format_map[A2R10G10B10] = ARGB_2101010;
    image_format_map[A2B10G10R10] = ABGR_2101010;
    image_format_map[RGB_BI_BITFIELDS] = RGB_MASK;

    subsampling_map[RGB_555]      = S_NONE;
    subsampling_map[RGB_565]      = S_NONE;
    subsampling_map[RGB_24]       = S_NONE;
    subsampling_map[RGB_32]       = S_NONE;
    subsampling_map[ARGB_32]      = S_NONE;
    subsampling_map[ARGB_1555]    = S_NONE;
    subsampling_map[ARGB_4444]    = S_NONE;
    subsampling_map[ARGB_2101010] = S_NONE;
    subsampling_map[ABGR_2101010] = S_NONE;
    subsampling_map[RGB_MASK]     = S_NONE;

    RGB_format_set.insert(RGB_555);
    RGB_format_set.insert(RGB_565);
    RGB_format_set.insert(RGB_24);
    RGB_format_set.insert(RGB_32);
    RGB_format_set.insert(ARGB_32);
    RGB_format_set.insert(ARGB_1555);
    RGB_format_set.insert(ARGB_4444);
    RGB_format_set.insert(ARGB_2101010);
    RGB_format_set.insert(ABGR_2101010);
    RGB_format_set.insert(RGB_MASK);

    fourcc_to_string[RGB_BI_BITFIELDS] = "BI_BITFIELDS";
    fourcc_to_string[RGB555] = "RGB555";
    fourcc_to_string[RGB565] = "RGB565";
    fourcc_to_string[RGB24] = "RGB24";
    fourcc_to_string[RGB32] = "RGB32";
    fourcc_to_string[ARGB1555] = "ARGB1555";
    fourcc_to_string[ARGB32] = "ARGB32";
    fourcc_to_string[ARGB4444] = "ARGB4444";
    fourcc_to_string[A2R10G10B10] = "A2R10G10B10";
    fourcc_to_string[A2B10G10R10] = "A2B10G10R10";

    /******** Macroblock formats ************************************************************/
    image_format_map[VSS1] = YCbCr_mb_420_8;
    image_format_map[VSS2] = YCbCr_mb_420_10;
    image_format_map[VST1] = YCbCr_mb_422_8;
    image_format_map[VST2] = YCbCr_mb_422_10;

    is_macroblock_set.insert(YCbCr_mb_420_8);
    is_macroblock_set.insert(YCbCr_mb_420_10);
    is_macroblock_set.insert(YCbCr_mb_422_8);
    is_macroblock_set.insert(YCbCr_mb_422_10);

    subsampling_map[YCbCr_mb_420_8] = S_420;
    subsampling_map[YCbCr_mb_420_10] = S_420;
    subsampling_map[YCbCr_mb_422_8] = S_422;
    subsampling_map[YCbCr_mb_422_10] = S_422;

    YCbCr_format_set.insert(YCbCr_mb_420_8);
    YCbCr_format_set.insert(YCbCr_mb_420_10);
    YCbCr_format_set.insert(YCbCr_mb_422_8);
    YCbCr_format_set.insert(YCbCr_mb_422_10);

    fourcc_to_string[VSS1] = "VSS1";
    fourcc_to_string[VSS2] = "VSS2";
    fourcc_to_string[VST1] = "VST1";
    fourcc_to_string[VST2] = "VST2";

    /******** YCbCr formats ************************************************************/
    //4:4:4 formats (all packed)
    image_format_map[AYUV] = AYCbCr;
    image_format_map[IYU2] = CbYCr;
    image_format_map[Y444] = CbYCr;
    image_format_map[V655] = Y6Cb5Cr5;
    image_format_map[Y416] = ACbYCr_16;
    image_format_map[Y410] = ACbYCr_2101010;

    subsampling_map[AYCbCr]   = S_444;
    // subsampling_map[YCbCr]    = S_444;
    subsampling_map[CbYCr]    = S_444;
    subsampling_map[Y6Cb5Cr5] = S_444;
    subsampling_map[ACbYCr_16] = S_444;
    subsampling_map[ACbYCr_2101010] = S_444;

    YCbCr_format_set.insert(AYCbCr);
    // YCbCr_format_set.insert(YCbCr);
    YCbCr_format_set.insert(CbYCr);
    YCbCr_format_set.insert(Y6Cb5Cr5);
    YCbCr_format_set.insert(ACbYCr_16);
    YCbCr_format_set.insert(ACbYCr_2101010);

    //4:2:2 formats
    image_format_map[YUYV] = YCbYCr;
    image_format_map[YUY2] = YCbYCr;
    image_format_map[YUNV] = YCbYCr;
    image_format_map[Y216] = YCbYCr_16; // 16bps
    image_format_map[Y210] = YCbYCr_10; // 10bps
    image_format_map[YVYU] = YCrYCb;
    image_format_map[VYUY] = CrYCbY;
    image_format_map[UYVY] = CbYCrY;
    image_format_map[Y422] = CbYCrY;
    image_format_map[UYNV] = CbYCrY;
    image_format_map[IUYV] = CbYCrY_interlaced; //explicitly interlaced
    image_format_map[V210] = Cb10Y10Cr10Y10_LE_6pack;    //10 bits per component, V210 packing
    image_format_map[YV16] = YCrCb_planar_422;
    image_format_map[Y42B] = YCbCr_planar_422;
    image_format_map[SB10] = YCbCr_planar_422_10;
    image_format_map[SB16] = YCbCr_planar_422_16;
    image_format_map[SV10] = YCrCb_planar_422_10;
    image_format_map[SV16] = YCrCb_planar_422_16;
    image_format_map[P216] = Y_CbCrP_422_16;
    image_format_map[P210] = Y_CbCrP_422_10;


    is_interlaced_set.insert(CbYCrY_interlaced);
    is_planar_set.insert(YCrCb_planar_422);
    is_planar_set.insert(YCbCr_planar_422);
    is_planar_set.insert(YCrCb_planar_422_10);
    is_planar_set.insert(YCbCr_planar_422_10);
    is_planar_set.insert(YCrCb_planar_422_16);
    is_planar_set.insert(YCbCr_planar_422_16);
    is_planar_set.insert(Y_CbCrP_422);    //planar/packed hybrid
    is_planar_set.insert(Y_CbCrP_422_16); //planar/packed hybrid
    is_planar_set.insert(Y_CbCrP_422_10); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_422);    //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_422_16); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_422_10); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_422_10); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_422_10); //planar/packed hybrid

    subsampling_map[YCbYCr]            = S_422;
    subsampling_map[YCbYCr_16]         = S_422;
    subsampling_map[YCbYCr_10]         = S_422;
    subsampling_map[YCrYCb]            = S_422;
    subsampling_map[CrYCbY]            = S_422;
    subsampling_map[CbYCrY]            = S_422;
    subsampling_map[CbYCrY_interlaced] = S_422;
    subsampling_map[YCrCb_planar_422]    = S_422;
    subsampling_map[YCbCr_planar_422]    = S_422;
    subsampling_map[YCrCb_planar_422_10] = S_422;
    subsampling_map[YCbCr_planar_422_10] = S_422;
    subsampling_map[YCrCb_planar_422_16] = S_422;
    subsampling_map[YCbCr_planar_422_16] = S_422;
    subsampling_map[Y_CbCrP_422]         = S_422;
    subsampling_map[Y_CbCrP_422_16]      = S_422;
    subsampling_map[Y_CbCrP_422_10]      = S_422;
    subsampling_map[Y_CrCbP_422]         = S_422;
    subsampling_map[Y_CrCbP_422_16]      = S_422;
    subsampling_map[Y_CrCbP_422_10]      = S_422;
    subsampling_map[Cb10Y10Cr10Y10_LE_6pack]  = S_422;

    YCbCr_format_set.insert(YCbYCr);
    YCbCr_format_set.insert(YCrYCb);
    YCbCr_format_set.insert(CrYCbY);
    YCbCr_format_set.insert(CbYCrY);
    YCbCr_format_set.insert(CbYCrY_interlaced);
    YCbCr_format_set.insert(YCbYCr_16);
    YCbCr_format_set.insert(YCbYCr_10);
    YCbCr_format_set.insert(Cb10Y10Cr10Y10_LE_6pack);
    YCbCr_format_set.insert(YCrCb_planar_422);
    YCbCr_format_set.insert(YCbCr_planar_422);
    YCbCr_format_set.insert(YCrCb_planar_422_10);
    YCbCr_format_set.insert(YCbCr_planar_422_10);
    YCbCr_format_set.insert(YCrCb_planar_422_16);
    YCbCr_format_set.insert(YCbCr_planar_422_16);
    YCbCr_format_set.insert(Y_CbCrP_422);
    YCbCr_format_set.insert(Y_CbCrP_422_16);
    YCbCr_format_set.insert(Y_CbCrP_422_10);
    YCbCr_format_set.insert(Y_CrCbP_422);
    YCbCr_format_set.insert(Y_CrCbP_422_16);
    YCbCr_format_set.insert(Y_CrCbP_422_10);

    //4:1:1 formats
    image_format_map[Y411] = CbYYCrYY;
    image_format_map[IYU1] = CbYYCrYY;
    image_format_map[Y41P] = CbYCrYCbYCrYYYYY;
    image_format_map[CLJR] = Y5Y5Y5Y5Cb6Cr6;
    image_format_map[IY41] = CbYCrYCbYCrYYYYY_interlaced;
    image_format_map[Y41B] = YCbCr_planar_411;

    is_interlaced_set.insert(CbYCrYCbYCrYYYYY_interlaced);
    is_planar_set.insert(YCbCr_planar_411);

    subsampling_map[CbYYCrYY]                    = S_411;
    subsampling_map[CbYCrYCbYCrYYYYY]            = S_411;
    subsampling_map[Y5Y5Y5Y5Cb6Cr6]              = S_411;
    subsampling_map[CbYCrYCbYCrYYYYY_interlaced] = S_411;
    subsampling_map[YCbCr_planar_411]            = S_411;

    YCbCr_format_set.insert(CbYYCrYY);
    YCbCr_format_set.insert(CbYCrYCbYCrYYYYY);
    YCbCr_format_set.insert(Y5Y5Y5Y5Cb6Cr6);
    YCbCr_format_set.insert(CbYCrYCbYCrYYYYY_interlaced);
    YCbCr_format_set.insert(YCbCr_planar_411);

    //4:2:0 format
    image_format_map[YV12] = YCrCb_planar_420;
    
    image_format_map[IYUV] = YCbCr_planar_420;
    image_format_map[I420] = YCbCr_planar_420;
    image_format_map[LV10] = YCrCb_planar_420_10;
    image_format_map[LB10] = YCbCr_planar_420_10;
    image_format_map[LV16] = YCrCb_planar_420_16;
    image_format_map[LB16] = YCbCr_planar_420_16;
    
    
    image_format_map[NV12] = Y_CbCrP_420;     //planar/packed hybrid
    image_format_map[P016] = Y_CbCrP_420_16;  //planar/packed hybrid
    image_format_map[P010] = Y_CbCrP_420_10;  //planar/packed hybrid
    image_format_map[NV21] = Y_CrCbP_420;
    
    image_format_map[IMC2] = Y_CrCbH_420;
    image_format_map[IMC4] = Y_CbCrH_420;

    is_planar_set.insert(YCrCb_planar_420);
    is_planar_set.insert(YCbCr_planar_420);
    is_planar_set.insert(YCrCb_planar_420_10);
    is_planar_set.insert(YCbCr_planar_420_10);
    is_planar_set.insert(YCrCb_planar_420_16);
    is_planar_set.insert(YCbCr_planar_420_16);
    is_planar_set.insert(Y_CbCrP_420); //planar/packed hybrid
    is_planar_set.insert(Y_CbCrP_420_16); //planar/packed hybrid
    is_planar_set.insert(Y_CbCrP_420_10); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_420); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_420_16); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbP_420_10); //planar/packed hybrid
    is_planar_set.insert(Y_CrCbH_420); //planar/packed hybrid
    is_planar_set.insert(Y_CbCrH_420); //planar/packed hybrid

    subsampling_map[YCrCb_planar_420]    = S_420;
    subsampling_map[YCrCb_planar_420_10] = S_420;
    subsampling_map[YCrCb_planar_420_16] = S_420;
    subsampling_map[YCbCr_planar_420]    = S_420;
    subsampling_map[YCbCr_planar_420_10] = S_420;
    subsampling_map[YCbCr_planar_420_16] = S_420;
    subsampling_map[Y_CbCrP_420]         = S_420;
    subsampling_map[Y_CbCrP_420_16]      = S_420;
    subsampling_map[Y_CbCrP_420_10]      = S_420;
    subsampling_map[Y_CrCbP_420]         = S_420;
    subsampling_map[Y_CrCbP_420_10]      = S_420;
    subsampling_map[Y_CrCbP_420_16]      = S_420;
    subsampling_map[Y_CrCbH_420]         = S_420;
    subsampling_map[Y_CbCrH_420]         = S_420;

    YCbCr_format_set.insert(YCrCb_planar_420);
    YCbCr_format_set.insert(YCrCb_planar_420_10);
    YCbCr_format_set.insert(YCrCb_planar_420_16);
    YCbCr_format_set.insert(YCbCr_planar_420);
    YCbCr_format_set.insert(YCbCr_planar_420_10);
    YCbCr_format_set.insert(YCbCr_planar_420_16);
    YCbCr_format_set.insert(Y_CbCrP_420);
    YCbCr_format_set.insert(Y_CbCrP_420_10);
    YCbCr_format_set.insert(Y_CbCrP_420_16);
    YCbCr_format_set.insert(Y_CrCbP_420);
    YCbCr_format_set.insert(Y_CrCbP_420_10);
    YCbCr_format_set.insert(Y_CrCbP_420_16);
    YCbCr_format_set.insert(Y_CrCbH_420);
    YCbCr_format_set.insert(Y_CbCrH_420);

    fourcc_to_string[AYUV] = "AYUV";
    fourcc_to_string[V655] = "V655";
    fourcc_to_string[IYU2] = "IYU2";
    fourcc_to_string[Y444] = "Y444";
    fourcc_to_string[Y410] = "Y410";
    fourcc_to_string[Y416] = "Y416";

    fourcc_to_string[UYVY] = "UYVY";
    fourcc_to_string[Y422] = "Y422";
    fourcc_to_string[UYNV] = "UYNV";
    fourcc_to_string[YUYV] = "YUYV";
    fourcc_to_string[Y216] = "Y216";
    fourcc_to_string[Y210] = "Y210";

    fourcc_to_string[YUY2] = "YUY2";
    fourcc_to_string[YUNV] = "YUNV";
    fourcc_to_string[YVYU] = "YVYU";
    fourcc_to_string[VYUY] = "VYUY";
    fourcc_to_string[Y411] = "Y411";
    fourcc_to_string[IYU1] = "IYU1";
    fourcc_to_string[Y41P] = "Y41P";
    fourcc_to_string[IUYV] = "IUYV";
    fourcc_to_string[IY41] = "IY41";
    fourcc_to_string[CLJR] = "CLJR";
    fourcc_to_string[V210] = "V210";
    fourcc_to_string[YV16] = "YV16";
    fourcc_to_string[Y42B] = "Y42B";
    fourcc_to_string[SV10] = "SV10";
    fourcc_to_string[SV16] = "SV16";
    fourcc_to_string[SB10] = "SB10";
    fourcc_to_string[SB16] = "SB16";
    fourcc_to_string[YV12] = "YV12";
    fourcc_to_string[LV10] = "LV10";
    fourcc_to_string[LV16] = "LV16";
    fourcc_to_string[IYUV] = "IYUV";
    fourcc_to_string[I420] = "I420";
    fourcc_to_string[LB10] = "LB10";
    fourcc_to_string[LB16] = "LB16";
    fourcc_to_string[Y41B] = "Y41B";
    fourcc_to_string[NV12] = "NV12";
    fourcc_to_string[NV21] = "NV21";
    fourcc_to_string[P216] = "P216";
    fourcc_to_string[P210] = "P210";
    fourcc_to_string[IMC2] = "IMC2";
    fourcc_to_string[IMC4] = "IMC4";
    fourcc_to_string[IMC2] = "IMC2";
    fourcc_to_string[IMC4] = "IMC4";
    fourcc_to_string[P016] = "P016";
    fourcc_to_string[P010] = "P010";

    // Reverse map
    for (std::map<FourCC, std::string>::const_iterator iter = fourcc_to_string.begin();
         iter != fourcc_to_string.end(); ++iter)
    {
        string_to_fourcc[iter->second] = iter->first;
    }
    //safety check
    if (fourcc_to_string.size() != string_to_fourcc.size()) {
        throw std::runtime_error("Video Type has not been matched.");
    }

    for (std::map<FourCC, std::string>::const_iterator iter = fourcc_to_string.begin();
         iter != fourcc_to_string.end(); ++iter)
    {
        if (image_format_map.find(iter->first) == image_format_map.end())
            std::cerr << "Missing FourCC in image_format_map: " << iter->second << std::endl;
    }

    for (std::map<FourCC, Image_Format>::const_iterator iter = image_format_map.begin();
         iter != image_format_map.end(); ++iter)
    {
        if (fourcc_to_string.find(iter->first) == fourcc_to_string.end())
            std::cerr << "Missing FourCC in fourcc_to_string: " << iter->first << std::endl;
    }

    //reverse image_format_map in fourcc_multimap
    for (std::map<FourCC, Image_Format>::const_iterator iter = image_format_map.begin();
         iter != image_format_map.end(); ++iter)
    {
        fourcc_multimap.insert(std::make_pair(iter->second, iter->first));
    }
    if (image_format_map.size() != fourcc_multimap.size()) {
        throw std::runtime_error("Image Format has not been matched.");
    }


    if (2 + RGB_format_set.size() + YCbCr_format_set.size() != subsampling_map.size())
    {
        std::cerr << "Missing format in subsampling_map" << std::endl;
    }
}
