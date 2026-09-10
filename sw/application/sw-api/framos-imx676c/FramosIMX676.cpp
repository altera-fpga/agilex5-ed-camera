/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#include <cmath>
#include <chrono>
#include <iostream>
#include <thread>
#include <format>
#include "FramosIMX676.h"
#include "FramosIMX676Regs.h"

#undef DEBUG

#ifdef DEBUG

#define I2C_WRITE(reg, value)                                                       \
{                                                                                   \
    std::cout << #reg << ": 0x" << std::hex << (uint16_t)(value) << std::dec << std::endl;    \
    I2CWrite(reg, (value));                                                         \
}                                                                                   \

#define I2C_WRITE16(reg, value)                                                     \
{                                                                                   \
    std::cout << #reg << " addr: 0x" << std::hex << reg << std::dec << std::endl;   \
    std::cout << "  0x" << std::hex << reg + 0 << ": 0x" << (uint16_t)((value) >> 8) << std::dec << std::endl;    \
    std::cout << "  0x" << std::hex << reg + 1 << ": 0x" << std::hex << (uint16_t)((value) & 0xFF) << std::dec << std::endl;  \
    I2CWrite16(reg, (value));                                                       \
}

#else
#define I2C_WRITE(reg, value) I2CWrite(reg, value);
#define I2C_WRITE16(reg, value) I2CWrite16(reg, value);
#endif

namespace SwApi
{
    std::shared_ptr<ICamera> FramosImx676::Create(const uint32_t idx, const uint32_t targetFrameRate)
    {
        std::shared_ptr<FramosImx676> p{nullptr};

        try{
            p = std::shared_ptr<FramosImx676>(new FramosImx676(idx, targetFrameRate));
        }
        catch(const std::exception& e){
#ifdef DEBUG
            std::cerr << e.what() << "\n";
#endif /* DEBUG */
        };

        return p;
    }

    FramosImx676::FramosImx676(const uint32_t idx, const uint32_t targetFrameRate):
        _idx(idx),
        //_targetFrameRate{targetFrameRate},
        _targetFrameRate{3000}, // Currently limited to 30fps due to MIPI configuration
        _sdrMaxFrameRate{(_targetFrameRate == 6000) ? 60.0F : 30.0F},
        _sdrVmaxMultiplier{_sdrMaxFrameRate * 3940.0F},
        _sdrHmaxMultiplier{_sdrMaxFrameRate * 314.0F}
    {
        _spI2C = SwApi::I2C::Create();

        const uint8_t imx676Address = (idx == 0 ? 0x37 : 0x1a);
        static constexpr uint16_t imx676Bus = 0x1;

        bool probe_ok = false;

        auto cameraBus = _spI2C->GetBus(imx676Bus);

        if(cameraBus)
        {
            _spI2CDevice = cameraBus->AddDevice<TFramosIMX676Device>(imx676Address, "Framos IMX676C " + std::to_string(idx));
            probe_ok = Probe();

            // Check if the sesnor is behind GMSL
            // GMSL setup is setting 0x36 as the slave address for the sensor
            if(!probe_ok)
            {
                // Also try GMSL kits
                const uint8_t GMSL_CAMERA_ADDRESS = (idx == 0 ? 0x36 : 0x10);
                _spI2CDevice = cameraBus->AddDevice<TFramosIMX676Device>(GMSL_CAMERA_ADDRESS, "Framos IMX676C " + std::to_string(idx));
                probe_ok = Probe();
                _hasGmsl = probe_ok;
            }            
        }

        if(!probe_ok)
            throw std::runtime_error(std::format("Framos IMX676C not available on i2c-{}@0x{:x}", imx676Bus, imx676Address));

        Initialise();
    }

    // Take the sensor out of standby, then confirm chip ID 676
    // at 0x4D12 / 0x4D13 (76, 6).
    bool FramosImx676::Probe()
    {
        const bool inStandby = I2CRead(0x3000) == 0x1;

        if(inStandby)
        {
            I2CWrite(0x3014, 0x1);
            I2CWrite(0x3000, 0x0);
            std::this_thread::sleep_for(std::chrono::milliseconds(IMX676_INT_REGULATOR_WAIT_MS));
            I2CWrite(0x3002, 0x0);
        }

        const uint8_t id_lsb = I2CRead(0x4D12);
        const uint8_t id_msb = I2CRead(0x4D13);

        if(inStandby)
        {
            I2CWrite(0x3000, 0x1);
            I2CWrite(0x3002, 0x1);
        }

        return (id_lsb == 0x76) && (id_msb == 0x06);
    }

