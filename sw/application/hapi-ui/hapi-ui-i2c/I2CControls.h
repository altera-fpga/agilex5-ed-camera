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
#include "I2C.h"

class I2CDeviceControls
{
public:
    I2CDeviceControls(const std::shared_ptr<SwApi::I2CDeviceBase>& spDevice);
    void AddUiElements(std::shared_ptr<UiControlContainer> spContainer);

private:
    std::shared_ptr<SwApi::I2CDeviceBase> _spDevice;
    uint16_t _registerAddress = 0;
    std::string _strDeviceName;
    std::string _strBus;
    std::string _strDeviceAddress;
    std::string _strRegisterAddress;
    std::string _strValue;
    std::string _strHexPrefix;
};

class I2CControls : public IpUiControls
{
public:
    I2CControls(const std::shared_ptr<SwApi::I2C>& spI2C);
    std::vector<std::shared_ptr<UiControlContainer>> AddUiElements() override;
	std::string GetSettingsSectionName() override { return "I2C"; }

private:
    std::shared_ptr<SwApi::I2C> _spI2C;
    std::vector<std::shared_ptr<I2CDeviceControls>> _devices;
    std::string _strDevices;
};
