/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once
#include "IpUiControls.h"
#include "MultiChannelWhiteBalance.h"

class MultiChannelWhiteBalanceControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given Switch instance.
    MultiChannelWhiteBalanceControls(const std::shared_ptr<SwApi::MultiChannelWhiteBalance>& spMultiChannelWhiteBalance);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

    std::string GetSettingsSectionName() override { return "MultiChannelWhiteBalance"; };

private:
    std::shared_ptr<SwApi::MultiChannelWhiteBalance>    _spMultiChannelWhiteBalance;

    std::shared_ptr<UiControlItemLabel>                 _spContributionToNormalLabel;
    std::vector<std::shared_ptr<UiControlItemSlider>>   _spContributionToNominal;
    std::shared_ptr<UiControlItemSlider>                _spMaxDeviationFromNominal;

    std::vector<uint8_t> _contributionToNominal;
    uint8_t _maxDeviationFromNominal;
};
