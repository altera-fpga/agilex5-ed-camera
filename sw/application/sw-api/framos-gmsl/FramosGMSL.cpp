/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#include <iostream>
#include <chrono>
#include <thread>

#include "FramosGMSL.h"
#include "I2C.h"

namespace SwApi
{
    bool FramosGMSL::Create(const uint32_t idx)
    {
        bool rc = false;

        using TFramosSerDesDevice = I2CDevice<Reg<16>, Val<8>>;
        using TFramosGPIODevice = I2CDevice<Reg<8>, Val<8>>;

        // Attempt to configure GMSL serializer/deserializers
        auto I2CWrite = [](const std::shared_ptr<TFramosSerDesDevice>& device, uint16_t addr, uint8_t byte)
        {
            ///uint8_t writeBytes[3] = {(uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF), byte};
            //return device->Write(writeBytes, 3);
            device->WriteRegister(addr, byte);
        };

        static constexpr uint8_t serDesBus = 0x1;
        static constexpr uint8_t gmslDevAddr[2][3] = {
            // Serialiser   Deserialiser    GPIO
            {0x42,          0x6a,           0x21},  // Primary GMSL kit on MIPI0
            {0x40,          0x4c,           0x20}   // Optional GMSL kit on MIPI1
        };

        std::shared_ptr<I2C> spI2C = I2C::Create();

        
        std::shared_ptr<TFramosSerDesDevice> spI2CDeviceSer;
        std::shared_ptr<TFramosSerDesDevice> spI2CDeviceDes;
        std::shared_ptr<TFramosGPIODevice> spI2CDeviceGPIO;

        if(idx > 1)
            return rc;

        const uint8_t* pGmslDev = gmslDevAddr[(idx == 0 ? 0 : 1)];

        auto cameraBus = spI2C->GetBus(serDesBus);

        spI2CDeviceSer = cameraBus->AddDevice<TFramosSerDesDevice>(pGmslDev[0], "Framos FFA-GMSL-SER-V2A " + std::to_string(idx));
        spI2CDeviceDes = cameraBus->AddDevice<TFramosSerDesDevice>(pGmslDev[1], "Framos FFA-GMSL-DES-V2A " + std::to_string(idx));
        spI2CDeviceGPIO = cameraBus->AddDevice<TFramosGPIODevice>(pGmslDev[2], "Framos GPIO " + std::to_string(idx));        

        // Adapted from Framos Jetson drivers
        // https://github.com/framosimaging/framos-jetson-drivers.git

        // Configure GPIO expander
        // Set a fixed i2c slave address of 0x36 or 0x10 for the sensor
        const uint8_t gpio_val = (idx == 0 ? 0xfe : 0xfd);
        static constexpr uint8_t TCA6408_OUT_REG = 0x1;
        static constexpr uint8_t TCA6408_CFG_REG = 0x3;
        static constexpr uint8_t TCA6408_CFG_REG_VAL = 0xfc;    // Set pins 1 and 0 as output

        if(spI2CDeviceGPIO->WriteRegister(TCA6408_OUT_REG, gpio_val) &&
            spI2CDeviceGPIO->WriteRegister(TCA6408_CFG_REG, TCA6408_CFG_REG_VAL))
        {
            // imx678_board_setup()
            // imx678_gmsl_serdes_setup()
            // max96792_reset_control()
            I2CWrite(spI2CDeviceDes, 0x0010, 0x81);
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            // max96792_gmsl_setup()
            I2CWrite(spI2CDeviceDes, 0x0001, 0x03);
            I2CWrite(spI2CDeviceDes, 0x0004, 0xC3);
            I2CWrite(spI2CDeviceDes, 0x0028, 0x62);
            I2CWrite(spI2CDeviceDes, 0x2001, 0x01);
            I2CWrite(spI2CDeviceDes, 0x2101, 0x01);
            I2CWrite(spI2CDeviceDes, 0x0443, 0x81);
            I2CWrite(spI2CDeviceDes, 0x0444, 0x81);
            // max96793_gmsl_setup()
            I2CWrite(spI2CDeviceSer, 0x14CE, 0x19);
            I2CWrite(spI2CDeviceSer, 0x0001, 0x0C);
            I2CWrite(spI2CDeviceSer, 0x0006, 0x11);
            I2CWrite(spI2CDeviceSer, 0x0028, 0x62);
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            I2CWrite(spI2CDeviceSer, 0x0010, 0x21);
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            // max96792_setup_link()
            I2CWrite(spI2CDeviceDes, 0x0010, 0x21);
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            // max96793_setup_control()
            I2CWrite(spI2CDeviceSer, 0x0040, 0x16);
            I2CWrite(spI2CDeviceSer, 0x02BE, 0x84);
            I2CWrite(spI2CDeviceSer, 0x02C0, 0x4F);
            I2CWrite(spI2CDeviceSer, 0x02D6, 0x90);
            // max96793_gpio10_xtrig1_setup()
            I2CWrite(spI2CDeviceSer, 0x02D0, 0x80);
            // max96792_setup_control()
            I2CWrite(spI2CDeviceDes, 0x0161, 0x01);
            I2CWrite(spI2CDeviceDes, 0x1D00, 0xF4);
            I2CWrite(spI2CDeviceDes, 0x0320, 0x38);
            I2CWrite(spI2CDeviceDes, 0x1D00, 0xF5);
            I2CWrite(spI2CDeviceDes, 0x0040, 0x16);
            I2CWrite(spI2CDeviceDes, 0x02C5, 0x83);
            I2CWrite(spI2CDeviceDes, 0x02C6, 0x6F);
            I2CWrite(spI2CDeviceDes, 0x02C8, 0x83);
            // imx678_start_streaming()
            // max96793_setup_streaming()
            I2CWrite(spI2CDeviceSer, 0x0330, 0x08);
            I2CWrite(spI2CDeviceSer, 0x0330, 0x00);
            I2CWrite(spI2CDeviceSer, 0x0331, 0x70);
            I2CWrite(spI2CDeviceSer, 0x031E, 0x2C);
            I2CWrite(spI2CDeviceSer, 0x0111, 0x4C);
            I2CWrite(spI2CDeviceSer, 0x0110, 0x28);
            I2CWrite(spI2CDeviceSer, 0x005B, 0x01);
            I2CWrite(spI2CDeviceSer, 0x0383, 0x80);
            // max96792_setup_streaming()
            I2CWrite(spI2CDeviceDes, 0x0474, 0x19);
            // max96792_start_streaming()
            I2CWrite(spI2CDeviceDes, 0x0112, 0x30);
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            I2CWrite(spI2CDeviceDes, 0x0112, 0x31);
            std::cout << "[GMSL" << idx << "] device intialised\n";
            rc = true;
        }
        else
        {
            std::cout << "[GMSL" << idx << "] device not available\n";
        }

        return rc;
    }
}