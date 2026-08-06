/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "MultiChannelWhiteBalanceControls.h"
#include "UiElements.h"

#include <string>
#include <iostream>

MultiChannelWhiteBalanceControls::MultiChannelWhiteBalanceControls(const std::shared_ptr<SwApi::MultiChannelWhiteBalance>& spMultiChannelWhiteBalance)
: _spMultiChannelWhiteBalance(spMultiChannelWhiteBalance)
, _maxDeviationFromNominal(0)
{
    uint32_t numChannels = _spMultiChannelWhiteBalance->GetNumberOfChannels();
    uint8_t contributionRemaining = 100;
    uint8_t channelsRemaining = numChannels;
    for(uint32_t pipeline_index = 0; pipeline_index < numChannels; pipeline_index++)
    {
        uint8_t contribution = contributionRemaining / channelsRemaining;
        _contributionToNominal.emplace_back(contribution);
        contributionRemaining -= contribution;
        channelsRemaining--;
    }
}

std::vector<std::shared_ptr<UiControlContainer>> MultiChannelWhiteBalanceControls::AddUiElements() 
{
    auto spContainer = std::make_shared<UiControlContainer>("Multi-Channel White Balance", GetSettingsSectionName());

    _spContributionToNormalLabel = spContainer->AddLabelControl("Contribution to normal", "[%]");

    if(_spMultiChannelWhiteBalance)
    {
        uint32_t numChannels = _spMultiChannelWhiteBalance->GetNumberOfChannels();
        for(uint32_t pipeline_index = 0; pipeline_index < numChannels; pipeline_index++)
        {
            auto contributionCB = [this, pipeline_index, numChannels](uint32_t clientID, int32_t& val)
            {
                uint8_t contribution = (uint8_t)val;
                uint8_t contributionChange = contribution - _contributionToNominal[pipeline_index];
                uint32_t channelsToAdjustRemaining = numChannels-1;

                for(uint32_t index = 0; index < numChannels; index++)
                {
                    if(index == pipeline_index)
                    {
                        _contributionToNominal[index] = contribution;
                    }
                    else
                    {
                        uint8_t adjustment = contributionChange / channelsToAdjustRemaining;
                        _contributionToNominal[index] -= adjustment;
                        contributionChange -= adjustment;
                        channelsToAdjustRemaining--;
                    }
                    _spMultiChannelWhiteBalance->SetChannelContributionToNominal(index, (float)_contributionToNominal[index]/100.0);
                    if((index < _spContributionToNominal.size()) && _spContributionToNominal[index])
                    {
                        _spContributionToNominal[index]->UpdateValue(_contributionToNominal[index]);
                    }
                }
            };
            std::stringstream ssName;
            ssName << "Channel " << pipeline_index;
            std::string name = ssName.str();
            auto spContributionControl = spContainer->AddSliderControl(name.c_str(), 0, 100, contributionCB, name.c_str(), _contributionToNominal[pipeline_index]);
            _spContributionToNominal.emplace_back(spContributionControl);
        }
    }

    auto deviationCB = [this](uint32_t clientID, int32_t& val)
    {
        _maxDeviationFromNominal = val;
        _spMultiChannelWhiteBalance->SetMaxDeviationFromNormal((float)_maxDeviationFromNominal/100.0);
    };
    _spMaxDeviationFromNominal = spContainer->AddSliderControl("Max deviation from normal [%]", 0, 100, deviationCB, "Max deviation from normal", _maxDeviationFromNominal);

    return std::vector<std::shared_ptr<UiControlContainer>>{std::move(spContainer)};
}
