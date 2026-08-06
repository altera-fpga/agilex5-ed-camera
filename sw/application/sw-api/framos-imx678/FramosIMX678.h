/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#pragma once

#include "ICamera.h"
#include "I2C.h"
#include "FramosIMX678Types.h"


namespace SwApi
{
    class FramosImx678 : public ICamera
    {
    public:
        static std::shared_ptr<ICamera> Create(const uint32_t idx, const uint32_t targetFrameRate);

        ~FramosImx678() {};

        uint32_t GetMIPIInterface() override;

        void Start() override;
        void Stop() override;

        void Reset() override;

        uint32_t GetFocus() override;
        void SetFocus(const uint32_t v)override;
        std::pair<uint32_t, uint32_t> GetFocusRange() override;

        float GetExposure() override;
        void SetExposure(const float v) override;
        float GetExposureTime() override;
        std::pair<float, float> GetExposureRange() override;
        float GetExposureStep()override;

        float GetFrameRate() override;
        void SetFrameRate(const float) override;
        std::pair<float, float> GetFrameRateRange() override;
        float GetFrameRateStep() override;

        float GetShutterSpeed() override;
        void SetShutterSpeed(float ms) override;
        std::pair<float, float> GetShutterSpeedRange() override;
        float GetShutterSpeedStep() override;

        float GetAnalogueGain() override;
        void SetAnalogueGain(const float gain) override;
        std::pair<float, float> GetAnalogueGainRange() override;
        float GetAnalogueGainStep() override;

        float GetDigitalGain() override;
        void SetDigitalGain(const float gain) override;
        std::pair<float, float> GetDigitalGainRange() override;
        float GetDigitalGainStep() override;

        std::pair<uint16_t, uint16_t> GetResolution() override;
        void SetResolution(const std::pair<uint16_t, uint16_t>&) override;

        uint32_t GetTargetFrameRate() override;
        void SetTargetFrameRate(const uint32_t) override;

        TCfaPhase GetCfaPhase() override;
        void SetCfaPhase(const TCfaPhase&) override;

        void SetHDRState(bool state) override;
        bool GetHDRState() override;

        std::string GetModel() override { return std::string{"FSM-IMX678"} + (_hasGmsl ? " (GMSL)" : ""); }

        void CtlWrite(const uint32_t addr, const uint32_t val) override;
        uint32_t CtlRead(const uint32_t addr) override;

    private:
        FramosImx678(const uint32_t idx, const uint32_t targetFrameRate);

        bool Probe();
        void Initialise();

        void WriteRegs(const img678_reg* p);

        void I2CWrite(uint16_t addr, uint8_t byte);
        uint8_t I2CRead(uint16_t addr);
        void I2CWrite16(uint16_t addr, uint16_t word);

        uint32_t _idx;
        uint32_t _targetFrameRate;
        float _sdrMaxFrameRate = 60.0F;
        const float _hdrMaxFrameRate = 25;
        float _sdrVmaxMultiplier = _sdrMaxFrameRate * 2250.0F;
        const float _hdrVmaxMultiplier = _hdrMaxFrameRate * 4500;
        float _sdrHmaxMultiplier;
        const float _hdrHmaxMultiplier = _hdrMaxFrameRate * 660;
        float _frameRateLow;
        float _frameRateHigh;
        bool _hdrEnabled;
        const float _shr0Offset = 6;
        bool _hasGmsl = false;

        std::shared_ptr<I2C> _spI2C;
        using TFramosIMX678Device = I2CDevice<Reg<16>, Val<8>>;
        std::shared_ptr<TFramosIMX678Device> _spI2CDevice;
    };
}