    void FramosImx676::Initialise()
    {
        WriteRegs(imx676_stop);
        WriteRegs(imx676_init_settings);
        // WriteRegs((_targetFrameRate == 6000) ? imx676_60fps : imx676_30fps);
        WriteRegs(imx676_30fps);

        Start();

        _hdrEnabled = false;
        _frameRateLow = 4;
        _frameRateHigh = (_targetFrameRate == 6000) ? 60 : 30;
    }

    uint32_t FramosImx676::GetMIPIInterface()
    {
        return _idx;
    }

    uint32_t FramosImx676::GetFocus()
    {
        return 0;
    }

    void FramosImx676::SetFocus(const uint32_t v)
    {
    }

    std::pair<uint32_t, uint32_t> FramosImx676::GetFocusRange()
    {
        return std::make_pair<uint32_t, uint32_t>(0, 0);
    }

    float FramosImx676::GetExposure()
    {
        return 0.0f;
    }

    void FramosImx676::SetExposure(const float v)
    {
    }

    float FramosImx676::GetExposureTime()
    {
        // Return exposure time in seconds
        return (1.0 / GetFrameRate()) * (GetShutterSpeed() / 100.0);
    }

    std::pair<float, float> FramosImx676::GetExposureRange()
    {
        return std::make_pair(1.0f, 2.0f);
    }

    float FramosImx676::GetExposureStep()
    {
        return 0.1f;
    }

    float FramosImx676::GetFrameRate()
    {
        uint32_t vmaxVal = (I2CRead(REG18) & REG18_MASK) << 16;
        vmaxVal |= (I2CRead(REG17) & REG17_MASK) << 8;
        vmaxVal |= I2CRead(REG16) & REG16_MASK;

        return _sdrVmaxMultiplier / vmaxVal;
    }

    void FramosImx676::SetFrameRate(const float v)
    {
        uint32_t vmaxVal = _sdrVmaxMultiplier / v;

        // Set the exposure time to minimum to prevent bright overshoots
        float shutterSpeed = GetShutterSpeed();
        SetShutterSpeed(0.01);
        // Set the frame rate
        I2C_WRITE(REG18, (vmaxVal >> 16) & REG18_MASK)
        I2C_WRITE(REG17, (vmaxVal >> 8) & REG17_MASK)
        I2C_WRITE(REG16, vmaxVal & REG16_MASK)
        // Restore the original exposure time
        SetShutterSpeed(shutterSpeed);
    }

    std::pair<float, float> FramosImx676::GetFrameRateRange()
    {
        _frameRateHigh = _sdrMaxFrameRate;

        return std::make_pair(_frameRateLow, _frameRateHigh);
    }

    float FramosImx676::GetFrameRateStep()
    {
        return 0.01f;
    }

    float FramosImx676::GetDigitalGain()
    {
        return 1.0f;
    }

    void FramosImx676::SetDigitalGain(const float gain)
    {
        // Unused for IMX676
    }

    std::pair<float, float> FramosImx676::GetDigitalGainRange()
    {
        return std::make_pair(1.0f, 1.0f);
    }

    float FramosImx676::GetDigitalGainStep()
    {
        return 0.1f;
    }

    void FramosImx676::SetResolution(const std::pair<uint16_t, uint16_t>&)
    {
    }

    uint32_t FramosImx676::GetTargetFrameRate()
    {
        return _targetFrameRate;
    }

    void FramosImx676::SetTargetFrameRate(const uint32_t frame_rate)
    {
        if(frame_rate != _targetFrameRate)
        {
            _targetFrameRate = frame_rate;
            _sdrMaxFrameRate = (_targetFrameRate == 6000) ? 60.0F : 30.0F;
            _sdrVmaxMultiplier = _sdrMaxFrameRate * 3940.0F;
            _sdrHmaxMultiplier = _sdrMaxFrameRate * 314.0F;
            //Stop();
            //WriteRegs((_targetFrameRate == 6000) ? imx676_60fps : imx676_30fps);
            //Start();
    	}
    }

    TCfaPhase FramosImx676::GetCfaPhase()
    {
        return TCfaPhase::RGGB;
    }

    void FramosImx676::SetCfaPhase(const TCfaPhase&)
    {
    }

    void FramosImx676::Start()
    {
        WriteRegs(imx676_start);
    }
    void FramosImx676::Stop()
    {
        WriteRegs(imx676_stop);
    }

    void FramosImx676::Reset()
    {
        Stop();

        Initialise();

        Start();
    }

