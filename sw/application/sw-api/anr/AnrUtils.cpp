/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "AnrUtils.h"
#include <cmath>


uint32_t mean(const SwApi::AnrUtils::AnrFrame& vals)
{
    uint32_t rv = 0;

    if(vals.data())
    {
        const uint16_t* p = reinterpret_cast<const uint16_t*>(vals.data());
        const uint16_t* const p_end = reinterpret_cast<const uint16_t*>(vals.data() + vals.size());
        const std::size_t sz = p_end - p;
        uint32_t total = 0;

        while(p != p_end)
        {
            total += (*p++);
        }

        rv = total / sz;
    }

    return rv;
}


float st_dev(const SwApi::AnrUtils::AnrFrame& vals)
{
    const float avg = mean(vals);
    float total_var = 0;

    const uint16_t* p = reinterpret_cast<const uint16_t*>(vals.data());
    const uint16_t* const p_end = reinterpret_cast<const uint16_t*>(vals.data() + vals.size());
    const std::size_t sz = p_end - p;

    while(p != p_end)
    {
        const int32_t tmp = static_cast<int32_t>(*p) - static_cast<int32_t>(avg);
        total_var += static_cast<float>(tmp * tmp);
        ++p;
    }

    return sqrtf(total_var / static_cast<float>(sz));
}


namespace SwApi
{
namespace AnrUtils
{

std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits, 
        std::pair<float, float> intensity_cutoff_pair, uint32_t bin_neighbourhood)
{
    std::pair<int, int> intensity_cutoffs;
    intensity_cutoffs.first = intensity_cutoff_pair.first * powf(2, bits-1);
    intensity_cutoffs.second = intensity_cutoff_pair.second * powf(2, bits-1);
    
    // dark noise and black level are estimated from the dark frame
    std::cout << "Estimating dark noise (1/7)" << std::endl;
    float dark_noise_est = st_dev(dark_frame);
    std::cout << "Estimating black level (2/7)" << std::endl;
    uint32_t black_level_est = round(mean(dark_frame));

    std::cout << "Initialising noise curve (3/7)" << std::endl;
    int data_length = intensity_cutoffs.second - intensity_cutoffs.first;
    std::vector<float> noise_curve;
    // int curve_size = pow(2, bits) - black_level_est - 1;
    noise_curve.reserve(data_length);
    noise_curve.push_back(dark_noise_est);
    for (int i = 0; i < data_length - 1; i++)
    {
        noise_curve.push_back(0);
    }

    std::cout << "Calculating noise curve (4/7)" << std::endl;
    // for (uint32_t i = black_level_est + 1; i < pow(2, bits); i++)
    // {
    //     std::cout << (i + 1 - black_level_est) << "/" << (pow(2, bits) - black_level_est) << std::endl;
    //     std::vector<float> subset;
    //     for (int j = 0; j < scene_frames[0].size(); j++)
    //     {
    //         uint16_t avg = (scene_frames[0][j] + scene_frames[1][j]) / 2;
    //         if ((avg <= i + bin_neighbourhood) && (avg >= i - bin_neighbourhood))
    //             subset.push_back(scene_frames[0][j] - scene_frames[1][j]);
    //     }

    //     if (subset.size() > 50)
    //     {
    //         // calculate standard deviations per bin
    //         noise_curve[i] = (st_dev(subset) / sqrt(2));
    //     }
    //     else
    //     {
    //         // too few samples, set bin to 0
    //         noise_curve[i] = 0;
    //     }
    // }
    {
        std::vector<WelfordInstance> st_dev_calcs;
        
        st_dev_calcs.resize(data_length);

        const uint16_t* p0 = reinterpret_cast<const uint16_t*>(scene_frames[0].data());
        const uint16_t* const p0_end = reinterpret_cast<const uint16_t*>(scene_frames[0].data() + scene_frames[0].size());
        const uint16_t* p1 = reinterpret_cast<const uint16_t*>(scene_frames[1].data());
        const std::size_t sz = p0_end - p0;
        
        for (int i = 0; i < (int)sz; i++)
        {
            uint16_t avg = (p0[i] + p1[i]) / 2;
 
            // work out which bins the pixel goes in
            int min_bin = avg - bin_neighbourhood - (black_level_est + 1);
            int max_bin = avg + bin_neighbourhood - (black_level_est + 1);
 
            if (min_bin < intensity_cutoffs.first) min_bin = intensity_cutoffs.first;
            if (max_bin > intensity_cutoffs.second) max_bin = intensity_cutoffs.second;
 
            if (max_bin > intensity_cutoffs.first)
            {
                for (int bin = min_bin; bin < max_bin; bin++)
                {
                    st_dev_calcs.at(bin - intensity_cutoffs.first).Update(static_cast<int>(p0[i]) - static_cast<int>(p1[i]));
                }
            }
        }
       
        for (int i = 0; i < data_length; i++)
        {
            if (st_dev_calcs.at(i).count > 50)
                noise_curve[i] = sqrt(st_dev_calcs.at(i).Finalize() / 2);
            else // Too few samples, set bin to 0
                noise_curve[i] = 0;
        }
    }

    // fit a square root curve to the noise estimations
    std::cout << "Creating curve data (5/7)" << std::endl;
    std::vector<float> x_data;
    std::vector<float> y_data;
    x_data.resize(data_length);
    y_data.resize(data_length);
    for (int i = 0; i < data_length; i++)
    {
        x_data[i] = i;
        y_data[i] = noise_curve[i];
    }
    
    // give equal weight to all noise sample points, except for the first point (dark noise)
    // which is known to be more accurate
    std::cout << "Defining curve weights (6/7)" << std::endl;
    std::vector<float> y_sigma;
    y_sigma.reserve(y_data.size());
    y_sigma.push_back(0.25);
    for (int i = 1; i < (int)y_data.size(); i++)
    {
        y_sigma.push_back(1.0f);
    }

    std::cout << "Fitting square root curve (7/7)" << std::endl;
    std::pair<float, float> results = FitSquareRootCurve(x_data, y_data, y_sigma, dark_noise_est, 1.0f);
    return results;
}

std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits, 
        float intensity_cutoff_scal, uint32_t bin_neighbourhood)
{
    return NoiseEstimator(dark_frame, scene_frames, bits, {0.0f, intensity_cutoff_scal}, bin_neighbourhood);
}

