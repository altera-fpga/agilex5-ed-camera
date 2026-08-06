/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "I2C.h"
#include "I2CImpl.h"
#include "SwUtils.h"

namespace SwApi
{
    static std::weak_ptr<I2C> wpInstance;


    std::shared_ptr<I2C> I2C::Create()
    {
        std::shared_ptr<I2C> spInstance = wpInstance.lock();

        if (spInstance)
            return spInstance;

        auto spI2C = std::make_shared<I2C>(nullptr, nullptr);
        wpInstance = spI2C;
        return spI2C;
    }


    I2C::I2C(const std::shared_ptr<II2CBusEnumerator>& spBusEnumerator,
        std::function<std::shared_ptr<II2CBusIo>(uint32_t)> busIoFactory)
    {
        auto spEnumerator = spBusEnumerator ? spBusEnumerator : std::make_shared<LinuxI2CBusEnumerator>();
        for (const auto& busInfo : spEnumerator->EnumerateBuses())
        {
            std::shared_ptr<II2CBusIo> spBusIo = busIoFactory ? busIoFactory(busInfo.busID) : std::make_shared<I2CBusIo>(busInfo.busID);

            auto spBus = std::make_shared<I2CBus>(busInfo.busID, busInfo.sysClassName, spBusIo);
            _buses.emplace(busInfo.busID, spBus);
            _busIDs.push_back(busInfo.busID);
        }
    }


    std::vector<uint32_t> I2C::GetBusIDs() const
    {
        return _busIDs;
    }

    std::shared_ptr<I2CBus> I2C::GetBus(uint32_t busID)
    {
        std::shared_ptr<I2CBus> spBus;
        if (SwUtils::Lookup<uint32_t, std::shared_ptr<I2CBus>>(_buses, busID, spBus))
        {
            return spBus;
        }
        else
            return nullptr;
    }

    std::vector<std::shared_ptr<I2CBus>> I2C::GetBuses()
    {
        std::vector<std::shared_ptr<I2CBus>> buses;
        for (auto& busItem : _buses)
        {
            auto bus = busItem.second;
            buses.emplace_back(bus);
        }

        return buses;
    }


    I2CBus::I2CBus(uint32_t busID, const std::string& sysClassName)
    : I2CBus(busID, sysClassName, std::make_shared<I2CBusIo>(busID))
    {
    }

    I2CBus::I2CBus(uint32_t busID, const std::string& sysClassName, std::shared_ptr<II2CBusIo> spBusIo)
    :	_busID(busID)
    ,   _sysClassName(sysClassName)
    ,   _spBusIo(std::move(spBusIo))
    {
    }

    std::vector<std::shared_ptr<I2CDeviceBase>> I2CBus::GetDevices()
    {
        return _devices;
    }

    // Make sure you don't release the shared_pointer returned by SwApi::II2C::Create
    // otherwise the I2CBusIo will get released too
    I2CDeviceBase::I2CDeviceBase(std::weak_ptr<II2CBusIo> wpBus,
                                                        uint16_t deviceAddress, 
                                                        const std::string& deviceName)
    :	_wpBus(wpBus)
    ,	_deviceAddress(deviceAddress)
    ,	_deviceName(deviceName)
    {
    }

    bool I2CDeviceBase::Read(uint8_t* data, uint16_t size)
    {
        auto spBus = _wpBus.lock();
        return spBus ? spBus->Read(_deviceAddress, data, size) : false;
    }

    bool I2CDeviceBase::Write(const uint8_t* data, uint16_t size)
    {
        auto spBus = _wpBus.lock();
        return spBus ? spBus->Write(_deviceAddress, data, size) : false;
    }

    uint32_t I2CDeviceBase::GetBusID()
    {
        auto spBus = _wpBus.lock();
        return spBus ? spBus->GetID() : 0;
    }

    uint16_t I2CDeviceBase::GetAddress()
    {
        return _deviceAddress;
    }

    std::string I2CDeviceBase::GetName()
    {
        return _deviceName;
    }
}