    void FramosImx676::SetShutterSpeed(float percent)
    {
        uint32_t vmaxVal = (I2CRead(REG18) & REG18_MASK) << 16;
        vmaxVal |= (I2CRead(REG17) & REG17_MASK) << 8;
        vmaxVal |= I2CRead(REG16) & REG16_MASK;

        float shr0Range = vmaxVal - 2 * _shr0Offset;

        uint32_t shr0Val = static_cast<uint32_t>(_shr0Offset + shr0Range * (100.00f - percent) / 100.00f);

        I2C_WRITE(REG35, shr0Val & REG35_MASK)
        I2C_WRITE(REG36, (shr0Val >> 8) & REG36_MASK)
        I2C_WRITE(REG37, (shr0Val >> 16) & REG37_MASK)
    }

    float FramosImx676::GetShutterSpeed()
    {
        uint32_t shr0Val = (I2CRead(REG37) & REG37_MASK) << 16;
        shr0Val |= (I2CRead(REG36) & REG36_MASK) << 8;
        shr0Val |= I2CRead(REG35) & REG35_MASK;

        uint32_t vmaxVal = (I2CRead(REG18) & REG18_MASK) << 16;
        vmaxVal |= (I2CRead(REG17) & REG17_MASK) << 8;
        vmaxVal |= I2CRead(REG16) & REG16_MASK;

        float shr0Range = vmaxVal - 2 * _shr0Offset;

        float percent = 100.00f * (_shr0Offset + shr0Range - shr0Val) / shr0Range;
        return percent;
    }

    std::pair<float, float> FramosImx676::GetShutterSpeedRange()
    {
        return std::make_pair(0.01f, 100.00f);
    }

    float FramosImx676::GetShutterSpeedStep()
    {
        return 0.01f;
    }

    void FramosImx676::SetAnalogueGain(float gain)
    {
        uint16_t setting = static_cast<uint16_t>(66.67f * log10f(gain));
        if (setting > 240) setting = 240;
        I2C_WRITE(REG52, setting & REG52_MASK);
    }

    float FramosImx676::GetAnalogueGain()
    {
        uint16_t setting = I2CRead(REG52) & REG52_MASK;

        return powf(10.0f, static_cast<float>(setting) / 66.67f);
    }

    std::pair<float, float> FramosImx676::GetAnalogueGainRange()
    {
        return std::make_pair(1.0f, 31.6f);
    }

    float FramosImx676::GetAnalogueGainStep()
    {
        return 0.1f;
    }

    void FramosImx676::SetHDRState(bool state)
    {
        _hdrEnabled = state;
    }

    bool FramosImx676::GetHDRState()
    {
        return _hdrEnabled;
    }

    std::pair<uint16_t, uint16_t> FramosImx676::GetResolution()
    {
        uint16_t h = (I2CRead(REG27) & REG27_MASK) << 8;
        h |= I2CRead(REG26) & REG26_MASK;

        uint16_t v = (I2CRead(REG34) & REG34_MASK) << 8;
        v |= I2CRead(REG33) & REG33_MASK;

        return {h, v};
    }

    void FramosImx676::I2CWrite16(uint16_t addr, uint16_t word)
    {
        uint8_t writeBytes[4] = {(uint8_t)(addr >> 8),
                                 (uint8_t)(addr & 0xFF),
                                 (uint8_t)(word >> 8),
                                 (uint8_t)(word & 0xFF)};
        _spI2CDevice->Write(writeBytes, 4);
    }

    void FramosImx676::I2CWrite(uint16_t addr, uint8_t byte)
    {
        _spI2CDevice->WriteRegister(addr, byte);
    }

    uint8_t FramosImx676::I2CRead(uint16_t addr)
    {
        // Pre-initialise with 0xff to indicate read failure
        uint8_t store{0xff};
        _spI2CDevice->ReadRegister(addr, store);
        return store;
    }

    void FramosImx676::WriteRegs(const img676_reg* p)
    {
        while(p->addr != IMX676_TABLE_END)
        {
            if(p->addr == IMX676_TABLE_WAIT_MS)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(p->val));
            }
            else
            {
                    I2C_WRITE(p->addr, p->val);
            }

            ++p;
        }
    }


    void FramosImx676::CtlWrite(const uint32_t addr, const uint32_t val)
    {
        I2CWrite(static_cast<uint16_t>(addr & 0xffff), static_cast<uint8_t>(val & 0xff));
    }


    uint32_t FramosImx676::CtlRead(const uint32_t addr)
    {
        return I2CRead(static_cast<uint16_t>(addr & 0xffff));
    }
}
