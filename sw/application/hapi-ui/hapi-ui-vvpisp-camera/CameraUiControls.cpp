/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#include "CameraUiControls.h"
#include <cmath>
#include "AtUtils.h"


CameraUiControls::CameraUiControls(const ICameraPtr camera, const std::string& name, Hapi::VvpExposureFusionPtr exposureFusion, const bool debugControls):
    _camera{camera},
    _name{name},
    _exposureFusion{exposureFusion},
    _debugControls{debugControls},
    _frameRate{0},
    _frameRateHdr{0}
{
}


CameraUiControls::~CameraUiControls()
{
}


std::string CameraUiControls::GetSettingsSectionName()
{
    std::string tmp{_name};
    std::replace(tmp.begin(), tmp.end(), ' ', '_');
    return tmp;
};


std::string CameraUiControls::GetModelName()
{
    const auto model = _camera->GetModel();
    return model.empty() ? "Generic" : std::move(model);
}


void CameraUiControls::EnableHdr(bool v)
{
    if(_exposureFusion)
    {
        if(_spModeEnum)
        {
            // In SDR mode set Exposure Fusion to "Long Exposure Bypass" mode
            auto exposureFusionMode = (v == true ? kIntelVvpExposureFusionModeFusion : kIntelVvpExposureFusionModeLongExposure);

            _spModeEnum->UpdateValue(exposureFusionMode, true);
            _spModeEnum->Enable(v == true);
        }

        const float currentFrameRate = _camera->GetFrameRate();
        float newFrameRate{currentFrameRate};

        _camera->SetHDRState(v);

        if(v)
        {
            _frameRate = currentFrameRate;
            newFrameRate = _frameRateHdr ? _frameRateHdr :  HDR_MAX_FRAME_RATE;
        }
        else
        {
            _frameRateHdr = currentFrameRate;
            newFrameRate = _frameRate;
        }

        UpdateFrameRateSlider(newFrameRate);
    }
}


void CameraUiControls::UpdateHdrBlackLevel(const std::array<uint32_t, 4>& blc)
{
    if(_exposureFusion)
    {
        const auto avg_blc = ((blc[0] + blc[1] + blc[2] + blc[3]) / 4) / 16;

        // UI control only available in Debug mode
        if(_spBlackLevel)
            _spBlackLevel->UpdateValue(avg_blc, false);

        SetExposureFusionBlackLevel(avg_blc);

        const uint32_t ratio = _spRatio ? _spRatio->GetValue() : _HDR_EXPOSURE_RATIO_DEFAULT;

        UpdateHdrThreshold(avg_blc, ratio);
    }
}


void CameraUiControls::UpdateHdrThreshold(const uint32_t blc, const uint32_t ratio)
{
    if(_exposureFusion)
    {
        static constexpr uint32_t bps_in = 12;  // From sensor
        static constexpr uint32_t bps_out = 16; // To the pipeline

        const int tmp = static_cast<int>((static_cast<float>(ratio) / powf(2.0f, static_cast<float>(17 - bps_out)) - powf(2.0f, static_cast<float>(bps_in) - 2.0f)) - (int)blc);
        const auto threshold = static_cast<uint32_t>(std::max(0, tmp));

        if(_spThreshold)
            _spThreshold->UpdateValue(threshold, false);

        SetExposureFusionThreshold(threshold);
    }
}


void CameraUiControls::UpdateFrameRateSlider(const float frameRate)
{
    auto [min, max] = _camera->GetFrameRateRange();
    _frameRateControl->UpdateRange(min, max);
    _frameRateControl->UpdateValue(frameRate, true);
}


void CameraUiControls::UpdateShutterSpeedSlider()
{
    if(_shutterControl)
    {
        const auto shutter_min = 0.01f;
        const auto shutter_max = (1.0f / _camera->GetFrameRate()) * 1000.0f;

        // Make sure the value is within the new range
        float value = _shutterControl->GetValue<float>();
        value = std::clamp(value, shutter_min, shutter_max);
        _shutterControl->UpdateValue(value, true);
        _shutterControl->UpdateRange(shutter_min, shutter_max);
    }
}


