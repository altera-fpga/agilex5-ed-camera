/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "MultiChannelAutoExposure.h"
#include "Logging.h"
#include <cmath>

namespace SwApi
{
    std::shared_ptr<MultiChannelAutoExposure> MultiChannelAutoExposure::Create()
    {
        auto spMultiChannelAutoExposure = std::make_shared<MultiChannelAutoExposure>();

        return spMultiChannelAutoExposure;
    }


    MultiChannelAutoExposure::MultiChannelAutoExposure()
    : _maxDeviation(0.0)
    {
    }

    MultiChannelAutoExposure::~MultiChannelAutoExposure()
    {
    }

    void MultiChannelAutoExposure::SetAutoExposureControllers(const std::vector<std::shared_ptr<AutoExposureController>>& vspAutoExposure)
    {
        IMultiChannelAutoExposurePtr spIMultiChannelAutoExposure = std::dynamic_pointer_cast<IMultiChannelAutoExposure>(shared_from_this());
        uint32_t num_channels = vspAutoExposure.size();
        for(uint32_t pipeline_index = 0; pipeline_index < num_channels; pipeline_index++)
        {
            vspAutoExposure[pipeline_index]->SetMultiChannelAutoExposure(spIMultiChannelAutoExposure, pipeline_index);
            _pipelines.emplace_back(pipelineStats{false, 0.0F, 0.0F, 0.0F, 1.0F/(float)num_channels});
        }
    }

    void MultiChannelAutoExposure::SetChannelContributionToNominal(uint32_t handle, float contribution)
    {
        _pipelines[handle].contribution = contribution;
    }

    void MultiChannelAutoExposure::SetMaxDeviationFromNormal(float maxDeviation)
    {
        _maxDeviation = maxDeviation;
    }

    uint32_t MultiChannelAutoExposure::GetNumberOfChannels()
    {
        return _pipelines.size();
    }

    void MultiChannelAutoExposure::BalanceMean(uint32_t handle, float& mean)
    {
        float balance_mean = 0.0;
        _pipelines[handle].valid = true;
        _pipelines[handle].mean = mean;
        for(auto& pipelineStats : _pipelines)
        {
            if(pipelineStats.valid)
            {
                balance_mean += pipelineStats.contribution * pipelineStats.mean;
            }
        }
        mean = _maxDeviation*mean + (1.0 - _maxDeviation)*balance_mean;
    }

    void MultiChannelAutoExposure::BalanceSaturation(uint32_t handle, float& sat_proportion)
    {
        float balance_sat_proportion = 0.0;
        _pipelines[handle].sat_proportion = sat_proportion;
        for(auto& pipelineStats : _pipelines)
        {
            if(pipelineStats.valid)
            {
                balance_sat_proportion += pipelineStats.contribution * pipelineStats.sat_proportion;
            }
        }
        sat_proportion = _maxDeviation*sat_proportion + (1.0 - _maxDeviation)*balance_sat_proportion;
    }

    void MultiChannelAutoExposure::BalanceUnderSaturation(uint32_t handle, float& under_proportion)
    {
        float balance_under_proportion = 0.0;
        uint32_t count = 0;
        _pipelines[handle].under_proportion = under_proportion;
        for(auto& pipelineStats : _pipelines)
        {
            if(pipelineStats.valid)
            {
                balance_under_proportion += pipelineStats.contribution * pipelineStats.under_proportion;
                count ++;
            }
        }
        under_proportion = _maxDeviation*under_proportion + (1.0 - _maxDeviation)*balance_under_proportion;
    }

} // namespace SwApi