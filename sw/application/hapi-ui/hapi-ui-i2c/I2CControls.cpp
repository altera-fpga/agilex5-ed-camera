/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "I2CControls.h"
#include "AtUtils.h"

I2CControls::I2CControls(const std::shared_ptr<SwApi::I2C>& spI2C)
    : _spI2C(spI2C)
{
    auto buses = _spI2C->GetBuses();
    for (auto spBus : buses)
    {
        auto devices = spBus->GetDevices();
        for (auto spDevice : devices)
        {
            _devices.push_back(std::make_shared<I2CDeviceControls>(spDevice));
        }
    }

    _strDevices = "I&#178;C Devices";
}

std::vector<std::shared_ptr<UiControlContainer>> I2CControls::AddUiElements()
{
    if (!_spI2C)
        return {};

    auto spContainer = std::make_shared<UiControlContainer>(_strDevices, GetSettingsSectionName());

    bool first = true;
    for (auto spDevice : _devices)
    {
        if (!first)
            spContainer->AddSeparator();
        first = false;

        spDevice->AddUiElements(spContainer);
    }

    return { std::move(spContainer) };
}

///////////////////////////////////////////////////////////////////////////////

I2CDeviceControls::I2CDeviceControls(const std::shared_ptr<SwApi::I2CDeviceBase>& spDevice)
:	_spDevice(spDevice)
{
    _strDeviceName = "Device name";
    _strBus = "Bus ID";
    _strDeviceAddress = "Device address";
    _strRegisterAddress = "Register address";
    _strValue = "I&#178;C value";
    _strHexPrefix = "0x";
}

void I2CDeviceControls::AddUiElements(std::shared_ptr<UiControlContainer> spContainer)
{
    if (_spDevice && spContainer)
    {
        std::string deviceName = _spDevice->GetName();
        spContainer->AddLabelControl(_strDeviceName, deviceName);

        std::string busIdString = AtUtils::FormatString("%d", _spDevice->GetBusID());
        spContainer->AddLabelControl(_strBus, busIdString);

        std::string deviceAddressString = AtUtils::FormatString("0x%x", _spDevice->GetAddress());
        spContainer->AddLabelControl(_strDeviceAddress, deviceAddressString);

        auto registerAddressCB = [this](uint32_t clientId, int32_t& value) { _registerAddress = (uint16_t)value; };
        spContainer->AddIntegerControl(_strRegisterAddress, _strHexPrefix, "", _registerAddress, 0, 0xffff, registerAddressCB, true);

        auto i2cValueCB = [this](uint32_t clientId, int32_t& value) { _spDevice->WriteRegisterFromU32(_registerAddress, static_cast<uint32_t>(value)); };
        spContainer->AddIntegerControl(_strValue, _strHexPrefix, "", 0x80, 0, 0xff, i2cValueCB, true);
    }
}
