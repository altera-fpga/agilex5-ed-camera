/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "MultiChannelWhiteBalance.h"
#include "Logging.h"
#include <cmath>

namespace SwApi
{
    std::shared_ptr<MultiChannelWhiteBalance> MultiChannelWhiteBalance::Create()
    {
        auto spMultiChannelWhiteBalance = std::make_shared<MultiChannelWhiteBalance>();

        return spMultiChannelWhiteBalance;
    }


    MultiChannelWhiteBalance::MultiChannelWhiteBalance()
    : _maxDeviation(0.0)
    {
    }

    MultiChannelWhiteBalance::~MultiChannelWhiteBalance()
    {
    }

    void MultiChannelWhiteBalance::SetWhiteBalanceControllers(const std::vector<std::shared_ptr<WhiteBalanceController>>& vspWhiteBalance)
    {
        IMultiChannelWhiteBalancePtr spIMultiChannelWhiteBalance = std::dynamic_pointer_cast<IMultiChannelWhiteBalance>(shared_from_this());
        uint32_t num_channels = vspWhiteBalance.size();
        for(uint32_t pipeline_index = 0; pipeline_index < num_channels; pipeline_index++)
        {
            vspWhiteBalance[pipeline_index]->SetMultiChannelWhiteBalance(spIMultiChannelWhiteBalance, pipeline_index);
            _pipelines.emplace_back(pipelineStats{false, 0, 1.0F/(float)num_channels});
        }
    }

    void MultiChannelWhiteBalance::SetChannelContributionToNominal(uint32_t handle, float contribution)
    {
        _pipelines[handle].contribution = contribution;
    }

    void MultiChannelWhiteBalance::SetMaxDeviationFromNormal(float maxDeviation)
    {
        _maxDeviation = maxDeviation;
    }

    uint32_t MultiChannelWhiteBalance::GetNumberOfChannels()
    {
        return _pipelines.size();
    }

    void MultiChannelWhiteBalance::BalanceTemp(uint32_t handle, uint32_t& temp)
    {
        uint32_t balance_temp = 0.0;
        _pipelines[handle].valid = true;
        _pipelines[handle].temp = temp;
        for(auto& pipelineStats : _pipelines)
        {
            if(pipelineStats.valid)
            {
                balance_temp += pipelineStats.contribution*pipelineStats.temp;
            }
        }
        temp = _maxDeviation*temp + (1.0-_maxDeviation)*balance_temp;
        temp = ((temp + 50) / 100) * 100;
    }

} // namespace SwApi