std::vector<std::shared_ptr<UiControlContainer>> CameraUiControls::AddUiElements()
{
    std::vector<std::shared_ptr<UiControlContainer>> elements{};

    if(_camera)
    {
        auto spContainer = std::make_shared<UiControlContainer>(_name, GetSettingsSectionName());

        spContainer->AddLabelControl("Model:", GetModelName());

        // Frame rate
        {
            auto frameRateControlCB = [this](uint32_t clientID, float& v)
            {
                _camera->SetFrameRate(v);
                UpdateShutterSpeedSlider();
            };

            const auto [min, max] = _camera->GetFrameRateRange();
            const auto defaultFrameRate = max >= 50 ? 50 : (max >= 25 ? 25 : max);

            _frameRateControl = spContainer->AddSliderControl("Frame Rate (Hz)", min, max,
                                                                frameRateControlCB,
                                                                "frameRate",
                                                                defaultFrameRate);
        }

        // Shutter speed / exposure time
        {
            auto shutterControlCB = [this](uint32_t clientID, float& v)
            {
                const auto shutter_max = (1.0f / _camera->GetFrameRate()) * 1000.0f;
                const float shutter_speed_relative = (v / shutter_max) * 100.0f;

                _camera->SetShutterSpeed(shutter_speed_relative);
            };

            const auto shutter_min = 0.01f;
            const auto shutter_max = (1.0f / _camera->GetFrameRate()) * 1000.0f;
            const auto defaultShutterSpeed = shutter_max;

            _shutterControl = spContainer->AddSliderControl("Shutter Speed (ms)", shutter_min, shutter_max,
                                                            shutterControlCB,
                                                            "shutterSpeedEx",
                                                            defaultShutterSpeed);
        }

        // Analogue gain
        {
            auto analogueGainCB = [this](uint32_t clientID, float analogueGain)
            {
                _camera->SetAnalogueGain(analogueGain);

                if(_analogueGainCb)
                    _analogueGainCb(analogueGain);
            };

            const auto [min, max] = _camera->GetAnalogueGainRange();
            const auto defaultAnalogueGain = min;

            _analogueGainControl = spContainer->AddSliderControl("Analog Gain", min, max,
                                                                analogueGainCB,
                                                                "analogueGain",
                                                                defaultAnalogueGain);
        }

        // Digital gain
        const auto [min, max] = _camera->GetDigitalGainRange();
        if (min < max)
        {
            auto digitalGainCB = [this](uint32_t clientID, float digitalGain)
            {
                _camera->SetDigitalGain(digitalGain);
            };


            const auto defaultDigitalGain = min;

            _digitalGainControl = spContainer->AddSliderControl("Digital Gain", min, max,
                                                                digitalGainCB,
                                                                "digitalGain",
                                                                defaultDigitalGain);
        }

        const auto [fmin, fmax] = _camera->GetFocusRange();
        if(fmin != fmax)
        {
            auto focusControlCB = [this](uint32_t clientID, int32_t& v)
            {
                _camera->SetFocus(v);
            };

            _focusControl = spContainer->AddSliderControl("Focus", static_cast<int32_t>(fmin), static_cast<int32_t>(fmax),
                                                            focusControlCB,
                                                            "focus",
                                                            static_cast<int32_t>(fmax));
        }

        if(_debugControls)
        {
            auto resyncCb = [this](uint32_t clientID)
            {
                _camera->Stop();
                _camera->Start();
            };

            auto spResyncButton = spContainer->AddButtonControl("Resync", resyncCb);

            _spAddressBox = spContainer->AddUIntegerControl("I2C ADDR:", 0x0, 0, 0xFFFF, nullptr, true);
            _spDataBox = spContainer->AddUIntegerControl("I2C DATA:", 0x0, 0, 0xFF, nullptr, true);

            auto readCB = [this](uint32_t clientID)
            {
                _spDataBox->UpdateValue(_camera->CtlRead(_spAddressBox->GetValue()));
            };

            auto spReadButton = spContainer->AddButtonControl("Read", readCB);


            auto writeCB = [this](uint32_t clientID)
            {
                _camera->CtlWrite(_spAddressBox->GetValue(), _spDataBox->GetValue());
            };

            auto spWriteButton = spContainer->AddButtonControl("Write", writeCB);
        }

        // Todo, a better system for this
        if (_exposureFusion && _camera->GetModel() == "FSM-IMX678")
        {
            if(_debugControls)
            {
                spContainer->AddSeparator();

                auto toggleHDRCB = [this](uint32_t clientID, bool value)
                {
                    EnableHdr(value);
                };

                /* Do not persist this for now */
                spContainer->AddBoolControl("HDR", toggleHDRCB, "", false);

                auto modeCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void
                {
                    intel_vvp_exposure_fusion_set_run_mode(_exposureFusion->GetInstance(), static_cast<eIntelVvpExposureFusionMode>(selected._userItemData));
                };

                std::vector<UiEnumOption> exposureFusionModes =
                {
                    { "Fusion", kIntelVvpExposureFusionModeFusion },
                    { "Short Exposure", kIntelVvpExposureFusionModeShortExposure },
                    { "Long Exposure", kIntelVvpExposureFusionModeLongExposure }
                };

                _spModeEnum = spContainer->AddEnumControl("Mode",
                                                        exposureFusionModes,
                                                        modeCB,
                                                        "", /* Do not persist for now! "exposureFusionModes",*/
                                                        kIntelVvpExposureFusionModeLongExposure);

                // Disable mode selection by default
                _spModeEnum->Enable(false);

                std::vector<UiEnumOption> gains =
                {
                    { "0dB", 0 },
                    { "6dB", 1 },
                    { "12dB", 2 },
                    { "18dB", 3 },
                    { "24dB", 4 },
                    { "30dB", 5 }
                };

                auto gainCB = [this](uint32_t clientID, const UiEnumOption& selected, uint32_t) -> void {
                    static constexpr uint32_t hdr_exposure_gain_to_ratio[] = {
                        39086, 21291, 10802, 5237, 2640, 1308
                    };

                    static constexpr int max_idx = sizeof(hdr_exposure_gain_to_ratio) / sizeof(uint32_t);

                    const int selectedDb = std::clamp(static_cast<int>(selected._userItemData), 0, max_idx);

                    _camera->CtlWrite(0x3081, selectedDb);

                    if (!_spRatio) return;

                    const uint32_t ratio_value = hdr_exposure_gain_to_ratio[selectedDb];

                    _spRatio->UpdateValue(ratio_value, true);

                    UpdateHdrThreshold(_spBlackLevel->GetValue(), ratio_value);
                };

                _spGainEnum = spContainer->AddEnumControl("ClearHDR Gain",
                                                        gains,
                                                        gainCB,
                                                        "gainSelector",
                                                        _HDR_GAIN_12dB_DEFAULT);

                auto setBlackLevelCB = [this](uint32_t clientID, uint32_t& v)
                {
                    SetExposureFusionBlackLevel(v);
                };

                _spBlackLevel = spContainer->AddUIntegerControl("Black Level", 0x0, 0, 0xFFFF, setBlackLevelCB, true);

                auto setRatioCB = [this](uint32_t clientID, uint32_t& v)
                {
                    SetExposureFusionRatio(v);
                };

                _spRatio = spContainer->AddUIntegerControl("Ratio", 0x0, 0, 0x1FFFF, setRatioCB, true);

                auto setThresholdCB = [this](uint32_t clientID, uint32_t& v)
                {
                    SetExposureFusionThreshold(v);
                };

                _spThreshold = spContainer->AddUIntegerControl("Threshold", 0x0, 0, 0xFFFF, setThresholdCB, true);

                _spGainEnum->UpdateValue(_HDR_GAIN_12dB_DEFAULT, true);
            }
        }

        elements.emplace_back(spContainer);

        _spContainer = std::move(spContainer);
    }

    return elements;
}


