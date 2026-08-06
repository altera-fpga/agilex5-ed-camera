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
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "SwUtilsMemBuffer.h"

namespace SwApi
{
namespace AnrUtils
{
    struct Matrix
    {
        int m;
        int n;
        std::vector<std::vector<float>> data;

        Matrix Transpose();
        float Determinant();

        Matrix operator*(const Matrix& M);
        Matrix operator*(const float& x);
        friend std::ostream& operator<<(std::ostream& os, const Matrix& m);
    };
    // Instances of Welfords Algorithm https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    struct WelfordInstance
    {
        uint32_t count = 0;
        float mean = 0;
        float M2 = 0;

        void Update(int newVal);
        // Returns variance of the set
        float Finalize();
    };

    using AnrFrame = SwUtils::mem_buffer_t;
    

    // \return {combinedGain, darkNoise}
    std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits,  
            std::pair<float, float> _intensity_cutoff_pair, uint32_t bin_neighbourhood = 3);
    // \return {combinedGain, darkNoise}
    std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits,  
            float intensity_cutoff_scal, uint32_t bin_neighbourhood = 3);
    // \return {combinedGain, darkNoise}
    std::pair<float, float> NoiseEstimator(const AnrFrame& dark_frame, const std::vector<AnrFrame>& scene_frames, uint32_t bits, uint32_t bin_neighbourhood = 3);

    std::pair<float, float> FitSquareRootCurve(const std::vector<float>& x_data, const std::vector<float>& y_data, const std::vector<float>& y_sigma, float x0, float y0);
}
}