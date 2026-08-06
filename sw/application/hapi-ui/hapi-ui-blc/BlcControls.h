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
#include "IspCommon.h"
#include "Blc.h"

/// Encapsulates UI functionality for the Black Level Correction IP
class BlcControls : public IpUiControls
{
public:
    /// Creates an instance of the class that'll manage the given BLC instance.
    BlcControls(const std::string& name, 
                const std::shared_ptr<SwApi::Blc>& spBlc,
                bool enableDebugUi = true, bool powerUser = false);
    /// Generates the UI, and returns the resulting container(s)
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;

	std::string GetSettingsSectionName() override { std::string settings_name(_name); std::replace(settings_name.begin(), settings_name.end(), ' ', '_'); return settings_name; }
    void BlockUIBasedOnBLSCallback(bool isBLSDrivingBLC);
    void BypassBasedOnAWBCallback(bool isBypass);
    void UpdateControlsFromHardware();
    
private:
    const std::string _name;
    std::shared_ptr<SwApi::Blc> _spBlc;

    std::shared_ptr<UiControlItemBoolean> _spBypassControl;
    std::shared_ptr<UiControlItemEnum>    _spCfaPhaseControl;
    std::shared_ptr<UiControlItemBoolean> _spClipZeroControl;

    std::shared_ptr<UiControlItemUInteger> _spBlackPedestal00;
    std::shared_ptr<UiControlItemUInteger> _spBlackPedestal01;
    std::shared_ptr<UiControlItemUInteger> _spBlackPedestal10;
    std::shared_ptr<UiControlItemUInteger> _spBlackPedestal11;

    std::shared_ptr<UiControlItemUInteger> _spColorScalar00;
    std::shared_ptr<UiControlItemUInteger> _spColorScalar01;
    std::shared_ptr<UiControlItemUInteger> _spColorScalar10;
    std::shared_ptr<UiControlItemUInteger> _spColorScalar11;

    bool _enableDebugUi = false;
    bool _powerUser = false;
};
