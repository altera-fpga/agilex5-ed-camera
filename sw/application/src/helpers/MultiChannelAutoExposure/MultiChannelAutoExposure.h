/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "AutoExposure.h"

namespace SwApi
{
    class MultiChannelAutoExposure : public IMultiChannelAutoExposure, public std::enable_shared_from_this<MultiChannelAutoExposure>
    {
    public:
        static std::shared_ptr<MultiChannelAutoExposure> Create();

        MultiChannelAutoExposure();
        virtual ~MultiChannelAutoExposure();

        void SetAutoExposureControllers(const std::vector<std::shared_ptr<AutoExposureController>>& vspAutoExposure);

        void SetChannelContributionToNominal(uint32_t handle, float contribution);
        void SetMaxDeviationFromNormal(float maxDeviation);

        virtual void BalanceMean(uint32_t handle, float& mean) final override;
        virtual void BalanceSaturation(uint32_t handle, float& sat_proportion) final override;
        virtual void BalanceUnderSaturation(uint32_t handle, float& under_proportion) final override;

        uint32_t GetNumberOfChannels();
    private:
        using pipelineStats = struct {
            bool valid;
            float mean;
            float sat_proportion;
            float under_proportion;
            float contribution;
        };
        std::vector<pipelineStats> _pipelines;
        float _maxDeviation;
    };
} // namespace SwApi