/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "CscControls.h"

#ifdef __linux__
#include <unistd.h>
#endif


CscControls::CscControls(const std::shared_ptr<SwApi::ICsc>& spCsc, bool enableDebugUi)
: _spCsc(spCsc), _enableDebugUi(enableDebugUi)
{
    for(std::size_t i = 0; i < INTEL_VVP_CSC_NUM_COLOR_PLANES; ++i)
    {
        _coefficientMatrix.coeffs[i] = {0.0f, 0.0f, 0.0f};
        _coefficientMatrix.s[i] = 0.0f;
    }

    if (_spCsc)
    {
        _spCsc->GetCoeffs(kIntelVvpCscPassthrough, &_coefficientMatrix, &_rescale, &_inCs, &_outCs);
    }
}

bool CscControls::PushCoefficientMatrix() 
{
    if (!_spCsc->ApplyCoefficientMatrix(&_coefficientMatrix, _rescale, _inCs, _outCs)) 
    {
        std::cout << "FAILED: Color Space Conversion NOT set" << '\n';
        return false;
    }
    return true;
}

std::vector<std::shared_ptr<UiControlContainer>> CscControls::AddUiElements()
{
    if (!_spCsc)
        return {};

    auto spContainer = std::make_shared<UiControlContainer>("Colour Space Converter", GetSettingsSectionName());

    if (_enableDebugUi) 
    {
        // nested coeff float input control callback to pass id/index
        auto coeffWithIdCb = [this](int coeffId) {
            auto coeffCb =  [this, coeffId](uint32_t clientId, const float& value) {
                ((float*)&_coefficientMatrix)[coeffId] = value;
                PushCoefficientMatrix();
            };
            return coeffCb;
        };
        
        /// TODO: fix the minimum and maximum settings otherwise its all broken
        for (int i = 0; i < _coeffCount; i++) 
        {
            std::string coeffIdx = std::to_string(i);
            _spCoeffItems.push_back(
                spContainer->AddFloatControl("Coeff " + coeffIdx, -_coeffMax, _coeffMax, coeffWithIdCb(i), "coeffs", 0.0f)
            );
        }

        // rescale slider callback and control
        auto rescaleCb = [this](uint32_t clientId, const int& value) 
        {
            _rescale = value;
            PushCoefficientMatrix();
        };
        _spRescaleSlider = spContainer->AddSliderControl("Rescale", -8, 8, rescaleCb, "rescale", 0);
    }

        // conversion selection drop down
    auto _spSelectConvCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void 
    {
        eIntelVvpCscConversion csConv = (eIntelVvpCscConversion)selected._userItemData;
        if (_spCsc->GetCoeffs(csConv, &_coefficientMatrix, &_rescale, &_inCs, &_outCs)) 
        {
            if (_enableDebugUi)
            {
                // Update coefficient matrix float control values, no callback to stop Push spam
                float* coeffMat = (float*)&_coefficientMatrix;
                for (int i = 0; i < _coeffCount; i++) 
                {
                    _spCoeffItems[i]->UpdateValue(coeffMat[i], false);
                }
                _spRescaleSlider->UpdateValue(_rescale, false);
            }
            // Push new conversion matrix
            PushCoefficientMatrix();
        } 
        else 
        {
            std::cerr << "[" << __FILE__ << ':' << __LINE__ << "] "
                      << "Failed as color space conversion not supported " << '\n';
        }
    };
    _spConversionSelect = spContainer->AddEnumControl(  "Colour Space Conversion",
                                                        _convOptions,
                                                        _spSelectConvCB,
                                                        "csConv",
                                                        (uint32_t)kIntelVvpCscPassthrough);

    return {std::move(spContainer)};
}
