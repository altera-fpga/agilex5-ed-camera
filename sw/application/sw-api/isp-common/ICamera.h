/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <cstdint>
#include <utility>
#include <memory>

enum class TCfaPhase : uint8_t 
{
    RGGB = 0,
    GRBG = 1,
    GBRG = 2,
    BGGR = 3,
};

class ICamera
{
public:
    virtual ~ICamera() {};

    virtual uint32_t GetMIPIInterface() = 0;

    virtual void Start() = 0;
    virtual void Stop() = 0;

    virtual void Reset() = 0;

    virtual uint32_t GetFocus() = 0;
    virtual void SetFocus(const uint32_t) = 0;
    virtual std::pair<uint32_t, uint32_t> GetFocusRange() = 0;

    virtual float GetExposure() = 0;
    virtual void SetExposure(const float) = 0;
    virtual float GetExposureTime() = 0;
    virtual std::pair<float, float> GetExposureRange() = 0;
    virtual float GetExposureStep() = 0;

    virtual float GetFrameRate() = 0;
    virtual void SetFrameRate(const float) = 0;
    virtual std::pair<float, float> GetFrameRateRange() = 0;
    virtual float GetFrameRateStep() = 0;

    virtual float GetShutterSpeed() = 0;
    virtual void SetShutterSpeed(float) = 0;
    virtual std::pair<float, float> GetShutterSpeedRange() = 0;
    virtual float GetShutterSpeedStep() = 0;

    virtual float GetAnalogueGain() = 0;
    virtual void SetAnalogueGain(const float) = 0;
    virtual std::pair<float, float> GetAnalogueGainRange() = 0;
    virtual float GetAnalogueGainStep() = 0;

    virtual float GetDigitalGain() = 0;
    virtual void SetDigitalGain(const float) = 0;
    virtual std::pair<float, float> GetDigitalGainRange() = 0;
    virtual float GetDigitalGainStep() = 0;

    virtual std::pair<uint16_t, uint16_t> GetResolution() = 0;
    virtual void SetResolution(const std::pair<uint16_t, uint16_t>&) = 0;

    virtual uint32_t GetTargetFrameRate() = 0;            // FPS x 100
    virtual void SetTargetFrameRate(const uint32_t) = 0;  // FPS x 100

    virtual TCfaPhase GetCfaPhase() = 0;
    virtual void SetCfaPhase(const TCfaPhase&) = 0;

    virtual void SetHDRState(bool) = 0;
    virtual bool GetHDRState() = 0;

    virtual std::string GetModel() = 0;

    virtual void CtlWrite(const uint32_t addr, const uint32_t val) = 0;
    virtual uint32_t CtlRead(const uint32_t addr) = 0;
};

using ICameraPtr = std::shared_ptr<ICamera>;