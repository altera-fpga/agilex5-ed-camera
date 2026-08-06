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
#include "ICsc.h"

class CscControls : public IpUiControls
{
public:
    CscControls(const std::shared_ptr<SwApi::ICsc>& spCsc, bool enableDebugUi);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return "ColorSpaceConversion"; }

private:
    bool PushCoefficientMatrix();

    std::shared_ptr<SwApi::ICsc> _spCsc;
    bool _enableDebugUi = false;
    const float _coeffMax = (float)(1 << 30);
    const int _coeffCount = 12;
    std::vector<std::shared_ptr<UiControlItemFloat>> _spCoeffItems;
    std::shared_ptr<UiControlItemSlider> _spRescaleSlider;
    std::shared_ptr<UiControlItemEnum> _spConversionSelect;

    intel_vvp_coefficients _coefficientMatrix;
    int _rescale = 0;
    uint8_t _inCs = 0;
    uint8_t _outCs = 0;

    std::vector<UiEnumOption> _convOptions = {
        {"Passthrough", (uint32_t)kIntelVvpCscPassthrough},                     // Passthrough
        {"RGB (Computer) to YCC (SDTV)", (uint32_t)kIntelVvpCscComputerSdtv},   // 8 bits (computer) BGR -> CbYCr SDTV
        {"YCC (SDTV) to RGB (Computer)", (uint32_t)kIntelVvpCscSdtvComputer},   // 8 bits CbYCr SDTV -> (computer) BGR
        {"RGB (Studio) to YCC (SDTV)", (uint32_t)kIntelVvpCscStudioSdtv},       // 8 bits (studio) BGR <-> CbYCr SDTV
        {"YCC (SDTV) to RGB (Studio)", (uint32_t)kIntelVvpCscSdtvStudio},       // 8 bits CbYCr SDTV -> (studio) BGR
        {"RGB (Computer) to YCC (HDTV)", (uint32_t)kIntelVvpCscComputerHdtv},   // 8 bits (computer) BGR -> CbYCr HDTV, beware that summands are set for a 8 bits per sample input
        {"YCC (HDTV) to RGB (Computer)", (uint32_t)kIntelVvpCscHdtvComputer}    // 10 bit CbYCr HDTV -> (computer) BGR, beware that summands are set for a 10 bits per sample input
    };
    
};
