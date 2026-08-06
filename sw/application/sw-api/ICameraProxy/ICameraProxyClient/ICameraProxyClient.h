/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "ICamera.h"
#ifndef __ICAMERA_PROXY_CLIENT_H__
#define __ICAMERA_PROXY_CLIENT_H__

class ICameraProxyClient : public ICamera
{
public:
    ICameraProxyClient(const char* const model, const uint32_t idx=0U);
    virtual ~ICameraProxyClient();
    ICameraProxyClient(const ICameraProxyClient&) = delete;
    ICameraProxyClient& operator=(const ICameraProxyClient&) = delete;

    virtual uint32_t GetMIPIInterface() final override;

    virtual void Start() final override;
    virtual void Stop() final override;
    virtual void Reset() final override;

    virtual uint32_t GetFocus() final override;
    virtual void SetFocus(const uint32_t focus) final override;
    virtual std::pair<uint32_t, uint32_t> GetFocusRange() final override;

    virtual float GetExposure() final override;
    virtual void SetExposure(const float exposure) final override;
    virtual float GetExposureTime() final override;
    virtual std::pair<float, float> GetExposureRange() final override;
    virtual float GetExposureStep() final override;

    virtual float GetFrameRate() final override;
    virtual void SetFrameRate(const float frameRate) final override;
    virtual std::pair<float, float> GetFrameRateRange() final override;
    virtual float GetFrameRateStep() final override;

    virtual float GetShutterSpeed() final override;
    virtual void SetShutterSpeed(float shutterSpeed) final override;
    virtual std::pair<float, float> GetShutterSpeedRange() final override;
    virtual float GetShutterSpeedStep() final override;

    virtual float GetAnalogueGain() final override;
    virtual void SetAnalogueGain(const float analogueGain) final override;
    virtual std::pair<float, float> GetAnalogueGainRange() final override;
    virtual float GetAnalogueGainStep() final override;

    virtual float GetDigitalGain() final override;
    virtual void SetDigitalGain(const float digitalGain) final override;
    virtual std::pair<float, float> GetDigitalGainRange() final override;
    virtual float GetDigitalGainStep() final override;

    virtual std::pair<uint16_t, uint16_t> GetResolution() final override;
    virtual void SetResolution(const std::pair<uint16_t, uint16_t>& resolution) final override;

    virtual uint32_t GetTargetFrameRate() final override;
    virtual void SetTargetFrameRate(const uint32_t framerate) final override;

    virtual TCfaPhase GetCfaPhase() final override;
    virtual void SetCfaPhase(const TCfaPhase& cfaPhase) final override;

    virtual void SetHDRState(bool hdrState) final override;
    virtual bool GetHDRState() final override;

    virtual std::string GetModel() final override;

    virtual void CtlWrite(const uint32_t addr, const uint32_t val) final override;
    virtual uint32_t CtlRead(const uint32_t addr) final override;

private:
    int m_fdDriver;
};

#endif //__ICAMERA_PROXY_CLIENT_H__