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
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>


namespace SwApi {
namespace Lut1dUtils {

using namespace std;


using FloatT = float;


enum class TransferFunctionType {
    OETF,
    EOTF,
    IEOTF,
    OOTF,
    OETF_LEGACY
};

enum class TransferFunctionCurve { Gamma, HLG, PQ, Custom };

struct Lut1dParams
{
    TransferFunctionType _type;
    TransferFunctionCurve _curve;
    float _gamma;
    float _blackLum;
    float _whiteLum;
    std::vector<std::pair<float, float>> _knots;
    std::vector<std::pair<float, float>> _controlPoints;
};

class Lut1d
{
public:
    Lut1d(uint32_t bits_lut, uint32_t bits_seg, uint32_t bits_step, bool lut_equidistant = false):
        bits_lut{bits_lut},
        bits_seg{bits_seg},
        bits_step{bits_step},
        equidistant{lut_equidistant}
    {};

    Lut1d()
    {}

    unsigned bits_lut = 0;
    unsigned bits_seg = 0;
    unsigned bits_step = 0;
    bool equidistant = false;

    std::vector<uint32_t> _lut;
    std::vector<uint32_t> _lut_shifted;
};

using ltf_t = std::function<float(float)>;

Lut1d Generate1dLut(ltf_t lut_tf, uint32_t bits_in, uint32_t bits_out = 0,
                    uint32_t _bits_lut = 0,
                    uint32_t _bits_seg = 0,
                    uint32_t bits_step = 2,
                    bool reverse_lut = false);

ltf_t LtfFactory(const Lut1dParams& params);

FloatT Lookup1dLut(Lut1d lut, FloatT x, unsigned bits_in, unsigned bits_out, bool reverse_lut = false);

float EvaluateSpline(const float x, const std::vector<std::pair<float, float>>& knots, 
    const std::vector<std::pair<float, float>>& controlPoints);

const char* ToString(TransferFunctionType tf);
const char* ToString(TransferFunctionCurve curve);
std::ostream& operator<<(std::ostream& os, TransferFunctionType tf);
std::ostream& operator<<(std::ostream& os, TransferFunctionCurve curve);
std::ostream& operator<<(std::ostream& os, std::pair<FloatT, FloatT> point);
std::ostream& operator<<(std::ostream& os, std::vector<std::pair<FloatT, FloatT>> points);


} // namespace Lut1dUtils
} // namespace SwApi
