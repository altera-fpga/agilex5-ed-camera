/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "HapiVvpAnr.h"
#include "AnrUtils.h"
#include "VvpCoreBase.h"

#include <cstdint>
#include <vector>

namespace SwApi
{

using namespace AnrUtils;

class Anr: public VvpCoreBase
{
    public:
        static std::shared_ptr<Anr> Create(Hapi::VvpAnrPtr spAnr, uint32_t initialOutputWidth, uint32_t initialOutputHeight);
        Anr(Hapi::VvpAnrPtr spAnr, uint32_t initialOutputWidth, uint32_t initialOutputHeight);

        bool SetBypass(bool bypass, UpdatePolicy policy = UpdatePolicy::Async());
        bool GetBypass(void);

        bool SetResolution(uint32_t width, uint32_t height);

        bool ApplyLuts(float combinedGain, float darkNoise, float intensityStrength = -1, float spatialRadiusScaler = -1, UpdatePolicy policy = UpdatePolicy::Async());
        bool ApplyLuts(float intensityStrength, UpdatePolicy policy = UpdatePolicy::Async());
        bool ApplyLuts(UpdatePolicy policy = UpdatePolicy::Async());
        bool ApplyUnityLuts(); // Essentially a reset

    private:

        struct ViewableFloat
        {
            union
            {
                float f;
                uint32_t ui;
            };

        };

        bool CommitSettings() override;
        bool IsCommitPending() override;

        void GenerateSpatialLut();
        void GenerateIntensityLut();

        Hapi::VvpAnrPtr _spAnr;

        std::vector<uint32_t> _currentSpatialLut;
        std::vector<uint32_t> _currentIntensityLut;

        float _currentIntensityStrength = 1.0f; // Unity by default
        float _currentSpatialRadiusScaler = 1.0f; // Unity by default
        float _currentCombinedGain = -1.0f;
        float _currentDarkNoise = -1.0f;

        uint8_t _spatialLutXDepth;
        uint8_t _spatialLutYDepth;

        bool _bypass;
};

} // namespace SwApi