void CameraUiControls::SetShutterSpeed(const float v)
{
    if (_shutterControl)
    {
        static constexpr float shutter_min = 0.01f;
        const auto shutter_max = (1.0f / _camera->GetFrameRate()) * 1000.0f;

        // Convert percentage to miliseconds
        float value = shutter_max * v / 100.0f;
        // Make sure the value is within the new range
        value = std::clamp(value, shutter_min, shutter_max);
        _shutterControl->UpdateValue(value, true);
    }
}


float CameraUiControls::GetShutterSpeed() const
{
    float v = 0.0f;

    if(_shutterControl)
    {
        const auto tmp_v = _shutterControl->GetValue<float>();
        const auto shutter_max = (1.0f / _camera->GetFrameRate()) * 1000.0f;
        v = (tmp_v * 100.0f) / shutter_max;
    }

    return v;
}


void CameraUiControls::SetAnalogueGain(const float v)
{
    if (_analogueGainControl)
        _analogueGainControl->UpdateValue(v, true);
}


float CameraUiControls::GetAnalogueGain() const
{
    return _analogueGainControl ? _analogueGainControl->GetValue<float>() : 0.0f;
}


void CameraUiControls::SetDigitalGain(const float v)
{
    if (_digitalGainControl)
        _digitalGainControl->UpdateValue(v, true);
}


float CameraUiControls::GetDigitalGain() const
{
    return _digitalGainControl ? _digitalGainControl->GetValue<float>() : 0.0f;
}


void CameraUiControls::SetActive(bool isActive)
{
    if(_spContainer)
        _spContainer->SetEnabled(isActive);
}


bool CameraUiControls::SetExposureFusionBlackLevel(const uint32_t v)
{
    return (intel_vvp_exposure_fusion_set_black_level(_exposureFusion->GetInstance(), v) == kIntelVvpCoreOk);
}


bool CameraUiControls::SetExposureFusionRatio(const uint32_t v)
{
    return (intel_vvp_exposure_fusion_set_exposure_ratio(_exposureFusion->GetInstance(), v) == kIntelVvpCoreOk);
}


bool CameraUiControls::SetExposureFusionThreshold(const uint32_t v)
{
    return (intel_vvp_exposure_fusion_set_threshold(_exposureFusion->GetInstance(), v) == kIntelVvpCoreOk);
}