std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits, uint32_t bin_neighbourhood)
{
    return NoiseEstimator(dark_frame, scene_frames, bits, {0.0f, 0.7f}, bin_neighbourhood);
}

std::pair<float, float> FitSquareRootCurve(const std::vector<float>& x_data, const std::vector<float>& y_data, const std::vector<float>& y_sigma, float x0, float y0)
{
    if (x_data.size() != y_data.size() || y_data.size() != y_sigma.size())
    {
        return {-1, -1};
    }
    
    // Pretend the square root curve is a linear function
    // Based on https://en.wikipedia.org/wiki/Weighted_least_squares
    Matrix x_dash = {static_cast<int>(x_data.size()), 2, {}};
    Matrix y_dash = {static_cast<int>(y_data.size()), 1, {}};
    x_dash.data.reserve(x_data.size());
    y_dash.data.reserve(y_data.size());
    for (int i = 0; i < (int)x_data.size(); i++)
    {
        std::vector<float> x_row = {};
        x_row.push_back((pow(y0, 2) * x_data[i] - x0) * y_sigma[i]);
        x_row.push_back(1 * y_sigma[i]);
        x_dash.data.push_back(std::move(x_row));
        std::vector<float> y_row = {};
        y_row.push_back(pow(y_data[i], 2) * y_sigma[i]);
        y_dash.data.push_back(std::move(y_row));
    }

    Matrix x_dash_T = x_dash.Transpose();

    Matrix sqr = x_dash_T * x_dash;

    const float determinant = sqr.Determinant();

    if(std::fabs(determinant) > std::numeric_limits<float>::epsilon())
    {
        float inv_mult = 1.0 / determinant;

        Matrix inv = {2, 2, {}};
        inv.data = {{sqr.data[1][1], -sqr.data[0][1]}, {-sqr.data[1][0], sqr.data[0][0]}};
        inv = inv * inv_mult;

        Matrix results = (inv * x_dash_T) * y_dash;

        //FIXME check for negative - shouldn't happen but can't be sure
        return {sqrt(results.data[0][0]), sqrt(results.data[1][0])};
    }
    else
    {
        // Singular matrix, can't invert
        return {-1, -1};
    }


}

Matrix Matrix::Transpose()
{
    Matrix new_m = {n, m, {}};
    new_m.data.reserve(n);
    for (int i = 0; i < n; i++)
    {
        std::vector<float> row;
        row.reserve(m);
        for (int j = 0; j < m; j++)
        {
            row.push_back(data[j][i]);
        }
        new_m.data.push_back(std::move(row));
    }
    return new_m;
}

float Matrix::Determinant()
{
    // Non-square matrix
    if (m != n)
        return -1;

    if (m == 2)
    {
        return data[0][0]*data[1][1] - data[0][1]*data[1][0];
    }
    else
    {
        std::cout << "If this is being printed I'm sad because now I have to code it" << std::endl;
        return 0;
    }
}

Matrix Matrix::operator*(const Matrix& M)
{
    if (this->n != M.m)
    {
        // Matrices not multiplicable
        return Matrix {-1, -1, {{}}};
    }
    
    Matrix new_m = {this->m, M.n, {}};
    new_m.data.reserve(this->m);
    for (int i = 0; i < this->m; i++)
    {
        std::vector<float> row;
        row.reserve(M.n);
        for (int j = 0; j < M.n; j++)
        {
            float val = 0;
            for (int k = 0; k < this->n; k++)
            {
                val += this->data[i][k] * M.data[k][j];
            }
            row.push_back(val);
        }
        new_m.data.push_back(std::move(row));
    }
    return new_m;
}

Matrix Matrix::operator*(const float& x)
{
    Matrix new_m = {m, n, {}};
    new_m.data.reserve(m);
    for (int i = 0; i < m; i++)
    {
        std::vector<float> row;
        row.reserve(n);
        for (int j = 0; j < n; j++)
        {
            row.push_back(data[i][j] * x);
        }
        new_m.data.push_back(std::move(row));
    }
    return new_m;
}

std::ostream& operator<<(std::ostream& os, const Matrix& m)
{
    os << "m = " << m.m << ", n = " << m.n << "\n{";
    for (std::vector<float> row : m.data)
    {
        os << "\n{";
        for (float val : row)
        {
            os << val << ", ";
        }
        os << "}";
    }
    os << "\n}";
    return os;
}

void WelfordInstance::Update(int newVal)
{
    count++;
    float delta = newVal - mean;
    mean += delta / count;
    float delta2 = newVal - mean;
    M2 += delta * delta2;
}

float WelfordInstance::Finalize()
{
    if (count < 2)
    {
        return -1;
    }
    else
    {
        return M2 / count;
    }
}

}
}