/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "Lut1dUtils.h"

#include <cmath>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>
#include <fenv.h>
#include <vector>
#include <utility>
#include <sstream>


namespace SwApi {
namespace Lut1dUtils {
using std::stringstream;
using namespace std;


std::string PrintVecD(const std::vector<FloatT>& ds) 
{
    stringstream ss;
    ss << "[";
    bool first = true;

    for (size_t i = 0; i < ds.size(); i++) 
    {
        if (first) 
        {
            ss << ds[i];
            first = false;
        } 
        else 
        {
            ss << ", " << ds[i];
        }
    }
    ss << "]";

    return ss.str();
}


Lut1d Generate1dLut(ltf_t lut_tf, uint32_t bits_in, uint32_t bits_out,
    uint32_t bits_lut,
    uint32_t bits_seg,
    uint32_t bits_step,
    bool reverse_lut)
{
    bool lut_equidistant = false;

    if(bits_out == 0)
        bits_out = bits_in;

    // Get/Infer bits_lut and bits_seg from arguments.

    assert((bits_lut != 0) or (bits_seg != 0));

    if((bits_lut == 0) and (bits_seg != 0)) 
    {
        // Non-equidistant LUT, so size is calculated internally.
        lut_equidistant = false;
        bits_lut = bits_seg + ceilf(log2f((bits_in - bits_seg) / static_cast<float>(bits_step) + 1.0f));
    } 
    else if ((bits_lut != 0) and (bits_seg == 0)) 
    {
        // Non-equidistant LUT, so size of a LUT segemnt is calculated internally.
        lut_equidistant = false;
        bits_seg = bits_lut;

        while( ((0x1 << (bits_lut - bits_seg)) * bits_step) < (bits_in - bits_seg + bits_step))
        {
            bits_seg--;
        }
    } 
    else if ((bits_lut != 0) and (bits_seg != 0)) 
    {
        // Both were provided. Need to check they're sensible.
        assert(bits_lut == bits_seg);
        lut_equidistant = true;
    }

    // lut_full is basically just taking f at [0,1], with 2**bits_in + 1 even points.
    // Then multiply by 2**bits_out
    const std::size_t num_points = (1 << bits_in) + 1;
    const float step_width = (reverse_lut ? -1.0 : 1.0) / static_cast<float>(num_points - 1);

    float x = reverse_lut ? 1.0f : 0.0f;
    
    std::vector<uint32_t> lut_full = {};
    lut_full.reserve(num_points);

    const float LUT_VAL_SCALER = static_cast<float>((1 << bits_out) - 1);

    for (std::size_t i = 0; i < num_points; i++) 
    {
        const float lut_ent = lut_tf(x) * LUT_VAL_SCALER; // Remember to clamp to the max value we can possibly have
        auto tmp = roundevenf(lut_ent);
        lut_full.push_back(static_cast<uint32_t>(tmp));
        x = x + step_width;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Generate LUT.
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    Lut1d lut{bits_lut, bits_seg, bits_step, lut_equidistant};

    if (lut_equidistant) 
    {
        const std::size_t x_step = 1 << (bits_in - bits_lut);

        for(std::size_t x = 0; x < (std::size_t)(1 << bits_in); x += x_step)
        {
            lut._lut.push_back(lut_full[x]);
        }

        // lut_shifted
        for (std::size_t i = 0; i < lut._lut.size()-1; i++) 
        {
            lut._lut_shifted.push_back(lut._lut[i+1]);
        }
        
        for (std::size_t j = 0; j < lut_full.size(); j++) 
        {
            std::size_t reverse_index = lut_full.size() - 1 - j;
            lut._lut_shifted.push_back(lut_full[reverse_index]);
        }
    } 
    else 
    {
        // Section of LUT never hit for particular configuration
        // Fill with zeros
        const std::size_t lut_pad_sz = (1 << (bits_seg - bits_step));
        static constexpr uint32_t lut_padding = 0;

        // First 2^bits_seg elements of the LUT are at full density, 1 elem
        // each.
        const std::size_t full_density_sz = 1 << bits_seg;

        for(std::size_t i = 0; i < std::min(full_density_sz, lut_full.size()); i++)
            lut._lut.push_back(lut_full[i]);

        for(std::size_t i = 1; i < lut._lut.size(); i++)
            lut._lut_shifted.push_back(lut._lut[i]);

        // Rest of the LUT is populated sparsely, with density of LUT going down
        // by 2^bits_step for each segment.

        std::vector<uint32_t> lut_tmp = {};

        for(std::size_t i = bits_seg; i < bits_in; i += bits_step)  
        {
            for(std::size_t j = (1 << i); j < std::min((std::size_t(1) << (i + bits_step)), lut_full.size()); j += (1 << (bits_step + i - bits_seg)))
            {
                lut_tmp.push_back(lut_full[j]);
            }

            // LUT shifted
            lut._lut_shifted.push_back(lut_tmp[0]);

            for (std::size_t j = 0; j < lut_pad_sz; j++) 
            {
                lut._lut_shifted.push_back(lut_padding);
            }

            for (std::size_t j = 1; j < lut_tmp.size(); j++) 
            {
                lut._lut_shifted.push_back(lut_tmp[j]);
            }

            // LUT
            for (std::size_t j = 0; j < lut_pad_sz; j++)
                lut._lut.push_back(lut_padding);

            for (std::size_t j = 0; j < lut_tmp.size(); j++)
                lut._lut.push_back(lut_tmp[j]);

            lut_tmp.clear();
        }

        lut._lut_shifted.push_back(lut_full.back());

        assert(lut._lut_shifted.size() == lut._lut.size());

        return lut;
    }

    return Lut1d{};
}


FloatT Lookup1dLut(
    Lut1d lut,
    FloatT x,
    unsigned bits_in,
    unsigned bits_out,
    bool reverse_lut)
{
    int xx = (int)x;
    int segment = 0;
    int shift = 0;

    if (reverse_lut) 
    {
        xx = pow(2, bits_in - 1 - x);
    }

    if (lut.equidistant) 
    {
        segment = 0;
        shift = bits_in - lut.bits_lut;
    } 
    else 
    {
        // Priority encoder to calculate integer part of log base 2.
        auto x_clog2 = (int)ceil(log2(xx + 1.0));
        if (x_clog2 <= 0) 
        {
            x_clog2 = 1;
        }

        // Calculate address segment of the LUT we're working on.
        segment = roundevenf(((int)x_clog2 - (int)(lut.bits_seg - lut.bits_step + 1)) / (FloatT)lut.bits_step);

        if (segment <= 0) 
        {
            segment = 0;
        }

        // Calculate step value between consequent LUT entries from the segment
        shift = segment * lut.bits_step;
    }

    // Shift the input right to calculate an offset
    int x_shift = xx >> shift;

    // Quantize input to level of precision of the current segment of the LUT
    int x_quantized = x_shift << shift;

    // Calculate lookup address
    int idx = (segment << lut.bits_seg) + x_shift;

    // Linear interpolate between LUT points (alpha blend)
    int delta_x = xx - x_quantized;
    int distance = 1 << shift;

    FloatT y = (distance - delta_x) * (lut._lut[idx]) + (delta_x) * lut._lut_shifted[idx];
    // Round by adding 0.5 before cropping for better accuracy.
    if (shift > 0) 
    {
        y = y + pow(2.0, (shift-1));
    }
    y = ((int)y) >> shift;

    if (y >= pow(2, bits_out)) 
    {
        y = pow(2, bits_out) - 1;
    }

    return y;
}


static inline float bezier(const float t, const float x0, const float x1, const float x2, const float x3)
{
    const float u = 1.0f - t;
    const float t2 = t * t;
    const float u2 = u * u;
    return u2 * u * x0 + 3.0f * u2 * t * x1 + 3.0f * u * t2 * x2 + t2 * t * x3;
}


// To generate 1D LUT custom transfer function
/// software needs to workout y for each x on the
// custom LUT bezier curve
// To achive this we first need to find t on the curve
// for the given x. Softare uses approximation algorithms for that.
// Two options are provided:
// Bisection - robust and always finds value but requires more iterations
// Newton's method - supposedly faster but may not converge in some cases
// In this particular case (tolerance and max number of iterations) the
// bisection algorithm has been performing better.

#define USE_BEZIER_BISECTION

#ifdef USE_BEZIER_BISECTION

static inline float bezier_find_t_bisection(float v, float v0, float v1, float v2, float v3)
{
    static constexpr float TOLERANCE = 1e-6;
    static constexpr float MAX_ITER = 100;

    // Start with the entire curve length
    float t1 = 0.0f;
    float t2 = 1.0f;

    const float dv1 = bezier(t1, v0, v1, v2, v3) - v;

    if(fabs(dv1) < TOLERANCE)
        return t1;

    const float dv2 = bezier(t2, v0, v1, v2, v3) - v;

    // Differences must have different signs
    // if the value sits between t1 and t2
    if((dv1 * dv2) > 0.0f)
        return t1;

    for(std::size_t i = 0; i < MAX_ITER; ++i)
    {
        const float t_mid = (t1 + t2) / 2.0f;
        const float dv = bezier(t_mid, v0, v1, v2, v3) - v;

        if(fabs(dv) < TOLERANCE)
            return t_mid;

        // Update the segment start or end based on the value difference with the midpoint
        if(dv < 0.0f)
            t1 = t_mid;
        else
            t2 = t_mid;
    }

    return (t1 + t2) / 2.0f;        
};

#else

// Find t for x (or y) on Bezier curve using Newton's method
static inline float bezier_find_t_newton(float x, float x0, float x1, float x2, float x3)
{        
    static constexpr float INITIAL_GUESS = 0.5f;
    static constexpr float epsilon = 1e-6;
    static constexpr std::size_t MAX_ITER = 100;

    float t = INITIAL_GUESS;
    std::size_t iterations = 0;

    for(std::size_t i = 0; i <MAX_ITER; ++i)
    {
        // Bezier curve formula
        const float u = 1.0f - t;
        const float t2 = t * t;
        const float u2 = u * u;
        const float fx = (u2 * u * x0 + 3.0f * u2 * t * x1 + 3.0f * u * t2 * x2 + t2 * t * x3) - x;

        if (std::fabs(fx) < epsilon)
            break;

        // Bezier curve derivative
        const float dfx = (-3.0f * u2 * x0 + 3.0f * (3.0f * u2 - 2.0f * u * t) * x1 + 3.0f * (2.0f * u * t - t2) * x2 + 3.0f * t2 * x3);

        t -= fx / dfx;
        ++iterations;
    }

    return t;
}

#endif /* USE_BEZIER_BISECTION */


float EvaluateSpline(const float x, const std::vector<std::pair<float, float>>& knots, 
                        const std::vector<std::pair<float, float>>& controlPoints)
{
    float y = x;

    // Find which segment the point is in
    std::size_t segment = 0;

    for(std::size_t i = 1; i < knots.size(); ++i)
    {
        if (x < knots[i].first)
            break;

        ++segment;
    }

    if(segment < (knots.size() - 1))
    {
        const float x0 = knots[segment].first;
        const float y0 = knots[segment].second;
        const float x1 = controlPoints[segment * 2].first;
        const float y1 = controlPoints[segment * 2].second;
        const float x2 = controlPoints[segment * 2 + 1].first;
        const float y2 = controlPoints[segment * 2 + 1].second;
        const float x3 = knots[segment + 1].first;
        const float y3 = knots[segment + 1].second;

#ifdef USE_BEZIER_BISECTION
        const float t = bezier_find_t_bisection(x, x0, x1, x2, x3);
#else
        const float t = bezier_find_t_newton(x, x0, x1, x2, x3);
#endif /* USE_BEZIER_BISECTION */

        y = bezier(t, y0, y1, y2, y3);
    }

    return y;
}


static ltf_t gamma_tf_factory(const TransferFunctionType type, const float gamma)
{
    switch (type) 
    {
        case TransferFunctionType::OETF: 
        {
            return [](const float x)->float {
                static constexpr float beta = 0.018053968510807f;
                static constexpr float alpha = 1.0f + 5.5f * beta;
                return (x < beta) ? ( 4.5f * x ) : (alpha * powf(x, 0.45f)) - (alpha - 1.0f);
            };
        }

        case TransferFunctionType::OETF_LEGACY: 
        {
            return [gamma](const float x)->float {
                return powf(x, 1.0f / gamma);
            };
        }        
        case TransferFunctionType::EOTF: 
        {
            return [gamma](const float x)->float {
                return powf(x, gamma);
            };
        }
        case TransferFunctionType::OOTF: 
        {
            auto f1 = gamma_tf_factory(TransferFunctionType::OETF, gamma);
            auto f2 = gamma_tf_factory(TransferFunctionType::EOTF, gamma);

            return [gamma, f1 = std::move(f1), f2 = std::move(f2)](const float x)->float {return f2(f1(x));};
        }
        default: 
            assert(false);
    }
}


static ltf_t hlg_tf_factory(TransferFunctionType tf, float Luminance_Black, float Luminance_White, optional<float> gamma)
{
    if (not gamma) 
    {
        gamma = static_cast<float>(1.2f + 0.42f * log10f(static_cast<float>(Luminance_White) / 1000.0f));
    }

    static constexpr float a = 0.17883277f;
    static constexpr float b = 1.0f - 4.0f * a;
    static const float c = 0.5f - a * logf(4.0f * a);

    switch (tf) 
    {
        default: 
        case TransferFunctionType::OETF: 
        {
            return [](const float x)->float {
                static constexpr float THRESHOLD = (1.0f / 12.0f);
                return (x > THRESHOLD) ? (a * logf(12.0f * x - b) + c) : sqrtf(3.0f * x);
            };
        }
        case TransferFunctionType::EOTF: 
        {
            return [Luminance_Black, Luminance_White, gamma](const float x)->float {
                static constexpr float THRESHOLD = (1.0f / 2.0f);
                float y = (x > THRESHOLD) ? ((expf((x - c) / a) + b) / 12.0f) : ((x * x) / 3.0f);

                if(fabs(y - 0.0f) > std::numeric_limits<float>::epsilon())
                    y = ((Luminance_White - Luminance_Black) * powf(y, *gamma - 1.0f) * y) / 1000.0f;
                return y;
            };
        }
        case TransferFunctionType::IEOTF:
        {
            return [](const float x){
                static constexpr float THRESHOLD = (1.0f / 2.0f);
                return (x > THRESHOLD) ? ((expf((x - c) / a) + b) / 12.0f) : ((x * x) / 3.0f);
            };
        }
        case TransferFunctionType::OOTF: 
        {
            auto f1 = hlg_tf_factory(TransferFunctionType::OETF, Luminance_Black, Luminance_White, gamma);
            auto f2 = hlg_tf_factory(TransferFunctionType::EOTF, Luminance_Black, Luminance_White, gamma);

            return [f1 = std::move(f1), f2 = std::move(f2)](const FloatT x){return f2(f1(x));};
        }
    }
}


static ltf_t pq_tf_factory(TransferFunctionType tf) 
{
    static constexpr float c3 = 2392.0f / 4096.0f * 32.0f;
    static constexpr float c2 = 2413.0f / 4096.0f * 32.0f;
    static constexpr float c1 = c3 - c2 + 1.0f; // 3424 / 4096
    static constexpr float m1 = 2610.0f / 4096.0f * (1.0f / 4.0f);
    static constexpr float m2 = 2523.0f / 4096.0f * 128.0f;

    switch (tf) 
    {
        case TransferFunctionType::IEOTF: 
        {
            return [](const FloatT _x){
                const float x = static_cast<float>(_x);
                const auto tmp = powf(x, m1);
                return powf((c1 + c2 * tmp) / (1.0f + c3 * tmp), m2);
            };
        }
        case TransferFunctionType::EOTF: 
        {
            return [](const FloatT _x){
                const float x = static_cast<float>(_x);
                const auto tmp = powf(x, (1.0f / m2));
                return powf(std::max(tmp - c1, 0.0f) / (c2 - c3 * tmp), (1.0f / m1));
            };
        }
        case TransferFunctionType::OOTF: 
        {
            return [](const FloatT _x){
                const float x = static_cast<float>(_x);
                static constexpr float beta = 0.018053968510807f;
                static constexpr float alpha = 1.0f + 5.5f * beta;
                static constexpr float scaler = 59.49080238715396f;
                static constexpr float THRESHOLD = beta / scaler;

                const float x_scaled = scaler * x;
                float y = (x > THRESHOLD) ? (alpha * powf(x_scaled, 0.45f) - (alpha - 1.0f)) : (4.5f * x_scaled);
                return powf(y, 2.4f) / 100.0f;
            };
        }
        case TransferFunctionType::OETF:
        default:
        {
            auto f1 = pq_tf_factory(TransferFunctionType::OOTF);
            auto f2 = pq_tf_factory(TransferFunctionType::IEOTF);

            return [f1 = std::move(f1), f2 = std::move(f2)](const FloatT x){
                return f2(f1(x));
            };
        }        
    }
}


ltf_t LtfFactory(const Lut1dParams& params)
{
    switch(params._curve)
    {
        case TransferFunctionCurve::HLG:
        {
            return hlg_tf_factory(params._type, params._blackLum, params._whiteLum, params._gamma);
        }
        case TransferFunctionCurve::PQ:
        {
            return pq_tf_factory(params._type);
        }
        case TransferFunctionCurve::Custom:
        {
            if((params._knots.size() > 1) && !params._controlPoints.empty())
                return [knots =  params._knots, controlPoints = params._controlPoints](const float x)->float {return EvaluateSpline(x, knots, controlPoints);};
            else
                return [](const float x)->float {return x;};
        }
        case TransferFunctionCurve::Gamma:
        default:
        {
            return gamma_tf_factory(params._type, params._gamma);
        }
    }    
}


const char* ToString(TransferFunctionType tf) 
{
    switch (tf) 
    {
        case TransferFunctionType::OETF: 
        {
            return "TransferFunctionType::OETF";
        }
        case TransferFunctionType::IEOTF: 
        {
            return "TransferFunctionType::IOETF";
        }
        case TransferFunctionType::EOTF: 
        {
            return "TransferFunctionType::EOTF";
        }
        case TransferFunctionType::OOTF: 
        {
            return "TransferFunctionType::OOTF";
        }
        default:
        {
            assert(false);
        }
    }
}
const char* ToString(TransferFunctionCurve curve) 
{
    switch (curve) 
    {
        case TransferFunctionCurve::Gamma: 
        {
            return "TransferFunctionCurve::Gamma";
        }
        case TransferFunctionCurve::HLG: 
        {
            return "TransferFunctionCurve::HLG";
        }
        case TransferFunctionCurve::PQ: 
        {
            return "TransferFunctionCurve::PQ";
        }
        default: 
        {
            assert(false);
        }
    }
}
std::ostream& operator<<(std::ostream& os, TransferFunctionType tf) 
{
    return os << ToString(tf);
}
std::ostream& operator<<(std::ostream& os, TransferFunctionCurve curve) 
{
    return os << ToString(curve);
}

std::ostream& operator<<(std::ostream& os, std::pair<FloatT, FloatT> point)
{
    return os << "(" << point.first << ", " << point.second << ")";
}

std::ostream& operator<<(std::ostream& os, std::vector<std::pair<FloatT, FloatT>> points)
{
    os << "{ ";
    for (int i = 0; i < (int)points.size(); i++)
    {
        os << points[i] << ", ";
    }
    return os << "}";
}

} // namespace Lut1dUtils
} // namespace SwApi
