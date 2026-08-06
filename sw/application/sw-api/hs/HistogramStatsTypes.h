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

namespace SwApi
{
    namespace Hs
    {
        struct RegionOfInterest
        {
            uint16_t h_start;
            uint16_t v_start;
            uint16_t h_end;
            uint16_t v_end;
        };

        static constexpr std::size_t MAX_LUMA_BINS = 256;

        struct LumaBinContainer
        {
            uint16_t num_luma_bins;
            uint32_t luma_bins[MAX_LUMA_BINS];

            uint32_t GetTotal()
            {
                uint32_t accumulator = 0;
                for (int i = 0; i < num_luma_bins; i++)
                {
                    accumulator += luma_bins[i];
                }
                return accumulator;
            };

            uint32_t GetLargestBin()
            {
                uint32_t idx = 0;
                uint32_t largest = 0;
                for (int i = 0; i < num_luma_bins; i++)
                {
                    if (luma_bins[i] > largest)
                    {
                        largest = luma_bins[i];
                        idx = i;
                    }
                }
                return idx;
            };

            // This is a LOT more computationally expensive than the mode
            uint32_t GetMeanBin()
            {
                float accumulator = 0;
                uint32_t total = GetTotal();

                for (int i = 0; i < num_luma_bins; i++)
                {
                    accumulator += (luma_bins[i] * i) / total;
                }

                return (uint32_t)accumulator;
            }
        };

        struct TableResults
        {
            bool valid;
            LumaBinContainer frame_luma_bins;
            LumaBinContainer roi_luma_bins;
        };

    } // namespace Hs
} // namespace SwApi