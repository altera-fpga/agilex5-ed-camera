/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#include <cmath>
#include <iostream>
#include <thread>
#include <format>
#include "FramosIMX678.h"
#include "FramosIMX678Regs.h"

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
    std::shared_ptr<ICamera> FramosImx678::Create(const uint32_t idx, const uint32_t targetFrameRate)
    {
        std::shared_ptr<FramosImx678> p{nullptr};

        try{
            p = std::shared_ptr<FramosImx678>(new FramosImx678(idx, targetFrameRate));
        }
        catch(const std::exception& e){
#ifdef DEBUG
            std::cerr << e.what() << "\n";
#endif /* DEBUG */
        };

        return p;
    }

    FramosImx678::FramosImx678(const uint32_t idx, const uint32_t targetFrameRate):
        _idx(idx),
        _targetFrameRate{targetFrameRate},
        _sdrMaxFrameRate{(_targetFrameRate == 6000) ? 60.0F : 30.0F},
        _sdrVmaxMultiplier{_sdrMaxFrameRate * 2250.0F},
        _sdrHmaxMultiplier{_sdrMaxFrameRate * 550.0F}
    {
        _spI2C = SwApi::I2C::Create();

        const uint8_t imx678Address = (idx == 0 ? 0x37 : 0x1a);
        static constexpr uint16_t imx678Bus = 0x1;

        bool probe_ok = false;

        auto cameraBus = _spI2C->GetBus(imx678Bus);

        if(cameraBus)
        {
            _spI2CDevice = cameraBus->AddDevice<TFramosIMX678Device>(imx678Address, "Framos IMX678 " + std::to_string(idx));
            probe_ok = Probe();

            // Check if the sesnor is behind GMSL
            // GMSL setup is setting 0x36 as the slave address for the sensor
            if(!probe_ok)
            {
                // Also try GMSL kits
                const uint8_t GMSL_CAMERA_ADDRESS = (idx == 0 ? 0x36 : 0x10);
                _spI2CDevice = cameraBus->AddDevice<TFramosIMX678Device>(GMSL_CAMERA_ADDRESS, "Framos IMX678 " + std::to_string(idx));
                probe_ok = Probe();
                _hasGmsl = probe_ok;
            }
        }

        if(!probe_ok)
            throw std::runtime_error(std::format("Framos IMX678 not available on i2c-{}@0x{:x}", imx678Bus, imx678Address));

        Initialise();
    }

    // Take the sensor out of standby, then confirm chip ID 678
    // at 0x4D12 / 0x4D13 (76, 8).
    bool FramosImx678::Probe()
    {
        const bool inStandby = I2CRead(0x3000) == 0x1;

        if(inStandby)
        {
            I2CWrite(0x3014, 0x1);
            I2CWrite(0x3000, 0x0);
            std::this_thread::sleep_for(std::chrono::milliseconds(IMX678_INT_REGULATOR_WAIT_MS));
            I2CWrite(0x3002, 0x0);
        }

        const uint8_t id_lsb = I2CRead(0x4D12);
        const uint8_t id_msb = I2CRead(0x4D13);

        if(inStandby)
        {
            I2CWrite(0x3000, 0x1);
            I2CWrite(0x3002, 0x1);
        }

        return (id_lsb == 0x76) && (id_msb == 0x08);
    }

    void FramosImx678::Initialise()
    {
        WriteRegs(imx678_stop);
        WriteRegs(imx678_init_settings);
        WriteRegs((_targetFrameRate == 6000) ? imx678_60fps : imx678_30fps);

        _hdrEnabled = false;
        _frameRateLow = 4;
        _frameRateHigh = (_targetFrameRate == 6000) ? 60 : 30;
    }

    uint32_t FramosImx678::GetMIPIInterface()
    {
        return _idx;
    }

    uint32_t FramosImx678::GetFocus()
    {
        return 0;
    }

    void FramosImx678::SetFocus(const uint32_t v)
    {
    }

    std::pair<uint32_t, uint32_t> FramosImx678::GetFocusRange()
    {
        return std::make_pair<uint32_t, uint32_t>(0, 0);
    }

    float FramosImx678::GetExposure()
    {
        return 0.0f;
    }

    void FramosImx678::SetExposure(const float v)
    {
    }

    float FramosImx678::GetExposureTime()
    {
        // Return exposure time in seconds
        return (1.0 / GetFrameRate()) * (GetShutterSpeed() / 100.0);
    }

    std::pair<float, float> FramosImx678::GetExposureRange()
    {
        return std::make_pair(1.0f, 2.0f);
    }

    float FramosImx678::GetExposureStep()
    {
        return 0.1f;
    }

    float FramosImx678::GetFrameRate()
    {
        uint32_t vmaxVal = (I2CRead(REG18) & REG18_MASK) << 16;
        vmaxVal |= (I2CRead(REG17) & REG17_MASK) << 8;
        vmaxVal |= I2CRead(REG16) & REG16_MASK;

        return _hdrEnabled ? _hdrVmaxMultiplier / vmaxVal : _sdrVmaxMultiplier / vmaxVal;
    }

    void FramosImx678::SetFrameRate(const float v)
    {
        // Max frame rate is limited to 25 fps in HDR mode
        uint32_t frameRate = std::max(v, _hdrEnabled ? 25 : _frameRateHigh);
        (void)frameRate; // unused

        uint32_t vmaxVal = _hdrEnabled ? _hdrVmaxMultiplier / v : _sdrVmaxMultiplier / v;

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

    std::pair<float, float> FramosImx678::GetFrameRateRange()
    {
        _frameRateHigh = _hdrEnabled ? _hdrMaxFrameRate : _sdrMaxFrameRate;

        return std::make_pair(_frameRateLow, _frameRateHigh);
    }

    float FramosImx678::GetFrameRateStep()
    {
        return 0.01f;
    }

    float FramosImx678::GetDigitalGain()
    {
        return 1.0f;
    }

    void FramosImx678::SetDigitalGain(const float gain)
    {
        // Unused for IMX678
    }

    std::pair<float, float> FramosImx678::GetDigitalGainRange()
    {
        return std::make_pair(1.0f, 1.0f);
    }

    float FramosImx678::GetDigitalGainStep()
    {
        return 0.1f;
    }

    void FramosImx678::SetResolution(const std::pair<uint16_t, uint16_t>&)
    {
    }

    uint32_t FramosImx678::GetTargetFrameRate()
    {
        return _targetFrameRate;
    }

    void FramosImx678::SetTargetFrameRate(const uint32_t frame_rate)
    {
        if(frame_rate != _targetFrameRate)
        {
            _targetFrameRate = frame_rate;
            _sdrMaxFrameRate = (_targetFrameRate == 6000) ? 60.0F : 30.0F;
            _sdrVmaxMultiplier = _sdrMaxFrameRate * 2250.0F;
            _sdrHmaxMultiplier = _sdrMaxFrameRate * 550.0F;
            Stop();
            WriteRegs((_targetFrameRate == 6000) ? imx678_60fps : imx678_30fps);
            Start();
    	}
    }

    TCfaPhase FramosImx678::GetCfaPhase()
    {
        return TCfaPhase::RGGB;
    }

    void FramosImx678::SetCfaPhase(const TCfaPhase&)
    {
    }

    void FramosImx678::Start()
    {
        WriteRegs(imx678_start);
    }
    void FramosImx678::Stop()
    {
        WriteRegs(imx678_stop);
    }

    void FramosImx678::Reset()
    {
        Stop();

        Initialise();

        Start();
    }

    void FramosImx678::SetShutterSpeed(float percent)
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

    float FramosImx678::GetShutterSpeed()
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

    std::pair<float, float> FramosImx678::GetShutterSpeedRange()
    {
        return std::make_pair(0.01f, 100.00f);
    }

    float FramosImx678::GetShutterSpeedStep()
    {
        return 0.01f;
    }

    void FramosImx678::SetAnalogueGain(float gain)
    {
        uint16_t setting = static_cast<uint16_t>(66.67f * log10f(gain));
        if (setting > 240) setting = 240;
        I2C_WRITE(REG52, setting & REG52_MASK);
    }

    float FramosImx678::GetAnalogueGain()
    {
        uint16_t setting = I2CRead(REG52) & REG52_MASK;

        return powf(10.0f, static_cast<float>(setting) / 66.67f);
    }

    std::pair<float, float> FramosImx678::GetAnalogueGainRange()
    {
        return std::make_pair(1.0f, 31.6f);
    }

    float FramosImx678::GetAnalogueGainStep()
    {
        return 0.1f;
    }

    void FramosImx678::SetHDRState(bool state)
    {
        _hdrEnabled = state;

        I2C_WRITE(REG1, 0x1);
        I2C_WRITE(REG2, 0x0);
        I2C_WRITE(REG3, 0x0);

        I2C_WRITE(REG4, 0x1);
        I2C_WRITE(REG5, state ? 0x3 : 0x2);

        I2C_WRITE(REG6, 0x04);


        I2C_WRITE(REG8, state ? 0x8 : 0x0);

        I2C_WRITE(REG16, state ? 0x94 : 0xCA);
        I2C_WRITE(REG17, state ? 0x11 : 0x08);
        I2C_WRITE(REG18, 0x00);

        I2C_WRITE(REG19, state ? 0x94 : 0x4C);
        I2C_WRITE(REG20, state ? 0x02 : 0x04);

        I2C_WRITE(REG51, state ? 0x4 : 0x0);
        I2C_WRITE(REG54, state ? 0x2 : 0x0);



        I2C_WRITE(REG61, 0xAA);
        I2C_WRITE(REG62, 0x0);

        I2C_WRITE(0x3460, 0x22);

        I2C_WRITE(0x355A, state ? 0x0 : 0x64);
        I2C_WRITE(0x3A20, state ? 0x34 : 0x2B);
        I2C_WRITE(0x3A24, state ? 0x44 : 0x22);
        I2C_WRITE(0x3A25, 0x25);
        I2C_WRITE(0x3A26, state ? 0x4E : 0x2A);
        I2C_WRITE(0x3A27, 0x2C);
        I2C_WRITE(0x3A28, state ? 0x57 : 0x39);

        I2C_WRITE(0x3A64, state ? 0x1 : 0x0);

        I2C_WRITE(0x3C37, state ? 0x30 : 0x10);
        I2C_WRITE(0x3CF2, state ? 0x78 : 0xFF);
        I2C_WRITE(0x3CF3, state ? 0x0 : 0x3);

        if (!state)
        {
            I2C_WRITE(0x3CF4, 0x0);
            I2C_WRITE(0x3EB6, 0x4D);
        }
        else
        {
            bool is12bit = I2CRead(REG14) & 0x1;
            I2C_WRITE(0x3CF4, is12bit ? 0xAA : 0xA5);
            I2C_WRITE(0x3EB6, is12bit ? 0xAA : 0xA5);
        }

        I2C_WRITE(0x3EB4, state ? 0x7B : 0xB);
        I2C_WRITE(0x3EB5, state ? 0x0 : 0x2);

        I2C_WRITE(0x3EB7, state ? 0x40 : 0x42);

        I2C_WRITE(0x3F24, state ? 0x17 : 0x10);

        I2C_WRITE(0x3F4C, state ? 0x2D : 0x00);


        I2C_WRITE(0x4420, state ? 0xFF : 0xB);
        I2C_WRITE(0x4421, state ? 0x3 : 0x2);
        I2C_WRITE(0x4422, state ? 0x0 : 0x4D);
        I2C_WRITE(0x4423, state ? 0x8 : 0xA);
        I2C_WRITE(0x44A4, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44A6, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44A8, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44AA, state ? 0x37 : 0x2C);

        I2C_WRITE(0x44B4, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44B6, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44B8, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44BA, state ? 0x37 : 0x2C);

        I2C_WRITE(0x44C4, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44C6, state ? 0x37 : 0x2C);
        I2C_WRITE(0x44C8, state ? 0x37 : 0x2C);


        I2C_WRITE(0x453D, state ? 0x18 : 0x1B);
        I2C_WRITE(0x453E, state ? 0x18 : 0x1B);
        I2C_WRITE(0x453F, state ? 0x11 : 0x15);
        I2C_WRITE(0x4540, state ? 0x11 : 0x15);
        I2C_WRITE(0x4541, state ? 0x11 : 0x15);
        I2C_WRITE(0x4542, state ? 0x11 : 0x15);
        I2C_WRITE(0x4543, state ? 0x11 : 0x15);
        I2C_WRITE(0x4544, state ? 0x11 : 0x15);
        I2C_WRITE(0x4548, 0x0);
        I2C_WRITE(0x4549, state ? 0x0 : 0x1);
        I2C_WRITE(0x454A, state ? 0x0 : 0x1);
        I2C_WRITE(0x454B, state ? 0x4 : 0x6);
        I2C_WRITE(0x454C, state ? 0x4 : 0x6);
        I2C_WRITE(0x454D, state ? 0x4 : 0x6);
        I2C_WRITE(0x454E, state ? 0x4 : 0x6);
        I2C_WRITE(0x454F, state ? 0x4 : 0x6);
        I2C_WRITE(0x4550, state ? 0x4 : 0x6);

        // if (!state)
        // {
        //     // Plan B
        //     Reset();
        // }

        I2C_WRITE(REG1, 0x0);
        I2C_WRITE(REG1, 0x1);
        I2C_WRITE(REG1, 0x0);
    }

    bool FramosImx678::GetHDRState()
    {
        return I2CRead(REG8) == 0x8;
    }

    std::pair<uint16_t, uint16_t> FramosImx678::GetResolution()
    {
        uint16_t h = (I2CRead(REG27) & REG27_MASK) << 8;
        h |= I2CRead(REG26) & REG26_MASK;

        uint16_t v = (I2CRead(REG34) & REG34_MASK) << 8;
        v |= I2CRead(REG33) & REG33_MASK;

        return {h, v};
    }

    void FramosImx678::I2CWrite16(uint16_t addr, uint16_t word)
    {
        uint8_t writeBytes[4] = {(uint8_t)(addr >> 8),
                                 (uint8_t)(addr & 0xFF),
                                 (uint8_t)(word >> 8),
                                 (uint8_t)(word & 0xFF)};
        _spI2CDevice->Write(writeBytes, 4);
    }

    void FramosImx678::I2CWrite(uint16_t addr, uint8_t byte)
    {
        _spI2CDevice->WriteRegister(addr, byte);
    }

    uint8_t FramosImx678::I2CRead(uint16_t addr)
    {
        // Pre-initialise with 0xff to indicate read failure
        uint8_t store{0xff};
        _spI2CDevice->ReadRegister(addr, store);
        return store;
    }

    void FramosImx678::WriteRegs(const img678_reg* p)
    {
        while(p->addr != IMX678_TABLE_END)
        {
            if(p->addr == IMX678_TABLE_WAIT_MS)
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


    void FramosImx678::CtlWrite(const uint32_t addr, const uint32_t val)
    {
        I2CWrite(static_cast<uint16_t>(addr & 0xffff), static_cast<uint8_t>(val & 0xff));
    }


    uint32_t FramosImx678::CtlRead(const uint32_t addr)
    {
        return I2CRead(static_cast<uint16_t>(addr & 0xffff));
    }
}
