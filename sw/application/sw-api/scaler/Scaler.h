/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpScaler.h"


namespace SwApi
{

class Scaler
{
public:
    // Factory method for convenience
    static std::shared_ptr<Scaler> Create(Hapi::VvpScalerPtr spScaler);
    Scaler(Hapi::VvpScalerPtr spVvpScaler);
    ~Scaler();

    bool GetLiteMode();
    std::pair<uint32_t, uint32_t> GetNumCoefficientBanks();

    // Required when the core is configured in "Lite" protocol mode
    void SetInputResolution(const uint32_t width, const uint32_t height);

    void SetOutputResolution(const uint32_t width, const uint32_t height);
    bool SetCoefficientBanks(const uint32_t v_bank, const uint32_t h_bank);

    // Only required when the core is configured in "Full" protocol mode
    void Commit();

    // Generate and apply filter coefficients using explicitly specified input and output resolutions
    // In the pair first refers to the horizontal dimension, second to the vertical
    using pair_t = std::pair<uint32_t, uint32_t>;
    bool GenerateAndLoadCoefficients(const pair_t& input, const pair_t& output, const pair_t& banks);

private:
    // Disable copy and assignment
    Scaler(const Scaler&) = delete;
    Scaler& operator=(const Scaler&) = delete; 
  
    void PrintCoreConfig();

    Hapi::VvpScalerPtr _spVvpScaler;
};

} // namespace SwApi