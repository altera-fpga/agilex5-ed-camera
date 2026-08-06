/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "WhiteBalance.h"

namespace SwApi
{
    class MultiChannelWhiteBalance : public IMultiChannelWhiteBalance, public std::enable_shared_from_this<MultiChannelWhiteBalance>
    {
    public:
        static std::shared_ptr<MultiChannelWhiteBalance> Create();

        MultiChannelWhiteBalance();
        virtual ~MultiChannelWhiteBalance();

        void SetWhiteBalanceControllers(const std::vector<std::shared_ptr<WhiteBalanceController>>& vspWhiteBalance);

        void SetChannelContributionToNominal(uint32_t handle, float contribution);
        void SetMaxDeviationFromNormal(float maxDeviation);

        virtual void BalanceTemp(uint32_t handle, uint32_t& temp) final override;

        uint32_t GetNumberOfChannels();
    private:
        using pipelineStats = struct {
            bool valid;
            uint32_t temp;
            float contribution;
        };
        std::vector<pipelineStats> _pipelines;
        float _maxDeviation;
    };
} // namespace SwApi